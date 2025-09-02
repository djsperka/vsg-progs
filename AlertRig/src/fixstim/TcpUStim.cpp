#include "TcpUStim.h"
#include "AlertUtil.h"
#include <iostream>
#include <boost/bind/bind.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
//using namespace boost;
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

	// screen distance and bkgd color
	//VSGTRIVAL c = m_arguments.bkgdColor.trival();
	vsg.setViewDistMM(800);
	vsg.setBackgroundColor(gray);


	// clear page 0
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	m_ipage = 0;

	// allocate a color 
	vsg.request_single(m_level);

	// init fixpt 
	m_fixpt.init(2);

	// blocking, but with async callbacks
	m_pWrapper->start();

	vsg.clear();
	return;
}

bool TcpUStim::callback(const std::string& s, std::ostream& out)
{
	std::cout << "callback: " << s << std::endl;
	// background expect b (127/127/127) or b [0.1/0.2/0.3]
	boost::regex b_expr{ "b\\s(.*)" };
	boost::regex f_expr{ "f\\s(.*)" };
	boost::smatch what;

	// page flipping
	m_ipage = 1 - m_ipage;

	if (boost::regex_search(s, what, b_expr))
	{
		std::cout << "matched background color cmd." << std::endl;
		COLOR_TYPE c;
		if (parse_color(what[1], c))
		{
			std::cerr << "Cannot parse background color: " << s << std::endl;
			out << "ERR,cannot parse color;";
		}
		else
		{
			std::cout << "color " << c << " to level " << m_level << std::endl;

			// set color in color table
			arutil_color_to_palette(c, m_level);

			// clear page
			vsgSetDrawPage(vsgVIDEOPAGE, m_ipage, m_level);
			vsgObjSetTriggers(vsgTRIG_ONPRESENT, 0, 0);
			vsgPresent();

			out << "OK;";
		}
	}
	else if (boost::regex_search(s, what, f_expr))
	{
		std::cout << "matched fixpt command." << std::endl;
		if (parse_fixation_point(what[1], m_fixpt))
		{
			std::cerr << "Cannot parse fixation point: " << s << std::endl;
			out << "ERR,cannot parse fixpt;";
		}
		else
		{
			std::cout << "Draw fixpt " << m_fixpt << " on page " << m_ipage << std::endl;

			// clear page
			vsgSetDrawPage(vsgVIDEOPAGE, m_ipage, m_level);

			// draw fixpt
			m_fixpt.draw();
			vsgObjSetTriggers(vsgTRIG_ONPRESENT, 0, 0);
			vsgPresent();

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

