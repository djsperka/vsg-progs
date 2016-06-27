#include "unic.h"
#include <QtWidgets/QApplication>
#include "alertlib.h"
#include <QHostAddress>
#include <QFileSystemWatcher>
#include <QFile>
#include <QStringList>
#include <QTcpSocket>
#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>

using namespace boost;
using namespace std;


QHostAddress f_nicHostAddress("127.0.0.1");
quint16 f_nicHostPort = 1235;
QString f_commandFile;
QTcpSocket f_nicSocket;
unic *f_pUnic;
bool f_bVerbose = false;

int process_args(int option, std::string& arg);
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	QString s;
	QByteArray localMsg = msg.toLocal8Bit();
	switch (type) {
	case QtDebugMsg:
		if (f_bVerbose) f_pUnic->message(QString("Debug: %1").arg(msg));
		break;
	case QtInfoMsg:
		f_pUnic->message(QString("Info: %1").arg(msg));
		break;
	case QtWarningMsg:
		f_pUnic->message(QString("Warning: %1").arg(msg));
		break;
	case QtCriticalMsg:
		f_pUnic->message(QString("Critical: %1").arg(msg));
		break;
	case QtFatalMsg:
		f_pUnic->message(QString("Fatal: %1").arg(msg));
		abort();
	}
}


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	if (prargs(argc, argv, process_args, "s:f:v"))
		return -1;

	// open socket
	f_nicSocket.connectToHost(f_nicHostAddress, f_nicHostPort);
	if (!f_nicSocket.waitForConnected(10))
	{
		qCritical() << "Cannot connect to nic host at " << f_nicHostAddress << ":" << f_nicHostPort;
		return -1;
	}

	// instantiate dialog
	f_pUnic = new unic(f_commandFile, f_nicSocket);
	qInstallMessageHandler(messageHandler);
	f_pUnic->resize(600, 480);
	f_pUnic->show();
	return a.exec();
}


int process_args(int option, std::string& arg)
{
	int errflg = 0;
	vector<string> strs;
	switch (option)
	{
	case 'v':
		f_bVerbose = true;
		break;
	case 's':
		boost::split(strs, arg, boost::is_any_of(":"));
		if (strs.size() == 2)
		{
			f_nicHostAddress.setAddress(QString(strs[0].c_str()));
			f_nicHostPort = lexical_cast<uint16_t>(strs[1]);
		}
		else
		{
			cerr << "Error in server input: should be ip:port" << endl;
			errflg++;
		}
		break;
	case 'f':
	{
		QFile f(arg.c_str());
		if (!f.open(QIODevice::ReadWrite))
		{
			qCritical() << "Cannot open/create file " << arg.c_str();
			errflg++;
		}
		else
		{
			f.close();
			f_commandFile = QString(arg.c_str());
		}
		break;
	}
	case 0:
		break;
	default:
		cerr << "Unknown option: " << (char)option << endl;
		errflg++;
		break;
	}
	return errflg;
}
