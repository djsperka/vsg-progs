#include <windows.h>
#include <vector>
#include <string>


// Note: this function will need the following:
// #pragma comment (lib, "Winmm.lib")
// or use some other method for getting that lib linked in. 

namespace alert
{
	class Stopwatch
	{
		LARGE_INTEGER timerFreq_;
		LARGE_INTEGER counterAtStart_;
		std::vector<unsigned int> times_;
		std::vector<std::string> labels_;
		void start();
	public:
		Stopwatch();
		virtual ~Stopwatch() {};
		unsigned int split(std::string& s);
		unsigned int split(char *str);
		unsigned int count() const { return times_.size(); };
		unsigned int mark();
		unsigned int time(unsigned int i) const;
		std::string label(unsigned int i) const;
		unsigned int lap(unsigned int i) const;

		void clear();
	};

	std::ostream& operator<<(std::ostream& out, const Stopwatch& w);

};

