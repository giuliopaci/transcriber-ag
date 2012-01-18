/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include <glib.h>

#include "TAGCommandLine.h"
#include "Common/globals.h"
#include "Common/VersionInfo.h"

namespace tag {


TAGCommandLine::TAGCommandLine()
{
	//> init values
	m_offset = "" ;
	m_version = false ;
	m_filename = "" ;
	m_reportLevel = "" ;
	error = Glib::OptionError::FAILED ;

	//> create context
	context = new Glib::OptionContext(_("- Annotation Graph Editor")) ;
	context->set_help_enabled(true) ;
	context->set_description("TranscriberAG - Bertin Technologies 2006-2009\n") ;
	context->set_ignore_unknown_options(true);

	//> create TAG group
	group = new Glib::OptionGroup("tag", _("TranscriberAG command line options"), _("options for TranscriberAG")) ;

	//> create all entries
	Glib::OptionEntry* entry = NULL ;

	entry = createEntry("filepath", 'f', 0, _("open an audio or an annotation file"), _("FILEPATH")) ;
	entries.push_back(entry) ;
	group->add_entry(*(entries.back()), m_filename) ;

	entry = createEntry("version", 'v', 0, _("display version of TranscriberAG"), "") ;
	entries.push_back(entry) ;
	group->add_entry(*(entries.back()), m_version) ;

	entry = createEntry("offset", 'o', 0, _("open file at a specific signal offset (seconds)"), _("OFFSET")) ;
	entries.push_back(entry) ;
	group->add_entry(*(entries.back()), m_offset) ;

	entry = createEntry("report", 'r', 0, _("specify the report level: 0(never) - 1(always) - 2(only if errors)"), _("REPORT_LEVEL")) ;
	entries.push_back(entry) ;
	group->add_entry(*(entries.back()), m_reportLevel) ;

	//> add group
	context->set_main_group(*group) ;
}

bool TAGCommandLine::parse(int argc, char *argv[])
{
	if (!context)
		return false ;

	bool res = true ;
	try {
		res = context->parse(argc, argv) ;
		if (!res)
			error = Glib::OptionError::FAILED ;
	}
	catch (Glib::OptionError e) {
		res = false ;
		error = e.code() ;
	}
	return res ;
}



TAGCommandLine::~TAGCommandLine()
{
	if (context)
		delete(context) ;
	if (group)
		delete(group) ;
	std::vector<Glib::OptionEntry*>::iterator it ;
	for (it=entries.begin(); it!=entries.end(); it++) {
		if (*it)
			delete(*it) ;
	}
}

Glib::OptionEntry* TAGCommandLine::createEntry(Glib::ustring longName, gchar shortName, int flag,
								Glib::ustring description, Glib::ustring description_arg)
{
	Glib::OptionEntry* ptr = new Glib::OptionEntry() ;
	ptr->set_long_name(longName) ;
	ptr->set_short_name(shortName) ;
	ptr->set_flags(flag) ;
	ptr->set_description(description) ;
	ptr->set_arg_description(description_arg) ;
	return ptr ;
}

void TAGCommandLine::print_version()
{
	std::cout << TRANSAG_DISPLAY_NAME << " version " << TRANSAG_VERSION_NO << " - " << getVersionStamp() << std::endl ;
	std::cout << "Bertin Technologies 2010\n" << std::endl ;
}

void TAGCommandLine::print_error()
{
	print_error(error) ;
}

void TAGCommandLine::print_error(Glib::OptionError::Code code)
{
	Log::trace() << get_error(code) << "\n" << std::endl ;
}

Glib::ustring TAGCommandLine::get_error(Glib::OptionError::Code code)
{
	Glib::OptionError::Code err = code ;
	Glib::ustring error ;
	if (err==Glib::OptionError::UNKNOWN_OPTION)
		error = _("Unknown option, use option -?/--help for usage") ;
	else if (err==Glib::OptionError::BAD_VALUE)
		error = _("Bad option value, use option -?/--help for usage") ;
	else
		error = _("Unknown option or bad option value, use option -?/--help for usage") ;
	return error ;
}

} //namespace
