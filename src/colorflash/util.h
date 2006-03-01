#if !defined(_UTIL_H_)
#define _UTIL_H_

#include <string>
#include <vector>
using namespace std;

int parse_integer(std::string s, int& dist);
int parse_xy(std::string s, double& x, double& y);
int parse_double(std::string s, double& d);
void mytokenize(const string& str, vector<string>& tokens, const string& delimiters = " ");



#endif
