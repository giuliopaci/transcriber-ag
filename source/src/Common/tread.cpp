/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include <fstream>
#include <iostream>
#include <string>
#include "util/StringOps.h"
// xerces
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/util/PlatformUtils.hpp>


#include "CommonXMLReader.h"

using namespace std;

string guessFileFormat(const string& path, string* file_dtd) throw (const char*)
{
	// read file firsts lines to figure out file format
	ifstream fi(path.c_str());
	if ( fi.bad() ) {
		string msg = "File open failed";
		msg += " : " ;
		msg += path;
		throw msg.c_str();
	}
	string tag, dummy, dtd;
	string format="";

	fi >> tag;
	if ( fi.good() && tag == "<?xml" ) {
		getline(fi, dummy);
		fi >> tag;
		if ( fi.good() && tag == "<!DOCTYPE" ) {
			unsigned int pos=0;
			fi >> tag >> dummy >> dtd;
			format = tag;
cout << "dummy=" << dummy << "- dtd=" << dtd << endl;

			if ( !format.empty() && dummy == "SYSTEM" && file_dtd != NULL ) 
					StringOps(dtd).getToken(pos, *file_dtd, "\"");
		}
	} else {
	}
	
	fi.close();
	return format;
}

class MyHandler : public CommonXMLHandler 
{
	
  
  void startElement( const   XMLCh* const    uri,
		     const   XMLCh* const    localname,
		     const   XMLCh* const    qname,
		     const   Attributes&     attrs) {
			
			cout << "localname" << getString(localname) << endl;
			cout << " qname=" << getString(localname) << endl;
		std::map<std::string, std::string> attmap;
		std::map<std::string, std::string>::iterator it;
			getAttributes(attrs, attmap);
		for ( it=attmap.begin(); it != attmap.end(); ++it )
			cout <<"\t" << it->first << " = " << it->second << endl;
	} 
  
};

int main(int argc, const char* argv[])
{
	string dtd("");

	//string format = guessFileFormat(argv[1], &dtd);

	try {
	MyHandler h;
	CommonXMLReader r(&h, "");

		r.parseFile(argv[1]);
	} catch (const char* msg) {
		Log::err() << "ERROR = " << msg;
	}

	cout << " OK READ " << argv[1] << endl;

	return 1;
}

