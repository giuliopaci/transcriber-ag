/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "Explorer_filter.h"
#include "icons/Icons.h"
#include "Formats.h"
#include "Common/FileInfo.h"

static const char *StdAudioExt[] = { ".wav", ".WAV", ".mp3", ".MP3", ".sph", ".SPH",
				".aif", ".AIF", ".aiff", ".AIFF", ".au", ".AU", ".ogg", ".OGG",
				NULL } ;

static const char *StdVideoExt[] = { ".mpg", ".MPG", ".mpeg", ".MPEG", ".avi", ".AVI", NULL} ;


namespace tag {

Explorer_filter* Explorer_filter::m_filter = NULL ;

Explorer_filter* Explorer_filter::getInstance(bool with_gui_composant)
{
	if (m_filter==NULL)
		m_filter = new Explorer_filter(with_gui_composant) ;
	return m_filter;
}

Explorer_filter::FileType::FileType(Glib::ustring p_format, Glib::ustring p_type,
										Glib::ustring ext, Glib::ustring name,
										Glib::ustring ico, bool is_special, bool is_importable)
{
	type = p_type ;
	extension = ext ;
	special = is_special ;
	importable = is_importable ;
	display = name ;
	format = p_format ;
	if (ico.empty())
		icon = ICO_TREE_DEFAULT ;
	else
		icon = ico ;
}

Explorer_filter::Explorer_filter(bool with_gui_composant)
{
	folder = NULL ;
	all = NULL ;
	none = NULL ;
	if (with_gui_composant)
		combo = Gtk::manage(new Gtk::ComboBoxText()) ;
	else
		combo = NULL ;
	fill() ;
	filter_chosen = DISPLAY_AUDIONOTE ;
	set_combo(filter_chosen) ;
}

Explorer_filter::~Explorer_filter()
{
	std::map<Glib::ustring, Explorer_filter::FileType*>::iterator it ;
	for (it=annotations.begin(); it!=annotations.end(); it++)
		if (it->second)
			delete(it->second) ;

	for (it=videos.begin(); it!=videos.end(); it++)
		if (it->second)
			delete(it->second) ;

	for (it=audios.begin(); it!=audios.end(); it++)
		if (it->second)
			delete(it->second) ;

	for (it=images.begin(); it!=images.end(); it++)
		if (it->second)
			delete(it->second) ;

	for (it=others.begin(); it!=others.end(); it++)
		if (it->second)
			delete(it->second) ;
}

void Explorer_filter::kill()
{
	if (m_filter) {
		delete(m_filter) ;
		m_filter = NULL ;
	}
}

//******************************************************************************
//******************************************************************* COMBO ****
//******************************************************************************

//TODO: fill with a file path
void Explorer_filter::fill()
{
	//INITIALISE SPECIAL TYPES THAT CAN BE CALLED BY EXTERNAL CLASSES

	folder = new Explorer_filter::FileType("FOLDER", "", "dir", _("Directory"), "", true) ;
	none = new Explorer_filter::FileType("NONE", "", EXT_NONE, _("unknown"), "", true) ;
	all = new Explorer_filter::FileType("ALL", "", "all", _("All files"), "", true) ;

	//AUDIO
	audios["WAVE"] = new Explorer_filter::FileType("WAVE", "", ".wav", _("WAVE Audio file"), ICO_TREE_WAV) ;
	audios["MP3"] = new Explorer_filter::FileType("MP3", "", ".mp3", _("MP3 Audio file"), ICO_TREE_WAV) ;
	audios["AU"] = new Explorer_filter::FileType("AU", "", ".au", _("AU Audio file"), ICO_TREE_WAV) ;
	audios["AIFF"] = new Explorer_filter::FileType("AIFF", "", ".aiff", _("AIFF Audio file"), ICO_TREE_WAV) ;
	audios["AIF"] = new Explorer_filter::FileType("AIF", "", ".aif", _("AIFF Audio file"), ICO_TREE_WAV) ;
	audios["SPH"] = new Explorer_filter::FileType("SPH", "", ".sph", _("SPHERE Audio file"), ICO_TREE_WAV) ;

	//IMAGE
	images["JPG"] = new Explorer_filter::FileType("JPG", "", ".jpg", _("JPG Image file"), ICO_TREE_JPG) ;
	images["JPEG"] = new Explorer_filter::FileType("JPEG", "", ".jpeg", _("JPEG Image file"), ICO_TREE_JPG) ;
	images["GIF"] = new Explorer_filter::FileType("GIF", "", ".gif", _("GIF Image file"), ICO_TREE_GIF) ;
	images["PNG"] = new Explorer_filter::FileType("PNG", "", ".png", _("PNG Image file"), ICO_TREE_PNG) ;
	images["BMP"] = new Explorer_filter::FileType("BMP", "", ".bmp", _("BMP Image file"), ICO_TREE_JPG) ;

	//VIDEO
	videos["MPEG"] = new Explorer_filter::FileType("MPEG", "", ".mpeg", _("MPEG Video file"), ICO_TREE_MPG) ;
	videos["MPG"] = new Explorer_filter::FileType("MPG", "", ".mpg", _("MPG Video file"), ICO_TREE_MPG) ;
	videos["AVI"] = new Explorer_filter::FileType("AVI", "", ".avi", _("AVI Video file"), ICO_TREE_AVI) ;

	//OTHERS
	others["INFO"] = new Explorer_filter::FileType("INFO", "", ".info", _("INFO file"), ICO_TREE_INFO) ;
	others["BAK"] = new Explorer_filter::FileType("BAK", "", ".back", _("BACKUP file"), ICO_TREE_BAK) ;

	others["PDF"] = new Explorer_filter::FileType("PDF", "", ".pdf", _("PDF document"), ICO_TREE_PDF) ;
	others["DOC"] = new Explorer_filter::FileType("DOC", "", ".doc", _("WORD document"), ICO_TREE_WORD) ;
	others["XLS"] = new Explorer_filter::FileType("XLS", "", ".xls", _("EXCEL document"), ICO_TREE_EXCEL) ;
	others["PPT"] = new Explorer_filter::FileType("PPT", "", ".ppt", _("POWERPOINT presentation"), ICO_TREE_PPT) ;
	others["SH"] = new Explorer_filter::FileType("SH", "", ".sh", _("SHELL script"), ICO_TREE_SH) ;

	others["JAR"] = new Explorer_filter::FileType("JAR", "", ".jar", _("TEXT file"), ICO_TREE_JAR) ;
	others["TAR"] = new Explorer_filter::FileType("TAR", "", ".tar", _("Archive"), ICO_TREE_TAR) ;
	others["RAR"] = new Explorer_filter::FileType("RAR", "", ".rar", _("WINRAR archive"), ICO_TREE_RAR) ;
	others["ZIP"] = new Explorer_filter::FileType("ZIP", "", ".zip", _("WINZIP archive"), ICO_TREE_ZIP) ;
	others["RPM"] = new Explorer_filter::FileType("RPM", "", ".rpm", _("RPM Package"), ICO_TREE_RPM) ;
	others["TGZ"] = new Explorer_filter::FileType("TGZ", "", ".tgz", _("Compressed archive"), ICO_TREE_TGZ) ;
	others["A"] = new Explorer_filter::FileType("A", "", ".a", _("Static library"), ICO_TREE_LIBA) ;

	others["O"] = new Explorer_filter::FileType("O", "", ".o", _("C/C++ object file"), ICO_TREE_CO) ;
	others["CPP"] = new Explorer_filter::FileType("CPP", "", ".cpp", _("C++ source file"), ICO_TREE_CPP) ;
	others["CC"] = new Explorer_filter::FileType("CC", "", ".cc", _("C++ source file"), ICO_TREE_CPP) ;
	others["C"] = new Explorer_filter::FileType("C", "", ".c", _("C source file"), ICO_TREE_C) ;
	others["H"] = new Explorer_filter::FileType("H", "", ".h", _("C/C++ header file"), ICO_TREE_H) ;
	others["JAVA"] = new Explorer_filter::FileType("JAVA", "", ".java", _("Java source file"), ICO_TREE_JAVA) ;
	others["CLASS"] = new Explorer_filter::FileType("CLASS", "", ".class",_("Java class file"), ICO_TREE_JAVACLASS) ;
	others["PHP"] = new Explorer_filter::FileType("PHP", "", ".php", _("PHP file"), ICO_TREE_PHP) ;

	//> SPECIALS
	others["FOLDER"] = folder ;
	others["NONE"] = none ;
	others["ALL"] = all ;

	//> ALL CONFIGURATED ANNOTATIONS
	Formats* formats = Formats::getInstance() ;
	if (formats)
	{
		const std::vector<string> agformats = formats->getFormats() ;
		std::vector<string>::const_iterator it ;
		for (it=agformats.begin(); it!=agformats.end(); it++)
		{
			string ext, display, ico, type ;
			bool is_importable;
			ext = formats->getExtensionFromFormat(*it) ;
			display = formats->getDisplayFromFormat(*it) ;
			type = formats->getTypeFromFormat(*it) ;
			is_importable = formats->isImport(*it) ;
			ico = ICO_TREE_TRS ;
			if (ext==".tag" && *it=="TransAG")
				ico = ICO_TREE_TAG ;

			annotations[*it] = new Explorer_filter::FileType(*it, type, ext, display, ico, false, is_importable) ;
		}
	}
}


//******************************************************************************
//******************************************************************* COMBO ****
//******************************************************************************

Gtk::ComboBoxText* Explorer_filter::get_combo()
{
	return combo ;
}

void Explorer_filter::set_filter_chosen(Glib::ustring s)
{
	filter_chosen = s ;
}

void Explorer_filter::set_combo(Glib::ustring default_option)
{
	if (!combo)
		return ;

	combo->clear_items() ;

	//> configure possibilities
	combo->append_text(get_DISPLAY_AUDIONOTE());
	combo->append_text(get_DISPLAY_AUDIO());
	combo->append_text(get_DISPLAY_VIDEO());
	combo->append_text(get_DISPLAY_TAG());
	combo->append_text(get_DISPLAY_ALL() );

	//> default option
	combo->set_active_text(default_option) ;
}


//******************************************************************************
//******************************************************** FILE IDENTICATION ***
//******************************************************************************

bool Explorer_filter::is_file_in_filter(Glib::ustring name)
{
	bool res = false ;
	Glib::ustring ext = get_extension(name) ;
	ext = ext.lowercase() ;

	//don't display backup file
	if (ext.find('~',0)!=std::string::npos )
		return false ;
	else {
		if ( filter_chosen.compare(get_DISPLAY_AUDIONOTE())==0 ) {
			if ( is_audio_file(name) || is_import_annotation_file(name) )
				res = true ;
		}
		else if ( filter_chosen.compare(get_DISPLAY_AUDIO())==0 ) {
			res = is_audio_file(name) ;
		}
		else if ( filter_chosen.compare(get_DISPLAY_TAG())==0 ) {
			if ( is_tag_file(name) )
				res = true ;
		}
		else if ( filter_chosen.compare(get_DISPLAY_VIDEO())==0 ) {
				res = is_video_file(name) ;
		}
		else if ( filter_chosen.compare(get_DISPLAY_ALL())==0 ) {
				res = true ;
		}
		else
			return false ;
	}
	return res ;
}

bool Explorer_filter::is_audio_file(Glib::ustring path)
{
	Glib::ustring ext = get_extension(path) ;
	ext = ext.lowercase() ;
	return is_extension_in_map(ext, audios) ;
}

bool Explorer_filter::is_tag_file(Glib::ustring path)
{
	Glib::ustring ext = get_extension(path) ;
	ext = ext.lowercase() ;
	if (ext==".tag")
		return true ;
	else
		return false ;
}

bool Explorer_filter::is_annotation_file(Glib::ustring path)
{
	Glib::ustring ext = get_extension(path) ;
	ext = ext.lowercase() ;
	return is_extension_in_map(ext, annotations) ;
}

bool Explorer_filter::is_import_annotation_file(Glib::ustring path)
{
	Glib::ustring ext = get_extension(path) ;
	ext = ext.lowercase() ;

	if (ext.empty())
		return false ;

	std::map<Glib::ustring, Explorer_filter::FileType*>::iterator it ;
	Formats* formats = Formats::getInstance() ;

	if (!formats)
		return false ;

	for (it = annotations.begin(); it!=annotations.end(); it++)
	{
		FileType* fileT = it->second ;
		if (fileT )
		{
			string fileExt = (fileT->extension).lowercase() ;
			if ( fileExt==ext && formats->isImport(fileT->format) )
				return true ;
		}
	}
	return false ;
}

bool Explorer_filter::is_video_file(Glib::ustring path)
{
	Glib::ustring ext = get_extension(path) ;
	ext = ext.lowercase() ;
	return is_extension_in_map(ext, videos) ;
}

bool Explorer_filter::is_image_file(Glib::ustring path)
{
	Glib::ustring ext = get_extension(path) ;
	ext = ext.lowercase() ;
	return is_extension_in_map(ext, images) ;
}

bool Explorer_filter::is_extension_in_map(Glib::ustring ext, const std::map<Glib::ustring, FileType*> formatsMap)
{
	if (ext.empty())
		return false ;

	std::map<Glib::ustring, Explorer_filter::FileType*>::const_iterator it ;
	for (it=formatsMap.begin(); it!=formatsMap.end(); it++)
	{
		FileType* ft = it->second ;
		if (ft && ft->extension==ext)
			return true ;
	}
	return false ;
}

//**************************************************************************************
//**************************************************************************** EXTENSION
//**************************************************************************************

Glib::ustring Explorer_filter::get_display_from_ext(Glib::ustring extension)
{
	Glib::ustring ext = extension.lowercase() ;
	std::map<Glib::ustring, Explorer_filter::FileType*>::iterator it ;

	for (it=annotations.begin(); it!=annotations.end(); it++)
	{
		FileType* ft = it->second ;
		if (ft)
		{
			std::string ftExt = (ft->extension).lowercase() ;
			if (ftExt==ext)
				return ft->display ;
		}
	}

	for (it = audios.begin(); it!=audios.end(); it++)
	{
		FileType* ft = it->second ;
		if (ft)
		{
			std::string ftExt = (ft->extension).lowercase() ;
			if (ftExt==ext)
				return ft->display ;
		}
	}

	for (it = videos.begin(); it!=videos.end(); it++)
	{
		FileType* ft = it->second ;
		if (ft)
		{
			std::string ftExt = (ft->extension).lowercase() ;
			if (ftExt==ext)
				return ft->display ;
		}
	}

	for (it = images.begin(); it!=images.end(); it++)
	{
		FileType* ft = it->second ;
		if (ft)
		{
			std::string ftExt = (ft->extension).lowercase() ;
			if (ftExt==ext)
				return ft->display ;
		}
	}

	for (it = others.begin(); it!=others.end(); it++)
	{
		FileType* ft = it->second ;
		if (ft)
		{
			std::string ftExt = (ft->extension).lowercase() ;
			if (ftExt==ext)
				return ft->display ;
		}
	}

	return none->extension ;
}

Glib::ustring Explorer_filter::get_icon_from_ext(Glib::ustring extension)
{
	Glib::ustring ext = extension.lowercase() ;
	std::map<Glib::ustring, Explorer_filter::FileType*>::iterator it ;

	for (it=annotations.begin(); it!=annotations.end(); it++)
	{
		FileType* ft = it->second ;
		if (ft)
		{
			std::string ftExt = (ft->extension).lowercase() ;
			if (ftExt==ext)
				return ft->icon ;
		}
	}

	for (it = audios.begin(); it!=audios.end(); it++)
	{
		FileType* ft = it->second ;
		if (ft)
		{
			std::string ftExt = (ft->extension).lowercase() ;
			if (ftExt==ext)
				return ft->icon ;
		}
	}

	for (it = videos.begin(); it!=videos.end(); it++)
	{
		FileType* ft = it->second ;
		if (ft)
		{
			std::string ftExt = (ft->extension).lowercase() ;
			if (ftExt==ext)
				return ft->icon ;
		}
	}

	for (it = images.begin(); it!=images.end(); it++)
	{
		FileType* ft = it->second ;
		if (ft)
		{
			std::string ftExt = (ft->extension).lowercase() ;
			if (ftExt==ext)
				return ft->icon ;
		}
	}

	for (it = others.begin(); it!=others.end(); it++)
	{
		FileType* ft = it->second ;
		if (ft)
		{
			std::string ftExt = (ft->extension).lowercase() ;
			if (ftExt==ext)
				return ft->icon ;
		}
	}

	return ICO_TREE_DEFAULT ;
}

std::vector<Glib::ustring>  Explorer_filter::get_extensions_from_map(const std::map<Glib::ustring, FileType*> map)
{
	std::vector<Glib::ustring> extensions ;
	std::map<Glib::ustring, FileType*>::const_iterator it_conv ;
	for (it_conv=map.begin(); it_conv!=map.end(); it_conv++) {
		if (it_conv->second)
			extensions.push_back(it_conv->second->extension) ;
	}
	return extensions ;
}

std::vector<Glib::ustring> Explorer_filter::get_audio_extensions()
{
	return get_extensions_from_map(audios) ;
}

std::vector<Glib::ustring> Explorer_filter::get_annotation_extensions()
{
	return get_extensions_from_map(annotations) ;
}

std::vector<Glib::ustring> Explorer_filter::get_video_extensions()
{
	return get_extensions_from_map(videos) ;
}


std::map<Glib::ustring, Glib::ustring> Explorer_filter::get_import_annotations()
{
	std::map<Glib::ustring, Glib::ustring> res ;
	std::map<Glib::ustring, FileType*>::iterator it ;
	for (it=annotations.begin(); it!=annotations.end(); it++)
	{
		FileType* ft = it->second ;
		Formats* formats = Formats::getInstance() ;
		if (ft && formats)
		{
			if (formats->isImport(ft->format))
				res[ft->format] = ft->extension ;
		}
	}
	return res ;
}

std::map<Glib::ustring, Glib::ustring> Explorer_filter::get_export_annotations()
{
	std::map<Glib::ustring, Glib::ustring> res ;
	std::map<Glib::ustring, FileType*>::iterator it ;
	for (it=annotations.begin(); it!=annotations.end(); it++)
	{
		FileType* ft = it->second ;
		Formats* formats = Formats::getInstance() ;
		if (ft && formats)
		{
			if (formats->isExport(ft->format))
				res[ft->format] = ft->extension ;
		}
	}
	return res ;
}

//**************************************************************************************
//**************************************************************************** ICONS
//**************************************************************************************

/* Return icone path for file extension or type */
// root type:
//-1-> all
// 2-> filesystem folder
// 3-> project tree folder
// 6-> shortcut folder
Glib::ustring Explorer_filter::switch_ico(Glib::ustring path, int root_type)
{
	Glib::ustring res;

	if (path=="" && root_type == -1)
		res = ICO_TREE_DEFAULT ;
	else if (path=="" && root_type == 4)
		res = ICO_TREE_DIR ;
	else {
		//> icon for directory
		if ( Glib::file_test(path, Glib::FILE_TEST_IS_DIR) )
		{
			if (root_type==3)
				res = ICO_ROOT_PROJECT ;
			else if (root_type==2)
				res = ICO_ROOT_FS ;
			else if (root_type==6)
				res = ICO_ROOT_PERSONNAL ;
			else
				res = ICO_TREE_DIR ;
		}//> icon for all file types
		else
			res = icons_by_extension(path) ;
	}
	return res ;
}

Glib::ustring Explorer_filter::icons_by_extension(Glib::ustring file_name)
{
	std::string ext = get_extension(file_name) ;
	return get_icon_from_ext(ext) ;
}


/* Return extension of a file */
Glib::ustring Explorer_filter::get_extension(Glib::ustring name)
{
	int last = -1 ;
	size_t size = name.size() ;
	Glib::ustring ext ;
	guint i ;

	if (name!="")
	{
		for (i=0; i<size-1; i++) {
			if ( name[i]=='.' )
				last = i ;
		}
		if (last>0)
			ext = name.substr(last,size-1) ;
		else if (last==0)
			ext = name ;
		else
			ext = EXT_NONE ;
	}
	else
		ext = EXT_NONE ;
	return ext.lowercase() ;

}

/* Return name file without extension */
Glib::ustring Explorer_filter::cut_extension(Glib::ustring name)
{
	int last = 0 ;
	size_t size = name.size() ;
	Glib::ustring cut ;
	guint i ;

	for (i=0; i<size-1; i++) {
		if ( name[i]=='.' )
			last = i ;
	}
	if (last!=0) {
		cut = name.substr(0, last) ;
	}
	else
		cut=name ;
	return cut ;
}


/*
 * check if audio file exists in given directory
 *  test with standard audio extensions
 */
std::string Explorer_filter::lookForMediaFile(const std::string& dir, const std::string& name, const std::string& myType)
{
	string path = FileInfo(dir).realpath();
	if ( path == "" )
		return "";

	TRACE << "\t\t\t\t searching " << myType << " for " << name << " in " << dir << std::endl ;

	path = FileInfo(path).join(name);
	Explorer_filter* filter = Explorer_filter::getInstance() ;

	// NO EXTENSION GIVEN
	if ( strcmp(FileInfo(path).tail(), "") == 0  )
	{
		//> CHECK WITH AUDIOS FILE ALLOWED BY FILTER
		if (filter)
		{
			TRACE << "\t\t\t\t\t filter [ok]" << std::endl ;

			std::vector<Glib::ustring>::iterator it ;
			std::vector<Glib::ustring> extensions ;

			if (myType=="audio")
				extensions = filter->get_audio_extensions() ;
			else if (myType == "video")
				extensions = filter->get_video_extensions() ;

			bool found = false;
			for (it = extensions.begin(); it != extensions.end() && !found; it ++)
			{
				TRACE << "\t\t\t\t\t\t checking " << *it << std::endl ;

				string extUP = (*it).uppercase() ;
				string extLOW = (*it).lowercase() ;
				string np1 = path +  extUP ;
				string np2 = path +  extLOW ;
				// ext UP
				if ( FileInfo(np1).exists() && !FileInfo(np1).isDirectory()) {
					path = np1 ;
					found = true ;
				}
				// ext LOW
				else if ( FileInfo(np2).exists() && !FileInfo(np2).isDirectory()) {
					path = np2 ;
					found = true ;
				}
			}
		}
		//> OTHERWISE CHECK FROM STATIC DEFAULT LIST
		else
		{
			if (myType == "audio")
			{
				for ( int i = 0; StdAudioExt[i] != NULL; ++i ) 	{
					string np = path + StdAudioExt[i];
					if ( FileInfo(np).exists() && !FileInfo(np).isDirectory()) {
						path = np;
						break;
					}
				}
			}
			else if (myType == "video")
			{
				for ( int i = 0; StdVideoExt[i] != NULL; ++i ) 	{
					string np = path + StdVideoExt[i];
					if ( FileInfo(np).exists() && !FileInfo(np).isDirectory()) {
						path = np;
						break;
					}
				}
			}
		}
	}

	// check for good mimetype (SPECIALLY NEEDED IF AN EXTENSION WAS GIVEN)
	if ( FileInfo(path).exists() && !FileInfo(path).isDirectory())
	{
		if (myType=="audio" && filter->is_audio_file(path)
					|| myType=="video" && filter->is_video_file(path))
		return path;
	}
	return "";
}

/*
 * check if audio file exists in given directory
 *  test with standard audio extensions
 */
std::string Explorer_filter::lookForFile(const std::string& dir, const std::string& name, const std::string& type)
{
	string path = FileInfo(dir).realpath();
	if ( path == "" )
		return "";

	path = FileInfo(path).join(name);

	Glib::ustring ext = "." + type ;

	string extUP = ext.uppercase() ;
	string extLOW = ext.lowercase() ;
	string np1 = path +  extUP ;
	string np2 = path +  extLOW ;

	// ext UP
	if ( FileInfo(np1).exists() && !FileInfo(np1).isDirectory())
		path = np1 ;
	// ext LOW
	else if ( FileInfo(np2).exists() && !FileInfo(np2).isDirectory())
		path = np2 ;

	// check for good mimetype (SPECIALLY NEEDED IF AN EXTENSION WAS GIVEN)
	if ( FileInfo(path).exists() && !FileInfo(path).isDirectory())
		return path;
	return "" ;
}

} //namespace
