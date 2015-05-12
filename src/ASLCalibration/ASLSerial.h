#include <string>

int aslserial_connect(std::string filename, long comPort);
int aslserial_getDotNumber(int *pdotnumber);
int aslserial_get(int *pdotnumber, int *pxdat, float *pxoffset, float *pyoffset);
int aslserial_disconnect();