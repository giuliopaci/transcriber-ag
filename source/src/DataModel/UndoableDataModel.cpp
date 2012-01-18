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
 *  @file UndoableDataModel.cpp
 *  @brief  implements data model Undo / Redo mecanism
 */

#include <fstream>
#include <sstream>
#include <iterator>
#include <list>
#include <set>

#include "UndoableDataModel.h"
#include "Common/util/StringOps.h"
#include "Common/util/Utils.h"


#include <ag/AGException.h>
#include <ag/agfio.h>

using namespace std;

set<string> tag::UndoableDataModel::not_a_feature;

namespace tag {

//#define DOTRACE_UNDOM true
#define DOTRACE_UNDOM false

bool UndoableDataModel::isFeatureItem(const string& key)
{
	// TODO ici ajuster les items en fonction de ce que produit GetAnnotationInfo
	if ( not_a_feature.empty() )  {
		not_a_feature.insert("id");
		not_a_feature.insert("type");
		not_a_feature.insert("start");
		not_a_feature.insert("end");
	}
	return ( not_a_feature.find(key) == not_a_feature.end() ) ;
}

bool UndoableDataModel::isUndoableKey(const string& key)
{
	if ( //key.compare("text")==0
//			|| key.compare("lang")==0
			key.compare("convention_id")==0
			|| key.compare("audio_settings")==0
			|| key.compare("speaker_hint")==0
			|| key.compare("wid")==0
			|| key.compare("path_hint")==0
			//TODO remove
			|| key.compare("speakers")==0
		)
		return false ;
	else
		return true ;
}

#define LOG_FINE 	Log::setTraceLevel(Log::FINE);
#define LOG_RESET	Log::resetTraceLevel();

void UndoableDataModel::onUndoRedo(const std::string& eventData, bool undo)
{
	const string& operation = (undo==1 ? " UNDO " : " REDO ") ;


	if (DOTRACE_UNDOM)
	{
		LOG_FINE
		Log::trace() << ">>>>>>>>>>>>>>>>>>>>>>>>>>> IN >UndoableDataModel::onUndoRedo <" << operation << "> " << eventData << endl;
	}

	if ( undo )
		undoAction(eventData);
	else
		redoAction(eventData);

	if (DOTRACE_UNDOM)
		LOG_RESET
}

void UndoableDataModel::undoAction(const std::string& eventData)
{
	istringstream is(eventData);
	string action, what, id;
	is >> action >> what >> id;

	// TODO si on veut passer par updateView pour restaurer affichage texte
	// -> voir l'envoi de signaux ElementModified
	try
	{
		//  delete element
		if ( action == "create" )
		{
			if ( what == "annot") {
				emitUndoRedoSignal(getElementType(id), id, DELETED) ;
				DeleteAnnotation(id);
				setUpdated(true) ;
			}
			else if ( what == "anchor" ) {
				emitUndoRedoSignal(getElementType(id), id, DELETED) ;
				DeleteAnchor(id);
				setUpdated(true) ;
			}
		}
		// re-create element
		else if ( action == "delete" )
		{
			if ( what == "annot" ) {
				string type, start, end, startId, endId;
				is >> type >> start >> end >> startId >> endId ;
				string buf;
				getline(is, buf);
				map<string, string> data;
				map<string, string>::iterator it;
				StringOps s(buf) ;
				s.getDCSVItems(data);
				string newid = CreateAnnotation(id, startId, endId, type);
				if ( newid != id ) {
					Log::trace() << "<?> STACK CORRUPTION: undo delete annot should be " << id << " but is " << newid << endl;
					Log::trace() << "\t - exist " << id << " ? "  << existsElement(id) << endl;
					m_signalUndoRedoStackCorruption.emit(false) ;
				}
				emitUndoRedoSignal(getElementType(id), id, INSERTED) ;
				bool updated = false ;
				for ( it = data.begin(); it != data.end(); ++it )
				{
					// reset annot features
					if ( isFeatureItem(it->first) )
					{
						SetFeature(id, it->first, it->second);
						updated = true ;
					}
				}
				if (updated)
					emitUndoRedoSignal(getElementType(id), id, UPDATED) ;
				setUpdated(true) ;
			}
			else if ( what == "anchor") {
				string buf;
				getline(is, buf);
				map<string, string> data;
				StringOps s(buf) ;
				s.getDCSVItems(data);
				string newid;
				if ( data.empty() )
					newid = CreateAnchor(id);
				else {
					float offset = atof((data["offset"]).c_str());
					buf = data["sigIds"] ;
					vector<string> v;
					StringOps(buf).split(v, ",");
					set<SignalId> sigIds;
					vector<string>::iterator itv;
					string signalId = "" ;
					for ( itv=v.begin(); itv != v.end(); ++itv ) {
						sigIds.insert(*itv);
						signalId = *itv ;
					}
					string anchored = data["anchored"] ;
					if (anchored=="1") {
						newid = CreateAnchor(id, offset, TRANS_ANCHOR_UNIT, sigIds);
						// mettre à jour le cache d'association ancre-notrack
						if (!signalId.empty()) {
							addAnchorToTrackMap(newid, getSignalNotrack(signalId) ) ;
						}
					}
					else {
						newid = CreateAnchor(id, sigIds) ;
						SetOffsetUnit(newid, TRANS_ANCHOR_UNIT) ;
					}
				}
				if ( newid != id ) {
					Log::trace() << "<?> STACK CORRUPTION undo delete anchor should be " << id << " but is " << newid << endl;
					Log::trace() << "\t - exist " << id << " ? "  << existsAnchor(id) << endl;
					m_signalUndoRedoStackCorruption.emit(false) ;
				}
				setUpdated(true) ;
			}
			else if ( what == "feature" ) {
				string buf;
				getline(is, buf);
				map<string, string> data;
				StringOps s(buf) ;
				s.getDCSVItems(data);
				SetFeature(id, data["key"], data["value"]);
				emitUndoRedoSignal(getElementType(id), id, UPDATED) ;
				setUpdated(true) ;
			}
		}
		// delete 2nd item
		else if ( action == "split")
		{
			string prev;
			is >> prev;
			if ( what == "annot") {
				emitUndoRedoSignal(getElementType(prev), prev, DELETED) ;
				undoSplitAnnotation(id, prev) ;
				emitUndoRedoSignal(getElementType(id), id, UPDATED) ;
				setUpdated(true) ;
			}
			else if ( what == "anchor" ) {
				emitUndoRedoSignal(getElementType(prev), prev, DELETED) ;
				undoSplitAnchor(id, prev) ;
				emitUndoRedoSignal(getElementType(id), id, UPDATED) ;
				setUpdated(true) ;
			}
		}
		// unset
		else if ( action == "set" )
		{
			if ( what == "feature") {
				string buf;
				getline(is, buf);
				map<string, string> data;
				StringOps s(buf) ;
				s.getDCSVItems(data);
				const string& key = data["key"];
				if ( ! data["old"].empty() )
					SetFeature(id, key, data["old"]);
				else if ( ExistsFeature(id, key) ) DeleteFeature(id, key);
				emitUndoRedoSignal(getElementType(id), id, UPDATED) ;
				setUpdated(true) ;
			}
			else if ( what == "start" ) { // reset previous anchor
				string prev;
				is >> prev;
				SetStartAnchor(id, prev);
				emitUndoRedoSignal(getElementType(id), id, UPDATED) ;
				setUpdated(true) ;
			}
			else if ( what == "end" ) { // reset previous anchor
				string prev;
				is >> prev;
				SetEndAnchor(id, prev);
				emitUndoRedoSignal(getElementType(id), id, UPDATED) ;
				setUpdated(true) ;
			}
			else if ( what == "link" ) { // delete links
				string toBeRemoved ;
				is >> toBeRemoved ;
				anchorLinks().removeFromLinks(id, toBeRemoved) ;
//				emitUndoRedoSignal(getElementType(id), id, UPDATED) ;
				setUpdated(true) ;
			}
			else if ( what == "offset" ) { // reset previous anchor
				string buf;
				getline(is, buf);
				map<string, string> data;
				StringOps s(buf) ;
				s.getDCSVItems(data);

				float offset = atof((data["offset"]).c_str());
				string anchored = data["anchored"] ;
				if (anchored=="1")
				{
					SetAnchorOffset(id, offset);
					emitUndoRedoSignal(getElementType(id), id, UPDATED) ;
				}
				else
				{
					UnsetAnchorOffset(id) ;
					emitUndoRedoSignal(getElementType(id), id, UNANCHORED) ;
				}

				setUpdated(true) ;
			}
		}
		// re-set
		else if ( action == "unset" )
		{
			if ( what == "link") //
			{
				string toBeInserted ;
				is >> toBeInserted ;
				anchorLinks().insertIntoLinks(id, toBeInserted) ;
//				emitUndoRedoSignal(getElementType(id), id, UPDATED) ;
				setUpdated(true) ;
			}
			else
			{
				string buf;
				getline(is, buf);
				map<string, string> data;
				StringOps s(buf) ;
				s.getDCSVItems(data);
				if ( atoi((data["anchored"]).c_str()) == 1 )
				{
					float offset = atof((data["offset"]).c_str());
					SetAnchorOffset(id, offset);
					emitUndoRedoSignal(getElementType(id), id, UPDATED) ;
//					emitUndoRedoSignal(getElementType(id), id, INSERTED) ;
					emitUndoRedoSignal(getElementType(id), id, ANCHORED) ;
					setUpdated(true) ;
				}
			}
		}
		// delete copied element
		else if ( action == "copy" )
		{
			if (what == "annot" ) {
				string prev ;
				is >> prev ;
				if (!prev.empty()) {
					emitUndoRedoSignal(getElementType(prev), prev, DELETED) ;
					DeleteAnnotation(prev) ;
					setUpdated(true) ;
				}
			}
		}
	}
	catch (AGException& e) {
		MSGOUT << "(UndoableDataModel::undoAction) Caught AGException : " << e.error() << ", eventdata=" << eventData << endl;
		m_signalUndoRedoStackCorruption.emit(false) ;
	}
}

void UndoableDataModel::redoAction(const std::string& eventData)
{
	istringstream is(eventData);
	string action, what, id;
	is >> action >> what >> id;
	try
	{
		//   re-create element
		if ( action == "create" )
		{
			if ( what == "annot") {
				string buf;
				getline(is, buf);
				map<string, string> data;
				map<string, string>::iterator it;
				StringOps s(buf) ;
				s.getDCSVItems(data);
				const string& newid = CreateAnnotation(id, data["start"], data["end"], data["type"]);
				if ( newid != id ) {
					Log::trace() << "<?> STACK CORRUPTION redo create annot should be " << id << " but is " << newid << endl;
					Log::trace() << "\t - exist " << id << " ? "  << existsElement(id) << endl;
					m_signalUndoRedoStackCorruption.emit(true) ;
				}
				emitUndoRedoSignal(getElementType(id), id, INSERTED) ;
				bool updated = false ;
				for ( it=data.begin(); it != data.end(); ++it ) {
					// reset annot features
					if ( isFeatureItem(it->first) ) {
						SetFeature(id, it->first, it->second);
						updated = true ;
					}
				}
				if (updated)
					emitUndoRedoSignal(getElementType(id), id, UPDATED) ;
				setUpdated(true) ;
			}
			else if ( what == "anchor" ) {
				string buf;
				getline(is, buf);
				map<string, string> data;
				StringOps s(buf) ;
				s.getDCSVItems(data);
				string newid;
				if ( data.empty() )
					newid = CreateAnchor(id);
				else {
					float offset = atof((data["offset"]).c_str());
					buf = data["sigIds"];
					vector<string> v;
					StringOps(buf).split(v, ",");
					set<SignalId> sigIds;
					vector<string>::iterator itv;
					string signalId = "" ;
					for ( itv=v.begin(); itv != v.end(); ++itv ) {
						sigIds.insert(*itv);
						signalId = *itv ;
					}
					string anchored = data["anchored"] ;
					if (anchored=="1") {
						newid = CreateAnchor(id, offset, TRANS_ANCHOR_UNIT, sigIds);
						// mettre à jour le cache d'association ancre-notrack
						if (!signalId.empty()) {
							addAnchorToTrackMap(newid, getSignalNotrack(signalId) ) ;
						}
					}
					else {
						newid = CreateAnchor(id, sigIds) ;
						SetOffsetUnit(newid, TRANS_ANCHOR_UNIT) ;
					}
				}
				if ( newid != id ) {
					Log::trace() << "<?> STACK CORRUPTION redo create anchor should be " << id << " but is " << newid << endl;
					Log::trace() << "\t - exist " << id << " ? "  << existsAnchor(id) << endl;
					m_signalUndoRedoStackCorruption.emit(true) ;
				}
				setUpdated(true) ;
			}
		}
		// re-delete element
		else if ( action == "delete" )
		{
			if ( what == "annot" ) {
				AnchorId end = GetEndAnchor(id) ;
				emitUndoRedoSignal(getElementType(id), id, DELETED) ;
				DeleteAnnotation(id);
				emitUndoRedoSignal("", end, REDO_DELETED) ;
				setUpdated(true) ;
			}
			else if ( what == "anchor") {
				DeleteAnchor(id);
				//emitUndoRedoSignal("", end, DELETED) ;
				setUpdated(true) ;
			}
			else if ( what == "feature" ) {
				string buf;
				getline(is, buf);
				map<string, string> data;
				StringOps s(buf) ;
				s.getDCSVItems(data);
				DeleteFeature(id, data["key"]);
				emitUndoRedoSignal(getElementType(id), id, UPDATED) ;
				setUpdated(true) ;
			}
		}
		// re-split
		else if ( action == "split")
		{
			if ( what == "annot") {
				string prev;
				is >> prev;
				string anchor ;
				is >> anchor ;
				// var used to send the anchorId we want to receive,
				// and used to receive the new anchorId created
				// (so we can compare)
				AnchorId anchor_created = anchor ;
				setInhibateSignals(true);
				const list<AnnotationId>& l = agSplitAnnotation(id, prev, anchor_created);
				setInhibateSignals(false) ;
				if ( l.back()!=prev || anchor_created!=anchor) {
					Log::trace() << "<?> CORRUPTION PILE REDO split annotation should be " << prev << " but is " << l.back() << endl;
					Log::trace() << "\t - exist " << prev << " ? " << existsAnchor(prev) << endl;
					m_signalUndoRedoStackCorruption.emit(true) ;
				}
				emitUndoRedoSignal(getElementType(l.back()), l.back(), INSERTED) ;
				emitUndoRedoSignal(getElementType(id), id, UPDATED) ;
				setUpdated(true) ;
			}
			else if ( what == "anchor" ) {
				string prev;
				is >> prev;
				setInhibateSignals(true);
				const string new_id  = agSplitAnchor(id, prev) ;
				setInhibateSignals(false) ;
				if ( new_id != prev ) {
					Log::trace() << "<?> CORRUPTION PILE REDO split anchor should be " << prev << " but is " << new_id << endl;
					Log::trace() << "\t - exist " << prev << " ? " << existsAnchor(prev) << endl;
					m_signalUndoRedoStackCorruption.emit(true) ;
				}
				//emitUndoRedoSignal(getElementType(new_id), new_id, INSERTED) ;
				emitUndoRedoSignal(getElementType(id), id, UPDATED) ;
				setUpdated(true) ;
			}
		}
		// re-set
		else if ( action == "set" )
		{
			 if ( what == "feature" ) {
				string buf;
				getline(is, buf);
				map<string, string> data;
				StringOps s(buf) ;
				s.getDCSVItems(data);
				const string& key = data["key"];
				if ( ! data["value"].empty() )
					SetFeature(id, key, data["value"]);
				else if ( ExistsFeature(id, key) ) DeleteFeature(id, key);
				emitUndoRedoSignal(getElementType(id), id, UPDATED) ;
				setUpdated(true) ;
			 }
			else if ( what == "link") // reset a link between 2 anchors
			{
				string toBeInserted ;
				is >> toBeInserted ;
				anchorLinks().insertIntoLinks(id, toBeInserted) ;
//				emitUndoRedoSignal(getElementType(id), id, UPDATED) ;
				setUpdated(true) ;
			}
			else if ( what == "start" ) { // reset previous anchor
				string prev, next ;
				is >> prev >> next;
				SetStartAnchor(id, next);
				emitUndoRedoSignal(getElementType(id), id, UPDATED) ;
				setUpdated(true) ;
			}
			else if ( what == "end" ) { // reset previous anchor
				string prev, next ;
				is >> prev >> next;
				SetEndAnchor(id, next);
				emitUndoRedoSignal(getElementType(id), id, UPDATED) ;
				setUpdated(true) ;
			}
			else if ( what == "offset" ) { // reset previous anchor
				string buf;
				getline(is, buf);
				map<string, string> data;
				StringOps s(buf) ;
				s.getDCSVItems(data);
				float offset = atof((data["new"]).c_str());
				bool was_anchored = GetAnchored(id) ;
				SetAnchorOffset(id, offset);
				if (was_anchored)
					emitUndoRedoSignal(getElementType(id), id, UPDATED) ;
				else
					emitUndoRedoSignal(getElementType(id), id, ANCHORED) ;
				setUpdated(true) ;
			}
		}
		// re-unset
		else if ( action == "unset" )
		{
			if ( what == "feature") {
				string buf;
				getline(is, buf);
				map<string, string> data;
				StringOps s(buf) ;
				s.getDCSVItems(data);
				DeleteFeature(id, data["key"]);
				emitUndoRedoSignal(getElementType(id), id, UPDATED) ;
				setUpdated(true) ;
			}
			else if ( what == "link" ) { // re-delete links
				string toBeRemoved ;
				is >> toBeRemoved ;
				anchorLinks().removeFromLinks(id, toBeRemoved) ;
//				emitUndoRedoSignal(getElementType(id), id, UPDATED) ;
				setUpdated(true) ;
			}
			else if ( what == "start" ) { // reset previous anchor
				string prev;
				is >> prev;
				SetStartAnchor(id, prev);
				emitUndoRedoSignal(getElementType(id), id, UPDATED) ;
				setUpdated(true) ;
			}
			else if ( what == "end" ) { // reset previous anchor
				string prev;
				is >> prev;
				SetEndAnchor(id, prev);
				emitUndoRedoSignal(getElementType(id), id, UPDATED) ;
				setUpdated(true) ;
			}
			else if (what == "offset") {
				UnsetAnchorOffset(id) ;
				emitUndoRedoSignal(getElementType(id), id, DELETED) ;
//				emitUndoRedoSignal(getElementType(id), id, UPDATED) ;
				emitUndoRedoSignal(getElementType(id), id, UNANCHORED) ;
				setUpdated(true) ;
			}
		}
		// re-copy
		else if ( action == "copy" )
		{
			if ( what == "annot") {
				string prev ;
				is >> prev ;
				setInhibateSignals(true) ;
				const string& new_id = agCopyAnnotation(id, prev) ;
				setInhibateSignals(false) ;
				if (new_id.compare(prev)!=0) {
					Log::trace() << "<?> CORRUPTION PILE REDO copy annot should be " << prev << " but is "<< new_id << endl ;
					Log::trace() << "\t - exist " << prev << " ? "  << existsElement(prev) << endl;
					m_signalUndoRedoStackCorruption.emit(true) ;
				}
				emitUndoRedoSignal(getElementType(new_id), new_id, INSERTED) ;
				setUpdated(true) ;
			}
		}
	}
	catch (AGException& e) {
		MSGOUT << "(UndoableDataModel::redoAction) Caught AGException : " << e.error() << ", eventdata=" << eventData << endl;
		m_signalUndoRedoStackCorruption.emit(true) ;
	}
}

// Basic graph operations
AnnotationId UndoableDataModel::agCreateAnnotation(const Id& id, const AnchorId& start, const AnchorId& end, const string& type)
{

	const AnnotationId& newid= CreateAnnotation(id, start, end, type);
	if ( !getInhibUndoRedo() && !getInhibateSignals()) {
		ostringstream action;
		// store newid in order to reset it when undoing action
		int notrack = getAViewTrack(start,"anchor") ;
		action << "create annot " << newid << " type=" << type << ";start=" << start << ";end=" << end;
		signalUndoableAction().emit(action.str(), notrack);
	}
	return newid;
}

void UndoableDataModel::agDeleteAnnotation(const AnnotationId& id)
{
	if ( !getInhibUndoRedo() && !getInhibateSignals() )
	{
		//> delete features
		std::map<string, string> features = GetFeatures(id);
		std::map<string, string>::iterator it ;
		for (it=features.begin(); it!=features.end(); it++)
			agDeleteFeature(id, it->first) ;

		//> delte annotation
		ostringstream action;
		action << "delete annot " << id << " " << GetAnnotationInfo(id);
		int notrack = getAViewTrack(id,"annot") ;
		signalUndoableAction().emit(action.str(), notrack);
	}
	//DeleteFeature(id, key);
	DeleteAnnotation(id) ;
}

AnnotationId UndoableDataModel::agCopyAnnotation(const AnnotationId& id, const AnnotationId& toBeCreated)
{
	// Re-implement function in order to control the id of new annotation
	// that will be create by the split

	string futureId = toBeCreated ;
	if (futureId.empty())
		futureId = GetAGId(id) ;

	//> create new annotation
	AnchorId start = GetStartAnchor(id) ;
	AnchorId end = GetEndAnchor(id) ;
	const AnnotationId& newid = CreateAnnotation(futureId, start, end, getElementType(id)) ;

	//> propagate all features of id to newid
	std::map<string, string> features = GetFeatures(id);
	SetFeatures(newid, features) ;

	if ( !getInhibUndoRedo() && !getInhibateSignals() ) {
		ostringstream action;
		int notrack = getAViewTrack(id,"annot") ;
		action << "copy annot " << id << " " << newid ;
		signalUndoableAction().emit(action.str(), notrack);
	}
	return newid;
}

list<AnnotationId> UndoableDataModel::agSplitAnnotation(const AnnotationId& id, const AnnotationId& toBeCreated, AnchorId& anchorToBeCreated)
{
	// Re-implement function in order to control the id of new annotation
	// that will be create by the split

	string futureId = toBeCreated ;
	if (futureId.empty())
		futureId = GetAGId(id) ;
	string futurAnchorId = anchorToBeCreated ;
 	if (futurAnchorId.empty())
 		futurAnchorId = futureId ;

	//> stock end anchor of basis annotation
	AnchorId end = GetEndAnchor(id) ;
	std::set<SignalId> signals = GetAnchorSignalIds(end) ;

	//> create new anchor without offset (so we don't set it to m_anchorTrack)
	AnchorId new_anchor = CreateAnchor(futurAnchorId) ;
	SetAnchorSignalIds(new_anchor, signals) ;
	SetOffsetUnit(new_anchor, TRANS_ANCHOR_UNIT) ;

	//> basis annotation receive the newly created anchor as end anchor
	SetEndAnchor(id, new_anchor) ;

	//> create an annotation between newly-created-anchor and old basis-annotation's end anchor
	AnnotationId newid = CreateAnnotation(futureId, new_anchor, end, getElementType(id)) ;

	//> propagate all features of id to newid
	std::map<string, string> features = GetFeatures(id);
	SetFeatures(newid, features) ;

	list<AnnotationId> l ;
	l.push_back(id) ;
	l.push_back(newid) ;

	if ( !getInhibUndoRedo() && !getInhibateSignals() ) {
		ostringstream action;
		int notrack = getAViewTrack(id,"annot") ;
		action << "split annot " << (l.front()).c_str() << " " << (l.back()).c_str() << " " << new_anchor ;
		//copy(l.begin(), l.end(), ostream_iterator<string>(action, " "));
		signalUndoableAction().emit(action.str(), notrack);
	}

	anchorToBeCreated = new_anchor ;

	return l;
}

void UndoableDataModel::undoSplitAnnotation(const AnnotationId& old, const AnnotationId& neww)
{
	//> re-attach old annotation to its old end anchor
	AnchorId neww_end = GetEndAnchor(neww) ;
	SetEndAnchor(old, neww_end) ;

	//> delete newly created annotation and delete newly create anchor
	AnchorId neww_start = GetStartAnchor(neww) ;
	DeleteAnnotation(neww) ;
	DeleteAnchor(neww_start) ;
}

void UndoableDataModel::agSetFeature(const AnnotationId& id, const string& key, const string& value)
{
	if ( !getInhibUndoRedo() && !getInhibateSignals() ) {
		ostringstream action;
		string old = getElementProperty(id, key, "") ;
		action << "set feature " << id << " key=" << key << ";value=" << value << ";old=" << old ;
		if ( isUndoableKey(key) ) {
			int notrack = getAViewTrack(id,"annot") ;
			signalUndoableAction().emit(action.str(), notrack);
		}
	}
	SetFeature(id, key, value);
}

void UndoableDataModel::agDeleteFeature(const AnnotationId& id, const string& key)
{
	if ( !getInhibUndoRedo() && !getInhibateSignals() ) {
		ostringstream action;
		action << "delete feature " << id << " key=" << key << ";value=" << GetFeature(id, key);
		if ( isUndoableKey(key) ) {
			int notrack = getAViewTrack(id,"annot") ;
			signalUndoableAction().emit(action.str(), notrack);
		}
	}
	DeleteFeature(id, key);
}

AnchorId UndoableDataModel::agCreateAnchor(const Id& id)
{
	AnchorId newid = CreateAnchor(id);
	if ( !getInhibUndoRedo() && !getInhibateSignals() ) {
		ostringstream action;
		// store newid in order to reset it when undoing action
		action << "create anchor " << newid << " anchored=0;";
		int notrack = getAViewTrack(newid,"anchor") ;
		signalUndoableAction().emit(action.str(), notrack);
	}
	return newid;
}

AnchorId UndoableDataModel::agCreateAnchor(const Id& id, float offset, string unit, set<SignalId>& sigIds)
{
	AnchorId newid = CreateAnchor(id, offset, unit, sigIds);
	if ( !getInhibUndoRedo() && !getInhibateSignals() ) {
		ostringstream action;
		// store newid in order to reset it when undoing action
		string offset_s = float_to_string(offset) ;
		action << "create anchor " << newid << " anchored=1;offset=" << offset_s << ";sigIds=" ;
		copy(sigIds.begin(), sigIds.end(), ostream_iterator<SignalId>(action, ","));

		int notrack ;
		set<SignalId>::iterator it = sigIds.begin() ;
		if (it!=sigIds.end())
			notrack = getAViewTrack(*it,"anchor") ;
		else
			notrack = getAnchorTrackHint();
		signalUndoableAction().emit(action.str(), notrack);
	}
	return newid;
}

AnchorId UndoableDataModel::agSplitAnchor(const AnchorId& id, const AnchorId& idToCreate)
{
	//redefine splitAnchor in order to control the id returned
	//AnchorId newid = SplitAnchor(id);
	string futureId = idToCreate ;
	if (futureId.empty())
		futureId = GetAGId(id) ;

	// offset to be propagated
	float offset;

	// signals id to be propagated
	std::set<SignalId> signals = GetAnchorSignalIds(id) ;

	//> creation of new anchor
	AnchorId newid = CreateAnchor(futureId);
	SetOffsetUnit(newid, TRANS_ANCHOR_UNIT) ;
	SetAnchorSignalIds(newid, signals) ;
	if (GetAnchored(id)) {
		offset = GetAnchorOffset(id) ;
		SetAnchorOffset(newid, offset) ;
		addAnchorToTrackMap(newid, getAnchorSignalTrack(id)) ;
	}

	//> all outgoing annotations of old one become outgoing annotations of new one
	std::set<string> outgoings = GetOutgoingAnnotationSet(id, "") ;
	std::set<string>::iterator it_out ;
	for (it_out=outgoings.begin(); it_out!=outgoings.end(); it_out++)
		SetStartAnchor(*it_out, newid) ;

	if ( !getInhibUndoRedo() && !getInhibateSignals() ) {
		ostringstream action;
		// store newid in order to reset it when undoing action
		action << "split anchor " << id << " " << newid ;
		int notrack = getAViewTrack(id,"anchor") ;
		signalUndoableAction().emit(action.str(), notrack);
	}

	return newid;
}


void UndoableDataModel::undoSplitAnchor(const AnchorId& old, const AnchorId& neww)
{
	//> all outgoing annotations of neww become outgoing annotations of old one
	std::set<string> outgoings = GetOutgoingAnnotationSet(neww, "") ;
	std::set<string>::iterator it_out ;
	for (it_out=outgoings.begin(); it_out!=outgoings.end(); it_out++)
		SetStartAnchor(*it_out, old) ;

	std::set<string> incoming = GetIncomingAnnotationSet(neww, "") ;
	std::set<string>::iterator it_in ;
	for (it_in=incoming.begin(); it_in!=incoming.end(); it_in++) {
		TRACE_D << "<!> undoSplitAnchor:> " << old << " " << neww << " :> remaining incoming: " << *it_in << endl ;
	}

	//> delete new
	DeleteAnchor(neww) ;
}

void UndoableDataModel::agDeleteAnchor(const AnchorId& id)
{
	if ( !getInhibUndoRedo() && !getInhibateSignals() ) {
		ostringstream action;
		// store newid in order to reset it when undoing action
		bool b=GetAnchored(id);

		action << "delete anchor " << id << " anchored=" << b;
		if ( b ) {
			string offset = float_to_string(GetAnchorOffset(id), 6) ;
			action << ";offset=" << offset  ;
		}
		const set<SignalId> sigIds = GetAnchorSignalIds(id);
		if (sigIds.size()>0) {
			action << ";sigIds=" ;
			copy(sigIds.begin(), sigIds.end(), ostream_iterator<SignalId>(action, ","));
		}
		int notrack = getAViewTrack(id,"anchor") ;
		signalUndoableAction().emit(action.str(), notrack);
	}

	// -- linked ? total unlink
	anchorLinks().unlink(id) ;

	// -- delete in model
	DeleteAnchor(id);
}

void UndoableDataModel::agSetStartAnchor(const AnnotationId& id, const AnchorId& a)
{
	TRACE_D << " agSetStartAnchor " << id << " from " <<  GetStartAnchor(id)  << " to " << a << endl;

	if ( !getInhibUndoRedo() && !getInhibateSignals() ) {
		ostringstream action;
		action << "set start " << id << " " << GetStartAnchor(id) << " " << a;
		int notrack = getAViewTrack(id,"annot") ;
		signalUndoableAction().emit(action.str(), notrack);
	}
	SetStartAnchor(id, a);
}

void UndoableDataModel::agSetEndAnchor(const AnnotationId& id, const AnchorId& a)
{
	TRACE_D << " agSetEndAnchor " << id << " from " <<  GetEndAnchor(id)  << " to " << a << endl;
	if ( !getInhibUndoRedo() && !getInhibateSignals() ) {
		ostringstream action;
		action << "set end " << id << " " << GetEndAnchor(id) << " " << a;
		int notrack = getAViewTrack(id,"annot") ;
		signalUndoableAction().emit(action.str(), notrack);
	}
	SetEndAnchor(id, a);
}

void UndoableDataModel::agSetAnchorOffset(const AnchorId& id, float f)
{
	if ( !getInhibUndoRedo() && !getInhibateSignals() ) {
		ostringstream action;
		bool b=GetAnchored(id);
		string time = float_to_string(GetAnchorOffset(id), 6) ;
		action << "set offset " << id << " anchored=" << b << ";offset=" << time << ";new=" << f;
		int notrack = getAViewTrack(id,"anchor") ;
		signalUndoableAction().emit(action.str(), notrack);
	}
	SetAnchorOffset(id, f);
}

void UndoableDataModel::agSetStartOffset(const AnnotationId& id, float f)
{
	AnchorId anchor = GetStartAnchor(id) ;
	agSetAnchorOffset(anchor,f) ;
}

void UndoableDataModel::agUnsetAnchorOffset(const AnchorId& id)
{
	if ( !getInhibUndoRedo() && !getInhibateSignals() ) {
		ostringstream action;
		bool b=GetAnchored(id);
		action << "unset offset " << id  << " anchored=" << b ;
		if ( b ) {
			string offset = float_to_string(GetAnchorOffset(id), 6) ;
			action << ";offset=" << offset << ";sigIds=" ;
			const set<SignalId> sigIds = GetAnchorSignalIds(id);
			copy(sigIds.begin(), sigIds.end(), ostream_iterator<SignalId>(action, ","));
		}
		int notrack = getAViewTrack(id,"anchor") ;
		signalUndoableAction().emit(action.str(), notrack);
	}
	UnsetAnchorOffset(id);
}

void UndoableDataModel::emitUndoRedoSignal(const string& type, const string& id, UpdateType upd)
{
	if (!getInhibateSignals()) {
//		m_inhib = true ;
		m_signalUndoRedoModification.emit(type, id, upd) ;
//		m_inhib = false ;
	}
}

int UndoableDataModel::getAViewTrack(string id, string mode)
{
	if (ExistsSignal(id))
	{
		return getSignalNotrack(id) ;
	}
	else if (ExistsAnnotation(id))
	{
		return getAViewTrack(GetStartAnchor(id), mode);
	}
	else if (ExistsAnchor(id))
	{
		if ( GetAnchored(id) ) {
			std::set<SignalId> sigs = GetAnchorSignalIds(id) ;
			if ( sigs.size()!=1 )
				return -2;
			else {
				std::set<SignalId>::iterator it = sigs.begin() ;
				return getSignalNotrack(*it) ;
			}
		} else return getAnchorTrackHint() ; // unanchored anchor -> return anchorTrackHint
	}
	//> agset feature: return code for not allowing undo but without displaying error
	//  message (not an error but a normal behavior)
	else if (ExistsAGSet(id))
	{
		return -3 ;
	}
	else {
		TRACE_D << "UndoableDataModel::getAViewTrack:> NO VIEW FOUND for id=" << id << std::endl ;
		return -2 ;
	}
}

void UndoableDataModel::tagInsertIntoAnchorLinks(const string& anchorId, const string& toBeInserted)
{
	if ( !getInhibUndoRedo() && !getInhibateSignals() )
	{
		ostringstream action;
		action << "set link " << anchorId << " " << toBeInserted ;
		int notrack = getAViewTrack(anchorId,"annot") ;
		signalUndoableAction().emit(action.str(), notrack) ;
	}
	anchorLinks().insertIntoLinks(anchorId, toBeInserted) ;
}

void UndoableDataModel::tagRemoveFromAnchorLinks(const string& anchorId, const string& toBeRemoved)
{
	if ( !getInhibUndoRedo() && !getInhibateSignals() )
	{
		ostringstream action;
		action << "unset link " << anchorId << " " << toBeRemoved ;
		int notrack = getAViewTrack(anchorId,"annot") ;
		signalUndoableAction().emit(action.str(), notrack) ;
	}
	anchorLinks().removeFromLinks(anchorId, toBeRemoved) ;
}


} //end namespace
