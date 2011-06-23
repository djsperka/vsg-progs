#include "stdafx.h"
#include "ASLSerial.h"
#include <iostream>

using namespace std;

IASLSerialOutPort3* gpISerialOutPort = NULL;
long f_dotIndex[1] = {-1};
long f_xdatIndex[1] = {-1};
long f_horzIndex[1] = {-1};
long f_vertIndex[1] = {-1};
bool f_pedantic = true;


void vval(char *name, CComVariant& value);


int aslserial_connect(string configfile)
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
#if 0
		CString filename;
		const char* SECTION = "System Settings";
#endif
		char* strStreaming = "0";
		CString strUpdateRate = "240";
		CString strBaudRate = "57600";
		long comPort = 2;	// Hopefully this means COM2
		CComBSTR bstrFile;
		VARIANT_BOOL eyeheadIntegration = VARIANT_FALSE;
		long baudRate, updateRate, itemCount;
		VARIANT_BOOL streamingMode;
		LPSAFEARRAY itemNames;
#if 0
		// Create temporary configuration file that defines standard message format
		GetCurrentDirectory(175, path);
		strcat_s(path, 200, "\\ETSerialPortViewer.cfg");
		filename = path;
		bstrFile = filename;
		cout << "Temp cfg file " << filename << endl;
		WritePrivateProfileString (SECTION, "using_6000_serial_out_format", "1", filename);
		WritePrivateProfileString (SECTION, "serial_out_streaming", strStreaming, filename);
		WritePrivateProfileString (SECTION, "eye_camera_update_rate", strUpdateRate, filename);
		WritePrivateProfileString (SECTION, "serial_out_baud_rate", strBaudRate, filename);

		// Standard message.. no
		// 7205 below indicates that XDAT is in the configuration.
		WritePrivateProfileString (SECTION, "serial_out_std_sel_1", "7205", filename);
		WritePrivateProfileString (SECTION, "serial_out_std_sel_2", "0", filename);
		WritePrivateProfileString (SECTION, "serial_out_ehi_sel_1", "14680097", filename);
		WritePrivateProfileString (SECTION, "serial_out_ehi_sel_2", "0", filename);
#endif

		// Connect to serial out port

		bstrFile = configfile.c_str();
		HRESULT hr = gpISerialOutPort->Connect(bstrFile, comPort, eyeheadIntegration,
					&baudRate, &updateRate, &streamingMode, &itemCount, &itemNames);
			
		// If connection succeeded then look in itemNames for the "XDAT" and "
		if (SUCCEEDED(hr))
		{
			long ix[1];     // array index must be declared as one-dimentional array
			cerr << "Connected to ASL controller serial port." << endl;
			cerr << "Baud rate: " << baudRate << endl;
			cerr << "Streaming? " << (streamingMode == VARIANT_TRUE ? "YES" : "NO") << endl;
			cerr << "Update rate: " << updateRate << endl;

			ix[0] = 0;
			for (long i=0; i < itemCount; i++)
			{
				CComBSTR bstrName;
				ix[0] = i;
				SafeArrayGetElement(itemNames, ix, &bstrName);

				cerr << i << ".) Serial output item: " << CString(bstrName) << endl;
				if (bstrName == "XDAT")
				{
					f_xdatIndex[0] = i;
					cerr << "Found XDAT at item index " << i << endl;
				}
				else if (bstrName == "pupil_diam")
				{
					f_dotIndex[0] = i;
					cerr << "Found pupil_diam at item index " << i << endl;
				}
				else if (bstrName == "horz_gaze_coord")
				{
					f_horzIndex[0] = i;
					cerr << "Found horz_gaze_coord at item index " << i << endl;
				}
				else if (bstrName == "vert_gaze_coord")
				{
					f_vertIndex[0] = i;
					cerr << "Found vert_gaze_coord at item index " << i << endl;
				}
			}

			if (f_xdatIndex[0] < 0)
			{
				cerr << "ERROR: Serial output does not contain XDAT. Check serial configuration!" << endl;
				status = -1;
			}
			if (f_dotIndex[0] < 0)
			{
				cerr << "ERROR: Serial output does not contain pupil_diam. Check serial configuration!" << endl;
				status = -1;
			}
			if (f_horzIndex[0] < 0)
			{
				cerr << "ERROR: Serial output does not contain horz_gaze_coord. Check serial configuration!" << endl;
				status = -1;
			}
			if (f_vertIndex[0] < 0)
			{
				cerr << "ERROR: Serial output does not contain vert_gaze_coord. Check serial configuration!" << endl;
				status = -1;
			}
			if (status < 0)
			{
				aslserial_disconnect();
			}
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

int aslserial_get(int *pdotnumber, int *pxdat, float *pxoffset, float *pyoffset)
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
		CComVariant value;
		SafeArrayGetElement(items, f_dotIndex, &value);
		if (f_pedantic) vval("dot", value);
		*pdotnumber = value.iVal;
		VariantClear(&value); // prevent memory leak
		SafeArrayGetElement(items, f_xdatIndex, &value);
		if (f_pedantic) vval("xdat", value);
		*pxdat = value.iVal;
		VariantClear(&value); // prevent memory leak
		SafeArrayGetElement(items, f_horzIndex, &value);
		if (f_pedantic) vval("xoffset", value);
		*pxoffset = value.fltVal;
		VariantClear(&value); // prevent memory leak
		SafeArrayGetElement(items, f_vertIndex, &value);
		if (f_pedantic) vval("yoffset", value);
		*pyoffset = value.fltVal;
		VariantClear(&value); // prevent memory leak
		status = 0;
		f_pedantic = false;
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
		for (long i=0; i < count; i++)
		{
			// Print value
			SafeArrayGetElement(items, &i, &value);
			value.ChangeType(VT_BSTR);
			CString str = value.bstrVal;
			VariantClear(&value); // prevent memory leak

			cout << i << " str=" << str << endl;
		}
#endif	

		// Clean up
		SafeArrayDestroy(items);
	}
	return status;
}


void vval(char *name, CComVariant& value)
{
	cout << name << ": ";
	switch (value.vt)
	{
	case VT_I4:
		cout << "VT_I4 : " << value.lVal << endl;
		break;
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
	case VT_R4:
		cout << "VT_R4 : " << value.fltVal << endl;
		break;
	default:
		cout << " ??? type ??? " << value.vt << endl;
		break;
	}
	return;
}