# ag_wrapper_test.py
# Haejoong Lee
# Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
# Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
# For license information, see the file `LICENSE' included
# with the distribution.


import sys
sys.path.insert(0,"../src/ag_wrapper/python/.libs")
import ag

agsetId = ag.CreateAGSet("TIMIT")
timelineId = ag.CreateTimeline(agsetId)
ag.SetFeature(timelineId, "length", "30 min")
signalIds = ag.CreateSignal(timelineId, "test_uri", "test_mimeclass",
                            "test_mimetype", "test_encoding", "test_unit",
                            "test_track")
agId = ag.CreateAG(agsetId, timelineId)

anchor1 = ag.CreateAnchor(agId, 10, "sec", signalIds)
anchor2 = ag.CreateAnchor(agId, 20, "sec", signalIds)
anchor3 = ag.CreateAnchor(agId, 30, "sec", signalIds)
annotation1 = ag.CreateAnnotation(agId, anchor1, anchor2, "timit")
annotation2 = ag.CreateAnnotation(agId, anchor2, anchor3, "timit")
annotation3 = ag.CreateAnnotation(agId, anchor1, anchor3, "timit")
ag.SetFeature(annotation1, "sentence", "It's raining.")
ag.SetFeature(annotation2, "sentence", "We'll see you around 8 o'clock.")

print "A simple AG with two nodes and three arcs:"
print ag.toXML(agId)
print "-------------------------"
print ""

ag.SetAnchorOffset(anchor1, 5.0)
print "change the offset of anchor1 to 5.00:"
print ag.toXML(agId)
print "-------------------------"
print ""

ag.SetStartAnchor(annotation2, anchor1)
print "change start of annotation2 to anchor1"
print ag.toXML(agId)
print "-------------------------"
print ""

ag.SetFeature(annotation1, "tense", "Present Continuous")
print "Set feature \"tense\" of annotation2 to \"Present Continuous\""
print ag.toXML(agId)
print "-------------------------"
print ""

ag.CopyAnnotation(annotation2)
print "copy annotation2"
print ag.toXML(agId)
print "-------------------------"
print ""

ag.SplitAnnotation(annotation3)
print "split annotation3"
print ag.toXML(agId)
print "-------------------------"
print ""

