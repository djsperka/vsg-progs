#include "coolj.h"
#include "RegHelper.h"
#include <QMessageBox>
#include <QDoubleValidator>
#include <QIntValidator>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>

using namespace sf;
using namespace std;
using namespace alert;

ostringstream f_oss;
bool f_keyMDown = false;
sf::Vector2i f_lastMousePosition;

coolj::coolj(const std::vector< pair<sf::IpAddress, int> >& servers, int sleepMS, bool bUseJoystick, int iMoveButton, QWidget *parent)
	: QMainWindow(parent)
	, m_servers(servers)
	, m_sleepMS(sleepMS)
	, m_bUseJoystick(bUseJoystick)
	, m_iMoveButton(iMoveButton)
	, m_joystickID(-1)
	, m_ptimer(0)
	, m_verbose(false)
	, m_bMouseActive(true)
{
	ui.setupUi(this);
	setup();
	init_servers();
	init_joystick(bUseJoystick);

	// Get current stimulus from registry, send it to servers as starting point.
	bool b = GetRegConfiguration();
	cout << "GetRegConfiguration()" << b << endl;


	ARApertureGratingSpec grating;
	if (GetRegStimulus(grating)) cout << "Stimulus: " << grating << endl;
	update_ui(grating, true);
	ostringstream oss;
	oss << "{\"cmd\":\"grating\", \"value\":\"" << grating << "\"}";
	send_commands(oss.str());
}

coolj::~coolj()
{

}

void coolj::init_joystick(bool bUseJoystick)
{
	// this timer will be use with joystick or with mouse
	m_ptimer = new QTimer(this);
	m_ptimer->setInterval(m_sleepMS);

	// is there a joystick connected? 
	if (bUseJoystick)
	{
		sf::Joystick::update();
		if (sf::Joystick::isConnected(0))
		{
			cout << "Joystick 0 connected." << endl;
			m_joystickID = 0;
			sf::Joystick::Identification joyID = sf::Joystick::getIdentification(0);
			cout << "Joystick 0: " << joyID.name.toAnsiString() << ":" << joyID.vendorId << ":" << joyID.productId << endl;
			// start timer
			connect(m_ptimer, SIGNAL(timeout()), this, SLOT(joystick()));
			m_ptimer->start();
		}
	}
	else
	{
		cout << "Using keyboard/mouse for input." << endl;
		connect(m_ptimer, SIGNAL(timeout()), this, SLOT(mouse()));
		m_ptimer->start();
	}
}

void coolj::setup()
{
	connect(ui.pushButtonExit, SIGNAL(clicked()), this, SLOT(exitClicked()));
	connect(ui.spinBoxContrast, SIGNAL(valueChanged(int)), this, SLOT(contrastChanged(int)));
	connect(ui.doubleSpinBoxDiameter, SIGNAL(valueChanged(double)), this, SLOT(diameterChanged(double)));

	ui.lineEditSF->setValidator(new QDoubleValidator(0.01, 100.0, 2));
	ui.lineEditTF->setValidator(new QDoubleValidator(0.01, 100.0, 2));
	ui.lineEditOrientation->setValidator(new QIntValidator(0, 360));
	ui.lineEditX->setValidator(new QDoubleValidator(-100, 100, 2));
	ui.lineEditY->setValidator(new QDoubleValidator(-100, 100, 2));
	connect(ui.lineEditSF, SIGNAL(editingFinished()), this, SLOT(sfEditingFinished()));
	connect(ui.lineEditTF, SIGNAL(editingFinished()), this, SLOT(tfEditingFinished()));
	connect(ui.lineEditOrientation, SIGNAL(editingFinished()), this, SLOT(orientationEditingFinished()));
	connect(ui.pushButtonSaveStimulus, SIGNAL(clicked()), this, SLOT(saveStimulus()));
}

void coolj::init_servers()
{
	for (int i = 0; i < m_servers.size(); i++)
	{
		// Connect to the server
		std::cout << "Connect to server at " << m_servers.at(i).first.toString() << ":" << m_servers.at(i).second << "... ";
		if (m_sockets[i].connect(m_servers.at(i).first, m_servers.at(i).second) != sf::Socket::Done)
		{
			std::cout << "Error connecting to server." << endl;
			return;
		}
		std::cout << "Connected" << std::endl;
	}
}

void coolj::saveStimulus()
{
	std::ostringstream oss;
	oss << m_gratings[0];
	cout << "reg string=" << oss.str() << endl;
	SaveRegStimulus(oss.str());
}


void coolj::exitClicked()
{
	ostringstream oss;
	oss << "{\"cmd\":\"q\" }";
	send_commands(oss.str());
	close();
}

void coolj::joystick()
{
	sf::Joystick::update();
	sf::Vector2i position = sf::Mouse::getPosition();
	std::cout << "mouse " << position.x << "," << position.y << endl;
	if (sf::Joystick::isButtonPressed(m_joystickID, m_iMoveButton))
	{
		float x = sf::Joystick::getAxisPosition(m_joystickID, sf::Joystick::X);
		float y = sf::Joystick::getAxisPosition(m_joystickID, sf::Joystick::Y);
		ostringstream oss;
		oss << "{\"cmd\":\"xy\", \"x\":" << x << ", \"y\":" << y << "}";
		send_commands(oss.str());
	}
}


void coolj::keyboard()
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::M))
	{
		if (!f_keyMDown)
		{
			// toggle mouse motion on/off
			m_bMouseActive = !m_bMouseActive;
			std::cout << "toggle mouse to " << m_bMouseActive << endl;
			f_keyMDown = true;
		}
	}
	else
	{
		f_keyMDown = false;
	}
}


void coolj::mouse()
{
	//sf::Lock lock(f_mutex);
	keyboard();
	if (!m_bMouseActive) return;
	sf::Vector2i position = sf::Mouse::getPosition();
	if (position != f_lastMousePosition)
	{
		ostringstream oss;
		oss << "{\"cmd\":\"mouse\", \"x\":" << position.x << ", \"y\":" << position.y << "}";
		send_commands(oss.str());
	}
	f_lastMousePosition = position;
}


void coolj::contrastChanged(int contrast)
{
	//QMessageBox::information(this, QString("info"), QString("contrast is %1").arg(contrast));
	ostringstream oss;
	oss << "{\"cmd\":\"contrast\", \"value\":" << contrast << "}";
	send_commands(oss.str());
}

void coolj::tfEditingFinished()
{
	double tf = ui.lineEditTF->text().toDouble();
	ostringstream oss;
	oss << "{\"cmd\":\"tf\", \"value\":" << tf << "}";
	send_commands(oss.str());
}

void coolj::sfEditingFinished()
{
	double sf = ui.lineEditSF->text().toDouble();
	ostringstream oss;
	oss << "{\"cmd\":\"sf\", \"value\":" << sf << "}";
	send_commands(oss.str());
}

void coolj::orientationEditingFinished()
{
	int ori = ui.lineEditOrientation->text().toInt();
	ostringstream oss;
	oss << "{\"cmd\":\"ori\", \"value\":" << ori << "}";
	send_commands(oss.str());
}

void coolj::diameterChanged(double diameter)
{
	ostringstream oss;
	oss << "{\"cmd\":\"a\", \"value\":" << diameter << "}";
	send_commands(oss.str());
}

void coolj::send_commands(const std::string& s)
{	
	sf::Packet packet;
	char buffer[128];
	size_t received;
	for (int i = 0; i < m_servers.size(); i++)
	{
		m_sockets[i].send(s.c_str(), s.size());
		sf::Socket::Status status = m_sockets[i].receive(buffer, 128, received);
		if (status == sf::Socket::Done)
		{
			//cout << string(buffer, received) << endl;
			if (parse_grating(string(buffer, received), m_gratings[i]))
			{
				cout << "Error in received grating spec: " << string(buffer, received) << endl;
			}
		}
		else
			cout << "send_commands - reply status " << status << endl;
	}
	update_ui(m_gratings[0], true);
}


void coolj::update_ui(const alert::ARGratingSpec& g, bool bEverything)
{
	ui.lineEditX->setText(QString("%1").arg(g.x));
	ui.lineEditY->setText(QString("%1").arg(g.y));
	if (bEverything)
	{
		ui.lineEditSF->setText(QString("%1").arg(g.sf));
		ui.lineEditTF->setText(QString("%1").arg(g.tf));
		ui.lineEditOrientation->setText(QString("%1").arg(g.orientation));
		ui.spinBoxContrast->setValue(g.contrast);
		ui.doubleSpinBoxDiameter->setValue(g.w);
	}
}
