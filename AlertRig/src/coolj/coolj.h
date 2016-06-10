#ifndef COOLJ_H
#define COOLJ_H

#include "Alertlib.h"
#include <QtWidgets/QMainWindow>
#include <QTimer>
#include "ui_coolj.h"
#include <SFML/Network.hpp>
#include <vector>
#include <algorithm>

class coolj : public QMainWindow
{
	Q_OBJECT

public:
	coolj(const std::vector< std::pair<sf::IpAddress, int> >& servers, int sleepMS, bool bUseJoystick, int iMoveButton=0, QWidget *parent = 0);
	~coolj();
	void setVerbose(bool verbose) { m_verbose = verbose; };
private:
	Ui::cooljClass ui;
	std::vector< std::pair<sf::IpAddress, int> >m_servers;
	sf::TcpSocket m_sockets[2];	// No copy constructor for TcpSocket - cannot put in vector?!?
	alert::ARApertureGratingSpec m_gratings[2];
	int m_sleepMS;
	bool m_bUseJoystick;
	int m_iMoveButton;
	unsigned int m_joystickID;
	QTimer *m_ptimer;
	bool m_bMouseActive;
	bool m_verbose;
	void setup();
	void init_servers();
	void send_commands(const std::string& s);
	void init_joystick(bool bUseJoystick);
	void update_ui(const alert::ARGratingSpec& g, bool bEverything=false);	// if bEverything is false, just update xy.

public slots:
	void exitClicked();
	void contrastChanged(int contrast);
	void tfEditingFinished();
	void sfEditingFinished();
	void orientationEditingFinished();
	void diameterChanged(double diameter);
	void joystick();
	void mouse();
	void keyboard();
	void saveStimulus();
};

#endif // COOLJ_H
