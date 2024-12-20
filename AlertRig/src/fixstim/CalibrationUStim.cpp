#include "stdafx.h"
#include "CalibrationUStim.h"
#include "ASLSerial.h"

const string CalibrationUStim::m_allowedArgs("b:c:d:f:i:p:vC:");
static int f_errflg = 0;


CalibrationUStim::CalibrationUStim()
: UStim()
, m_gpISerialOutPort(nullptr)
, m_verbose(false)
, m_pulse(0x4)
, m_screenDistanceMM(-1)
, m_lComPort(1)
, m_dCalibrationOffset(5)
{};

int CalibrationUStim::init_calibration()
{
	int status = 0;

	// COM initialzation
	CoInitialize(NULL);

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		status = -1;
	}
	else
	{
		// Create COM object
		// The call to CoCreateInstance uses CLSCTX_LOCAL_SERVER, which means the server dll is being 
		// loaded elsewhere (not by this program). The ASL dll is 32-bit, so some registry magic is done
		// so it is loaded by dllhost.exe and is accessible by a 64 bit program. 

		HRESULT hr = CoCreateInstance(CLSID_ASLSerialOutPort3, NULL, CLSCTX_LOCAL_SERVER,
			IID_IASLSerialOutPort3, (void**)&m_gpISerialOutPort);
		if (FAILED(hr))
		{
			cerr << "Error creating COM Server ASLSerialOutLib3 (" << hr << ")" << endl;
			if (hr == S_OK) cerr << "S_OK" << endl;
			else if (hr == REGDB_E_CLASSNOTREG)
			{
				CComBSTR bsError;
				m_gpISerialOutPort->GetLastError(&bsError);
				CString strError = bsError;
				cerr << "REGDB_E_CLASSNOTREG" << endl;
				cerr << bsError << endl;
			}
			else if (hr == CLASS_E_NOAGGREGATION) cerr << "CLASS_E_NOAGGREGATION" << endl;
			else if (hr == E_NOINTERFACE) cerr << "E_NOINTERFACE" << endl;
			else cerr << "unknown error " << hr << endl;
			status = 1;
		}
	}
	return status;
}


void CalibrationUStim::run_stim(alert::ARvsg& vsg)
{
	cout << "CalibrationUStim: init ASL COM thing..." << endl;

	init_calibration();

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


	// connect to asl box via serial port
	cerr << "Open ASL serial port at COM" << m_lComPort << endl;
	cerr << "Using config file " << m_szConfigFile << endl;
	if (aslserial_connect(m_gpISerialOutPort, m_szConfigFile, m_lComPort))
	{
		cerr << "Failed to connect to ASL serial port." << endl;
		return;
	}

	// Initialize VSG card or cards

	if (init_vsg_for_calibration())
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
	int idot=0;
	int iserial;
	int lastidot = -1;
	int last_output_trigger = 0;
	int counter = 0;

	while (!bquit)
	{
		counter++;
		iserial = aslserial_getDotNumber(m_gpISerialOutPort, &idot);
		if (iserial < 0)
		{
			cerr << "Error in asiserial_getDotNumber!" << endl;
			bquit = true;
		}
		else if (iserial>0)
		{
			// no data available.
			if (counter % 100 == 0)
				cerr << "Start \"Custom Calibration\"" << endl;
		}
		else 
		{
			if (!bstarted && idot == 0)
			{
				if (counter % 100 == 0)
					cerr << "Start \"Custom Calibration\"" << endl;
			}
			else if (bstarted && idot == 0)
			{
				cerr << "Exiting calibration loop (calibration finished or dialog closed)" << endl;
				m_fixpts[lastidot-1].setContrast(0);
				vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 0, 0);
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
					vsgObjSetTriggers(vsgTRIG_ONPRESENT + vsgTRIG_OUTPUTMARKER, 2, 0);
					vsgPresent();

					// save dot number
					lastidot = idot;
				}
			}
		}
		Sleep(100);
	}
	aslserial_disconnect(m_gpISerialOutPort);

	// turn off the com stuff
	CoUninitialize();

	return;
}

int CalibrationUStim::callback(int &output, const FunctorCallbackTrigger* ptrig, const std::string&)
{
	return 0;	// no callbacks!. Still need to implement this to satisfy 
				// prargs_handler interface. That's not really required, either
				// as we don't use the triggers at this point. NBD.
}


int CalibrationUStim::init_vsg_for_calibration()
{
	int status=0;
	vsgSetDrawPage(vsgVIDEOPAGE, 1, vsgBACKGROUND);
	vsgSetZoneDisplayPage(vsgVIDEOPAGE, 1);
	vsgSetDrawPage(vsgVIDEOPAGE, 0, vsgBACKGROUND);
	cerr << m_fixpt << endl;
	for (unsigned int i=0; i<9; i++)
	{
		// each fixpt is a duplicate of the fixpt from command line. We change the x,y for each, however. 

		m_fixpts[i] = m_fixpt;
		m_fixpts[i].x = (((int)i % 3) - 1) * m_dCalibrationOffset;
		m_fixpts[i].y = (((int)i / 3) - 1) * m_dCalibrationOffset;
		m_fixpts[i].init(2);
		m_fixpts[i].setContrast(0);
		m_fixpts[i].draw();
		cerr << i << " " << m_fixpts[i] << endl;
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

	//cout << "process_arg " << c << " arg " << arg << " f_errflg " << f_errflg << endl;

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

