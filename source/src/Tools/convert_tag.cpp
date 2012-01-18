/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/*                                                                              */
/*   conversion au format TransAG1.5 : introduction d'arcs "unit" (pouvant être ancrés) */
/*      en sous-elements des arcs "segment", qui eux représentent maintenant un */
/*      speech segment complet                                                  */
/* 	         																	*/
/********************************************************************************/

#include <iostream>
#include <iterator>
#include <algorithm>
#include <set>
#include <getopt.h>
#include <vector>
#include <list>
#include <ag/AGAPI.h>
#include <glibmm.h>
#include <glib.h>

#include "DataModel/DataModel.h"
#include "Common/VersionInfo.h"
#include "Common/FileInfo.h"

using namespace tag;

string baseType;
string agId;

void USAGE(const char* progname)
{
	Log::err() << "USAGE: " << progname << " [-v] [-f <fmt>] <filename>" << endl;
	Log::err() << "\toptions :" << endl;
	Log::err() << "\t\t-v : print program version " << endl;
	Log::err() << "\t\t-f <fmt>: define file format" << endl;
	Log::err() << "\t\t-c <conv>: applicable conventions" << endl;
	exit(1);
}

void fix_segment(DataModel& data, string id)
{
	int nb = 0;
	int order = data.getOrder(id);
	const string& sid = GetStartAnchor(id);
	string aid = sid;
	string eid;

	cout << " FIX SEGMENT " << id << " from " << sid << " to " << GetEndAnchor(id) << endl;

	while (true)
	{
		++nb;
		eid = GetEndAnchor(id);
		map<string, string> props = GetFeatures(id);
		DeleteAnnotation(id);
		cout << " Create unit from " << aid << " to " << eid << endl;
		id = CreateAnnotation(id, aid, eid, "unit");

		//TODO
		/* for event create a unit_event */
		map<string, string>::iterator it = props.find("text");
		if (it != props.end())
		{
			SetFeature(id, "value", it->second) ;
			props.erase("text");
			it = props.find("confidence_high") ;
			if ( it != props.end() )
				props.erase("confidence_high") ;
		}

		SetFeature(id, "subtype", "unit_text") ;
		SetFeatures(id, props);
		if ( GetAnchored(eid) )
		{
			break;
		}
		const set<AnnotationId>& ids = GetOutgoingAnnotationSet(eid, baseType);
		if ( ids.size() == 0 ) break;
		if ( ids.size() > 1 )
		{
			cerr << "STRANGE : more than one at anchor " << eid << "!! " << endl;
		}
		id = *(ids.begin());
		aid = eid;
	}

	id = CreateAnnotation(agId, sid, eid, baseType);
	if ( order > 0 ) data.setElementProperty(id, "order", order);
}

void fix_segments_at_anchor(DataModel& data, string aid)
{
	const set<AnnotationId>& ids = GetOutgoingAnnotationSet(aid, baseType);

	set<AnnotationId>::const_iterator it;
	for (it = ids.begin(); it != ids.end(); ++it)
	{
		fix_segment(data, *it);
	}
}

int main(int argc, char* const argv[])
{
	string format = "";
	string conventions = "";
	const char* progname = argv[0];
	int c;

	while ((c = getopt(argc, argv, "vf:c:")) != -1)
	{
		switch (c)
		{
		case 'f':
			format = optarg;
			break;
		case 'c':
			conventions = optarg;
			break;
		case 'v':
			cout << "convert_tag version " << TRANSAG_VERSION_NO << endl;
			return 0;
		default:
			USAGE(progname);
		}
	}

	if (optind == argc)
		USAGE(progname);
	const char* filename = argv[optind];
	if (!FileInfo(filename).exists())
	{
		Log::err() << "File not found: " << filename << endl;
		return 0;
	}

	DataModel::initEnviron(progname);
	DataModel data("TransAG");
	try
	{

		if (format == "")
			format = data.guessFileFormat(filename);
		if (format == "")
		{
			Log::err() << "Unknown file format" << endl;
			return 1;
		}

		if (!conventions.empty())
		{
			data.addAGOption("conventions", conventions);
			data.addAGOption("duration", "600.0");
		}
		cout << endl << "================================================" << endl;
		data.loadFromFile(filename, format);
		cout << endl << "================================================" << endl;

		// locate pasted graph first node
		baseType = data.segmentationBaseType("transcription_graph");
		cout << "baseType = " << baseType;

		agId = data.getAG("transcription_graph");
		const set<AnnotationId>& bids = GetAnnotationSet(agId, baseType);
		if (bids.size() == 0)
		{
			cout << "No " << baseType << " element found, upgrading to parent type" << endl;
			baseType = data.conventions().getParentType(baseType, "transcription_graph");
			cout << " -> using baseType = " << baseType;
		}
		const list<AnchorId>& aids = GetAnchorSet(agId);
		list<AnchorId>::const_iterator ita;

		for (ita = aids.begin(); ita != aids.end(); ++ita)
		{
			if (GetAnchored(*ita))
				fix_segments_at_anchor(data, *ita);
		}

		string oldfile = filename + std::string(".old");
		rename(filename, oldfile.c_str());
		data.saveToFile(filename, format, false);

	}
	catch (...)
	{
		cerr << "caught exception" << endl;
		return 1;
	}
	return 0;
}
