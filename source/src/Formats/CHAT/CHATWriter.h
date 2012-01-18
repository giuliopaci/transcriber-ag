/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */
/**
 * @file	CHATWriter.h
 */

#ifndef _CHATWRITER_H_
#define _CHATWRITER_H_

#include <ostream>
#include "DataModel/DataModel.h"

/**
* @class 	CHATWriter
* @ingroup	Formats
*
* Specific text handler for writing data into CHAT format.\n
*/
class CHATWriter
{
	public:
		/**
		 * Writes TranscriberAG data into CHAT format
		 * @param out		Stream the data will be written to.
		 * @param data		Pointer on the AG model containing transcription data.
		 * @param name		- deprecated -
		 * @return			0
		 */
		int		write(std::ostream& out, tag::DataModel* data, const string& name);

		/**
		 * Accessor to all speaker used in the transcription
		 * @param data		Pointer on the AG model containing transcription data.
		 * @return			All speakers (separated by coma) in a string representation.
		 */
		string	getParticipants(tag::DataModel* data);
	
		/**
		 * Accessor to speaker IDs details in CHAT format
		 * @param data		Pointer on the AG model containing transcription data.
		 * @return			All speaker IDs (one by line) in a string representation.
		 */
		string	getParticipantsIDs(tag::DataModel* data);

		/**
		 * Sets track identifier
		 * @param id		Track id
		 */
		void	setTrackID(int id)	{ trackID = id; }	// -1 -> no track ID

	private:
		bool	hasOverlappingSegments(tag::DataModel* data, tag::SignalSegment& s);
		string	processSegmentText(tag::DataModel* data, string segmentID);
		string formatSPKid(string spkID);

		int		startOffset;
		int		endOffset;
		int		trackID;
};

#endif /*  _CHATWRITER_H_ */

