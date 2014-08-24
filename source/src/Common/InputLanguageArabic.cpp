/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "InputLanguageArabic.h"

#include <glibmm.h>
#include <glib.h>
#include <iostream>
#include <stdlib.h>
#include "Common/globals.h"

#define XML_ISOLATED_FORM 						"isolated_form"
#define XML_INITIAL_FORM						"initial_form"
#define XML_MEDIAL_FORM							"medial_form"
#define XML_FINAL_FORM							"final_form"
#define XML_UNLINKED_CHAR						"Unlinked_char"
#define XML_IS_VOWEL							"is_vowel"
#define XML_NO_CHECK							"no_check"
#define XML_IS_PUNCTUATION						"is_punctuation"
#define XML_IS_COMPOSED							"is_composed"

namespace tag
{

InputLanguageArabic::InputLanguageArabic(std::string display, std::string lan, std::string shortcut, Glib::ustring mappingMode, bool modeLeft2Right, bool spaceSeparated, bool isActivated, bool modifyMapping)
:InputLanguage(display, lan, shortcut, mappingMode, modeLeft2Right, spaceSeparated, isActivated, modifyMapping)
{
}

InputLanguageArabic::~InputLanguageArabic()
{
	std::map<unsigned int, Unicode_Ar_Form*>::iterator ite;
	for(ite = m_formTable.begin(); ite != m_formTable.end(); ite++) {
		if ( (*ite).second )
		delete (*ite).second;
}
}


gunichar_type
InputLanguageArabic::get_gunichar_type(gunichar c)
{
	gunichar_type type = G_UNI_OTHER;
	if(is_gunichar_in_bound(c) && (is_punctuation(c) == false))
		type = G_UNI_ARABIC;
	else if(Glib::Unicode::isspace(c))
		type = G_UNI_SPACE;
	return type;
}

bool
InputLanguageArabic::is_punctuation(gunichar c)
{
	return g_unichar_ispunct(c) ;
}

unicode_val_flag
InputLanguageArabic::get_unicode_ar_type(gunichar_type before_type, gunichar_type after_type, bool is_char_before_unlinked)
{

	if(before_type != G_UNI_ARABIC && after_type != G_UNI_ARABIC)//no char after and before
		return AR_ISOLATED;

	if(before_type != G_UNI_ARABIC) //char before not of type arabic
	{
		if(after_type == G_UNI_SPACE) //followed by a space
			return AR_ISOLATED;
		else
			return AR_INITIAL;

	}
	if(after_type != G_UNI_ARABIC) //no char after
	{
		if(before_type == G_UNI_SPACE)//char before is a space
			return AR_ORIGINAL;
		else if (is_char_before_unlinked) //char before is not a space
			return AR_ISOLATED;
		else
			return AR_FINAL;
	}
	else
	{
		if(before_type == G_UNI_SPACE)//char before is a space-
		{
			if(after_type == G_UNI_SPACE)//char after is a space
				return AR_ISOLATED;
			else //char after is not a space
				return AR_INITIAL;
		}
		else //char before is not a space
		{
			if(after_type == G_UNI_SPACE) //char after is a space
				if(is_char_before_unlinked == false)//char before can be linked
					return AR_FINAL;
				else
					return AR_ISOLATED;
			else //char after is not a space
				if(is_char_before_unlinked == false) //char before can be linked
					return AR_MEDIAL;
				else
					return AR_INITIAL;
		}
	}
	return AR_NONE;
}

void
InputLanguageArabic::postLoadingKeyMap(xercesc::DOMNode *node, gunichar c)
{
	if(node == NULL)
		return;
	xercesc::DOMNode *childNode = node->getFirstChild();
	xercesc::DOMNode *attrNode;
	char *buf, *buf2, *stopPtr;
	unsigned int keyVal;
	XMLCh* ch;
	xercesc::DOMNamedNodeMap *attrList;
	Unicode_Ar_Form *ar_form = new Unicode_Ar_Form();
	ar_form->available_types = (unicode_val_flag)(ar_form->available_types | AR_ORIGINAL);
	ar_form->original_val = c;
	bool composed = false ;

	while(childNode)
	{
		if(childNode->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
		{
			attrList = childNode->getAttributes();
			if(attrList == NULL)
				continue;
			buf = xercesc::XMLString::transcode(childNode->getNodeName());
			ch = xercesc::XMLString::transcode(XML_VALUE);
			attrNode = attrList->getNamedItem(ch);
			xercesc::XMLString::release(&ch);
			if(attrNode)
			{
				buf2 = xercesc::XMLString::transcode(attrNode->getNodeValue());
				keyVal = (unsigned int)strtoul(buf2, &stopPtr, 0);
				if(!strcasecmp("", stopPtr))
				{
					if(!strcasecmp(buf, XML_ISOLATED_FORM))
					{
						ar_form->isolated_val = keyVal;
						ar_form->available_types = (unicode_val_flag)(ar_form->available_types | AR_ISOLATED);
					}
					else if(!strcasecmp(buf, XML_INITIAL_FORM))
					{
						ar_form->initial_val = keyVal;
						ar_form->available_types = (unicode_val_flag)(ar_form->available_types | AR_INITIAL);
					}
					else if(!strcasecmp(buf, XML_MEDIAL_FORM))
					{
						ar_form->medial_val = keyVal;
						ar_form->available_types = (unicode_val_flag)(ar_form->available_types | AR_MEDIAL);
					}
					else if(!strcasecmp(buf, XML_FINAL_FORM))
					{
						ar_form->final_val = keyVal;
						ar_form->available_types = (unicode_val_flag)(ar_form->available_types | AR_FINAL);
					}
					else if(!strcasecmp(buf, XML_UNLINKED_CHAR))
					{
						ar_form->unlinked_char = keyVal ? true : false;
					}
					else if(!strcasecmp(buf, XML_IS_VOWEL))
					{
						ar_form->is_vowel = keyVal ;
					}
					else if(!strcasecmp(buf, XML_IS_PUNCTUATION))
					{
						ar_form->is_punctuation = keyVal ? true : false;
					}
					else if(!strcasecmp(buf, XML_IS_COMPOSED))
					{
						ar_form->is_composed = keyVal ? true : false;
						composed = ar_form->is_composed ;
					}
					else if(!strcasecmp(buf, XML_NO_CHECK))
					{
						ar_form->no_check = keyVal ;
					}
				}
				xercesc::XMLString::release(&buf2);
			}
			xercesc::XMLString::release(&buf);
		}
		childNode = childNode->getNextSibling();
	}
	//> just add in map information for not composed element
	// composed element will be used retrieving elements that compose them
	if (!composed)
		m_formTable[c] = ar_form;

}

void
InputLanguageArabic::postProcessing(Gtk::TextView *view, Gtk::TextIter iter)
{
	if(!view)
		return;
	Glib::RefPtr<Gtk::TextBuffer> buffer = view->get_buffer();
	int offset = iter.get_offset();
	gunichar c = iter.get_char();

	if(iter != buffer->end())
		update_char_at_iter(view, iter);

	if(offset > 0)
	{
		Gtk::TextIter tmp = view->get_buffer()->get_iter_at_offset(offset-1);
		if(tmp != buffer->begin())
		{
			c = tmp.get_char();
 			update_char_at_iter(view, tmp);
			if(offset > 2)
			{
				tmp = view->get_buffer()->get_iter_at_offset(offset-2);
				c = tmp.get_char();
				//JUMP VOWEL FOR REACHING BASE LETTER
				while(tmp != buffer->begin() && is_vowel(c) == true)
				{
					tmp.backward_char();
					c = tmp.get_char();
				}
				update_char_at_iter(view, tmp);
			}
		}
	}
}

bool
InputLanguageArabic::proceedDeletion(GdkEventKey *event, Gtk::TextView  *view, Gtk::TextIter it)
{
	//weird management of backsapce event by default ==> so override it
	Gtk::TextIter tmp = it;

	//manage backspace key
	if(event->keyval == GDK_BackSpace && it != view->get_buffer()->begin())
	{
		Gtk::TextIter before = it;
		before.backward_char();
		tmp = view->get_buffer()->erase(before, it);
	}
	//manage delete key
	else if((event->keyval == GDK_Delete || event->keyval == GDK_KP_Delete) && it != view->get_buffer()->end())
	{
		Gtk::TextIter after = it;
		after.forward_char();
		gunichar c = after.get_char();
		while(is_vowel(c) == true && after != view->get_buffer()->end())
		{
			after++;
			c = after.get_char();
		}
		tmp = view->get_buffer()->erase(it, after);

	}
	//-------- update chars--------
	if(tmp != view->get_buffer()->end())
	{
		int offset = tmp.get_offset();
		update_char_at_iter(view, tmp);
		tmp = view->get_buffer()->get_iter_at_offset(offset);
	}
	if(tmp != view->get_buffer()->begin())
	{
		tmp--;
		gunichar c = tmp.get_char();
		while(is_vowel(c) == true && tmp != view->get_buffer()->begin())
		{
			tmp--;
			c = tmp.get_char();
		}
		update_char_at_iter(view, tmp);
	}
	return true;
}

gunichar
InputLanguageArabic::get_unicode_ar_form(gunichar c, unicode_val_flag type)
{
	Unicode_Ar_Form *form = NULL;
	std::map<unsigned int, Unicode_Ar_Form*>::const_iterator ite;
	gunichar ret = c;
	if((ite = m_formTable.find(c)) != m_formTable.end())
	{
		form = (*ite).second;
		switch(type)
		{
			case AR_ORIGINAL:
				if(form->available_types & AR_ORIGINAL)
					ret = form->original_val;
				break;
			case AR_INITIAL:
				if(form->available_types & AR_INITIAL)
					ret = form->initial_val;
				break;
			case AR_MEDIAL:
				if(form->available_types & AR_MEDIAL)
					ret = form->medial_val;
				break;
			case AR_FINAL:
				if(form->available_types & AR_FINAL)
					ret = form->final_val;
				break;
			case AR_ISOLATED:
				if(form->available_types & AR_ISOLATED)
					ret = form->isolated_val;
				break;
			default:
				break;
		}
	}
	return ret;
}


void
 InputLanguageArabic::update_char_at_iter(Gtk::TextView *view, Gtk::TextIter it)
 {


	if(!view || it.editable() == false)
		return;
	Glib::RefPtr<Gtk::TextBuffer> buffer = view->get_buffer();
	 if(it == buffer->end())//end of buffer ==> do nothing
		 return;
	Gtk::TextIter itBefore = it;
	Gtk::TextIter tmp = it;
	Gtk::TextIter afterCurrent = it;
	tmp++;
	bool is_unlinked_char = false;
	gunichar_type afterType = G_UNI_OTHER;
	gunichar_type beforeType = G_UNI_OTHER;

	itBefore--;
	afterCurrent++;

	gunichar cCurrent = it.get_char();
	if(cCurrent == 0 /*|| get_gunichar_type(cCurrent) != G_UNI_ARABIC*/)
		 return;
	if(cCurrent != 0)
		cCurrent = get_unicode_ar_original_unichar(cCurrent, is_unlinked_char);
	//bool is_vowel = is_vowel(cCurrent);
	gunichar cBefore = 0;
	gunichar cAfter = 0;
	 if(it == buffer->begin() && afterCurrent != buffer->end())//iter = beginning of textview
	 {
		 cAfter = afterCurrent.get_char();
		 while(is_vowel(cAfter) == true && afterCurrent != buffer->end())
		 {
			 afterCurrent++;
			 cAfter = afterCurrent.get_char();
		 }
		 afterType = InputLanguageArabic::get_gunichar_type(cAfter);
		 if(afterType != G_UNI_ARABIC)//char after is not arabic
			 cAfter = 0;
	 }
	 else
	 {
		 cBefore = itBefore.get_char();
		 while(is_vowel(cBefore) == true && itBefore != buffer->begin())
		 {
			 itBefore--;
			 cBefore = itBefore.get_char();
		 }
		 beforeType = InputLanguageArabic::get_gunichar_type(cBefore);
		 if(afterCurrent != buffer->end())
		 {
			 cAfter = afterCurrent.get_char();
			 while(is_vowel(cAfter) == true && afterCurrent != buffer->end())
			 {
				 afterCurrent++;
				 cAfter = afterCurrent.get_char();
			 }
			 afterType = InputLanguageArabic::get_gunichar_type(cAfter);
			 if(afterType != G_UNI_ARABIC)//char after is not of type arabic
				 cAfter = 0;
		 }
	 }
	 //get original form 4 string after cursor
	 if(cAfter != 0)
		 cAfter = get_unicode_ar_original_unichar(cAfter, is_unlinked_char);

	 //get original form 4 string before cursor
	 if(cBefore != 0)
		 cBefore = get_unicode_ar_original_unichar(cBefore, is_unlinked_char);

	 unicode_val_flag arType = InputLanguageArabic::get_unicode_ar_type(beforeType, afterType, is_unlinked_char);//linked char referes to the previous char

	 if(get_gunichar_type(cCurrent) == G_UNI_ARABIC)
		 cCurrent = this->get_unicode_ar_form(cCurrent, arType);
	 if(cCurrent == 0)
		 return;

 	 //replace string at cursor position with its new type
	 bool mustMoveBackCursor = false;
	 if(it == buffer->get_insert()->get_iter() && it != buffer->begin())
		 mustMoveBackCursor = true;
	 int offset = it.get_offset();
	 buffer->erase(it, tmp);
	 if(is_vowel(cCurrent) == true  && (is_punctuation(cBefore) == true || beforeType != G_UNI_ARABIC))//do not add vowel if previous char is not arabic
		 return;
	 afterCurrent = buffer->get_iter_at_offset(offset);
	 buffer->insert(afterCurrent, Glib::ustring(1, cCurrent)) ; /*, g_unichar_to_utf8(cCurrent, NULL));*/
	 if(mustMoveBackCursor == true)
	 {
		 afterCurrent = buffer->get_insert()->get_iter();
		 afterCurrent--;
		 buffer->place_cursor(afterCurrent);
	 }
}

gunichar
InputLanguageArabic::get_unicode_ar_original_unichar(gunichar c, bool & is_linked_char)
{
	std::map<unsigned int, Unicode_Ar_Form*>::const_iterator ite;
	for(ite = m_formTable.begin(); ite != m_formTable.end(); ite++)
	{
		if ( (*ite).second )
		{
		unicode_val_flag type = (*ite).second->available_types;
		if(((type & AR_ORIGINAL) && (*ite).second->original_val == c) 	||
			((type & AR_INITIAL) && (*ite).second->initial_val == c) 	||
			((type & AR_MEDIAL) && (*ite).second->medial_val == c) 		||
			((type & AR_FINAL) && (*ite).second->final_val == c)       ||
			((type & AR_ISOLATED) && (*ite).second->isolated_val == c))
		{
			is_linked_char = (*ite).second->unlinked_char;
			return (*ite).second->original_val;
		}
	}
		else {
			Log::err() << "<!> ar-form null InputLanguageArabic:> get_unicode_ar_original_unichar" << std::endl ;
		}
	}
return c;
}

bool
InputLanguageArabic::is_vowel(gunichar c)
{
	if(c == 0)
		return false;

	std::map<unsigned int, Unicode_Ar_Form*>::const_iterator ite;
	for(ite = m_formTable.begin(); ite != m_formTable.end(); ite++)
	{
		if ( (*ite).second ) 	{
			unicode_val_flag type = (*ite).second->available_types;
			if(((type & AR_ORIGINAL) && (*ite).second->original_val == c) 	||
				((type & AR_INITIAL) && (*ite).second->initial_val == c) 	||
				((type & AR_MEDIAL) && (*ite).second->medial_val == c) 		||
				((type & AR_FINAL) && (*ite).second->final_val == c)       ||
				((type & AR_ISOLATED) && (*ite).second->isolated_val == c))
			{
				int value = (*ite).second->is_vowel ;
				if (value==1 || value==2)
					return true ;
			}
		}
		else {
			Log::err() << "<!> InputLanguageArabic::is_vowel:> ar-form null" << std::endl ;
		}
	}
	return false ;
}

bool InputLanguageArabic::is_nocheck(gunichar c)
{
	if(c == 0)
		return false;

	std::map<unsigned int, Unicode_Ar_Form*>::const_iterator ite;
	for(ite = m_formTable.begin(); ite != m_formTable.end(); ite++)
	{
		if ( (*ite).second ) 	{
			unicode_val_flag type = (*ite).second->available_types;
			if(((type & AR_ORIGINAL) && (*ite).second->original_val == c) 	||
				((type & AR_INITIAL) && (*ite).second->initial_val == c) 	||
				((type & AR_MEDIAL) && (*ite).second->medial_val == c) 		||
				((type & AR_FINAL) && (*ite).second->final_val == c)       ||
				((type & AR_ISOLATED) && (*ite).second->isolated_val == c))
			{
				int value = (*ite).second->no_check ;
				if (value==1)
					return true ;
			}
		}
		else {
			Log::err() << "<!> InputLanguageArabic::no_check:> ar-form null" << std::endl ;
		}
	}
	return false ;
}


void InputLanguageArabic::remove_vowel_and_nocheck_string(const Glib::ustring& s, Glib::ustring& res)
{
	res.clear() ;
	Glib::ustring::const_iterator ite ;
	for(ite=s.begin(); ite!=s.end(); ite++)
	{
		// if no vowel and no no-check word Go !
		// in same time, remove presentation character if there are som
		if ( !is_vowel(*ite) && !is_nocheck(*ite) && !is_presentation_character(*ite) )
			res = res + *ite ;
	}
}

char* InputLanguageArabic::remove_vowel_and_nocheck(const char* s)
{
	char* res ;
	Glib::ustring su = Glib::ustring(s) ;
	Glib::ustring resu ;
	InputLanguageArabic::remove_vowel_and_nocheck_string(su,resu) ;
	res = strdup(resu.c_str()) ;
	return res  ;
}

Glib::ustring InputLanguageArabic::special_string_format(const Glib::ustring& s)
{
	Glib::ustring res ;
	remove_vowel_and_nocheck_string(s, res) ;
	return res ;
}


Glib::ustring InputLanguageArabic::processingString(Glib::ustring s)
{
	bool is_unlinked_char = false ;
	gunichar_type afterType = G_UNI_OTHER;
	gunichar_type beforeType = G_UNI_OTHER;

	Glib::ustring::const_iterator ite;
	gunichar cBefore, cCurrent, cAfter;
	std::string str = "";
	for(int i = 0; i < s.size(); i++)
	{
		afterType = G_UNI_OTHER;
		beforeType = G_UNI_OTHER;
		int j = i;
		while(j > 0)
		{
			cBefore = s[j-1];
			beforeType = InputLanguageArabic::get_gunichar_type(cBefore);
			if(is_vowel(cBefore) == false || beforeType != G_UNI_ARABIC)
				break;
			j--;
		}
		if(j == 0 || is_vowel(cBefore))
		{
			cBefore = 0;
			if(beforeType == G_UNI_ARABIC)
				beforeType = G_UNI_OTHER;
		}

		j = i;
		while(j < (s.size() -1))
		{
			cAfter = s[j+1];
			afterType = InputLanguageArabic::get_gunichar_type(cAfter);
			if(is_vowel(cAfter) == false || afterType != G_UNI_ARABIC)
				break;
			j++;
		}
		if (j == (s.size()-1) || is_vowel(cAfter))
		{
			cAfter = 0;
			if(afterType == G_UNI_ARABIC)
				afterType = G_UNI_OTHER;
		}

		cCurrent = s[i];

		//get original form 4 string after cursor
		if(cAfter != 0)
			cAfter = get_unicode_ar_original_unichar(cAfter, is_unlinked_char);

		//get added string type according to chars on both side of the cursor
		if(cCurrent != 0)
			cCurrent = get_unicode_ar_original_unichar(cCurrent, is_unlinked_char);

		 //get original form 4 string before cursor
		if(cBefore != 0)
			cBefore = get_unicode_ar_original_unichar(cBefore, is_unlinked_char);

		unicode_val_flag arType = InputLanguageArabic::get_unicode_ar_type(beforeType, afterType, is_unlinked_char);//linked char referes to the previous char
		cCurrent = this->get_unicode_ar_form(cCurrent, arType);

		if(cCurrent == 0)
			 str += Glib::ustring(1, this->get_unicode_ar_form(s[i], AR_ISOLATED));
		 else
			 str += Glib::ustring(1,cCurrent);

	}
	return str ;
}

Glib::ustring InputLanguageArabic::unprocessingString(Glib::ustring s)
{
	Glib::ustring res ;
	bool unused ;
	for(int i = 0; i < s.size(); i++)
	{
		gunichar current ;
		if ( get_gunichar_type(s[i])==G_UNI_ARABIC ) {
			current = get_unicode_ar_original_unichar(s[i], unused) ;
		}
		else
			current = s[i] ;
		res = res + Glib::ustring(1, current);
	}
	return res ;
}

bool InputLanguageArabic::check_insertion_rules(Gtk::TextView *view, const Gtk::TextIter& iter, const Glib::ustring& s)
{
	if(!view )  return false ;

	Glib::RefPtr<Gtk::TextBuffer> buffer = view->get_buffer();
	Gtk::TextIter tmp = iter ;
	tmp-- ;
	bool ok = check_vowels_and_shadda(tmp.get_char(), *(s.begin()), iter.is_start());
	/** EVENTUALLY ADD RULES FOR OTHER ARABIC INSERT **/
	return ok ;
}

/**
 * 2 successive vowels not allowed, and insertion of vowel or shadda at the beginning of text forbidden
 */
bool InputLanguageArabic::check_vowels_and_shadda(gunichar char_before, gunichar toBeInserted, bool at_start)
{
	Unicode_Ar_Form* struct_current = m_formTable[toBeInserted] ;

	if ( struct_current == NULL ) return true;

	//> RULES FOR VOWELS AND SHADDA
	if ( struct_current->is_vowel ==1 ||  struct_current->is_vowel==2 )
	{
		//> can't insert a vowel or shadda at the beginning of text
		if (at_start)
			return false ;

		//> can't insert a vowel without any letter, or after a punctuation
		if (g_unichar_isspace(char_before) || is_punctuation(char_before) )
			return false ;

		Unicode_Ar_Form* struct_before = NULL ;
		struct_before = m_formTable[char_before] ;

		if (struct_before!=NULL) {
			//> can't insert a shadda/a vowel after a vowel
			if ( struct_before->is_vowel==1  )
				return false ;
			//> can't insert a shadda after a shadda
			if ( struct_current->is_vowel==2 && struct_before->is_vowel==2)
				return false ;
		}
		//> no structure found , allow insertion (shouldn't be a arabic symbol)
		return true ;
	}

	return true;
}


bool InputLanguageArabic::check_insertion_rules_str(const Glib::ustring& text_before_inserted, int pos, const Glib::ustring& toBeInserted)
{
	//compute char before
	gunichar char_before = 0 ;
	if ( pos > 0 && !(pos>text_before_inserted.size()))
		char_before = text_before_inserted[pos-1];

	/*	int cpt = 1 ;
	Glib::ustring::iterator it = text_before_inserted.begin() ;
	while (it!=text_before_inserted.end() && cpt!=-1) {
		if (cpt==pos) {
			char_before = *it ;
			cpt=-1 ;
		}
		else {
			cpt++ ;
			it++ ;
		}
	}*/

	bool ok = check_vowels_and_shadda(char_before, *(toBeInserted.begin()), (pos == 0));
	/** EVENTUALLY ADD RULES FOR OTHER ARABIC INSERT **/
	return ok ;
}

bool InputLanguageArabic::is_presentation_character(gunichar c)
{
	if ( c == InputLanguageArabic::RLM
			|| c == InputLanguageArabic::RLE
			|| c == InputLanguageArabic::RLM
			|| c == InputLanguageArabic::LRE
			|| c == InputLanguageArabic::LRM
			|| c == InputLanguageArabic::PDF
			|| c == InputLanguageArabic::RLO )
	{
		return true ;
	}
	else
		return false ;
}

int InputLanguageArabic::printArForm(bool display)
{
	int cpt = 0 ;
	if (display) {
		TRACE << "\n -----------[ printArForm ]---------------**" << std::endl ;

		std::map<unsigned int, Unicode_Ar_Form*>::const_iterator ite ;
		for(ite = m_formTable.begin(); ite != m_formTable.end(); ite++) {
			cpt++ ;
			if ( (*ite).second ) {
				TRACE << "ar-form= " <<std::hex<< (*ite).second->original_val << std::dec<< std::endl ;
			}
			else {
				TRACE  << "ar-form= null" << std::endl ;
			}
		}
		TRACE  << "**-----------[ printArForm ]---------------" << std::endl ;
	}
	TRACE  << "-----------[ NB= " << cpt <<"]---------------\n" << std::endl ;
	return cpt ;
}


}
