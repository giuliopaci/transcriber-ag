// agdb_demo.cc: annotation database test program
// Xiaoyi Ma
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#include <iostream>
#include <string>
#include <set>
#include <ag/AGAPI.h>


/* agdb_demo is a program to test the AG database setup.
   It create an AG, store it to the database, then load
   it back from the database. Finally, dump it out in XML
   format.
*/



int main (int argc, char **argv)
{
  if (argc != 2) {
    cout << "Usage: agdb_demo <connection string>" << endl;
    return 0;
  }
  string connStr = string(argv[1]);

  // Create a simple AGSet
  AGSetId agsetId = CreateAGSet("AGDBDEMO");
  TimelineId timelineId = CreateTimeline(agsetId);
  SetFeature(timelineId,"length","30 min");
  SignalId signalId2 = CreateSignal(timelineId,"test_uri","test_mimeclass","test_mimetype","test_encoding","test_unit","test_track");
  set<SignalId> signalIds = GetSignals(timelineId);
  AGId agId = CreateAG(agsetId,timelineId);

  AnchorId anchor1 = CreateAnchor(agId,10,"sec",signalIds);
  AnchorId anchor2 = CreateAnchor(agId,20,"sec",signalIds);
  AnchorId anchor3 = CreateAnchor(agId,30,"sec",signalIds);
  AnnotationId annotation1 = CreateAnnotation(agId,anchor1,anchor2,"timit");
  AnnotationId annotation2 = CreateAnnotation(agId,anchor2,anchor3,"timit");
  AnnotationId annotation3 = CreateAnnotation(agId,anchor1,anchor3,"timit");
  SetFeature(annotation1,"sentence","It's raining.");
  SetFeature(annotation1,"num_of_words","2");
  SetFeature(annotation1,"sense","perfect");
  SetFeature(annotation2,"sentence","We'll see you around 8 o'clock.");
  SetFeature(annotation2,"num_of_words","6");

  // Store the current AGSet to the annotation database server
  StoreToDB(connStr,agsetId);

  // Load the same AGSet from the annotation database server
  LoadFromDB(connStr,agsetId);

  // Dump it
  cout << toXML(agsetId) << endl;

  return 0;
}
