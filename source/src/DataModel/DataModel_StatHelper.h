/*
 * DataModel_StatHelper.h
 *
 *  Created on: 24 mai 2010
 *      Author: lecuyer
 */

#ifndef DATAMODEL_STATHELPER_H_
#define DATAMODEL_STATHELPER_H_

#include <set>
#include <vector>
#include <string>
using namespace std;
#include "DataModel/DataModel.h"

namespace tag {

/**
 * @class 	DataModel_StatHelper
 * @ingroup DataModel
 * DataModel statistic helper
 */
class DataModel_StatHelper {

public:

	/**
	 * constructor
	 * @param data target data model
	 */
	DataModel_StatHelper(DataModel& data) : m_dataModel(data) {};

	/**
	 * Accessor to DataModel used
	 * @return  DataModel reference
	 */
	DataModel& getData() { return m_dataModel; }


	/*!   */
	bool getSpeechSegments(vector<SignalSegment>& v, int notrack=-1, bool checkNoise=false, bool checkPron=false);
	/*!   */
	bool getOverlappingNoise(const SignalSegment& s);
	/**
	 * return speech duration for given speaker id
	 * @param spkid speaker id
	 * @param notrack target signal track (-1 for any track); will be set to actual track on which speaker is found.
	 * @return total speech duration for speaker on given track
	 */
	float getSpeechDuration(const string& spkid, int& notrack);
	/**
	 * returns the ids of all turns to which given speaker is associated, ordered by their start timecode
	 * @param spkid speaker id
	 * @param notrack target notrack / -1 for all tracks
	 * @return turn ids list
	 */
	list<string> getSpeakerTurns(const string& spkid, int notrack=-1);
	//@}

private:

	DataModel& m_dataModel;
};

};	// namespace tag


#endif /* DATAMODEL_STATHELPER_H_ */
