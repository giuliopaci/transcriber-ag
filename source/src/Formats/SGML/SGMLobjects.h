/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/* $Id */

/**
 * @file	SGMLobjects.h
 */

#ifndef SGMLOBJECTS_H_
#define SGMLOBJECTS_H_

#include <string>
#include <map>
#include <vector>
#include <glibmm.h>
#include <stdlib.h>
#include <iostream>

using namespace std ;

/**
* @class 		SGMLobjects
* @ingroup		Formats
*
* Class representing all SGML structured information.\n
* Used to parse SGML file and save data.\n
* Once filled, the structures will be used to fill the real model AG.
*
* @remarks		It could be more efficient to directly fill the model AG,
* 				however these objects enables to get the file parsing independent
* 				from the model AG filling, which can be very helpful if
* 				either the SGML format or the internal TransAG format changes.
*/
class SGMLobjects
{
	public:
		/**
		* @class 		SGMLentry
		* @ingroup		Formats
		*
		* Class representing an SGML entry, the finest granularity.
		* @note			Exemple: S,"Reftext","Hyptext","startTime","endTime","percent"
		*/
		class SGMLentry
		{
			public:
				/**
				 * @var type
				 * Alignment type:\n
				 * - D: deletion\n
				 * - I: insertion\n
				 * - C: correct\n
				 * - S: substitution\n
				 */
				Glib::ustring type ;
				Glib::ustring ref_word ;	/**< Reference word */
				Glib::ustring hyp_word ;	/**< Hypothese word */
				float start_time ;			/**< Start time */
				float end_time ;			/**< End time */
				float percent ;				/**< ? unused for TransAG format */
				bool checked ;				/**< internal stamp */

				/**
				 * Formats the given text into available TranscriberAG text.
				 * @param text		Text read from SGML file
				 * @return			Correct text for application.
				 * @note			The SGML format use text with " at start and end.\n
				 * 					This method removes the two " characters.
				 */
				std::string formatText(string text) ;

				/**
				 * Constructor
				 * @param entry_s		SGML entry text representation
				 * @note				exemple:  S,"Reftext","Hyptext","startTime","endTime","percent"
				 */
				SGMLentry(Glib::ustring entry_s) ;
				virtual ~SGMLentry() {} ;

				/**< to string method */
				Glib::ustring toString() ;
		} ;

		/**
		* @class 		SGMLpath
		* @ingroup		Formats
		*
		* Class representing an SGML path (group of SGML entries).\n
		*/
		class SGMLpath
		{
			public:
				Glib::ustring id ;			/**< unused for TranscriberAG */
				Glib::ustring word_cnt ;	/**< unused for TranscriberAG */
				Glib::ustring labels ;		/**< unused for TranscriberAG */
				Glib::ustring file ;		/**< unused for TranscriberAG */
				Glib::ustring chanel ;		/**< chanel used */
				Glib::ustring sequence ;	/**< sequence order */
				float R_T1 ;				/**< Path start time */
				float R_T2 ;				/**< Path end time */
				Glib::ustring word_aux ;	/**< unused for TranscriberAG */

				/**
				 * Add entries information
				 * @param entries		Text representation of entries.
				 * @note				Entries list is represented by entries text representation
				 * 						separated by ":"
				 */
				void set_entries(Glib::ustring entries) ;

				/**
				 * Accessor to the last end time encountered.
				 * @return		The end time of the last entry of the path if this entry has
				 * 				valide end time, otherwise the end time of the path.
				 */
				float getLastTime() ;

				std::vector<SGMLobjects::SGMLentry*> entries ;  /**< Path entries */

				/**
				 * Constructor
				 */
				SGMLpath() ;
				virtual ~SGMLpath()  ;

				/** to string method */
				Glib::ustring toString() ;
		} ;

		/**
		* @class 		SGMLspeaker
		* @ingroup		Formats
		*
		* Class representing an SGML speaker block.\n
		*/
		class SGMLspeaker
		{
			public:
				Glib::ustring id ;								/**< Speaker id - unused */
				std::map<int,SGMLobjects::SGMLpath*> paths ;	/**< Paths of the given block */

				/**
				 * Adds a SGMLPath to the current block.
				 * @param id			<em>id</em> attribute value
				 * @param word_cnt		<em>word_cnt</em> attribute value
				 * @param labels		<em>labels</em> attribute value
				 * @param file			<em>file</em> attribute value
				 * @param channel		<em>chanel</em> attribute value
				 * @param sequence		<em>sequence</em> attribute value
				 * @param R_T1			<em>t1</em> attribute value
				 * @param R_T2			<em>t2</em> attribute value
				 * @param word_aux		<em>word_aux</em> attribute value
				 * @return
				 */
				SGMLobjects::SGMLpath* addPath (Glib::ustring id, Glib::ustring word_cnt, Glib::ustring labels,
													Glib::ustring file, Glib::ustring channel, Glib::ustring sequence,
													Glib::ustring R_T1, Glib::ustring R_T2, Glib::ustring word_aux) ;

				/**
				 * Constructor
				 * @param id		<em>id</em> attribute value
					 */
				SGMLspeaker(Glib::ustring id) ;
				virtual ~SGMLspeaker()  ;
				/** to string method */
				Glib::ustring toString() ;
		} ;

		/**
		* @class 		SGMLcategory
		* @ingroup		Formats
		*
		* Class representing an SGML category block.\n
		*/
		class SGMLcategory
		{
			public:
				Glib::ustring id ;		/**< <em>id</em> attribute value */
				Glib::ustring title ;	/**< <em>title</em> attribute value */
				Glib::ustring desc ;	/**< <em>desc</em> attribute value */

				/**
				 * Constructor
				 * @param id		<em>id</em> attribute value
				 * @param title		<em>title</em> attribute value
				 * @param desc		<em>desc</em> attribute value
				 */
				SGMLcategory(Glib::ustring id, Glib::ustring title, Glib::ustring desc) ;
				virtual ~SGMLcategory() {} ;
		} ;

		/**
		* @class 		SGMLlabel
		* @ingroup		Formats
		*
		* Class representing an SGML label block.\n
		*/
		class SGMLlabel
		{
			public:
				Glib::ustring id ;		/**< <em>id</em> attribute value */
				Glib::ustring title ;	/**< <em>title</em> attribute value */
				Glib::ustring desc ;	/**< <em>desc</em> attribute value */

				/**
				 * Constructor
				 * @param id		<em>id</em> attribute value
				 * @param title		<em>title</em> attribute value
				 * @param desc		<em>desc</em> attribute value
				 */
				SGMLlabel(Glib::ustring id, Glib::ustring title, Glib::ustring desc) ;
				virtual ~SGMLlabel() {} ;
		} ;

	public:

		/**
		 * Constructor
		 */
		SGMLobjects()  ;
		virtual ~SGMLobjects() ;

		/**
		 * Print all objects in standard output
		 * @note 	Debug method
		 */
		void print() ;

		/**
		 * Adds a speaker
		 * @param id	Speaker <em>id</em> attribute value
		 * @return		Pointer on the newly created speaker
		 */
		SGMLspeaker* addSpeaker(Glib::ustring id) ;

		/**
		 * Adds a new label
		 * @param id		Label <em>id</em> attribute value
		 * @param title		Label <em>title</em> attribute value
		 * @param desc		Label <em>desc</em> attribute value
		 * @return			Pointer on the newly created label
		 */
		SGMLentry* addLabel(Glib::ustring id, Glib::ustring title, Glib::ustring desc) ;

		/**
		 * Adds a new category
		 * @param id		Category <em>id</em> attribute value
		 * @param title		Category <em>title</em> attribute value
		 * @param desc		Category <em>desc</em> attribute value
		 * @return			Pointer on the newly created label
		 */
		SGMLcategory* addCategory(Glib::ustring id, Glib::ustring title, Glib::ustring desc) ;

		/**
		 * Specifies general information relative to the SGML file
		 * @param title					<em>title</em> attribute value
		 * @param ref_fname				<em>ref_fname</em> attribute value
		 * @param hyp_fname				<em>hyp_fname</em> attribute value
		 * @param creation_date			<em>creation_date</em> attribute value
		 * @param format				<em>format</em> attribute value
		 * @param frag_corr				<em>frag_corr</em> attribute value
		 * @param opt_del				<em>opt_del</em> attribute value
		 * @param weight_ali			<em>weight_ali</em> attribute value
		 * @param weight_filename		<em>weight_filename</em> attribute value
		 */
		void setData(Glib::ustring title, Glib::ustring ref_fname, Glib::ustring hyp_fname,
								Glib::ustring creation_date, Glib::ustring format,
								Glib::ustring frag_corr, Glib::ustring opt_del,
								Glib::ustring weight_ali, Glib::ustring weight_filename) ;

		/**
		 * Gets number of SGML path encountered.
		 * @return			Number of paths
		 */
		int getNBpaths() { return paths.size() ; }

		/**
		 * Add the given path to the SGML object
		 * @param path		Pointer on the path to add.
		 */
		void addPath(SGMLobjects::SGMLpath* path) ;

		/**
		 * Gets the higher path sequence.
		 * @return			The last sequence (numeric order)
		 */
		int getLastSequence() { return last_sequence; }

		/**
		 * Accessor to the nNth path
		 * @param n		Number of the path we want (sequence attribute)
		 * @return		Pointer on the corresponding path
		 */
		SGMLobjects::SGMLpath* getPathN(int n) ;

	private:
		Glib::ustring title ;
		Glib::ustring ref_fname ;
		Glib::ustring hyp_fname ;
		Glib::ustring creation_date ;
		Glib::ustring format ;
		Glib::ustring frag_corr ;
		Glib::ustring opt_del ;
		Glib::ustring weight_ali ;
		Glib::ustring weight_filename ;

		std::map<int,SGMLobjects::SGMLpath*> paths ;
		std::vector<SGMLobjects::SGMLspeaker*> speakers ;
		std::vector<SGMLobjects::SGMLlabel*> labels ;
		std::vector<SGMLobjects::SGMLcategory*> categories ;

		int last_sequence ;
} ;

#endif /* SGMLOBJECTS_H_ */
