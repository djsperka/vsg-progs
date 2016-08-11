// ftdi-test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <sstream>
#include "ftd2xx.h"

using namespace std;

/*
 * macros for bitbang mode 
 */
#define PIN_TX  0x01  /* Orange wire on FTDI cable */
#define PIX_RX  0x02  /* Yellow */
#define PIN_RTS 0x04  /* Green */
#define PIN_CTS 0x08  /* Brown */
#define PIN_DTR 0x10
#define PIN_DSR 0x20
#define PIN_DCD 0x40
#define PIN_RI  0x80

int main()
{
	int i, n;
	//unsigned char data[255 * 256];
	FT_HANDLE handle;
	DWORD bytes;

#if 0

#define LED1 0x08  /* CTS (brown wire on FTDI cable) */
#define LED2 0x01  /* TX  (orange) */
#define LED3 0x02  /* RX  (yellow) */
#define LED4 0x14  /* RTS (green on FTDI) + DTR (on SparkFun breakout) */
	/* Generate data for a single PWM 'throb' cycle */
	memset(data, 0, sizeof(data));
	for (i = 1; i<128; i++) {
		/* Apply gamma correction to PWM brightness */
		n = (int)(pow((double)i / 127.0, 2.5) * 255.0);
		memset(&data[i * 255], LED1, n);         /* Ramp up */
		memset(&data[(256 - i) * 255], LED1, n); /* Ramp down */
	}

	/* Copy data from first LED to others, offset as appropriate */
	n = sizeof(data) / 4;
	for (i = 0; i<sizeof(data); i++)
	{
		if (data[i] & LED1) {
			data[(i + n) % sizeof(data)] |= LED2;
			data[(i + n * 2) % sizeof(data)] |= LED3;
			data[(i + n * 3) % sizeof(data)] |= LED4;
		}
	}
#endif

	/* Initialize, open device, set bitbang mode w/5 outputs */
	if (FT_Open(0, &handle) != FT_OK) {
		puts("Can't open device");
		return 1;
	}
	FT_SetBitMode(handle, 0xff, 1);
	FT_SetBaudRate(handle, 9600);  /* Actually 9600 * 16 */

	char buffer[2];
	DWORD written = 0;
	FT_STATUS ftstat;
	string s;
	buffer[0] = 0xff;
	buffer[1] = 0;

	cout << "Enter # or q: ";
	cin >> s;
	while (s[0] != 'q')
	{
		int i;
		istringstream iss(s);
		iss >> i;
		buffer[0] = (char)i;
		cout << "will write " << (int)buffer[0] << endl;
		ftstat = FT_Write(handle, buffer, 2, &written);

		if (ftstat == FT_OK)
			cout << "write OK, wrote " << written << " byte" << endl;
		else
			cout << "write NOT OK" << endl;


		cout << "Enter # or q: ";
		cin >> s;
	}



	ftstat = FT_Write(handle, buffer, 1, &written);

	if (ftstat == FT_OK)
		cout << "OK" << written << endl;
	else
		cout << "NOT OK" << endl;

	/* Endless loop: dump precomputed PWM data to the device */
	//for (;;) FT_Write(handle, &data, (DWORD)sizeof(data), &bytes);


	return 0;
}

