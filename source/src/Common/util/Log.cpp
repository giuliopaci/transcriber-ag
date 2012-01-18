/*
*  @class Log
*  basic logging for DACTILO services
*/

#include <fstream>
#include "Log.h"
#include "FormatTime.h"


using namespace std;

#ifdef _WIN32
#define DEVNUL "NUL"
#else
#define DEVNUL "/dev/null"
#endif

ostream* Log::_out = &cout;
ostream* Log::_err = &cerr;
ostream* Log::_null = NULL;
bool  Log::_firstTry = true;
bool  Log::_printTimestamp = false;

Log::Level Log::_prevLevel = Log::MEDIUM;
Log::Level Log::_traceLevel = Log::MEDIUM;


ostream& Log::timeStamp(ostream& os) {
	if ( _printTimestamp ) {
		os << FormatTime::formatDate(time(NULL), FormatTime::FMT_ISO8601) << ": ";
	}
	return os;
}

ostream& Log::getNull() {
	if ( _null == NULL ) {
		if ( Log::_firstTry ) {
			Log::_firstTry = false;
			ofstream* fo = new ofstream(DEVNUL);
			if ( fo->good() ) {
				Log::_null = fo;
				return *Log::_null;
			} else
				cerr << "Open " << DEVNUL << "Failed." << endl;
		}
		return *Log::_err;
	}
	return *Log::_null;
}

void Log::redirect(string prefix, bool distinct_err ) throw (const char*)
{
	string where = "(Log::redirect) ";

	close();
	getNull();

	if ( prefix == DEVNUL ) {
		if ( Log::_null != NULL ) {
			Log::_out = Log::_err = Log::_null;
		}
	} else {
		if ( prefix.length() > 4 ) {
			string tail = prefix.substr(prefix.length()-4);
			if ( tail == ".log" ) prefix.erase(prefix.length()-4, 4);
		}
		string log_out = prefix + ".log";
		ofstream* fo = new ofstream(log_out.c_str());
		if ( ! fo->good() ) {
			string err = where + log_out + " : Can't open file for writing";
			throw err.c_str();
		}
		Log::_out = fo;

		if ( distinct_err ) {
			string log_err = prefix+".trace";
			ofstream* fe = new ofstream(log_err.c_str());
			if ( ! fe->good() ) {
				string err = where + log_err + " : Can't open file for writing";
				throw err.c_str();
			}
			Log::_err = fe;
		} else
			Log::_err = fo;
	}
}

void Log::close()
{
	if ( Log::_out != &cout && Log::_out != Log::_err ) {
		ofstream* fo = dynamic_cast<ofstream*>(Log::_out);
		if ( fo != NULL ) {
			fo->close();
			delete fo;
		}
		Log::_out = &cout;
	}
	if ( Log::_err != &cerr ) {
		ofstream* fo = dynamic_cast<ofstream*>(Log::_err);
		if ( fo != NULL ) {
			fo->close();
			delete fo;
		}
		Log::_err = &cerr;
	}

	if ( Log::_null != NULL ) {
		ofstream* fo = dynamic_cast<ofstream*>(Log::_null);
		if ( fo != NULL ) {
			fo->close();
			delete fo;
		}
		Log::_null = NULL;
		Log::_firstTry = true;
	}
}
