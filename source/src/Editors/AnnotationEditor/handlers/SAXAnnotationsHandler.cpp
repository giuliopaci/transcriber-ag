/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#include "SAXAnnotationsHandler.h"
SAXAnnotationsHandler::SAXAnnotationsHandler(std::map<string, list<Property> >* p_annotationProperties, list<Property>* p_fileProperties, std::map<string, list<string> >* p_choiceLists) {
	a_annotationProperties = p_annotationProperties;
	a_fileProperties = p_fileProperties;
	a_choiceLists = p_choiceLists;
}

void SAXAnnotationsHandler::startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const Attributes& attrs) {

	char* str = XMLString::transcode(localname);

	int len = attrs.getLength();
	map<string, string> attmap;
	attmap.clear();

	for (int i = 0; i < len; i++) {
		char* name = XMLString::transcode(attrs.getQName(i));
		char* value = XMLString::transcode(attrs.getValue(i));
		attmap[name] = string(value);
		XMLString::release(&name);
		XMLString::release(&value);
	}

	if (strcasecmp(str, "choice_list") == 0) {
		a_choiceListID = string(attmap["id"]);
		a_choiceListValues.clear();
	}
	else if (strcasecmp(str, "value") == 0) a_choiceListValues.push_back(string(attmap["text"]));
	else if (strcasecmp(str, "file_properties") == 0) a_properties = a_fileProperties;
	else if (strcasecmp(str, "annotation") == 0) {
		list<Property> empty_list;
		(*a_annotationProperties)[attmap["type"]] = empty_list;
		a_properties = &((*a_annotationProperties)[attmap["type"]]);
	}
	else if (strcasecmp(str, "property") == 0) {
		Property p;
		p.label = string(attmap["label"]);
		string type = string(attmap["type"]);
		if (type == "text") {
			p.type = PROPERTY_TEXT;
			p.choiceList = "";
		}
		if (type == "choice_list") {
			p.type = PROPERTY_CHOICELIST;
			p.choiceList = string(attmap["choice_list"]);
		}
		a_properties->push_back(p);
	}

	XMLString::release(&str);

}

void SAXAnnotationsHandler::endElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname) {

	char* str = XMLString::transcode(localname);

	if (strcasecmp(str, "choice_list") == 0) (*a_choiceLists)[a_choiceListID.c_str()] = a_choiceListValues;

	XMLString::release(&str);

}

void SAXAnnotationsHandler::characters(const XMLCh* const chars, const XMLSize_t length) {

	if (length == 0) return;
	char* str = XMLString::transcode(chars);
	char* strToRelease = str;
	if (*str) {
		while ((str[0] == '\t') || (str[0] == '\n') || (str[0] == '\r') || (str[0] == ' ')) str++;
		int len = 0;
		while (str[len] != '\0') len++;
		while (true) {
			if (len == 0) break;
			if ((str[len-1] != '\t') && (str[len-1] != '\n') && (str[len-1] != '\r') && (str[len-1] != ' ')) break;
			str[len-1] = '\0';
			len--;
		}
	}
	XMLString::release(&strToRelease);

}
