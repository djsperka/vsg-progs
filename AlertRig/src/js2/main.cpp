#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>
#include "alertlib.h"
#include "AlertUtil.h"

using namespace boost;
using namespace alert;
using namespace std; 

const char *f_allowedArgs = "S:";
int f_errflg = 0;

int prargs_callback(int c, string& arg);

int main(int argc, char **argv)
{
	int count = 0;
	int status = prargs(argc, argv, prargs_callback, f_allowedArgs, 'F');
	if (status)
	{
		cerr << "Error in input args." << endl;
		return -1;
	}


//    sf::RenderWindow window(sf::VideoMode(200, 200), "SFML works!");
//    sf::CircleShape shape(100.f);
//    shape.setFillColor(sf::Color::Green);
/*
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
		*/

	sf::Joystick::update();

	if (sf::Joystick::isConnected(0))
	{
		cout << "Joystick 0 connected: " << std::string(sf::Joystick::getIdentification(0).name) << endl;
		cout << "Number of buttons: " << sf::Joystick::getButtonCount(0);
		cout << "X Axis? " << sf::Joystick::hasAxis(0, sf::Joystick::Axis::X) << endl;
		cout << "Y Axis? " << sf::Joystick::hasAxis(0, sf::Joystick::Axis::Y) << endl;
		cout << "Z Axis? " << sf::Joystick::hasAxis(0, sf::Joystick::Axis::Z) << endl;
		cout << "R Axis? " << sf::Joystick::hasAxis(0, sf::Joystick::Axis::R) << endl;
		cout << "U Axis? " << sf::Joystick::hasAxis(0, sf::Joystick::Axis::U) << endl;
		cout << "V Axis? " << sf::Joystick::hasAxis(0, sf::Joystick::Axis::V) << endl;
		cout << "PovX Axis? " << sf::Joystick::hasAxis(0, sf::Joystick::Axis::PovX) << endl;
		cout << "PovY Axis? " << sf::Joystick::hasAxis(0, sf::Joystick::Axis::PovY) << endl;
	}

	while (true)
	{
		sf::Joystick::update();

		if (sf::Joystick::isConnected(0))
		{
			// what's the current position of the X and Y axes of joystick number 0?
			float x = sf::Joystick::getAxisPosition(0, sf::Joystick::X);
			float y = sf::Joystick::getAxisPosition(0, sf::Joystick::Y);
			cout << count << ":";
			for (int i = 0; i < sf::Joystick::getButtonCount(0); i++)
			{
				if (sf::Joystick::isButtonPressed(0, i))
					cout << std::hex << i;
				else
					cout << "-";
			}
			cout << " " << 
				sf::Joystick::getAxisPosition(0, sf::Joystick::X) << "," <<
				sf::Joystick::getAxisPosition(0, sf::Joystick::Y) << "," <<
				sf::Joystick::getAxisPosition(0, sf::Joystick::Z) << "," <<
				sf::Joystick::getAxisPosition(0, sf::Joystick::R) << endl;
				count++;
		}
		Sleep(500);
	}

//            if (event.type == sf::Event::Closed)
 //               window.close();
  //      }

//        window.clear();
//        window.draw(shape);
//        window.display();


//    }

    return 0;
}

int prargs_callback(int c, string& arg)
{	
	istringstream iss;
	sf::IpAddress server;
	unsigned int port;
    vector<std::string> tokens;
	switch(c)
	{
	case 'S':
		split(tokens, arg, boost::is_any_of(":"));
		iss.str(tokens[0]);
		iss >> server;
		iss.str(tokens[1]);
		iss >> port;
		cout << "Server " << server << " port " << port << endl;
		break;
	case 0:
		break;
	default:
		{
			cerr << "prargs_callback(): Unknown option - " << (char)c << endl;
			f_errflg++;
			break;
		}
	}

	return f_errflg;
}
