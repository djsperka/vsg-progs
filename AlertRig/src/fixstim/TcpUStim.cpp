#include "TcpUStim.h"
#include "AlertUtil.h"
#include <iostream>
#include <boost/bind/bind.hpp>
#include <boost/regex.hpp>

using namespace boost::placeholders;

bool TcpUStim::parse(int argc, char** argv)
{
	std::cout << "TcpUStim::parse" << std::endl;
	for (int i = 0; i < argc; i++)
		std::cout << 1 << ": " << std::string(argv[i]) << std::endl;
	std::cout << "TcpUStim::parse done" << std::endl;
	return true;
}


void TcpUStim::run_stim(alert::ARvsg& vsg)
{
	std::cout << "TcpUStim::run_stim(): started" << std::endl;
	m_pWrapper = std::unique_ptr<AsyncTCPServerWrapper>(new AsyncTCPServerWrapper(boost::bind(&TcpUStim::callback, this, _1, _2), 7001, ';'));

	// clear page 0
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);

	// allocate a color 
	vsg.request_single(m_level);

	// blocking, but with async callbacks
	m_pWrapper->start();

	vsg.clear();
	return;
}

bool TcpUStim::callback(const std::string& s, std::ostream& out)
{
	std::cout << "callback: " << s << std::endl;
	// background expect b (127/127/127) or b [0.1/0.2/0.3]
	boost::regex expr{ "b\\s(.*)" };
	boost::smatch what;
	if (boost::regex_search(s, what, expr))
	{
		std::cout << "parse background color : " << what[1] << std::endl;
		COLOR_TYPE c;
		if (parse_color(what[1], c))
		{
			std::cerr << "Cannot parse background color: " << s << std::endl;
			out << "ERR,cannot parse color;";
		}
		else
		{
			// set color in color table
			arutil_color_to_palette(c, m_level);

			// clear page
			vsgSetDrawPage(vsgVIDEOPAGE, 0, m_level);

			out << "OK;";
		}
	}
	else if (s.find("quit") != std::string::npos)
	{
		out << "OK;";
		m_pWrapper->stop();
	}
	return true;
}

