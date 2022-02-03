#pragma once

#include "ARtypes.h"
#include <boost/algorithm/string.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

// The trigger class (and its derivatives) perform a specific action upon receipt of a
// "trigger". Triggers can be ascii characters or bits (in a 32 bit integer).  

namespace alert
{

	class Trigger
	{
	public:
		Trigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val);
		~Trigger();
		virtual bool checkString(const std::string& input);
		virtual bool checkAscii(const std::string& input);
		virtual bool checkBinary(int input);
		virtual void setMarker(int& output);
		virtual std::string toString() const;

		// Execute the triggers' action(s). Subclasses should return >0 if a vsgPresent() will be 
		// needed, 0 if no present(), and <0 if this trigger means quit (a call to vsgPresent() is 
		// is made in this case, too -- that way a quitting trigger can set the page to blank
		// or something similarly intelligent. 
		virtual int execute(int& output);

		// reset the trigger's notion of what the last input value was. 
		// This should be called in order to ensure that the first instance of the 
		// trigger will in fact fire the trigger. Remember that in order for the trigger
		// to fire the input must be DIFFERENT than the last time it was called, AND it
		// must match the in_val. Its best to call TriggerVector::reset(), not this
		// directly


		virtual void reset(int input = 0);
		const std::string& getKey() const;

		// these are valid after a call to checkAscii, checkString, or checkBinary which 
		// returned true. 
		const std::string& getMatchedKey() const;
		const std::string& getArgs() const;

	protected:
		std::string m_key;
		std::string m_args;
		std::string m_matchedKey;
		int m_in_mask;
		int m_in_val;
		int m_in_last;	// last value of this trigger's input&m_in_mask (initial = 0) 
		int m_out_mask;
		int m_out_val;
		int m_last_out_val;
		bool m_btoggleOut;
		bool m_btoggleIn;
	};

	std::ostream& operator<<(std::ostream& out, const alert::Trigger& t);


	// callback function type for CallbackTrigger
	class CallbackTrigger;
	typedef int (*TriggerCallbackFunc)(int& output, const CallbackTrigger* ptrig);

	class CallbackTrigger : public Trigger
	{
	public:
		CallbackTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, TriggerCallbackFunc tcf);
		~CallbackTrigger();
		virtual int execute(int& output);
		virtual std::string toString() const;
	protected:
		TriggerCallbackFunc m_callback;
	};



	class MultiInputSingleOutputCallbackTrigger : public CallbackTrigger, public std::vector< std::pair< std::string, int > >
	{
	private:
		int m_input_matched;	// index of input key/value matched in a call to checkAscii/checkBinary

	public:
		MultiInputSingleOutputCallbackTrigger(std::vector<std::pair<std::string, int> >& v, int i_in_mask, int i_out_mask, int i_out_val, TriggerCallbackFunc tcf);
		~MultiInputSingleOutputCallbackTrigger();
		virtual bool checkBinary(int input);
		virtual int execute(int& output);
		std::string getKey() const;
		virtual std::string toString() const;

	};


	// Functor technique from http://www.newty.de/fpt/functor.html

	class FunctorCallbackTrigger;	// forward declaration needed

	// Abstract callback base class. 
	class TFunctor
	{
	public:
		virtual int callback(int& output, const FunctorCallbackTrigger* ptrig, const std::string& args) = 0;
	};



	// Derived template for specific functor that can be used in CallbackFunctorTrigger
	template <class TClass> class TSpecificFunctor : public TFunctor
	{
	private:
		int (TClass::* fpt)(int& output, const FunctorCallbackTrigger* ptrig, const std::string& args);   // pointer to member function
		TClass* pt2Object;                  // pointer to object

	public:

		// constructor - takes pointer to an object and pointer to a member and stores
		// them in two private variables
		TSpecificFunctor(TClass* _pt2Object, int(TClass::* _fpt)(int& output, const FunctorCallbackTrigger* ptrig, const std::string& args))
		{
			pt2Object = _pt2Object;  fpt = _fpt;
		};

		// override function "Call"
		virtual int callback(int& output, const FunctorCallbackTrigger* ptrig, const std::string& args)
		{
			return (*pt2Object.*fpt)(output, ptrig, args);
		};             // execute member function
	};


	class FunctorCallbackTrigger : public Trigger
	{
	public:
		FunctorCallbackTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, TFunctor* pfunc);
		~FunctorCallbackTrigger();
		virtual int execute(int& output);
		virtual std::string toString() const;
	protected:
		TFunctor* m_pfunc;
	};

#if 0
	class MISOFunctorCallbackTrigger : public FunctorCallbackTrigger, public std::vector< std::pair< std::string, int > >
	{
	private:
		int m_input_matched;	// index of input key/value matched in a call to checkAscii/checkBinary

	public:
		MISOFunctorCallbackTrigger(std::vector<std::pair<std::string, int> >& v, int i_in_mask, int i_out_mask, int i_out_val, TFunctor* pfunc);
		~MISOFunctorCallbackTrigger();
		virtual bool checkBinary(int input);
		std::string getKey() const;
		virtual std::string toString() const;
	};
#else


	class MISOFunctorCallbackTrigger : public Trigger
	{
	private:
		boost::ptr_vector<alert::Trigger> m_trigs;
		alert::Trigger* m_pMatchedTrigger;

	public:
		MISOFunctorCallbackTrigger(std::vector<std::pair<std::string, int> >& v, int i_in_mask, int i_out_mask, int i_out_val, TFunctor* pfunc);
		~MISOFunctorCallbackTrigger();
		virtual bool checkString(const std::string& input);
		virtual bool checkBinary(int input);
		virtual std::string toString() const;
		virtual int execute(int& output);
		virtual void reset(int input);

	};

#endif



	class PageTrigger : public Trigger
	{
	public:
		PageTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, int i_page);
		~PageTrigger();
		virtual int execute(int& output);
		virtual std::string toString() const;
	protected:
		int m_page;
	};



	class QuitTrigger : public PageTrigger
	{
	public:
		QuitTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, int i_page);
		~QuitTrigger();
		virtual int execute(int& output);
		virtual std::string toString() const;
	private:
	};


	// callback for page cycling trigger
	class PageCyclingTrigger;
	typedef int (*PageCyclingTriggerCallbackFunc)(int icycle);

	class PageCyclingTrigger : public Trigger
	{
	public:
		PageCyclingTrigger(std::string i_key, int n_repeats, PageCyclingTriggerCallbackFunc callback = NULL) :
			Trigger(i_key, 0, 0, 0, 0), m_nrepeats(n_repeats), m_repeat_count(0), m_is_started(false), m_start_pending(false), m_callback(callback) {};
		~PageCyclingTrigger() {};
		virtual bool checkAscii(std::string input);
		virtual bool checkBinary(int input);
		void started();
		void stopped();
		virtual int execute(int& output);

	protected:
		int m_nrepeats;
		int m_repeat_count;
		bool m_is_started;
		bool m_start_pending;
		PageCyclingTriggerCallbackFunc m_callback;
	};


	class ResetTriggerFunc
	{
	public:
		ResetTriggerFunc(int val) : m_val(val) {};
		~ResetTriggerFunc() {};
		void operator()(Trigger* pitem)
		{
			pitem->reset(m_val);
		}
		int m_val;
	};


	// Functor class used to evaluate triggers. 
	// This functor is written such that it will stop evaluating triggers once one fires -- it is intended to be used with a vector of triggers, 
	// which are sent through std::for_each to this functor. Triggers should be ordered so the right one fires first when their bits overlap!

	class TriggerFunc
	{
	public:
		TriggerFunc(std::string key, int otrigger, bool verbose = false) : m_quit(false), m_fired(false), m_binary(false), m_skey(key), m_count(0), m_present(false), m_otrigger(otrigger), m_page(-1), m_ideferred(0), m_verbose(verbose) {};
		TriggerFunc(int itrigger, int otrigger, bool verbose = false) : m_quit(false), m_fired(false), m_binary(true), m_skey(""), m_count(0), m_itrigger(itrigger), m_present(false), m_otrigger(otrigger), m_page(-1), m_ideferred(0), m_verbose(verbose) {};
		TriggerFunc() : m_quit(false), m_fired(false), m_binary(false), m_count(0), m_skey(""), m_present(false), m_otrigger(0), m_page(-1), m_ideferred(0), m_verbose(false) {};
		TriggerFunc(const TriggerFunc& tf) : m_quit(tf.m_quit), m_fired(tf.m_fired), m_binary(tf.m_binary), m_itrigger(tf.m_itrigger), m_count(tf.m_count), m_skey(tf.m_skey), m_present(tf.m_present), m_otrigger(tf.m_otrigger), m_page(tf.m_page) {};
		int page() { return m_page; };
		bool present() { return m_present; };
		bool fired() { return m_fired; }
		int output_trigger() { return m_otrigger; };
		bool quit() { return m_quit; };
		int deferred() { return m_ideferred; };
		int count() { return m_count; }
		const std::string& triggers_matched() { return m_triggers_matched; };

		TriggerFunc& operator=(const TriggerFunc& tf);

		virtual void operator()(Trigger* pitem);

	protected:
		bool m_quit;
		bool m_fired;	// becomes true when a trigger is fired, will no fire again!!!
		bool m_binary;	// if true, do a binary trigger test. Otherwise do ascii
		int m_itrigger;	// input trigger value to test against
		std::string m_skey;
		int m_count;	// number of triggers processed. 
		bool m_present;	// if true, at least one trigger requires vsgPresent()
		int m_otrigger;	// if m_present is true, this is the output trigger value
		int m_page;
		int m_ideferred;	// flag set to indicate deferred processing of some sort is needed.
		bool m_verbose;
		std::string m_triggers_matched;
	};


	class MasterSlaveTriggerFunc : public TriggerFunc
	{
	public:
		MasterSlaveTriggerFunc(std::string key, int otrigger) : TriggerFunc(key, otrigger) {};
		MasterSlaveTriggerFunc(int itrigger, int otrigger) : TriggerFunc(itrigger, otrigger) {};

		// This func differs from regular TriggerFunc's method in the handling of m_ideferred - because I'm scared
		// of breaking apps that use negative values in return values, or those where multiple triggers fire. 
		virtual void operator()(Trigger* pitem)
		{
			bool bTest = false;
			if (m_binary) bTest = pitem->checkBinary(m_itrigger);
			else bTest = pitem->checkAscii(m_skey);

			if (bTest)
			{
				int i;
				i = pitem->execute(m_otrigger);
				m_ideferred |= i;
				if (i > 0) m_present = true;
				else if (i < 0)
				{
					m_present = true;
					m_quit = true;
				}
			}
		};
	};



	class TriggerVector : public std::vector<Trigger*>
	{
	public:
		TriggerVector() {};
		virtual ~TriggerVector()
		{
			for (unsigned int i = 0; i < size(); i++)
			{
				delete (*this)[i];
			}
		};
		void addTrigger(Trigger* t)
		{
			push_back(t);
		};

		void reset(int input)
		{
			std::for_each(this->begin(), this->end(), ResetTriggerFunc(input));
		}
	};
};	// namespace


