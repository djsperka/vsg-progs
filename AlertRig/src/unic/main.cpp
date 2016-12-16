#include "unic.h"
#include <QtWidgets/QApplication>
#include "alertlib.h"
#include "ftd2xx.h"
#include <QHostAddress>
#include <QFileSystemWatcher>
#include <QFile>
#include <QStringList>
#include <QTcpSocket>
#include <QLockFile>
#include <QDir>
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
bool f_bUseFTDIChip = true;		// -z disables use of FTDI chip
FT_HANDLE f_ftdiHandle;
char f_cStimCurrentStatus = 0;

// bits for ftdi output
#define FTDI_BIT_COMMAND_RECEIVED 0x1
#define FTDI_BIT_STATUS_OK 0x16
#define FTDI_BIT_STIMULATING 0x4


int process_args(int option, std::string& arg);
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

// These are slots
void gotFileChange();
void gotStatusOK();
void stimulationIsOn(bool);

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

	// Check if application is already running...
//	QLockFile lockFile(QDir::temp().absoluteFilePath("unic.lock"));
//	lockFile.setStaleLockTime(0);
//	if (!lockFile.tryLock(100)) 
//	{
//		return 1;
//	}

	if (prargs(argc, argv, process_args, "s:f:vz"))
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
	f_pUnic->show();

	// initialize ftdi chip 
	if (f_bUseFTDIChip)
	{
		/* Initialize, open device, set bitbang mode */
		if (FT_OK != FT_Open(0, &f_ftdiHandle)) {
			qCritical() << "Cannot open FTDI USB-Serial device";
			return 1;
		}
		if (FT_OK != FT_SetBitMode(f_ftdiHandle, FTDI_BIT_COMMAND_RECEIVED | FTDI_BIT_STATUS_OK | FTDI_BIT_STIMULATING, 1))
		{
			qCritical() << "Cannot set asynch bit-bang mode!";
			FT_Close(f_ftdiHandle);
			return 1;
		}
		FT_SetBaudRate(f_ftdiHandle, 600);

		// connections for setting output bits
		QObject::connect(f_pUnic, &unic::fileChangeDetected, gotFileChange);
		QObject::connect(f_pUnic, &unic::commandStatusOK, gotStatusOK);
		QObject::connect(f_pUnic, &unic::stimulationOn, stimulationIsOn);

	}


	// start event loop
	a.exec();

	// set outputs low, close ftdi chip
	if (f_bUseFTDIChip)
	{
		char buffer[2];
		DWORD written = 0;
		buffer[0] = 0;
		buffer[1] = 0;
		FT_Write(f_ftdiHandle, buffer, 2, &written);
		FT_Close(f_ftdiHandle);
	}

	return 0;
}

void gotFileChange()
{
	char buffer[2];
	DWORD written = 0;
	buffer[0] = FTDI_BIT_COMMAND_RECEIVED | f_cStimCurrentStatus;
	buffer[1] = f_cStimCurrentStatus;

	qDebug() << "gotFileChange()";
	if (FT_OK != FT_Write(f_ftdiHandle, buffer, 2, &written))
	{
		qCritical() << "Error writing to ftdi device.";
	}
	return;
}

void gotStatusOK()
{
	char buffer[2];
	DWORD written = 0;
	buffer[0] = FTDI_BIT_STATUS_OK | f_cStimCurrentStatus;
	buffer[1] = f_cStimCurrentStatus;

	qDebug() << "gotStatusOK()";
	if (FT_OK != FT_Write(f_ftdiHandle, buffer, 2, &written))
	{
		qCritical() << "Error writing to ftdi device.";
	}
	return;
}

void stimulationIsOn(bool on)
{
	char buffer[1];
	DWORD written = 0;
	if (on) f_cStimCurrentStatus = FTDI_BIT_STIMULATING;
	else f_cStimCurrentStatus = 0;
	buffer[0] = f_cStimCurrentStatus;
	qDebug() << "stimulationIsOn(" << on << ")";
	if (FT_OK != FT_Write(f_ftdiHandle, buffer, 1, &written))
	{
		qCritical() << "Error writing to ftdi device.";
	}
	return;
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
	case 'z':
		f_bUseFTDIChip = false;
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
