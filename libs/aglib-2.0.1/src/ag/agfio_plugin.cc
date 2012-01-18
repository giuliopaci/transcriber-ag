#include <ag/agfio_plugin.h>
#include <ag/AGAPI.h>
#include <ag/RE.h>

// automatically identify AGSet, TL, SIG, AG
// create objects if needed
void
agfio_plugin::auto_init(const Id& id,
			map<string,string>* signalInfo,
			AGSetId& agsetId,
			TimelineId& timelineId,
			SignalId& signalId,
			AGId& agId)
  throw (const string&)
{
  // id could be an AGSet or AG id
  static RE idre("^ *(([^: ]+)(:[^: ]+)?) *$");

  if (!idre.match(id))
    throw string("agfio_plugin::idcheck():bad AG/AGSet id: ") + id;

  agsetId = idre.get_matched(2);
  agId = idre.get_matched(3).empty() ? "" : idre.get_matched(1);

  if (! ExistsAGSet(agsetId))
    CreateAGSet(agsetId);

//  if (!agId.empty() && ExistsAG(agId))
//    cerr << "WARNING: AG of id " << agId << " already exists." << endl
//       << "  Continue to load the annotation into this AG anyway." << endl;

  ////////////////////////////////////
  /// get Timeline, Signal, and AG ///
  ////////////////////////////////////

  bool new_AG;

  if (agId.empty() || !ExistsAG(agId))
    new_AG = true;
  else
    new_AG = false;

  // get Timeline
  if (new_AG)
    timelineId = CreateTimeline(agsetId);
  else
    timelineId = GetTimelineId(agId);


  // get Signal
  signalId.erase();

  if (signalInfo) {
    if (new_AG) {
      string& uri       = (*signalInfo)["uri"];
      string& mimeClass = (*signalInfo)["mimeClass"];
      string& mimeType  = (*signalInfo)["mimeType"];
      string& encoding  = (*signalInfo)["encoding"];
      string& unit      = (*signalInfo)["unit"];
      if (uri.empty()) uri = "uri";
      if (mimeClass.empty()) mimeClass = "mimeClass";
      if (mimeType.empty()) mimeType = "mimeType";
      if (encoding.empty()) encoding = "encoding";
      if (unit.empty()) unit = "unit";
      signalId = CreateSignal(timelineId,
			      uri, mimeClass, mimeType, encoding, unit,
                              (*signalInfo)["track"]);
    }
    else {
      // find an identical(hopefully) Signal
      URI key = (*signalInfo)["uri"];
      set<SignalId> ids = GetSignals(timelineId);
      for (set<SignalId>::iterator id=ids.begin(); id != ids.end(); ++id) {
	if (GetSignalXlinkHref(*id) == key) {
	  signalId = *id;
	  break;
	}
      }
    }
  }

  // get an AG
  if (new_AG) {
    if (agId.empty())
      agId = CreateAG(agsetId, timelineId);
    else
      CreateAG(agId, timelineId);
  }
}



