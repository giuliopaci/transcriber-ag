/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 *  $Id$
 *
 *  @file DataModelMerge.cpp
 *  @brief data model for Transcriber application
 *    implements data models merge operations
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <glib/gstdio.h>

#include <fstream>
#include <sstream>
#include <set>


#include <ag/AGException.h>
#include <ag/agfio.h>

#include "Common/util/Utils.h"
#include "Common/util/StringOps.h"
#include "DataModelMerge.h"
#include "Common/FileInfo.h"
#include "Common/VersionInfo.h"


namespace tag {


bool DataModelMerge::merge(DataModel& dest, DataModel& merged, MergeType merge_type, float merge_offset)
{
	m_trackstart=0;
	bool ok = true;
	mergeMetaData(dest, merged, merge_type);
	mergeSpeakers(dest, merged, merge_type);
	mergeSignals(dest, merged, merge_type);
	mergeAnnotations(dest, merged, merge_type);
	return ok;
}

void DataModelMerge::mergeMetaData(DataModel& dest, DataModel& merged, MergeType merge_type)
{
	// TODO
}

void DataModelMerge::mergeSpeakers(DataModel& dest, DataModel& merged, MergeType merge_type)
{
	SpeakerDictionary& mdic = merged.getSpeakerDictionary();
	SpeakerDictionary& curdic = dest.getSpeakerDictionary();
	SpeakerDictionary toadd = merged.getSpeakerDictionary();

	SpeakerDictionary::iterator it;
	for ( it=toadd.begin(); it != toadd.end(); ++it ) {
		if (curdic.existsSpeaker(it->first) ) {

			const Speaker& spk=curdic.getSpeaker(it->first);
//			cout << "it->id=" << it->first << " it->name=" << it->second.getFullName() << endl;
//			cout << "spk.id=" << spk.getId() << " spk.name=" << spk.getFullName() << endl;

			switch ( merge_type ) {
			case merge_append :
			{
				char name[40];
				// then rename speaker in merged and add to current dic.
				bool is_default_name = ( toadd.getDefaultName(it->first).compare(it->second.getFullName()) == 0 );
				string newid ;
				do {
					newid = mdic.getUniqueId(name);
					it->second.setId(newid);
					if ( is_default_name ) {
						it->second.setLastName(mdic.getDefaultName(newid));
					}
					mdic.addSpeaker(it->second);
				} while (curdic.existsSpeaker(newid) );

				// update all speaker-attached annotations in merged data model
				merged.replaceSpeaker(it->first, newid, "", false);

				// insert updated speaker in speaker dictionary
				curdic.addSpeaker(it->second);
				break;
			}
			case merge_replace_if_exists :
				curdic.updateSpeaker(it->second);
				break;
			case merge_keep_if_exists :
				break;
			}
		} else { // just add to speaker dictionary;
			curdic.addSpeaker(it->second);
		}
	}
}

void DataModelMerge::mergeSignals(DataModel& dest, DataModel& merged, MergeType merge_type)
{
	switch ( merge_type ) {
	case merge_append :
	{
//		int notrack_hint = dest.getNbTracks();
		int nbtracks = dest.getNbTracks();
		m_trackstart = nbtracks;
		// addSignal(merged.getSignalFileURL(), ");
		dest.addSignal(merged.getSignalFileURL(), "audio", "wav", "pcm", merged.getNbTracks(), -1, false);
		break;
	}
	case merge_replace_if_exists :
		break;
	case merge_keep_if_exists :
		break;
	}
}


void DataModelMerge::mergeAnnotations(DataModel& dest, DataModel& merged, MergeType merge_type)
{
	// TODO -> check signal track !!!!
	switch ( merge_type ) {
	case merge_append :
	{
		// append all anchors and all annotations
		map<string, string>::const_iterator itg;
		for ( itg = merged.getGraphs().begin(); itg != merged.getGraphs().end(); ++itg ) {
			addGraph(dest, merged, itg->first, itg->second);
		}

		break;
	}
	case merge_replace_if_exists :
		break;
	case merge_keep_if_exists :
		break;
	}
}

/**
 * add full graph to current set
 */
void DataModelMerge::addGraph(DataModel& dest, DataModel& merged, const string& graphtype, const string& agid)
{
	// retrieve corresponding graph
	// first add anchors
	string src_agId = merged.getAG(graphtype);
	const list<AnchorId>& anchors = GetAnchorSet(src_agId);
	list<AnchorId>::const_iterator ita;
	string dest_agId = dest.getAG(graphtype);

	if ( dest_agId == "" ) {
		dest.initAnnotationGraphs(graphtype, "", "");
		dest_agId = dest.getAG(graphtype);
	}

	map<string, string> aids;
	set<SignalId> signals;
	int prevtrack = -1;
	bool oksig = false;
	if ( merged.getNbTracks() == 1 ) {
		// only one signal -> prepare signal ids for all anchors
		signals.insert(dest.getSignalId(m_trackstart));
		oksig = true;
	}

	for (ita = anchors.begin(); ita != anchors.end(); ++ita ) {
		// get anchor signal Id
		if ( GetAnchored(*ita) ) {
			if ( ! oksig ) {
				set<SignalId> sigIds = GetAnchorSignalIds(*ita) ;
				if ( sigIds.size() > 0 ) {
					set<SignalId>::iterator its = sigIds.begin();
					int notrack = merged.getSignalNotrack(*its)+m_trackstart;
					if ( notrack != prevtrack ) {
						signals.clear();
						signals.insert(dest.getSignalId(notrack));
						prevtrack = notrack;
					}
				}
			}
			aids[*ita] = CreateAnchor(dest_agId, GetAnchorOffset(*ita), GetOffsetUnit(*ita), signals);
		} else
			aids[*ita] = CreateAnchor(dest_agId);
	}

	// now insert all annotations
	set<AnnotationId> ids = GetAnnotationSet(src_agId);
	set<AnnotationId>::iterator it;
	for ( it=ids.begin(); it != ids.end(); ++it ) {
		const string& nid = CreateAnnotation(dest_agId, aids[GetStartAnchor(*it)], aids[GetEndAnchor(*it)], GetAnnotationType(*it));
		map<string, string> features = GetFeatures(*it);
		SetFeatures(nid, features);
	}

	// done
}

}
