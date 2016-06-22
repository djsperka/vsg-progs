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

int process_args(int option, std::string& arg);

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	if (prargs(argc, argv, process_args, "s:f:"))
		return -1;

	// open socket
#if 0
	f_nicSocket.connectToHost(f_nicHostAddress, f_nicHostPort);
	if (!f_nicSocket.waitForConnected(10))
	{
		qCritical() << "Cannot connect to nic host at " << f_nicHostAddress << ":" << f_nicHostPort;
		return -1;
	}
#endif

	// instantiate dialog
	unic w(f_commandFile, f_nicSocket);

	w.show();
	return a.exec();
}


int process_args(int option, std::string& arg)
{
	int errflg = 0;
	vector<string> strs;
	switch (option)
	{
	case 's':
		boost::split(strs, arg, boost::is_any_of(":"));
		if (strs.size() == 2)
		{
			f_nicHostAddress.setAddress(QString(strs[0].c_str()));
			f_nicHostPort = lexical_cast<uint16_t>(strs[1]);
			cout << "Got server pair " << strs[0] << " + " << strs[1] << endl;
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
