// AG.cc: A AG is a set of annotations along with the index structure.
// Author: Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <ag/AG.h>

AG::AG(Id id, Timeline* timeline, string type)
  : id(id),type(type),timeline(timeline)
{
  anchorIds = new Identifiers(id,"");
  annotationIds = new Identifiers(id,"E");
}

AG::~AG() {
#ifdef _MSC_VER
  {
#endif
  for (AnnotationSeq::iterator pos = this->begin(); pos != this->end(); ++pos) {
    Identifiers::deleteAnnotationRef((*pos)->getId());
    delete *pos;
  }
#ifdef _MSC_VER
  }
#endif

// deletion of anchors is handled by AnnotationIndex
//    for (AnchorSet::iterator pos = I.anchorSet.begin(); pos != I.anchorSet.end(); ++pos) {
//      Identifiers::deleteAnnotationRef((*pos)->getId());
//      delete *pos;
//    }

  delete anchorIds;
  delete annotationIds;

}

void AG::addAnchor(Anchor* a) {
  Identifiers::addAnchorRef(a->getId(),a);
  if (I.anchorSet.find(a) == I.anchorSet.end()) {
    I.addAnchor(a);
  }
}

void AG::deleteAnchor(Anchor* a) throw (AGException) {
  AnchorId anchorId = a->getId();
  if (I.incoming[a].empty() && I.outgoing[a].empty()) {
    Identifiers::deleteAnchorRef(anchorId);
    I.deleteAnchor(a);
    delete a;
    anchorIds->reclaim_id(anchorId);
  } else
    throw AGException("Anchor " + anchorId + "are used by some annotations!");
}


// modify existing offset
void AG::setAnchorOffset(Anchor* a, Offset o) {
  AnnotationSet& anns1 = I.incoming[a];
  AnnotationSet& anns2 = I.outgoing[a];
  AnnotationSet::iterator pos;
  for (pos=anns1.begin(); pos!=anns1.end(); ++pos) erase(*pos);
  for (pos=anns2.begin(); pos!=anns2.end(); ++pos) erase(*pos);

  I.deleteAnchor(a);   // delete anchor from indexes
  a->setOffset(o);     // update the anchor
  I.addAnchor(a);      // add new anchor to indexes

  for (pos=anns1.begin(); pos!=anns1.end(); ++pos) insert(*pos);
  for (pos=anns2.begin(); pos!=anns2.end(); ++pos) insert(*pos);
}

// split an anchor into two anchors - a1 gets all the incoming
// annotations while a2 gets all the outgoing annotations...
Anchor* AG::splitAnchor(Anchor* a1) {
  Anchor* a2 = new Anchor(anchorIds->new_id(), a1);  // clone the anchor
  addAnchor(a2);

  // for each outgoing arc, change its start node to node2
  AnnotationSet as = getOutgoingAnnotationSet(a1);
  for (AnnotationSet::iterator i = as.begin(); i != as.end(); i++)
    setStartAnchor((Annotation *)*i, a2);
  return a2;
}

// editing functions on annotations (updating indexes)

void AG::add(Annotation* a) {
  insert(a);                // add annotation to the annotationSet
  I.add(a);                 // update index
  Identifiers::addAnnotationRef(a->getId(),a);
}

Id AG::createAnnotation(Id id, Anchor* anchor1,Anchor* anchor2,
			AnnotationType annotationType) throw(AGException) {
  Annotation* a;
  Id nspace = Identifiers::getNamespace(id);

  if (id == this->id)
    a = new Annotation(annotationIds->new_id(),anchor1,anchor2,annotationType);
  else if (nspace == this->id)
    a = new Annotation(annotationIds->new_id(id),anchor1,anchor2,annotationType);
  else
    throw AGException(id + " is not a AG or Annotation id!");

  add(a);
  return a->getId();
}

Annotation* AG::copyAnnotation(const Annotation* a) {
  Annotation* na = new Annotation(annotationIds->new_id(), a);
  add(na);
  return na;
}

Annotation* AG::splitAnnotation(Annotation* a1) {  
  Anchor* end = a1->getEndAnchor();
  Annotation* a2 = copyAnnotation(a1);   // make two arcs with same span
  Anchor* n = new Anchor(anchorIds->new_id(), end->getUnit(), end->getSignals());
  addAnchor(n);
  setEndAnchor(a1,n);              // make a1 end at this anchor
  setStartAnchor(a2,n);            // make a2 start at this anchor
  return a2;
}

AnnotationSet AG::nSplitAnnotation(Annotation* a, int n) {
  AnnotationSet as;
  Annotation* tmp = a;
  for (int i=n; i>1; i--) {
    tmp = splitAnnotation(tmp);
    as.insert(tmp);
  }
  return as;
}

void AG::deleteAnnotation(Annotation* a) {
  if (getById(a->getId()) != NULL) {
    Identifiers::deleteAnnotationRef(a->getId());
    I.deleteAnnotation(a);
    annotationIds->reclaim_id(a->getId());
    erase(a);
    delete a;
  }
}

void AG::setFeature(Id id, FeatureName featureName, FeatureValue featureValue) {
  if (id == this->id)
    metadata.setFeature(featureName,featureValue);
  else 
    setFeature(getById(id), featureName, featureValue);
}

bool AG::existsFeature(Id id, FeatureName featureName) {
  if (id == this->id)
    return metadata.existsFeature(featureName);
  else {
    Annotation* a = getById(id);
    return a->existsFeature(featureName);
  }
}
void AG::deleteFeature(Id id, FeatureName featureName) {
  if (id == this->id)
    metadata.deleteFeature(featureName);
  else {
    Annotation* a = getById(id);
    I.deleteFeature(a, featureName);
    a->deleteFeature(featureName);
  }
}

string AG::getFeature(Id id, FeatureName featureName) {
  if (id == this->id)
    return metadata.getFeature(featureName);
  else {
    Annotation* a = getById(id);
    return a->getFeature(featureName);
  }
}

set<string>
AG::getFeatureNames(Id id)
{
  if (id == this->id)
    return metadata.getFeatureNames();
  else {
    Annotation* a = getById(id);
    return a->getFeatureNames();
  }
}

void
AG::getAnnotationFeatureNames(set<string>& S, const AnnotationType& type)
{
  ByFeature::iterator p = I.byFeature.begin();

  if (type.empty()) {
    for (; p != I.byFeature.end(); ++p)
      S.insert(p->first.first);
  }
  else {
    AnnotationSet& T = I.byType[type];
    for (; p != I.byFeature.end(); ++p) {
      AnnotationSet::iterator pp = p->second.begin();
      for (; pp != p->second.end(); ++pp)
	if (T.find(*pp) != T.end()) {
	  S.insert(p->first.first);
	  break;
	}
    }
  }
}

void
AG::getAnnotationTypes(set<string>& S)
{
  ByType::iterator p = I.byType.begin();
  for (; p != I.byType.end(); ++p)
    S.insert(p->first);
}

void
AG::setFeatures(Id id, map<string,string>& features)
{
  if (id == this->id)
    metadata.setFeatures(features);
  else {
    Annotation* a = getById(id);
    for (map<string,string>::iterator pos=features.begin();
	 pos!=features.end();
	 ++pos)
      setFeature(a, pos->first, pos->second);
  }
}

map<string,string>
AG::getFeatures(Id id)
{
  if (id == this->id)
    return (map<string,string>) metadata;
  else {
    Annotation* a = getById(id);
    return a->getFeatures();
  }
}

void AG::unsetFeatures(Id id) {
  if (id == this->id)
    metadata.unsetFeatures();
  else {
    Annotation* a = getById(id);
    set<string> fnames = a->getFeatureNames();
    for (set<string>::iterator pos = fnames.begin(); pos != fnames.end(); ++pos)
      setFeature(a,*pos,"");
  }
}

// set a feature of arc n to the specified value, updating the index
void AG::setFeature(Annotation* a, const String& feature, const String& value) {
  I.deleteFeature(a, feature);            // delete the old index entry
  a->setFeature(feature, value);    // set the feature value
  I.addFeature(a, feature, value);        // update the index entry
}

AnchorId
AG::createAnchor(Id id, Offset offset, Unit unit, set<string> const &signals)
{
  Anchor* a;

  if (id == this->id)
    a = new Anchor(anchorIds->new_id(),offset,unit,signals);
  else if (Identifiers::getNamespace(id) == this->id)
    a = new Anchor(anchorIds->new_id(id),offset,unit,signals);
  else
    throw AGException(id + " is not an AG id or Anchor id!");

  addAnchor(a);
  return a->getId();
}

AnchorId
AG::createAnchor(Id id, Unit unit, set<string> const &signals)
{
  Anchor* a;

  if (id == this->id)
    a = new Anchor(anchorIds->new_id(),unit,signals);
  else if (Identifiers::getNamespace(id) == this->id)
    a = new Anchor(anchorIds->new_id(id),unit,signals);
  else
    throw AGException(id + " is not an AG id or Anchor id!");

  addAnchor(a);
  Identifiers::addAnchorRef(a->getId(),a);
  return a->getId();
}



void AG::unsetAnchorOffset(Anchor* a) {
  AnnotationSet& anns1 = I.incoming[a];
  AnnotationSet& anns2 = I.outgoing[a];
  AnnotationSet::iterator pos;
  for (pos=anns1.begin(); pos!=anns1.end(); ++pos) erase(*pos);
  for (pos=anns2.begin(); pos!=anns2.end(); ++pos) erase(*pos);

  I.deleteAnchor(a);   // delete anchor from indexes
  a->unsetOffset();     // update the anchor
  I.addAnchor(a);      // add new anchor to indexes

  for (pos=anns1.begin(); pos!=anns1.end(); ++pos) insert(*pos);
  for (pos=anns2.begin(); pos!=anns2.end(); ++pos) insert(*pos);
}

//// Index ////
 
AnchorSet AG::getAnchorSetByOffset(Offset offset, double epsilon) {
  double dist;
  AnchorSet byoffset;
  epsilon = epsilon>0.0?epsilon:-epsilon;
  
  if (epsilon == 0.0)
    return I.anchorSetByOffset[offset];
  else {
    AnchorSeq aset = *(getAnchorSet());
    for(AnchorSet::iterator pos = aset.begin(); pos != aset.end(); ++pos) {
      dist = (*pos)->getOffset() - offset;
      dist = dist>0?dist:-dist;
      if (dist <= epsilon)
	byoffset.insert(*pos);
    }
    return byoffset;
  }
}

// Set the start anchor of an annotation to the specified anchor, updating the index
void AG::setStartAnchor(Annotation* a, Anchor* anchor) {
  I.deleteAnnotation(a);
  erase(a);
  a->setStartAnchor(anchor);
  I.add(a);
  insert(a);
}

// Set the end anchor of an annotation to the specified anchor, updating the index
void AG::setEndAnchor(Annotation* a, Anchor* anchor) {
  I.deleteAnnotation(a);
  erase(a);
  a->setEndAnchor(anchor);
  I.add(a);
  insert(a);
}

Anchor* AG::getAnchorById(const Id& id) throw (AGException) {
  if(I.existsAnchor(id))
    return I.anchorById[id];
  else
    throw AGException("Anchor " + id + " doesn't exist!");
}

Annotation* AG::getById(const Id& id) throw (AGException) {
  if(I.existsAnnotation(id))
    return I.byId[id];
  else
    throw AGException("Annotation " + id + " doesn't exist!");
}

AnchorSet AG::getAnchorSetNearestOffset(Offset offset) {
  return getAnchorSetByOffset(getNearestOffset(offset));
}

AnnotationSeq AG::getAnnotationSeqByOffset(Offset begin, Offset end) {
  AnnotationSeq as;
  AnnotationSeq::iterator pos;
  
  if (!this->empty()) {
    if (begin == 0.0 && end == 0.0)
      as = *this;
    else if (begin != 0.0 && end == 0.0) {
      for (pos = this->begin(); pos != this->end(); ++pos)
	if ((*pos)->getStartAnchor()->getOffset() >= begin)
	  break;
      if (pos != this->end())
	for (; pos != this->end(); ++pos)
	  as.insert(*pos);
    } else if (begin == 0.0 && end != 0.0) {
      for (pos = this->begin(); pos != this->end(); ++pos)
	if ((*pos)->getStartAnchor()->getOffset() <= end)
	  as.insert(*pos);
	else
	  break;
    } else {
      for (pos = this->begin(); pos != this->end(); ++pos)
	if ((*pos)->getStartAnchor()->getOffset() >= begin)
	  break;
      for (; pos != this->end(); ++pos)
	if ((*pos)->getStartAnchor()->getOffset() <= end)
	  as.insert(*pos);
	else
	  break;
    }
  }
  return as;
}

// Get the incoming annotations of this type to the specified node
AnnotationSet AG::getIncomingAnnotationSetByType(Anchor* a, AnnotationType t) {
  AnnotationSet as = getIncomingAnnotationSet(a);
  AnnotationSet typed;
  for(AnnotationSet::iterator i = as.begin(); i != as.end(); ++i) {
    if ((*i)->getType() == t)
      typed.insert((*i));
  }
  return typed;
}

// Get the incoming annotations of this type to the specified node
AnnotationSet AG::getOutgoingAnnotationSetByType(Anchor* a, AnnotationType t) {
  AnnotationSet as = getOutgoingAnnotationSet(a);
  AnnotationSet typed;
  for(AnnotationSet::iterator i = as.begin(); i != as.end(); ++i) {
    if ((*i)->getType() == t)
      typed.insert((*i));
  }
  return typed;
}

list<string> AG::storeSQLs(AGSetId agSetId, FeatureNameSet features) {
  list<string> sqls,subsqls;
  string tid;
  string values;
  AnchorSeq* anchorSet = getAnchorSet();

  if (timeline)
    tid = timeline->getId();
  else
    tid = "";

  sqls.push_back("insert into AG (AGSETID,AGID,TYPE,TIMELINEID) values ('"
	      + agSetId + "','" + id + "','" + type + "','" + tid + "')");

  subsqls = metadata.storeSQLs(agSetId,id,id);
  sqls.insert(sqls.end(),subsqls.begin(),subsqls.end());

  // Insert anchors
  values.erase();
#ifdef _MSC_VER
  {
#endif
  for (AnchorSeq::iterator pos = anchorSet->begin();
       pos != anchorSet->end(); ++pos) {
    char buffer[100];
    sprintf(buffer, "%f", (*pos)->getOffset());
    string aid = (*pos)->getId();
    string unit = (*pos)->getUnit();
    set<string> signals = (*pos)->getSignals();
    values += "('" + agSetId + "','" + id + "','" + aid + "'," +  buffer + ",'" 
      + unit + "','";
    if (!signals.empty()) {
      set<string>::iterator sig = signals.begin();
      values += *sig;
      for (++sig; sig != signals.end(); ++sig)
	values += " " + *sig;
    }
    values += "')";
    if (pos != --anchorSet->end())
      values += ",";
  }
#ifdef _MSC_VER
  }
#endif
  if (!values.empty())
    sqls.push_back("insert into ANCHOR (AGSETID,AGID,ANCHORID,OFFSET,UNIT,SIGNALS) values " + values);


  // insert annotations
  values.erase();
#ifdef _MSC_VER
  {
#endif
  for (AnnotationSeq::iterator pos = begin(); pos != end(); ++pos) {
    string aid = (*pos)->getId();
    string startid = (*pos)->getStartAnchor()->getId();
    string endid = (*pos)->getEndAnchor()->getId();
    string type = (*pos)->getType();
    values += "('" + agSetId + "','" + id + "','" + aid 
      + "','" +  startid + "','" + endid+ "','" + type + "')";

    if (pos != --end())
      values += ",";
  }
#ifdef _MSC_VER
  }
#endif
  if (!values.empty())
    sqls.push_back("insert into ANNOTATION (AGSETID,AGID,ANNOTATIONID,START,END,TYPE) values " + values);


  // insert annotation features
  string colNames;
  if (features.empty())
    return sqls;
  else
    colNames = "ANNOTATIONID,`";
#ifdef _MSC_VER
  {
#endif
  for (FeatureNameSet::iterator pos = features.begin(); pos != features.end(); ++pos) {
    colNames += *pos + "`,`";
    //if (pos != --features.end())
    //  colNames += ",";
  }
  colNames = colNames.substr(0,colNames.size()-2);
#ifdef _MSC_VER
  }
#endif

  values.erase();
#ifdef _MSC_VER
  {
#endif
  for (AnnotationSeq::iterator pos = begin(); pos != end(); ++pos) {
    Id aid = (*pos)->getId();
    values += "('" + aid + "',";
    for (set<string>::iterator pos2 = features.begin(); pos2 != features.end(); ++pos2) {
      string name = *pos2;
      string value;
      if (existsFeature(aid,name)) {
	value = getFeature(aid,name);
	string slashedFValue = value;
	int length = value.length();

	// escape all quotations
	if (value.find_first_of("'") != std::string::npos) {
	  slashedFValue = "";
	  for (int i = 0; i < length; i++) {
	    if (value[i] == '\'')
	      slashedFValue += "\\'";
	    else
	      slashedFValue += value[i];
	  }
	}
	values += "'" + slashedFValue + "'";
      } else
	values += "NULL";
      
      if (pos2 == --features.end())
	values += ")";
      else
	values += ",";
    }
    if (pos != --end())
      values += ",";
  }
#ifdef _MSC_VER
  }
#endif
  if (!values.empty())
    sqls.push_back("insert into " + agSetId + " (" + colNames + ") values " + values);
  
  return sqls;
}

string AG::toString() {
  string outString;

  if (type == "")
    if (timeline == NULL)
      outString += "<AG id=\"" + id + "\">\n";
    else
      outString += "<AG id=\"" + id + "\" timeline=\"" + timeline->getId() + "\">\n";
  else
    if (timeline == NULL)
      outString += "<AG id=\"" + id + "\" type=\"" + getType()
	       + "\">\n";
    else
      outString += "<AG id=\"" + id + "\" type=\"" + getType()
	       + "\" timeline=\"" + timeline->getId() + "\">\n";

  if (!metadata.empty())
    outString += metadata.toString() + "\n";

#ifdef _MSC_VER
  {
#endif
  for (AnchorSet::iterator pos = getAnchorSet()->begin();
       pos != getAnchorSet()->end(); ++pos) {
    outString += (*pos)->toString() + "\n";
  }
#ifdef _MSC_VER
  }
#endif

#ifdef _MSC_VER
  {
#endif
  for (AnnotationSeq::iterator pos = begin(); pos != end(); ++pos) {
    outString += (*pos)->toString() + "\n";
  }
#ifdef _MSC_VER
  }
#endif

  outString += "</AG>\n";

  return outString;
}
