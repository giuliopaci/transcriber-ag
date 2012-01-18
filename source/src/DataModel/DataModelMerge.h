/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/*
 * $Id$
 */


#include "DataModel/DataModel.h"

namespace tag {
/**
 *  @class DataModelMerge
 *  @ingroup DataModel
 *  implements data models merge operations
 **/

class DataModelMerge {
public:

	enum MergeType {
		merge_append,
		merge_replace_if_exists,
		merge_keep_if_exists
	};

	/**
	 * merge 2 data models
	 * @param dest destination data model
	 * @param merged merged data model
	 * @param merge_type merge type
	 * @param merge_offset signal offset to apply on merged data model before merging
	 * @return true if merge completed, else false
	 */
	bool merge(DataModel& dest, DataModel& merged, MergeType merge_type, float merge_offset);

private:

	void mergeMetaData(DataModel& dest, DataModel& merged, MergeType merge_type);
	void mergeSpeakers(DataModel& dest, DataModel& merged, MergeType merge_type);
	void mergeSignals(DataModel& dest, DataModel& merged, MergeType merge_type);
	void mergeAnnotations(DataModel& dest, DataModel& merged, MergeType merge_type);
	/**
	 * add full graph to current set
	 */
	void addGraph(DataModel& dest, DataModel& merged, const string& graphtype, const string& agid);

private:
	int m_trackstart;

};
}
