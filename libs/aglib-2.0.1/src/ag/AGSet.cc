// AGSet.cc: implementation of AGSet class.
// Author: Xiaoyi Ma
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.

#include <vector>

#include <ag/AGSet.h>

// Constructors
AGSet::AGSet(Id id)
  : id(id)
{
  agIds = new Identifiers(id, "");
  timelineIds = new Identifiers(id,"T");
}

AGSet::~AGSet()
{
#ifdef _MSC_VER
  {
#endif
  for (TimelineSet::iterator pos = timelineSet.begin(); pos !=timelineSet.end(); ++pos) {
    Identifiers::deleteTimelineRef((*pos)->getId());
    delete *pos;
  }
#ifdef _MSC_VER
  }
#endif

#ifdef _MSC_VER
  {
#endif
  for (AGS::iterator pos = ags.begin(); pos !=ags.end(); ++pos) {
    Identifiers::deleteAGRef((*pos)->getId());
    delete *pos;
  }
#ifdef _MSC_VER
  }
#endif

  delete timelineIds;
  delete agIds;
}

TimelineId AGSet::createTimeline(Id id) throw (AGException) {
  TimelineId timelineId;
  Timeline* t;

  if (id == this->id)
    timelineId = timelineIds->new_id();
  else if (Identifiers::getNamespace(id) == this->id)
    timelineId = timelineIds->new_id(id);
  else
    throw AGException(id + " is not a AGSet or Timeline id!");

  t = new Timeline(timelineId);
  Identifiers::addTimelineRef(timelineId, t);
  timelineSet.push_back(t);
  return timelineId;
}

void AGSet::deleteTimeline(Id id) throw (AGException) {
  if (Identifiers::existsTimeline(id)) {
    Timeline* t = Identifiers::getTimelineRef(id);
#ifdef _MSC_VER
    {
#endif
    for (TimelineSet::iterator pos=timelineSet.begin(); pos != timelineSet.end(); ++pos) {
      if (*pos == t) {
	timelineSet.erase(pos);
	break;
      }
    }
#ifdef _MSC_VER
    }
#endif
    delete t;
    Identifiers::deleteTimelineRef(id);
    timelineIds->reclaim_id(id);
  }
}

// Id is AGSetId or AGId
Id AGSet::createAG(Id id, Timeline* timeline) throw (AGException) {
  AG* ag;
  AGId agId;

  if (id == this->id)
    agId = agIds->new_id();
  else if(Identifiers::getNamespace(id) == this->id)
    agId = agIds->new_id(id);
  else
    throw AGException(id + " is not a AGSetId or AGId!");

  ag = new AG(agId,timeline);
  Identifiers::addAGRef(agId, ag);
  ags.push_back(ag);
  return agId;
}

void AGSet::deleteAG(Id agId) {
  if (Identifiers::existsAG(agId)) {
    AG* ag = Identifiers::getAGRef(agId);
#ifdef _MSC_VER
    {
#endif
    for (AGS::iterator pos=ags.begin(); pos != ags.end(); ++pos) {
      if (*pos == ag) {
	ags.erase(pos);
	break;
      }
    }
#ifdef _MSC_VER
    }
#endif
    delete ag;
    Identifiers::deleteAGRef(agId);
    agIds->reclaim_id(agId);
  }
}


set<AGId>
AGSet::getAGIds() {
  set<AGId> agids;

  for (AGS::iterator pos = ags.begin(); pos != ags.end(); pos++)
    agids.insert((*pos)->getId());

  return agids;
}
//// Features ////

// this is for the metadata AGSet
void AGSet::setFeature(FeatureName featureName, FeatureValue featureValue) {
  metadata.setFeature(featureName,featureValue);
}

bool AGSet::existsFeature(FeatureName featureName) {
    return metadata.existsFeature(featureName);
}

void AGSet::deleteFeature(FeatureName featureName) {
    metadata.deleteFeature(featureName);
}

string AGSet::getFeature(FeatureName featureName) {
    return metadata.getFeature(featureName);
}

set<string>
AGSet::getFeatureNames()
{
  return metadata.getFeatureNames();
}

void
AGSet::getAnnotationFeatureNames(set<string>& S, const AnnotationType& type)
{
  for (int i=0; i < ags.size(); ++i)
    ags[i]->getAnnotationFeatureNames(S, type);
}

void
AGSet::getAnnotationTypes(set<string>& S)
{
  for (int i=0; i < ags.size(); ++i)
    ags[i]->getAnnotationTypes(S);
}

void AGSet::unsetFeatures() {
  metadata.unsetFeatures();
}

list<string> AGSet::storeSQLs() {
  list<string> sqls;
  list<string> subsqls;
  string f = "CREATE TABLE " + id + "(ANNOTATIONID VARCHAR(50) BINARY NOT NULL,";

  // SQLs to drop the old feature table and create a new one
  sqls.push_back("DROP TABLE IF EXISTS " + id);
  if (!features.empty()) {
    for (FeatureNameSet::iterator pos = features.begin(); pos != features.end(); ++pos)
      f += "`" + *pos + "` TEXT,";
    f += "PRIMARY KEY (ANNOTATIONID))";
    sqls.push_back(f);
  }

  string agsets = "insert into AGSET (AGSETID,VERSION,XMLNS,XLINK,DC) values ('"
    + id +"','1.0','http://www.ldc.upenn.edu/atlas/ag/','http://www.w3.org/1999/xlink','http://purl.org/DC/documents/rec-dces-19990702.htm')";
  sqls.push_back(agsets);

  subsqls = metadata.storeSQLs(id,"",id);
  sqls.insert(sqls.end(),subsqls.begin(),subsqls.end());

#ifdef _MSC_VER
  {
#endif
  for (TimelineSet::const_iterator pos = timelineSet.begin(); pos != timelineSet.end(); ++pos) {
    subsqls.clear();
    subsqls = (*pos)->storeSQLs(id);
    sqls.insert(sqls.end(),subsqls.begin(),subsqls.end());
  }
#ifdef _MSC_VER
  }
#endif

#ifdef _MSC_VER
  {
#endif
  for (AGS::const_iterator pos = ags.begin(); pos != ags.end(); ++pos) {
    subsqls.clear();
    subsqls = (*pos)->storeSQLs(id,features);
    sqls.insert(sqls.end(),subsqls.begin(),subsqls.end());
  }
#ifdef _MSC_VER
  }
#endif

  return sqls;
}

string AGSet::toXML(AGId agId) {
  string outString;

/* (( BT Patch -- */
/** 13/09/2009 -> added AGSet version number in output file */
	string version("");
	if ( existsFeature("version") )
		version = getFeature("version");

	if ( version.empty() )
		version = "1.0";
/* -- BT Patch )) */

  outString = "<?xml version=\"1.0\"?>\n<!DOCTYPE AGSet SYSTEM \"ag.dtd\">\n";
  outString += "<AGSet id=\"" + id
/* (( BT Patch -- */
// >> replaced    + "\" version=\"1.0\""
    + "\" version=\"" + version + "\""
/* -- BT Patch )) */    
    + " xmlns=\"http://www.ldc.upenn.edu/atlas/ag/\""
    + " xmlns:xlink=\"http://www.w3.org/1999/xlink\""
    + " xmlns:dc=\"http://purl.org/DC/documents/rec-dces-19990702.htm\">\n";

/* (( BT Patch -- */
	if ( existsFeature("version") ) {
		// do not print it again !!
		Metadata copy = metadata;
		copy.deleteFeature("version");
 	 	outString += copy.toString() + "\n";
	} else
/* -- BT Patch )) */

  outString += metadata.toString() + "\n";
  for (TimelineSet::const_iterator pos = timelineSet.begin(); pos != timelineSet.end(); ++pos)
    outString += (*pos)->toString();

  outString += Identifiers::getAGRef(agId)->toString();
  outString += "</AGSet>\n";

  return outString;
}

string AGSet::toString() {
  string outString;

/* (( BT Patch -- */
 	/** Patch BT-PLr 13/09/2009 -> added AGSet version number in output file */
  	string version("");
  	if ( existsFeature("version") )
  		version = getFeature("version");

  	if ( version.empty() )
  		version = "1.0";
/* -- BT Patch ) */

  outString = "<?xml version=\"1.0\"?>\n<!DOCTYPE AGSet SYSTEM \"ag.dtd\">\n";

  outString += "<AGSet id=\"" + id

/* (( BT Patch -- */
	//+ "\" version=\"1.0\""
    + "\" version=\"" + version + "\""
/* -- BT Patch )) */
    + " xmlns=\"http://www.ldc.upenn.edu/atlas/ag/\""
    + " xmlns:xlink=\"http://www.w3.org/1999/xlink\""
    + " xmlns:dc=\"http://purl.org/DC/documents/rec-dces-19990702.htm\">\n";
/* (( BT Patch -- */
  	if ( existsFeature("version") ) {
  		// do not print it again !!
  		Metadata copy = metadata;
  		copy.deleteFeature("version");
   	 	outString += copy.toString() + "\n";
  	} 
  	else
/* -- BT Patch )) */
  		outString += metadata.toString() + "\n";

#ifdef _MSC_VER
  {
#endif
  for (TimelineSet::const_iterator pos = timelineSet.begin(); pos != timelineSet.end(); ++pos)
    outString += (*pos)->toString();
#ifdef _MSC_VER
  }
#endif

#ifdef _MSC_VER
  {
#endif
  for (AGS::const_iterator pos = ags.begin(); pos != ags.end(); ++pos)
    outString += (*pos)->toString();
#ifdef _MSC_VER
  }
#endif

  outString += "</AGSet>\n";

  return outString;
}

