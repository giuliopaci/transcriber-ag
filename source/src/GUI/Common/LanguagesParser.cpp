/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "LanguagesParser.h"
#include "Common/Parameters.h"
#include <stdio.h>
#include <iostream>
#include "Common/util/Log.h"

namespace tag {

LanguagesParser::LanguagesParser()
{
}

LanguagesParser::~LanguagesParser()
{
}

void LanguagesParser::makeConfigFile()
{
	Glib::ustring line = "/home/montilla/Desktop/test" ;
	Glib::ustring config = "/home/montilla/Desktop/languagesAG.xml" ;

	std::vector<Glib::ustring> vect_gen ;

	read_ctl(line, &vect_gen) ;

	std::vector<Glib::ustring>::iterator it = vect_gen.begin() ;

	std::vector <std::vector<Glib::ustring> > languages ;
	std::vector<Glib::ustring> language ;

	while(it!=vect_gen.end())
	{
		if ( (*it) == "SEPARATOR" ) {
			languages.insert(languages.end(),std::vector<Glib::ustring>(language)) ;
			language.clear() ;
		}
		else {
			language.insert(language.end(), *it) ;
		}
		it++ ;
	}

	Parameters param  ;
	try {
		param.load(config) ;
	}
	catch (const char* e) {
		Log::err() << "LanguagesParser:> ERROR " << e << std::endl ;
	}

	std::vector <std::vector<Glib::ustring> >::iterator i_languages = languages.begin();
	while(i_languages!=languages.end())
	{
		std::vector<Glib::ustring> v = (*i_languages) ;
		std::vector<Glib::ustring>::iterator i = v.begin() ;
		Glib::ustring sectionParamHead = v[0] ;

		param.addSection("All", sectionParamHead, v[3]);

		param.setParameterLabel("All", sectionParamHead + ",code3", "ISO 639 code 3 numbers", true);
		param.setParameterValue("All", sectionParamHead + ",code3", v[0], true);

		param.setParameterLabel("All", sectionParamHead + ",code2", "ISO 639 code 2 numbers", true);
		param.setParameterValue("All", sectionParamHead + ",code2", v[1], true);

		param.setParameterLabel("All", sectionParamHead + ",english", "English Description", true);
		param.setParameterValue("All", sectionParamHead + ",english", v[2], true);

		param.setParameterLabel("All", sectionParamHead + ",french", "French Description", true);
		param.setParameterValue("All", sectionParamHead + ",french", v[3], true);

		i_languages++ ;
	}

	param.save() ;
}

int LanguagesParser::read_ctl(Glib::ustring path, std::vector<Glib::ustring>* vect_gen)
{
	int MAX = 200 ;
	FILE* file = NULL ;
	file = fopen(path.c_str(),"r") ;
	char buf[MAX] ;
	char buf_cpt = 0 ;

	bool line = false ;
	bool arg = false ;

	char c ;

	if (file!=NULL) {
		while ( (c=fgetc(file)) != EOF )
		{
			Glib::ustring tmp ;
			std::vector<Glib::ustring> v ;
			if (c=='{')	{
				v.clear() ;
				line=true ;
			}
			else if (c=='}') {
				vect_gen->insert(vect_gen->end(),"SEPARATOR") ;
				line=false ;
			}
			else if (c=='"' && arg) {
				arg=false ;
				//Glib::ustring tmp2 = Glib::ustring(tmp) ;
				Glib::ustring tmp = Glib::ustring(buf,buf_cpt) ;
				try { Log::trace() << "Word: " << tmp << std::endl ; }
				catch (Glib::ConvertError e) { Log::trace() << "Error: " << e.what() << std::endl ; }
				vect_gen->insert(vect_gen->end(), Glib::ustring(buf,buf_cpt)) ;
				buf_cpt = 0 ;
			}
			else if (c=='"' && !arg) {
				arg=true ;
				tmp = "" ;
			}
			else {
				if (line && arg) {
					Log::trace() << "Char: " << c << std::endl ;
					buf[buf_cpt] = c  ;
					buf_cpt++ ;
				}
			}
		}
		Log::trace() << "FINISH" << std::endl ;
		fclose(file) ;
		return 1 ;
	}
	else {
		Log::err() << "write_line:> PB" << std::endl ;
		return 0 ;
	}
}

} //namespace
