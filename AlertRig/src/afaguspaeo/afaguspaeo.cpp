// afaguspaeo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>

// ASL File COM server
#import "ASLFileLib2.tlb" no_namespace named_guids raw_interfaces_only

CComPtr<IASLFile> m_pIASLFile;

using namespace std;

int main(int argc, char *argv[])
{
	HRESULT hr;

	cerr << "input file: " << argv[1] << endl;
	// Create COM server
	m_pIASLFile = NULL;


	//AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

//#ifdef _AFXDLL
//	Enable3dControls();			// Call this when using MFC in a shared DLL
//#else
//	//	Enable3dControlsStatic();	// Call this when linking to MFC statically
//#endif

	// Enable COM client
	cerr << "co init" << endl;
	CoInitialize(NULL);

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		return -1;
	}

	cerr << "coCreateInstance" << endl;
	hr = m_pIASLFile.CoCreateInstance(CLSID_ASLFile);
	if (FAILED(hr))
	{
		cerr << "Error creating COM server. Make sure ASLFileLib.DLL is registered" << endl;
		return -1;
	}

	CComBSTR bstrFileName = argv[1];
	long segmentCount, itemCount, overtimeCount;
	LPSAFEARRAY itemNames;
//	ASLFile_HeaderData headerData;

	cerr << "open file" << endl;
	hr = m_pIASLFile->OpenFileForReading(bstrFileName, &segmentCount, &itemNames, &itemCount, &overtimeCount);
	if (FAILED(hr))
	{
		cerr << "Error Opening File" << endl;
		return -1;
	}

	cerr << "close file" << endl;
	m_pIASLFile->CloseFile();

	// Clean up COM
	cerr << "uninitialize" << endl;
	CoUninitialize();


    return 0;
}
