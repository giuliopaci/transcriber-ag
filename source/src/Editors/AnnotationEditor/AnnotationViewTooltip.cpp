/*
 * AnnotationViewTooltip.cpp
 *
 *  Created on: 26 janv. 2009
 *      Author: montilla
 */

#include "AnnotationViewTooltip.h"
#include "Editors/AnnotationEditor/renderers/AnnotationRenderer.h"

namespace tag {

AnnotationViewTooltip::AnnotationViewTooltip(AnnotationView& p_parent)
: parent(p_parent)
{
	setRCname("tooltip_editor") ;
}

AnnotationViewTooltip::~AnnotationViewTooltip()
{
	// TODO Auto-generated destructor stub
}


bool AnnotationViewTooltip::prepare_tooltip(string tagtype, const std::string& id)
{

	//> -- Erase tail for end tags
	unsigned long pos = tagtype.find("_end") ;
	bool start = true;
	if ( pos !=  string::npos )
	{
		start=false;
		tagtype.erase(pos, string::npos);
	}
//	//> -- Erase tail for not anchored element
//	else
//	{
//		pos = tagtype.find("_timeless") ;
//		if ( pos !=  string::npos )
//			tagtype.erase(pos, string::npos);
//	}

	std::map<std::string, AnnotationRenderer*>::const_iterator itr = parent.getRendererMap().find(tagtype);
	if ( itr != parent.getRendererMap().end() )
	{
		Gtk::Widget* widget = itr->second->getTooltipBox(id, start);
		if ( widget != NULL )
		{
			clean();
			getGeneralFrame()->add(*widget) ;
			getGeneralFrame()->show_all_children();
			return true ;
		}
		else
			return false ;
	}
	return false ;
}

/*
bool AnnotationViewTooltip::prepare_4_tags(string tagtype, const std::string& id, Gtk::TextIter iter)
{
	Glib::ustring tagname = "TAGS:\n-----" ;
	list< Glib::RefPtr<Gtk::TextTag > > tags = const_cast<Gtk::TextBuffer::iterator&>(iter).get_tags();

	if (tags.empty())
		return false ;

	list< Glib::RefPtr<Gtk::TextTag > >::iterator it;
	for ( it = tags.begin(); it != tags.end(); ++it ) {
		tagname.append("\n") ;
		tagname.append((*it)->property_name()) ;
	}
	tagname.append("\n") ;
	if (iter.editable())
		tagname.append("EdiTabLe") ;
	else
		tagname.append("Non-EdiTabLe") ;

	Gtk::Widget* widget = Gtk::manage(new Gtk::Label(tagname)) ;
	clean();
	getGeneralFrame()->add(*widget) ;
	getGeneralFrame()->show_all_children();
	return true ;
}

bool AnnotationViewTooltip::prepare_4_anchors(const Gtk::TextIter& iter)
{
	std::string res ;
	const std::vector<Anchor*> v = parent.getBuffer()->anchors().getAnchorsAtPos(iter, "", false) ;

	if (v.empty())
		return false ;

	std::vector<Anchor*>::const_iterator it ;
	for (it=v.begin(); it!=v.end(); it++)
		res = res + "\n" + (*it)->getPrintInfo() ;

	Gtk::Widget* widget = Gtk::manage(new Gtk::Label(res)) ;
	clean();
	getGeneralFrame()->add(*widget) ;
	getGeneralFrame()->show_all_children();
	return true ;
}
*/

bool AnnotationViewTooltip::prepare_4_debug(const Gtk::TextIter& iter)
{
	ostringstream res0 ;
	Gtk::TextIter it2 = iter;
	++it2;
	Glib::ustring txt = parent.getBuffer()->get_text(iter, it2) ;
	res0 <<  "At " << iter << " char='" << txt << "'" ;

	Gtk::VBox* widget = Gtk::manage(new Gtk::VBox()) ;

	Gtk::Label* title0 = Gtk::manage(new Gtk::Label(res0.str())) ;
	title0->set_name("bold_label") ;
	widget->pack_start(*title0, false, false, 2) ;

	const std::vector<Anchor*> v = parent.getBuffer()->anchors().getAnchorsAtPos(iter, "", false) ;

	Glib::ustring tagname ;
	list< Glib::RefPtr<Gtk::TextTag > > tags = const_cast<Gtk::TextBuffer::iterator&>(iter).get_tags();

//	if (v.empty() && tags.empty())
//		return false ;


	//> ANCHORS
	if (!v.empty())
	{
		ostringstream res ;
		std::vector<Anchor*>::const_iterator it2 ;
		for (it2=v.begin(); it2!=v.end(); it2++)
		{
			res << "\n<(o.O)> \n" << (*it2)->getPrintInfo() ;
			string id = ((*it2)->getId()) ;
			if (!id.empty())
			{
				res << endl <<  parent.getDataModel().toString(id) ;
				res << endl <<  (*it2)->getData() ;
			}
		}

		Gtk::Label* title1 = Gtk::manage(new Gtk::Label("Anchors")) ;
		title1->set_name("bold_label") ;
		Gtk::Widget* lab1 = Gtk::manage(new Gtk::Label(res.str())) ;
		widget->pack_start(*title1, false, false, 2) ;
		widget->pack_start(*lab1, true, true, 2) ;
	}

	//> TAGS
	if (!tags.empty())
	{
		list< Glib::RefPtr<Gtk::TextTag > >::iterator it;
		for ( it = tags.begin(); it != tags.end(); ++it ) {
			tagname.append("\n") ;
			tagname.append((*it)->property_name()) ;
		}
		tagname.append("\n") ;
		if (iter.editable())
			tagname.append("EdiTabLe") ;
		else
			tagname.append("Non-EdiTabLe") ;

		Gtk::Label* title2 = Gtk::manage(new Gtk::Label("Tags")) ;
		title2->set_name("bold_label") ;
		Gtk::Widget* lab2 = Gtk::manage(new Gtk::Label(tagname)) ;
		widget->pack_start(*title2, false, false, 2) ;
		widget->pack_start(*lab2, true, true, 2) ;
	}

	clean();
	getGeneralFrame()->add(*widget) ;
	getGeneralFrame()->show_all_children();
	return true ;
}


bool AnnotationViewTooltip::display(GdkEventMotion event, Gtk::Widget* win)
{
	if (/*!event ||*/ !win)
		return true ;

	// check path
	Gtk::TextIter iter;
	int x, y;
	int ex, ey;

	x = (int) event.x ;
	y = (int) event.y ;

	parent.window_to_buffer_coords(Gtk::TEXT_WINDOW_TEXT, x, y, ex, ey);
	parent.get_iter_at_location(iter, ex, ey);

	const std::string& id = parent.getBuffer()->getTaggedElementId(iter);
	std::string tagtype = parent.getBuffer()->getTaggedElementType(iter);

	bool debug = parent.isDebugMode() ;

	/* DEBUG switch test for monitoring all tags */
	if (!id.empty() || debug)
	{
		// compute position
		int toolx, tooly ;
		compute_position(win, event, toolx, tooly) ;

		// prepare tooltip
		bool ok ;

		/* DEBUG for monitoring all tags */
		if (debug && (event.state & GDK_CONTROL_MASK) )
			ok = prepare_4_debug(iter) ;
		/* CLASSIC behaviour */
		else if (!id.empty())
			ok = prepare_tooltip(tagtype, id) ;

		// if nothing to display (renderer doesn't have to be displayed) exit !
		if (!ok) {
			timeout.disconnect() ;
			return true ;
		}

		// adjust size
		resize(5,5) ;
		set_default_size(5,5) ;

		//adjust position
		adust_in_screen(toolx, tooly, gdk_get_default_root_window ()) ;
		move(toolx, tooly) ;

		//> don't use show method cause dispay redrawed
		//> resizing for forcing complete resize, otherwise keep all size
		reshow_with_initial_size() ;
	}

	timeout.disconnect() ;
	return true ;
}


}
