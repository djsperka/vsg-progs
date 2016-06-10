#ifndef UNIC_H
#define UNIC_H

#include <QtWidgets/QMainWindow>
#include <QTcpSocket>
#include "ui_unic.h"

class unic : public QMainWindow
{
	Q_OBJECT

public:
	unic(QTcpSocket& socket, QWidget *parent = 0);
	~unic();

private:
	Ui::unicClass ui;
	QTcpSocket& m_socket;
};

#endif // UNIC_H
