#ifndef UNIC_H
#define UNIC_H

#include <QtWidgets/QMainWindow>
#include <QTcpSocket>
#include <QFileSystemWatcher>
#include <QStateMachine>
#include <QTimer>
#include <QFile>
#include <QMutex>
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
	QTimer m_timerReadStatusResponse;
	QTimer m_timerSendStatusCommand;
	QFile m_file;
	QMutex m_socketMutex;	// for access to m_socket.

	void buildStateMachine();

protected slots:
	void fileChangedStateEntered();
	void idleStatusStateEntered();
	void sendStatusCommandStateEntered();
	void readStatusResponseStateEntered();
	void readStatusTimeoutStateEntered();
	
};


#endif // UNIC_H
