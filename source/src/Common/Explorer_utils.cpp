/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  TransAGVideo				*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "Explorer_utils.h"
#include "Common/util/Utils.h"
#include "Common/icons/Icons.h"
#include "Common/Explorer_filter.h"
#include "Common/FileInfo.h"
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "util/Log.h"

namespace tag {

/*
 * from time in second to time in HH/MM/SS
 */
void Explorer_utils::get_time(double seconds_time, std::vector<int>* result)
{
	int h, m, s ;
	h = (int)(seconds_time / 3600) ;
	m = (int)((seconds_time - 3600*h) / 60) ;
	s = (int)(seconds_time - 3600*h - 60*m) ;

	result->insert(result->end(),h) ;
	result->insert(result->end(),m) ;
	result->insert(result->end(),s) ;
}


int Explorer_utils::write_line(Glib::ustring path, Glib::ustring line, Glib::ustring mode)
{
	FILE* file = NULL ;
	file = fopen(path.c_str(),mode.c_str()) ;
	if (file!=NULL)
	{
		Glib::ustring s = line+"\n" ;
		fputs(s.c_str(), file) ;
		fclose(file) ;
		return 1 ;
	}
	else
	{
		Explorer_utils::print_trace("Explorer_utils::write_line:> <!>failure<!>", path, 0) ;
		return 0 ;
	}
}

int Explorer_utils::read_lines(Glib::ustring path, std::vector<Glib::ustring>* lines)
{
	int MAX = 300 ;
	FILE* file = NULL ;
	file = fopen(path.c_str(),"r") ;
	char buf[MAX] ;
	string tmp ;
	if (file!=NULL)
	{
		while ( fgets(buf, MAX, file) !=NULL )
		{
			tmp = string(buf) ;
			//cut \n at the end
			tmp = tmp.substr(0,tmp.size()-1);
			int err ;
			tmp = FormatToUTF8::checkUTF8(tmp, "", false, err) ;
			lines->insert(lines->end(), tmp) ;
		}
		fclose(file) ;
		return 1 ;
	}
	else
	{
		Explorer_utils::print_trace("Explorer_utils::read_lines:> <!>failure<!>", path, 0) ;
		return -1 ;
	}
}

int Explorer_utils::write_lines(Glib::ustring path, std::vector<Glib::ustring> lines, Glib::ustring mode)
{
	FILE* file = NULL ;
	file = fopen(path.c_str(),mode.c_str()) ;
	std::vector<Glib::ustring>::iterator it = lines.begin() ;

	if (file!=NULL)
	{
		while ( it!=lines.end() )
		{
			if ( (*it) != "" )
			{
				Glib::ustring s = (*it) +"\n" ;
				fputs(s.c_str(), file) ;
			}
			it++ ;
		}
		fclose(file) ;
		return 1 ;
	}
	else
	{
		Explorer_utils::print_trace("Explorer_utils::write_lines:> <!>failure<!>", path, 0) ;
		return 0 ;
	}
}

int Explorer_utils::write_line(FILE* file, Glib::ustring line)
{
	if (file!=NULL)
	{
		Glib::ustring s = line+"\n" ;
		fputs(s.c_str(), file) ;
		return 1 ;
	}
	else
	{
		Explorer_utils::print_trace("Explorer_utils::write_line(FILE):> <!>failure<!>",0) ;
		return 0 ;
	}
}


/**
 * 	return the position of ending substring in complete string s
 */
int Explorer_utils::is_substring(Glib::ustring sub, Glib::ustring s)
{
	if (sub.size() > s.size())
		return -1 ;
	else
	{
		s = s.lowercase() ;
		sub = sub.lowercase() ;
		bool match = true ;
		guint i =0 ;
		while ( i!=sub.size()-1 && match)
		{
			if (s[i]!=sub[i])
				match=false ;
			else
				i++	;
		}
		if (match)
			return i ;
		else
			return 0 ;
	}
}

Glib::ustring Explorer_utils::format_date(Glib::ustring time)
{
	//TODO translation time
	return time ;
}

Glib::ustring Explorer_utils::format_size(Glib::ustring size)
{
	Glib::ustring display_size ;
	float size_i = string_to_number<float>(size) ;
	float b = size_i ;
	float kb = size_i / 1024 ;
	float mb = size_i / (1024*1024) ;

	char buf[100] ;
	Glib::ustring tmp ;

	if (b<1024)
	{
		sprintf(buf, "%.2f", b) ;
		g_strstrip(buf) ;
		display_size = Glib::ustring(buf) + " " +  _("bytes") ;
	}
	else if (kb<1024)
	{
		sprintf(buf, "%.2f", kb) ;
		g_strstrip(buf) ;
		display_size = Glib::ustring(buf) + " " +  _("Kb") ;
	}
	else
	{
		sprintf(buf, "%.2f", mb) ;
		g_strstrip(buf) ;
		display_size = Glib::ustring(buf) + " " + _("Mb") ;
	}
	return display_size ;
}

void Explorer_utils::print_trace(Glib::ustring s, int mode)
{
	if (mode==1)
		Log::out() << s << std::endl ;
	else if (mode==0)
		Log::err() << s << std::endl ;
}


Glib::ustring Explorer_utils::format_recent_file(Glib::ustring path)
{
	Glib::ustring res  = path ;

	size_t indice = 0 ;
	while (indice!=std::string::npos)
	{
		indice = res.find('_', indice) ;
		if (indice!=std::string::npos)
		{
			res.replace(indice,1, "__") ;
			indice = indice + 2 ;
		}
	}
	return res ;
}

} //namespace
