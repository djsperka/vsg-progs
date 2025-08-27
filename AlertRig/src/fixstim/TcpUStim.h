#pragma once
#include "UStim.h"
#include "AsyncTCPServerWrapper.h"
#include <memory>

struct tcpustim_arguments
{
	std::string addr;
	int port;
};

class TcpUStim : public UStim
{
public:
	TcpUStim() {};
	virtual ~TcpUStim() {};

	bool parse(int argc, char** argv);
	void run_stim(alert::ARvsg& vsg);

	bool TcpUStim::callback(const std::string& s, std::ostream& out);

	//int process_arg(int option, std::string& arg);

	// This acts as the callback function for the triggers
	//int callback(int& output, const FunctorCallbackTrigger* ptrig);

private:
	struct tcpustim_arguments m_arguments;
	std::unique_ptr<AsyncTCPServerWrapper> m_pWrapper;
	PIXEL_LEVEL m_level;	
	PIXEL_LEVEL m_level_fixpt;
	int m_ipage;
	alert::ARContrastFixationPointSpec m_fixpt;
	//void init_triggers(TSpecificFunctor<TcpUStim>* pfunctor);
	//bool parseImageArg(const std::string& arg, std::string& filename, double& x, double& y, double& duration, double& delay, int& nlevels);
};
