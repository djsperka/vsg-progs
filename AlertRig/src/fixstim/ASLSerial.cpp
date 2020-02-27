#include "stdafx.h"
#include "ASLSerial.h"
#include <iostream>

using namespace std;

//IASLSerialOutPort3* gpISerialOutPort = NULL;
long f_dotIndex[1] = {-1};
long f_xdatIndex[1] = {-1};
long f_horzIndex[1] = {-1};
long f_vertIndex[1] = {-1};
bool f_pedantic = false;
int f_counter = 0;


void vval(char *name, CComVariant& value);
void dump(const LPSAFEARRAY& items, long count);


int aslserial_connect(IASLSerialOutPort3* gpISerialOutPort, string configfile, long comPort=2)
{
	int status = 0;

	char* strStreaming = "0";
	CString strUpdateRate = "240";
	CString strBaudRate = "57600";
	CComBSTR bstrFile;
	VARIANT_BOOL eyeheadIntegration = VARIANT_FALSE;
	long baudRate, updateRate, itemCount;
	VARIANT_BOOL streamingMode;
	LPSAFEARRAY itemNames;

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
			else
			{
				cerr << "Found item in serial output: " << bstrName << endl;
			}
			SysFreeString(bstrName);
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
	}
	else
	{
		CComBSTR bsError;
		gpISerialOutPort->GetLastError(&bsError);
		CString strError = bsError;
		cerr << "ERROR connecting to ASL serial port: " << bsError << endl;
		status = -1;
		SysFreeString(bsError);
	}

	// Clean up
	SafeArrayDestroy(itemNames);

	return status;
}



int aslserial_disconnect(IASLSerialOutPort3* gpISerialOutPort)
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

int aslserial_getDotNumber(IASLSerialOutPort3* gpISerialOutPort, int *pdotnumber)
{
	int xdat;
	float xoffset, yoffset;
	return aslserial_get(gpISerialOutPort, pdotnumber, &xdat, &xoffset, &yoffset);
}

// return -1 on failure to read serial port, 
// return 0 on success
// return 1 on data unavailable
int aslserial_get(IASLSerialOutPort3* gpISerialOutPort, int *pdotnumber, int *pxdat, float *pxoffset, float *pyoffset)
{
	int status = -1;
	LPSAFEARRAY items;
	long count;
	VARIANT_BOOL bAvailable;
	f_counter++;
	HRESULT hr = gpISerialOutPort->GetScaledData(&items, &count, &bAvailable);

	if (FAILED(hr))
	{
		CComBSTR bsError;
		gpISerialOutPort->GetLastError(&bsError);
		CString strError = bsError;
		cerr << "Error reading data from ASL serial connection: " << strError << endl;
	}
	else if (bAvailable == VARIANT_TRUE)
	{
		CComVariant value;
		SafeArrayGetElement(items, f_dotIndex, &value);
		if (f_pedantic) vval("dot", value);
		//if (f_counter % 100 == 0) dump(items, count);
		*pdotnumber = value.iVal;
		VariantClear(&value); // prevent memory leak
		*pxdat = 0;
		*pxoffset = 0;
		*pyoffset = 0;
		SafeArrayDestroy(items);

		status = 0;
	}
	else
	{
		status = 1;
	}
	return status;
}

void dump(const LPSAFEARRAY& items, long count)
{
	char tmp[64];
	CComVariant value;
	for (long i = 0; i < count; i++)
	{
		sprintf(tmp, "item %d", (int)i);
		SafeArrayGetElement(items, &i, &value);
		vval(tmp, value);
	}
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