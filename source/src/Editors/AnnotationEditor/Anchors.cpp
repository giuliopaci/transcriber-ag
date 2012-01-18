/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
* @file Anchors.cpp
* @brief text anchors
*   text anchor are positionned just before the corresponding annotation
*   element text; for labelled elements, like turns, the anchor is set just
*   before the label.
*/

#include <sstream>
#include <stdlib.h>

#include "Anchors.h"
#include "Common/util/Log.h"
#include "Common/util/Utils.h"
#include "AnnotationBuffer.h"
using namespace std;

namespace tag {

std::string AnchorSet::_notFound("");
bool AnchorSet::_debug_mode = false;

//
//  createAnchor : create text mark at given text position and store it in anchors map
//
const string& AnchorSet::createAnchor(const string& type, const Gtk::TextIter& pos, bool left_gravity, bool timeAnchored, bool textType, bool with_attachment)
{
	ostringstream os;

	if ( m_nos.find(type) == m_nos.end() ) m_nos[type] = 0;
	m_nos[type]++;

	os << type << "_" << m_nos[type];
	return createAnchor(os.str(), type, pos, left_gravity, timeAnchored, textType, with_attachment);
}


const string& AnchorSet::createAnchor(const string& id, const string& type, const Gtk::TextIter& pos, bool left_gravity, bool timeAnchored, bool textType, bool with_attachment)
{
	const Glib::RefPtr<Gtk::TextMark>& mark = pos.get_buffer()->create_mark(id, pos, left_gravity);

	if ( _debug_mode )
		const_cast<Glib::RefPtr<Gtk::TextMark>&>(mark)->set_visible(true);

	pair< map<string, Anchor>::iterator, bool> res =
			m_anchors.insert(pair<string, Anchor>(id, Anchor(type, mark, timeAnchored, textType, with_attachment)));
	if ( res.second ) {
		Anchor& a = res.first->second;
		//if ( !data.empty() ) m_anchors[id].setData(data);
		a.setPrecedenceIndex(getPrecedenceIndex(type));
	}

//	TRACE << " CREATE ANCHOR " << m_anchors[id].getType() << " id " << id << " at " << pos << "  main=" << m_anchors[id].isTimeAnchored() << "  left=" << m_anchors[id].getMark()->get_left_gravity() << endl;
	signalCreateAnchor().emit(pos, mark) ;
	return id;
}


const string& AnchorSet::createAnchor(const string& id, const Gtk::TextIter& pos, bool left_gravity, const string& data)
{
	const Glib::RefPtr<Gtk::TextMark>& mark = pos.get_buffer()->create_mark(id, pos, left_gravity);

	if ( _debug_mode )
		const_cast<Glib::RefPtr<Gtk::TextMark>&>(mark)->set_visible(true);

	pair< map<string, Anchor>::iterator, bool> res =
			m_anchors.insert(pair<string, Anchor>(id, Anchor("", mark)));
	if ( res.second ) {
		Anchor& a = res.first->second;
		//if ( !data.empty() ) m_anchors[id].setData(data);
		a.setData(data);
		a.setPrecedenceIndex(getPrecedenceIndex(a.getType()));
	}

//	TRACE << " CREATE ANCHOR " << m_anchors[id].getType() << " id " << id << " at " << pos << "  main=" << m_anchors[id].isTimeAnchored() << "  left=" << m_anchors[id].getMark()->get_left_gravity() << endl;
	signalCreateAnchor().emit(pos, mark) ;
	return id;
}

/**
	* delete anchor with given id
	* @param id anchor identifier
	*/
bool AnchorSet::deleteAnchor(const std::string& id)
{
//	cout << " AnchorSet::deleteAnchor" << id << endl;
	map<string, Anchor>::iterator it = m_anchors.find(id);
	if ( it != m_anchors.end() ) {
		signalDeleteAnchor().emit(it->second.getMark()) ;
		it->second.deleteMark();
		m_anchors.erase(it);
		return true;
	}
	return false;
}

/**
	* delete anchors in given iterator range
	* @param start  start iterator
	* @param stop  end iterator
	*/
bool AnchorSet::deleteAnchors(const Gtk::TextIter& start, const Gtk::TextIter& stop, bool with_main)
{
	map<string, Anchor>::iterator it;
	vector<string> todel;
	vector<string>::iterator itd;

	for ( it = m_anchors.begin(); it != m_anchors.end(); ++it ) {
		const Gtk::TextIter& iter = it->second.getIter();
		if ( iter.compare(start) >= 0 && iter.compare(stop) < 0 ) {
			if ( with_main || ! it->second.isTimeAnchored() ) {
				signalDeleteAnchor().emit(it->second.getMark()) ;
				it->second.deleteMark();
				todel.push_back(it->second.getId());
			}
		}
	}

	for ( itd=todel.begin(); itd != todel.end(); ++itd ) {
//		TRACE_D << " xxxxx  Delete Anchor " << *itd << endl;
		m_anchors.erase(m_anchors.find(*itd));
	}
	return (todel.size() > 0 );
}


//
// get anchor for given id
Anchor* AnchorSet::getAnchor(const string& id)
{
	if ( !id.empty() ) {
		map<string, Anchor>::iterator it = m_anchors.find(id);
		if ( it != m_anchors.end() ) return &(it->second);
	}
	return NULL;
}

//
// get anchor for given type and no
Anchor* AnchorSet::getAnchor(const string& type, int no)
{
	ostringstream os;
	os << type << "_" << no;
	return getAnchor(os.str());
}

/**
* get anchors in given iterator range
* @param start  start iterator
* @param stop  end iterator
*/
vector<string> AnchorSet::getAnchorIds(const Gtk::TextIter& start, const Gtk::TextIter& stop, bool with_main)
{
	map<string, Anchor>::iterator it;
	vector<string> ids;

	for ( it = m_anchors.begin(); it != m_anchors.end(); ++it ) {
		const Gtk::TextIter& iter = it->second.getIter();
		if ( iter.compare(start) >= 0 && iter.compare(stop) < 0 ) {
			if ( with_main || ! it->second.isTimeAnchored() )
				ids.push_back(it->second.getId());
		}
	}
	return ids;
}


//
// get next anchor for given iterator
const std::string& AnchorSet::getNextAnchorId(Gtk::TextIter pos, const std::string& type, bool timeAnchored)
{
	if ( pos.forward_char() )
		return getAnchorIdNearPos(pos, type, timeAnchored, FORWARD_SEEK) ;
	return AnchorSet::_notFound;
}


//
// get next anchor for given anchor
Anchor* AnchorSet::getNextAnchor(const Anchor& anchor, const std::string& type)
{
	const string& id = getNextAnchorId(anchor.getIter(), type, false);
	if ( ! id.empty() ) return getAnchor(id);
	return NULL;
}

//
// get next anchor for given anchor
const std::string& AnchorSet::getNextAnchorId(const string& id, const std::string& type, bool timeAnchored)
{
	Anchor* anchor = getAnchor(id);
	if ( anchor != NULL )
		return getNextAnchorId(anchor->getIter(), type, false);
	return AnchorSet::_notFound;
}


//
// get previous anchor
Anchor* AnchorSet::getPreviousAnchor(const Anchor& anchor, const std::string& type)
{
	Gtk::TextIter pos = anchor.getIter();
	if ( pos.backward_char() ) {
		const string& id = getPreviousAnchorId(pos, type, false);
		if ( ! id.empty() ) return getAnchor(id);
	}
	return NULL;
}

Anchor* AnchorSet::getPreviousAnchor(const Gtk::TextIter& iter, const std::string& type)
{
	Gtk::TextIter pos = iter;
	Anchor* prev = NULL;
	while ( prev == NULL ) {
		prev = getAnchorAtPos(pos, type);
		if ( prev == NULL )
			if ( ! pos.backward_char() ) return NULL;
	}
	return prev;
}

//
// get next anchor for given anchor
const std::string& AnchorSet::getPreviousAnchorId(const string& id, const std::string& type, bool timeAnchored)
{
	Anchor* anchor = getAnchor(id);
	if ( anchor != NULL )
	{
		Gtk::TextIter pos = anchor->getIter();
		if ( pos.backward_char() )
			return getPreviousAnchorId(pos, type, timeAnchored);
	}
	return AnchorSet::_notFound;
}


//
//  get anchor id for given type near pos
//
vector<Anchor*> AnchorSet::getAnchorsAtPos(const Gtk::TextIter& pos, const string& type, bool timeAnchored)
{
	// lookup for all mark of given type at to given position
	map<string, Anchor>::iterator it;
	vector<Anchor*> res ;

	if ( m_anchors.size() > 0 ) {
		const list < Glib::RefPtr<Gtk::TextMark> >& marks = const_cast<Gtk::TextIter&>(pos).get_marks();
		if ( marks.size() > 0 ) {
			list < Glib::RefPtr<Gtk::TextMark> >::const_iterator itm;
			for ( itm=marks.begin(); itm != marks.end(); ++itm ) {
				it = m_anchors.find((*itm)->get_name());
				if ( it != m_anchors.end()
					&& (type.empty() || it->second.getType() == type)
					&& ( !timeAnchored || it->second.isTimeAnchored()) ) {
					res.push_back(&(it->second));
				}
			}
		}
	}
	return res;
}

/**
*  Get the first anchor of given type at given position in text buffer
*  @param pos 		Position in text buffer
*  @param type 		Type of anchor
*  @param timeAnchored 	True if end anchor must be signal-anchored, else false
*  @return 			pointer on first anchor found at pos / NULL if none
*/
Anchor* AnchorSet::getAnchorAtPos(const Gtk::TextIter& pos, const std::string& type, bool timeAnchored, bool textType, const string& exclude)
{
	if ( m_anchors.size() > 0 ) {
		const list < Glib::RefPtr<Gtk::TextMark> >& marks = const_cast<Gtk::TextIter&>(pos).get_marks();
		if ( marks.size() > 0 ) {
			list < Glib::RefPtr<Gtk::TextMark> >::const_iterator itm;
			map<string, Anchor>::iterator it;
			for ( itm=marks.begin(); itm != marks.end(); ++itm ) {
				const string& id = (*itm)->get_name();
				if ( exclude.empty() || exclude != id ) {
					it = m_anchors.find(id);
					if ( it != m_anchors.end()
						&& ( ! textType || it->second.isTextType() )
						&& (type.empty() || it->second.getType() == type)
						&& ( !timeAnchored || it->second.isTimeAnchored()) ) {
						return (&(it->second));
					}
				}
			}
		}
	}
	return NULL;
}

//
//  get anchor id for given type near pos
//
const std::string& AnchorSet::getAnchorIdNearPos(const Gtk::TextIter& pos, const string& type, bool timeAnchored, SeekDirection dir)
{
	map<string, Anchor>::iterator it = getAnchorIdNearPosInternal(pos, type, timeAnchored, dir);
	if ( it != m_anchors.end() ) return it->second.getId();
	return AnchorSet::_notFound;
}

//
//  get anchor id for given type near pos
//
map<string, Anchor>::iterator AnchorSet::getAnchorIdNearPosInternal(const Gtk::TextIter& pos, const string& type, bool timeAnchored, SeekDirection dir)
{
	// lookup for mark of given type closest to given position
	map<string, Anchor>::iterator it;

	if ( m_anchors.size() == 0 ) return m_anchors.end();

	Gtk::TextIter start = pos;
	bool ok = true;
	while ( ok ) {
		const list < Glib::RefPtr<Gtk::TextMark> >& marks = start.get_marks();
		if ( marks.size() > 0 ) {
			list < Glib::RefPtr<Gtk::TextMark> >::const_iterator itm;
			for ( itm=marks.begin(); itm != marks.end(); ++itm ) {
				it = m_anchors.find((*itm)->get_name());
				if ( it != m_anchors.end()
					&& (type.empty() || it->second.getType() == type)
					&& ( !timeAnchored || it->second.isTimeAnchored()) ) {
					return it;
				}
			}
		}

		if ( dir ==  BACKWARD_SEEK  )
		{
			ok = start.backward_char();
//			while ( ok /*&& !start.editable()*/ && ! start.toggles_tag() )
//				ok = start.backward_char();
		}
		else
		{
			// <caution> (o.O)
			// If start is 1 char before buffer end, forward_char() will return false !
			// Just test if we have moved and keep on looping in this case
			Gtk::TextIter tmp = start ;
			ok = start.forward_char();
			if ( !ok && tmp.compare(start)!=0 )
				ok = true ;
//			while ( ok /*&& !start.editable()*/ && ! start.toggles_tag() )
//				ok = start.forward_char();
		}

	}
	return m_anchors.end();
}

bool AnchorSet::iterHasAnchor(const Gtk::TextIter& pos, const std::string& type, bool timeAnchored)
{
	const string& id = getPreviousAnchorId(pos, type, timeAnchored);
	if ( id != "" ) {
		Anchor* anchor = getAnchor(id);
		if ( anchor != NULL ) return (anchor->getIter().compare(pos) == 0);
	}
	return false;
}

//
//  get end anchor id for given type near pos
//
const std::string& AnchorSet::getEndAnchor(Gtk::TextIter pos, const string& type, bool timeAnchored, SeekDirection dir)
{
	vector<Anchor*> at_pos = getAnchorsAtPos(pos, type, timeAnchored);
	Anchor *a = NULL;
	if ( ! at_pos.empty() ) {
		a = getEndAnchor(*(at_pos.front()), timeAnchored, dir);
	} else {
		bool ok = true;
		if ( dir == BACKWARD_SEEK ) ok = pos.backward_char();
		else ok = pos.forward_char();
		if ( ok ) {
			map<string, Anchor>::iterator it = getAnchorIdNearPosInternal(pos, type, timeAnchored, dir);
			if ( it != m_anchors.end() ) {
				a = &(it->second);
			}
		}
	}
	if ( a != NULL ) return a->getId();

	return  AnchorSet::_notFound;
}

//
//  get end anchor for given start anchor
//
Anchor* AnchorSet::getEndAnchor(const Anchor& start, bool timeAnchored, SeekDirection dir)
{
	int prec1 = start.getPrecedenceIndex();
	Anchor* end= NULL;
	map<string, Anchor>::iterator it;

	Gtk::TextIter cur_iter = start.getMark()->get_iter();
	bool ok=true;

	while ( ok && end == NULL ) {
		if ( dir == BACKWARD_SEEK )
			ok =cur_iter.backward_char();
		else
			ok =cur_iter.forward_char();
		if ( ok ) {
			const list< Glib::RefPtr<Gtk::TextMark> >& marks = cur_iter.get_marks();
			if ( marks.size () > 0 ) {
				list< Glib::RefPtr<Gtk::TextMark> >::const_iterator itm;
				for ( itm=marks.begin(); itm != marks.end(); ++itm ) {
					map<string, Anchor>::iterator it = m_anchors.find((*itm)->get_name());
					if ( it != m_anchors.end() ) {
						if ( it->second.getPrecedenceIndex() <= prec1 && (!timeAnchored || it->second.isTimeAnchored() ))
							end = &(it->second);
					}
				}
			}
		}
	}
	return end;
}

//
//  get end anchor for given start anchor
//
Anchor* AnchorSet::getEndAnchor(const string& id, bool timeAnchored, SeekDirection dir)
{
	Anchor* anchor= getAnchor(id);
	if ( anchor != NULL ) return getEndAnchor(*anchor, timeAnchored, dir);
	return NULL;
}

//
//  get end anchor for given start anchor
//
const std::string& AnchorSet::getEndAnchorId(const string& id, bool timeAnchored, SeekDirection dir)
{
	Anchor* anchor = getAnchor(id);
	if ( anchor != NULL ) {
		anchor = getEndAnchor(*anchor, timeAnchored, dir);
		if ( anchor != NULL ) return anchor->getId();
	}
	return AnchorSet::_notFound;
}



void AnchorSet::dump()
{
	map<string, Anchor>::iterator it = m_anchors.begin();
	Anchor *an;
//	Glib::RefPtr<Gtk::TextBuffer> buf;
//	if ( m_anchors.size() > 0 ) buf = it->second.getMark()->get_buffer();

	for ( it = m_anchors.begin(); it != m_anchors.end(); ++it )
	{
		an = getNextAnchor(it->second);
		const char* gravity ;
		if (it->second.getMark()->get_left_gravity())
			gravity="LEFT" ;
		else
			gravity="RIGHT" ;

		TRACE << " Anchor[" << it->second.getType() << "]: " << it->second.getId()
			<< " at " << it->second.getIter() << "- main=" << it->second.isTimeAnchored ()
			<< " - gravity(" << gravity << ") - " ;


		TRACE << std::endl;
	}
}


void AnchorSet::clear()
{
	map<string, Anchor>::iterator it;
	for ( it = m_anchors.begin(); it != m_anchors.end(); ++it ) {
		signalDeleteAnchor().emit(it->second.getMark()) ;
		it->second.deleteMark();
	}
	m_anchors.clear();
}


void AnchorSet::setLeftGravity(bool left_gravity, Anchor* anchor)
{
 	if (anchor) {
 		signalDeleteAnchor().emit(anchor->getMark()) ;
 		anchor->setLeftGravity(left_gravity) ;
 		if ( _debug_mode )
 			const_cast<Glib::RefPtr<Gtk::TextMark>&>(anchor->getMark())->set_visible(true);

 		signalCreateAnchor().emit(anchor->getMark()->get_iter(), anchor->getMark()) ;
	}
	else
		TRACE_D << "AnchorSet::setLeftGravity:> NO ANCHOR" << std::endl ;
}

void AnchorSet::moveAnchor(Anchor* anchor, const Gtk::TextIter& iter, bool forceDisplayUpdate)
{
 	if (anchor) {
 		float old_offset = anchor->getIter().get_offset() ;
 		anchor->move(iter) ;
 		signalMoveAnchor().emit(old_offset, iter, anchor->getMark(), forceDisplayUpdate) ;
	}
	else
		TRACE_D << "AnchorSet::moveAnchor:> NO ANCHOR" << std::endl ;
}

int AnchorSet::getPrecedenceIndex(const string& type) const {
	int i;
	if ( type.empty() ) return 99;
	for (i = 0; i < m_types.size() && m_types[i] != type; ++i) ;
	return i ;
}


/**
 * @param type anchor type / "" to get the lowest precedence type
 * @return anchor type  / "" if no lower level
 */
std::string AnchorSet::getLowerPrecedenceType(const std::string& type) const
{
	int i;
	if ( m_types.empty() ) return "";
	if ( type.empty() ) return m_types.back();
	for (i = 0; i < m_types.size() && m_types[i] != type; ++i) ;
	if ( i >= (m_types.size()-1) ) return "";
	return m_types[i+1];
}

/**
 * @param type anchor type / "" to get the highest precedence type
 * @return anchor type  / "" if no higher level
 */
std::string AnchorSet::getHigherPrecedenceType(const std::string& type) const
{
	int i;
	if ( m_types.empty() ) return "";
	if ( type.empty() ) return m_types.front();
	for (i = 0; i < m_types.size() && m_types[i] != type; ++i) ;
	if ( i == 0 || i == (m_types.size()) ) return "";
	return m_types[i-1];
}

/*** debug purpose ****/
const std::map<int, std::vector<std::string> >& AnchorSet::checkAnchors()
{
	std::map<int, std::vector<std::string> >::iterator itchk ;
	tmpChkAnchors.clear() ;

	int off ;
	std::map<std::string, Anchor >::iterator it ;
	for ( it=m_anchors.begin(); it!=m_anchors.end(); it++ )
	{
		off = it->second.getIter().get_offset() ;
		tmpChkAnchors[off].push_back( it->second.getId() ) ;
	}
	return tmpChkAnchors ;
}

/*******************************************************************************
*  anchors
*******************************************************************************/

Anchor::Anchor(std::string type, const Glib::RefPtr<Gtk::TextMark>& mark,
					bool timeAnchored, bool textType, bool with_attachment)
{
	m_type= type ;
	m_mark = mark;
	m_id = m_mark->get_name() ;
	m_isTimeAnchored = timeAnchored;
	m_isTextType = textType ;
	m_isAttached = with_attachment ;
	storeMarkData();
}

Anchor::Anchor(const Anchor& copy)
: m_mark(copy.getMark()), m_precedenceIndex(copy.getPrecedenceIndex()) {
	m_id = m_mark->get_name() ;
	setData(copy.getConstData());
}

Anchor& Anchor::operator=(const Anchor& copy) {
	m_mark=copy.getMark();
	m_id=copy.getId();
	m_precedenceIndex = copy.getPrecedenceIndex();
	setData(copy.getConstData());
	return *this;
}

/* destructor
*  -> remove corresponding text mark & data
*/
Anchor::~Anchor()
{

}

void Anchor::storeMarkData()
{
	ostringstream os;
	os << m_type << " " << m_isTimeAnchored << " " << m_isTextType << " " << m_isAttached;
	m_data = os.str();
	m_mark->set_data("anchor", (void*)m_data.c_str());
}

/*
* set data -> store data in text mark data so that is can be handled by
*  undo/redo mechanism
*/
void Anchor::setData(const string& data)
{
	istringstream is(data);
	is >> m_type >>  m_isTimeAnchored >>  m_isTextType >>  m_isAttached;
	m_data = data;
	m_mark->set_data("anchor", (void*)m_data.c_str());
}

/*
* get data -> retrieve data from text mark-stored data
*/
const string& Anchor::getData()
{
//	char* str = (char*)m_mark->get_data("anchor");
//	if ( str != NULL ) m_data = str;
//	else
//		m_data = "" ;
	return m_data;
}

string Anchor::getConstData() const
{
	char* str = (char*)m_mark->get_data("anchor");
	if ( str != NULL )
		return str ;
	else
		return "" ;
}


/* delete text mark associated to anchor */
void Anchor::setLeftGravity(bool left_gravity)
{
	if ( left_gravity != m_mark->get_left_gravity() )
	{
		Glib::RefPtr<Gtk::TextBuffer> buffer = m_mark->get_buffer();
		if ( buffer != 0 ) {
			const Gtk::TextIter& pos = m_mark->get_iter();
			string id = m_mark->get_name();
			buffer->delete_mark(m_mark);
			m_mark = buffer->create_mark(id, pos, left_gravity);
			setData(m_data);
		}
	}
}

/* delete text mark associated to anchor */
void Anchor::deleteMark() {
	Glib::RefPtr<Gtk::TextBuffer> buf = m_mark->get_buffer() ;
	buf->delete_mark(m_mark);
}

//
//  move text anchors
//
void Anchor::move(int nbchar)
{
	if ( nbchar == 0 ) return;

	Gtk::TextIter pos = getMark()->get_iter();
	if ( nbchar > 0 ) pos.forward_chars(nbchar);
		else pos.backward_chars((nbchar*-1));

	pos.get_buffer()->move_mark(getMark(), pos);
}

void Anchor::move(const Gtk::TextIter& pos)
{
	pos.get_buffer()->move_mark(getMark(), pos);
}

std::string Anchor::getPrintInfo()
{
	std::string res = std::string("Anchor[") + getType()
							+ std::string("]: ") + getId()
							+ std::string(" at ") ;

	res = res + number_to_string(getIter().get_line()) + "." + number_to_string(getIter().get_line_offset()) ;

	res= res + std::string(" - timeAnchored=") ;
	if (isTimeAnchored())
		res = res + "yes" ;
	else
		res = res + "no" ;

	res= res + std::string(" - isTextType=") ;
	if (isTextType())
		res = res + "yes" ;
	else
		res = res + "no" ;

	res= res + std::string(" - isAttached=") ;
	if (isAttached())
		res = res + "yes" ;
	else
		res = res + "no" ;

	res = res + std::string(" - gravity(") ;

	if (getMark()->get_left_gravity())
		res = res + "LEFT" ;
	else
		res = res + "RIGHT" ;

	res = res + ")" ;

	return res ;
}

} // namespace tag
