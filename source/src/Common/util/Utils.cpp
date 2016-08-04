/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include <string.h>

#include "Utils.h"
#include "Common/util/StringOps.h"
#include "Common/FileInfo.h"
#include "Common/globals.h"


static double my_exp10f(int n)
{
#if (defined WIN32) || (defined __APPLE__)
	double res=1. ;
	for ( int i=0; i < n; ++i) res *= 10. ;
	return res ;
#else
	return exp10f(n) ;
#endif
}

long get_time_millisec()
{
  struct timeval t;
  gettimeofday(&t, NULL);

  return (t.tv_sec*1000 + t.tv_usec/1000);
}

void* my_calloc( int nb, int sz, const char* f, int l, int trace_alloc)
{
	void*p = calloc(nb, sz);
	if ( trace_alloc )
		fprintf(stderr, "%s:%d ALLOC 0x%08x \n", f, l, p); fflush(stderr);
	return p;
}

void my_free(void** p, const char* f, int l, int trace_alloc)
{
	if (*p!=NULL) {
		if (trace_alloc)
			fprintf(stderr, "%s:%d FREE 0x%08x \n", f, l, *p); fflush(stderr);
		free(*p);
		*p=NULL;
	}
}

float my_atof(const char* s)
{
	float	f;
	int		n;
	char*	pt;

	f = (float) strtol(s, &pt, 10);

	if ( (*pt == '.' || *pt == ',') && (n=strlen(pt+1)) > 0 )
	{
		f += ((float)strtol(pt+1, NULL, 10) / my_exp10f(n));
	}
	return f;
}

bool is_in_list(const std::list<string>& list, Glib::ustring value)
{
	std::list<string>::const_iterator it ;
	for (it=list.begin(); it!=list.end(); it++) {
		if (value.compare(*it)==0)
			return true ;
	}
	return false ;
}

bool is_in_svect(const std::vector<Glib::ustring>& vect, Glib::ustring value)
{
	std::vector<Glib::ustring>::const_iterator it ;
	for (it=vect.begin(); it!=vect.end(); it++) {
		if (value.compare(*it)==0)
			return true ;
	}
	return false ;
}

bool is_in_svect(const std::vector<std::string>& vect, Glib::ustring value)
{
	std::vector<string>::const_iterator it ;
	for (it=vect.begin(); it!=vect.end(); it++) {
		if (value.compare(*it)==0)
			return true ;
	}
	return false ;
}

Glib::ustring replace_in_string(Glib::ustring data, Glib::ustring toBeReplaced, Glib::ustring replace)
{
	guint pos = 0 ;
	int size = toBeReplaced.size() ;
	pos = data.find(toBeReplaced) ;
	while ( pos!= string::npos ) {
		data.replace(pos, size, replace) ;
		pos = data.find(toBeReplaced) ;
	}
	return data ;
}

int is_valid_filename(Glib::ustring file_name)
{
	guint max = 256 ;
	int res = 1 ;

	//> CHECK NULL
	if( file_name.compare("")==0 ||  file_name.compare(" ")==0 )
		res=-1 ;
	//> CHECK LENGTH
	else if (file_name.length() >= max)
		res=-2 ;
	//> CHECK CHAR
	bool correct = StringOps::check_string(STRING_OPS_FORBIDDEN_FILE, file_name) ;
	if (correct && res>0)
		res = 1 ;
	else if (!correct)
		res = -3 ;

	return res ;
}

void print_map_s(const std::map<string,string>& map, string display)
{
	TRACE << "\n############################# Print " << display << std::endl ;
	std::map<string,string>::const_iterator it ;
	for (it=map.begin(); it!=map.end(); it++)
		TRACE << "\t\t key= " << it->first << " - value=" << it->second << std::endl ;
	TRACE << "Print MAP #############################\n"  << std::endl ;
}

void print_map_s(const std::map<Glib::ustring,Glib::ustring>& map, string display)
{
	TRACE << "\n############################# Print " << display << std::endl ;
	std::map<Glib::ustring,Glib::ustring>::const_iterator it ;
	for (it=map.begin(); it!=map.end(); it++)
		TRACE << "\t\t key= " << it->first << " - value=" << it->second << std::endl ;
	TRACE << "Print MAP #############################\n"  << std::endl ;
}

void print_map_s(const std::map<string,int>& map, string display)
{
	TRACE << "\n############################# Print " << display << std::endl ;
	std::map<string,int>::const_iterator it ;
	for (it=map.begin(); it!=map.end(); it++)
		TRACE << "\t\t key= " << it->first << " - value=" << it->second << std::endl ;
	TRACE << "Print MAP #############################\n"  << std::endl ;
}


void print_vector_s(const std::vector<string>& vect, string display)
{
	TRACE << "\n############################# Print " << display << std::endl ;
	std::vector<string>::const_iterator it ;
	for (it=vect.begin(); it!=vect.end(); it++)
		TRACE << "\t\t value= " << *it << std::endl ;
	TRACE << "Print VECTOR #############################\n"  << std::endl ;
}

void print_vector_s(const std::vector<Glib::ustring>& vect, string display)
{
	TRACE << "\n############################# Print " << display << std::endl ;
	std::vector<Glib::ustring>::const_iterator it ;
	for (it=vect.begin(); it!=vect.end(); it++)
		TRACE << "\t\t value= " << *it << std::endl ;
	TRACE << "Print VECTOR #############################\n"  << std::endl ;
}


// ugly but works
// no existence control
std::map<std::string, std::string> merge_map_s(const std::map<std::string, std::string>& map1,
													const std::map<std::string, std::string>& map2 )
{
	std::map<std::string, std::string> res ;
	std::map<std::string, std::string>::const_iterator it ;
	for (it=map1.begin(); it!=map1.end(); it++)
		res[it->first] = it->second ;
	for (it=map2.begin(); it!=map2.end(); it++)
		res[it->first] = it->second ;
	return res ;
}

void merge_vector_s(std::vector<std::string>& v1, const std::vector<std::string>& v2, bool unique)
{
	std::vector<std::string>::const_iterator it ;
	for (it=v2.begin(); it!=v2.end(); it++)
	{
		if ( (find(v1.begin(), v1.end(), *it)==v1.end() ) || !unique )
			v1.push_back(*it) ;
	}
}

std::string remove_prefix_s(std::string s, std::string sub)
{
	if (sub.size()>=s.size())
		return "" ;

	guint pos = s.find(sub) ;
	if (pos == string::npos || pos != 0)
		return "" ;
	else {
		int start = pos+sub.size() ;
		int size = s.size() - sub.size() ;
		string res = "" ;
		if (size>0)
			res = s.substr(start, size) ;
		return res ;
	}
}

std::map<string,string> fromDCSVstringTOmap(std::string str)
{
	std::map<string,string> res ;
	StringOps ops(str) ;
	ops.getDCSVItems(res) ;
	return res ;
}

string fromDCSVmapTOstring(const std::map<string,string>& map)
{
	StringOps ops("") ;
	string res = ops.makeDCSVStr(map) ;
	return res ;
}

int my_roundf(double myfloat)
{
	char tmp[50] ;
	sprintf(tmp, "%0.f", myfloat) ;
	int number = my_atof(tmp) ;
	return number ;
}

/** get a string composed of elements separated by personnal separator, and return in vector
 *  each element
 */
int mini_parser(char separator, Glib::ustring str, std::vector<Glib::ustring>* vector )
{
	int res = - 1 ;
	vector->clear() ;
	guint last = 0 ;

	bool cut = false ;

	if (str!="") {
		for (guint i=0; i<str.size(); i++) {
			if ( str[i]==separator ) {
				cut = true ;
				vector->insert(vector->end(), str.substr(last,i-last) );
				last=i+1 ;
			}
		}
		if (cut)
			vector->insert(vector->end(), str.substr(last,str.size()-1) );
		else
			vector->insert(vector->end(), str.substr(last,str.size()) );
		res = 1 ;
	}
	else {
		Log::err() << "mini_parser <information:> no string value" << std::endl ;
	}
	return res ;
}

/** get a string composed of elements separated by personnal separator, and return in vector
 *  each element
 */
int mini_parser(char separator, Glib::ustring str, std::vector<string>* vector )
{
	int res = - 1 ;
	vector->clear() ;
	guint last = 0 ;

	bool cut = false ;

	if (str!="") {
		for (guint i=0; i<str.size(); i++) {
			if ( str[i]==separator ) {
				cut = true ;
				vector->insert(vector->end(), str.substr(last,i-last) );
				last=i+1 ;
			}
		}
		if (cut)
			vector->insert(vector->end(), str.substr(last,str.size()-1) );
		else
			vector->insert(vector->end(), str.substr(last,str.size()) );
		res = 1 ;
	}
	else {
		Log::err() << "mini_parser <information:> no string value" << std::endl ;
	}
	return res ;
}

/** get a string composed of elements separated by personnal separator, and return in vector
 *  each element
 */
int string2set(std::string str, std::set<std::string>* set )
{
	int res = - 1 ;
	char separator(';') ;
	set->clear() ;
	guint last = 0 ;

	bool cut = false ;

	if (str!="")
	{
		for (guint i=0; i<str.size(); i++)
		{
			if ( str[i]==separator )
			{
				cut = true ;
				set->insert(str.substr(last,i-last) );
				last=i+1 ;
			}
		}
		if (cut)
			set->insert(str.substr(last,str.size()-1) );
		else
			set->insert(str.substr(last,str.size()) );
		res = 1 ;
	}
	else
		Log::err() << "mini_parser <information:> no string value" << std::endl ;
	return res ;
}

std::string set2string(std::set<std::string> set)
{
	std::string result = "" ;
	if (set.empty())
		return result ;

	std::set<std::string>::iterator it ;
	for (it=set.begin(); it!=set.end(); it++)
	{
		if (!result.empty() && !(*it).empty())
			result = result + ";" ;
		result = result + *it ;
	}
	return result ;
}


string real_regular_path(string path)
{
	if (!g_path_is_absolute(path.c_str()))
		return "" ;

	if (!g_file_test(path.c_str(), G_FILE_TEST_EXISTS))
		return "" ;

	// compute realpath
	FileInfo info(path) ;
	path = info.realpath() ;

	// remove symlink
	string base = "" ;
	string dir = path ;
	string symdone = "" ;
	std::vector<string> bases ;

	// for each step in path check if we're a symlink
	while (base!=dir && !dir.empty() && symdone.empty())
	{
		// keep all basenames we skip
		if (!base.empty())
			bases.insert(bases.begin(), base) ;
		// if we have a symlink, resolve it
		if ( g_file_test(dir.c_str(), G_FILE_TEST_IS_SYMLINK) ) {
			GError** error ;
			dir = g_file_read_link(dir.c_str(), error) ;
			symdone = dir ;
		}
		// prepare for next loop
		base = g_path_get_basename(dir.c_str()) ;
		dir = g_path_get_dirname(dir.c_str()) ;

		#ifdef WIN32
			if (base == "\\")
				base = dir;
		#endif
	}

	if (!symdone.empty())
	{
		std::vector<string>::iterator it ;
		path = symdone ;
		// reconstruct path from symdone with all skipped basenames
		for (it=bases.begin(); it!=bases.end(); it++) {
			FileInfo info(path) ;
			path = std::string(info.join(*it)) ;
		}
		// check if the newly path really exist (if not, we've done an error)
		if (!g_file_test(path.c_str(), G_FILE_TEST_EXISTS))
			path = "" ;
	}
	return path ;
}

void print_trace(Glib::ustring s, int mode)
{
	if (mode==1)
		Log::out() << s << std::endl ;
	else if (mode==0)
		Log::err() << s << std::endl ;
}

void mySleep(float time)
{
	Glib::Timer tim ; 
	tim.start() ;
	while( tim.elapsed() < time ) ;
}

void deleteAndNull(void** pointer)
{
	if (!pointer || !(*pointer))
		return ;
	delete (*pointer) ;
	*pointer = NULL ;
}

Glib::ustring base64decode(Glib::ustring url)
{
	if (url.empty())
		return url ;

	guchar* decoded = NULL ;

	try
	{
		gsize size ;
		decoded = g_base64_decode(url.c_str(), &size) ;

		if (!decoded)
			return "" ;

		std::string result ;

		Log::out() << "base64decode ~~~> decoded size = " << size << std::endl ;
		result.assign((gchar*)decoded, size) ;
		g_free(decoded) ;
		return result ;
	}
	catch (...)
	{
		Log::err() << "base64decode: error." << std::endl ;
		if (decoded)
			g_free(decoded) ;
		return "" ;
	}
}

Glib::ustring base64encode(Glib::ustring url)
{
	const gchar* glou = url.c_str() ;
	const guchar* glop = (const guchar*) glou ;
	gchar* encoded = g_base64_encode(glop, url.size()) ;
	std::string encodeds = std::string(encoded) ;
	g_free(encoded) ;
	return encodeds ;
}

std::string float_to_string(float myFloat, int precision)
{
	char buf[30] ;
	std::string format = string("%f") ;

	if (precision>0)
		format = format + "." + number_to_string(precision) ;

	sprintf(buf, format.c_str(), myFloat) ;
	return std::string(buf) ;
}

string getFirstLastWord(const string& s, string& remaining, bool first)
{
	string word = s ;
	remaining = "" ;

	// no word to cut, word is total string
	if (s.empty() || s==" ")
		return word ;

	string buf = s ;
	if (first)
	{
		// (trim) let's place to first letter (in case where 1st char is space)
		unsigned int begin = buf.find_first_not_of(" \n\t", 0) ;
		// find first space after first char and cut
		unsigned int pos = buf.find_first_of(" \n\t", begin) ;
		if (pos != string::npos)
		{
			word = buf.substr(0, pos) ;
			remaining = buf.substr(pos, buf.size()) ;
		}
	}
	else
	{
		// (trim) let's place to last letter (in case where last char is space)
		unsigned int end = buf.find_last_not_of(" \n\t", buf.size()) ;
		// find last space before last char and cut
		unsigned int pos = buf.find_last_of(" \n\t", end) ;
		if (pos != string::npos)
		{
			word = buf.substr(pos+1, buf.size()) ;
			remaining = buf.substr(0, pos+1) ;
		}
	}

	// specific case because of our trim : nothing in remaining except space ?
	// -- consider we had nothing to cut
	unsigned int nospace = remaining.find_first_not_of(" \n\t", 0) ;
	if (nospace==string::npos)
	{
		remaining = "" ;
		word = s ;
	}

	return word ;
}

int checkStringOffset(Glib::ustring str, int toffset)
{
	//-- Empty string
	if (str.empty() || StringOps(str).trim().empty())
		return -1 ;
	//-- Before text
	if (toffset <= 0)
		return 0 ;
	//-- After text
	else if (toffset >= (str.length()-1) )
		return 1 ;
	else
	{
		const Glib::ustring& tmpBefore = str.substr(0, toffset) ;
		const Glib::ustring& tmpAfter = str.substr(toffset, str.length()-1) ;
		StringOps tmpB(tmpBefore) ;
		StringOps tmpAft(tmpAfter) ;
		//-- Before is only spaces, it's before text :)
		if (tmpB.trim(" \n").empty())
			return 0 ;
		//-- After is only spaces, it's after text :)
		else if (tmpAft.trim(" \n").empty())
			return 1 ;
		//-- Otherwise it's middle :)
		else
			return 2 ;
	}
}
