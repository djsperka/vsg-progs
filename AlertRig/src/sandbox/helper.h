#if !defined(_HELPER_H_)
#define _HELPER_H_
#include <string>
#include <vector>
#include "Alertlib.h"

int getcolor(std::string s, COLOR_TYPE& c);
int getcolorvector(std::string s, COLOR_VECTOR_TYPE& v);
int getpattern(std::string s, PATTERN_TYPE& p);
int getaperture(std::string s, APERTURE_TYPE& a);
void tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters);

#endif