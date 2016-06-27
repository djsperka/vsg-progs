#include "unic.h"
#include <QScrollBar>
#include <QState>
#include <map>

using namespace std;

// status reply values. Taken from 
// CoreGUI TCP Server document dated 14 April, 2016

static const std::pair<int, QString> arr[] = {
	std::pair<int, QString>(230, "IDLE"),
	std::pair<int, QString>(231, "EEG_ON"),
	std::pair<int, QString>(232, "EEG_OFF"),
	std::pair<int, QString>(233, "TEMPLATE_NOT_LOADED"),
	std::pair<int, QString>(234, "TEMPLATE_LOADING"),
	std::pair<int, QString>(235, "TEMPLATE_LOADED"),
	std::pair<int, QString>(236, "STIMULATION_READY"),
	std::pair<int, QString>(237, "STIMULATION_RAMPUP"),
	std::pair<int, QString>(238, "STIMULATION_FULL"),
	std::pair<int, QString>(239, "STIMULATION_RAMPDOWN"),
	std::pair<int, QString>(240, "STIMULATION_FINISHED"),
	std::pair<int, QString>(241, "REMOTE_CONTROL_ALLOWED"),
	std::pair<int, QString>(242, "REMOTE_CONTROL_REJECTED"),
	std::pair<int, QString>(243, "STIMULATION_ABORTING"),
	std::pair<int, QString>(244, "DEVICE_NOT_RESENT"),
	std::pair<int, QString>(245, "PRE_EEG"),
	std::pair<int, QString>(246, "POST_EEG"),
	std::pair<int, QString>(247, "WAITING_FOR_SECOND_SHAM"),
	std::pair<int, QString>(248, "EEG_RECORDING_ON"),
	std::pair<int, QString>(249, "SYNCHRONIZING"),
	std::pair<int, QString>(250, "VERSION_RECEIVED"),
};

static std::map<int, QString> f_mapNICStatusStrings(arr, arr + sizeof(arr) / sizeof(arr[0]));

unic::unic(const QString& commandFile, QTcpSocket& socket, QWidget *parent)
: QMainWindow(parent)
, m_file(commandFile)
, m_socket(socket)
{
	m_nicCommandFileWatcher.addPath(commandFile);
	ui.setupUi(this);

	ui.labelCommandFile->setText(commandFile);
	ui.plainTextEditLog->setReadOnly(true);
	ui.plainTextEditLog->verticalScrollBar()->setValue(ui.plainTextEditLog->verticalScrollBar()->maximum());

	QPalette* palette = new QPalette();
	palette->setBrush(QPalette::Base, *(new QBrush(*(new QPixmap(":/unic/mphone.jpg")))));

	ui.plainTextEditLog->setPalette(*palette);

	buildStateMachine();
}

unic::~unic()
{
	m_pMachine->stop();
	delete m_pMachine;
}


void unic::buildStateMachine()
{
	m_pMachine = new QStateMachine;

	// timer will be used to signal timeout while waiting for status from NIC. 
	m_timer.setInterval(2000);
	m_timer.setSingleShot(true);

	// create states and add to machine
	QState *sIdle = new QState(m_pMachine);
	QState *sChanged = new QState(m_pMachine);
	QState *sTimeout = new QState(m_pMachine);
	QState *sStatus = new QState(m_pMachine);

	// there's stuff to do on entering these states
	connect(sChanged, SIGNAL(entered()), this, SLOT(fileChangedStateEntered()));
	connect(sTimeout, SIGNAL(entered()), this, SLOT(timeoutStateEntered()));
	connect(sStatus, SIGNAL(entered()), this, SLOT(statusStateEntered()));

	// Initial state is Idle. Transition when file changes.
	// The fileChangedStateEntered() slot will be called on entry to sChanged.
	m_pMachine->setInitialState(sIdle);
	sIdle->addTransition(&m_nicCommandFileWatcher, SIGNAL(fileChanged(const QString&)), sChanged);

	// After command is read and sent, status command is sent. When the reply comes, the socket
	// emits readyRead() - which causes transition to sStatus. 
	sChanged->addTransition(&m_socket, SIGNAL(readyRead()), sStatus);

	// The timeout is another way out of sChanged.
	sChanged->addTransition(&m_timer, SIGNAL(timeout()), sTimeout);
	sTimeout->addTransition(sIdle);

	// sStatus transition back to Idle.
	sStatus->addTransition(sIdle);

	m_pMachine->start();
}


// The readyRead() signal was emitted from the socket -- just read whatever is there. 

void unic::statusStateEntered()
{
	qInfo() << "Got status reply from NIC";

	// read status
	bool bOK;
	QByteArray ba = m_socket.readAll();

	// status reply should be a single byte.
	if (ba.size() == 1)
	{
		unsigned int status = (unsigned char)ba.at(0);
		if (f_mapNICStatusStrings.count(status) == 1)
		{
			qInfo() << QString("NIC status (%1): %2").arg(status).arg(f_mapNICStatusStrings[status]);
		}
		else
		{
			qWarning() << "Unknown status value from NIC: " << status;
		}
	}
	else
	{
		qWarning() << "Expecting single byte status response from NIC, but we got : " << ba << " length " << ba.size();
	}
}

// The command file has changed. Read and send command+parameters

void unic::fileChangedStateEntered()
{
	qInfo() << "Command file change detected";

	if (m_file.open(QIODevice::ReadOnly))
	{
		QByteArray ba = m_file.readAll();
		m_file.close();

		// there should be two lines in file
		QList<QByteArray> list = ba.split('\n');
		qDebug() << "FileChangedState: found " << list.size() << " lines.";
		if (list.size() == 2)
		{
			// first line should be an integer between 200 and 230
			// second line should be a string, and it must be terminated 
			// with '\n' (even if it has no parameters)
			bool bOK;
			int commandNumber = list.at(0).trimmed().toInt(&bOK);
			QByteArray params;
			QByteArray command;
			if (bOK)
			{
				if (commandNumber > 199 && commandNumber < 230)
				{
					char command[2];
					command[0] = (unsigned char)(commandNumber);
					command[1] = 0;

					params = list.at(1).trimmed();
					params.append('\n');

					// write command number as a single byte
					qInfo() << "Sending command number: " << commandNumber;
					m_socket.write(command, 1);

					// write parameters
					qInfo() << "Sending parameter list: " << params;
					m_socket.write(params);

					// request status
					command[0] = 200;
					command[1] = 0;
					params.clear();
					params.append('\n');
					qInfo() << "Sending status request...";
					m_socket.write(command, 1);
					m_socket.write(params);
					m_timer.start();
				}
				else
				{
					qWarning() << "Bad command number: should be in [200,230) - got " << commandNumber;
				}
			}
			else
			{
				qWarning() << "Bad command number: should be an integer, got " << list.at(0).constData();
			}
		}
	}
	else
	{
		qCritical() << "Cannot open file " << m_file.fileName();
	}



}

void unic::timeoutStateEntered()
{
	qWarning() << "Timeout waiting for NIC status reply.";
}


void unic::message(const QString& msg)
{
	ui.plainTextEditLog->appendPlainText(msg);
}

