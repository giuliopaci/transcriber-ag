// Signal.cc: Signal is the thing to be annotated
// Author: Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.

#include <ag/Signal.h>

Signal::Signal(const Id id, string xlinkHref, MimeClass mimeClass,
	       MimeType mimeType, Encoding encoding, Unit unit, string track)
  : id(id),
    mimeClass(mimeClass),
    mimeType(mimeType),
    encoding(encoding),
    xlinkType("simple"),
    xlinkHref(xlinkHref),
    unit(unit),
    track(track)
{}

const Id Signal::getId() const { return id; }

const MimeClass Signal::getMimeClass() const { return mimeClass; }

const MimeType Signal::getMimeType() const { return mimeType; }

const Encoding Signal::getEncoding() const { return encoding; }

const string Signal::getXlinkHref() const { return xlinkHref; }

const string Signal::getXlinkType() const { return xlinkType; }

const Unit Signal::getUnit() const { return unit; }

const Track Signal::getTrack() const { return track; }

void Signal::setFeature(FeatureName featureName, FeatureValue featureValue) {
  metadata.setFeature(featureName,featureValue);
}

bool Signal::existsFeature(FeatureName featureName) {
    return metadata.existsFeature(featureName);
}

void Signal::deleteFeature(FeatureName featureName) {
    metadata.deleteFeature(featureName);
}

string Signal::getFeature(FeatureName featureName) {
    return metadata.getFeature(featureName);
}

set<string>
Signal::getFeatureNames()
{
  return metadata.getFeatureNames();
}

void Signal::unsetFeatures() {
  metadata.unsetFeatures();
}

list<string> Signal::storeSQLs(AGSetId agSetId, TimelineId timelineId) {
  list<string> sqls,subsqls;
  
  sqls.push_back("insert into SIGNAL (AGSETID,TIMELINEID,SIGNALID,MIMECLASS,MIMETYPE,ENCODING,UNIT,XLINKTYPE,XLINKHREF,TRACK) values ('" 
		 + agSetId + "','" + timelineId + "','" + id + "','" + mimeClass + "','" + mimeType
		 + "','" + encoding + "','" + unit + "','" + xlinkType + "','" + xlinkHref
		 + "','" + track + "')");

  subsqls = metadata.storeSQLs(agSetId,"",id);
  sqls.insert(sqls.end(),subsqls.begin(),subsqls.end());

  return sqls;
}

string Signal::toString() {
  string outString;

  outString += "<Signal id=\"" + id + "\""
	   + " mimeClass=\"" + mimeClass + "\""
	   + " mimeType=\"" + mimeType + "\""
	   + " encoding=\"" + encoding + "\"";

  if (!unit.empty())
    outString += " unit=\"" + unit + "\"";
  
  outString += " xlink:type=\"" + xlinkType + "\""
	   + " xlink:href=\"" + xlinkHref + "\"";
  
  if (!track.empty())
    outString += " track=\"" + track + "\"";

  outString += ">";

  if (!metadata.empty())
    outString += metadata.toString();

  outString += "</Signal>";

  return outString;
}
