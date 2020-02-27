#include <string>

int aslserial_connect(IASLSerialOutPort3* gpISerialOutPort, std::string filename, long comPort);
int aslserial_getDotNumber(IASLSerialOutPort3* gpISerialOutPort, int *pdotnumber);
int aslserial_get(IASLSerialOutPort3* gpISerialOutPort, int *pdotnumber, int *pxdat, float *pxoffset, float *pyoffset);
int aslserial_disconnect(IASLSerialOutPort3* gpISerialOutPort);