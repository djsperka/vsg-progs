#include <string>
#include "alertlib.h"

bool GetRegConfiguration();
bool GetRegScreenDistance(int& dist);
bool GetRegFixpt(double& fixX, double& fixY, double &fixD, COLOR_TYPE& c);
bool GetRegStimulus(alert::ARApertureGratingSpec &stim);
bool SaveRegStimulus(std::string s);
bool GetRegLockFile(std::string &s);
bool GetRegVSGConfig(std::string& s);
bool GetRegVSGMaster(std::string& s);
bool GetRegVSGSlave(std::string& s);
bool GetRegString(const char *key, const char *value, std::string& s);
