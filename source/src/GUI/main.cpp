/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include <gtkmm/main.h>
#include <gtkmm/icontheme.h>
#include <gtkmm.h>
#include <gtkmm/accelmap.h>
#include <xercesc/sax/SAXParseException.hpp>
#include <string>

#include "GuiWidget.h"
#include "Configuration.h"
#include "Explorer_dialog.h"
#include "UserDialog.h"
#include "StartingError.h"
#include "TAGCommandLine.h"

#include "Common/util/FileHelper.h"
#include "Common/Parameters.h"
#include "Common/InputLanguageHandler.h"
#include "Common/globdef.h"
#include "Common/FileInfo.h"
#include "Common/Explorer_utils.h"
#include "Common/Parameters.h"
#include "Common/VersionInfo.h"
#include "Common/util/Log.h"
#include "Common/util/FormatTime.h"
#include "Common/VersionInfo.h"
#include "Common/util/Utils.h"
#include "TranscriberAG-config.h"

using namespace tag ;

int main(int argc, char *argv[])
{
	//> -- Prepare options
	TAGCommandLine* parser = new TAGCommandLine() ;
	Glib::OptionContext* context = parser->getContext() ;

	Gtk::Main* kit = NULL ;
	try
	{
		kit = new Gtk::Main(argc, argv, *context) ;
	}
	catch (Glib::OptionError e)
	{
		parser->print_error(e.code()) ;
		deleteAndNull((void**)&parser) ;
		return 0 ;
	}

	//****************************************************** SIMPLE COMMAND LINE
	Glib::ustring parserror = "" ;
	bool done = parser->parse(argc, argv) ;
	bool version = parser->getVersion() ;
	float offset = parser->getOffset() ;
	Glib::ustring commandFileName = parser->getFilename() ;

	//> -- Options error
	if (!done || !version && offset==-1 && commandFileName.empty() && argc!=1)
	{
		parser->print_error() ;
		deleteAndNull((void**)&parser) ;
		deleteAndNull((void**)&kit) ;
		return 0 ;
	}

	//> -- Display version
	if (version)
	{
		parser->print_version() ;
		if (offset==-1 && commandFileName.empty())
		{
			deleteAndNull((void**)&parser) ;
			deleteAndNull((void**)&kit) ;
			return 0;
		}
	}

	//> -- Check file value
	if (!commandFileName.empty() && !Glib::file_test(commandFileName, Glib::FILE_TEST_EXISTS))
	{
		Log::out() << "TranscriberAG --> (!) Unable to find specified file, abort." << std::endl ;
		Log::out() << "\n" << std::endl ;
		deleteAndNull((void**)&parser) ;
		deleteAndNull((void**)&kit) ;
		return 0 ;
	}

	//********************************************************* COMMAND LINE END

	Log::out() << "TranscriberAG --> <*> Starting version " << TRANSAG_VERSION_NO << "\n" << std::endl  ;

	Glib::ustring rcFile_name = "transcriberAG.rc" ;
	Glib::ustring rcUserFile_name = "userAG.rc" ;

	//> -- Execution base
	string cur_exe=FileInfo(argv[0]).realpath() ;
	string dir_exe = "" ;
	if ( Glib::file_test(cur_exe, Glib::FILE_TEST_EXISTS) )
		dir_exe = Glib::path_get_dirname(cur_exe) ;
	Log::out() << "TranscriberAG --> <*> Execution directory: " << dir_exe << std::endl  ;

	FileInfo actual_path(cur_exe);
	string exedir =  FileInfo(cur_exe).dirname() ;
	if ( ! actual_path.exists() )
	{
		// it happens when executing a command solely by its name (when it is in the execution path)
		// then exepath is false (user's home dir is prepended to exe name)
		// -> we check execution path to find the actual path
		string cur_name = actual_path.Basename();
		exedir = "/usr/local/bin";
		const gchar* path_var ;
		if ( (path_var = g_getenv("PATH")) != NULL ) 
		{
			vector<string> v;
			vector<string>::iterator itv;
			StringOps(path_var).split(v, ":");
			for (itv=v.begin(); itv != v.end(); ++itv) 
			{
				cur_exe = FileInfo(*itv).join(cur_name);
				if ( FileInfo(cur_exe).exists() ) 
				{
					exedir = *itv;
					break;
				}
			}
		}
	}

	//> -- TranscriberAG base
	FileInfo info(exedir);
	int lev = 2;

	//> -- If libtool binary, go up one time more
	if ( strcmp(info.tail(), ".libs") == 0 )
		++lev;
	string transcriber_root = FileInfo(exedir).dirname(lev);

	//> -- TranscriberAG Configuration base
	Glib::ustring defaultConfig_path = "" ;
	Glib::ustring defaultLocale_path = "" ;
	Glib::ustring current_name, up ;
	bool found = false ;
	up = FileInfo(exedir).dirname(lev-1);
	bool start_ok0 = true ;
	#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	try
	{
		Glib::Dir dir(up) ;
		//> -- For each files contained in it do recurence
		while ( (current_name=dir.read_name()) != "" && !found)
		{
			if (current_name=="etc")
			{
				found = true ;
				Glib::ustring etcTransAG = FileHelper::build_path("etc","TransAG") ;
				defaultConfig_path = FileHelper::build_path(up,etcTransAG) ;
			}
		}

		if ( !found )
		{
			Glib::ustring etcTransAG = FileHelper::build_path("etc","TransAG") ;
			defaultConfig_path = FileHelper::build_path(transcriber_root,etcTransAG) ;
		}

		dir.close() ;
	}
	catch (Glib::FileError e)
	{
		start_ok0 = false ;
		Log::err() << "TranscriberAG --> <*> General configuration problem :> can't read configuration file "<< std::endl ;
	}
	defaultLocale_path = defaultConfig_path + "/locales";
    #else
    Glib::ustring etcTransAG = "/etc/TransAG";
    defaultConfig_path = "/etc/TransAG";
    if ( ! Glib::file_test(defaultConfig_path, Glib::FILE_TEST_EXISTS) ) {
		start_ok0 = false ;
		Log::err() << "TranscriberAG --> <*> General configuration problem :> can't read configuration file "<< std::endl ;
    }
    defaultLocale_path = LOCALEDIR;
    #endif
	Log::out() << "TranscriberAG --> <*> Configuration directory: " << defaultConfig_path << endl;

	Glib::ustring ime_to_restore = "" ;

	/*
	 * 0: complete
	 * 1: update
	 * 2: launch
	 */
	int starting_mode = -9 ;

	bool start_ok1 = true ;
	bool start_ok2 = true ;
	bool start_ok3 = true ;

	Glib::ustring error ;

	Parameters default_parameters  ;
	Parameters default_user_parameters  ;

	Parameters local_parameters ;
	Parameters local_user_parameters  ;

	Glib::ustring rcDefaultFile_path ;
	Glib::ustring rcDefaultUserFile_path ;
	Glib::ustring localConfig_name ;
 	Glib::ustring localConfig_path ;
 	Glib::ustring home_dir = Glib::get_home_dir() ;

	if (start_ok0)
	{
		//> -- Prepare domain
		setlocale(LC_ALL,"");
		bindtextdomain(GETTEXT_PACKAGE, (defaultLocale_path).c_str());
		bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
        textdomain (GETTEXT_PACKAGE);
		
		#ifdef __APPLE__
		bindtextdomain("gtk20", (defaultLocale_path).c_str());
		#endif

		//> -- Load Default Parameters
		try
		{
			//load default system parameters
 			rcDefaultFile_path = FileHelper::build_path(defaultConfig_path,rcFile_name) ;
			default_parameters.load(rcDefaultFile_path);

			//load default user parameters
			rcDefaultUserFile_path = FileHelper::build_path(defaultConfig_path,rcUserFile_name) ;
			default_user_parameters.load(rcDefaultUserFile_path);

			//merge parameters
			Parameters::mergeUserParameters(&default_user_parameters, &default_parameters, false) ;

			localConfig_name = default_parameters.getParameterValue("General", "start,homeFolder") ;
		}
		catch (const char* e)
		{
			Log::err() << "TranscriberAG --> (!) ERROR 1: Loading Default Parameters\n" << e << std::endl ;
			start_ok1 = false ;
			error=e ;
		}

		if (start_ok1)
		{
		 	bool exist_user_parameter = false ;

			//> -- Compute path for rc local file
		 	localConfig_path = FileHelper::build_path(home_dir,localConfig_name) ;
			Log::out() << "TranscriberAG --> <*> Home directory: " << localConfig_path<< endl;
			Glib::ustring rcLocalFile_path = FileHelper::build_path(localConfig_path,rcFile_name) ;
			Glib::ustring rcUserLocalFile_path = FileHelper::build_path(localConfig_path,rcUserFile_name) ;

 			Glib::ustring dtdConf_name = default_parameters.getParameterValue("General", "start,dtd_config") ;
	 		Glib::ustring localDtdConf_path = FileHelper::build_path(localConfig_path, dtdConf_name) ;
	 		Glib::ustring defaultDtdConfig_path = FileHelper::build_path(defaultConfig_path, dtdConf_name) ;

			//> -- Check home configuration directory
			if ( FileHelper::exist_file_in_dir(home_dir, localConfig_name) )
		 	{
				try
				{
					//copy TAG CONFIGURATION DTD
					if (FileHelper::existFile(defaultDtdConfig_path))
					{
			 			int res = FileHelper::copy_in_filesystem(defaultDtdConfig_path, localConfig_path) ;
			 			Log::out() << "TranscriberAG --> <*> TAG configuration DTD existence [" << res << "]"  << std::endl ;
					}

					local_parameters.load(rcLocalFile_path) ;
					Glib::ustring file_version = local_parameters.getParameterValue("General", "VERSION,versionREF") ;
					Glib::ustring user_version = "0" ;

					if ( Glib::file_test(rcUserLocalFile_path, Glib::FILE_TEST_EXISTS) )
					{
						local_user_parameters.load(rcUserLocalFile_path) ;
						user_version = local_user_parameters.getParameterValue("General", "VERSION,versionUSER") ;
						exist_user_parameter = true ;
						Log::out() << "TranscriberAG --> <*> Checking existing user profile [YES] "  << endl;
					}
					else
						Log::out() << "TranscriberAG --> <*> Checking existing user profile [N0] "  << endl;

					//if same version LAUNCHING MODE
					if (file_version.compare(TRANSAG_VERSION_NO)==0 && user_version.compare(TRANSAG_VERSION_NO)==0)
					{
						exist_user_parameter = true ;
						starting_mode = 2 ;
						Log::out() << "TranscriberAG --> <*> Starting mode [LAUNCH] "  << endl;
					}
					//if different version UPDATE MODE
					else
					{
						starting_mode = 1 ;
						Log::out() << "TranscriberAG --> <*> Starting mode [UPDATE] "  << endl;
					}
				}
				catch (const char* e)
				{
					Log::err() << "TranscriberAG --> (!) ERROR2: Loading existing local parameters <!>\n" << e << std::endl ;
					start_ok2 = false ;
				}
		 	}
		 	//if no local configuration directory CREATING MODE
		 	else
		 	{
				Log::out() << "TranscriberAG --> <*> Starting mode [CREATE] "  << endl;
		 		starting_mode = 0 ;
		 	}

		 	if (start_ok2)
		 	{
	 			Glib::ustring dtd_name = default_parameters.getParameterValue("General", "start,dtd") ;
			 	Glib::ustring gtkrc_name = default_parameters.getParameterValue("General", "start,gtkrc") ;
		 		Glib::ustring localDtd_path = FileHelper::build_path(localConfig_path, dtd_name) ;
		 		Glib::ustring localGtkrc_path = FileHelper::build_path(localConfig_path, gtkrc_name) ;
		 		Glib::ustring rcLocalFile_path =  FileHelper::build_path(localConfig_path, rcFile_name) ;
		 		Glib::ustring rcUserLocalFile_path =  FileHelper::build_path(localConfig_path, rcUserFile_name) ;

		 		//>>>>>>>>>>>>>>>>>>>>>> UPDATING or COMPLETE START
		 		if (starting_mode==0 || starting_mode==1)
		 		{
		 			//>> UPDATING MODE
				 	if (starting_mode==1)
				 	{
				 		Parameters old_user_parameters, old_system_parameters ;
				 		//> -- If exists DTD, load old values
				 		if ( FileHelper::existFile(localDtdConf_path) )
				 		{
							if (exist_user_parameter)
								old_user_parameters.load(rcUserLocalFile_path) ;
							old_system_parameters.load(rcLocalFile_path) ;
				 		}
				 		else
				 			exist_user_parameter = false ;

				 		//> -- Remove DTD, gtkRC, system conf, user conf
				 		FileHelper::remove_from_filesystem(localDtd_path) ;
				 		FileHelper::remove_from_filesystem(localGtkrc_path) ;
				 		FileHelper::remove_from_filesystem(rcLocalFile_path) ;
				 		if (exist_user_parameter)
				 			FileHelper::remove_from_filesystem(rcUserLocalFile_path) ;

				 		//> -- Copy new system conf and merge with old values
				 		int res = FileHelper::copy_in_filesystem(rcDefaultFile_path, localConfig_path) ;
				 		local_parameters.load(rcLocalFile_path) ;
				 		Log::out() << "TranscriberAG --> <UPDATE> System configuration [" << res << "]"  << std::endl ;

						//> -- Update user conf
				 		res = FileHelper::copy_in_filesystem(rcDefaultUserFile_path, localConfig_path) ;
				 		local_user_parameters.load(rcUserLocalFile_path) ;
						if (exist_user_parameter)
						{
							Parameters::updateUserParameters(&old_user_parameters, &local_user_parameters, true) ;
							Log::out() << "TranscriberAG --> <UPDATE> User configuration [" << res << "]"  << std::endl ;
						}
						else
							Log::out() << "TranscriberAG --> <UPDATE> User configuration initialized [" << res << "]"  << std::endl ;

				 		Glib::ustring localUserRc_path = FileHelper::build_path(localConfig_path, gtkrc_name) ;
				 	} // end update treatment

				 	//> CREATING MODE: create local directory and copy files
				 	else if (starting_mode==0)
				 	{
				 		//-- Create directory
				 		int res = FileHelper::create_directory(localConfig_path) ;
						Log::out() << "TranscriberAG --> <START> parameters directory [" << res << "]"  << std::endl ;

						//-- Copy rcFile
						res = FileHelper::copy_in_filesystem(rcDefaultFile_path, localConfig_path) ;
						Log::out() << "TranscriberAG --> <START> System files [" << res << "]"  << std::endl ;
						res = FileHelper::copy_in_filesystem(rcDefaultUserFile_path, localConfig_path) ;
						Log::out() << "TranscriberAG --> <START> User files [" << res << "]"  << std::endl ;

						//-- Add demo shortcut
						Glib::ustring demo_name = default_parameters.getParameterValue("General", "start,demo") ;
						Glib::ustring demo_path = FileHelper::build_path(defaultConfig_path, demo_name) ;
						Glib::ustring shortcut_name = default_parameters.getParameterValue("GUI", "data,shortcuts") ;
						Glib::ustring shortcut_path = FileHelper::build_path(localConfig_path, shortcut_name) ;
						demo_path = "demoAG|" + demo_path ;
						Explorer_utils::write_line(shortcut_path, demo_path, "w") ;

				 		//-- Create default workdir
						Glib::ustring default_work = default_parameters.getParameterValue("General","start,workdir") ;
						if (default_work!="")
						{
							FileInfo info1(default_work) ;
							default_work = info1.realpath() ;
							FileHelper::create_directory(default_work) ;
						}

			 			//-- Copy DTD required for read local config path
						res = FileHelper::copy_in_filesystem(defaultDtdConfig_path, localConfig_path) ;
			 			Log::out() << "TranscriberAG --> <START> TAG configuration DTD existence [" << res << "]"  << std::endl ;

				 	} //end from scratch treatment

		 			//>> -- BOTH CASE: copy default configuration files in directory

				 	//-- DTDs
			 		Glib::ustring defaultDtd_path = FileHelper::build_path(defaultConfig_path, dtd_name) ;

			 		//-- RC file
			 		Glib::ustring defaultGtkrc_path = FileHelper::build_path(defaultConfig_path, gtkrc_name) ;

			 		int res ;

		 			//-- Copy dtd TAG
		 			res = FileHelper::copy_in_filesystem(defaultDtd_path, localConfig_path) ;
		 			Log::out() << "TranscriberAG --> <START> DTD TAG format initialization [" << res << "]"  << std::endl ;

					//-- Copy gtkrc
					res = FileHelper::copy_in_filesystem(defaultGtkrc_path, localConfig_path) ;
					Log::out() << "TranscriberAG --> <START> GTK theme initialization [" << res << "]"  << std::endl ;
		 		}

		 		//>>>>>>>>>>>>>>>>>>> READ LOCAL CONFIGURATION AND SAVE ABSOLUTE VALUES
				try
				{
					Glib::ustring default_work ;

					local_parameters.load(rcLocalFile_path) ;
					local_user_parameters.load(rcUserLocalFile_path) ;

					//> -- Load theme file
					gtk_rc_parse(localGtkrc_path.c_str());

					//> -- Save config path
					local_parameters.setParameterValue("General", "start,config", defaultConfig_path, true);

			 		//> -- Compute and save real path for work directory in USER FILE
					default_work = local_user_parameters.getParameterValue("General","start,workdir") ;
					if (default_work!="")
					{
						FileInfo info1(default_work) ;
						default_work = info1.realpath() ;
						local_user_parameters.setParameterValue("General", "start,workdir", default_work, true);
					}

					//> -- If CREATING or UPDATING, save real path and load languages, and compute and save real demo path
					if (starting_mode==0 || starting_mode==1)
					{
				 		//> set current version in user and system conf
						local_parameters.setParameterValue("General", "VERSION,versionREF", TRANSAG_VERSION_NO, true) ;
						local_user_parameters.setParameterValue("General", "VERSION,versionUSER", TRANSAG_VERSION_NO, true) ;

						//> compute and save input languages path in SYSTEM FILE
						Glib::ustring input = local_parameters.getParameterValue("Data", "inputLanguage,inputFile") ;
						Glib::ustring name = Explorer_filter::cut_extension(input) ;
						Glib::ustring ext = Explorer_filter::get_extension(input) ;
						Glib::ustring os ;
#ifdef WIN32
						os = ".windows" ;
#else
	#ifdef __APPLE__
							os= ".mac" ;
	#else
							os = ".linux" ;
	#endif
#endif
						input = name + os + ext ;
						if (input!="")
						{
							Glib::ustring folder_lang = FileHelper::build_path(defaultConfig_path, "input") ;
							Glib::ustring lang = FileHelper::build_path(folder_lang, input) ;
							local_parameters.setParameterValue("Data", "inputLanguage,inputFile", lang, true);
						}

						//> -- Compute real path and save it for DEMO in SYSTEM FILE
						Glib::ustring demo = local_parameters.getParameterValue("General", "start,demo") ;
						demo = FileHelper::build_path(defaultConfig_path,demo);
						local_parameters.setParameterValue("General", "start,demo", demo, true);

						//> -- Compute real path and save it for USER DOC in SYSTEM FILE
						Glib::ustring doc = local_parameters.getParameterValue("General", "Doc,Docpath") ;
						doc = FileHelper::build_path(defaultConfig_path,doc);
						local_parameters.setParameterValue("General", "Doc,Docpath", doc, true);

						//> -- Compute real path and save it for SPELLER
						Glib::ustring speller_directory = local_user_parameters.getParameterValue("AnnotationEditor", "Speller,directory") ;
						if ( ! Glib::path_is_absolute(speller_directory) && speller_directory.compare("nospell")!=0 )
						{
							Glib::ustring etc_dir = Glib::path_get_dirname(defaultConfig_path) ;
							speller_directory = Glib::build_filename(etc_dir,speller_directory) ;
							if ( Glib::file_test(speller_directory, Glib::FILE_TEST_EXISTS) )
								local_user_parameters.setParameterValue("AnnotationEditor", "Speller,directory", speller_directory) ;
						}

						//> -- Compute real path and save it for SPEAKER DICTIONARY
						Glib::ustring speaker_dico = local_user_parameters.getParameterValue("Data", "speaker,globalDictionary") ;
						if ( ! Glib::path_is_absolute(speaker_dico) )
						{
							speaker_dico = Glib::build_filename(defaultConfig_path, speaker_dico) ;
							if ( Glib::file_test(speaker_dico, Glib::FILE_TEST_EXISTS) )
								local_user_parameters.setParameterValue("Data", "speaker,globalDictionary", speaker_dico) ;
						}

					} // end of starting or update mode

					// -- Load languages
					Glib::ustring languages = local_parameters.getParameterValue("Data", "languages,languagesFile") ;
			 		if ( !Glib::file_test(languages, Glib::FILE_TEST_EXISTS) && (languages!="") )
			 		{
						Glib::ustring lang = FileHelper::build_path(defaultConfig_path, languages) ;
						local_parameters.setParameterValue("Data", "languages,languagesFile", lang, true);
					}

					// -- Save new configuration
					local_parameters.setParameterValue("General", "start,userFile", rcUserLocalFile_path, true) ;
					local_parameters.save() ;
					local_user_parameters.save() ;

					// -- Merge new configuration
					Parameters::mergeUserParameters(&local_user_parameters, &local_parameters, false) ;
				}
				catch (const char* e)
				{
					Log::err() << "TranscriberAG --> (!) ERROR3: Loading updated local parameters\n" << e << std::endl ;
					start_ok3=false ;
					error=e ;

				 	gtkrc_name = default_parameters.getParameterValue("General", "start,gtkrc") ;
				 	Glib::ustring config_gtkrc_path = FileHelper::build_path(defaultConfig_path,gtkrc_name) ;
					gtk_rc_parse(config_gtkrc_path.c_str());
				}
			} //end start_ok2
		} //end start_ok1
	} //end start_ok0

	g_thread_init(NULL);
	gdk_threads_init();

	//> -- Compute ICONS BASE
	string iconsdir = FileInfo(defaultConfig_path).join("icons");
	string iconsdir_gui = FileInfo(iconsdir).join("GUI");

	//> -- Load icons
	Glib::RefPtr<Gtk::IconTheme> theme = Gtk::IconTheme::get_default();
    theme->prepend_search_path(iconsdir);
    theme->prepend_search_path(iconsdir_gui);

	Gtk::Window* window  ;
	Glib::ustring accelmap ;

	//> -- DISPLAY ERROR MESSAGE
	if (!start_ok1 || !start_ok2 || !start_ok3 || !start_ok0)
	{
		Glib::ustring message;
		if (!start_ok1)
			message =  _("Error loading default configuration");
		else if (!start_ok2)
			message =  _("Error loading existing user local configuration");
		else if (!start_ok3)
			message =  _("Error loading updated user local configuration");
		else if (!start_ok0)
			message =  _("Error reading configuration directory");

		window = new StartingError(message) ;
	}
	//> -- LAUNCHING
	else
	{
#ifndef WIN32
		bool setenv ;

		//> -- Get current IME variable
		const gchar* ime =  g_getenv("GTK_IM_MODULE") ;

		//>  -- CLEAR CURRENT IME VARIABLE TO ENABLE GOOD BEHAVIOUR IN CASE scim-bridge IS. ALREADY SET
		if (ime!=NULL)
		{
			//stock for restoring later
			ime_to_restore = ime ;
			setenv = g_setenv("GTK_IM_MODULE", "nogtkimevalue", true) ;
			Log::out() << "TranscriberAG --> Input Method Environment Cleaned: [" << setenv << " - " << ime_to_restore << "]" << std::endl ;
		}

		//> -- SET VARIABLE FOR INPUT METHOD IF EXTERNAL IME IS ALLOWED
		Glib::ustring IME = local_parameters.getParameterValue("Data", "inputLanguage,enableIME") ;
		if (IME.compare("true")==0)
		{
			Glib::ustring im_module = local_parameters.getParameterValue("General", "ENV,gtk_im_module") ;
			bool setenv = g_setenv("GTK_IM_MODULE", im_module.c_str(), true) ;
			Log::out() << "TranscriberAG --> Input Method Environment [" << setenv << " - " << im_module << "]" << std::endl ;
		}
		else
			Log::out() << "TranscriberAG --> Input Method Environment [disabled]" << std::endl ;
#endif

		// -- SET LOG ENVIRONMENT
		string log_pref = local_parameters.getParameterValue("General", "LOG,log_file");
		if ( log_pref.empty() )
			log_pref = g_get_prgname();
		string log_file = FileInfo(log_pref).realpath();

		if ( Glib::file_test(log_file + ".log" , Glib::FILE_TEST_EXISTS) )
		{
			setlocale(LC_TIME,"");

			Glib::Date date;
			date.set_time(time(0));

			string current_date = date.format_string("-%Y-%m-%d_") + FormatTime::formatDate(time(0), "%H-%M-%S");

			string old_file = log_file + current_date + ".log";
			FileHelper::rename_in_filesystem(log_file + ".log", old_file);
 		}
		string parentdir = FileInfo(log_file).dirname();
		if ( ! FileInfo(parentdir).exists() )
		{
			FileHelper helper;
			helper.create_directory_with_parents(parentdir);
		}

		string redirect = local_parameters.getParameterValue("General", "LOG,mode");
		if (redirect == "classic")
		{
			Log::out() << "TranscriberAG --> File logging [ON]" << endl;
			try
			{
				Log::redirect(log_file);
			}
			catch(const char* msg)
			{
				Log::out() << "Log::redirect : " << msg << endl;
				Log::redirect("/dev/null");
			}
		}
		else
			Log::out() << "TranscriberAG --> File logging [OFF]" << endl;

		//> -- Prepare accelmap
	 	accelmap = FileHelper::build_path(localConfig_path, "AccelMap") ;

		//> -- Load accel map if exists
		if ( FileInfo(accelmap).exists() )
			Gtk::AccelMap::load(accelmap);

		Log::out() << "TranscriberAG --> <*> Launching display...\n" << std::endl ;

		window = new GuiWidget(&local_parameters, parser) ;
	}

	gdk_threads_enter() ;
	kit->run(*window) ;
	gdk_threads_leave() ;

	if (start_ok1 && start_ok2 && start_ok3 && start_ok0)
	{
		Gtk::AccelMap::save(accelmap);
		Log::trace() << "TranscriberAG --> <.> Saving Accelmap [OK]" << std::endl  ;
	}

	delete(window);

	//> -- RESTORE OLD IME VARIABLE VALUE BEFORE EXITING
	if (ime_to_restore.compare("")!=0)
		g_setenv("GTK_IM_MODULE", ime_to_restore.c_str(), true) ;

	Log::trace() << "TranscriberAG --> <.> Closing TranscriberAG [OK]" << std::endl  ;

	if (kit)
		delete(kit) ;

	if (parser)
		delete(parser) ;

	return 0;
}
