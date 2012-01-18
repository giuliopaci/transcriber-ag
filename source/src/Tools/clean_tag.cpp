/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 *
 * *  OBSOLETE  - DO NOT USE !!
 *
*  clean_tag : check annotation graph and make some fixes when required
*
*/

#include <iostream>
#include <iterator>
#include <algorithm>
#include <set>
#include <vector>
#include <list>
#include <getopt.h>
#include <ag/AGAPI.h>
#include <sys/time.h>
#include <stdlib.h>
#include <glib.h>
#include <glibmm.h>

#include "DataModel/DataModel.h"
#include "Common/util/FormatTime.h"
#include "Common/VersionInfo.h"
#include "Common/util/StringOps.h"
#include "Common/InputLanguageArabic.h"

using namespace tag;

bool checkGraph(DataModel& data, ostream& out);
bool checkQualifiers(DataModel& data, ostream& out);
bool trimTexts(DataModel& data, ostream& out);
bool isSpecialCharacter(gunichar c) ;
bool trimSpecialCharacters(DataModel& data, ostream& out) ;
bool locateFloatingOverlappingBranches(DataModel& data, ostream& out, bool do_fix);

void USAGE(const char* progname)
{
	Log::err() << "USAGE: " << progname << " [-v] [-trim] <filename>" << endl;
	Log::err() << "\toptions :" << endl;
	Log::err() << "\t\t-v : print program version " << endl;
	Log::err() << "\t\t-f : apply fixes for floating graph branches " << endl;
	Log::err() << "\t\t-trim : trim texts (remove eventual leading or trailing spaces, and presentation characters) " << endl;
	exit(1);
}

int main(int argc, char* const argv[])
{
	bool do_trim = false;
	bool do_fix = false;
	bool updated = false;
	bool trimmed = false;
	bool special_cleaned = false;

	const char* progname = argv[0];
	int c;

	while ((c = getopt(argc , argv, "vt:")) != -1) {
		switch (c) {
		case 't': do_trim=true; break;
		case 'f': do_fix=true; break;
		case 'v':
			cout << "clean_tag version " << TRANSAG_VERSION_NO << endl;
			return 0;
		default: USAGE(progname);
		}
	}

	if ( optind == argc ) USAGE(progname);
	const char* filename = argv[optind];

	DataModel::initEnviron(progname);

	DataModel data("TransAG");
	string format;

	format = data.guessFileFormat(filename);
	if ( format == "" ) {
		Log::err() << "Unknown file format" << endl;
		return 1;
	}

	data.loadFromFile(filename,format, false);

	updated = checkGraph(data, cout);
	if ( do_trim ) {
		special_cleaned = trimSpecialCharacters(data,cout) ;
		trimmed = trimTexts(data, cout);
	}

	if ( updated || trimmed || special_cleaned ) {
		string res = filename;
		data.updateVersionInfo("clean_tag", "1");
		res += ".new";
		data.saveToFile(res,format, false);
	}
	cout << "all done."  << endl ;
}

bool checkGraph(DataModel& data, ostream& out)
{
	bool updated=false;

	// browse through graph to get all "valid" annotation ids
  const vector<string>&  types = data.getMainstreamTypes() ;
  vector<string>::const_iterator itt, itt2;;
	vector<SignalSegment> v0;
	vector<SignalSegment> v1;
	vector<SignalSegment> v2;
	vector<SignalSegment> v3;
	vector<SignalSegment>::iterator it0;
	vector<SignalSegment>::iterator it1;
	vector<SignalSegment>::iterator it3;
	map<string, string> ok_id;
	itt= types.begin();
	itt2 = itt;
	if ( itt2 != types.end() ) ++itt2;

	v0.clear();
	while ( itt2 != types.end() ) {
		Log::err() << " Checking graph ..." << *itt << endl;

		if ( v0.empty() )
				data.getSegments(*itt, v0, 0.0, 0.0);
			ok_id.clear();
			int cnt =0;
			for (it0 = v0.begin(); it0 != v0.end(); ++it0 ) {
					v1.clear();
					data.getChildSegments(*itt2, v1, *it0);
					cnt +=  v1.size() ;
					for (it1 = v1.begin(); it1 != v1.end(); ++it1 ) {
						if ( ok_id.find(it1->getId()) != ok_id.end() ) {
								Log::err() << " DOUBLON " << it1->getId() << " in " << it0->getId() << endl;
						} else {
							ok_id[it1->getId()] = it0->getId();
							v2.push_back(*it1);
						}
					}
			}
			v0.clear();
			data.getSegments(*itt2, v0, 0.0, 0.0);
			for (it0 = v0.begin(); it0 != v0.end(); ++it0 ) {
				const string& id = it0->getId();
				if ( ok_id.find(id) == ok_id.end() ) {
					bool with_children =false;
					string diag;
					set<string> over;
					string todel =  id;
					// annotation is out of mainstream graph
					//  - either it is overlapped by another annotation of same type
					//     ->  then terminate overlapping annotation to current annot start
					//  - or it is unattached to graph
					//    -> then delete annotation

					out << "Warn: unlinked " << *itt2 << " with id=" << id  ;
					const string& start = GetStartAnchor(id);
					if ( GetAnchored(start) )
							out << " at " << FormatTime(GetAnchorOffset(start)).c_str();
					out << endl;


					if ( *itt2 != data.mainstreamBaseType() )
						data.getOverlappingSegmentsIds(*it0, over, *itt2, true);
					if ( over.size() > 0 ) {
						// if overlapping and overlapped segments have same end anchor,
						// then terminate overlapping at overlapped start
						set<string>::iterator it_over;
						vector<string> may_be_fixed;
						for ( it_over = over.begin(); it_over != over.end(); ++it_over ) {

							if ( GetEndAnchor(*it_over) == GetEndAnchor(id)
								&& GetStartAnchor(*it_over) == GetStartAnchor(id) ) {
								may_be_fixed.push_back(*it_over);
							} else if ( GetEndAnchor(*it_over) == GetEndAnchor(id) ) {
									SetEndAnchor(*it_over, GetStartAnchor(id));
									out << " -> Fixed : previous " << *itt2 << " resized." ;
									out << "  (set " << *it_over << " end anchor to " <<  GetStartAnchor(id)<< ")";
									out << endl;
									updated=true;
								todel = "";
							} else if ( GetStartAnchor(*it_over) == GetStartAnchor(id) ) {
									SetStartAnchor(*it_over, GetEndAnchor(id));
									out << " -> Fixed : next " << *itt2 << " resized." ;
									out << "  (set " << *it_over << " start anchor to " <<  GetEndAnchor(id)<< ")";
									out << endl;
									updated=true;
								todel = "";
							}
						}
						if ( todel == "" ) {
							v2.push_back(*it0);
							if ( may_be_fixed.size() > 0 ) {
								vector<string>::iterator it_fix;
								for ( it_fix = may_be_fixed.begin(); it_fix != may_be_fixed.end(); ++it_fix ) {
									SignalSegment s;
									data.getSegment(*it_fix, s, -1, true, true);
									ok_id[*it_fix] = *it_fix;
									v2.push_back(s);
								}
							}
						}
					}
					if ( todel != "" ) {
						try {
							out << " -> Fixed : " << *itt2 << " removed from graph " << endl;
							data.deleteMainstreamElement(todel, false);
							updated=true;
						} catch (...) {
							out << " >>> can't delete " << todel << " : " << diag << endl;
						}
					}
				}
			}
			v0.clear();
			v0 = v2;
			++itt;
			++itt2;
	}

	bool upd_qual = checkQualifiers(data, out);
	return (updated || upd_qual);
}


/**
 * check that all qualifiers are correctly defined, ie. anchors exists and no anchor inversion
 */

bool checkQualifiers(DataModel& data, ostream& out)
{
	bool updated = false;
	 const vector<string>&  types = data.getQualifierTypes() ;
	 vector<string>::const_iterator itt;

	 for ( itt = types.begin(); itt != types.end(); ++itt ) {
		 const set<AnnotationId>& ids = GetAnnotationSet(data.getAGTrans(), *itt);
		 set<AnnotationId>::const_iterator it;
		Log::err() << " Checking Qualifiers ..." << *itt << endl;

		 for ( it = ids.begin(); it != ids.end(); ++it ) {
			 string start = GetStartAnchor(*it);
			 string stop = GetEndAnchor(*it);
			 bool do_remove = false;
			 bool do_invert = false;
			 if ( GetAnchored(start) && GetAnchored(stop) ) {
				if (GetAnchorOffset(start) > GetAnchorOffset(stop) ) {
					out << " Warn: inverted anchors for " << *itt << " qualifier with id " << *it ;
					do_invert=true;
				}
			 } else {
				 string estart = data.getMainstreamStartElement(*it);
				 string estop = data.getMainstreamEndElement(*it);
				 if ( estart.empty() ) {
					 out << " Warn: " << *itt << " qualifier with id " << *it << " start anchor not linked to graph ";
						if ( ! estop.empty() ) {
							SetStartAnchor(*it, GetStartAnchor(estop));
							out << " -> Fixed : set start anchor to " << GetStartAnchor(estop) << " for " << *it << endl;
							updated = true;
						} else do_remove = true;
				 } else if ( estop.empty() ) {
					 out << " Warn: " << *itt << " qualifier with id " << *it << " end anchor not linked to graph ";
						if ( ! estart.empty() ) {
							SetEndAnchor(*it, GetEndAnchor(estart));
							out << " -> Fixed : set end anchor to " << GetEndAnchor(estart) << " for " << *it << endl;
							updated = true;
						} else do_remove = true;
				 } else {
					 string bstart = data.getStartAnchor(estart);
					 string bstop = data.getStartAnchor(estop);
					 if ( bstart != bstop
							 && GetAnchorOffset(bstart) > GetAnchorOffset(bstop) ) {
						 out << " Warn: inverted anchors for " << *itt << " qualifier with id " << *it ;
						do_invert=true;
					 } else {
							const set<AnnotationId>& ids2 =
									GetOutgoingAnnotationSet(stop, data.mainstreamBaseType());
							if (ids2.size() == 0) {
								out << "Warn: unattached " << *itt << " with id " << *it << endl;
								do_remove = true;
							} else {
								string send = GetEndAnchor(*(ids2.begin()));
								if ( send == start ) {
									out << "Warn: anchor inversion for" << *itt << " with id " << *it << endl;
									do_invert = true;
								}
							}
					 }
				 }
			 }
			 if ( do_invert ) {
				SetStartAnchor(*it, stop);
				SetEndAnchor(*it, start);
				// check that no span rule violated
				if ( data.conventions().getConfiguration(*itt+",can_span_over_anchor") == "false") {
					string bstart = data.getAnchoredBaseTypeStartId(*it);
					string bstop = data.getAnchoredBaseTypeEndId(*it);
					 if ( bstart != bstop ) {
						do_remove = true;
						out << "  but can't span over segments, will be removed " << endl;
					 }
				}
				if ( ! do_remove ) {
					out << " -> Fixed : anchors set to correct order for " << *it << endl;
					updated = true;
				}
			 }
			 if ( do_remove ) {
				out << " -> Fixed : " << *it << " removed from graph " << endl;
				data.deleteElement(*it, false);
				updated = true;
			 }
		 }
	 }
	return updated;
}

/**
 * check that all qualifiers are correctly defined, ie. anchors exists and no anchor inversion
 */

bool trimTexts(DataModel& data, ostream& out)
{
	bool updated = false;
	const set<AnnotationId>& ids = GetAnnotationSet(data.getAGTrans(), data.mainstreamBaseType());
	 set<AnnotationId>::const_iterator it;
	 for ( it = ids.begin(); it != ids.end(); ++it ) {
		if ( ExistsFeature(*it, "value") ) {
			string text = GetFeature(*it, "value");
			if ( !text.empty() ) {
				if ( isspace(text[0]) || isspace(text[text.size()-1]) ) {
					out << "Warn : " << data.mainstreamBaseType() << " with id " << *it << " contains unwanted spacings " << endl;
					SetFeature(*it, "value", StringOps(text).trim(" \t\n"));
					out << " -> Fixed : " << *it << " trimmed " << endl;
					updated=true;
				}
			}
		}
	}
	return updated;
}

bool trimSpecialCharacters(DataModel& data, ostream& out)
{
	static int cpt = 0 ;

	bool updated = false;
	const set<AnnotationId>& ids = GetAnnotationSet(data.getAGTrans(), data.mainstreamBaseType());
	 set<AnnotationId>::const_iterator it;
	 for ( it = ids.begin(); it != ids.end(); ++it ) {
		if ( ExistsFeature(*it, "value") ) {
			string text = GetFeature(*it, "value");
			if ( !text.empty() )
			{
				Glib::ustring res  ;
				//> special CHARACTERS
				bool special_cleaned = false ;
				Glib::ustring::iterator its ;
				Glib::ustring tmp = text ;
				res.clear() ;
				for (its=tmp.begin(); its!=tmp.end(); its++ ) {
					if ( !isSpecialCharacter(*its) )
						res = res + *its ;
					else {
						cpt ++ ;
						out << " -> Fixed : " <<hex<< *its <<dec<< " special cleaned " << endl;
						special_cleaned = true ;
					}
				}

				if (special_cleaned) {
					SetFeature(*it, "value", res);
					updated=true;
				}
			}
		}
	}
	 if (updated)
		 out << " Special cleaned [" << cpt << "]" << endl;
	return updated;
}

bool isSpecialCharacter(gunichar c)
{
	if ( c == InputLanguageArabic::RLM
			|| c == InputLanguageArabic::RLE
			|| c == InputLanguageArabic::RLM
			|| c == InputLanguageArabic::LRE
			|| c == InputLanguageArabic::LRM
			|| c == InputLanguageArabic::PDF
			|| c == InputLanguageArabic::RLO )
	{
		return true ;
	}
	else
		return false ;
}


bool locateFloatingOverlappingBranches(DataModel& data, ostream& out, int notrack, bool do_fix)
{
#ifdef EN_COURS
	set<AnnotationId>& graphids;
	// if graph already initialized for some tracks, initialize it for added tracks
	string aid = getAnchorAtOffset(notrack, 0.0);
	const set<AnnotationId>& startids = GetOutgoingAnnotationSet(aid, data.getBaseSegmentType());
	if (startids.size() == 0)
	{
		Log::err() << "No graph found !!" << endl;
		return false;
	}
	vector<string> v;
	vector<string>::iterator itv;
	string nextid = *(startids.begin());
	while ( nextid != "") {
		data.getLinkedElements(nextid, v, data.getBaseSegmentType(), false, false);
		for ( itv=v.begin(); itv != v.end(); ++itv ) graphids.insert(*itv);
		aid =GetEndAnchor(v.back());
		const set<AnnotationId>& startids = GetOutgoingAnnotationSet(aid, data.getBaseSegmentType());
		if (startids.size() > 0) {

		}
		v.clear();
	}

	GetAnchorSetByOffset (AGId agId, Offset offset)
	string startAnchor =
	const set<AnnotationId>& ids = GetAnnotationSet(data.getAGTrans(), data.mainstreamBaseType());
	 set<AnnotationId>::const_iterator it;
#endif
	return false;
}
