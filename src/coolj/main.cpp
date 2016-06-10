#include "coolj.h"
#include <QtWidgets/QApplication>
#include "alertlib.h"
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <SFML/Network.hpp>

using namespace boost;
using namespace std;

int f_iMoveButton = 0;
bool f_verbose = false;
bool f_bUseJoystick = false;
vector< pair<sf::IpAddress, int> >f_servers;
int f_sleepMS = 100;

int process_args(int option, std::string& arg)
{
	int errflg = 0;
	std::vector<std::string> strs;
	switch (option)
	{
	case 's':
		boost::split(strs, arg, boost::is_any_of(":"));
		if (strs.size() == 2)
		{
			sf::IpAddress addr(strs[0]);
			int port = lexical_cast<int>(strs[1]);
			cout << "Got server pair " << addr << " + " << port << endl;
			f_servers.push_back(pair<sf::IpAddress, int>(addr, port));
		}
		else
		{
			cerr << "Error in server input: should be ip:port" << endl;
			errflg++;
		}
		break;
	case 'S':
		f_sleepMS = lexical_cast<int>(arg);
		break;
	case 'B':
		if (parse_integer(arg, f_iMoveButton))
			cerr << "Bad button number." << endl;
		break;
	case 'v':
		f_verbose = true;
		break;
	case 'j':
		f_bUseJoystick = true;
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

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	if (prargs(argc, argv, process_args, "s:S:B:j"))
		return -1;
	coolj w(f_servers, f_sleepMS, f_bUseJoystick, f_iMoveButton);
	w.show();
	return a.exec();
}
