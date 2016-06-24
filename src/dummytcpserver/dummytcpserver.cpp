#include "dummytcpserver.h"

dummytcpserver::dummytcpserver(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	connect(&server, SIGNAL(newConnection()), this, SLOT(newConnection()));
	server.listen(QHostAddress::Any, 9898);
	ui.plainTextEdit->appendPlainText("Listening on port 9898");
}

dummytcpserver::~dummytcpserver()
{

}


void dummytcpserver::newConnection()
{
	psocket = server.nextPendingConnection();
	ui.plainTextEdit->appendPlainText(QString("Connected to %1:%2").arg(psocket->peerAddress().toString()).arg(psocket->peerPort()));
	connect(psocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
}

void dummytcpserver::readyRead()
{
	QByteArray ba = psocket->readAll();
	ui.plainTextEdit->appendPlainText(QString(ba));
}