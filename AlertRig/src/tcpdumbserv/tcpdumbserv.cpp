#include "tcpdumbserv.h"

tcpdumbserv::tcpdumbserv(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	setCentralWidget(ui.plainTextEdit);
	connect(&server, SIGNAL(newConnection()), this, SLOT(newConnection()));
	server.listen(QHostAddress::Any, 9898);
	ui.plainTextEdit->appendPlainText("Listening on port 9898");
}

tcpdumbserv::~tcpdumbserv()
{

}


void tcpdumbserv::newConnection()
{
	psocket = server.nextPendingConnection();
	ui.plainTextEdit->appendPlainText(QString("Connected to %1:%2").arg(psocket->peerAddress().toString()).arg(psocket->peerPort()));
	connect(psocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
}

void tcpdumbserv::readyRead()
{
	QByteArray ba = psocket->readAll();
	ui.plainTextEdit->appendPlainText(QString(ba));
}