#include "Trigger.h"
#include <sstream>
#include <iostream>
#include <algorithm>

namespace alert 
{

	Trigger::Trigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val)
	{
		m_key = i_key;
		m_in_mask = i_in_mask;
		m_out_mask = i_out_mask;

		// Check if the output value should be toggled
		if (i_out_val & AR_TRIGGER_TOGGLE)
		{
			//				std::cout << "TOGGLE(output) " << m_key << " out_val " << i_out_val << std::endl;
			m_btoggleOut = true;
			m_out_val = i_out_val & ~(AR_TRIGGER_TOGGLE);
			m_last_out_val = ~m_out_val & m_out_mask;
		}
		else
		{
			m_btoggleOut = false;
			m_out_val = i_out_val;
			m_last_out_val = m_out_val;
		}

		// Check if the input value will be toggled
		if (i_in_val & AR_TRIGGER_TOGGLE)
		{
			//				std::cout << "Trigger(" << m_key << ")" << " TOGGLE(input) " << m_key << " in_val " << i_in_val << std::endl;
			m_btoggleIn = true;
			m_in_val = i_in_val & ~(AR_TRIGGER_TOGGLE);
			//				m_in_last = ~m_in_val & m_in_mask;
			//				m_in_last = 0;
		}
		else
		{
			m_btoggleIn = false;
			m_in_val = i_in_val;
			//				m_in_last = m_in_val;
			//				m_in_last = 0;
		}

		reset();	// this gives the m_in_val a value.

	};

	Trigger::~Trigger() {}

	bool Trigger::checkAscii(std::string input)
	{
		return input == m_key;
	};

	bool Trigger::checkBinary(int input)
	{
		bool bValue = false;
		int current = input & m_in_mask;

		if (!m_btoggleIn)
		{
			if (current != m_in_last && current == m_in_val)
			{
				bValue = true;
			}
			m_in_last = current;
		}
		else
		{
			// We expect the trigger to be the last trigger toggled. If a trigger covers more than 
			// one bit, then all must be inverted. Note that the FIRST trigger expected is the value
			// given as i_in_val (without the AR_TRIGGER_TOGGLE bit). Subsequent triggers are expected
			// to be toggled. 
			if (((~current)&m_in_mask) == m_in_last)
			{
				bValue = true;
				m_in_last = current;	// Note that the last value is saved only if a toggled trigger
			}
			//				std::cout << "Trigger(" << m_key << ")" <<  std::hex << " input " << input << " m_in_mask=" << m_in_mask << " m_in_val=" << m_in_val << " current=" << current << " last=" << m_in_last << " bval=" << bValue << std::endl;
		}
		//			if (bValue) std::cerr << "Trigger(" << m_key << ")" << std::hex << " input " << input << " m_in_mask=" << m_in_mask << " m_in_val=" << m_in_val << " current=" << current << " bval=" << bValue << std::endl;
		return bValue;
	};

	void Trigger::setMarker(int& output)
	{
		if (!m_btoggleOut)
		{
			if (m_out_val >= 0)
			{
				//					std::cout << "setMarker: output(in)=" << output;
				output = m_out_val | (output&(~m_out_mask));
				//					std::cout << "setMarker: output(out)=" << output << std::endl;
			}
		}
		else
		{
			int temp;
			temp = ~m_last_out_val & m_out_mask;
			output = temp | (output&(~m_out_mask));
			m_last_out_val = temp;
		}
	};


	std::string Trigger::toString() const
	{
		std::ostringstream oss;
		oss << "Trigger " << m_key << " in m/v/t: 0x" << std::hex << m_in_mask << "/0x" << m_in_val << "/" << m_btoggleIn << " out m/v/t: 0x" << m_out_mask << "/0x" << m_out_val << "/" << m_btoggleOut;
		return oss.str();
	}

	// Execute the triggers' action(s). Subclasses should return >0 if a vsgPresent() will be 
	// needed, 0 if no present(), and <0 if this trigger means quit (a call to vsgPresent() is 
	// is made in this case, too -- that way a quitting trigger can set the page to blank
	// or something similarly intelligent. 
	int Trigger::execute(int& output) { return 0; };

	void Trigger::reset()
	{
		reset(0);
	};


	// reset the trigger's notion of what the last input value was. 
	// This should be called in order to ensure that the first instance of the 
	// trigger will in fact fire the trigger. Remember that in order for the trigger
	// to fire the input must be DIFFERENT than the last time it was called, AND it
	// must match the in_val. Its best to call TriggerVector::reset(), not this
	// directly


	void Trigger::reset(int input)
	{
		if (!m_btoggleIn)
		{
			// why is this "~ here? m_in_last = (~input) & m_in_mask;
			m_in_last = input & m_in_mask;
		}
		else
		{
			m_in_last = input & m_in_mask;
		}
	};


	std::string Trigger::getKey() const { return m_key; };
	int Trigger::inVal() const { return m_in_val; };
	int Trigger::inMask() const { return m_in_mask; };
	int Trigger::outMask() const { return m_out_mask; };
	int Trigger::outVal() const { return m_out_val; };

	std::ostream& operator<<(std::ostream& out, const alert::Trigger& t)
	{
		out << t.toString();
		return out;
	}


	CallbackTrigger::CallbackTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, TriggerCallbackFunc tcf) 
		: Trigger(i_key, i_in_mask, i_in_val, i_out_mask, i_out_val)
		, m_callback(tcf) 
	{}
	
	CallbackTrigger::~CallbackTrigger() 
	{}

	int CallbackTrigger::execute(int& output)
	{
		setMarker(output);
		return m_callback(output, this);
	};

	std::string CallbackTrigger::toString() const
	{
		std::ostringstream oss;
		oss << "Callback " << m_key << " in m/v/t: 0x" << std::hex << m_in_mask << "/0x" << m_in_val << "/" << m_btoggleIn << " out m/v/t: 0x" << m_out_mask << "/0x" << m_out_val << "/" << m_btoggleOut;
		return oss.str();
	}



	MultiInputSingleOutputCallbackTrigger::MultiInputSingleOutputCallbackTrigger(std::vector<std::pair<std::string, int> >& v, int i_in_mask, int i_out_mask, int i_out_val, TriggerCallbackFunc tcf) 
		: std::vector< std::pair < std::string, int> >(v)
		, CallbackTrigger("", i_in_mask, 0, i_out_mask, i_out_val, tcf)
		, m_input_matched(-1)
		{};

	MultiInputSingleOutputCallbackTrigger::~MultiInputSingleOutputCallbackTrigger() 
	{
	};

	bool MultiInputSingleOutputCallbackTrigger::checkAscii(std::string input)
	{
		unsigned int i;
		m_input_matched = -1;
		m_key = "NO MATCH";
		for (i = 0; i < this->size(); i++)
		{
			if (input == (*this)[i].first)
			{
				m_input_matched = i;
				m_key = (*this)[i].first;
				break;
			}
		}
		return (m_input_matched >= 0);
	};

	bool MultiInputSingleOutputCallbackTrigger::checkBinary(int input)
	{
		unsigned int i;
		bool bValue = false;
		int current = input & m_in_mask;

		m_input_matched = -1;
		m_key = "NO MATCH";
		if (!m_btoggleIn)
		{
			if (current != m_in_last)
			{
				for (i = 0; i < this->size(); i++)
				{
					if (current == (*this)[i].second)
					{
						m_input_matched = i;
						m_key = (*this)[i].first;
						break;
					}
				}
			}
			m_in_last = current;
			bValue = (m_input_matched >= 0);
		}
		else
		{
			// Toggling inputs is a bit ill-defined for this type of trigger. 
			// I'll just not try and define it now and call it an error. Always false. 
			std::cerr << "Cannot use toggled input trigger for MultiInputSingleOutputCallbackTrigger" << std::endl;
			bValue = false;
		}

		return bValue;
	};

	int MultiInputSingleOutputCallbackTrigger::execute(int& output)
	{
		setMarker(output);
		return m_callback(output, this);
	};

	std::string MultiInputSingleOutputCallbackTrigger::getKey() const
	{
		//			std::cout << "getKey(): m_input_matched = " << m_input_matched << std::endl;
		if (m_input_matched < 0 || m_input_matched >(int)this->size()) return "ERROR";
		else return (*this)[m_input_matched].first;
	};

	std::string MultiInputSingleOutputCallbackTrigger::toString() const
	{
		unsigned int i;
		std::ostringstream oss;
		oss << "MISOCallback " << std::endl;
		for (i = 0; i < this->size(); i++)
		{
			oss << "         " << (*this)[i].first << " in m/v/t: 0x" << std::hex << m_in_mask << "/0x" << (*this)[i].second << "/" << m_btoggleIn << " out m/v/t: 0x" << m_out_mask << "/0x" << m_out_val << "/" << m_btoggleOut;
			if (i < (this->size() - 1)) oss << std::endl;
		}
		return oss.str();
	}


	FunctorCallbackTrigger::FunctorCallbackTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, TFunctor *pfunc) 
		: Trigger(i_key, i_in_mask, i_in_val, i_out_mask, i_out_val)
		, m_pfunc(pfunc) 
	{
	};

	FunctorCallbackTrigger::~FunctorCallbackTrigger() 
	{
	};
	
	int FunctorCallbackTrigger::execute(int& output)
	{
		setMarker(output);
		return m_pfunc->callback(output, this);
	};

	std::string FunctorCallbackTrigger::toString() const
	{
		std::ostringstream oss;
		oss << "Callback " << m_key << " in m/v/t: 0x" << std::hex << m_in_mask << "/0x" << m_in_val << "/" << m_btoggleIn << " out m/v/t: 0x" << m_out_mask << "/0x" << m_out_val << "/" << m_btoggleOut;
		return oss.str();
	}



	PageTrigger::PageTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, int i_page)
		: Trigger(i_key, i_in_mask, i_in_val, i_out_mask, i_out_val)
		, m_page(i_page) 
	{
	};

	PageTrigger::~PageTrigger() 
	{
	};

	int PageTrigger::execute(int& output)
	{
		setMarker(output);
		vsgSetDrawPage(vsgVIDEOPAGE, m_page, vsgNOCLEAR);
		return 1;
	};

	std::string PageTrigger::toString() const
	{
		std::ostringstream oss;
		oss << "Page(" << m_page << ")" << m_key << " in m/v/t: 0x" << std::hex << m_in_mask << "/0x" << m_in_val << "/" << m_btoggleIn << " out m/v/t: 0x" << m_out_mask << "/0x" << m_out_val << "/" << m_btoggleOut;
		return oss.str();
	}

	TogglePageTrigger::TogglePageTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, int i_pageA, int i_pageB)
		: Trigger(i_key, i_in_mask, i_in_val, i_out_mask, i_out_val)
		, m_pageA(i_pageA)
		, m_pageB(i_pageB) 
	{};
	
	TogglePageTrigger::~TogglePageTrigger() 
	{};

	int TogglePageTrigger::execute(int& output)
	{
		int iPage = vsgGetZoneDisplayPage(vsgVIDEOPAGE);
		if (iPage == m_pageA) iPage = m_pageB;
		else if (iPage == m_pageB) iPage = m_pageA;
		vsgSetDrawPage(vsgVIDEOPAGE, iPage, vsgNOCLEAR);
		setMarker(output);
		return 1;
	};

	std::string TogglePageTrigger::toString() const
	{
		std::ostringstream oss;
		oss << "TogglePage(" << m_pageA << "/" << m_pageB << ")" << m_key << " in m/v/t: 0x" << std::hex << m_in_mask << "/0x" << m_in_val << "/" << m_btoggleIn << " out m/v/t: 0x" << m_out_mask << "/0x" << m_out_val << "/" << m_btoggleOut;
		return oss.str();
	}

	// callback for page cycling trigger
	typedef int(*PageCyclingTriggerCallbackFunc)(int icycle);

	PageCyclingTrigger::PageCyclingTrigger(std::string i_key, int n_repeats, PageCyclingTriggerCallbackFunc callback) 
		: Trigger(i_key, 0, 0, 0, 0)
		, m_nrepeats(n_repeats)
		, m_repeat_count(0)
		, m_is_started(false)
		, m_start_pending(false)
		, m_callback(callback) 
	{};

	OverlayPageTrigger::OverlayPageTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, int i_page) 
		: Trigger(i_key, i_in_mask, i_in_val, i_out_mask, i_out_val)
		, m_page(i_page) 
	{};

	OverlayPageTrigger::~OverlayPageTrigger() 
	{};
	
	int OverlayPageTrigger::execute(int& output)
	{
		setMarker(output);
		vsgSetDrawPage(vsgOVERLAYPAGE, m_page, vsgNOCLEAR);
		return 1;
	};

	std::string OverlayPageTrigger::toString() const
	{
		std::ostringstream oss;
		oss << "OverlayPage(" << m_page << ")" << m_key << " in m/v/t: 0x" << std::hex << m_in_mask << "/0x" << m_in_val << "/" << m_btoggleIn << " out m/v/t: 0x" << m_out_mask << "/0x" << m_out_val << "/" << m_btoggleOut;
		return oss.str();
	}


	QuitTrigger::QuitTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, int i_page) 
		: PageTrigger(i_key, i_in_mask, i_in_val, i_out_mask, i_out_val, i_page) 
	{};
	
	QuitTrigger::~QuitTrigger() 
	{};

	int QuitTrigger::execute(int& output)
	{
		setMarker(output);
		vsgSetCommand(vsgCYCLEPAGEDISABLE);
		vsgSetDrawPage(vsgVIDEOPAGE, m_page, vsgNOCLEAR);
		return -1;
	};
	
	std::string QuitTrigger::toString() const
	{
		std::ostringstream oss;
		oss << "Quit    " << m_key << " in m/v/t: 0x" << std::hex << m_in_mask << "/0x" << m_in_val << "/" << m_btoggleIn << " out m/v/t: 0x" << m_out_mask << "/0x" << m_out_val << "/" << m_btoggleOut;
		return oss.str();
	}



	ContrastTrigger::ContrastTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val) 
		: Trigger(i_key, i_in_mask, i_in_val, i_out_mask, i_out_val) 
	{};

	ContrastTrigger::~ContrastTrigger() 
	{};

	int ContrastTrigger::execute(int& output)
	{
		setMarker(output);
		for (unsigned int i = 0; i < this->size(); i++)
		{
			vsgObjSelect((*this)[i].first);
			vsgObjSetContrast((*this)[i].second);
		}
		return 1;
	};

	std::string ContrastTrigger::toString() const
	{
		unsigned int i;
		std::ostringstream oss;
		oss << "Contrast( ";
		for (i = 0; i < this->size(); i++)
		{
			oss << (int)((*this)[i].first) << "->" << (*this)[i].second;
			if (i < this->size() - 1) oss << ":";
		}
		oss << ") ";
		oss << m_key << " in m/v/t: 0x" << std::hex << m_in_mask << "/0x" << m_in_val << "/" << m_btoggleIn << " out m/v/t: 0x" << m_out_mask << "/0x" << m_out_val << "/" << m_btoggleOut;
		return oss.str();
	}





	TriggerFunc::TriggerFunc(std::string key, int otrigger, bool verbose) 
		: m_binary(false)
		, m_skey(key)
		, m_present(false)
		, m_otrigger(otrigger)
		, m_page(-1)
		, m_quit(false)
		, m_ideferred(0)
		, m_verbose(verbose) 
	{};

	TriggerFunc::TriggerFunc(int itrigger, int otrigger, bool verbose)
		: m_binary(true)
		, m_itrigger(itrigger)
		, m_present(false)
		, m_otrigger(otrigger)
		, m_page(-1)
		, m_quit(false)
		, m_ideferred(0)
		, m_verbose(verbose) 
	{};

	int TriggerFunc::page() { return m_page; }
	bool TriggerFunc::present() { return m_present; }
	int TriggerFunc::output_trigger() { return m_otrigger; }
	bool TriggerFunc::quit() { return m_quit; }
	int TriggerFunc::deferred() { return m_ideferred; }
	const std::string& TriggerFunc::triggers_matched() { return m_triggers_matched; }
	void TriggerFunc::operator()(Trigger* pitem)
	{
		bool bTest = false;
		if (m_binary) bTest = pitem->checkBinary(m_itrigger);
		else bTest = pitem->checkAscii(m_skey);

		if (bTest)
		{
			int i;
			m_triggers_matched.append(pitem->getKey());
			i = pitem->execute(m_otrigger);
			if (m_verbose) std::cout << "Trigger " << pitem->getKey() << " execute: " << std::hex << i << std::endl;
			m_ideferred = i;
			if (i > 0) m_present = true;
			else if (i < 0)
			{
				m_present = true;
				m_quit = true;
			}
		}
	}

	bool PageCyclingTrigger::checkAscii(std::string input)
	{
		return checkBinary(0);
	};

	bool PageCyclingTrigger::checkBinary(int input)
	{
		bool bValue = false;
		if (m_start_pending)
		{
			int cyclingPage = vsgGetSystemAttribute(vsgPAGECYCLINGSTATE);
			if (cyclingPage < 0)
			{
				std::cerr << "Page cycling trigger - start pending..." << std::endl;
			}
			else
			{
				std::cerr << "Page cycling trigger - started" << std::endl;
				m_start_pending = false;
				m_is_started = true;
			}
		}

		if (m_is_started)
		{
			// check if cycling is still running. If not, then set bValue = true.
			int cyclingPage = vsgGetSystemAttribute(vsgPAGECYCLINGSTATE);
			if (cyclingPage < 0)
			{
				m_repeat_count++;
				bValue = true;
			}
		}
		return bValue;
	};

	void PageCyclingTrigger::started()
	{
		m_start_pending = true;
		//m_is_started = true;
	}

	void PageCyclingTrigger::stopped()
	{
		m_is_started = false;
		m_start_pending = false;
		m_repeat_count = 0;
	}

	int PageCyclingTrigger::execute(int& output)
	{
		std::cerr << "PageCyclingTrigger::execute repeat_count " << m_repeat_count << " nrepeats " << m_nrepeats << std::endl;
		if (m_repeat_count < m_nrepeats)
		{
			if (m_callback)
			{
				if (m_callback(m_repeat_count))
				{
					m_is_started = false;
				}
				else
				{
					setMarker(output);
					vsgSetCommand(vsgCYCLEPAGEENABLE);
				}
			}
			else
			{
				std::cerr << "Re-enable cycling" << std::endl;
				setMarker(output);
				vsgSetCommand(vsgCYCLEPAGEENABLE);
			}
		}
		else
		{
			std::cerr << "Done with all cycles" << std::endl;
			m_is_started = false;
		}
		return 0;
	};

	FileChangedTrigger::FileChangedTrigger(std::string i_key, std::string filename, FileChangedTriggerCallbackFunc callback, int i_out_mask, int i_out_val)
		: Trigger(i_key, 0, 0, i_out_mask, i_out_val)
		, m_filename(filename)
		, m_callback(callback)
		, m_exists(false)
		, m_bCreated(false)
		, m_bDeleted(false)
	{
		init();
	}

	FileChangedTrigger::~FileChangedTrigger()
	{};

	bool FileChangedTrigger::checkAscii(std::string input)
	{
		return checkBinary(0);
	};

	bool FileChangedTrigger::checkBinary(int input)
	{
		struct _stat fileinfo;
		bool bValue = false;
		int status = 0;

		status = _stat(m_filename.c_str(), &fileinfo);
		if (status)
		{
			// nonzero status value means the file was not found.
			if (m_exists)
			{
				bValue = true;
				m_bCreated = false;
				m_bDeleted = true;
				m_exists = false;
			}
		}
		else
		{
			if (!m_exists)
			{
				bValue = true;
				m_bCreated = true;
				m_bDeleted = false;
				m_exists = true;
				m_fileinfo = fileinfo;
			}
			else if (fileinfo.st_mtime != m_fileinfo.st_mtime)
			{
				bValue = true;
				m_bCreated = false;
				m_bDeleted = false;
				m_fileinfo = fileinfo;
			}
		}
		return bValue;
	};

	int FileChangedTrigger::execute(int& output)
	{
		if (m_callback)
		{
			if (!m_callback(m_bCreated, m_bDeleted))
			{
				setMarker(output);
			}
		}
		else
		{
			setMarker(output);
		}
		// When writing via vsgIOWriteDigitalOut we must shift left 1 bit because vsgObjSetTriggers does. 
		if (m_out_mask) vsgIOWriteDigitalOut(output << 1, m_out_mask << 1);
		return 0;
	};

	void FileChangedTrigger::init()
	{
		if (_stat(m_filename.c_str(), &m_fileinfo)) m_exists = false;
		else m_exists = true;
	};

	ResetTriggerFunc::ResetTriggerFunc(int val) 
		: m_val(val) 
	{};

	ResetTriggerFunc::~ResetTriggerFunc() 
	{};

	void ResetTriggerFunc::operator()(Trigger* pitem)
	{
		pitem->reset(m_val);
	}

	TriggerVector::TriggerVector() 
	{};

	TriggerVector::~TriggerVector()
	{
		for (unsigned int i = 0; i < size(); i++)
		{
			delete (*this)[i];
		}
	};

	void TriggerVector::addTrigger(Trigger* t)
	{
		push_back(t);
	};

	void TriggerVector::reset(int input)
	{
		std::for_each(this->begin(), this->end(), ResetTriggerFunc(input));
	}

}