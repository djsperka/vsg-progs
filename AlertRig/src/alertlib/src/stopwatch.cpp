#include "stopwatch.h"

using namespace alert;
using namespace std;

Stopwatch::Stopwatch()
{
	start();
}

void Stopwatch::start()
{
	QueryPerformanceFrequency(&timerFreq_);  
	QueryPerformanceCounter(&counterAtStart_);  
//	cout<<"timerFreq_ = "<<timerFreq_.QuadPart<<endl;  
//	cout<<"counterAtStart_ = "<<counterAtStart_.QuadPart<<endl;  
	TIMECAPS ptc;  
	UINT cbtc = 8;  
	MMRESULT result = timeGetDevCaps(&ptc, cbtc);  
#if 0
	if (result == TIMERR_NOERROR)  
	{    
		cout<<"Minimum resolution = "<<ptc.wPeriodMin<<endl;    
		cout<<"Maximum resolution = "<<ptc.wPeriodMax<<endl;  
	}  
	else  
	{    
		cout<<"result = TIMER ERROR"<<endl;  
	}
#endif
}

unsigned int Stopwatch::mark()
{
	if (timerFreq_.QuadPart == 0)  
	{    
		return -1;  
	}  
	else  
	{    
		LARGE_INTEGER c;    
		QueryPerformanceCounter(&c);    
		return static_cast<unsigned int>( (c.QuadPart - counterAtStart_.QuadPart) * 1000 / timerFreq_.QuadPart );  
	}
}


unsigned int Stopwatch::split(std::string& s)
{
	unsigned int t = mark();
	times_.push_back(t);
	labels_.push_back(s);
	return t;
}

unsigned int Stopwatch::split(char *str)
{
	string s;
	s.assign(str);
	return split(s);
}

void Stopwatch::clear()
{
	times_.clear();
	labels_.clear();
}

unsigned int Stopwatch::time(unsigned int i) const
{
	unsigned int t = UINT_MAX;
	if (i >= 0 && i < times_.size())
		t = times_[i];
	return t;
}

unsigned int Stopwatch::lap(unsigned int i) const
{
	unsigned int t = UINT_MAX;
	if (i == 0)
	{
		t = times_[0];
	}
	else if (i < times_.size())
	{
		t = times_[i] - times_[i-1];
	}
	return t;
}

string Stopwatch::label(unsigned int i) const
{
	string s;
	if (i >= 0 && i < times_.size())
		s = labels_[i];
	else
		s = "error!";
	return s;
}

std::ostream& alert::operator<<(std::ostream& out, const Stopwatch& w)
{
	unsigned int i;
	for (i=0; i<w.count(); i++)
	{
		out << i << ") " << w.lap(i) << " (" << w.time(i) << ") " << w.label(i) << endl;
	}
	return out;
}


#ifdef USE_TIMER_STUFF
static LARGE_INTEGER timerFreq_;
static LARGE_INTEGER counterAtStart_;


void startTime()
{  
	QueryPerformanceFrequency(&timerFreq_);  
	QueryPerformanceCounter(&counterAtStart_);  
	cout<<"timerFreq_ = "<<timerFreq_.QuadPart<<endl;  
	cout<<"counterAtStart_ = "<<counterAtStart_.QuadPart<<endl;  
	TIMECAPS ptc;  
	UINT cbtc = 8;  
	MMRESULT result = timeGetDevCaps(&ptc, cbtc);  
	if (result == TIMERR_NOERROR)  
	{    
		cout<<"Minimum resolution = "<<ptc.wPeriodMin<<endl;    
		cout<<"Maximum resolution = "<<ptc.wPeriodMax<<endl;  
	}  
	else  
	{    
		cout<<"result = TIMER ERROR"<<endl;  
	}
}

unsigned int calculateElapsedTime()
{  
	if (timerFreq_.QuadPart == 0)  
	{    
		return -1;  
	}  
	else  
	{    
		LARGE_INTEGER c;    
		QueryPerformanceCounter(&c);    
		return static_cast<unsigned int>( (c.QuadPart - counterAtStart_.QuadPart) * 1000 / timerFreq_.QuadPart );  
	}
}


void dump_times(unsigned int *times, int n)
{
	cout << "dump times" << endl;
	for (int i=1; i<n; i++)
	{
		cout << i << " " << times[i]-times[i-1] << "        " << times[i] << " - " << times[i-1] << endl;
	}
	cout << "T" << " " << times[n-1]-times[0] << endl;
}

#define TIME(n) (times[n] = calculateElapsedTime())

#endif
