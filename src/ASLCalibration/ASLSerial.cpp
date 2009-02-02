#include "stdafx.h"
#include "ASLSerial.h"
#include <iostream>

using namespace std;

IASLSerialOutPort3* gpISerialOutPort = NULL;

int aslserial_connect()
{
	int status = 0;

	// Create COM object
	HRESULT hr = CoCreateInstance(CLSID_ASLSerialOutPort3, NULL, CLSCTX_INPROC_SERVER,
								IID_IASLSerialOutPort3, (void**)&gpISerialOutPort);
	if (FAILED(hr))
	{
		cerr << "Error creating COM Server ASLSerialOutLib2" << endl;
		if (hr == S_OK) cerr << "S_OK" << endl;
		else if (hr == REGDB_E_CLASSNOTREG) cerr << "REGDB_E_CLASSNOTREG" << endl;
		else if (hr == CLASS_E_NOAGGREGATION) cerr << "CLASS_E_NOAGGREGATION" << endl;
		else if (hr == E_NOINTERFACE) cerr << "E_NOINTERFACE" << endl;
		else cerr << "unknown error " << hr << endl;
		status = 1;
	}
	else
	{
		// Connect
		CString filename;
		char path[200];
		const char* SECTION = "System Settings";
		char* strStreaming = "0";
		CString strUpdateRate = "240";
		CString strBaudRate = "57600";
		long comPort = 2;	// Hopefully this means COM2
		CComBSTR bstrFile;
		VARIANT_BOOL eyeheadIntegration = VARIANT_FALSE;
		long baudRate, updateRate, itemCount;
		VARIANT_BOOL streamingMode;
		LPSAFEARRAY itemNames;

		// Create temporary configuration file that defines standard message format
		GetCurrentDirectory(175, path);
		strcat(path, "\\ETSerialPortViewer.cfg");
		filename = path;
		bstrFile = filename;
		cout << "Temp cfg file " << filename << endl;
		WritePrivateProfileString (SECTION, "using_6000_serial_out_format", "1", filename);
		WritePrivateProfileString (SECTION, "serial_out_streaming", strStreaming, filename);
		WritePrivateProfileString (SECTION, "eye_camera_update_rate", strUpdateRate, filename);
		WritePrivateProfileString (SECTION, "serial_out_baud_rate", strBaudRate, filename);

		// Standard message
		WritePrivateProfileString (SECTION, "serial_out_std_sel_1", "7201", filename);
		WritePrivateProfileString (SECTION, "serial_out_std_sel_2", "0", filename);
		WritePrivateProfileString (SECTION, "serial_out_ehi_sel_1", "14680097", filename);
		WritePrivateProfileString (SECTION, "serial_out_ehi_sel_2", "0", filename);

		// Connect to serial out port

		HRESULT hr = gpISerialOutPort->Connect(bstrFile, comPort, eyeheadIntegration,
					&baudRate, &updateRate, &streamingMode, &itemCount, &itemNames);
			
		// update user interface
		if (SUCCEEDED(hr))
		{
			cerr << "Connected to ASL controller serial port." << endl;
			cerr << "Baud rate: " << baudRate << endl;
			cerr << "Streaming? " << (streamingMode == VARIANT_TRUE ? "YES" : "NO") << endl;
			cerr << "Update rate: " << updateRate << endl;
		}
		else
		{
			CComBSTR bsError;
			gpISerialOutPort->GetLastError(&bsError);
			CString strError = bsError;
			cerr << "ERROR connecting to ASL serial port: " << strError << endl;
			status = -1;
		}

		
		// Clean up
		SafeArrayDestroy(itemNames);

	}






	
	
	return status;
}

int aslserial_disconnect()
{
	int status = 0;


	// initiate COM object
	if (gpISerialOutPort == NULL)
		return status;

	HRESULT hr = gpISerialOutPort->Disconnect();

	// update user interface
	if (SUCCEEDED(hr))
	{
		cerr << "Disconnected from ASL controller serial port." << endl;

		// Release COM interface
		if (gpISerialOutPort)
			gpISerialOutPort->Release();

		gpISerialOutPort = NULL;
	}
	else
	{
		CComBSTR bsError;
		gpISerialOutPort->GetLastError(&bsError);
		CString strError = bsError;
		cerr << "Error disconnecting from ASL controller serial port: " << strError << endl;
		status = 1;
	}

	return status;
}

int aslserial_dotnumber(int *pdotnumber)
{
	int status = -1;
	LPSAFEARRAY items;
	long count;
	VARIANT_BOOL bAvailable;

	HRESULT hr = gpISerialOutPort->GetScaledData(&items, &count, &bAvailable);

	if (FAILED(hr))
	{
		CComBSTR bsError;
		gpISerialOutPort->GetLastError(&bsError);
		CString strError = bsError;
		cerr << "Error reading data from ASL serial connection: " << strError << endl;
	}

	if (bAvailable == VARIANT_TRUE)
	{
		long i=1;
		CComVariant value;
		SafeArrayGetElement(items, &i, &value);
		*pdotnumber = value.iVal;
		VariantClear(&value); // prevent memory leak
		status = 0;

#if 0
		for (long i=0; i < count; i++)
		{
			SafeArrayGetElement(items, &i, &value);
			cout << "Item " << i << " : ";
			switch (value.vt)
			{
			case VT_UI1:
				cout << "VT_UI1 : " << value.bVal << endl;
				break;
			case VT_I1:
				cout << "VT_I1 : " << value.cVal << endl;
				break;
			case VT_UI2:
				cout << "VT_UI2 : " << value.uiVal << endl;
				break;
			case VT_I2:
				cout << "VT_I2 : " << value.iVal << endl;
				break;
			default:
				cout << " ??? type ???" << endl;
				break;
			}
			VariantClear(&value); // prevent memory leak
		}
#endif
		
		// Clean up
		SafeArrayDestroy(items);
	}
	return status;
}
