/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

/**
 * @file	HTMLWriter.h
 */

#ifndef _HTMLWRITER_H_
#define _HTMLWRITER_H_

#include <ostream>
#include "DataModel/DataModel.h"

/**
* @class 	HTMLWriter
* @ingroup	Formats
*
* Specific text handler for writing data into CHAT format.\n
*/
class HTMLWriter
{
	public:
		HTMLWriter() : m_printTimeCode(false) {};

		/**
		 * Writes TranscriberAG data into HTML format
		 * @param out				Stream the data will be written to.
		 * @param data				Pointer on the AG model containing transcription data.
		 * @return			0
		 */
		int write(std::ostream& out, tag::DataModel* data);

		/**
		 * Indicates whether the writer will print timecodes.
		 * @param b		True for printing timecode, False otherwise
		 */
		void setPrintTimecode(bool b) { m_printTimeCode = b; }

	private:
		void renderAll(ostream& out, const vector<string>& mainstream_types, int itype=0, const string& parent="", int notrack=0);
		void renderSectionStart(ostream& out, const string& id);
		void renderSectionEnd(ostream& out, const string& id);
		void renderTurnStart(ostream& out, const string& id);
		void renderTurnEnd(ostream& out, const string& id);
		void renderSegmentStart(ostream& out, const string& id);
		void renderSegmentEnd(ostream& out, const string& id);
		void renderBaseElement(ostream& out, const string& id);
		void renderQualifiersAtStart(ostream& out, const string& id, bool no_text=false);
		void renderQualifiersAtEnd(ostream& out, const string& id, bool no_text=false);

		std::string header();
		std::string trailer();
		std::string noSpeaker();
		std::string section_beg();
		std::string section_end();
		std::string speaker_beg(tag::Speaker::Gender gender=tag::Speaker::UNDEF_GENDER);
		std::string speaker_end();

	private:
		static std::string speaker_m;
		static std::string speaker_f;

		tag::DataModel* m_data;
		string m_graphtype;
		string m_graphId;
		bool m_printTimeCode;
};

#endif /*  _HTMLWRITER_H_ */
