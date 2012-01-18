/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/* $Id */

/**
 * @file	STMWriter.h
 */

#ifndef _STMWRITER_H_
#define _STMWRITER_H_

#include <ostream>
#include "DataModel/DataModel.h"

/**
* @class 	STMWriter
* @ingroup	Formats
*
* Specific text handler for writing data into STM format.\n
*/
class STMWriter
{
	public:

	STMWriter() : m_name("stm") {};
	/**
	 * Writes TranscriberAG data into TXT format
	 * @param out				Stream the data will be written to.
	 * @param data				Pointer on the AG model containing transcription data.
	 * @return			0
	 */
	int write(std::ostream& out, tag::DataModel* data);

	/** @param name root file name */
	void setName(const std::string& name) { m_name=name; }

	private:
		typedef struct {
			string id;
			string type;
			float so;
			float eo;
		} BgDef;


		void renderAll(ostream& out, const vector<string>& mainstream_types, int itype=0, const string& parent="", int notrack=0);
		bool renderSectionStart(ostream& out, const string& id);
		void renderSectionEnd(ostream& out, const string& id);
		bool renderTurnStart(ostream& out, const string& id);
		void renderTurnEnd(ostream& out, const string& id);
		bool renderSegmentStart(ostream& out, const string& id);
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


		tag::DataModel* m_data;
		string m_graphtype;
		string m_graphId;
		string m_name;
		string m_prefix;
		string m_lab;
		string m_spkgender;
		string m_final_text;
		bool m_nospeechTurn;
		bool m_nospeechSeg;

		vector<BgDef> m_backgrounds;

		int	m_track;
};

#endif /*  _STMWRITER_H_ */
