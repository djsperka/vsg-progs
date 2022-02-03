#include "alert-triggers.h"
#include <iostream>
#include <sstream>

using namespace alert;

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

	// Check if the input value will be toggled. 
	if (i_in_val & AR_TRIGGER_ASCII_ONLY)
	{
		m_btoggleIn = false;
		m_in_val = 0;
	}
	else
	{
		if (i_in_val & AR_TRIGGER_TOGGLE)
		{
			//				std::cout << "Trigger(" << m_key << ")" << " TOGGLE(input) " << m_key << " in_val " << i_in_val << std::endl;
			m_btoggleIn = true;
			m_in_val = i_in_val & ~(AR_TRIGGER_TOGGLE);
		}
		else
		{
			m_btoggleIn = false;
			m_in_val = i_in_val;
		}
	}
	reset();	// this gives the m_in_val a value.

};

Trigger::~Trigger() {};

bool Trigger::checkAscii(const std::string& input)
{
	return checkString(input);
};

bool Trigger::checkString(const std::string& input)
{
	bool b = false;
	size_t ind;
	m_args.clear();
	if (input.size() == 1)
	{
		ind = input.find_first_of(getKey());
		if (ind != std::string::npos)
		{
			b = true;
			m_matchedKey = input[ind];
		}
	}
	else
	{
		// split into everything before first blank, and everything after (trimmed). 
		std::string token = input.substr(0, input.find(' '));
		ind = token.find_first_of(getKey());

		//std::cerr << "token " << token << " getKey() " << getKey() << " ind " << ind << std::endl;
		if (ind != std::string::npos)
		{
			b = true;
			m_matchedKey = input[ind];
			m_args = boost::trim_left_copy(input.substr(input.find(' ')));
		}
	}
	if (b)
		std::cout << "input " << input << " key " << getKey() << " matched " << m_matchedKey << std::endl;
	//else
	//	std::cout << "input " << input << " key " << getKey() << " not matched " << std::endl;

	return b;
}

bool Trigger::checkBinary(int input)
{
	bool bValue = false;
	int current = input & m_in_mask;
	m_args.clear();
	m_matchedKey.clear();
	// djs - create trigger with 
	if (m_in_mask == 0) return false;
	if (!m_btoggleIn)
	{
		//std::cout << getKey() << " " << std::hex << m_in_mask << " " << m_in_val << " " << m_in_last << " " << current << std::endl;
		if (current != m_in_last && current == m_in_val)
		{
			bValue = true;
			m_matchedKey = getKey();
		}
		m_in_last = current;
	}
	else
	{
		// We expect the trigger to be the last trigger toggled. If a trigger covers more than 
		// one bit, then all must be inverted. Note that the FIRST trigger expected is the value
		// given as i_in_val (without the AR_TRIGGER_TOGGLE bit). Subsequent triggers are expected
		// to be toggled. 
		if (((~current) & m_in_mask) == m_in_last)
		{
			bValue = true;
			m_matchedKey = getKey();
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
			output = m_out_val | (output & (~m_out_mask));
			//					std::cout << "setMarker: output(out)=" << output << std::endl;
		}
	}
	else
	{
		int temp;
		temp = ~m_last_out_val & m_out_mask;
		output = temp | (output & (~m_out_mask));
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


const std::string& Trigger::getKey() const { return m_key; };
const std::string& Trigger::getArgs() const { return m_args; };
const std::string& Trigger::getMatchedKey() const { return m_matchedKey; };

//========================================

CallbackTrigger::CallbackTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, TriggerCallbackFunc tcf) :
	Trigger(i_key, i_in_mask, i_in_val, i_out_mask, i_out_val), m_callback(tcf) {};

CallbackTrigger::~CallbackTrigger() {};

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

//==================================================
#if 0
MISOFunctorCallbackTrigger::MISOFunctorCallbackTrigger(std::vector<std::pair<std::string, int> >& v, int i_in_mask, int i_out_mask, int i_out_val, TFunctor* pfunc) :
	std::vector< std::pair < std::string, int> >(v), FunctorCallbackTrigger("", i_in_mask, 0, i_out_mask, i_out_val, pfunc), m_input_matched(-1)
{
	// form key using each char from the consituent triggers.
	for (auto p : *this)
		m_key.append(p.first);
	std::cerr << "MISOC using key " << m_key << std::endl;
};

MISOFunctorCallbackTrigger::~MISOFunctorCallbackTrigger() {};

bool MISOFunctorCallbackTrigger::checkBinary(int input)
{
	unsigned int i;
	bool bValue = false;
	int current = input & m_in_mask;

	m_input_matched = -1;
	m_matchedKey.clear();
	if (!m_btoggleIn)
	{
		if (current != m_in_last)
		{
			for (i = 0; i < this->size(); i++)
			{
				if ((*this)[i].second & AR_TRIGGER_ASCII_ONLY)
					continue;
				if (current == (*this)[i].second)
				{
					m_input_matched = i;
					m_matchedKey = (*this)[i].first;
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
		std::cerr << "Cannot use toggled input trigger for MISOFunctorCallbackTrigger" << std::endl;
		bValue = false;
	}

	return bValue;
};

std::string MISOFunctorCallbackTrigger::getKey() const
{
	//			std::cout << "getKey(): m_input_matched = " << m_input_matched << std::endl;
	if (m_input_matched < 0 || m_input_matched >(int)this->size()) return "ERROR";
	else return (*this)[m_input_matched].first;
};

std::string MISOFunctorCallbackTrigger::toString() const
{
	unsigned int i;
	std::ostringstream oss;
	oss << "MISOFunctorCallback " << std::endl;
	for (i = 0; i < this->size(); i++)
	{
		oss << "         " << (*this)[i].first << " in m/v/t: 0x" << std::hex << m_in_mask << "/0x" << (*this)[i].second << "/" << m_btoggleIn << " out m/v/t: 0x" << m_out_mask << "/0x" << m_out_val << "/" << m_btoggleOut;
		if (i < (this->size() - 1)) oss << std::endl;
	}
	return oss.str();
}

#else

MISOFunctorCallbackTrigger::MISOFunctorCallbackTrigger(std::vector<std::pair<std::string, int> >& v, int i_in_mask, int i_out_mask, int i_out_val, TFunctor* pfunc)
	: Trigger("", i_in_mask, 0, i_out_mask, i_out_val)
	, m_pMatchedTrigger(nullptr)
{
	for (auto p : v)
	{
		m_trigs.push_back(new Trigger(p.first, i_in_mask, p.second, i_out_mask, i_out_val));
		m_key.append(p.first);	// key becomes a list of all the keys in the vector
	}
}
MISOFunctorCallbackTrigger::~MISOFunctorCallbackTrigger() {};

bool MISOFunctorCallbackTrigger::checkString(const std::string& input)
{
	bool b = false;
	m_pMatchedTrigger = nullptr;
	// Check each in sequence. If a trigger is fired, do not check any others. 
	for (auto t : m_trigs)
	{
		if (t.checkString(input))
		{
			// save first trig that fires
			if (!b)
			{
				m_pMatchedTrigger = &t;
				b = true;
			}
		}
	}
	return b;
}

bool MISOFunctorCallbackTrigger::checkBinary(int input)
{
	bool b = false;
	m_pMatchedTrigger = nullptr;
	// check each trigger in sequence. 
	std::cerr << " MISOFunctorCallbackTrigger::checkBinary" << std::endl;
	for (auto t : m_trigs)
	{
		std::cerr << t << std::endl;
		if (t.checkBinary(input))
		{
			std::cerr << "true" << std::endl;
			// save first trig that fires
			if (!b)
			{
				m_pMatchedTrigger = &t;
				b = true;
			}
		}
		else
			std::cerr << "true" << std::endl;
	}
	return b;
}

std::string MISOFunctorCallbackTrigger::toString() const
{
	std::ostringstream oss;
	for (auto t : m_trigs)
		oss << t.toString() << std::endl;
	return oss.str();
}

int MISOFunctorCallbackTrigger::execute(int& output)
{
	if (m_pMatchedTrigger)
		return m_pMatchedTrigger->execute(output);
	else
		return 0;
}

void MISOFunctorCallbackTrigger::reset(int input)
{
	for (auto t : m_trigs)
		t.reset(input);
}


#endif
//=====================================

FunctorCallbackTrigger::FunctorCallbackTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, TFunctor* pfunc) :
	Trigger(i_key, i_in_mask, i_in_val, i_out_mask, i_out_val), m_pfunc(pfunc) {};
FunctorCallbackTrigger::~FunctorCallbackTrigger() {};
int FunctorCallbackTrigger::execute(int& output)
{
	setMarker(output);
	return m_pfunc->callback(output, this, getArgs());
};

std::string FunctorCallbackTrigger::toString() const
{
	std::ostringstream oss;
	oss << "Callback " << m_key << " in m/v/t: 0x" << std::hex << m_in_mask << "/0x" << m_in_val << "/" << m_btoggleIn << " out m/v/t: 0x" << m_out_mask << "/0x" << m_out_val << "/" << m_btoggleOut;
	return oss.str();
}

//================================



std::ostream& alert::operator<<(std::ostream& out, const Trigger& t)
{
	out << t.toString();
	return out;
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

//===============================

PageTrigger::PageTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, int i_page) :
	Trigger(i_key, i_in_mask, i_in_val, i_out_mask, i_out_val), m_page(i_page) {};
PageTrigger::~PageTrigger() {};
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

//===================================

QuitTrigger::QuitTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, int i_page) :
	PageTrigger(i_key, i_in_mask, i_in_val, i_out_mask, i_out_val, i_page) {};
QuitTrigger::~QuitTrigger() {};
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

//==========================

TriggerFunc& TriggerFunc::operator=(const TriggerFunc& tf)
{
	if (&tf != this)
	{
		m_quit = tf.m_quit;
		m_fired = tf.m_fired;
		m_binary = tf.m_binary;
		m_itrigger = tf.m_itrigger;
		m_skey = tf.m_skey;
		m_count = tf.m_count;
		m_present = tf.m_present;
		m_otrigger = tf.m_otrigger;
		m_page = tf.m_page;
	}
	return *this;
}

void TriggerFunc::operator()(Trigger* pitem)
{
	bool bTest = false;
	m_count++;

	// This trigger func fires only on the first trigger! 
	// Each trigger is checked, however, so that each can keep track of the current input. 

	if (m_binary) bTest = pitem->checkBinary(m_itrigger);
	else bTest = pitem->checkString(m_skey);

	if (bTest && !fired())
	{
		int i;
		m_fired = true;
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
};
