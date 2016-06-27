#ifndef UNIC_H
#define UNIC_H

#include <QtWidgets/QMainWindow>
#include <QTcpSocket>
#include <QFileSystemWatcher>
#include <QStateMachine>
#include <QTimer>
#include <QFile>
#include "ui_unic.h"

class unic : public QMainWindow
{
	Q_OBJECT

public:
	unic(const QString& commandFile, QTcpSocket& socket, QWidget *parent = 0);
	~unic();
	void message(const QString& msg);

private:
	Ui::unicClass ui;
	QTcpSocket& m_socket;
	QStateMachine *m_pMachine;
	QFileSystemWatcher m_nicCommandFileWatcher;
	QTimer m_timer;
	QFile m_file;

	void buildStateMachine();

protected slots:
	void fileChangedStateEntered();
	void timeoutStateEntered();
	void statusStateEntered();
};


#endif // UNIC_H
