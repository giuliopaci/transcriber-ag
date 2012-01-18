/*
 * DataModel_StatHelper.cpp
 *
 *  Created on: 9 juin 2010
 *      Author: lecuyer
 */

#include "DataModel_StatHelper.h"
#include "Common/util/StringOps.h"

namespace tag {

/*========================================================================
 *
 *  Statistics.
 *  get speech-only segments + some properties :
 *    - overlapping_noise : with "noise" events || background
 *    - pron : with "pron" events  (eventually has no text)
 *    - nbwords : nb text words
 *========================================================================*/

bool DataModel_StatHelper::getSpeechSegments(vector<SignalSegment>& v, int notrack,
		bool checkNoise, bool checkPron)
{
	v.clear();

	string graphtype = "transcription_graph";
	vector<string> ids;
	const string& speakerType = m_dataModel.conventions().getSpeakerType(graphtype);
	const string& baseType = m_dataModel.mainstreamBaseType(graphtype);

	m_dataModel.getChilds(ids, speakerType, "", notrack);

	vector<string>::iterator it, itt;

	for (itt = ids.begin(); itt != ids.end(); ++itt)
	{

		if ( m_dataModel.isSpeechSegment(*itt)) {
			vector<string> units;
			int nbwords = 0;

			v.push_back(SignalSegment(*itt, m_dataModel.getStartOffset(*itt), m_dataModel.getEndOffset(*itt), m_dataModel.getElementSignalTrack(*itt)));
			SignalSegment& s = v.back();
			s.setOrder(m_dataModel.getOrder(*itt));
			s.setProperty("pronounce", "0");
			s.setProperty("overlapping_noise", "0");
			m_dataModel.getChilds(units, baseType, *itt, notrack);

			for (it = units.begin(); it != units.end(); ++it)
			{

//				if (checkPron)
//				{
//
//					// check for pronounce problems
//					v2.clear();
//					m_dataModel.getLinkedElements(it->getId(), v2, "pronounce");
//					if (v2.size() > 0)
//					{
//						it->setProperty("pronounce", "1");
//					}
//					else
//						it->setProperty("pronounce", "0");
//				}
//				else
//
//				if ( m_dataModel.isSpeechSegment(it->getId()) )
//					it->setProperty("overlapping_noise", (checkNoise
//							&& getOverlappingNoise(*it) ? "1" : "0"));

				// compute nb words
				if (m_dataModel.getElementProperty(*it, "subtype") == "unit_text" ) {
						const string& text = m_dataModel.getElementProperty(*it, "value");
						nbwords += StringOps(text).getTokenCount();
				}
			}
			s.setProperty("nbwords", StringOps().fromInt(nbwords));
		}
	}
	return true;
}

/**
 * check if there is overlapping noise over given segments
 * @param s signal segment
 * @return true/false
 *
 * @note :
 *  overlapping noise may be :
 *    - a background segment overlapping current signal segment
 *       - any segment if config parameter "stats,is_noise,low_level" is true,
 *       - else only "level=high" background segments
 *    - a "noise" type event attached to current signal segment :
 *      - any noise event if config parameter "stats,is_noise,instantaneous" is true,
 - else only non-instantaneous noise events.
 */

bool DataModel_StatHelper::getOverlappingNoise(const SignalSegment& s)
{
	vector<string> v3;
	set<string> v2;
	set<string>::iterator it2;
	vector<string>::iterator it3;
	bool with_noise = false;
	bool instant_is_noise = (m_dataModel.conventions().getConfiguration("stats,is_noise,instantaneous")
			== "true");
	bool low_is_noise = (m_dataModel.conventions().getConfiguration("stats,is_noise,low_level") == "true");

	v2.clear();
	with_noise = false;
	string id = s.getId() ;
	m_dataModel.getLinkedElements(id, v3, "noise");
	if (instant_is_noise)
		with_noise = (v3.size() > 0);
	else
		for (it3 = v3.begin(); !with_noise && it3 != v3.end(); ++it3)
			if (m_dataModel.getStartAnchor(*it3) != m_dataModel.getEndAnchor(*it3) )
				with_noise = true;
	if ( !with_noise)
	{
		// check if overlapping background noise
		m_dataModel.getOverlappingSegmentsIds(s, v2, "background");
		if (low_is_noise)
			with_noise = (v2.size() > 0 );
		else
			for (it2 = v2.begin(); !with_noise && it2 != v2.end(); ++it2)
				if (m_dataModel.getElementProperty(*it2, "level") == "high")
					with_noise = true;
	}

	return with_noise;
}

/**
 return speech duration for given speaker id
 */
float DataModel_StatHelper::getSpeechDuration(const string& spkid, int& notrack)
{
	const list<string>& ids = getSpeakerTurns(spkid, notrack);
	list<string>::const_iterator it;
	float dur = 0.0;

	if ( ids.size() > 0 ) {
		for ( it=ids.begin(); it != ids.end(); ++it) {
			// turns are always anchored -> use their start / end offsets
			dur += (m_dataModel.getEndOffset(*it) - m_dataModel.getStartOffset(*it));
		}
		notrack = m_dataModel.getAnchorSignalTrack(m_dataModel.getStartAnchor(ids.front()));
	}
	return dur;
}

/**
 returns ids of all turns to which given speaker is associated, ordered by their start timecode
 */
list<string> DataModel_StatHelper::getSpeakerTurns(const string& spkid, int notrack)
{
	list<string> res;
	string graphtype = "transcription_graph";
	const string& type = m_dataModel.conventions().getTypeFromFeature("speaker", graphtype);

	if ( ! type.empty() && m_dataModel.hasAnnotationGraph(graphtype) ) {
		const string& agId = m_dataModel.getAGTrans();

		const set<AnnotationId>& ids = GetAnnotationSetByFeature(agId,
				"speaker", spkid, type);
		set<AnnotationId>::const_iterator its;
		map<float, string> v;
		map<float, string>::iterator itv;

		for (its = ids.begin(); its != ids.end(); ++its)
			v[GetStartOffset(*its)] = *its;

		for (itv = v.begin(); itv != v.end(); ++itv)
			res.push_back(itv->second);
	}
	return res;
}

}

