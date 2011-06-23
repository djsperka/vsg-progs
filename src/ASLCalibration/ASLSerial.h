#include <string>

int aslserial_connect(std::string filename);
int aslserial_get(int *pdotnumber, int *pxdat, float *pxoffset, float *pyoffset);
int aslserial_disconnect();