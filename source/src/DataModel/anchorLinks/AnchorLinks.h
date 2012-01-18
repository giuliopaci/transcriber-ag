/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#ifndef ANCHORLINKS_H_
#define ANCHORLINKS_H_

#include <glibmm.h>
#include <set>

namespace tag {

class DataModel ;

/**
 *  @class 		AnchorLinks
 *  @ingroup 	DataModel
 *  Class managing temporal links between anchors from different graphes.
  */
class AnchorLinks
{
	public:
		AnchorLinks();
		virtual ~AnchorLinks();

		/**
		 * Fills a node for a given anchor id with given linked anchors
		 * @param anchorId		Anchor id
		 * @param links			List of anchor id (';' separated string)
		 */
		void loadLinks(const std::string& anchorId, const std::string& links) ;

		/**
		 * Sets DataModel object for validation utilities
		 * @param model		DataModel object reference
		 */
		void setModel(DataModel* model) ;

		/**
		 * Links two anchors.
		 * @param anchorId1		AnchorId
		 * @param anchorId2		AnchorId
		 * @param check			True for checking link permission, false for forcing link
		 * @return				 1:  link created\n
		 * 						 0:  the link already exists\n
		 * 						-1:	link is not allowed (invalid anchor, same graph, anchored without timestamp)
		 */
		int link(const std::string& anchorId1, const std::string& anchorId2, bool check) ;

		/**
		 * Unlinks two anchors
		 * @param anchorId1		AnchorId
		 * @param anchorId2		AnchorId
		 * @note 				Nothing will be done if the link doesn't exist
		 */
		void unlink(const std::string& anchorId1, const std::string& anchorId2) ;

		/**
		 * Removes all links the anchor is concerned by
		 * @param anchorId		AnchorId
		 */
		void unlink(const std::string& anchorId) ;

		/**
		 * Checks whether a link exists
		 * @param anchorId1		AnchorId
		 * @param anchorId2		AnchorId
		 * @return			True or False
		 */
		bool existLink(const std::string& anchorId1, const std::string& anchorId2) ;

		/**
		 * Checks whether an anchor has link(s)
		 * @param anchordId		AnchorId
		 * @return				True if the anchor has at least one links, False otherwise
		 */
		bool hasLinks(const std::string& anchordId) ;

		/**
		 * Adjust offset to all anchors linked to the given anchor
		 * @param 	anchorId	Anchor id
		 * @param 	offset		New offset to anchor
		 * @warning				This method only set offset, a validation should
		 * 						be previously proceded.
		 */
		void setLinksOffset(const std::string& anchorId, float offset) ;

		/**
		 * Checks whether two anchors can be linked. They should respect the following rules:\n
		 * - existence\n
		 * - time marked\n
		 * - from different graphes\n
		 *
		 * @param anchorId1		Anchor id
		 * @param anchorId2		Anchor id
		 * @return				True if all criterias are respected, false otherwise
		 */
		bool canBeLinked(const std::string& anchorId1, const std::string& anchorId2) ;

		/**
		 * Loads anchor links from XML format
		 * @param in	XML stream to be loaded
		 * @param dtd	Dtd to be used (empty for default)
		 */
		void fromXML(const std::string & in, const std::string& dtd="") throw (const char *) ;

		/**
		 * Writes anchor links into XML format
		 * @param out
		 * @param delim
		 * @return
		 */
		std::ostream& toXML(std::ostream& out, const char* delim) const ;

		/**
		 * Accessor to anchor links elements
		 * @return		Anchor links map size
		 */
		int getSize() { return linksMap.size() ;}

		/**
		 * Get anchor links has std::string
		 * @return		String representation of all anchor links
		 */
		std::string toString() ;

		/**
		 * Gets id of all anchors linked to the given anchor
		 * @param anchorId		Anchor id
		 * @return				Set containning all linked anchor id
		 */
		std::set<std::string> getLinks(const std::string& anchorId) ;


	/*** TOOLS ***/
	public:
		/**
		 * Flags the given anchor. Mainly usefull as temporary tool for preventing
		 * loop in anchor checking.
		 * @param anchorId		Anchor id
		 * @return				True if the anchor could be flagged, false otherwise
		 * 						(already flagged)
		 * @warning				This is just a buffer that keeps anchor ids when they are
		 * 						flagged. It does not check if the anchor has links or is linked.
		 * @note				Should be use with tmpIsFlaggedAnchor and tmpClearBuffer methods
		 */
		bool tmpFlagAnchor(const std::string& anchorId) ;

		/**
		 * Checks whether the given anchor has been already flagged.
		 * @param anchor		Anchor id
		 * @return				True if the anchor is flagged, false otherwise
		 */
		bool tmpIsFlaggedAnchor(const std::string& anchor) ;

		/**
		 * Reset flag tool (erase all flags)
		 */
		void tmpClearBuffer() ;

		/**
		 * Inserts toBeInserted into anchorId links. This method is mainly dedicated to be
		 * called from the undoRedo manager, otherwise call the
		 * tagInsertIntoAnchorLinks(const string&,const string&) method from class DataModel
		 * @param anchorId			Anchor identifier
		 * @param toBeInserted		Anchor identifier to be inserted inside anchorId links
		 */
		void insertIntoLinks(const std::string& anchorId, const std::string& toBeInserted) ;

		/**
		 * Removes toBeRemoved from anchorId links. This method is mainly dedicated to be
		 * called from the undoRedo manager, otherwise call the
		 * tagRemoveFromAnchorLinks(const string&,const string&) method from class DataModel
		 * @param anchorId			Anchor identifier
		 * @param toBeRemoved		Anchor identifier to be removed from anchorId links
		 */
		void removeFromLinks(const std::string& anchorId, const std::string& toBeRemoved) ;

	protected:
		typedef std::set<std::string> AnchorLinksSet ;
		typedef std::map<std::string, AnchorLinksSet > AnchorLinksMap ;
		typedef std::map<std::string, AnchorLinksSet >::iterator AnchorLinksMapIterator ;

	private:
		/** reference to applied conventions object **/
		DataModel* dataModel ;

		/**
		 * links map :
		 * (AnchorId - list of linked anchorId)
		 **/
		AnchorLinksMap linksMap ;

	private :
		std::set<std::string> tmpCheck ;

} ;

} // namespace

#endif /* ANCHORLINKS_H_ */
