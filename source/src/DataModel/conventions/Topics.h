/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
#ifndef TOPICS_H_
#define TOPICS_H_

#include <list>
#include <set>
#include <glibmm.h>
#include <glibmm/objectbase.h>
#include "Common/CommonXMLReader.h"
#include <iostream>

#define TAG_TOPIC_NULL_LABEL "---"
#define TAG_TOPIC_UNKNOWN_LABEL _("not in database")

namespace tag {

/**
 *  @class TopicDetails
 *  @ingroup DataModel
 *  Annotation topics details
 */

class TopicDetails
{
	public:
		/**
		 * constructor
		 * @param type topic detail type
		 * @param label topic detail label
		 */
		TopicDetails(Glib::ustring type, Glib::ustring label)
			: m_type(type), m_label(label) {} ;
			 /*! destructor */
		~TopicDetails() {}  ;
		Glib::ustring getType() { return m_type ;} /**< get detail type */
		Glib::ustring getLabel() { return m_label ;}  /**< get detail label */
		Glib::ustring getValue() { return m_value ;}  /**< get detail value */
		void setValue(Glib::ustring value) { m_value = value ; }  /**< set detail value */
//		void print() { Log::err() << "DETAILS type= " << m_type << " - label= " << m_label << " - m_value= " << m_value << std::endl ;}
		  /**
		   * print out language definition as xml-formatted string
		   * @param out destination ostream
		   * @param delim string to print between detail items
		   * @return destination ostream
		   */
		std::ostream& toXML(std::ostream& out, const char* delim="") const;

	private:
		Glib::ustring m_type  ;
		Glib::ustring m_label ;
		Glib::ustring m_value ;
};

/**
 *  @class Topic
 *  @ingroup DataModel
 *  Annotation topic definition
 */
class Topic
{
	public :

		/**
		 * constructor
		 * @param id topic i d
		 * @param label topic label
		 * @param group_id topic group id
		 * @param group_label topic group label
		 */
		Topic(Glib::ustring id,Glib::ustring label, Glib::ustring group_id, Glib::ustring group_label)
			: m_id(id), m_label(label), m_group_id(group_id), m_group_label(group_label) {} ;
			 /*! destructor */
		~Topic() {} ;
		Glib::ustring getId() { return m_id ;} /**< get topic id */
		Glib::ustring getLabel() { return m_label ;}  /**< get topic label */
		Glib::ustring getContext() { return m_context ;}  /**< get topic context description */
		Glib::ustring getGroupId() { return m_group_id ;}  /**< get topic group id */
		Glib::ustring getGroupLabel() { return m_group_label ;} /**< get topic group label */
		const std::vector<TopicDetails>& getDetails() { return m_details ;} /**< get topic details */
		void setContext(Glib::ustring value) { m_context = value; } /**< set topic context description */

		/**
		 * add topic detail item
		 * @param type detail item type
		 * @param label detail item label
		 * @return pointer on inserted topic detail item
		 */
		TopicDetails* addDetail(Glib::ustring type, Glib::ustring label) {
			m_details.insert(m_details.begin(), TopicDetails(type,label)) ;
			return &m_details[0] ;
		}

		// DEBUG
//		void print() {
//			Log::err() << "Topic id= " << m_id << " - label= " << m_label
//				<< " - context= " << m_context 	<< " - grouId= " << m_group_id << std::endl ;
//			std::vector<TopicDetails>::iterator it ;
//			for (it=m_details.begin(); it!= m_details.end(); it++) (*it).print() ;
//		}

		  /**
		   * print out language definition as xml-formatted string
		   * @param out destination ostream
		   * @param delim string to print between topic items
		   *
		   * @return destination ostream
		   */
		std::ostream& toXML(std::ostream& out, const char* delim="") const;

	private :
		Glib::ustring m_id ;
		Glib::ustring m_label ;
		Glib::ustring m_context ;
		Glib::ustring m_group_id ;
		Glib::ustring m_group_label ;
		std::vector<TopicDetails> m_details ;
} ;

/**
 *  @class Topics
 *  @ingroup DataModel
 *  Annotation topics dictionary, clustered by topic group
 */
class Topics
{
	public:
		/**
		 * constructor
		 * @param id group id
		 * @param label group label
		 */
		Topics(Glib::ustring id, Glib::ustring label)
			: m_id(id), m_label(label) {} ;

		/*! destructor */
		~Topics() {
			std::map<Glib::ustring, Topic*>::iterator it;
			for (it=m_topics.begin(); it != m_topics.end(); ++it )
				delete it->second;
		}

		Glib::ustring getId() { return m_id ;}  /**< get topic group id */
		Glib::ustring getLabel() { return m_label ;}  /**< get topic group label */

		/**
		 * add topic to current topics group
		 * @param id topic id
		 * @param label topic label
		 * @return pointer on inserted topic
		 */
		Topic* addTopic(Glib::ustring id, Glib::ustring label) {
			m_topics[id] = new Topic(id, label, m_id, m_label) ;
			return m_topics[id] ;
		}

//		void print() ;

		/**
		 * load topics from file
		 * @param in file path
		 * @param l (return) loaded map of topics groups
		 */
		static void loadTopics(Glib::ustring in, std::map<Glib::ustring,Topics*>& l) throw(const char *);
		/**
		 * find topic identified by id in map of topics groups
		 * @param id topic id
		 * @param l map of topics groups
		 * @return pointer on topic found / NULL if not found
		 */
		static Topic* getTopicFromAll(Glib::ustring id, const std::map<Glib::ustring,Topics*>& l) ;
		/**
		 * get topic label for topic identified by id from map of topics groups
		 * @param id topic id
		 * @param l map of topics groups
		 * @return topic label / "" if not found
		 */
		static Glib::ustring getTopicLabel(Glib::ustring id,std::map<Glib::ustring,Topics*>& l) ;
		/**
		 * find topic identified by id in current topics group
		 * @param id topic id
		 * @return pointer on topic found / NULL if not found
		 */
		Topic* getTopic(Glib::ustring id) ;
		/**
		 * get topic label for topic identified by id from current topics group
		 * @param id topic id
		 * @return topic label / "" if not found
		 */
		Glib::ustring getTopicLabel(Glib::ustring id) ;
		/**
		 * @return topics map from current topic group
		 */
		const std::map<Glib::ustring, Topic*>& getAllTopics() { return m_topics ;}

		  /**
		   * print out language definition as xml-formatted string
		   * @param[out] 	out 		destination stream
		   * @param 		delim 		string delimiter between items
		   * @return 					modified ostream
		   */
		std::ostream& toXML(std::ostream& out,  const char* delim="") const;

		/**
		 * print out language definition as xml-formatted string
		 * @param[out] 	out 		destination stream
		 * @param 		topics		Topics in map format
		 * @param 		to_store 	set of topic ids to store
		 * @param 		delim 		string delimiter between items
		 * @return 					modified ostream
		 */
		static std::ostream& toXML(std::ostream& out, const std::map<Glib::ustring,Topics*>& topics, const std::set<Glib::ustring>& to_store, const char* delim="");

	private:
		Glib::ustring m_id ;
		Glib::ustring m_label ;
		std::map<Glib::ustring, Topic*> m_topics ;
};

/**
 *  @class Topics_XMLHandler
 *  @ingroup DataModel
 *  SAX-based XML parsing handler for topics dictionary
 */
class Topics_XMLHandler : public CommonXMLHandler
{
	public:
		/**
		 * constructor
		 * @param l  map of topics groups where to store loaded data
		 */
		Topics_XMLHandler(std::map<Glib::ustring,Topics*>& l) ;
		/*! desctructor */
		~Topics_XMLHandler();

		/**
		 * handler for XML element start
		 * @param uri
		 * @param localname
		 * @param qname
		 * @param attrs
		 */
		void startElement( const   XMLCh* const    uri,
			     const   XMLCh* const    localname,
			     const   XMLCh* const    qname,
			     const   Attributes&     attrs);
		/**
		 * handler for XML element end
		 * @param uri
		 * @param localname
		 * @param qname
		 */
		void endElement( const XMLCh* const uri,
			   const XMLCh* const localname,
			   const XMLCh* const qname);

		/**
		 * handler for XML element character data
		 * @param chars
		 * @param length
		 */
		void characters(const XMLCh* const chars, const unsigned int length);

	private:
		std::map<Glib::ustring,Topics*>& m_topicsGroups;
		XMLCh* m_detailsTag ;
		XMLCh* m_detailTag ;
		XMLCh* m_contextTag ;
		XMLCh* m_topicTag ;
		XMLCh* m_topicgroupTag ;
		Topics* m_current_group ;
		Topic* m_current_topic ;
		TopicDetails* m_current_detail ;
		bool in_context ;
		bool in_detail_value ;
		Glib::ustring current_context ;
		Glib::ustring current_detail_value ;
} ;


} /* namespace tag */

#endif /*TOPICS_H_*/
