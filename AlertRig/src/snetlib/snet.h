#pragma once

//#include <SDKDDKVer.h>

#include <stdio.h>
#include <tchar.h>

/* windows.h must be included AFTER winsock2 */
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "ws2_32.lib")
//#pragma comment (lib, "Mswsock.lib")
//#pragma comment (lib, "AdvApi32.lib")
