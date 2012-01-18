/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/* $Id */

/**
 * @file	TXTWriter.h
 */

#ifndef _TXTWRITER_H_
#define _TXTWRITER_H_

#include <ostream>
#include "DataModel/DataModel.h"

/**
* @class 	TXTWriter
* @ingroup	Formats
*
* Specific text handler for writing data into TXT format.\n
*/
class TXTWriter
{
	public:
		TXTWriter() : m_printTimeCode(false), m_printDetailed(false) {};

		/**
		 * Writes TranscriberAG data into TXT format
		 * @param out				Stream the data will be written to.
		 * @param data				Pointer on the AG model containing transcription data.
		 * @return			0
		 */
		int write(std::ostream& out, tag::DataModel* data);

		/**
		 * Indicates whether the writer will print timecodes.
		 * @param b		True for printing timecode, False otherwise
		 */
		void setPrintTimecode(bool b) { m_printTimeCode = b; if ( !b) m_printDetailed = false; }

		/**
		 * Indicates whether the writer will print detailed text
		 * @param b		True for printing details, False otherwise
		 */
		void setPrintDetailed(bool b) { m_printDetailed = b; if ( b) m_printTimeCode = true; }

		/**
		 * Renders all element
		 * @param[out] 	out  				container to receive elements
		 * @param 		mainstream_types	available elements (mainstream types)
		 * @param 		itype				type level
		 * @param 		parent				annotation from which begin the rendering ("" for totol rendering)
		 * @param 		notrack				track number
		 */
		void renderAll(ostream& out, const vector<string>& mainstream_types, int itype=0, const string& parent="", int notrack=0);

		/**
		 * Renders section start
		 * @param[out] out		container
		 * @param id			Annotation id
		 */
		void renderSectionStart(ostream& out, const string& id);
		/**
		 * Renders section end
		 * @param[out] out		container
		 * @param id			Annotation id
		 */
		void renderSectionEnd(ostream& out, const string& id);

		/**
		 * Renders turn start
		 * @param[out] out		container
		 * @param id			Annotation id
		 */
		void renderTurnStart(ostream& out, const string& id);

		/**
		 * Renders turn end
		 * @param[out] out		container
		 * @param id			Annotation id
		 */
		void renderTurnEnd(ostream& out, const string& id);

		/**
		 * Renders segment start
		 * @param[out] out		container
		 * @param id			Annotation id
		 */
		void renderSegmentStart(ostream& out, const string& id);

		/**
		 * Renders section end
		 * @param[out] out		container
		 * @param id			Annotation id
		 */
		void renderSegmentEnd(ostream& out, const string& id);

		/**
		 * Renders base element
		 * @param[out] out		container
		 * @param id			Annotation id
		 */
		void renderBaseElement(ostream& out, const string& id);

	private:

		void renderQualifiersAtStart(ostream& out, const string& id, bool no_text=false);
		void renderQualifiersAtEnd(ostream& out, const string& id, bool no_text=false);

		std::string header();
		std::string trailer();
		std::string noSpeaker();
		std::string section_beg();
		std::string section_end();
		std::string speaker_beg(tag::Speaker::Gender gender=tag::Speaker::UNDEF_GENDER);
		std::string speaker_end();


		tag::DataModel* m_data;
		string m_graphtype;
		string m_graphId;
		bool m_printTimeCode;
		bool m_printDetailed;


};

#endif /*  _TXTWRITER_H_ */
