#ifndef TCPDUMBSERV_H
#define TCPDUMBSERV_H

#include <QtWidgets/QMainWindow>
#include "ui_tcpdumbserv.h"
#include <QTcpServer>
#include <QTcpSocket>

class tcpdumbserv : public QMainWindow
{
	Q_OBJECT

public:
	tcpdumbserv(QWidget *parent = 0);
	~tcpdumbserv();

protected slots:
	void newConnection();
	void readyRead();

private:
	Ui::tcpdumbserv ui;
	QTcpServer server;
	QTcpSocket* psocket;
};

#endif // TCPDUMBSERV_H
