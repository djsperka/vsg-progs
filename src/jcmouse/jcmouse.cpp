// jcmouse.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "conio.h"

#include "alertlib.h"
#include "RegHelper.h"
#include <iostream>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
//#include <SFML/Network.hpp>
#include <QtNetwork/QTcpSocket>

using namespace boost;
using namespace std;
using namespace alert;

bool f_verbose = false;
vector< pair<string, int> >f_servers;
int f_sleepMS = 100;
QTcpSocket f_sockets[2];
//sf::TcpSocket f_sockets[2];	// No copy constructor for TcpSocket - cannot put in vector?!?
ARApertureGratingSpec f_gratings[2];
double f_tfPrevious = 0;

int process_args(int option, string& arg);
void init_servers();
void do_loop();
void sendCommands(const string& s);
void diameterChanged(double diameter);
void orientationChanged(double ori);
void sfChanged(double sf);
void tfChanged(double tf);
void contrastChanged(int contrast);
void xyChanged(int x, int y);
void sendQuit();

int main(int argc, char **argv)
{
	if (prargs(argc, argv, process_args, "s:l:"))
		return -1;

	init_servers();

	do_loop();

    return 0;
}

void do_loop()
{
	bool bUseManualTriggers = false;
	bool bSendTrigger = false;
	bool bMouseOn = true;
	bool bQuit = false;
	int key;
	int iStepSize = 10;
	POINT mousePos;
	POINT lastMousePos;
	int iPage = 1;
	long lDigitalIO = 0;
	long lDigitalIOLast = 0;
	long last_output_trigger = 0;

	// loop forever until 'q' is hit on keyboard or a quit signal is received on digition IO lines
	while (!bQuit)
	{
		Sleep(f_sleepMS);

		// get cursor position if mouse is on.
		// Convert to degrees outside the if(bMouseOn) because arrow keys may have moved position.
		if (bMouseOn)
		{
			GetCursorPos(&mousePos);
			if (mousePos.x != lastMousePos.x || mousePos.y != lastMousePos.y)
			{
				// Note: pixels are y-positive-down!
				xyChanged(mousePos.x, mousePos.y);
				lastMousePos = mousePos;
			}
		}

		if (_kbhit())
		{
			key = _getch();
			switch (key)
			{
			case 'a':
			{
				// get aperture size. It will be updated next time through.
				float a = -1;
				cout << "Aperture Diameter (degrees) = ";
				cin >> a;
				if (!cin.fail())
				{
					if (a > 0)
					{
						diameterChanged(a);
					}
					else
					{
						cerr << "Error in input: Aperture diam must be a number>0." << endl;
					}
				}
				else
				{
					cerr << "Bad format - must be a number." << endl;
					cin.clear(); cin.ignore(INT_MAX, '\n');
				}
				break;
			}
			case 's':
			{
				double s;
				cout << "Spatial frequency [cycles/degree]= ";
				cin >> s;
				if (!cin.fail())
				{
					if (s>0.005 && s<100)
					{
						sfChanged(s);
					}
					else
					{
						cout << "Error in input: spatial freq must be a number between 0.005 and 100." << endl;
					}
				}
				else
				{
					cerr << "Bad format - must be a number." << endl;
					cin.clear(); cin.ignore(INT_MAX, '\n');
				}
				break;
			}
			case 't':
			{
				double tf = -1;
				cout << "Temporal Frequency = ";
				cin >> tf;
				if (!cin.fail())
				{
					if (tf >= 0 && tf < 50)
					{
						tfChanged(tf);
						f_tfPrevious = 0;
					}
					else
					{
						cout << "Error in input: Temporal freq must be between 0 and 50." << endl;
					}
				}
				else
				{
					cerr << "Bad format - must be a number." << endl;
					cin.clear(); cin.ignore(INT_MAX, '\n');
				}
				break;
			}
			case ' ':
			{
				double tmp = f_gratings[0].tf;
				tfChanged(f_tfPrevious);
				f_tfPrevious = tmp;
				break;
			}
			case 'c':
			{
				int c;
				cout << "Contrast = ";
				cin >> c;
				if (!cin.fail())
				{
					if (c >= 0 && c <= 100)
					{
						contrastChanged(c);
					}
					else
					{
						cout << "Error in input: Contrast must be between 0 and 100." << endl;
					}
				}
				else
				{
					cerr << "Bad format - must be an integer." << endl;
					cin.clear(); cin.ignore(INT_MAX, '\n');
				}
				break;
			}
			case 'o':
			{
				double oo = 999;
				cout << "Orientation = ";
				cin >> oo;
				if (!cin.fail())
				{
					if (oo >= 0 && oo <= 360)
					{
						orientationChanged(oo);
					}
					else
					{
						cout << "Error in input: Orientation must be a number between 0 and 360" << endl;
					}
				}
				else
				{
					cerr << "Bad format - must be a number." << endl;
					cin.clear(); cin.ignore(INT_MAX, '\n');
				}
				break;
			}
			case 'p':
			case 'v':
			{
				cout << "X, Y (degrees)            = " << f_gratings[0].x << ", " << f_gratings[0].y << endl;
				cout << "Diameter (deg)            = " << f_gratings[0].w << endl;
				cout << "Orientation(deg)          = " << f_gratings[0].orientation << endl;
				cout << "Spatial Freq(cycles/deg)  = " << f_gratings[0].sf << endl;
				cout << "Temporal Freq(cycles/sec) = " << f_gratings[0].tf << endl;
				cout << "Contrast(%)               = " << f_gratings[0].contrast << endl;
				break;
			}
			case 27:
			case 'q':
			case 'Q':
			{
				bQuit = true;
				sendQuit();
				break;
			}
			case 'm':
			{
				if (bMouseOn)
				{
					cout << "Mouse movement OFF" << endl;
					bMouseOn = false;
				}
				else
				{
					cout << "Mouse movement ON" << endl;
					bMouseOn = true;
				}
				break;
			}
			case 'S':
			{
				int i;
				cout << "Step size (pixels) = ";
				cin >> i;
				if (!cin.fail())
				{
					if (i > 0)
					{
						iStepSize = i;
					}
					else
					{
						cout << "Error in input: step size must be positive" << endl;
					}
				}
				else
				{
					cerr << "Bad format - must be an integer." << endl;
					cin.clear(); cin.ignore(INT_MAX, '\n');
				}
				break;
			}
			case '8':
			{
				if (!bMouseOn)
				{
					mousePos = lastMousePos;
					mousePos.y -= iStepSize;
					xyChanged(mousePos.x, mousePos.y);
					lastMousePos = mousePos;
				}
				else
				{
					cout << "Mouse movement is ON - turn off with 'f'" << endl;
				}
				break;
			}
			case '2':
			{
				if (!bMouseOn)
				{
					mousePos = lastMousePos;
					mousePos.y += iStepSize;
					xyChanged(mousePos.x, mousePos.y);
					lastMousePos = mousePos;
				}
				else
				{
					cout << "Mouse movement is ON - turn off with 'f'" << endl;
				}
				break;
			}
			case '4':
			{
				if (!bMouseOn)
				{
					mousePos = lastMousePos;
					mousePos.x -= iStepSize;
					xyChanged(mousePos.x, mousePos.y);
					lastMousePos = mousePos;
				}
				else
				{
					cout << "Mouse movement is ON - turn off with 'f'" << endl;
				}
				break;
			}
			case '6':
			{
				if (!bMouseOn)
				{
					mousePos = lastMousePos;
					mousePos.x += iStepSize;
					xyChanged(mousePos.x, mousePos.y);
					lastMousePos = mousePos;
				}
				else
				{
					cout << "Mouse movement is ON - turn off with 'f'" << endl;
				}
				break;
			}
			case 13:
			{
				std::ostringstream oss;
				/*
				double xtemp, ytemp;
				xtemp = f_gratings[0].x;
				ytemp = f_gratings[0].y;
				f_gratings[0].x = degVSGMouseX;
				f_gratings[0].y = -degVSGMouseY;
				*/
				oss << f_gratings[0];
				/*
				f_gratings[0].x = xtemp;
				f_gratings[0].y = ytemp;
				*/
				cout << "reg string=" << oss.str() << endl;

				// Save to registry
				SaveRegStimulus(oss.str());
				break;
			}
			case 'h':
			default:
			{
				cout << "~~~~~~~~~HELP MENU~~~~~~~~~" << endl << endl;
				cout << "Grating Parameter controls:" << endl;
				cout << "---------------------------" << endl;
				cout << "<a> aperture size" << endl;
				cout << "<o> orientation" << endl;
				cout << "<s> spatial frequency" << endl;
				cout << "<t> temporal frequency" << endl;
				cout << "<c> contrast [0-100]" << endl << endl;
				cout << "<space bar> toggles between drifting and static" << endl;
				cout << "Background luminance control:" << endl;
				cout << "-----------------------------" << endl;
				cout << "<b> background luminance [0-1]" << endl << endl;
				cout << "Mouse controls:" << endl;
				cout << "---------------" << endl;
				cout << "<m> toggle mouse movement on/off" << endl;
				cout << "<8>,<6>,<2>,<4> move aperture with arrows (when mouse movenment off)" << endl;
				cout << "<S> step size in pixels" << endl << endl;
				cout << "Status information:" << endl;
				cout << "---------------" << endl;
				cout << "<p> position information" << endl;
				cout << "<v> current spatiotemporal values" << endl;
				cout << "<M> use manual triggers for fixation point (<1>/<0>)" << endl;
				cout << "<Enter> save current stim parameters in registry" << endl;
				cout << "<esc>,<q> end program" << endl;
				cout << endl << endl;
				break;
			}
			}
		}
	}
}


// Send x,y pixel location. 
void xyChanged(int x, int y)
{
	ostringstream oss;
	oss << "{\"cmd\":\"mouse\", \"x\":" << x << ", \"y\":" << y << "}";
	sendCommands(oss.str());
}


void contrastChanged(int contrast)
{
	ostringstream oss;
	oss << "{\"cmd\":\"contrast\", \"value\":" << contrast << "}";
	sendCommands(oss.str());
}

void tfChanged(double tf)
{
	ostringstream oss;
	oss << "{\"cmd\":\"tf\", \"value\":" << tf << "}";
	sendCommands(oss.str());
}

void sfChanged(double sf)
{
	ostringstream oss;
	oss << "{\"cmd\":\"sf\", \"value\":" << sf << "}";
	sendCommands(oss.str());
}

void orientationChanged(double ori)
{
	ostringstream oss;
	oss << "{\"cmd\":\"ori\", \"value\":" << ori << "}";
	sendCommands(oss.str());
}

void diameterChanged(double diameter)
{
	ostringstream oss;
	oss << "{\"cmd\":\"a\", \"value\":" << diameter << "}";
	sendCommands(oss.str());
}

void sendQuit()
{
	ostringstream oss;
	oss << "{\"cmd\":\"q\" }";
	sendCommands(oss.str());
}

void sendCommands(const string& s)
{
	char buffer[128];
	qint64 received;
	for (unsigned int i = 0; i < f_servers.size(); i++)
	{
		cout << "TODO: sendCommands " << s << endl;
		if (f_sockets[i].isValid())
		{
			f_sockets[i].write(s.c_str(), s.size());
			// wait for response
			if (f_sockets[i].waitForReadyRead())
			{
				received = f_sockets[i].read(buffer, 128);
				if (received > 0)
				{
					if (parse_grating(string(buffer, received), f_gratings[i]))
					{
						cout << "Error in received grating spec: " << string(buffer, received) << endl;
					}
				}
				else
					cout << "send_commands - reply status " << received << endl;
			}
		}
		else
			cout << "jcmouse: socket closed " << f_servers.at(i).first << ":" << f_servers.at(i).second << endl;
	}
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
			int port = lexical_cast<int>(strs[1]);
			cout << "Got server pair " << strs[0] << " + " << port << endl;
			f_servers.push_back(pair<string, int>(strs[0], port));
		}
		else
		{
			cerr << "Error in server input: should be ip:port" << endl;
			errflg++;
		}
		break;
	case 'l':
		f_sleepMS = lexical_cast<int>(arg);
		break;
	case 'v':
		f_verbose = true;
		break;
	case 0:
		break;
	default:
		cerr << "Unknown option: " << (char)option << endl;
		errflg++;
		break;
	}
	return errflg;
}

void init_servers()
{
	for (unsigned int i = 0; i < f_servers.size(); i++)
	{
		// Connect to the server
		cout << "Connect to server at " << f_servers.at(i).first << ":" << f_servers.at(i).second << "... ";
		f_sockets[i].connectToHost(QString(f_servers.at(i).first.c_str()), f_servers.at(i).second);
		if (!f_sockets[i].waitForConnected())
		{
			std::cout << "Error connecting to server." << endl;
			return;
		}
		std::cout << "Connected" << std::endl;
	}
}