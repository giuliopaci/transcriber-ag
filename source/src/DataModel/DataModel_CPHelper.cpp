/*
 * @class DataModel_CPHelper
 * @brief DataModel Copy/Paste operations helper
 *
 *  Created on: 31 mars 2009
 *      Author: lecuyer
 */

#include "DataModel/DataModel.h"
#include "DataModel_CPHelper.h"
#include "Common/util/StringOps.h"
#include "Common/util/Log.h"
#include "Formats/TransAG/TransAG_StringParser.h"

#include <ag/AGException.h>
#include <glibmm.h>
#include <stdlib.h>
#include <math.h>

using namespace std;

#define LOG_FINE 	Log::setTraceLevel(Log::FINE);
#define LOG_RESET	Log::resetTraceLevel();


namespace tag {

DataModel_CPHelper::~DataModel_CPHelper()
{
	try {
		vector<string>::iterator it;
		for ( it=m_agsetIds.begin(); it != m_agsetIds.end(); ++it ) {
			if ( ExistsAGSet(*it) ) DeleteAGSet(*it);
		}
	} catch(AGException& ex) 	{
		MSGOUT << "Caught AGException " << ex.error() << endl;
	}
}



/**
 * insert subgraph in data model at given graph position
 * @param whereId annotation id after which new graph is to be inserted
 * @param whereOffset if whereId is a text-type annotation, text offset in whereId where subgraph is to be inserted, or -1 to insert at text end.
 * @throw error message if insertion failed
 */

bool DataModel_CPHelper::insertSubgraph(string whereId, int whereOffset, float signalOffset, const string& copyId, bool doShiftOffsets, bool emitSignal, string& err) throw (const char*)
{
	string where = "(DataModel_CPHelper::insertSubgraph) ";
	Log::trace() << where << " in " << whereId << " at " << whereOffset << endl;

	LOG_FINE

	TRACE << "=============================================================" << endl
		<< toXML(copyId)
		<< "=============================================================" << endl;

	m_updates.clear();
	m_graphType = m_data->getGraphType(whereId);
	m_baseType = m_data->mainstreamBaseType(m_graphType);
	string graphId = m_data->getAG(m_graphType);

	if ( m_data->getElementType(whereId) != m_baseType )
		whereId = m_data->getAnchoredBaseTypeStartId(whereId);

	set<AnnotationType> copyTypes = GetAnnotationTypes(copyId);

	if ( copyTypes.find(m_baseType) == copyTypes.end() )
	{
		ostringstream os;
		os << _("Incompatible graphs : no") << " " << m_baseType << " "<< _("type annotation found in inserted graph.") << " " ;
		throw os.str().c_str();
	}

	float prev_offset = m_data->getStartOffset(whereId);
	float next_offset = m_data->getEndOffset(whereId);
	string where_start = GetStartAnchor(whereId);
	string where_stop = GetEndAnchor(whereId);
	string first_anchored = "";
	
	ostringstream os;
	os << _("Segment size must be >") << " " << m_data->conventions().minSegmentSize(m_graphType) << " " << _("secondes") << " " ;
	string minsize_warn = os.str();
	int max_overlap = m_data->conventions().maxOverlap("", m_graphType);

//	vector< pair<string, string> > updates;

	// to insert subgraph in current graph, we first completely copy it in current AGSet
	// then we connect its start and end nodes to current AG at given insert position
	// and we eventually merge inserted subgraph start and end anchors with existing anchors

	// TODO -> check + warn for "out of" convention annotations

	try
	{
		const list<AnchorId>& aids = GetAnchorSet(copyId);

		//-- Check graph is valid
		list<AnchorId>::const_iterator ita;
		if ( aids.size() == 0 )
			return false ;

		float offset_shift ;

		if (!doShiftOffsets)
			offset_shift = 0.0 ;
		else if (signalOffset==-1)
			offset_shift = prev_offset ;
		else
			offset_shift = signalOffset ;

		//-- 1) Check that inserted anchors do not overlap next offset
		bool force = false ;
		bool doskip = false ;
		for ( ita=aids.begin(); ita != aids.end(); ++ita )
		{
			// Don't proceed the first timespamped anchor if no using signal offset (we'll unanchor it)
			doskip = ( (ita == aids.begin()) && signalOffset==-1 ) ;
			// Force to proceed the no-timespamped first anchor if using signal offset (we'll anchor it)
			force = ( (ita == aids.begin()) && signalOffset!=-1 ) ;

			if ( GetAnchored(*ita) && !doskip || force )
			{
				float offset = GetAnchorOffset(*ita) + offset_shift;

				// -- Insertion time < prevOffset
				if ( offset < prev_offset )
				{
					err = _("Inserted graph offsets go beyond current segment start offset") ;
					Log::err() << err << std::endl ;
					return false ;
				}

				// -- Segment duration too small
				if ( (offset - prev_offset) > DataModel::EPSILON
						&& (offset - prev_offset) < m_data->conventions().minSegmentSize(m_graphType) )
				{
					err =  minsize_warn.c_str() ;
					return false ;
				}

				// -- Insertion time > nextOffset
				if ( offset > next_offset )
				{
					err = _("Inserted graph offsets go after current segment end offset") ;
					Log::err() << err << std::endl ;
					return false ;
				}

				// -- Segment duration too small
				if ( (next_offset - offset) < m_data->conventions().minSegmentSize(m_graphType) )
				{
					err = minsize_warn ;
					Log::err() << minsize_warn << std::endl ;
					return false ;
				}

				// --
				if ( (offset - prev_offset) < DataModel::EPSILON && ! GetAnchored(where_start) )
				{
					err = _("Can't insert segment after segment start") ;
					Log::err() << err << std::endl ;
					return false ;
				}

				// -- Keep first anchored id
				if ( first_anchored.empty() || GetAnchorOffset(first_anchored) > GetAnchorOffset(*ita) )
					first_anchored = *ita ;
			}
		}
		
		// -- 2) Create all annotations in current graph
		string s_where_order = m_data->getElementProperty(whereId, "order", "0");
		int where_order	= m_data->getOrder(whereId);

		// check overlapping case
		if ( where_order > 0 )
		{
			if ( !first_anchored.empty() ) //  forbidden if anchored element
			{
				err = string(_("Insertion of signal-anchored elements in overlapping segment not supported")) ;
				return false ;
			}
		}

		// locate pasted graph first node
		string aid = aids.front();
		while (true)
		{
			const set<AnnotationId>& ids = GetIncomingAnnotationSet(aid, m_baseType);
			if ( ids.size() > 0 )
				aid = GetStartAnchor(*(ids.begin()));
			else
				break;
		} ;

		// Ok, graph can be pasted
		set<SignalId> sigIds = GetSignals(GetTimelineId(copyId));
		set<AnnotationId>::const_iterator it;
		string copy_start = aid;
		map<string, string> anchor_copy;
		int curtrack = m_data->getElementSignalTrack(whereId);

		SpeakerDictionary speakersDict;
		if ( ExistsFeature(copyId, "speakers") )
		{
			const string& speakers = GetFeature(copyId, "speakers");
			speakersDict.fromXML(speakers);
		}

		// -- 1) Create all anchors in current graph
		TRACE_D << "> first step : create all anchors in current graph" << endl;
		for (ita = aids.begin(); ita != aids.end(); ++ita )
		{
			//-- Anchored anchors
			if ( GetAnchored(*ita) && GetAnchorOffset(*ita) > 0 )
			{
				// Special case for 1st anchor : don't use offset info if we don'nt want to use signal position
				if ( (*ita==copy_start) && (signalOffset==-1) )
					anchor_copy[*ita] = m_data->createNode( graphId ) ;
				else
					anchor_copy[*ita] = m_data->createAnchor( graphId, curtrack, GetAnchorOffset(*ita)+offset_shift ) ;
			}
			//-- Unanchored
			else
			{
				// Special case for 1st anchor : use signal position if needed
				if ( (*ita==copy_start) && (signalOffset!=-1) )
					anchor_copy[*ita] = m_data->createAnchor( graphId, curtrack, GetAnchorOffset(*ita)+offset_shift ) ;
				else
					anchor_copy[*ita] = m_data->createNode( graphId ) ;
			}
		}

		// -- 2) Eventually add speakers to speaker dictionary
		TRACE_D << "> second step : eventually add speakers to speaker dictionary" << endl;
		map<string, string>::const_iterator itp;
		if ( speakersDict.size() > 0 )
		{
			SpeakerDictionary::iterator its;
			for ( its = speakersDict.begin(); its != speakersDict.end(); ++its )
			{
				bool to_add =  true;
				try
				{
					const Speaker& spk = m_data->getSpeakerDictionary().getSpeaker(its->first);
					to_add = (spk.getFullName().compare(its->second.getFullName()) != 0);
				}
				catch (...)
				{
				}

				if ( to_add )
				{
					char name[30];
					Speaker spk = speakersDict.getSpeaker(its->first);
					const string& spkid = m_data->getSpeakerDictionary().getUniqueId(name);
					if ( spk.getFullName() == speakersDict.getDefaultName(spk.getId()) )
						spk.setLastName(name);
					spk.setId(spkid);
					m_data->getSpeakerDictionary().addSpeaker(spk);
					set<AnnotationId> sids = GetAnnotationSetByFeature(copyId, "speaker", its->first);
					for ( it=sids.begin(); it != sids.end(); ++it )
						SetFeature(*it, "speaker", spk.getId());
				}
			}
		}

		// -- 3) Create all annotations in current graph
		TRACE_D << "> third step : create all annotations in current graph" << endl;
		const set<AnnotationId>& annots =  GetAnnotationSet(copyId, "");
		for (it = annots.begin(); it != annots.end(); ++it )
		{
			//-- if we want to insert without using signal offset, just keep base type
			if ( (GetStartAnchor(*it)==copy_start) && (GetAnnotationType(*it)!=m_baseType) && (signalOffset==-1) )
				continue ;

			// insert in overlapping branch -> set branch order on inserted elements.
			if ( where_order != 0 )
				SetFeature(*it, "order", s_where_order) ;

			const map<string, string>& props = GetFeatures(*it);
			map<string, string>::const_iterator itprop = props.find("order");
			if ( itprop != props.end() )
			{
				// Then skip overlapping annotation !!
				if ( StringOps(itprop->second).toInt() > max_overlap )
					continue;
			}
			const string& new_id = m_data->createAnnotation(graphId, GetAnnotationType(*it), anchor_copy[GetStartAnchor(*it)], anchor_copy[GetEndAnchor(*it)], props );
			TRACE_D << " INSERTED  id=" << new_id << "  : " << GetAnnotationInfo(new_id) << endl;
		}

		// now connect inserted graph to current graph
		TRACE_D << endl<< " WHERE  id=" << whereId << "  : " << GetAnnotationInfo(whereId) << endl;

		bool current_is_empty = isEmptyAnnotation(whereId);
		string split_id = "";
		string parent_id = m_data->getParentElement(whereId);
		string where_next = m_data->getNextElementId(whereId);
		string where_stop_orig = where_stop;
		string new_anchor_start = anchor_copy[copy_start];
		string new_end = "";

		if ( ! current_is_empty )
		{
			const string& text = m_data->getElementProperty(whereId, "value");
			int utf8_len = utf8->strlen(text) ;
			// split required
			if ( whereOffset < utf8_len )
			{
				split_id= m_data->splitTextMainstreamElement(whereId, whereOffset, -1, false, emitSignal);
				TRACE << " SPLIT  id=" << split_id << "  : " << GetAnnotationInfo(split_id) << endl;
				where_next = split_id;
				where_stop = GetEndAnchor(whereId);
			}
		}

		TRACE_D << endl<< "// now connect inserted graph to current graph" << endl;
		const set<AnnotationId>& sids = GetIncomingAnnotationSet(where_stop, m_baseType );
		for ( it=sids.begin(); it != sids.end(); ++it )
		{
			m_data->agSetEndAnchor(*it, new_anchor_start);
			m_updates.push_back(LocalUpdate(m_baseType, *it, DataModel::RESIZED));
		}

		// store inserted graph display update messages
		new_end = new_anchor_start;
		vector<string> overlap_starts;

		while ( true  )
		{
			const set<AnnotationId>& eids = GetOutgoingAnnotationSet(new_end, m_baseType);
			if ( eids.size() == 0 ) break;
			for ( it=eids.begin(); it != eids.end(); ++it )
			{
				if ( m_data->getOrder(*it) == where_order )
				{
					m_updates.push_back(LocalUpdate(m_baseType, *it, DataModel::INSERTED));
					new_end = GetEndAnchor(*it);
				}
				else
					overlap_starts.push_back(*it);
			}
		}

		int max_order = 0;
		// gestion parole superposée -> parcourir la sous branche !!
		if ( overlap_starts.size() > 0 ) {
			vector<string>::iterator ito;
			for (ito = overlap_starts.begin(); ito != overlap_starts.end(); ++ito) {
				int order = m_data->getOrder(*ito);
				if ( order > max_order ) max_order = order;
				m_updates.push_back(LocalUpdate(m_baseType, *ito, DataModel::INSERTED));

				string end_anchor=GetEndAnchor(*ito);
				while ( end_anchor != "" ) {
					const set<AnnotationId>& iids = GetIncomingAnnotationSet(end_anchor, m_baseType);
					if ( iids.size() > 1 ) break;
					const set<AnnotationId>& eids = GetOutgoingAnnotationSet(end_anchor, m_baseType);
					if ( eids.size() == 0 ) break;
					end_anchor = "";
					for ( it=eids.begin(); it != eids.end(); ++it ) {
						if ( m_data->getOrder(*it) == order ) {
							m_updates.push_back(LocalUpdate(m_baseType, *it, DataModel::INSERTED));
							end_anchor = GetEndAnchor(*it);
						}
					}
				}
			}
		}

		// Fix higher level annotations anchoring
		TRACE_D << endl << "// Fix higher level annotations anchoring" << endl;
		while (!parent_id.empty())
		{
			const set<AnnotationType>& insertedTypes = GetAnnotationTypes(copyId);
			const string& ptype = GetAnnotationType(parent_id);
			string next_parent = m_data->getNextElementId(parent_id);
			if (insertedTypes.find(ptype) != insertedTypes.end())
			{
				string pstart = firstWithType(new_anchor_start, ptype, 0);
				string pend = lastLinkedTo(pstart);

				if (!pstart.empty() && !pend.empty())
				{
					string unused("");
					if (!next_parent.empty())
					{
						if (GetStartAnchor(next_parent) != where_stop_orig)
						{
							// need to split parent
							const list<AnnotationId>& spid = m_data->agSplitAnnotation(parent_id, "", unused);
							// newly created begin at pend endanchor
							m_data->agSetStartAnchor(spid.back(), GetEndAnchor(pend));
							/*
							 * WARNING:
							 * At this point, since the last node of temporary graph is not time-anchored,
							 * it can happen we've created an annotation on a non-time-anchored node.
							 * For mainstream types such as segments or section, it's not correct
							 * (indeed, we'll merge later or we'll change this node by another one).
							 * Therefore, for these types, beeing no-time-anchored is not possible, so we don't tell the view to render it.
							 */
							if ( GetAnchored(GetEndAnchor(pend)) || !m_data->isMainstreamType(ptype, m_graphType) || ptype==m_baseType )
								m_updates.push_back(LocalUpdate(ptype, spid.back(), DataModel::INSERTED));
						}
					}
					else
						m_data->agSetEndAnchor(pend, GetEndAnchor(parent_id)) ;

					m_data->agSetEndAnchor(parent_id, GetStartAnchor(pstart));
					m_updates.push_back(LocalUpdate(ptype, parent_id, DataModel::RESIZED));
					if (!unused.empty())
					{
						m_data->agDeleteAnchor(unused);
						unused.clear() ;
					}
				} // pstart && pend ok :)
			}
			else if (GetEndAnchor(parent_id) == where_stop)
			{
				m_data->agSetEndAnchor(parent_id, new_end);
				m_updates.push_back(LocalUpdate(ptype, parent_id, DataModel::RESIZED));
			}
			parent_id = m_data->getParentElement(parent_id);
		}

		// add display requests for qualifiers
		aid = new_anchor_start;
		while ( !aid.empty() ) {
			const set<AnnotationId>& eids = GetOutgoingAnnotationSet(aid, "");
			aid="";
			for ( it=eids.begin(); it != eids.end(); ++it ) {
				const string& atype = GetAnnotationType(*it) ;
				if ( atype != m_baseType )
					m_updates.push_back(LocalUpdate(atype, *it, DataModel::INSERTED));
				else
				// TODO -> gérer parole superposée -> parcourir la sous branche !!
					aid = GetEndAnchor(*it);
			}
		}


		// Fix potentential canSpanOverType constraints
		const set<AnnotationId>& sids2 = GetIncomingAnnotationSet(where_stop, "" );
		for ( it=sids2.begin(); it != sids2.end(); ++it ) {
			const string& atype = GetAnnotationType(*it);
			if ( !m_data->conventions().isMainstreamType(atype, m_graphType) ) {
				if ( ! first_anchored.empty() && ! m_data->conventions().canSpanOverType(atype, m_baseType) )
					m_data->agSetEndAnchor(*it, first_anchored);
				else
					m_data->agSetEndAnchor(*it, new_end);
//				m_updates.push_back(LocalUpdate(atype, *it, DataModel::DELETED));
//				m_updates.push_back(LocalUpdate(atype,*it, DataModel::INSERTED));
				m_updates.push_back(LocalUpdate(atype, *it, DataModel::UPDATED));
			}
		}


//		if ( where_next.empty() ) {
		
		// inserting at graph end  : set inserted graph end anchor to where_stop
		const set<AnnotationId>& sid3s = GetIncomingAnnotationSet(new_end, "");
		for ( it=sid3s.begin(); it != sid3s.end(); ++it )
		{
			if ( GetStartAnchor(*it) == GetEndAnchor(*it) )
				m_data->agSetStartAnchor(*it, where_stop);
			m_data->agSetEndAnchor(*it, where_stop);
		}

		// inserting at graph end  : set updated graph end anchor to where_stop
		const set<AnnotationId>& sid4s = GetOutgoingAnnotationSet(new_end, "");
		for ( it=sid4s.begin(); it != sid4s.end(); ++it )
			m_data->agSetStartAnchor(*it, where_stop);

		m_data->deleteUnusedAnchor(new_end);
		new_end = where_stop;

//		} else {
//			const set<AnnotationId>& sids = GetOutgoingAnnotationSet(where_stop, "");
//			for ( it=sids.begin(); it != sids.end(); ++it ) {
//				m_data->agSetStartAnchor(*it, new_end);
//			}
//		}

		if ( emitSignal )
		{
			vector<LocalUpdate>::iterator itu;
			for ( itu=m_updates.begin(); itu != m_updates.end(); ++itu )
				m_data->emitSignal(itu->type, itu->id, itu->upd);
		}
		m_updates.clear();

//		TRACE_D  << endl << "===================== BEFORE MERGE ANCHOR   ================" << endl
//			<< toXML(graphId)
//			<< "=============================================================" << endl;

		// final stage : get rid of some unwanted "empty" annotations -> at paste start && paste end
		eventuallyMergeAnchors(where_start, new_anchor_start);

		if ( ! split_id.empty() ) {
			const set<AnnotationId>& prevs = GetIncomingAnnotationSet(new_end, m_baseType);
			new_end = GetStartAnchor(*(prevs.begin()));
			eventuallyMergeAnchors(new_end, GetStartAnchor(split_id));
		}

		// can get rid of unused anchor
		m_data->deleteUnusedAnchor(where_stop);


//		TRACE  << endl << "===================== AFTER MERGE ANCHOR   ================" << endl
//			<< toXML(graphId)
//			<< "=============================================================" << endl;
		if ( emitSignal )
		{
			vector<LocalUpdate>::iterator itu;
			for ( itu=m_updates.begin(); itu != m_updates.end(); ++itu )
					m_data->emitSignal(itu->type, itu->id, itu->upd);
		}
		m_updates.clear();

		return true ;
	}
	catch (AGException& e)
	{
		Log::err() << "insert subgraph exception: " << e.error() << std::endl ;
		string err = e.error() ;
		throw err.c_str() ;
		return false ;
	}
	LOG_RESET

	Log::trace()  << where << " Done." << endl;
}


/**
 * check if 2 nodes of a continuous graph can be merged
 * @param baseId base node id
 * @param mergeId tested node id
 * @return true if mergeId can be merged on baseId, else false
 */
bool DataModel_CPHelper::getCanMerge(const string& baseId, const string& mergeId)
{
	bool canMerge = false;

	if (!ExistsAnchor(baseId) || !ExistsAnchor(mergeId))
	{
		Log::err() << "DM_CPH::getCanMerge: invalid anchor" << std::endl ;
		return false ;
	}

	if ( ! GetAnchored(mergeId) ) {
		const set<AnnotationId>& sid1 = GetIncomingAnnotationSet(mergeId, "");
		set<AnnotationId>::const_iterator it1;
		for ( it1=sid1.begin(); it1 != sid1.end(); ++it1 ) {
			// if has incoming qualifiers -> non mergeable node
			const string& atype = GetAnnotationType(*it1);
			if ( !m_data->isMainstreamType(atype, m_graphType) )
				return false;
			else if ( atype == m_data->mainstreamBaseType(m_graphType) ) {
				if ( m_data->getElementProperty(*it1, "subtype") == "unit_event" ) return false;
			}
		}
		const set<AnnotationId>& sid2 = GetOutgoingAnnotationSet(mergeId, "");
		if ( sid2.size() <= 1 ) {
			canMerge = true;
			if ( sid2.size() > 0 ) {
				const string& id=*(sid2.begin());
				const string& atype = GetAnnotationType(id);
				if ( atype == m_data->mainstreamBaseType(m_graphType) ) {
					if ( m_data->getElementProperty(id, "subtype") == "unit_event" )
						canMerge=false;
				}
			}
		}
		else {
			// can merge if "empty" base annotation precedes
			canMerge = ( isEmptyAnnotation(*(sid1.begin()))) ;
		}
	}  else {
		if ( !GetAnchored(baseId) ) canMerge=false;
		else {
			if ( fabs(GetAnchorOffset(baseId) - GetAnchorOffset(mergeId)) < DataModel::EPSILON ) {
				// same offset -> mergeable if previous base segment is "empty"
				const set<AnnotationId>& sid1 = GetIncomingAnnotationSet(mergeId, m_baseType);
				canMerge = ( isEmptyAnnotation(*(sid1.begin())));
			}
		}
	}

	return canMerge;
}


/**
 * test if annotation is "empty"
 * @param id annotation id
 * return true if "empty" annotation, else false
 */
bool DataModel_CPHelper::isEmptyAnnotation(const string& id)
{
	const string& text = m_data->getElementProperty(id, "value");
	return text.empty();
}

/**
 * merge 2 anchors, reattaching appropriately annotations
 * @param base anchor to be kept
 * @param to merge anchor to "merge" with base
 * @return deleted anchor id / "" if no merge performed
 */

void DataModel_CPHelper::eventuallyMergeAnchors(const string& base, const string& toMerge)
{
	if ( ! getCanMerge(base, toMerge) ) return ;

	TRACE_D << "CAN MERGE " << base << " and " << toMerge << endl;
	const set<AnnotationId>& thesid = GetOutgoingAnnotationSet(toMerge, "");
	set<AnnotationId>::const_iterator it0, it2;
	string next_end="";
	vector<AnnotationId> sid1;
	vector<AnnotationId>::iterator it1;

	// force ordering of annotations upon precedence order
	for ( it0=thesid.begin(); it0 != thesid.end(); ++it0 ) {
		const string& ptype = GetAnnotationType(*it0);
		if ( m_data->conventions().isMainstreamType(ptype, m_graphType) ) {
			bool done= false;
			for (it1 = sid1.begin(); !done && it1 != sid1.end(); ++it1 ) {
				const string& atype = GetAnnotationType(*it1);
				if ( m_data->conventions().isHigherPrecedence(atype, ptype) ) {
					it1 = sid1.insert(it1, *it0);
					done=true;
				}
			}
			if ( ! done ) sid1.push_back(*it0);
		}
	}
	for ( it0=thesid.begin(); it0 != thesid.end(); ++it0 ) {
		const string& ptype = GetAnnotationType(*it0);
		if ( ! m_data->conventions().isMainstreamType(ptype, m_graphType) )
			sid1.push_back(*it0);
	}


	for (it1 = sid1.begin(); it1 != sid1.end(); ++it1 ) {
		const string& atype = GetAnnotationType(*it1);
		if ( atype == m_baseType ) next_end = GetEndAnchor(*it1);
		const set<AnnotationId>& sid2 = GetIncomingAnnotationSet(toMerge, atype);
		bool do_reattach = true;

		if ( sid2.size() > 0 ) {
			const string& baseId = *(sid2.begin());
			if ( GetStartAnchor(baseId) == base ) {
				const string&  todel = m_data->mergeAnnotations(baseId, *it1, (atype == m_baseType), false);
				TRACE_D << " Deleted = " << todel << endl;
				m_updates.push_back(LocalUpdate(atype, todel, DataModel::DELETED));
				do_reattach = false;
			} else
				m_data->agSetEndAnchor(baseId, base);
			m_updates.push_back(LocalUpdate(atype, baseId, DataModel::UPDATED));
		}
		if ( do_reattach ) {
			// reattach start
			m_data->agSetStartAnchor(*it1, base);
			m_updates.push_back(LocalUpdate(atype, *it1, DataModel::DELETED));
			m_updates.push_back(LocalUpdate(atype, *it1, DataModel::INSERTED));
		}
	}

	if ( ExistsAnchor(toMerge) ) {
		// is there any remaining annotation ??
		const set<AnnotationId>& sid2 = GetIncomingAnnotationSet(toMerge, "");
		if ( sid2.size() > 0 ) {
			for (it2 = sid2.begin(); it2 != sid2.end(); ++it2 ) {
				if ( next_end.empty() )
					TRACE_D << " PROBLEME : que faire de " << *it2 << " ???? " << endl;
				else {
					TRACE_D << " > agSetEndAnchor " << *it2 << " to " << next_end << endl;
					m_data->agSetEndAnchor(*it2, next_end);
					m_updates.push_back(LocalUpdate(GetAnnotationType(*it2), *it2, DataModel::UPDATED));
				}
			}
		}
		m_data->deleteUnusedAnchor(toMerge);
	}
}

/**
 * extract subgraph starting and ending at given ids (+ eventual text_offsets)
 * @param startId	start annotation id
 * @param startOffset start text offset
 * @param stopId	end annotation id
 * @param stopOffset end text offset
 * @return subgraph id
 *
 * @note : if more than one track -> copy only annotations related to track associated to startId
 */

// TODO -> also export required speakerDict entries

string DataModel_CPHelper::getSubgraph(string startId, int startOffset, string stopId, int stopOffset, bool basetype_only, bool in_convention_only)
{
	LOG_FINE
	string where = "(DataModel_CPHelper::getSubgraph) ";

	// -- Check IDs
	if ( ! ExistsAnnotation(startId) || ! ExistsAnnotation(stopId) ) {
		Log::trace() << where << "One of the following ids is missing: " << startId << " , " << stopId << endl;
		return "";
	}

	// -- Check graphtype
	m_graphType = m_data->getGraphType(startId);
	m_baseType = m_data->mainstreamBaseType(m_graphType);
	if ( m_graphType != m_data->getGraphType(startId) ) {
		Log::trace() << where << "Graph type mismatch: " << stopId << "does not belong to " << m_graphType << " graph." << endl;
		return "";
	}

	Log::trace()  << where << " from " << startId << " textoffset=" << startOffset << " to " << stopId << " textoffset=" <<stopOffset << endl;

	// -- Initialize copy filter dara
	CopyFilter filter;
	filter.allowAnyQualifier = ( !basetype_only && !in_convention_only);
	filter.copySpeakers = true;
	filter.startOffset = startOffset ;
	filter.stopOffset = stopOffset ;
	filter.startId = startId ;
	filter.stopId = stopId ;

	AnchorId start_anchor = GetStartAnchor(startId);

	// -- Create new temporary AGSet
	string agsetId = CreateAGSet(DataModel::mktmpAGSetId());
	m_agsetIds.push_back(agsetId);

	// -- Create new temporary Timeline
	string timelineId = CreateTimeline(agsetId);
	string trackId = StringOps().fromInt(m_data->getElementSignalTrack(startId)+1);

	// -- Create new temporary Signal
	string signalId = CreateSignal(timelineId, "", "", "", "", TRANS_ANCHOR_UNIT, trackId);
	set<SignalId> sigIds;
	sigIds.insert(signalId);

	// -- Create new temporary Graph
	string newgraphId = CreateAG(agsetId, timelineId);

	//  -- Annotation types to be copied :
	//		base segment type + qualifiers types for current graph type
	//		else all mainstream + qualifiers types for current graph type
	if ( basetype_only )
		filter.copyTypes.insert(m_baseType);
	else
	{
		const vector<string>& v = m_data->getMainstreamTypes(m_graphType);
		vector<string>::const_iterator itv;
		for (itv = v.begin(); itv != v.end(); ++itv ) filter.copyTypes.insert(*itv);
	}

	// -- Qualifiers to be copied (conventions ones or all)
	if ( in_convention_only ) {
		const vector<string>& v = m_data->getQualifierTypes(m_graphType);
		vector<string>::const_iterator itv;
		for (itv = v.begin(); itv != v.end(); ++itv ) filter.copyTypes.insert(*itv);
	} else
		filter.allowAnyQualifier = true;

	// -- Copy will occur up to stopId start if stopOffset <= 0, else partially including stopId up to stopOffset
	string end_anchor =	( stopOffset != 0 ? GetEndAnchor(stopId) : GetStartAnchor(stopId) );

	// -- Specify whether we need to use the first anchor offset : if just a part of the text is selected, don't.
	bool unanchoredStart = ( startOffset > 0 ? true : false ) ;
	Log::out() << "Warning : copy graph unanchored start mode : " << unanchoredStart << "(offset=" << startOffset << ")" << std::endl ;

	copyGraph(newgraphId, start_anchor, end_anchor, filter, unanchoredStart);

	LOG_RESET

	Log::trace() << where << " Done  graphId=" << newgraphId << endl;

//	TRACE << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl
//		<< toXML(newgraphId)
//		<< "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n" << endl;

	return newgraphId;
}

void DataModel_CPHelper::copyGraph(const string& destGraphId, const string& start_anchor, const string& end_anchor, CopyFilter& filter, bool unanchoredStart)
{
	string where = "(DataModel_CPHelper::copyGraph) ";

	TimelineId tid = GetTimelineId(destGraphId);
	set<SignalId> sigIds = GetSignals(tid);

	map<AnchorId, AnchorId> anchor_copy;

	string astart;
	SpeakerDictionary speakersDict;

	string baseType = m_data->mainstreamBaseType(m_graphType);
	while (filter.copyTypes.find(baseType) == filter.copyTypes.end())
		baseType = m_data->conventions().getParentType(baseType, m_graphType);

	// -- If the offset of the first anchor should not be used, then only keep basetype
	string max_type_at_start("");
	if ( unanchoredStart )
		max_type_at_start = m_baseType ;

	int order_at_start = -1;
	string startId = filter.startId;
	string stopId = filter.stopId;
	int startOffset = filter.startOffset;
	int stopOffset = filter.stopOffset;
	float offset_base = GetAnchorOffset(start_anchor);

	order_at_start = m_data->getOrder(startId);

	if (m_data->getElementType(startId) != m_baseType) // adjust startId to mainstreamBaseType
		startId = m_data->getAnchoredBaseTypeStartId(startId);
	if (m_data->getElementType(stopId) != m_baseType)
		stopId = m_data->getAnchoredBaseTypeStartId(stopId);

	bool with_overlap = false;
	int min_order = 99;
	int max_order = 0;

	anchor_copy[start_anchor] = createAnchorCopy(destGraphId, start_anchor, sigIds, offset_base, !unanchoredStart, false);

	// copy all annotations between copied anchors with type listed in copyTypes
	for (astart = start_anchor; !astart.empty() && astart != end_anchor;)
	{
		const set<AnnotationId>& ids = GetOutgoingAnnotationSet(astart, "");
		set<AnnotationId>::const_iterator it;
		string anext = "";

		for (it = ids.begin(); it != ids.end(); ++it)
		{
			const string& atype = GetAnnotationType(*it);
			const string& aend = GetEndAnchor(*it);
			bool is_mainstream = m_data->isMainstreamType(atype, m_graphType);
			bool exists_end = (anchor_copy.find(aend) != anchor_copy.end());

			if ((!is_mainstream && filter.allowAnyQualifier) || filter.copyTypes.find(atype) != filter.copyTypes.end())
			{
				// on first anchor, check if we should keep only basetype segments
				bool keep_it = true;
				if (astart == start_anchor)
				{
					bool notHigher = !m_data->conventions().isHigherPrecedence(atype, max_type_at_start) ;
					keep_it = ( max_type_at_start.empty() || notHigher );
				}

				// -- check order to determine if we should keep it
				if (keep_it)
					keep_it = (order_at_start == -1 || m_data->getOrder(*it) == order_at_start);

				// -- check end anchor to determine if we should keep it
				if (keep_it)
					keep_it = ( !GetAnchored(aend) || (GetAnchored(end_anchor) && (GetAnchorOffset(aend)<=GetAnchorOffset(end_anchor))) ) ;

				if (keep_it)
				{
					if (!exists_end)
						anchor_copy[aend] = createAnchorCopy(destGraphId, aend, sigIds, offset_base, (aend != end_anchor /*|| stopOffset <= 0*/));

//					TRACE_D << " to copy " << atype << " id=" << *it << endl;
//					string annotId = CreateAnnotation(destGraphId, anchor_copy[astart], (exists_end ? anchor_copy[aend] : anchor_copy[end_anchor]), atype );
					string annotId = CreateAnnotation(destGraphId, anchor_copy[astart], anchor_copy[aend], atype);
					map<string, string> props = GetFeatures(*it);
					SetFeatures(annotId, props);
					if (startOffset > 0 && *it == startId && ExistsFeature(annotId, "value"))
					{
						// adjust text content to selected part
						Glib::ustring text = GetFeature(annotId, "value");
						if (startOffset >= text.size())
							text = "";
						else
						{
							if (*it == stopId && stopOffset > 0)
								text = text.substr(startOffset, stopOffset - startOffset);
							else
								text = text.substr(startOffset);
						}
						if (text.empty())
							DeleteFeature(annotId, "value");
						else
							SetFeature(annotId, "value", text);
					}
					else
					{
						if (*it == stopId && ExistsFeature(annotId, "value"))
						{
							if (stopOffset > 0)
							{
								if (*it == startId && startOffset > 0)
									stopOffset -= startOffset;
								// adjust text content to selected part
								Glib::ustring text = GetFeature(annotId, "value");
								if (stopOffset <= 0)
									text = "";
								else
									text = text.substr(0, stopOffset);
								if (text.empty())
									DeleteFeature(annotId, "value");
								else
									SetFeature(annotId, "value", text);
							}
							else
								DeleteFeature(annotId, "value");
						}
					}
					if (props.find("speaker") != props.end())
					{
						const string& spkid = props["speaker"];
						if (!spkid.empty())
						{
							if (filter.copySpeakers && !speakersDict.existsSpeaker(spkid))
							{
								try
								{
									const Speaker& spk = m_data->getSpeakerDictionary().getSpeaker(spkid);
									speakersDict.addSpeaker(spk);
								}
								catch (...)
								{
									Log::trace(Log::MEDIUM) << where << "speaker " << spkid << " not found !!" << endl;
									Speaker spk = speakersDict.defaultSpeaker("");
									speakersDict.addSpeaker(spk);
									SetFeature(annotId, "speaker", spk.getId());
								}
							}
						}
						else
							DeleteFeature(annotId, "speaker");
					}
					if (atype == baseType)
					{
						int order = m_data->getOrder(*it);
						if (order < min_order)
							min_order = order;
						if (order > max_order)
							max_order = order;
					}
				}
				else
					Log::trace(Log::MEDIUM) << "copyGraph ~> don't want " << *it << std::endl ;
			}

			if (atype == baseType && (m_data->getOrder(*it) == 0 || anext.empty()))
				anext = aend;
		}
		if (anext.empty())
			Log::trace() << where << "NO " << baseType << " FOUND !!" << endl;

		astart = anext;
	}

	// if overlapping branch only copied -> reset order, and remove extra overlap.
	if (min_order > 0)
	{
		const set<AnnotationId> ids = GetAnnotationSet(destGraphId, "");
		set<AnnotationId>::const_iterator iti;
		for (iti = ids.begin(); iti != ids.end(); ++iti)
		{
			if (m_data->getOrder(*iti) > min_order)
				DeleteAnnotation(*iti);
			else
			{
				try
				{
					DeleteFeature(*iti, "order");
				}
				catch (...)
				{
				}
			}
		}
	}

	// store speaker dictionary in copied graph
	if (filter.copySpeakers && speakersDict.size() > 0)
	{
		ostringstream os;
		speakersDict.toXML(os);
		SetFeature(destGraphId, "speakers", os.str());
	}
}

/**
 * extract subgraph starting and ending at given ids as XML buffer following TransAG DTD
 * @param startId	start annotation id
 * @param startOffset start text offset
 * @param stopId	end annotation id
 * @param stopOffset end text offset
 * @return XML string buffer
 */

const string& DataModel_CPHelper::getSubgraphTAGBuffer(string startId, int startOffset, string stopId, int stopOffset, bool basetype_only, bool in_convention_only)
{
	Log::trace() << "(DataModel_CPHelper::getSubgraphTAGBuffer) "
			<< " startId=" << startId <<" startOffset=" << startOffset
			<< " stopId=" << stopId <<" stopOffset=" << stopOffset << endl;
	string graphId = getSubgraph(startId, startOffset, stopId, stopOffset, basetype_only, in_convention_only);
	m_textBuffer = "";
	if ( ! graphId.empty() ) {
		m_textBuffer = toXML(graphId);
		DeleteAGSet(GetAGSetId(graphId));
	}

//	TRACE << " ====================================  XML >" << endl;
//	TRACE << m_textBuffer << endl;
//	TRACE << " ====================================  XML >" << endl;

	return m_textBuffer;
}

/**
 * extract text annotations from XML string buffer following TransAG DTD
 * @param buffer XML string buffer
 * @return subgraph id
 */
const string& DataModel_CPHelper::getTextFromTAGBuffer(const string& buffer, const string& atype, const string& aprop)
{
	try {
		TransAG_StringParser parser(buffer);
		m_agsetIds.push_back(GetAGSetId(parser.getGraphId()));
		getTextFromGraph(parser.getGraphId(), atype, aprop);
	} catch (agfio::LoadError& e) {
		Log::err() << "(DataModel_CPHelper::getGraphFromTAGBuffer) " << e.what() << endl;
	}
	return m_textBuffer ;
}


/**
 * build subgraph from XML string buffer following TransAG DTD
 * @param buffer XML string buffer
 * @return subgraph id
 */
string DataModel_CPHelper::getGraphFromTAGBuffer(const string& buffer)
{
	try {
		TransAG_StringParser parser(buffer);
		string id = parser.getGraphId();
		m_agsetIds.push_back(GetAGSetId(id));
		return id;
	} catch (agfio::LoadError& e) {
		Log::err() << "(DataModel_CPHelper::getGraphFromTAGBuffer) " << e.what() << endl;
	}
	return "";
}

/**
 * extract text annotations graph
 * @param graphId graph id
 * @param atype annotation type to follow
 * @param aprop property to extract
 * @return plain text string holding all annotations value
 * @note annotation should form a continuous graph
 */
const string& DataModel_CPHelper::getTextFromGraph(const string& graphId, const string& atype, const string& aprop)
{
	m_textBuffer = "";

	try {
		const list<AnchorId>& aids = GetAnchorSet(graphId);
		// find the first one
		list<AnchorId>::const_iterator ita;
		if ( aids.size() == 0 ) return m_textBuffer;  // empty graph
		string aid = aids.front();

		// eventually go back to graph start
		while (true) {
			const set<AnnotationId>& ids = GetIncomingAnnotationSet(aid, atype);
			if ( ids.size() > 0 ) aid = GetStartAnchor(*(ids.begin()));
			else break;
		} ;

		// now browse through graph to get annotations text property
		while (true) {
			set<AnnotationId>::const_iterator it;
			const set<AnnotationId>& ids = GetOutgoingAnnotationSet(aid, atype);
			if ( ids.size() == 0 ) break;
			for ( it=ids.begin(); it != ids.end(); ++it ) {
				const string& txt = GetFeature(*it, aprop);
				if ( ! txt.empty() ) {
					if ( !m_textBuffer.empty() ) m_textBuffer += " ";
					m_textBuffer += txt;
				}
			}
			aid = GetEndAnchor(*(ids.begin()));
		}
	} catch (AGException& e) {
	    throw agfio::LoadError("TransAG_StringParser::getAnnotationText failed due to the following error\n" +
				   e.error());
	}
	return m_textBuffer;
}


///////////////////////////////////
/// 		  INTERNALS  		///
///////////////////////////////////

/* copy anchor in new graph */
string DataModel_CPHelper::createAnchorCopy(const string& graphId, const string& aid, set<SignalId>& sigIds, float offsetBase, bool use_offset, bool addOffset)
{
	if (GetAnchored(aid) && use_offset )
	{
		float off = GetAnchorOffset(aid) - offsetBase ;
		if (addOffset)
			off = off + m_data->conventions().minSegmentSize(m_graphType) ;
		return CreateAnchor(graphId, off, TRANS_ANCHOR_UNIT, sigIds);
	}
	else
		return CreateAnchor(graphId);
}


/*  get 1st element with given type attached at given anchor */
string DataModel_CPHelper::getElementAtAnchor(const string& aid, const string& type, bool outgoing)
{
	if ( outgoing) {
		const set<AnnotationId>& ids = GetOutgoingAnnotationSet(aid, type);
		if ( ids.size() > 0 )
			return *(ids.begin());
	} else {
		const set<AnnotationId>& ids = GetIncomingAnnotationSet(aid, type);
		if ( ids.size() > 0 )
			return *(ids.begin());
	}
	return "";
}

/*  get 1st element with given type in graph starting at given anchor */
string DataModel_CPHelper::firstWithType(const string& graph_start, const string& type, int order)
{
	string aid = graph_start;
	while ( !aid.empty() ) {
		const set<AnnotationId>& sids = GetOutgoingAnnotationSet(aid, "");
		set<AnnotationId>::const_iterator it;
		aid = "";
		for ( it=sids.begin(); it != sids.end(); ++it ) {
			const string& atype = GetAnnotationType(*it);
			if ( atype == type && m_data->getOrder(*it) == order) return *it;
			else if ( atype == m_baseType && m_data->getOrder(*it) == 0 )
				aid = GetEndAnchor(*it);
		}
	}
	return "";
}

/*  get last element with given type in graph starting at given annotation id */
string DataModel_CPHelper::lastLinkedTo(const string& id)
{
	if (id.empty())
		return "" ;

	string aid = GetEndAnchor(id);
	string last = id;
	const string& atype = GetAnnotationType(id);
	while ( true ) {
		const set<AnnotationId>& sids = GetOutgoingAnnotationSet(aid, atype);
		if ( sids.size() > 0 ) {
			set<AnnotationId>::const_iterator it = sids.begin();
			last = (*it);
			aid = GetEndAnchor(last);
		} else
			break;
	}
	return last;

}


///* get graph end anchor for continuous annotation graph including element identified by id */
//string DataModel_CPHelper::getGraphEndAnchorById(const string& id)
//{
//	const string& eid = GetEndAnchor(id);
//	const set<AnnotationId>& ids = GetOutgoingAnnotationSet(eid, GetAnnotationType(id));
//	if ( ids.size() == 0 ) return eid;
//	return getGraphEndAnchorById(*(ids.begin()));
//}

}
