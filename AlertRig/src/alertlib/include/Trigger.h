#if !defined(_TRIGGER_H_)
#define _TRIGGER_H_

#include <string>
#include <vector>
#include "VSGV8.h"

// Set this bit in the out_val of a trigger to indicate it should be toggled.
// The given out_val (without the toggle bit) will be the first value triggered.
// After that the value will toggle.
#define AR_TRIGGER_TOGGLE 0x8000

namespace alert
{
	// The trigger class (and its derivatives) perform a specific action upon receipt of a
	// "trigger". Triggers can be ascii characters or bits (in a 32 bit integer).  

	class Trigger
	{
	public:
		Trigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val);
		~Trigger();

		virtual bool checkAscii(std::string input);
		virtual bool checkBinary(int input);
		virtual void setMarker(int& output);
		virtual std::string toString() const;

		// Execute the triggers' action(s). Subclasses should return >0 if a vsgPresent() will be 
		// needed, 0 if no present(), and <0 if this trigger means quit (a call to vsgPresent() is 
		// is made in this case, too -- that way a quitting trigger can set the page to blank
		// or something similarly intelligent. 
		virtual int execute(int& output);

		void reset();


		// reset the trigger's notion of what the last input value was. 
		// This should be called in order to ensure that the first instance of the 
		// trigger will in fact fire the trigger. Remember that in order for the trigger
		// to fire the input must be DIFFERENT than the last time it was called, AND it
		// must match the in_val. Its best to call TriggerVector::reset(), not this
		// directly


		void reset(int input);

		std::string getKey() const;
		int inVal() const;
		int inMask() const;
		int outMask() const;
		int outVal() const;
	protected:
		std::string m_key;
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
	typedef int (*TriggerCallbackFunc)(int &output, const CallbackTrigger* ptrig);

	class CallbackTrigger: public Trigger
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

		virtual bool checkAscii(std::string input);
		virtual bool checkBinary(int input);
		virtual int execute(int& output);
		std::string getKey() const;
		virtual std::string toString() const;
	};


	// Functor technique from http://www.newty.de/fpt/functor.html

	class FunctorCallbackTrigger;	// forward declaration needed

	// Abstract callback base class
	class TFunctor 
	{
	public:
		//virtual int operator()(int &output, const FunctorCallbackTrigger* ptrig)=0;
		virtual int callback(int &output, const FunctorCallbackTrigger* ptrig)=0;
	};



	// Derived template for specific functor that can be used in CallbackFunctorTrigger
   template <class TClass> class TSpecificFunctor : public TFunctor
   {
   private:
      int (TClass::*fpt)(int &output, const FunctorCallbackTrigger* ptrig);   // pointer to member function
      TClass* pt2Object;                  // pointer to object

   public:

      // constructor - takes pointer to an object and pointer to a member and stores
      // them in two private variables
      TSpecificFunctor(TClass* _pt2Object, int(TClass::*_fpt)(int &output, const FunctorCallbackTrigger* ptrig))
         { pt2Object = _pt2Object;  fpt=_fpt; };

      // override operator "()"
//      virtual int operator()(int &output, const FunctorCallbackTrigger* ptrig)
 //      { return (*pt2Object.*fpt)(output, ptrig);};              // execute member function

      // override function "Call"
      virtual int callback(int &output, const FunctorCallbackTrigger* ptrig)
        { return (*pt2Object.*fpt)(output, ptrig);};             // execute member function
   };


   	class FunctorCallbackTrigger: public Trigger
	{
	public:
		FunctorCallbackTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, TFunctor *pfunc);
		~FunctorCallbackTrigger();
		virtual int execute(int& output);
		virtual std::string toString() const;

	protected:
		TFunctor *m_pfunc;
	};






	class PageTrigger: public Trigger
	{
	public:
		PageTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, int i_page);
		~PageTrigger();
		virtual int execute(int& output);
		virtual std::string toString() const;
	protected:
		int m_page;
	};


	class TogglePageTrigger: public Trigger
	{
	public:
		TogglePageTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, int i_pageA, int i_pageB);
		~TogglePageTrigger();
		virtual int execute(int& output);
		virtual std::string toString() const;
	protected:
		int m_pageA;
		int m_pageB;
	};


	class OverlayPageTrigger: public Trigger
	{
	public:
		OverlayPageTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, int i_page);
		~OverlayPageTrigger();
		virtual int execute(int& output);
		virtual std::string toString() const;
	protected:
		int m_page;
	};


	class QuitTrigger: public PageTrigger
	{
	public:
		QuitTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val, int i_page);
		~QuitTrigger();
		virtual int execute(int& output);
		virtual std::string toString() const;
	private:
	};



	class ContrastTrigger: public Trigger, public std::vector< std::pair< VSGOBJHANDLE, int > >
	{
	public:
		ContrastTrigger(std::string i_key, int i_in_mask, int i_in_val, int i_out_mask, int i_out_val);
		~ContrastTrigger();
		virtual int execute(int& output);
		virtual std::string toString() const;
	};



	// callback for page cycling trigger
	class PageCyclingTrigger;
	typedef int (*PageCyclingTriggerCallbackFunc)(int icycle);

	class PageCyclingTrigger: public Trigger
	{
	public:
		PageCyclingTrigger(std::string i_key, int n_repeats, PageCyclingTriggerCallbackFunc callback = NULL);
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


	// callback for file monitor trigger
	class FileChangedTrigger;
	typedef int (*FileChangedTriggerCallbackFunc)(bool created, bool deleted);


	// If an output trigger value and mask are supplied, then the output is fired when this trigger executes. 
	// We have to do this because there is no present() issued by this trigger. 
	// That could lead to problems if multiple triggers will fire at once, I suppose. 
	class FileChangedTrigger: public Trigger
	{
	public:
		FileChangedTrigger(std::string i_key, std::string filename, FileChangedTriggerCallbackFunc callback = NULL, int i_out_mask = 0, int i_out_val = 0);
		~FileChangedTrigger();

		virtual bool checkAscii(std::string input);
		virtual bool checkBinary(int input);
		virtual int execute(int& output);
	private:
		void init();
	protected:
		std::string m_filename;
		bool m_exists;
		bool m_bCreated;
		bool m_bDeleted;
		FileChangedTriggerCallbackFunc m_callback;
		struct _stat m_fileinfo;
	};


	class ResetTriggerFunc
	{
	public:
		ResetTriggerFunc(int val);
		~ResetTriggerFunc();
		void operator()(Trigger* pitem);
		int m_val;
	};



	class TriggerFunc
	{
	public:
		TriggerFunc(std::string key, int otrigger, bool verbose = false);
		TriggerFunc(int itrigger, int otrigger, bool verbose = false);

		int page();
		bool present();
		int output_trigger();
		bool quit();
		int deferred();
		const std::string& triggers_matched();

		virtual void operator()(Trigger* pitem);
	protected:
		bool m_quit;
		bool m_binary;	// if true, do a binary trigger test. Otherwise do ascii
		int m_itrigger;	// input trigger value to test against
		std::string m_skey;
		bool m_present;	// if true, at least one trigger requires vsgPresent()
		int m_otrigger;	// if m_present is true, this is the output trigger value
		int m_page;
		int m_ideferred;	// flag set to indicate deferred processing of some sort is needed.
		bool m_verbose;
		std::string m_triggers_matched;
	};



	class TriggerVector: public std::vector<Trigger*>
	{
	public:
		TriggerVector();
		virtual ~TriggerVector();
		void addTrigger(Trigger* t);
		void reset(int input);
	};

};

#endif