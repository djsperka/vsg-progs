#include "unic.h"
#include <QScrollBar>

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

