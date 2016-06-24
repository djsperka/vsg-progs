#include "unic.h"
#include <QScrollBar>
#include <QState>

unic::unic(const QString& commandFile, QTcpSocket& socket, QWidget *parent)
: QMainWindow(parent)
, m_socket(socket)
{
	m_nicCommandFileWatcher.addPath(commandFile);
	connect(&m_nicCommandFileWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(fileChanged(const QString&)));
	ui.setupUi(this);

	ui.labelCommandFile->setText(commandFile);
	ui.plainTextEditLog->setReadOnly(true);
	ui.plainTextEditLog->verticalScrollBar()->setValue(ui.plainTextEditLog->verticalScrollBar()->maximum());
}

unic::~unic()
{

}

void unic::fileChanged(const QString& s)
{
	qDebug() << "File changed: " << s;
	ui.plainTextEditLog->appendPlainText(s);
}

void unic::buildStateMachine()
{
	m_pMachine = new QStateMachine;

	QState *sIdle = new QState(m_pMachine);
//	FileChangedState *sChanged = new FileChangedState(m_nicCommandFileWatcher.files().at(0), *m_socket);

	sIdle->addTransition(&m_nicCommandFileWatcher, SIGNAL(fileChanged(const QString&)), sChanged);

	m_pMachine->start();
}


FileChangedState::FileChangedState(const QString& filename, QTcpSocket* socket)
	: psocket(socket)
	, m_file(filename)
{
}

void FileChangedState::onEntry(QEvent *event)
{
	Q_UNUSED(event);
	if (m_file.open(QIODevice::ReadOnly))
	{
		QByteArray ba = m_file.readAll();
		m_file.close();

		// there should be two lines in file
		QList<QByteArray> list = ba.split('\n');
		if (list.size() == 2)
		{
//			psocket->write()
		}
	}
}

