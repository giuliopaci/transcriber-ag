/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/


#ifndef DATAMODEL_CPHELPER_H_
#define DATAMODEL_CPHELPER_H_

#include <set>
#include <vector>
#include <string>
using namespace std;

namespace tag {


class DataModel;

/**
 * @class DataModel_CPHelper
 * @ingroup DataModel
 * DataModel Copy/Paste operations helper
 */
class DataModel_CPHelper
{
public:

	/**
	 * @class CopyFilter
	 * @ingroup DataModel
	 * DataModel Copy/Paste operations helper
	 */
	class CopyFilter
	{
		public:
			/**
			 * Constructor
			 */
			CopyFilter() : allowAnyQualifier(false), copySpeakers(false), startId(""), startOffset(0), stopId(""), stopOffset(0)  {}
			set<string> copyTypes;		/**< types to be copied */
			bool allowAnyQualifier;		/**< if true, allow any qualifier whatever copyTypes value is */
			bool copySpeakers;			/**< copy speakers to destination graph speaker dictionary */
			string startId;				/**< start element Id */
			int startOffset;			/**< text offset for copy start */
			string stopId;				/**< stop element Id */
			int stopOffset;				/**< text offset for copy stop */
		};

	/**
	 * constructor
	 * @param data target data model
	 */
	DataModel_CPHelper(DataModel* data) : m_data(data) {};
	/**
	 * destructor
	 */
	~DataModel_CPHelper();


	/**
	 * insert subgraph in data model at given graph position
	 * @param whereId 			Annotation id after which new graph is to be inserted
	 * @param whereOffset 		Offset in whereId where subgraph is to be inserted
	 * @param signalOffset		Position in signal where inserting subgraph (-1 for insertion without using signal position)
	 * @param agId 				Inserted subgraph AG id
	 * @param doShiftOffsets 	If true, add whereId signal offset to all inserted signal-anchored nodes before insertion takes place
	 * @param emitSignal 		True to emit signalModelUpdated
	 * @param err				Error message if insertion failed
	 * @throw msg 				AG Exception mesage for strong failure
	 * @note  If whereId is a text-type annotation, whereOffset defines an offset into associated text feature.
	 */
	bool insertSubgraph(string whereId, int whereOffset, float signalOffset, const string& agId, bool doShiftOffsets, bool emitSignal, string& err) throw (const char*) ;

	/**
	 * extract subgraph starting and ending at given ids (+ eventual offsets)
	 * @param startId	start annotation id
	 * @param startOffset offset in start annotation
	 * @param stopId	stop annotation id
	 * @param stopOffset	offset in stop annotation
	 * @param basetype_only	if true (default), keep only base type annotations and lower precedence annotations (qualifiers) .
	 * @param in_convention_only	if true (default), keep only annotations types defined in current annotation conventions.
	 * @return	subgraph AG id
	 * @note if startId and stopId are text-type annotations, startOffset and stopOffset define an offset into associated text feature. If
	 */
	string getSubgraph(string startId, int startOffset, string stopId, int stopOffset, bool basetype_only=false, bool in_convention_only=false);

	/**
	 * extract subgraph starting and ending at given ids as XML buffer following TransAG DTD
	 * @param startId	start annotation id
	 * @param startOffset offset in start annotation
	 * @param stopId	stop annotation id
	 * @param stopOffset	offset in stop annotation
	 * @param basetype_only	if true (default), keep only base type annotations and lower precedence annotations (qualifiers) .
	 * @param in_convention_only	if true (default), keep only annotations types defined in current annotation conventions.
	 * @return XML string buffer
	 * @note if startId and stopId are text-type annotations, startOffset and stopOffset define an offset into associated text feature. If
	 */

	const string& getSubgraphTAGBuffer(string startId, int startOffset, string stopId, int stopOffset, bool basetype_only=false, bool in_convention_only=false);

	/**
	 * extract text annotations from XML string buffer following TransAG DTD
	 * @param buffer XML string buffer
	 * @return subgraph id
	 */
	/**
	 * xtract text annotations from XML representation following TransAG DTD
	 * @param buffer XML string buffer containing AG graph representation
	 * @param atype	annotation type to extract from (defaults to "segment")
	 * @param aprop	annotation property to extract from (defaults to "value")
	 * @return plain text string holding concatenated annotations properties
	 * @note this method is only a wrapper around getGraphFromTAGBuffer and getTextFromGraph methods
	 */
	const string& getTextFromTAGBuffer(const string& buffer, const string& atype="segment", const string& aprop="value");

	/**
	 * build AG subgraph from XML representation following TransAG DTD
	 * @param buffer XML string buffer
	 * @return subgraph AG id
	 */
	string getGraphFromTAGBuffer(const string& buffer);

	/**
	 * extract text annotations graph
	 * @param agId graph AG id
	 * @param atype	annotation type to extract from (defaults to "segment")
	 * @param aprop	annotation property to extract from (defaults to "value")
	 * @return plain text string holding concatenated annotations properties
	 * @note annotations with type atype must constitute a continuous graph
	 */
	const string& getTextFromGraph(const string& agId, const string& atype="segment", const string& aprop="value");

	/**
	 * Copies a graph
	 * @param destGraphId		Destination graph
	 * @param start_anchor		Anchor for copy start
	 * @param end_anchor		Anchor for copy end
	 * @param filter			CopyFiler reference
	 * @param unanchoredStart	Specify whether the offset of start anchor should be used if anchored.
	 */
	void copyGraph(const string& destGraphId, const string& start_anchor, const string& end_anchor, CopyFilter& filter, bool unanchoredStart=false);


private:
	string createAnchorCopy(const string& agId, const string& aid, set<string>& sigIds, float offsetBase, bool use_offset=true, bool addOffset=false);
	/*! check if 2 nodes can be merged */
	bool getCanMerge(const string& baseId, const string& mergeId);
	/*! check if given annotation is "empty"  (eg no transcription text )  */
	bool isEmptyAnnotation(const string& id);
	void eventuallyMergeAnchors(const string& base, const string& toMerge);
	/*  get 1st element with given type attached at given anchor */
	string getElementAtAnchor(const string& aid, const string& type, bool outgoing=true);

	string lastLinkedTo(const string& id);
	string firstWithType(const string& graph_start, const string& type, int order=0);


private:

	class LocalUpdate {
	public:
		string type;
		string id;
		DataModel::UpdateType upd;
		LocalUpdate(const string& t, const string& i, DataModel::UpdateType u) : type(t), id(i), upd(u) {};
	};

	DataModel* m_data;
	string m_textBuffer;
	string m_graphType;
	string m_baseType;
	vector<string> m_agsetIds;
	vector< LocalUpdate > m_updates;

};

};

#endif /* DATAMODEL_CPHELPER_H_ */
