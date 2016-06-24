#ifndef UNIC_H
#define UNIC_H

#include <QtWidgets/QMainWindow>
#include <QTcpSocket>
#include <QFileSystemWatcher>
#include <QStateMachine>
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
	QStateMachine *m_pMachine;
	QFileSystemWatcher m_nicCommandFileWatcher;

	void buildStateMachine();

protected slots:
	void fileChanged(const QString&);
};

class FileChangedState : public QState
{
	QTcpSocket *psocket;
	QFile m_file;
public:
	FileChangedState(const QString& filename, QTcpSocket* socket);
protected:
	void onEntry(QEvent *event);
};

class StatusCommandState : public QState
{
public:
	StatusCommandState(const QString& filename, QTcpSocket *socket);
protected:
	void onEntry(QEvent *event);
};

#endif // UNIC_H
