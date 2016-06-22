#ifndef UNIC_H
#define UNIC_H

#include <QtWidgets/QMainWindow>
#include <QTcpSocket>
#include <QFileSystemWatcher>
#include "ui_unic.h"

class unic : public QMainWindow
{
	Q_OBJECT

public:
	unic(const QString& commandFile, QTcpSocket& socket, QWidget *parent = 0);
	~unic();

private:
	Ui::unicClass ui;
	QTcpSocket& m_socket;
	QFileSystemWatcher m_nicCommandFileWatcher;

protected slots:
	void fileChanged(const QString&);
};

#endif // UNIC_H
