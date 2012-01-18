// Timeline.cc: A Timeline is a set of Signals
// Author: Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <ag/AGTypes.h>
#include <ag/Identifiers.h>
#include <ag/Signal.h>
#include <ag/Metadata.h>
#include <ag/Timeline.h>

// Constructor
Timeline::Timeline(const Id& id)
: id(id)
{
  signalIds = new Identifiers(id,"");
}

Timeline::~Timeline()
{
  for (SignalSet::iterator pos = signals.begin(); pos != signals.end(); ++pos) {
    Identifiers::deleteSignalRef((*pos)->getId());
    delete *pos;
  }
  
  delete signalIds;
}

Id Timeline::createSignal(Id id, string xlinkHref, 
			       MimeClass mimeClass, MimeType mimeType,
			       Encoding encoding, Unit unit, string track)
  throw (AGException) {
  Signal* signal;
  
  if (id == this->id)
    id = signalIds->new_id();
  else if (Identifiers::getNamespace(id) == this->id)
    id = signalIds->new_id(id);
  else
    throw(AGException(id + " is not a TimelineId or SignalId!"));

  signal = new Signal(id, xlinkHref, mimeClass, mimeType, encoding, unit, track);
  signals.insert(signal);
  Identifiers::addSignalRef(id,signal);
  return id;
}

void Timeline::deleteSignal(Signal* signal) {
  SignalId signalId = signal->getId();
  signals.erase(signal);
  signalIds->reclaim_id(signalId);
  Identifiers::deleteSignalRef(signalId);
}


set<SignalId>
Timeline::getSignals() const
{
  set<SignalId> signalIds;
  SignalSet::const_iterator pos;
  for (pos = signals.begin(); pos != signals.end(); ++pos)
    signalIds.insert((*pos)->getId());
  return signalIds;
}

void Timeline::setFeature(FeatureName featureName, FeatureValue featureValue) {
  metadata.setFeature(featureName,featureValue);
}

bool Timeline::existsFeature(FeatureName featureName) {
    return metadata.existsFeature(featureName);
}

void Timeline::deleteFeature(FeatureName featureName) {
    metadata.deleteFeature(featureName);
}

string Timeline::getFeature(FeatureName featureName) {
    return metadata.getFeature(featureName);
}

set<string>
Timeline::getFeatureNames()
{
  return metadata.getFeatureNames();
}

void Timeline::unsetFeatures() {
  metadata.unsetFeatures();
}

list<string> Timeline::storeSQLs(AGSetId agSetId) {
  list<string> sqls,subsqls;
  
  sqls.push_back("insert into TIMELINE (AGSETID,TIMELINEID) values ('"
	      + agSetId + "','" + id + "')");

  subsqls = metadata.storeSQLs(agSetId,"",id);
  sqls.insert(sqls.end(),subsqls.begin(),subsqls.end());

  for (SignalSet::const_iterator pos = signals.begin();
       pos != signals.end(); ++pos) {
    subsqls.clear();
    subsqls = (*pos)->storeSQLs(agSetId,id);
    sqls.insert(sqls.end(),subsqls.begin(),subsqls.end());
  }

  return sqls;
}

string Timeline::toString() {
  string outString;

  outString += "<Timeline id=\"" + id + "\">\n";

  if (!metadata.empty())
    outString += metadata.toString() + "\n";

  for (SignalSet::const_iterator pos = signals.begin();
       pos != signals.end(); ++pos)
    outString += (*pos)->toString() + "\n";

  outString += "</Timeline>\n";

  return outString;

}
