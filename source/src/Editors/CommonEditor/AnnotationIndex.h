/*
 * AnnotationIndex.h
 *
 *  Created on: 6 mai 2010
 *      Author: plecuyer
 */

#ifndef ANNOTATIONINDEX_H
#define ANNOTATIONINDEX_H

#include <string>
#include <map>
#include <vector>

using namespace std;

namespace tag {

/**
*  @class 		AnnotationIndex
*  @ingroup		AnnotationEditor
*
*  Class used for providing a segment management by cache.\n
*  Represents a time coded segment, and provides a segment comparison operator
*/
class AnnotationIndex
{
	public:
		/**
		 *  @class 		ItemData
		 *  @ingroup	AnnotationEditor
		 *  Item of AnnotationIndex
		 */
		class ItemData {
		public:
			ItemData() : startTime(-1), order(0) {}
			ItemData(float f, int o) : startTime(f), order(o) {};
			ItemData(const ItemData& d) : startTime(d.startTime), order(d.order) {};
			float startTime;
			int order;
		} ;

	public:
		/**
		 * Constructor
		 * @param duration	Duration
		 */
		AnnotationIndex(float duration=-1);

		virtual ~AnnotationIndex();

		/**
		 * add segment to index
		 * @param id		annotation id
		 * @param startTime	Start time
		 * @param order	annotation order
		 */
		void add(const string& id, float startTime, int order);
		/**
		 * delete segment from index
		 * @param id		annotation id
		 */
		void remove(const string& id);
		/**
		 * update segment in index
		 * @param id		annotation id
		 * @param startTime	Start time
		 */
		void update(const string& id, float startTime);

		/**
		 * clear index
		 */
		void clear();

		/**
		 * check if index contains id
		 * @param id 	annotation id
		 * @return true/false
		 */
		bool contains(const string& id) ;

		/**
		 * get start time from id
		 * @param id 	annotation id
		 * @return annotation start time / -1 if not found
		 */
		float getStartTime(const string& id) ;

		/**
		 * get id from start time
		 * @param f 	annotation start time
		 * @return annotation id / "" if not found
		 */
		const string&  getIdAtTime(float f);

		/**
		 * Accessor to index size
		 * @return	Index size (int)
		 */
		int size() { return m_ids.size(); }

		/**
		 * Print all index
		 * @note monitoring purpose
		 */
		void printIndexes() ;

		/**
		 * Print index at given key
		 * @param key	Index key
		 * @note monitoring purpose
		 */
		void printIndex(int key) ;

	private:
		typedef std::map< std::string, ItemData >::iterator id_iterator;
		typedef std::map< int, std::map< long, std::string > >::iterator time_iterator;
		std::map< std::string, ItemData > m_ids;
		std::map< int, std::map< long, std::string > > m_index;
		int m_grain;
		static string _IDNotFound;
};

class AnnotationIndexes
{
	public:
		const map<string, AnnotationIndex>& indexes() const { return m_indexes; }
		AnnotationIndex& operator[] (const string& type) {
			return m_indexes[type];
		}
		map<string, AnnotationIndex>::iterator begin() { return m_indexes.begin(); }
		map<string, AnnotationIndex>::iterator end() { return m_indexes.end(); }
		void clear() { m_indexes.clear(); }
		int size() { return m_indexes.size(); }

	private:
		map<string, AnnotationIndex> m_indexes;
};

} //namespace

#endif /* ANNOTATIONINDEX_H */
