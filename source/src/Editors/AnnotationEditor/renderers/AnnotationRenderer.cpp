/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id$ */

/**
 * @file AnnotationRenderer.cpp
 * Annotation Renderers base class implementation
 */

#include "Editors/AnnotationEditor/renderers/AnnotationRenderer.h"
using namespace std;
using namespace tag;


//********************************** CONSTRUCTION ******************************

AnnotationRenderer::AnnotationRenderer(AnnotationView* view, const string& name)
: 	m_view(view), m_dataModel(view->getDataModel()),
	m_name(name), m_tagFlags(0), m_emptyProperties(false),
	m_useTooltip(false), m_isFirstChild(false), contextMenu(NULL), contextMenu_lock(NULL)
{
}

AnnotationRenderer::~AnnotationRenderer()
{
	if (contextMenu)
		delete(contextMenu) ;
	if (contextMenu_lock)
		delete(contextMenu_lock) ;
}

Glib::RefPtr<Gtk::TextTag>
AnnotationRenderer::configure(string tagname, string tagclass, bool undoable, bool is_end,
									Glib::ustring fg, Glib::ustring bg,
									Glib::ustring style, Glib::ustring weight, bool use_bg)
{
	return configure(tagname, tagclass, undoable, is_end, fg, bg, style, weight, use_bg, m_tagFlags) ;
}

Glib::RefPtr<Gtk::TextTag>
AnnotationRenderer::configure(string tagname, string tagclass, bool undoable, bool is_end,
									Glib::ustring fg, Glib::ustring bg,
									Glib::ustring style, Glib::ustring weight,
									bool use_bg, unsigned long& tagFlags)
{
	if ( tagname.empty() )
		tagname=m_name;
	if ( tagclass.empty() )
		tagclass=m_name;

	bool b ;
	b = (m_view->getConfigurationOption("AnnotationLayout,"+m_name+",newline") == "true");
	//TRACE_D<< " NEWLINE_BEFORE for " << m_name << " = " << b << endl;
	if ( b )
		tagFlags |= NEWLINE_BEFORE_TAG;
	b = (m_view->getConfigurationOption("AnnotationLayout,"+m_name+",newline_after_label") == "true");
	if ( b )
		tagFlags |= NEWLINE_AFTER_TAG;

	if ( ! use_bg )
		tagFlags |= INHIBATE_BACKGROUND_TAG;

	if (fg.empty())
		fg = getLook(tagname, "fg") ;
	if (bg.empty())
		bg = getLook(tagname, "bg") ;
	if (style.empty())
		style = getLook(tagname, "style") ;
	if (weight.empty())
		weight = getLook(tagname, "weight") ;

	if (is_end)
		tagname = tagname + "_end" ;

	const Glib::RefPtr<Gtk::TextTag>& tag=getBuffer()->createAnnotationTag(tagname, tagclass, undoable, fg, bg, style, weight, tagFlags);
	m_tags.push_back(tag) ;
	return tag;
}

/**
 *  get annotation rendering configuration item
 * @param tagname annotation tagname
 * @param item  tag configuration item (bg/fg/font/weight)
 * @return item value
 */
string AnnotationRenderer::getLook(const string& tagname, const string& item) {
	string key = "Colors-editor," + tagname ;
	if ( !item.empty() ) {
		key += "_" ;
		key += item;
	}
	string value = "" ;
	if (m_view)
		value =  m_view->getColorsCfgOption(key) ;
	return value ;
}


//******************************* TOGGLE LABEL STATE **************************

bool AnnotationRenderer::hideTag(bool hide)
{
	if ( !(m_tagFlags & CAN_BE_HIDDEN_TAG ) )
		return false ;

	vector< Glib::RefPtr<Gtk::TextTag> >::iterator it ;
	for (it= m_tags.begin() ; it!=m_tags.end(); it++)
		(*it)->set_property("invisible", hide) ;

	return true ;
}


//******************************** ELEMENT STATE *********************************

void AnnotationRenderer::setInvisibleTag(const string& id, bool invisible)
{
	if (invisible)
		m_invisibleTagSet.insert(id) ;
	else
		m_invisibleTagSet.erase(id) ;
}

bool AnnotationRenderer::getInvisibleTag(const string& id)
{
	std::set<string>::iterator it = m_invisibleTagSet.find(id) ;
	if (it==m_invisibleTagSet.end())
		return false ;
	else
		return true ;
}

//********************************** LAYOUT  **********************************
/**
 * init layout for given layout class
 */
void AnnotationRenderer::initLayoutRules(const string& layoutClass)
{
	const std::map<string, string>& layout = m_dataModel.conventions().getLayout("Layout");
	std::map<string, string>::const_iterator itl;
	for ( itl = layout.begin(); itl != layout.end(); ++itl )  {
		int l = layoutClass.length();
		if ( itl->first.length() > l && itl->first.compare(0, l, layoutClass) == 0 ) {
			m_layout[itl->first.substr(l+1)] = itl->second;
		}
	}
}

/**
 * layout (qualifier or event-type) item according to layout rules defined in conventions
 * @param id	item id (qualifier or event-type unit
 * @param layoutClass	applicable layout class
 * @param label (OUT) buffer in which to store layout results
 */
void AnnotationRenderer::layoutItem(const string& id, const string& type, Glib::ustring& label, bool at_start)
{
	const string& desc = m_dataModel.getElementProperty(id, "desc");
	std::map<string, string>::iterator itl;

	if ( (itl = m_layout.find(type)) != m_layout.end() )
		label = itl->second;
	else label = type ;

	if ( ! desc.empty() ) {
		string key = type +","; key += desc;
		if ( (itl = m_layout.find(key)) != m_layout.end() )
			label = itl->second;
		else
		{ // specific desc
			key = type +",";
			key += "user,layout";
			bool add_desc = true;
			if ( (itl = m_layout.find(key)) != m_layout.end() )
			{
				add_desc = ( (at_start && itl->second == "start")
								|| ( !at_start && itl->second != "start") );
			}
			if ( add_desc ) {
				label += desc;
			}
		}
	}
}

//********************************** ACCESSOR **********************************


Glib::ustring AnnotationRenderer::getSegmentText(const string & id)
{
	return "" ;
}

