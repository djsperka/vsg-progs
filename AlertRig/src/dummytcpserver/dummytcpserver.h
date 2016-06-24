#ifndef DUMMYTCPSERVER_H
#define DUMMYTCPSERVER_H

#include <QtWidgets/QMainWindow>
#include "ui_dummytcpserver.h"
#include <QTcpServer>
#include <QTcpSocket>

class dummytcpserver : public QMainWindow
{
	Q_OBJECT

public:
	dummytcpserver(QWidget *parent = 0);
	~dummytcpserver();

	protected slots:
	void newConnection();
	void readyRead();

private:
	Ui::dummytcpserverClass ui;
	QTcpServer server;
	QTcpSocket* psocket;
};

#endif // DUMMYTCPSERVER_H
