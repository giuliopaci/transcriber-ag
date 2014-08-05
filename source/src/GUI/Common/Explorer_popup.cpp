/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include "Explorer_popup.h"
#include <iostream>
#include "icons/IcoPackImage.h"

namespace tag {


//***************************************************************************************
//********************************************************************* CALLBACK FUNCTION
//***************************************************************************************

Explorer_popup::Explorer_popup()
{
	indice = 0 ;
}

Explorer_popup::~Explorer_popup()
{
	std::map<int, Gtk::Image*>::iterator it ;
	it = map_image.begin() ;

	while( it!=map_image.end()) {
		delete(it->second) ;
		it++ ;
	}

	std::vector<Gtk::MenuItem*>::iterator it2 = itemList.begin() ;
	while( it2!=itemList.end()) {
		Gtk::MenuItem *tmp = *it2 ;
		it2++ ;
		delete(tmp) ;
	}
}

void Explorer_popup::insert_item(Gtk::MenuItem* item, Gtk::Image* image, Glib::ustring name)
{
	if (!item)
		return ;

	if (image)
		map_image[indice] = image ;

	itemList.insert(itemList.end(),item) ;
	item->show() ;
	map_indice[name] = indice ;

	indice++ ;
}

Gtk::Image* Explorer_popup::create_image(Glib::ustring ico_path)
{
	IcoPackImage* image = new IcoPackImage() ;
	if (image)
		image->set_image(ico_path, 17) ;
	return image;
}

//***************************************************************************************
//******************************************************************************* METHODS
//***************************************************************************************


void Explorer_popup::enable_popup_paste(bool value)
{
	Gtk::MenuItem* item_paste = itemList[ map_indice[POPUP_TREE_PASTE] ] ;
	if (item_paste!=NULL)
		item_paste->set_sensitive(value) ;
}

/*
 * mode:
 * -1: not supported file
 *  1: directory
 *  2: system root directory
 *  3: personnal root directory
 *  4: for media file
 *	5: for annotation file
  */
void Explorer_popup::prepare_explorer_popup(int mode)
{
	Gtk::Menu::MenuList& menulist = items() ;

	//> supress all possibilities in popup
	menulist.clear();

	//> add possibilities for NOT SUPPORTED FILES
	if (mode==-1) {
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_RENAME] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_COPY] ] ) ) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_CUT] ] ) ) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_PASTE] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_SUPPRESS] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_PROPERTY] ] ) ) ;
	}
	//> CLASSIC DIRECTORIES
	else if (mode==1) {
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_REFRESHDIR] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_NEW_DIR] ]) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_RENAME] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_COPY] ] ) ) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_CUT] ] ) ) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_PASTE] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_DEFINESHORTCUT] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_SUPPRESS] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_PROPERTY] ] ) ) ;
	}
	//> SYSTEM ROOT
	else if (mode==2) {
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_REFRESHDIR] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_NEW_DIR] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_PASTE] ] ) ) ;
	}
	//> PROJECT ROOT
	else if (mode==3) {
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_REFRESHDIR] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_CHANGESHORTCUT] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_NEW_DIR] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_PASTE] ] ) ) ;
	}
	//> FTP Root
	else if (mode==4) {
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_REFRESHFTPDIR] ] ) ) ;
		/*menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(map_items[ map_indice[POPUP_TREE_PROPERTY] ] ) ) ;*/
	}
	//> cache FTP ROOT
	else if (mode==5) {
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_REFRESHDIR] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_CHANGESHORTCUT] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_NEW_DIR] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_PASTE] ] ) ) ;
	}
	//> shortcut ROOT
	else if (mode==6) {
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_REFRESHDIR] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_NEW_DIR] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_PASTE] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_CHANGESHORTCUT] ] ) ) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_DELETESHORTCUT] ] ) ) ;
	}
	//> AUDIO FILES
	else if(mode==101) {
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_OPEN] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_CREATE_SINGLETRANSCRIPTION] ] ) ) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_CREATE_MULTITRANSCRIPTION] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_RENAME] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_COPY] ] ) ) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_CUT] ] ) ) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_PASTE] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_SUPPRESS] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_PROPERTY] ] ) ) ;
	}
	//> ANNOTATION FILES
	else if(mode==102) {
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_OPEN] ] ) ) ;
		//menulist.push_back( *(map_items[ map_indice[POPUP_TREE_IMPORT_IN] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_RENAME] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_COPY] ] ) ) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_CUT] ] ) ) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_PASTE] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_SUPPRESS] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_PROPERTY] ] ) ) ;
	}
	//> FTP FILEs
	else if(mode==16) {
		menulist.push_back( *(itemList[ map_indice[POPUP_FTP_DOWNLOAD] ] ) ) ;
	}
	//> AUDIO FTP cache FILEs
	else if(mode==151) {
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_OPEN] ] ) ) ;
		//menulist.push_back( *(map_items[ map_indice[POPUP_TREE_PLAY] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_SUPPRESS] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_PROPERTY] ] ) ) ;
	}
	//> ANNOTATION FTP cache FILEs
	else if(mode==152) {
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_OPEN] ] ) ) ;
		//menulist.push_back( *(map_items[ map_indice[POPUP_TREE_IMPORT_IN] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_FTP_UPLOAD] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_COPY] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_SUPPRESS] ] ) ) ;
		menulist.push_back(Gtk::Menu_Helpers::SeparatorElem()) ;
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_PROPERTY] ] ) ) ;
	}
	//> FTP directory
	else if(mode==41) {
		menulist.push_back( *(itemList[ map_indice[POPUP_TREE_REFRESHFTPDIR] ] ) ) ;
	}
}


/*
 * mode:
 * 	0: for first element
 *  1: for last element
 *  2: for all
 */
void Explorer_popup::prepare_treatment_popup(int mode)
{
	Gtk::Menu::MenuList& menulist = items() ;

	//> supress all possibilities in popup
	menulist.clear();

	//> add possibilities for file
	if (mode!=0)
		menulist.push_back( *(itemList[ map_indice[POPUP_TREATMENT_UP] ]) ) ;
	if (mode!=1)
		menulist.push_back( *(itemList[ map_indice[POPUP_TREATMENT_DOWN] ]) ) ;
	menulist.push_back( *(itemList[ map_indice[POPUP_TREATMENT_PAUSE] ]) ) ;
	menulist.push_back( *(itemList[ map_indice[POPUP_TREATMENT_CANCEL] ]) ) ;
}

} //namespace
