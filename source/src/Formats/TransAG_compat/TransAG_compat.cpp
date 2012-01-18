/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
// TransAG_compat.cc: TransAG upward compatibility loader class implementation
// based on AG loader class definition by Haejoong Lee, Xiaoyi Ma, Steven Bird
// ( Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
//   Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
//   For license information, see the file `LICENSE' included with aglib  distribution)


#include <errno.h>
#include <string.h>
#include <fstream>
#include <ag/AGException.h>
#include <ag/RE.h>
#include <ag/AGAPI.h>
#include "TransAG_compat.h"
#include "SAX_TransAGHandler.h"
#include "agfXercesUtils.h"
#include "DataModel/DataModel.h"
#include "Common/util/Log.h"

#ifdef EXTERNAL_LOAD
list<AGId>
TransAG_compat::plugin_load(const string& filename,
		const Id& id,
		map<string,string>* signalInfo,
		map<string,string>* options)
throw (agfio::LoadError)
{
	try {
		load(filename, id, signalInfo, options);
	}
	catch (const agfioError& e) {
		throw agfio::LoadError(string("TransAG_compat:") + e.what());
	}
}

string
TransAG_compat::plugin_store(const string& filename,
		const string& id,
		map<string,string>* options)
throw (agfio::StoreError)
{
	try {
		store(filename, id, options);
	}
	catch (const agfioError& e) {
		throw agfio::StoreError(string("TransAG_compat:") + e.what());
	}
}

#endif

static string  graphtype("transcription_graph");
static string agId("");

static void fix_segment(tag::DataModel& data, string id)
{
	int nb = 0;
	int order = data.getOrder(id);
	const string& sid = GetStartAnchor(id);
	string aid = sid;
	string eid;
	map<string, string> props;
	list<string> ev_candidate;
	list<string> other_qual;

	while (true)
	{
		++nb;
		eid = GetEndAnchor(id);
		props.clear();
		props = GetFeatures(id);

		map<string, string>::iterator it = props.find("text");
		bool is_text = ( it != props.end() && ! it->second.empty() ) ;
		bool check_can_merge = ( nb == 1 && ! GetAnchored(eid) && ! is_text );

		DeleteAnnotation(id);
		id = CreateAnnotation(id, aid, eid, "unit");


		const set<AnnotationId>& ids = GetOutgoingAnnotationSet(aid, "");

		if ( ids.size() > 0 ) {
			set<AnnotationId>::const_iterator ito;
			for (ito=ids.begin(); ito != ids.end(); ++ito ) {
				const string& otype = GetAnnotationType(*ito);
				if ( ! data.isMainstreamType(otype, graphtype) ) {
					const string& end = GetEndAnchor(*ito) ;
					if (otype == "noise" || otype == "pronounce") { // then maybe add a unit_event
						if ( end == aid ) {
							// "instantaneous" noise -> will become event
							ev_candidate.push_back(*ito);
						} else if ( ! is_text  && end == eid ) {
							// noise on empty segment -> will become event
							ev_candidate.push_back(*ito);
						} else other_qual.push_back(*ito);
					} else {
						if ( end == aid  && ! data.conventions().isInstantaneous(otype))
							SetEndAnchor(*ito, eid); // NO LOOPS
						other_qual.push_back(*ito);
					}
				}
			}
		}
		bool kept = true;
		bool do_event = (ev_candidate.size() > 0);
		bool has_qual = (other_qual.size() > 0);
		list<string>::iterator it_ev;

		if ( ! is_text && ! has_qual ) {
			// empty seg -> can be removed -> reattach eid to aid
			if ( ! GetAnchored(eid) ) {
				Log::out() << "delete empty seg " << id << endl;
				Log::out() << " .. Will delete end anchor " << eid << endl;
				DeleteAnnotation(id);
				const set<AnnotationId>& ids = GetOutgoingAnnotationSet(eid, "");
				set<AnnotationId>::const_iterator it;
				for (it=ids.begin(); it != ids.end(); ++it ) {
					SetStartAnchor(*it, aid);
					if ( GetEndAnchor(*it) == eid ) SetEndAnchor(*it,aid);
				}
				// eventually reattach events ends -> will create a loop but this will be fixed at next passage
				for (it_ev=ev_candidate.begin(); it_ev != ev_candidate.end(); ) {
					if ( GetEndAnchor(*it_ev) == eid ) {
						SetEndAnchor(*it_ev, aid);
						it_ev = ev_candidate.erase(it_ev);
					} else ++it_ev;
				}
				// finally check possible remaining incoming event at eid
				const set<AnnotationId>& evids = GetIncomingAnnotationSet(eid, "");
				for (it=evids.begin(); it != evids.end(); ++it ) {
					SetEndAnchor(*it, aid);
				}

				DeleteAnchor(eid);

				eid = aid;
				kept = false;
			} else if ( ! GetAnchored(aid) && ! do_event ) {
				Log::out() << "delete empty seg " << id << endl;
				Log::out() << " .. Will delete start anchor " << aid << endl;
				DeleteAnnotation(id);
				const set<AnnotationId>& ids = GetIncomingAnnotationSet(aid, "");
				set<AnnotationId>::const_iterator it;
				for (it=ids.begin(); it != ids.end(); ++it ) {
					SetEndAnchor(*it, eid);
				}
				DeleteAnchor(aid);
				kept = false;
			}
		}
		if ( kept ) {

			string value = "";

			it = props.find("text") ;
			if ( is_text ) {
				value = it->second;
			}
			if ( it != props.end() )
				props.erase(it) ;
			it = props.find("confidence_high") ;
			if ( it != props.end() )
				props.erase(it) ;
			it = props.find("value") ;
			if ( it != props.end() )
				props.erase(it) ;

			if ( is_text ) {
				if ( do_event ) {
					// Need to add an event at text start
					list<AnnotationId> nids = SplitAnnotation(id);
					// if qualifiers attached at text start -> split again to keep proper attachment point
					// TODO -> when qualifier attachment on a event-type unit will be solved , remove this split
					if ( other_qual.size() > 0 ) {
						SetFeature(nids.front(), "subtype", "unit_text") ;
						id = nids.back();
						nids = SplitAnnotation(id);
					}
					SetFeature(nids.front(), "subtype", "unit_event") ;
					SetFeature(nids.front(), "value", GetAnnotationType(ev_candidate.front()));
					SetFeature(nids.front(), "desc", data.getElementProperty(ev_candidate.front(), "desc"));
					DeleteAnnotation(ev_candidate.front());
					id = nids.back();
				}
				SetFeature(id, "subtype", "unit_text") ;
				SetFeature(id, "value", value);
				SetFeatures(id, props);
			} else {
				if ( do_event ) {
					SetFeature(id, "subtype", "unit_event") ;
					SetFeature(id, "desc", data.getElementProperty(ev_candidate.front(), "desc"));
					SetFeature(id, "value", GetAnnotationType(ev_candidate.front()));
					DeleteAnnotation(ev_candidate.front());
				} else
					SetFeature(id, "subtype", "unit_text") ;
			}
			SetFeatures(id, props);
			ev_candidate.clear();
			other_qual.clear();
		}
		if ( eid != aid && GetAnchored(eid) )
		{
			break;
		}
		const set<AnnotationId>& eids = GetOutgoingAnnotationSet(eid, "segment");
		if ( eids.size() == 0 ) break;
		if ( eids.size() > 1 )
		{
			cerr << "STRANGE : more than one at anchor " << eid << "!! " << endl;
		}
		id = *(eids.begin());
		aid = eid;
	}


	id = CreateAnnotation(agId, sid, eid, "segment");
	if ( order > 0 ) data.setElementProperty(id, "order", order);
}

static void fix_segments_at_anchor(tag::DataModel& data, string aid)
{
	const set<AnnotationId>& ids = GetOutgoingAnnotationSet(aid, "segment");
	set<AnnotationId>::const_iterator it;
	for (it = ids.begin(); it != ids.end(); ++it)
	{
		fix_segment(data, *it);
	}
}

static void tagConvert(tag::DataModel& data) {
	// locate pasted graph first node

	try {

		agId = data.getAG("transcription_graph");
		if ( data.hasElementsWithType("unit", agId) ) {
			// already converted
			return;
		}
		list<AnchorId> aids = GetAnchorSet(agId);
		list<AnchorId>::iterator ita;

		for (ita = aids.begin(); ita != aids.end(); )
		{
			if (! GetAnchored(*ita))
				ita = aids.erase(ita);
			else ++ita;
		}

		for (ita = aids.begin(); ita != aids.end(); ++ita) 	{
			fix_segments_at_anchor(data, *ita);
		}

	} catch ( AGException& e ) {
		cerr << "CAUGHT AGException " << e.error() << endl;
	}
	data.setUpdated(true);
}


list<AGId>
TransAG_compat::load(const string& filename,
		const Id& id,
		map<string,string>* signalInfo,
		map<string,string>* options)
throw (agfio::LoadError)
{
	try {
		xercesc_open();
	}
	catch (const agfioError& e) {
		throw agfio::LoadError(string("TransAG_compat:") + e.what());
	}

	SAX_TransAGHandler handler("UTF-8");

	bool val_opt = true;
	string encoding = "";
	string conventions = "";

	if (options != NULL) {
		if ((*options)["dtd"] != "")
			handler.set_localDTD((*options)["dtd"]);
		if ((*options)["encoding"] != "")
			encoding = (*options)["encoding"];
		if ((*options)["DTDvalidation"] == "false")
			val_opt = false;
		if ( (options->find("conventions")) != options->end() )
			conventions = (*options)["conventions"];
	}

	try {
		tagSAXParse(&handler, filename, val_opt, encoding);
	}
	catch (const XMLException& e) {
		throw agfio::LoadError(string("TransAG_compat:loading failed due to the following error\n") + trans(e.getMessage()));
	}
	catch (AGException& e) {
		throw agfio::LoadError("TransAG_compat:loading failed due to the following error\n" +
				e.error());
	}

	list<AGId> loaded_agids = handler.get_agids();
	if ( loaded_agids.size() > 0 ) {
		tag::DataModel data;
		data.setKeepAG(true); // do not delete graph upon datamodel deletion
		if ( ! conventions.empty() )
			data.setConventions(conventions, "fre", false);
		data.initFromLoadedAG("TransAG_compat", loaded_agids, false);
		tagConvert(data);
	}
	xercesc_close();

	return loaded_agids;
}


string
TransAG_compat::store(const string& filename,
		const string& id,
		map<string,string>* options)
throw (agfio::StoreError)
{
	ofstream out(filename.c_str());
	if (! out.good())
		throw agfio::StoreError("TransAG_compat::store():can't open "+filename+"for writing");
	string encoding, dtd;
	if (options) {
		encoding = (*options)["encoding"];
		dtd = (*options)["dtd"];
	}
	if (encoding.empty())
		out << "<?xml version=\"1.0\"?>" << endl;
	else
		out << "<?xml version=\"1.0\" encoding=\""
		<< encoding << "\"?>" << endl;
	if (dtd.empty())
		out << "<!DOCTYPE AGSet SYSTEM \"http://agtk.sf.net/doc/xml/ag-1.1.dtd\">" << endl;
	else
		out << "<!DOCTYPE AGSet SYSTEM \""
		<< dtd << "\">" << endl;
	out << toXML(id).substr(55);
	if ( !out.good() ) {
		throw agfio::StoreError(strerror(errno));
	}
	out.close();
	return "";
}


