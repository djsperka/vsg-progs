#include "stdafx.h"
#include "CalibrationUStim.h"
#include "ASLSerial.h"

const string CalibrationUStim::m_allowedArgs("b:c:d:f:i:p:vC:");
static int f_errflg = 0;


CalibrationUStim::CalibrationUStim()
: UStim()
, m_verbose(false)
, m_pulse(0x4)
, m_screenDistanceMM(-1)
, m_lComPort(1)
, m_dCalibrationOffset(5)
{};


void CalibrationUStim::run_stim(alert::ARvsg& vsg)
{
	cout << "CalibrationUStim: running." << endl;

	// set screen distance
	vsg.setViewDistMM(m_screenDistanceMM);

	// clear all dig outputs
	vsgIOWriteDigitalOut(0, 0xff);
	vsgPresent();

	// initialize triggers
	TSpecificFunctor<CalibrationUStim> functor(this, &CalibrationUStim::callback);
	init_triggers(&functor);
	if (m_verbose)
	{
		for (unsigned int i=0; i<triggers().size(); i++)
		{
			cout << "Trigger " << i << " " << *(triggers().at(i)) << std::endl;
		}
	}

	// COM initialzation
	CoInitialize(NULL);

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		return;
	}

	// connect to asl box via serial port
	if (aslserial_connect(m_szConfigFile, m_lComPort))
	{
		cerr << "Failed to connect to ASL serial port." << endl;
		return;
	}

	// Initialize VSG card or cards

	if (init_calibration())
	{
		cerr << "VSG initialization failed." << endl;
		return;
	}





	// Loop forever. Break out of loop when calibration quit signal received.
	// djs 6-21-2010 Modifications to this loop for rivalry. 
	// When running in rivalry mode (-r on command line) we are using two VSGs. 
	// The alertlib uses dual singletons (doesn't sound right, but that's what 
	// it is) ARvsg::master() and ARvsg::slave(). We have to make sure VSG commands
	// are aimed at the correct VSG card using vsgInitSelectDevice(). Since its rare
	// that the slave is used (we'll only use it for displaying dot numbers 10-14)
	// I'm going to leave the card set for the master all the time and only switch it
	// when one of those dot numbers is received. Below I send the ready signal and 
	// set that up. 

	if (m_pulse) ARvsg::instance().ready_pulse(100, m_pulse);


	// Trigger loop.
	// We rely on the behavior of the eye tracker serial output with respect to 
	// calibration. When running ASL's "Custom Calibration", the dot number appears 
	// in the serial output as a value from 1-9. When the calibration is finished or 
	// otherwise quit, the dot number changes back to 0. That becomes our signal to 
	// quit. 

	bool bquit = false;
	bool bstarted = false;
	bool bwaiting = false;
	int idot;
	int iserial;
	int lastidot = -1;
	int input_trigger;
	int last_output_trigger = 0;

	while (!bquit)
	{
		iserial = aslserial_getDotNumber(&idot);
		if (iserial < 0)
		{
			cerr << "Error in asiserial_getDotNumber!" << endl;
			bquit = true;
		}
		else if (iserial>0)
		{
			if (!bwaiting)
			{
				cerr << "Waiting for calibration messages from eye tracker. Start Custom Calibration..." << endl;
				bwaiting = true;
			}
		}
		else 
		{
			if (bstarted && idot == 0)
			{
				cerr << "Exiting calibration loop (calibration finished or dialog closed)" << endl;
				m_fixpts[lastidot-1].setContrast(0);
				vsgSetTriggerOptions(vsgTRIGOPT_PRESENT, 0, vsgTRIG_OUTPUTMARKER, 0.5, 0, 0x0, 0x1FE);
				vsgPresent();
				bquit = true;
			}
			else if (idot > 0 && idot < 10)
			{
				bstarted = true;
				if (idot != lastidot)
				{
					cerr << "Switch to dot " << idot << endl; 

					// change dot on screen here
					if (lastidot > 0) m_fixpts[lastidot-1].setContrast(0);
					if (idot > 0) m_fixpts[idot-1].setContrast(100);

					// set trigger and present
					vsgSetTriggerOptions(vsgTRIGOPT_PRESENT, 0, vsgTRIG_OUTPUTMARKER, 0.5, 0, 0x2, 0x1FE);
					vsgPresent();

					// save dot number
					lastidot = idot;
				}
			}
		}
		Sleep(100);
	}
	aslserial_disconnect();

	return;
}

int CalibrationUStim::callback(int &output, const FunctorCallbackTrigger* ptrig)
{
	return 0;	// no callbacks!. Still need to implement this to satisfy 
				// prargs_handler interface. That's not really required, either
				// as we don't use the triggers at this point. NBD.
}


int CalibrationUStim::init_calibration()
{
	int status=0;
	vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgBACKGROUND);
	vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1);
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	for (unsigned int i=0; i<9; i++)
	{
		m_fixpts[i].color = m_fixpt.color;
		m_fixpts[i].d = m_fixpt.d;
		m_fixpts[i].x = (((int)i % 3) - 1) * m_dCalibrationOffset;
		m_fixpts[i].y = (((int)i / 3) - 1) * m_dCalibrationOffset;
		m_fixpts[i].init(ARvsg::instance(), 2);
		m_fixpts[i].setContrast(0);
		m_fixpts[i].draw();
	}
	vsgPresent();
	vsgSetZoneDisplayPage(vsgVIDEOPAGE, 0);

	return status;
}


void CalibrationUStim::init_triggers(TSpecificFunctor<CalibrationUStim>* pfunctor)
{
	triggers().clear();

	// quit trigger
	triggers().addTrigger(new QuitTrigger("q", 0x10, 0x10, 0xff, 0x0, 0));

	return;
}

bool CalibrationUStim::parse(int argc, char **argv)
{
	bool b = false;
	int status;
	f_errflg = 0;
	status = prargs(argc, argv, (process_args_func)NULL, m_allowedArgs.c_str(), 'F', this);
	if (!status)
	{
		b = true;
	}
	return b;
}

int CalibrationUStim::process_arg(int c, std::string& arg)
{
	int i;
	static bool have_f=false;
	static bool have_d=false;
	static bool have_i=false;
	static bool have_offset = false;

	cout << "process_arg " << c << " arg " << arg << " f_errflg " << f_errflg << endl;

	switch (c) 
	{
		case 'v':
			m_verbose = true;
			break;
		case 'f': 
			if (parse_fixation_point(arg, m_fixpt)) f_errflg++;
			else have_f = true;
			break;
		case 'b': 
			if (parse_color(arg, m_background)) f_errflg++; 
			break;
		case 'd':
			if (parse_distance(arg, m_screenDistanceMM)) f_errflg++;
			else have_d=true;
			break;
		case 'c':
			if (parse_integer(arg, i)) f_errflg++;
			else m_lComPort = (long)i;
			break;
		case 'p':
			if (parse_integer(arg, m_pulse)) f_errflg++;
			break;
		case 'C':
			if (parse_double(arg, m_dCalibrationOffset)) f_errflg++;
			else have_offset = true;
			break;
		case 'i':
			m_szConfigFile = arg;
			have_i = true;
			break;
		case '?':
            f_errflg++;
			break;
		case 0:
			if (!have_f) 
			{
				cerr << "Fixation point not specified!" << endl; 
				f_errflg++;
			}
			if (!have_d)
			{
				cerr << "Screen distance not specified!" << endl; 
				f_errflg++;
			}
			if (!have_offset)
			{
				cerr << "Calibration dots offset not specified!" << endl; 
				f_errflg++;
			}
			if (!have_i)
			{
				cerr << "Serial out config file not specified!" << endl;
				f_errflg++;
			}
			break;
		default:
			f_errflg++;
			break;
	}

	return f_errflg;
}

