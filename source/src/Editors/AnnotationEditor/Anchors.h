/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */


#ifndef _HAVE_ANCHORS_H
#define _HAVE_ANCHORS_H

#include <string>
#include <map>
#include <vector>
#include <set>
#include <gtkmm.h>

namespace tag {

/**
* @class 	Anchor
* @ingroup	AnnotationEditor
* Anchors in text buffer for annotation elements
*
* This class associates a position in text buffer to corresponding annotation
* element.
*/

class Anchor
{
public:

	/*! constructors  */
	Anchor()
	: m_type(""),  m_id(""), m_isTimeAnchored(true), m_isTextType(false), m_isAttached(false), m_precedenceIndex(0) { }

	/**
	* Constructor
	* @param type				Annotation type
	* @param mark				Associated anchor mark
	* @param timeAnchored		Anchor id
	* @param textType	 		True for text-type element, false otherwise
	* @param with_attachment	True if tagged element is linked to the anchor, false otherwise
	*/
	Anchor(std::string type, const Glib::RefPtr<Gtk::TextMark>& mark, bool timeAnchored=true, bool textType=false, bool with_attachment=false) ;

	/**
	* Constructor
	* @param copy		Anchor to be copied
	*/
	Anchor(const Anchor& copy) ;

	/*! destructor */
	~Anchor();

	/**
	* Copy operator
	* @param copy		Anchor to be copied
	* @return			Newly copied anchor
	*/
	Anchor& operator=(const Anchor& copy) ;

	/**
	* Accessor to anchor id
	* @return		Anchor id
	*/
	const std::string& getId() const { return m_id; }

	/*! Gets associated text mark in buffer */
	const Glib::RefPtr<Gtk::TextMark>& getMark() const { return m_mark; }

	/*! Gets associated text iter in buffer */
	Gtk::TextIter getIter() const { return m_mark->get_iter(); }

	/*! Gets mark gravity */
	bool getLeftGravity() const { return m_mark->get_left_gravity(); }

	/*! Gets anchor type */
	const std::string& getType() const { return m_type; }

	/*! Gets anchor time property */
	bool isTimeAnchored() const { return m_isTimeAnchored; }

	/*! Sets anchor time property */
	void setTimeAnchored(bool anchored) { m_isTimeAnchored = anchored ; storeMarkData(); }

	/*! Gets anchor text-type property */
	bool isTextType() const { return m_isTextType; }

	/*! Sets anchor time property */
	void setTextType(bool t) { m_isTextType = t ; storeMarkData(); }


	/*! Gets anchor attachment property */
	bool isAttached() const { return m_isAttached; }

	/*! Sets anchor attachment property */
	void setAttached(bool t) { m_isAttached = t ; storeMarkData(); }

	/*! Delete mark from text buffer */
	void deleteMark() ;

	/**
	* set anchor gravity
	* @param left_gravity 	True if left, else false
	*/
	void setLeftGravity(bool left_gravity);

	/**
	* move anchor position in text buffer
	* @param nbchar 	Displacement in nb of chars
	*/
	void move(int nbchar);

	/**
	* move anchor position in text buffer
	* @param pos 		Text iter where to move mark
	*/
	void move(const Gtk::TextIter& pos);

	/**
	* Sets anchor data
	* @param data		Data to set
	*/
	void setData(const std::string& data);

	/**
	 *	Uses Anchor member values to compute data and applies it to anchor mark
	 */
	void storeMarkData();

	/*! get anchor data */
	const std::string& getData() ;

	/**
	 * Accessor to anchor data
	 * @return	anchor data string representation
	 */
	std::string getConstData() const ;

	/**
	 * Accessor to precedence index
	 * @return	int
	 */
	int getPrecedenceIndex() const { return m_precedenceIndex; }

	/**
	 * Sets the precedence index
	 * @param p		Precedence index
	 */
	void setPrecedenceIndex(int p) { m_precedenceIndex = p; }

	/**
	 * @param prec_index anchor type precedence index
	 * @return true if current anchor has lower precedence than given index
	 */
	bool hasLowerPrecedence(int prec_index) const { return (getPrecedenceIndex() > prec_index) ; }

	/**
	 * @param 	a	Anchor (reference)
	 * @return 		True if current anchor has lower precedence than given index
	 */
	bool hasLowerPrecedence(const Anchor& a) const { return (getPrecedenceIndex() > a.getPrecedenceIndex()) ; }

	/**
	 * Accessor to all data in string representation
	 * @return	string
	 * @note	development purpose
	 */
	std::string getPrintInfo() ;

private:

	std::string	m_type;
	Glib::RefPtr<Gtk::TextMark> m_mark;
	std::string m_id;  // anchor id
	std::string m_data;  // anchor data
	bool m_isTimeAnchored; // is mainstream anchors
	bool m_isTextType;		// anchor for text-type element
	bool m_isAttached;		// anchor is attached to some tagged element in buffer
	int m_precedenceIndex;	// anchor type precedence

} ;


/**
* @class 	AnchorSet
* @ingroup	AnnotationEditor
* Management of a set of annotation buffer anchors
*/

class AnchorSet
{
	public:

		/**
		 * @enum 	SeekDirection
		 * Defines anchor seeking directions
		 */
		typedef enum { BACKWARD_SEEK, FORWARD_SEEK } SeekDirection;

		/**
		*  Gets anchor of given type at or preceding given position in text buffer
		*  @param pos 		Position in text buffer
		*  @param type 		Type of anchor / "" for any type
		*  @param timeAnchored 	True if end anchor must be signal-anchored, else false
		*  @return 			Anchor id
		*/
		const std::string& getPreviousAnchorId(const Gtk::TextIter& pos, const std::string& type="", bool timeAnchored=false)
		{ return getAnchorIdNearPos(pos, type, timeAnchored, BACKWARD_SEEK) ; }

		/**
		*  Gets anchor of given type following given position in text buffer
		*  @param pos 		Position in text buffer
		*  @param type 		Type of anchor / "" for any type
		*  @param timeAnchored 	True if end anchor must be signal-anchored, else false
		*  @return 			Anchor id
		*/
		const std::string& getNextAnchorId(Gtk::TextIter pos, const std::string& type="", bool timeAnchored=false);

		/**
		* Checks if an anchor is set at given iter position
		*  @param pos 		Position in text buffer
		*  @param type 		Type of anchor / "" for any type
		*  @param timeAnchored 	True if end anchor must be signal-anchored, else false
		*  @return 			True if anchor set
		*/
		bool iterHasAnchor(const Gtk::TextIter& pos, const std::string& type="", bool timeAnchored=false);

		/**
		 *  Gets next anchor for given anchor
		 *  @param anchor 	Current anchor
		 *  @param type		Anchor type
		 *  @return			Pointer on previous anchor / NULL if no end anchor
		 */
		Anchor* getNextAnchor(const Anchor& anchor, const std::string& type="") ;

		/**
		 * Gets next anchor id for given anchor id
		 * @param id		Anchor id
		 * @param type 		Type of anchor / "" for any type
		 * @param timeAnchored 	True if end anchor must be signal-anchored, else false
		 * @return			Anchor id
		 */
		const std::string& getNextAnchorId(const std::string& id, const std::string& type="", bool timeAnchored=false) ;

		/**
		*  Gets previous anchor for given anchor
		*  @param anchor 	Current anchor
		*  @param type		Anchor type
		*  @return 			Pointer on previous anchor / NULL if no end anchor
		*/
		Anchor* getPreviousAnchor(const Anchor& anchor, const std::string& type="") ;

		/**
		*  Gets anchor with given type at or preceeding given text iterator
		*  @param iter 		Current text iterator
		*  @param type		Anchor type
		*  @return 			Pointer on previous anchor / NULL if no end anchor
		*/
		Anchor* getPreviousAnchor(const Gtk::TextIter& iter, const std::string& type="") ;
		/**
		 * Gets previous anchor id for given anchor id
		 * @param id		Anchor id
		 * @param type 		Type of anchor / "" for any type
		 * @param timeAnchored 	True if end anchor must be signal-anchored, else false
		 * @return			Anchor id
		 */
		const std::string& getPreviousAnchorId(const std::string& id, const std::string& type="", bool timeAnchored=false) ;

		/**
		*  Gets end anchor for given type after given position in text buffer
		*  @param pos 		Position in text buffer
		*  @param type 		Type of anchor
		*  @param timeAnchored 	True if end anchor must be signal-anchored, else false
		*  @param dir 		Seek direction
		*  @return 			Anchor id
		*/
		const std::string& getEndAnchor(Gtk::TextIter pos, const std::string& type="", bool timeAnchored=false, SeekDirection dir=FORWARD_SEEK);

		/**
		*  Gets end anchor for given anchor
		*  @param anchor 	Current anchor
		*  @param timeAnchored 	True if end anchor must be signal-anchored, else false
		*  @param dir 		Seek direction
		*  @return 			Pointer on end anchor / NULL if no end anchor
		*/
		Anchor* getEndAnchor(const Anchor& anchor, bool timeAnchored=false, SeekDirection dir=FORWARD_SEEK);

		/**
		*  Gets end anchor for given anchor id
		*  @param id 		Current anchor identifier
		*  @param timeAnchored 	True if end anchor must be signal-anchored, else false
		*  @param dir 		Seek direction
		*  @return 			Pointer on end anchor / NULL if no end anchor
		*/
		Anchor* getEndAnchor(const std::string& id, bool timeAnchored=false,SeekDirection dir=FORWARD_SEEK);

		/**
		*  Gets end anchor id for given anchor id
		*  @param id 	Current anchor identifier
		*  @param timeAnchored 	True if end anchor must be signal-anchored, else false
		*  @param dir 		Seek direction
		*  @return 			Id of end anchor / "" if no end anchor
		*/
		const std::string& getEndAnchorId(const std::string& id, bool timeAnchored=false, SeekDirection dir=FORWARD_SEEK);

		/**
		*  Gets anchor of given type near given position in text buffer
		*  @param pos 		Position in text buffer
		*  @param type 		Type of anchor
		*  @param timeAnchored 	True if end anchor must be signal-anchored, else false
		*  @param dir 		Seek direction BACKWARD_SEEK / FORWARD_SEEK
		*  @return 			Anchor id
		*/
		const std::string& getAnchorIdNearPos(const Gtk::TextIter& pos, const std::string& type="", bool timeAnchored=false, SeekDirection dir=BACKWARD_SEEK);

		/**
		*  Gets all anchors of given type at given position in text buffer
		*  @param pos 		Position in text buffer
		*  @param type 		Type of anchor
		*  @param timeAnchored 	True if end anchor must be signal-anchored, else false
		*  @return 			Vector holding pointers on anchors found at pos
		*/
		std::vector<Anchor*> getAnchorsAtPos(const Gtk::TextIter& pos, const std::string& type, bool timeAnchored=false);

		/**
		*  Get the first anchor of given type at given position in text buffer
		*  @param pos 		Position in text buffer
		*  @param type 		Type of anchor
		*  @param timeAnchored 	True if end anchor must be signal-anchored, else false
		*  @param textType 	True if end anchor must be text-type, else false
		*  @param exclude	anchor id not to be taken into account
		*  @return 			pointer on first anchor found at pos / NULL if none
		*/
		Anchor* getAnchorAtPos(const Gtk::TextIter& pos, const std::string& type, bool timeAnchored=false,bool textType=false, const std::string& exclude="");

		/**
		 * Moves anchor position in text buffer
		 * @param pos					New text pos where to set anchor
		 * @param anchor				Anchor to move
		 * @param forceDisplayUpdate	True for forcing update of anchor associated display, False otherwise
		 */
		void moveAnchor(Anchor* anchor, const Gtk::TextIter& pos, bool forceDisplayUpdate=false);

		/**
		* Gets anchor with given id
		* @param id 	Anchor id
		* @return 		Pointer on found anchor / NULL
		*/
		Anchor* getAnchor(const std::string& id);

		/**
		* Gets anchor with given type and no
		* @param type 	Anchor type
		* @param no 	Anchor no
		* @return 		Pointer on found anchor / NULL
		*/
		Anchor* getAnchor(const std::string& type, int no);

		/**
		 * Gets anchors in given iterator range
		 * @param start  		Start iterator
		 * @param stop  		End iterator
		 * @param with_main		Whether or not getting main anchors
		 * @return				Vector with matching anchors
		 */
		std::vector<std::string> getAnchorIds(const Gtk::TextIter& start, const Gtk::TextIter& stop, bool with_main=true);

		/**
		 * Creates unnamed anchor with given type at given text position
	 	 * @param type 				Anchor type
	 	 * @param pos 				Text position
		 * @param left_gravity 		Text mark gravity set to left if true, to right else
		 * @param timeAnchored		True if end anchor must be signal-anchored, else false
		 * @param textType			True if the anchor will handle text data, false otherwise
		 * @param with_attachment
		 * @return					New anchor id
		 */
		const std::string& createAnchor(const std::string& type, const Gtk::TextIter& pos, bool left_gravity=true, bool timeAnchored=true, bool textType=false, bool with_attachment=false);

		/**
		 * Creates anchor with given id and type at given text position
		 * @param id 				Anchor id to be used (if possible)
		 * @param type 				Anchor type
		 * @param pos 				Text position
		 * @param left_gravity 		Text mark gravity set to left if true, to right else
 		 * @param timeAnchored		True if end anchor must be signal-anchored, else false
		 * @param textType			True if the anchor will handle text data, false otherwise
		 * @param with_attachment
		 * @return 					New anchor id
		 */
		const std::string& createAnchor(const std::string& id, const std::string& type, const Gtk::TextIter& pos, bool left_gravity=true, bool timeAnchored=true, bool textType=false, bool with_attachment=false);

		/**
		 * Creates anchor with given id and type at given text position
		 * @param id 				Anchor id
		 * @param pos 				Text position
		 * @param left_gravity 		Text mark gravity set to left if true, to right else
		 * @param data				Anchor data
		 * @return
		 */
		const std::string& createAnchor(const std::string& id, const Gtk::TextIter& pos, bool left_gravity=true, const std::string& data="");

		/**
		 * Sets or unsets anchor left gravity
		 * @param left_gravity		True for left gravity, false for right on
		 * @param anchor			Anchor to modify
		 */
		void setLeftGravity(bool left_gravity, Anchor* anchor) ;

		/**
		* Deletes anchor with given id
		* @param id anchor identifier
		* @return true if anchor deleted, else false
		*/
		bool deleteAnchor(const std::string& id);

		/**
		* delete anchors in given iterator range
		* @param start  start iterator
		* @param stop  end iterator
		* @param with_main		Whether or not getting main anchors
		*/
		bool deleteAnchors(const Gtk::TextIter& start, const Gtk::TextIter& stop, bool with_main=true);

		/**
		 * Signal emitted at anchor deletion.\n
		 * <b>const Glib::RefPtr<Gtk::TextMark>& parameter:</b> Pointer on the impacted TextMark
		 */
		sigc::signal<void, const Glib::RefPtr<Gtk::TextMark>& >& signalDeleteAnchor() { return m_signalDeleteAnchor; }

		/**
		 * Signal emitted at anchor creation.\n
		 * <b>const Gtk::TextBuffer::iterator parameter:</b>		Corresponding position in text
		 * <b>const Glib::RefPtr<Gtk::TextMark>& parameter:</b> 	Pointer on the corresponding TextMark
		 */
		sigc::signal<void, const Gtk::TextBuffer::iterator&, const Glib::RefPtr<Gtk::TextMark>&  >& signalCreateAnchor() { return m_signalCreateAnchor ; }

		/**
		 * Signal emitted at anchor deplacement.\n
		 * <b>const Gtk::TextBuffer::iterator parameter:</b>		Corresponding position in text
		 * <b>const Glib::RefPtr<Gtk::TextMark>& parameter: </b>	Pointer on the corresponding TextMark
		 * <b>bool parameter: </b>									True for making view updating the mark associatesd renderer
		 * @remarks													The bool parameter will mainly be set to false
		 */
		sigc::signal<void, float, const Gtk::TextBuffer::iterator&, const Glib::RefPtr<Gtk::TextMark>&, bool  >& signalMoveAnchor() { return m_signalMoveAnchor ; }

		/**
		 * Access operator
		 * @param id	Anchor id
		 * @return		Reference on the corresponding Anchor
		 */
		Anchor& operator[] (const std::string& id) { return m_anchors[id]; }

		/*! dump anchor table contents to stdout */
		void dump();

		/**
		 * Clear anchor table
		 */
		void clear();

		/**
		 * configure anchor types for current anchor set
		 * @param types mainstream types sequence
		 */
		void setOrderedTypes(const std::vector<std::string>& types) { m_types = types; }

		/*! switch to debug mode (anchors will be visible in editor) */
		void setDebugMode(bool b) { _debug_mode = b; }

		/**
		* get anchor type precedence
		* @param type element type
		* @return type precedence index  (the lowest index is the highest precedence)
		*/
		int getPrecedenceIndex(const std::string& type) const;

		/**
		 * @param type anchor type / "" to get the lowest precedence type
		 * @return anchor type / "" if no lower level
		 */
		std::string getLowerPrecedenceType(const std::string& type) const;
		/**
		 * @param type anchor type / "" to get the highest precedence type
		 * @return anchor type / "" if no higher level
		 */
		std::string getHigherPrecedenceType(const std::string& type) const;

		/**
		 * @param a anchor
		 * @param type anchor type
		 * @return true if given anchor has lower precedence than given type
		 */
		bool hasLowerPrecedence(Anchor& a, const std::string& type) const { return a.getPrecedenceIndex() > getPrecedenceIndex(type); }

		/**
		 * Debug purpose
		 * @return vector
		 */
		const std::map<int, std::vector<std::string> >& checkAnchors() ;

	private:

		/**
		*  Gets anchor of given type near given position in text buffer
		*  @param pos 		Position in text buffer
		*  @param type 		Type of anchor
		*  @param timeAnchored 	True if end anchor must be signal-anchored, else false
		*  @param dir 		Seek direction BACKWARD_SEEK / FORWARD_SEEK
		*  @return 			map iterator
		*/
		std::map<std::string, Anchor >::iterator getAnchorIdNearPosInternal(const Gtk::TextIter& pos, const std::string& type="", bool timeAnchored=false, SeekDirection dir=BACKWARD_SEEK);

		std::map<std::string, Anchor > m_anchors; // text marks
		std::map<std::string, int> m_nos;	// for unnamed anchors
		std::vector<std::string> m_types; // anchor type
		std::map<int, std::vector<std::string> > tmpChkAnchors ; //debug

		sigc::signal<void, const Glib::RefPtr<Gtk::TextMark>& > m_signalDeleteAnchor;
		sigc::signal<void, const Gtk::TextBuffer::iterator&, const Glib::RefPtr<Gtk::TextMark>&> m_signalCreateAnchor ;
		sigc::signal<void, float, const Gtk::TextBuffer::iterator&, const Glib::RefPtr<Gtk::TextMark>&, bool> m_signalMoveAnchor ;



		static bool _debug_mode;
		static std::string _notFound;
};

}

#endif  /* _HAVE_ANCHORS_H  */
