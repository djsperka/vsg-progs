#include <string>
#include "alertlib.h"

bool GetRegConfiguration();
bool GetRegScreenDistance(int& dist);
bool GetRegFixpt(double& fixX, double& fixY, double &fixD, COLOR_TYPE& c);
bool GetRegStimulus(alert::ARApertureGratingSpec &stim);
bool SaveRegStimulus(std::string s);
bool GetRegLockFile(std::string &s);

