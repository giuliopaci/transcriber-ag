/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include "InputLanguageHandler.h"
#include "InputLanguage.h"
#include "InputLanguageArabic.h"
#include "Common/globals.h"
#include "Languages.h"
#include "util/StringOps.h"

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <iostream>
#include <iterator>

#ifdef WIN32
	#include <gdk/gdkwin32.h>
#endif

using namespace std;

namespace tag
{


std::map<std::string, InputLanguage*> InputLanguageHandler::m_ilMap;

InputLanguageHandler::InputLanguageHandler()
{
	//m_current_input_language = get_input_language_by_shortcut(DEFAULT_LANGUAGE) ;
}

InputLanguage* InputLanguageHandler::defaultInputLanguage()
{
	return get_input_language_by_name(DEFAULT_LANGUAGE);
}


bool
InputLanguageHandler::load_language_map(const std::string & filePath, bool externalIME)
{
	try
	{
		xercesc::XMLPlatformUtils::Initialize();
	}
    catch (const xercesc::XMLException& toCatch)
    {
        char* message = xercesc::XMLString::transcode(toCatch.getMessage());
        Log::err() << "Error during initialization! :\n"
             << message << "\n";
        xercesc::XMLString::release(&message);
        return false;
    }

	xercesc::XercesDOMParser* parser = new xercesc::XercesDOMParser();

	std::vector<std::string> mappedTOime ;

	try
    {
    	parser->parse(filePath.c_str());
    	xercesc::DOMDocument *doc = parser->getDocument();
    	if(!doc)
    	{
    		delete parser;
    		return false;
    	}
    	InputLanguageHandler::free_resources();//if reload==>free resources
    	xercesc::DOMNode *rootNode = doc->getDocumentElement();
    	char *buf1 = xercesc::XMLString::transcode(rootNode->getNodeName());
    	if(!strcasecmp(buf1, XML_LANGUAGES))//<Languages>
    	{
	    	xercesc::DOMNode *childNode = rootNode->getFirstChild();
	    	xercesc::DOMNode *tmp1;
	    	xercesc::DOMNamedNodeMap *attrList;
	    	xercesc::DOMNode *attrNode, *attrNode2, *attrNode3, *attrNode4, *attrNode5, *attrNode6;
	    	char *buf2, *buf3, *buf4, *buf5, *buf6;
	    	XMLCh *ch1;
	    	InputLanguage *il;
	    	char *stopPtr, *stopPtr2, *stopPtr3;
	    	unsigned int gdkKeyVal, keyVal, hardware_code;
	    	std::string type;
	    	std::string shortcut;
	    	std::string spaceSeparated;
	    	std::string mappingMode ;
	    	while(childNode)
	    	{
	    		type = "";
	    		shortcut = "";

	    		if(childNode->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
	    		{
	    			buf2 = xercesc::XMLString::transcode(childNode->getNodeName());
	    			//> NIVEAU 0: balise Language
	    			if(!strcasecmp(buf2, XML_LANGUAGE))
					{
	    				attrList = childNode->getAttributes();
						if(attrList)
						{

							ch1 = xercesc::XMLString::transcode(XML_NAME);
							attrNode = attrList->getNamedItem(ch1);
							xercesc::XMLString::release(&ch1);
							if(attrNode)
							{

								//get language name
								buf3 = xercesc::XMLString::transcode(attrNode->getNodeValue());

								//get language type
								ch1 = xercesc::XMLString::transcode(XML_TYPE);
								attrNode2 = attrList->getNamedItem(ch1);
								xercesc::XMLString::release(&ch1);
								if(attrNode2)
								{
									buf4 = xercesc::XMLString::transcode(attrNode2->getNodeValue());
									type = std::string(buf4);
									xercesc::XMLString::release(&buf4);
								}

								//get language shortcut
								ch1 = xercesc::XMLString::transcode(XML_SHORTCUT);
								attrNode2 = attrList->getNamedItem(ch1);
								xercesc::XMLString::release(&ch1);
								if(attrNode2)
								{
									buf4 = xercesc::XMLString::transcode(attrNode2->getNodeValue());
									shortcut = std::string(buf4);
									xercesc::XMLString::release(&buf4);
								}

								//get language write mode
								ch1 = xercesc::XMLString::transcode(XML_MODE_LEFT_2_RIGHT);
								attrNode2 = attrList->getNamedItem(ch1);
								xercesc::XMLString::release(&ch1);
								unsigned long modeLeft2Right = 1;
								if(attrNode2)
								{
									buf4 = xercesc::XMLString::transcode(attrNode2->getNodeValue());
									modeLeft2Right = strtoul(buf4, &stopPtr, 0);
									if(strcmp(stopPtr, ""))//if an error occured during conversion
										modeLeft2Right = 1;
									xercesc::XMLString::release(&buf4);
								}

								//get spaceseparated mode
								ch1 = xercesc::XMLString::transcode(XML_SPACESEPARATED);
								attrNode2 = attrList->getNamedItem(ch1);
								xercesc::XMLString::release(&ch1);
								unsigned long spaceSeparated = 1 ;
								if(attrNode2)
								{
									buf4 = xercesc::XMLString::transcode(attrNode2->getNodeValue());
									spaceSeparated = strtoul(buf4, &stopPtr, 0);
									if(strcmp(stopPtr, ""))//if an error occured during conversion
										spaceSeparated = 1;
									xercesc::XMLString::release(&buf4);
								}

								//get isActivated
								ch1 = xercesc::XMLString::transcode(XML_ISACTIVATED);
								attrNode2 = attrList->getNamedItem(ch1);
								xercesc::XMLString::release(&ch1);
								unsigned long isActivated = 1 ;
								if(attrNode2)
								{
									buf4 = xercesc::XMLString::transcode(attrNode2->getNodeValue());
									isActivated = strtoul(buf4, &stopPtr, 0);
									if(strcmp(stopPtr, ""))//if an error occured during conversion
										isActivated = 1;
									xercesc::XMLString::release(&buf4);
								}

								//get mapping mode
								ch1 = xercesc::XMLString::transcode(XML_MAPPINGMODE);
								attrNode2 = attrList->getNamedItem(ch1);
								xercesc::XMLString::release(&ch1);
								if(attrNode2)
								{
									buf4 = xercesc::XMLString::transcode(attrNode2->getNodeValue());
									mappingMode = std::string(buf4);
									xercesc::XMLString::release(&buf4);
								}

								il = NULL ;
								//> isActivated ==2 means language needing external IME
								if (isActivated==2 && externalIME)
								{
									string name = IME_LANGUAGE ;
									string display = Languages::getInstance()->get_name(shortcut) ;
									bool modifyMap = false ;
					    	    	il = createInputLanguage(display, IME_LANGUAGE, "", true, shortcut, "", true, true, modifyMap) ;
								}
								//> other cases let's add corresponding language IME
								else if (isActivated!=2)
								{
									string name = Languages::getInstance()->get_name(shortcut) ;
									string display = name ;
									bool isLeftToRight = (modeLeft2Right == 0 ? false : true) ;
									bool isSpaceSeparated = (spaceSeparated == 0 ? false : true) ;
									bool isIlActivated = (isActivated == 0 ? false : true ) ;
									il = InputLanguageHandler::createInputLanguage(display, name, type, isLeftToRight, shortcut, mappingMode, isSpaceSeparated, isIlActivated);
								}

								tmp1 = childNode->getFirstChild();
								while(tmp1 && il)
								{
									if(tmp1->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
									{
										buf4 = xercesc::XMLString::transcode(tmp1->getNodeName());
										if(!strcasecmp(buf4, XML_KEY_MAP))//<KeyMap>
										{
											attrList = tmp1->getAttributes();
											if(attrList)
											{
												//get gdk Key ID attributes
												ch1 = xercesc::XMLString::transcode(XML_KEY_VAL);
												attrNode2 = attrList->getNamedItem(ch1);
												xercesc::XMLString::release(&ch1);

												ch1 = xercesc::XMLString::transcode(XML_KEY_HARDWARE);
												attrNode3 = attrList->getNamedItem(ch1);
												xercesc::XMLString::release(&ch1);

												if(attrNode2 && attrNode3)
												{
													buf5 = xercesc::XMLString::transcode(attrNode2->getNodeValue());
													gdkKeyVal = (unsigned int)strtoul(buf5, &stopPtr, 0);
													xercesc::XMLString::release(&buf5);

													buf5 = xercesc::XMLString::transcode(attrNode3->getNodeValue());
													hardware_code = (unsigned int)strtoul(buf5, &stopPtr2, 0);
													xercesc::XMLString::release(&buf5);
													if(!strcmp(stopPtr, "") && !strcmp(stopPtr2, ""))//succeed in conversion
													{
														//get unicode value
														ch1 = xercesc::XMLString::transcode(XML_UNICODE_VALUE);
														attrNode4 = attrList->getNamedItem(ch1);
														xercesc::XMLString::release(&ch1);
														//get unicode replace
														ch1 = xercesc::XMLString::transcode(XML_UNICODE_REPLACE);
														attrNode5 = attrList->getNamedItem(ch1);
														xercesc::XMLString::release(&ch1);
														//get modifier
														ch1 = xercesc::XMLString::transcode(XML_MODIFIER);
														attrNode6 = attrList->getNamedItem(ch1);
														xercesc::XMLString::release(&ch1);

														if(attrNode4)
														{
															Glib::ustring replace = "" ;
															if (attrNode5)
															{
																buf5 = xercesc::XMLString::transcode(attrNode5->getNodeValue());
																vector<string> sub;
																vector<string>::iterator it;
																StringOps(buf5).split(sub, ", ");
																for ( it=sub.begin(); it != sub.end(); ++it ) {
																	gunichar c = strtoul(it->c_str(), &stopPtr, 16);
																	if ( *stopPtr == 0 )
																		replace += Glib::ustring(1, c);
																	else {
																		MSGOUT << " invalid replacement char value in map for  keyval= " << attrNode2 << endl;
																		break;
																	}
																}
																xercesc::XMLString::release(&buf5);
															}

															std::string modifier = "" ;
															if (attrNode6)
															{
																buf6 = xercesc::XMLString::transcode(attrNode6->getNodeValue());
																Glib::ustring str = Glib::ustring(buf6);
																modifier = str ;
																xercesc::XMLString::release(&buf6);
															}

															buf5 = xercesc::XMLString::transcode(attrNode4->getNodeValue());
															keyVal = (unsigned int)strtoul(buf5, &stopPtr3, 0);
															if ( replace == "" ) replace = Glib::ustring(1, keyVal);

															if(!strcasecmp(stopPtr3, ""))//no error during conversion ==> add key map
															{
																il->addKeyMap(gdkKeyVal, hardware_code, keyVal, replace, modifier);
																il->postLoadingKeyMap(tmp1, keyVal);
															}
															xercesc::XMLString::release(&buf5);
														}
													}
													else MSGOUT << " syntax error in map for  keyval= " << attrNode2 << endl;
												}

											}
										}
										else if(!strcasecmp(buf4, XML_LANGUAGE_GUNICHAR_RANGE))
										{
											gunichar start = 0, end = 0;
											attrList = tmp1->getAttributes();
											if(attrList)
											{
												//start attribute
												ch1 = xercesc::XMLString::transcode(XML_START);
												attrNode2 = attrList->getNamedItem(ch1);
												xercesc::XMLString::release(&ch1);
												buf5 = xercesc::XMLString::transcode(attrNode2->getNodeValue());
												keyVal = (unsigned int)strtoul(buf5, &stopPtr, 0);
												xercesc::XMLString::release(&buf5);
												if(!strcmp(stopPtr, ""))
													start = keyVal;

												ch1 = xercesc::XMLString::transcode(XML_END);
												attrNode2 = attrList->getNamedItem(ch1);
												xercesc::XMLString::release(&ch1);
												buf5 = xercesc::XMLString::transcode(attrNode2->getNodeValue());
												keyVal = (unsigned int)strtoul(buf5, &stopPtr, 0);
												xercesc::XMLString::release(&buf5);
												if(!strcmp(stopPtr, ""))
													end = keyVal;
											}
											if(start != 0 && end != 0 && start < end)
												il->addGunicharRange(new GunicharRange(start, end));
										}

										xercesc::XMLString::release(&buf4);
									}

									tmp1 = tmp1->getNextSibling();
								}

								//add input language to map
								if (il)
									m_ilMap[shortcut] = il;
								xercesc::XMLString::release(&buf3);
							}
						}
					}
					xercesc::XMLString::release(&buf2);
				}
	    		childNode = childNode->getNextSibling();
	    	}
    	}
    	xercesc::XMLString::release(&buf1);

    	//> ADD INPUT THAT DOESN'T MODIFY MAPPING
    	// (USING X or EXTERNAL IME)
    	bool modifyMap = false;
    	//> default language
    	m_ilMap[DEFAULT_LANGUAGE] = createInputLanguage(DEFAULT_LANGUAGE, DEFAULT_LANGUAGE, "", true, DEFAULT_LANGUAGE, "", true, true, modifyMap) ;
    	if (externalIME)
    		m_ilMap[IME_LANGUAGE] = createInputLanguage(IME_LANGUAGE, IME_LANGUAGE, "", true, IME_LANGUAGE, "", true, true, modifyMap) ;
    }
    catch (const xercesc::XMLException& toCatch)
    {
        char* message = xercesc::XMLString::transcode(toCatch.getMessage());
        Log::err() << "Exception message is: \n"
             << message << "\n";
        xercesc::XMLString::release(&message);
        return false;
    }
	catch (const xercesc::DOMException& toCatch)
	{
		char* message = xercesc::XMLString::transcode(toCatch.msg);
	    Log::err() << "Exception message is: \n"
	         << message << "\n";
	    xercesc::XMLString::release(&message);
	    return false;
	}
	catch (...)
	{
	    Log::err() << "Unexpected Exception \n" ;
	    return false;
	}

	delete parser;

	return true;
}

InputLanguage*
InputLanguageHandler::createInputLanguage(std::string display, std::string name, std::string ilType, bool modeLeft2Right, std::string shortcut, Glib::ustring mappingMode, bool spaceSeparated, bool isActivated, bool modifyMapping)
{
	if(ilType == "ar" )
		return new InputLanguageArabic(display, name, shortcut, mappingMode, modeLeft2Right, spaceSeparated, isActivated, modifyMapping);
	return new InputLanguage(display, name, shortcut, mappingMode , modeLeft2Right, spaceSeparated, isActivated, modifyMapping);
}

InputLanguage*
InputLanguageHandler::get_input_language_by_name(std::string langName)
{
	std::map<std::string, InputLanguage*>::const_iterator ite;
	Glib::ustring code ;
	//> for non modifying mapping input language, shortcut is the name
	if ( langName.compare(DEFAULT_LANGUAGE)==0 || langName.compare(IME_LANGUAGE)==0 )
		code = langName ;
	else
		code = Languages::getInstance()->get_code(langName);
	if ( (ite = m_ilMap.find(code)) != m_ilMap.end() )
		return (*ite).second;
	return NULL;
}

InputLanguage*
InputLanguageHandler::get_input_language_by_shortcut(std::string code)
{
	std::map<std::string, InputLanguage*>::const_iterator ite;
	if((ite = m_ilMap.find(code)) != m_ilMap.end())
		return (*ite).second;
	return NULL;
}

InputLanguage*
InputLanguageHandler::change_current_language(InputLanguage* current_il, int offset)
{
	int size = m_ilMap.size() ;
	bool found = false ;

	if (size == 0 || current_il==NULL)
		return NULL ;

	Glib::ustring current_shortcut = current_il->get_language_shortcut() ;
	Glib::ustring tmp_shortcut ;

	if (offset == 1)
	{
		std::map<std::string,InputLanguage*>::iterator ite = m_ilMap.begin();
		found = false ;
		//> search for current input
		while ( !found && ite!=m_ilMap.end() ) {
			if ( ite->second!=NULL && (current_shortcut.compare( ite->second->get_language_shortcut() )==0) )
				found = true ;
			ite++ ;
		}
		//> if found, search for next activated iterator
		if (found) {
			found=false ;
			while ( !found && ite!=m_ilMap.end() ) {
				if ( ite->second!=NULL && ite->second->isActivated() )
					found = true ;
				else
					ite++ ;
			}
			if (found)
				return ite->second ;
			else
				return current_il ;
		}
		//> not found, return current
		else
			return current_il ;
	}
	else if (offset == -1)
	{
		std::map<std::string,InputLanguage*>::reverse_iterator ite = m_ilMap.rbegin() ;
		found = false ;
		//> search for current input
		while ( !found && ite!=m_ilMap.rend() ) {
			if ( ite->second!=NULL && (current_shortcut.compare( ite->second->get_language_shortcut() )==0) )
				found = true ;
			ite++ ;
		}
		//> if found, search for previous activated iterator
		if (found) {
			found=false ;
			while ( !found && ite!=m_ilMap.rend() ) {
				if ( ite->second!=NULL && ite->second->isActivated() )
					found = true ;
				else
					ite++ ;
			}
			if (found)
				return ite->second ;
			else
				return current_il ;
		}
		//> not found, return current
		else
			return current_il ;
	}
}

InputLanguage*
InputLanguageHandler::get_input_language_by_char(gunichar c)
{
	std::map<std::string, InputLanguage*>::iterator ite;
	for(ite = m_ilMap.begin() ; ite != m_ilMap.end() ; ite++)
	{
		if((*ite).second->is_gunichar_in_bound(c) == true)
			return (*ite).second;
	}
	return NULL;
}

void InputLanguageHandler::activate_external_IME(GtkIMContext* context, GdkWindow* window, bool activate)
{
#ifdef WIN32
	if (window!=NULL) {
		HIMC himc;
		HWND hwnd = NULL;
		hwnd =(HWND)GDK_WINDOW_HWND(window);
		himc = ImmGetContext (hwnd);
		ImmSetOpenStatus(himc, activate);
		ImmReleaseContext(hwnd, himc);
	}
#else
	if (context!=NULL) {
		if (activate)
			gtk_im_context_focus_in(context) ;
		else
			gtk_im_context_focus_out(context) ;
	}
#endif
}


void
InputLanguageHandler::free_resources()
{
	std::map<std::string, InputLanguage*>::iterator ite;
	for(ite = m_ilMap.begin() ; ite != m_ilMap.end() ; ite++)
		delete (*ite).second;
}

}
