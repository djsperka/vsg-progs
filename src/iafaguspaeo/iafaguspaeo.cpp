// iafaguspaeo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "iafaguspaeo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ASL File COM server
//#import "ASLFileLib2.tlb" no_namespace named_guids raw_interfaces_only

CComPtr<IASLFile> m_pIASLFile;

// The one and only application object

CWinApp theApp;

using namespace std;

int doStuff(char *filename);

int main(int argc, char *argv[])
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // initialize MFC and print and error on failure
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: change error code to suit your needs
            wprintf(L"Fatal Error: MFC initialization failed\n");
            nRetCode = 1;
        }
        else
        {
            // TODO: code your application's behavior here.
			doStuff(argv[1]);
        }
    }
    else
    {
        // TODO: change error code to suit your needs
        wprintf(L"Fatal Error: GetModuleHandle failed\n");
        nRetCode = 1;
    }

    return nRetCode;
}


int doStuff(char *filename)
{
	HRESULT hr;

	//		cerr << "input file: " << argv[1] << endl;
		// Create COM server

	m_pIASLFile = NULL;
	// Enable COM client
	cerr << "co init" << endl;
	CoInitialize(NULL);

	cerr << "coCreateInstance" << endl;
	hr = m_pIASLFile.CoCreateInstance(CLSID_ASLFile);
	if (FAILED(hr))
	{
		cerr << "Error creating COM server. Make sure ASLFileLib.DLL is registered" << endl;
		return -1;
	}

	CComBSTR bstrFileName = filename;
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

