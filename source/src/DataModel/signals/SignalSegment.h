/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#ifndef _HAVE_SEGMENT_H
#define _HAVE_SEGMENT_H

#include <string>
#include <sstream>
#include <map>

namespace tag {

/**
*  @class SignalSegment
*  @ingroup DataModel
*  Signal segment properties.
*  <br>A signal segment describes a signal portion delimited by 2 signal-anchored nodes;
*/
class SignalSegment
{
public:
	/**
	 * Constructor
	 * @param id segment id / "" for default constructor
	 */
	SignalSegment(const std::string& id="")
		: m_id(id), m_label(id), m_startOffset(0.0), m_endOffset(0.0), m_track(0), m_order(0),
		m_isSpeech(false) { m_endId = id; }
	/**
	 * Constructor
	 * @param id segment id
	 * @param start start offset in signal (in seconds)
	 * @param stop stop offset in signal (in seconds)
	 * @param notrack signal track no
	 */
	SignalSegment(const std::string& id, float start, float stop, int notrack=0)
			: m_id(id), m_label(id), m_startOffset(start), m_endOffset(stop), m_track(notrack), m_order(notrack),
		m_isSpeech(false) { m_endId = id; }
	/*! destructor */
	virtual ~SignalSegment() {}

	/**
	 * get segment id
	 * @return segment id
	 */
	const std::string& getId() const { return m_id; }
	/**
	 * set segment id
	 * @param id segment id
	 */
	void setId(const std::string& id) { m_id = id; }
	/**
	 * get segment end id
	 * @return end id
	 */
	const std::string& getEndId() const { return m_endId; }
	/**
	 * set segment end id
	 * @param id end id
	 */
	void setEndId(const std::string& id) { m_endId = id; }
	/**
	 * get segment label
	 * @return segment label
	 */
	const std::string& getLabel() const { return m_label; }	/**
	 * set segment label
	 * @param label segment label
	 */
	void setLabel(const std::string& label) { m_label = label; }
	/**
	 * ret segment start offset
	 * @return start offset (in seconds)
	 */
	float getStartOffset() const { return m_startOffset; }
	/**
	 * set segment start offset
	 * @param f start offset (in seconds)
	 */
	void setStartOffset(float f) { m_startOffset = f; }
	/**
	 * get segment end offset
	 * @return end offset (in seconds)
	 */
	float getEndOffset() const { return m_endOffset; }
	/**
	 * set segment end offset
	 * @param f end offset (in seconds)
	 */
	void setEndOffset(float f) { m_endOffset = f; }
	/**
	 * ret segment signal track no
	 * @return signal track no
	 */
	int getTrack() const { return m_track; }
	/**
	 * set segment signal track no
	 * @param notrack signal track no
	 */
	void setTrack(int notrack) { m_track = notrack; }
	/**
	 * get segment order (overlapping segments)
	 * @return segment order
	 */
	int getOrder() const { return m_order; }
	/**
	 * set segment order (overlapping segments)
	 * @param order segment order
	 */
	void setOrder(int order) { m_order = order; }

	/**
	 * return segment property value
	 * @param name target property name
	 * @return property value
	 */
	std::string getProperty (const std::string& name) const throw (const char*);
	/**
	 * check if segment has a given property
	 * @param name property name
	 * @return true if has property, else false
	 */
	bool hasProperty (const std::string& name) const throw (const char*);
	/**
	 * set segment property
	 * @param name property name
	 * @param value property value
	 */
	void setProperty (const std::string& name, const std::string& value) throw (const char*);

	/**
	 * comparison operator
	 * @param s signal segment to compare with
	 * @return true if current segment is lower than s, else false
	 */
	bool operator< (const SignalSegment& s) const ;

private:
	std::string m_id;
	std::string m_label;
	float m_startOffset;
	float m_endOffset;
	int m_track;
	int m_order;
	bool m_isSpeech;
	std::string m_endId;  // id of end-anchored annotation for segment.

	std::map<std::string, std::string> m_properties;

protected:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
	virtual bool validPropertyName(const std::string& name) const { return false; }
	virtual const char* getSegmentClass() const { return "SignalSegment"; }
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

};

}

#endif /* _HAVE_SEGMENT_H */
