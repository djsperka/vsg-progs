#include "unic.h"
#include <QScrollBar>
#include <QState>
#include <QHostAddress>
#include <map>

using namespace std;


#define USE_PARALLEL_STATES

// status reply values. Taken from 
// CoreGUI TCP Server document dated 14 April, 2016

static const std::pair<int, QString> arr[] = {
	std::pair<int, QString>(230, "REMOTE_CONTROL_ALLOWED"),
	std::pair<int, QString>(231, "REMOTE_CONTROL_REJECTED"),
	std::pair<int, QString>(232, "VERSION_RECEIVED"),
	std::pair<int, QString>(233, "SYNCHRONIZING"),
	std::pair<int, QString>(234, "IDLE"),
	std::pair<int, QString>(235, "PROTOCOL_NOT_LOADED"),
	std::pair<int, QString>(236, "PROTOCOL_LOADING"),
	std::pair<int, QString>(237, "PROTOCOL_LOADED"),
	std::pair<int, QString>(238, "PROTOCOL_RUNNING"),
	std::pair<int, QString>(239, "PROTOCOL_ERROR"),
	std::pair<int, QString>(240, "PROTOCOL_FINISHED"),
	std::pair<int, QString>(241, "PROTOCOL_PAUSED"),
	std::pair<int, QString>(242, "PROTOCOL_ABORTED"),
	std::pair<int, QString>(243, "PROTOCOL_ABORTING"),
	std::pair<int, QString>(244, "EEG_ON"),
	std::pair<int, QString>(245, "EEG_OFF"),
	std::pair<int, QString>(246, "STIMULATION_RAMPUP"),
	std::pair<int, QString>(247, "STIMULATION_FULL"),
	std::pair<int, QString>(248, "STIMULATION_RAMPDOWN"),
	std::pair<int, QString>(249, "STIMULATION_FINISHED"),
	std::pair<int, QString>(250, "WAITING_FOR_SECOND_SHAM"),
	std::pair<int, QString>(251, "CHECK_IMPEDANCE"),
	std::pair<int, QString>(252, "CHECK_IMPEDANCE_FINISHED"),
	std::pair<int, QString>(253, "CHECK_IMPEDANCE_ABORTING"),
	std::pair<int, QString>(254, "CHECK_IMPEDANCE_ABORTED")
};

static std::map<int, QString> f_mapNICStatusStrings(arr, arr + sizeof(arr) / sizeof(arr[0]));

static const std::pair<int, int> arr2[] = {
	std::pair<int, int>(201, 248),	// start eeg
	std::pair<int, int>(202, 232),	// stop eeg
	std::pair<int, int>(203, 236),	// load template
	std::pair<int, int>(204, 236),	// abort stim
	std::pair<int, int>(205, 238),	// start stim
	std::pair<int, int>(214, 230)
};

static std::map<int, int> f_mapNICExpectedResponses(arr2, arr2 + sizeof(arr2) / sizeof(arr2[0]));

unic::unic(const QString& commandFile, QTcpSocket& socket, QWidget *parent)
: QMainWindow(parent)
, m_file(commandFile)
, m_socket(socket)
, m_lastNICStatus(0)
{
	m_nicCommandFileWatcher.addPath(commandFile);
	ui.setupUi(this);

	ui.labelCommandFile->setText(commandFile);
	ui.plainTextEditLog->setReadOnly(true);
	ui.plainTextEditLog->verticalScrollBar()->setValue(ui.plainTextEditLog->verticalScrollBar()->maximum());

	ui.labelNICServerIP->setText(QString("%1:%2").arg(socket.peerAddress().toString()).arg(socket.peerPort()));

//	QPalette* palette = new QPalette();
//	palette->setBrush(QPalette::Base, *(new QBrush(*(new QPixmap(":/unic/mphone.jpg")))));
//	ui.plainTextEditLog->setPalette(*palette);

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

#ifdef USE_PARALLEL_STATES
	// timer will be used to check status periodically. 
	m_timerSendStatusCommand.setInterval(1000);
	m_timerSendStatusCommand.setSingleShot(true);

	// this timer is for waiting on status response
	m_timerReadStatusResponse.setInterval(500);
	m_timerReadStatusResponse.setSingleShot(true);
	m_timerReadProtocolStatusResponse.setInterval(500);
	m_timerReadProtocolStatusResponse.setSingleShot(true);

	// Parent State - parent to two parallel state sets.
	QState *sParent = new QState(QState::ParallelStates, m_pMachine);
	m_pMachine->setInitialState(sParent);
	QState *sCommandLoop = new QState(sParent);
	QState *sStatusLoop = new QState(sParent);

	// The command loop has two states. 
	// One state set waits for changes in the command file. 
	// When change is detected, read and send commands (see sChanged::onEntry)

	QState *sIdle = new QState(sCommandLoop);
	QState *sChanged = new QState(sCommandLoop);
	sCommandLoop->setInitialState(sIdle);

	// transitions

	sIdle->addTransition(&m_nicCommandFileWatcher, SIGNAL(fileChanged(const QString&)), sChanged);
	sChanged->addTransition(sIdle);

	// there's stuff to do on entering these sChanged state

	connect(sChanged, SIGNAL(entered()), this, SLOT(fileChangedStateEntered()));
	connect(sChanged, SIGNAL(entered()), this, SIGNAL(fileChangeDetected()));


	// The status loop has three states. The idle state is exited every
	// timeout of the timer (which is started in sChillin::onEntry)

	QState* sIdleStatus = new QState(sStatusLoop);
	QState* sSendStatusCommand = new QState(sStatusLoop);
	QState* sReadStatusResponse = new QState(sStatusLoop);
	QState* sReadStatusTimeout = new QState(sStatusLoop);
	QState* sSendProtocolStatusCommand = new QState(sStatusLoop);
	QState* sReadProtocolStatusResponse = new QState(sStatusLoop);
	QState* sReadProtocolStatusTimeout = new QState(sStatusLoop);
	sStatusLoop->setInitialState(sIdleStatus);

	// transitions for status loop states

	connect(sIdleStatus, SIGNAL(entered()), this, SLOT(idleStatusStateEntered()));
	sIdleStatus->addTransition(&m_timerSendStatusCommand, SIGNAL(timeout()), sSendStatusCommand);
	connect(sSendStatusCommand, SIGNAL(entered()), this, SLOT(sendStatusCommandStateEntered()));	// send status command, start m_timerReadStatusResponse;
	sSendStatusCommand->addTransition(&m_socket, SIGNAL(readyRead()), sReadStatusResponse);
	sReadStatusResponse->addTransition(sSendProtocolStatusCommand);
	connect(sReadStatusResponse, SIGNAL(entered()), this, SLOT(readStatusResponseStateEntered()));
	sSendStatusCommand->addTransition(&m_timerReadStatusResponse, SIGNAL(timeout()), sReadStatusTimeout);
	connect(sReadStatusTimeout, SIGNAL(entered()), this, SLOT(readStatusTimeoutStateEntered()));
	sReadStatusTimeout->addTransition(sIdleStatus);

	connect(sSendProtocolStatusCommand, SIGNAL(entered()), this, SLOT(sendProtocolStatusCommandStateEntered()));	// send protocol status command, start m_timerReadProtocolStatusResponse;
	sSendProtocolStatusCommand->addTransition(&m_socket, SIGNAL(readyRead()), sReadProtocolStatusResponse);
	sReadProtocolStatusResponse->addTransition(sIdleStatus);
	connect(sReadProtocolStatusResponse, SIGNAL(entered()), this, SLOT(readProtocolStatusResponseStateEntered()));
	sSendProtocolStatusCommand->addTransition(&m_timerReadProtocolStatusResponse, SIGNAL(timeout()), sReadProtocolStatusTimeout);
	connect(sReadProtocolStatusTimeout, SIGNAL(entered()), this, SLOT(readProtocolStatusTimeoutStateEntered()));
	sReadProtocolStatusTimeout->addTransition(sIdleStatus);

	// finally, set up slot to catch changes in nic status emitted by the status loop
	connect(this, SIGNAL(nicStatusChangedSignal(int, int)), this, SLOT(nicStatusChanged(int, int)));

#else

	QState *sCommandLoop = new QState(m_pMachine);
	QState *sIdle = new QState(m_pMachine);
	QState *sChanged = new QState(m_pMachine);

	QState* sSendStatusCommand = new QState(m_pMachine);
	QState* sReadStatusResponse = new QState(m_pMachine);
	QState* sReadStatusTimeout = new QState(m_pMachine);

	// this timer is for waiting on status response
	m_timerReadStatusResponse.setInterval(500);
	m_timerReadStatusResponse.setSingleShot(true);


	m_pMachine->setInitialState(sIdle);

	// transitions

	sIdle->addTransition(&m_nicCommandFileWatcher, SIGNAL(fileChanged(const QString&)), sChanged);
	sChanged->addTransition(sSendStatusCommand);

	connect(sSendStatusCommand, SIGNAL(entered()), this, SLOT(sendStatusCommandStateEntered()));	// send status command, start m_timerReadStatusResponse;
	sSendStatusCommand->addTransition(&m_socket, SIGNAL(readyRead()), sReadStatusResponse);
	sReadStatusResponse->addTransition(sIdle);
	connect(sReadStatusResponse, SIGNAL(entered()), this, SLOT(readStatusResponseStateEntered()));
	sSendStatusCommand->addTransition(&m_timerReadStatusResponse, SIGNAL(timeout()), sReadStatusTimeout);
	connect(sReadStatusTimeout, SIGNAL(entered()), this, SLOT(readStatusTimeoutStateEntered()));
	sReadStatusTimeout->addTransition(sIdle);



	// there's stuff to do on entering these sChanged state

	connect(sChanged, SIGNAL(entered()), this, SLOT(fileChangedStateEntered()));
	connect(sChanged, SIGNAL(entered()), this, SIGNAL(fileChangeDetected()));



#endif



	m_pMachine->start();
}


void unic::idleStatusStateEntered()
{
	// start timer
	m_timerSendStatusCommand.start();
}

void unic::sendStatusCommandStateEntered()
{
	char command[2];

//	qInfo() << "Sending status request...";

	// get lock
	m_socketMutex.lock();

	// request status command
	command[0] = 200;
	command[1] = 0;
	m_socket.write(command, 1);

	// and the (empty) parameter list
	command[0] = '\n';
	m_socket.write(command, 1);

	// unlock mutex
	m_socketMutex.unlock();

	// start timeout timer
	m_timerReadStatusResponse.start();
}

void unic::sendProtocolStatusCommandStateEntered()
{
	char command[2];

	//	qInfo() << "Sending status request...";

	// get lock
	m_socketMutex.lock();

	// request status command
	command[0] = 201;
	command[1] = 0;
	m_socket.write(command, 1);

	// and the (empty) parameter list
	command[0] = '\n';
	m_socket.write(command, 1);

	// unlock mutex
	m_socketMutex.unlock();

	// start timeout timer
	m_timerReadProtocolStatusResponse.start();
}


// The readyRead() signal was emitted from the socket -- just read whatever is there. 

void unic::readStatusResponseStateEntered()
{
//	qInfo() << "Got status reply from NIC";

	// read status
	m_socketMutex.lock();
	QByteArray ba = m_socket.readAll();
	m_socketMutex.unlock();

	// status reply should be a single byte.
	if (ba.size() == 1)
	{
		unsigned int status = (unsigned char)ba.at(0);
		if (status != m_lastNICStatus)
		{
			emit nicStatusChangedSignal(status, m_lastNICStatus);
			m_lastNICStatus = status;
		}

		if (f_mapNICStatusStrings.count(status) == 1)
		{
			//qInfo() << QString("NIC status (%1): %2").arg(status).arg(f_mapNICStatusStrings[status]);
			ui.labelNICStatus->setText(QString("(%1): %2").arg(status).arg(f_mapNICStatusStrings[status]));
		}
		else
		{
			qWarning() << "Unknown status value from NIC: " << status;
		}
	}
	else
	{
		qWarning() << "Expecting single byte status response from NIC, but we got : " << ba << " length " << ba.size();
		for (unsigned int i = 0; i < ba.size(); i++)
		{
			unsigned int status = (unsigned char)ba.at(i);
			if (f_mapNICStatusStrings.count(status) == 1)
			{
				//qInfo() << QString("NIC status (%1): %2").arg(status).arg(f_mapNICStatusStrings[status]);
				ui.labelNICStatus->setText(QString("(%1): %2").arg(status).arg(f_mapNICStatusStrings[status]));
			}
			else
			{
				qWarning() << "Unknown status value from NIC: " << status;
			}
		}
	}
}

void unic::readProtocolStatusResponseStateEntered()
{
	//	qInfo() << "Got status reply from NIC";

	// read status
	m_socketMutex.lock();
	QByteArray ba = m_socket.readAll();
	m_socketMutex.unlock();

	// status reply should be a single byte.
	if (ba.size() == 1)
	{
		unsigned int status = (unsigned char)ba.at(0);
#if 0
		if (status != m_lastNICStatus)
		{
			emit nicStatusChangedSignal(status, m_lastNICStatus);
			m_lastNICStatus = status;
		}
#endif
		if (f_mapNICStatusStrings.count(status) == 1)
		{
			//qInfo() << QString("NIC status (%1): %2").arg(status).arg(f_mapNICStatusStrings[status]);
			ui.labelProtocolStatus->setText(QString("(%1): %2").arg(status).arg(f_mapNICStatusStrings[status]));
		}
		else
		{
			qWarning() << "Unknown protocol status value from NIC: " << status;
		}
	}
	else
	{
		qWarning() << "Expecting single byte protocol status response from NIC, but we got : " << ba << " length " << ba.size();
		for (unsigned int i = 0; i < ba.size(); i++)
		{
			unsigned int status = (unsigned char)ba.at(i);
			if (f_mapNICStatusStrings.count(status) == 1)
			{
				//qInfo() << QString("NIC status (%1): %2").arg(status).arg(f_mapNICStatusStrings[status]);
				ui.labelProtocolStatus->setText(QString("(%1): %2").arg(status).arg(f_mapNICStatusStrings[status]));
			}
			else
			{
				qWarning() << "Unknown status value from NIC: " << status;
			}
		}
	}
}


// nic status changed
void unic::nicStatusChanged(int newStatus, int oldStatus)
{
	qDebug() << "Old status: " << f_mapNICStatusStrings[oldStatus] << " New status: " << f_mapNICStatusStrings[newStatus];
#if 0
	if (m_bStatusExpected)
	{
		if (newStatus == m_statusExpected)
		{
			qDebug() << "SUCCESS - GOT EXPECTED STATUS";
			m_bStatusExpected = false;
			emit commandStatusOK();
		}
	}

	if (m_bStimulationIsOn && newStatus == 236)
	{
		m_bStimulationIsOn = false;
		emit stimulationOn(false);
	}

	if (newStatus == 238)
	{
		m_bStimulationIsOn = true;
		emit stimulationOn(true);
	}
#endif
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

					// get lock
					m_socketMutex.lock();

					// do we expect a specific status after this command?
					if (f_mapNICExpectedResponses.count(commandNumber) == 1)
					{
						m_bStatusExpected = true;
						m_statusExpected = f_mapNICExpectedResponses[commandNumber];
						qInfo() << "Expecting status " << f_mapNICStatusStrings[m_statusExpected];
					}

					// write command number as a single byte
					qInfo() << "Sending command number: " << commandNumber;
					m_socket.write(command, 1);

					// write parameters
					qInfo() << "Sending parameter list: " << params;
					m_socket.write(params);

					// unlock mutex
					m_socketMutex.unlock();
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
		else
		{
			qWarning() << "Expecting 2 lines in file, got " << list.size();
		}
	}
	else
	{
		qCritical() << "Cannot open file " << m_file.fileName();
	}



}
 
void unic::readStatusTimeoutStateEntered()
{
	qWarning() << "Timeout waiting for NIC status reply.";
}

void unic::readProtocolStatusTimeoutStateEntered()
{
	qWarning() << "Timeout waiting for NIC protocol status reply.";
}


void unic::message(const QString& msg)
{
	ui.plainTextEditLog->appendPlainText(msg);
}

