# ag_wrapper_test.tcl
# Haejoong Lee
# Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
# Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
# For license information, see the file `LICENSE' included
# with the distribution.


load ../src/ag_wrapper/tcl/.libs/ag.so

set agsetId [AG::CreateAGSet "TIMIT"]
set timelineId [AG::CreateTimeline $agsetId]
AG::SetFeature $timelineId "length" "30 min"
set signalIds [AG::CreateSignal $timelineId "test_uri" "test_mimeclass" "test_mimetype" "test_encoding" "test_unit" "test_track"]
set agId [AG::CreateAG $agsetId $timelineId]

set anchor1 [AG::CreateAnchor $agId 10 "sec" $signalIds]
set anchor2 [AG::CreateAnchor $agId 20 "sec" $signalIds]
set anchor3 [AG::CreateAnchor $agId 30 "sec" $signalIds]
set annotation1 [AG::CreateAnnotation $agId $anchor1 $anchor2 "timit"]
set annotation2 [AG::CreateAnnotation $agId $anchor2 $anchor3 "timit"]
set annotation3 [AG::CreateAnnotation $agId $anchor1 $anchor3 "timit"]
AG::SetFeature $annotation1 "sentence" "It's raining."
AG::SetFeature $annotation2 "sentence" "We'll see you around 8 o'clock."

puts "A simple AG with two nodes and three arcs:"
puts [AG::toXML $agId] 
puts "-------------------------"
puts ""

AG::SetAnchorOffset $anchor1 5.0
puts "change the offset of anchor1 to 5.00:"
puts [AG::toXML $agId]
puts "-------------------------"
puts ""

AG::SetStartAnchor $annotation2 $anchor1
puts "change start of annotation2 to anchor1"
puts [AG::toXML $agId]
puts "-------------------------"
puts ""

AG::SetFeature $annotation1 "tense" "Present Continuous"
puts "Set feature \"tense\" of annotation2 to \"Present Continuous\""
puts [AG::toXML $agId]
puts "-------------------------"
puts ""

AG::CopyAnnotation $annotation2
puts "copy annotation2"
puts [AG::toXML $agId]
puts "-------------------------"
puts ""

AG::SplitAnnotation $annotation3
puts "split annotation3"
puts [AG::toXML $agId]
puts "-------------------------"
puts ""

