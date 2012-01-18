/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 * @defgroup DataModel DataModel
 */


#ifndef _HAVE_DATA_MODEL_H
#define _HAVE_DATA_MODEL_H 1

#include <string>
#include <vector>
#include <set>
#include <list>
#include <glibmm.h>
#include <math.h>
#include <glibmm/objectbase.h>


#include "DataModel/speakers/SpeakerDictionary.h"
#include "DataModel/signals/SignalConfiguration.h"
#include "DataModel/signals/SignalSegment.h"
#include "DataModel/versions/VersionList.h"
#include "DataModel/anchorLinks/AnchorLinks.h"
#include "DataModel/conventions/ModelChecker.h"
#include "DataModel/conventions/Conventions.h"

#include "Common/Encoding.h"
#include "Common/Parameters.h"
#include "Common/util/Log.h"

#include <ag/AGAPI.h>

using namespace std;

extern const utils::Encoding* utf8;

/**
 * @namespace	tag
 * Main namespace of TranscriberAG
 */

namespace tag {


#define TRANS_ANCHOR_UNIT "sec"
#define UNDEFINED_LENGTH 9999999.0


/**
 *  @class DataModel
 *  @ingroup DataModel
 *  data model for transcriberAG <br>
 *	acts as a wrapper for AGlib-based data structure manipulation
 *
 *	@note a DataModel corresponds to an AGSet in AGLIB terminology; this AGSet is defined for the annotation of a given signal
 * (or set of synchronized signals), and may hold one or more annotation graphs (an AG in AGLIB terminology), with respect to
 * selected annotation conventions (eg a transcription graph and a background noise graph).
 * The Data model AGSet can be saved to / loaded from a file.
 */

class DataModel : public Glib::ObjectBase
{
	public:

		/**< Data model action type */
		typedef enum { INSERTED, UPDATED, RESIZED, MOVED, DELETED, SPLITTED, REDO_DELETED, ANCHORED, UNANCHORED} UpdateType ;

		/**< Small value used as reference for timecode gaps */
		static const float EPSILON;

		/**< model internal value */
		static const float USER_INPUT;

		class CompareIdsByOffset : public binary_function < const string&, const string&, bool >
		{
		    private :
		        DataModel& m_data;
		    public:
		        CompareIdsByOffset(DataModel& data) : m_data(data) {}
		        bool operator() (const string& id1, const string& id2) {
		        	const string& a1 = m_data.getStartAnchor(id1);
		        	const string& a2 = m_data.getStartAnchor(id2);
		        	if ( a1 == a2 ) {
		        		return (m_data.getOrder(id1) < m_data.getOrder(id2));
		        	}
		        	float diff = m_data.getAnchorOffset(a1) - m_data.getAnchorOffset(a2);
		        	if ( fabs(diff) < DataModel::EPSILON ) {
		        		return (m_data.getAnchorSignalTrack(a1) < m_data.getAnchorSignalTrack(a2));
		        	}
		        	return (diff < 0);
		        }
		};

	public:

		/*! @name data model initialization	 */
		//@{
		/**
		 * default constructor - does not initialize an AGSet
		 */
		DataModel();
		/**
		 * alternate constructor - initializes an AGSet for given corpus name.
		 * @param corpus_name corpus name
		 */
		DataModel(const string& corpus_name);
		/**
		 * destructor
		 */
		~DataModel();

		/**
		 * initialize DataModel structures from loaded AG data
		 * @param import_format 	original import format
		 * @param loaded_agids  	loaded AG data
		 * @param fullmode			True for full loading, false for loading only model data
		 * @param convention_name	Conventions to be applied
		 */
		bool initFromLoadedAG(const string& import_format, const list<AGId>& loaded_agids, bool fullmode=false, const string& convention_name="");

		/**
		 * Activates the cheching data model module.\n
		 * @see					ModelChecker class
		 * @param activate		If true, the dataModel will rely on a checking module for loading graph.
		 * 						If false, no check will be proceeded. If false and a checker is already existing,
		 * 						it will be cleaned and destructed.
		 */
		void activateModelChecker(bool activate) ;

		/**
		 * Indicates whether the data model is using its checker module
		 * @return				True or False
		 */
		bool getModelCheckerStatus() { return (m_checker!=NULL); }

		/**
		 * Accessor to the model checker
		 * @return			Reference on the checker module (NULL is no check activated)
		 */
		ModelChecker* getModelChecker() { return m_checker ;}

		/**
		 * Reset model checker
		 */
		void cleanModelChecker() {
			if (m_checker)
			{ m_checker->clear() ; delete(m_checker) ; m_checker=NULL; }
		}

		/**
		 * inhibate / activate validation checks when updating model (eg qualifier span rules)
		 */
		void setInhibateChecking(bool b) {
			m_inhibateChecking = b;
		}

		/**
		 * sets applicable annotation conventions for current DataModel instance
		 * @param convention	convention name
		 * @param lang	language for annotation labels display
		 * @param fullmode if true (default), will load all files associated to given convention, including annotations label formats, topics list, ..
		 * if false, load only convention specification file (faster).
		 */
		void setConventions(string convention="", string lang="fre", bool fullmode=true);

		/**
		 * initialize AGSet for current Data Model
		 * @param corpus_name current corpus name
		 */
		void initAGSet(const string& corpus_name="TransAG");

		/**
		 * initialize annotation graph for current annotation conventions
		 * @param lang	transcription language
		 * @param scribe	current user id
		 * @param graphtype		graph type to be initialized / "" to initialize all graph types defined in conventions
		 * @param reset_graphs	if true, reset eventual existing graphs
		 * @note new graphs will contain 2 anchors, anchored at annotated signal start and end, and one annotation
		 * for each mainstream type attached to those nodes.
		 */
		void initAnnotationGraphs(const string& graphtype="", const string& lang="", const string& scribe="", bool reset_graphs=false);

		/**
		*  clear all graphs stored in data model, releasing all associated memory
		*/
		void clear() { closing = true ; deleteAGElements(); closing = false ;}

		//@}



		/*! @name Data model global descriptors and versions info  */
		//@{
		/**
		 * set current data model corpus name
		 * @param name corpus name
		 */
		void setCorpusName(const string& name) ;
		/**
		 * @return corpus name
		 */
		string getCorpusName() ;
		/**
		 * set current data model corpus version
		 * @param version corpus version
		 */
		void setCorpusVersion(const string& version) ;
		/**
		 * @return corpus version
		 */
		string getCorpusVersion() ;
		/**
		 * @return current conventions name
		 */
		const string& getConventionsName() { return m_conventions.name(); }

		/**
		 * @return map of existing graph types and associated ids
		 */
		const map<string, string>& getGraphs() { return m_agIds; }
		/**de
		 * @return current AGSet id
		 */
		const string& getAGPrefix() { return m_agsetId; }
		/**
		 * @return transcription graph id
		 */
		const string& getAGTrans() { return m_agIds["transcription_graph"]; }
		/**
		 * @param type graph type
		 * @return graph id for given graph type / "" if non-existent graph
		 */
		const string& getAG(const string& type) { return m_agIds[type]; }
		/**
		 * get AGSet property value
		 * @param item property item name
		 * @param defval default value if not set
		 * @return property item value
		 */
		string getAGSetProperty(const string& item, const string& defval="");
		/**
		 * get all properties attached to current AGSet
		 * @return map of (label, values) properties
		 */
		map<string, string> getAGSetProperties();
		/**
		 * set AGSet property value
		 * @param item property item name
		 * @param value property value
		 */
		void setAGSetProperty(const string& item, const string& value);
		/**
		 * Deletes feature from AGSet element
		 * @param item	 Feature to delete
		 */
		void unsetAGSetProperty(const string& item) ;
		/**
		 * set graph property value
		 * @param graphtype graph type
		 * @param item property item name
		 * @param value property value
		 */
		void setGraphProperty(const string& graphtype, const string& item, string value);
		/**
		 * Deletes graph property feature.
		 * @param graphtype 		Graph type
		 * @param item 				Property item name
		 */
		void unsetGraphProperty(const string& graphtype, const string& item) ;
		/**
		 * get graph property value
		 * @param graphtype graph type
		 * @param item property item name
		 * @param defval default value if not set
		 * @return property item value
		 */
		string getGraphProperty(const string& graphtype, string item, string defval="");

		/**
		 * Checks the existence of the given property for the given graph
		 * @param graphtype 	Graph type
		 * @param item 			Property item name
		 * @return				True if it exists, False otherwise
		 */
		bool existGraphProperty(const string& graphtype, string item) ;

		/**
		 * get all properties attached to given graph
		 * @param graphtype graph type
		 * @return map of (label, values) properties
		 */
		map<string, string> getGraphProperties(const string& graphtype);

		/**
		 *
		 * @param type
		 * @return true if has annotation graph of given type
		 */
		bool hasAnnotationGraph(const string& type) { return (m_agIds.find(type) != m_agIds.end()); }
		/*! wrapper for setGraphProperty for transcription graph */
		void unsetTranscriptionProperty(string item) { unsetGraphProperty("transcription_graph", item); }
		/*! wrapper for setGraphProperty for transcription graph */
		void setTranscriptionProperty(string item, string value) { setGraphProperty("transcription_graph", item, value); }
		/*! wrapper for getGraphProperty for transcription graph */
		string getTranscriptionProperty(string item, string defval="") { return getGraphProperty("transcription_graph", item, defval); }
		/*! wrapper for getTranscriptionProperties for transcription graph */
		map<string, string> getGraphProperties() { return getGraphProperties("transcription_graph"); }
		/**
		 * get main transcription language  for transcription graph
		 * <br>the case of a multi-signal track with different language for each track is handled
		 * @param notrack track no (default to 0)
		 * @return transcription language (iso639-2 3-letter code)
		 */
		const string& getTranscriptionLanguage(int notrack=0);
		/**
		 * set main transcription language  for transcription graph
		 * @param lang transcription language (iso639-2 3-letter code)
		 */
		void setTranscriptionLanguage(string lang) { setAGSetProperty("lang", lang); }
		/**
		 * get dialect for transcription graph
		 * @return transcription dialect
		 */
		string getTranscriptionDialect() { return getGraphProperty("transcription_graph","dialect"); }
		/**
		 * set dialect for transcription graph  (main language must be set before)
		 * @param dialect transcription dialect
		 */
		void setTranscriptionDialect(string dialect) { setGraphProperty("transcription_graph","dialect", dialect); }

		/**
		 * initialize data model versions information
		 * @param scribe	current annotator name
		 * @param date	current date
		 * @param comment	version comment
		 */
		void initVersionInfo(const std::string& scribe, const std::string& date="", const string& comment="");
		/**
		 * update current version information
		 * @param scribe current annotator name
		 * @param wid	version info
		 */
		void updateVersionInfo(const std::string& scribe, const std::string& wid="");
		/**
		 * @return data model versions list
		 */
		VersionList& getVersionList() { return m_versions; }
		/**
		 * replace data model version list
		 * @param l new  versions list
		 */
		void setVersionList(const VersionList& l) { m_versions = l; }

		//@}


		/*! @name Data model I/O  */
		//@{
		/**
		*  load data model from annotation file
		* @param path 			Annotation file path
		* @param format 		Annotation file format - must be valid and can be checked first with guessFileFormat (default to "TransAG")
		* @param fullmode 		If true, will fully load conventions (including related items such as topics and qualifiers labels layout), else will only load required items) (default to true)
		* @param reset_agId 	If true, will reload temporary agId previously set in order to enable multi agSet using (default to false)
		* @throw errmsg 		Error message if load operation failed
		* @return				True if loading complete, False otherwise
		* @note Fullmode is generally set to true when loading occurs from annotation editor, else set to false
		*
		*/
		bool loadFromFile(const string& path, const string& format="TransAG", bool fullmode=true, bool reset_agId=false) throw (const char*);

		/**
		*  save data model to annotation file
		* @param path annotation file path
		* @param format annotation file format (default to "TransAG")
		* @param cleanup_unused if true, cleanup unused speakers from speakers dictionnary before saving file (default to true)
		* @throw errmsg error message if save operation failed
		*/
		void saveToFile(const string& path, const string& format="TransAG", bool cleanup_unused=true ) throw (const char*);

		/**
		*  save data model to annotation file
		* @param options export options
		* @param path annotation file path
		* @param format annotation file format (default to "TransAG")
		* @param cleanup_unused if true, cleanup unused speakers from speakers dictionnary before saving file (default to true)
		* @throw errmsg error message if save operation failed
		*/
		void saveToFileWithOptions(const std::map<string,string>& options, const string& path, const string& format="TransAG", bool cleanup_unused=true ) throw (const char*);

		/**
		*  guess annotation file format
		* @param path annotation file path
		* @param file_dtd (returned) address of string in which dtd name will be returned if file requires one.
		* @return	guessed file format, that can be later on passed to loadFromFile.
		* @throw errmsg error message if save operation failed
		*/
		string guessFileFormat(const string& path, string* file_dtd=NULL) throw (const char*);
		/**
		 * check if format name is the TranscriberAG file format
		 * @param format name
		 * @return true if tested format name is the TranscriberAG file format, else false
		 */
		bool isTAGformat(const string& format) ;

		/**
		* get file path associated to current data model
		* @return file path / "" if no data model not loaded from file and never saved to file
		*/
		const string& getPath() { return m_path; }

		/**
		 * add option to option map that will be passed to agfio plugin when loading file
		 * @param key					option key
		 * @param value					option value
		 * @param replaceIfExists		If true and if the given key already exists, then replaces the existing value
		 */
		void addAGOption(string key, string value, bool replaceIfExists=true);

		/**
		 * Used for formats import : determines the convention to be used for specific import
		 * @param format		Import format name
		 * @return 				Convention file path
		 * @todo 				To move to configurator object
		 */
		string getConversionConventionFile(const string& format) ;

		/**
		 * Used for formats import : determines the convention to be used for specific import from a
		 * a specific configuration set.
		 * @param format				Import format name
		 * @param qualifierMapping		Configuration mapping set
		 * @return 						Convention file path
		 * @remarks						If you want to use the default configuration set, prefer the
		 * 								string getConversionConventionFile(const string&) method
		 * @todo 						To move to configurator object
		 */
		string getConversionConventionFile(const string& format, Parameters* qualifierMapping) ;

		/**
		 * load conventions definition fro file format
		 * @param format
		 * @return conventions definition map
		 */
		Parameters* getConventionsFromFormat(const string& format) ;

		/**
		 * @return imported file format
		 */
		Glib::ustring getLoadedFileFormat() { return m_import_format; }

		/**
		 * Indicates whether the file loaded in the model is a TAG format
		 * @return		True or false
		 */
		bool isLoadedTagFormat() { return isTAGformat(m_import_format); }

		/* TODO : messaging mechanism should be homogeneized in next code release */
		void setImportWarnings(vector<string> warnings)  ;
		//@}


		 /**
		  *  @name Annotated signal management
		  *  @note in TranscriberAG data model, a distinct SignalId (in AGLIB terminology) is associated to each signal track
		  */
		 //@{
		/**
		 * get current annotated signals configuration
		 * @return current signal configuration descriptor
		 */
		SignalConfiguration& getSignalCfg() { return signalCfg ; }

		/**
		 * If set to true, indicates that a multi channel signal will be treated as a mono signal
		 * @param single	True or False
		 */
		void setSingleSignal(bool single) ;

		/**
		 * return signal file url for given track
		 * @param notrack
		 * @return signal file url
		 */
		string getSignalFileURL(int notrack=0);
		/**
		 * return annotated signal files names for given signal mime class (audio / video / ..)
		 * @param mimeclass	signal type
		 * @return map holding (SignalId, signal name) pairs
		 */
		std::map<string,string> getSignalFileNames(string mimeclass) ;
		/**
		 * get annotated signal files path
		 * @return map holding (SignalId, signal file path) pairs
		 */
		std::map<string,string> getSignalFilePaths() ;
		/**
		 * get Signal id for given signal track no
		 * @param notrack track no
		 * @return signal id
		 */
		string getSignalId(int notrack=0);
		/**
		 * get Signal no for given signal id
		 * @param signalId signal id
		 * @return track no
		 */
		int getSignalNotrack(const string& signalId) ;
		/**
		 * get Signal mime type for given signal track no
		 * @param notrack track no
		 * @return signal type audio / video
		 */
		string getSignalFileClass(int notrack=0);
		/**
		 * store signal duration in data model (and accordingly adjust last anchor offset, if AGSet has been initialized)
		 * <br>all tracks are synchronized, thus only one duration if considered even if more than one signal file is used,
		 * this duration ought to be that of the shortest one.
		 * @param dur_in_secs 	Signal duration (in seconds)
		 * @param recheck 		If set to true, check the last anchor and re-adjust its offset.\n
		 * 						Otherwise, only set feature in graph
		 * @note				When creating a new graph, it may be happen that the last anchor offset
		 * 						isn't correct if the graph initialization is proceeded whereas information isn't
		 * 						yet available. Set parameter <em>recheck</em> re-adjust the offset.
		 */
		void setSignalDuration(float dur_in_secs, bool recheck=true);
		/**
		 * get signal duration from data model
		 * @param fromAnchor if true, return "last" anchor signal offset, else return duration stored in "duration" data model property
		 * @return signal duration (in seconds)
		 */
		float getSignalDuration(bool fromAnchor=false);
		/**
		 * get signal property for given signal track
		 * @param notrack track no
		 * @param item property item
		 * @return property value
		 */
		string getSignalProperty(int notrack, const std::string& item);
		/**
		 * get signal property for given signal id
		 * @param signalId signal id
		 * @param item property item
		 * @return property value
		 */
		string getSignalProperty(const std::string& signalId, const std::string& item);
		/**
		 * get signal property for given track
		 * @param notrack track no
		 * @param item property item
		 * @param value property value
		 */
		void setSignalProperty(int notrack, const std::string& item, const std::string& value);
		/**
		 * set signal property for given signal id
		 * @param signalId signal id
		 * @param item property item
		 * @param value property value
		 */
		void setSignalProperty(const std::string& signalId, const std::string& item, const std::string& value);
		/**
		 * get all signal properties for all signal tracks
		 * @param signals signals map in which properties will be stored
		 */
		void getSignalsProperties(std::map <std::string, std::map<std::string, std::string> >& signals);
		/**
		 * @return total number of tracks associated to data model
		 */
		int getNbTracks();
		/**
		 *	associates one (or more) signal to data model from given file
		 * @param path signal file path
		 * @param sigclass signal mime class (audio/video)
		 * @param sigtype signal mime type (wav/mp3/...)
		 * @param encoding signal encoding (pcm,...)
		 * @param nbtracks  number of tracks to take into account from file (starting from the first one)
		 * @param notrack_hint hint for track number assignation. if -1, let data model fix the track no.
		 * @param init_graphs if true, initialize all graphs for added tracks, accordingly to annotation conventions.
		 * @return vector of all added signal ids
		 */
		std::vector<string> addSignal(const string& path, const string& sigclass="audio", const string& sigtype="wav", const string& encoding="pcm", int nbtracks=1, int notrack_hint=-1, bool init_graphs=true);
		/**
		 * dissociates a signal track from data model
		 * @param signalId id of signal to remove
		 * @param all_tracks if true, also dissociates all tracks having the same signal file url.
		 * @return true
		 */
		bool removeSignal(const string& signalId, bool all_tracks=true);
		/**
		 * dissociates all tracks having the same signal file url from data model
		 * @param signalUrl url of signals to remove
		 * @return true
		 */
		bool removeSignalFile(const string& signalUrl);
		/**
		 * store meta-data from "info" file associated to signal file into data model
		 * @param sigId signal id to with meta data will be associated
		 * @param path info file path
		 * @param itemlist items to be loaded from file. if empty, no item is loaded.
		 * @note info file should contains lines of the form "key value"; lines starting with dash character are skipped
		 */
		void loadSignalInfo(const string& sigId, const string& path, const string& itemlist="");
		//@}


		/*!  @name Data model status  */
		//@{
		/**
		 * query data model update state
		 * @return true if data model has been updated since last save
		 */
		bool isUpdated() { return m_updated; }
		/**
		 * set data model update state
		 * @param b
		 */
		void setUpdated(bool b=true) ;
		//@}

		/**
		 *  @name Query annotation conventions
		 *  see also class Conventions for detailed documentation
		 */
		//@{
		/**
		 * get annotation conventions manager object
		 * @return annotation conventions manager object reference
		 */
		Conventions& conventions() { return m_conventions; }

		/**
		 * Gets anchor links manager object
		 * @return	Anchors links manager object reference
		 */
		AnchorLinks& anchorLinks() { return m_anchorLinks; }

		/**
		 * get graph type for given annotation graph id
		 * @param id annotation graph id
		 * @param is_graph_id MUST be false if id is anchor or annotation id (default), MUST be true if id is a graph id
		 * @return graph type
		 */
		const string& getGraphType(const string& id, bool is_graph_id=false);
		/**
		 * setGraphType : define applicable graph type for all datamodel operations
		 * @param gtype graph type (transcription_graph eg)
		 * @note use this when loading a file which contains only one graph type (CTM eg) to speed up processings.
		 */
		void setGraphType(const string& gtype) { m_graphType = gtype; }

		/** @see Conventions */
		const vector<string>& getMainstreamTypes(const string& graphtype="transcription_graph") { return m_conventions.getMainstreamTypes(graphtype); }
		/** @see Conventions */
		const vector<string>& getQualifierTypes(const string& qclass="", const string& graphtype="transcription_graph") { return m_conventions.getQualifierTypes(qclass, graphtype); }
		/** @see Conventions */
		string getQualifierClass(const string& type, const string& graphtype="transcription_graph") { return m_conventions.getQualifierClass(type, graphtype); }
		/** @see Conventions */
		bool isQualifierType(const string& type, const string& graphtype="transcription_graph") { return m_conventions.isQualifierType(type, graphtype); }
		/** @see Conventions */
		bool isMainstreamType(const string& type, const string& graphtype="transcription_graph") { return m_conventions.isMainstreamType(type, graphtype); }
		/** @see Conventions */
		string normalizeSubmain(const string& type, const string& graphtype="transcription_graph") { return m_conventions.normalizeSubmain(type, graphtype); }

		/** @see Conventions */
		const string& mainstreamBaseType(const string& graphtype="transcription_graph") { return m_conventions.mainstreamBaseType(graphtype); }
		/** @see Conventions */
		const string& segmentationBaseType(const string& graphtype="transcription_graph") { return m_conventions.segmentationBaseType(graphtype); }

		//@}

		/**
		 * Returns whether a mainstream base typed element uses text data
		 * @param id			Mainstream element id
		 * @param graphtype		Normalized graphtype name, or empty for auto guessing
		 * @return				True for mainstream element of "unit_text" subtype, false otherwise
		 */
		bool mainstreamBaseElementHasText(const string& id, const string& graphtype="") ;

		/**
		 * Checks whether the given annotartion is an event mainstream element (base type without text)
		 * @param id			Mainstream element id
		 * @param graphtype		Normalized graphtype name, or empty for auto guessing
		 * @return				True for mainstream element of "unit_event" subtype, false otherwise
		 */
		bool isEventMainstreamElement(const string& id, const string& graphtype="") ;

		/**
		 * Checks whether the mainstream element is fully defined in conventions
		 * @param id			Annotation id
		 * @param graphtype		Graph type
		 * @return
		 */
		bool mainstreamBaseElementIsValid(const string& id, const string& graphtype) ;

		/**
		 * main transcription graph initialisation
		 */
		/**
		 * create annotation in graph and set its default features from convention specifications
		 * @param graph annotation graph
		 * @param type annotation type
		 * @param start annotation start anchor id
		 * @param end annotation end anchor id
		 * @param emit_signal if true, emit signalElementModified for new annotation
		 */
		string createAnnotation(const string& graph, const string& type, const string& start, const string& end, bool emit_signal=false);
		/**
		 * create annotation in graph and set its features from given properties map
		 * @param graph annotation graph
		 * @param type annotation type
		 * @param start annotation start anchor id
		 * @param end annotation end anchor id
		 * @param props annotation properties
		 * @param emit_signal if true, emit signalElementModified for new annotation
		 */
		string createAnnotation(const string& graph, const string& type, const string& start, const string& end, const map<string, string>& props, bool emit_signal=false);

		/**
		 * utility function for SGML parser
		 * @todo to be replaced by base DataModel methods in SGML parser
		 */
		std::string createParseElement(const string& prevId, const string& graphtype, const string& annottype, int notrack, float start, float end) ;
		/** same as above */
		std::string createParseElement(const string& graphtype, const string& annottype, int notrack,
														const string& anchor_start, const string& anchor_end) ;

		/**
		* create signal-anchored node at given offset even if an anchor at same place already exist
		* @param graphId graph id in which to create
		* @param notrack signal track no
		* @param offset offset in signal
		* @return created anchor id / existing anchor id
		*/
		string createAnchor(const string& graphId, int notrack, float offset) ;
		/**
		* create signal-unanchored node
		* @param graphId graph id in which to create
		* @return created node id / existing anchor id
		*/
		string createNode(const string& graphId) { return agCreateAnchor(graphId); }
		/**
		 * check if given anchor id exists in current AGSet
		 * @param id	anchor id
		 * @return true if exists, else false
		 */
		bool existsAnchor(const string& id) ;

		/**
		* get transcription language for given segment id
		* @note will return segment language property if set, else parent segment language property if set,
		* else document transcription language
		* @param id segment id
		* @param use_default if true and no segment language defined, will return transcription language
		* @return segment language
		*/
		const string& getSegmentLanguage(const string& id, bool use_default=true);

		/**
		 * @name local speaker dictionary management
		 */
		//@{
		/**
		 * @return (reference to) local speaker dictionnary
		 */
		SpeakerDictionary& getSpeakerDictionary() { return m_speakersDict; }
		/**
		 * force emit signalElementModified for all turns associated to given speaker
		 * @param id speaker id
		 */
		void speakerUpdated(const string& id);

		/**
		 * set annotation hint
		 * @param type annotation type
		 * @param name feature name
		 * @param value feature value
		 * @param notrack signal track no  (starting from 0)
		 */
		void setHint(const string& type, const string& name, const string& value, int notrack=0) ;

		/**
		 * set annotation hint
		 * @param hint_label
		 * @param hint_value
		 * @param notrack signal track no  (starting from 0)
		 */
		void setHint(const string& hint_label, const string& hint_value, int notrack=0) ;

		/**
		 * get annotation hint
		 * @param type annotation type
		 * @param name feature name
		 * @param notrack signal track no  (starting from 0)
		 * @return hint value / "" if no hint
		 */
		const string& getHint(const string& type, const string& name, int notrack=0) ;

		/**
		 * Gets annotation hint
		 * @param label
		 * @param notrack
		 * @return hint value / "" if no hint
		 */
		const string& getHint(const string& label, int notrack) ;

		/**
		 * Cleans a hint information
		 * @param label		Label hint
		 * @param notrack	Track number
		 */
		void cleanHint(const string& label, int notrack) ;

		/**
		 * set speaker hint for new turns creation
		 * @param notrack signal track no (starting from 0)
		 * @param spkid speaker id
		 */
		void setSpeakerHint(const string& spkid, int notrack=0);

		/**
		 * eet speaker hint for given track
		 * @param notrack signal track no
		 * @return speaker id
		 */
		string getSpeakerHint(int notrack=1) { return getSignalProperty(notrack, "speaker_hint"); }
		/**
		 * @return true if given speaker id is associated to a turn
		 */
		bool speakerInUse(const string& spkid);
		/**
		 * @return true if some speakers in dictionary are not associated to any turn
		 */
		bool hasUnusedSpeakers();

		/**
		 * replace given speaker by another speaker, for given turn if turnid != "", else for all turns associated to old_spkid
		 * @param old_spkid old speaker id
		 * @param new_spkid replacement speaker id
		 * @param turnid turn id / "" for all turns
		 * @param emit_signal if true, emit signalElementModified for each modified turn
		 * @return true if at least one turn modified, else false
		 */
		bool replaceSpeaker(const string& old_spkid, const string& new_spkid, const string& turnid="", bool emit_signal=true);

		//@}

		/**
		 * @name annotations management
		 */
		//@{

		std::string addBackgroundSegment(int notrack, float start, float stop, const string& subtype="", const string& level="",  bool emit_signal=true);
		/**
		 *  add qualifier annotation to segments
		 *  @param qtype qualifier type
		 *  @param start_id id of text segment on which start qualifier is attached
		 *  @param end_id id of text segment on which end qualifier is attached, if different from start
		 *  @param desc qualifier desc
		 *  @param emit_signal true if element modified signal to be emitted.
		 */
		std::string addQualifier(const string& qtype, const string& start_id, const string& end_id="", const string& desc="", bool emit_signal=true);

		/* Delete annotations */
		/**
		 * delete any element with given id
		 * <br>function first checks element deletion rules and does nothing upon check failure
		 * @param id 			Id of element to be deleted
		 * @param emit_signal 	True to emit signalElementModified() for any deleted or resized element
		 * @param force_wtext   By default, mainstream base type element won't be deleted if they still have text data.
		 * 						If set to true, deletion is forced
		 * @return 				True if element has been deleted, False otherwise
		 */
		bool deleteElement(const std::string& id, bool emit_signal=true, bool force_wtext=false);

		/**
		 * delete any element with given id
		 * <br>function first checks element deletion rules and does nothing upon check failure
		 * @param	 	id 				Id of element to be deleted
		 * @param	 	emit_signal 	True to emit signalElementModified() for any deleted or resized element
		 * @param[out] 	err				Get error message
		 * @param force_wtext  			By default, mainstream base type element won't be deleted if they still have text data.
		 * 								If set to true, deletion is forced
		 * @return 						True if element has been deleted, False otherwise
		 */
		bool deleteElement(const std::string& id, bool emit_signal, string& err, bool force_wtext);

		/**
		 *  delete mainstream element with given id
		 * @param id 		   Id of element to be deleted
		 * @param emit_signal  If true, emit signalElementModified() signal for both deleted element and eventually resized element
		 * @param force_wtext  By default, mainstream base type element won't be deleted if they still have text data.
		 * 					   If set to true, deletion is forced.
		 * @note
		 *  When a segment is deleted, previous segment of same type is extended to deleted's end.
		 *  @
		 *	<br>If deleted segment is a "text" segment, deleted segment data is "attached" to previous segment :
		 *  - if no "event" annotation is attached to deleted segment start node, then segment text is added to previous segment, previous segment end is set to deleted segment end and start node is deleted,
		 *  - if any "event" annotation is attached to deleted segment start node, then this node is kept but "unanchored", ie. its signal offset is unset.
		 *	<br><br>
		 *  If any "mainstream" upper level annotation (eg. a turn) is attached at deleted segment start node, it will also be deleted
		 *	<br><br>
		 *  if "overlapping" branches terminate at deleted segment start, then all branches are extended to deleted segment end, as shown:
		 *	<br><br>
		 *       ___ a2___a3___                        ___ a2___a3______
		 *  a1 /               \ a5_a6   ==>      a1 /                  \ a6
		 *     \ ______a4______/                     \ ______a4_________/
		 *  .
		 *  <br><br>
		 *  if deleted segment is the first segment of an "overlapping" branch :
		 *	  - if its order is > 0 : it is attached to the end of main branch first segment, and eventual following segments are also attached to main branch first segment.
		 *
		 *      ___ a2___a3___                     ___ a2___a3
		 *   a1/              \a5_a6   ==>      a1/         /      a5-a6
		 *     \______a4______/                            a4_____/
		 *
		 *   - if its order is = 0 : ??? -> deletion not allowed.
		 */
		 /*	TODO -> allow deletion if "empty" (ie no text, no qualifiers) -> then overlapping branch becomes main branch. */
		void deleteMainstreamElement(const std::string& id, bool emit_signal=true, bool force_wtext=false);
		/**
		 * check deletion rules for given segment id:
		 *   - can't delete start segment of main branch
		 *   - if deleting an overlapping turn, then also delete "empty" childs
		 *   - if fast transcription, also delete childs
		 * @param 		id 				Segment id
		 * @param[out] 	with_children 	Children annotations also to be deleted
		 * @param[out] 	diagnosis 		Eventual check failure diagnosis
		 * @param 		forceFirst
		 * @return true if segment can be deleted, else false
		 *
		 */
		bool checkDeletionRules(const std::string& id, bool& with_children, string& diagnosis, bool forceFirst=false);
		/**
		 * delete background segment
		 * @param id id of element to be deleted
		 * @param emit_signal true to emit signalElementModified() for any deleted or resized element
		 */
		void deleteBackground(const string& id, bool emit_signal) ;
		/**
		 * check background segment insertion rules
		 * <br> diagnosis will contains eventual error messages or warnings
		 * @param notrack corresponding signal track
		 * @param start_offset inserted segment start offset in signal / -1 if not to be checked
		 * @param stop_offset inserted segment end offset in signal / <= 0 if not to be checked
		 * @param inactive_to_update (returned) id of inactive neighbour segment that would be modified by insertion
		 * @param diagnosis (returned) eventual check failure diagnosis
		 * @return true if background segment can be inserted, else false
		 */
		bool checkBackgroundInsertionRules(int notrack, float start_offset, float stop_offset, string& inactive_to_update, string& diagnosis) ;
		/**
		 * utilitity function for background renderer
		 * @todo check if can be done in  background renderer - do not use
		 */
		string checkBackgroundParallelSegmentAtStart(const string& segment, const string& background);
		/**
		 * utilitity function for background renderer
		 * @todo check if can be done in  background renderer - do not use
		 */
		string checkBackgroundParallelSegmentAtEnd(const string& segment, const string& background);

		/* update annotations */
		/**
		 * set segment start and stop signal offsets and perform eventual adjustments of neighbour and overlapping segments
		 * @param id 						Segment id
		 * @param start_offset				New start offset
		 * @param stop_offset 				New stop offset
		 * @param use_epsilon 				Use epsilon value when checking new offset vs previous offset
		 * @param emit_signal				True to emit signalElementModified() for any deleted or resized element
		 */
		void setSegmentOffsets(const string& id, float start_offset, float stop_offset, bool use_epsilon=true, bool emit_signal=true) ;
		/**
		 * retrieve qualifier annot id from parent segment id, qualifier type & qualifier desc
		 * @param pid parent segment id
		 * @param qtype target qualifier type
		 * @param qdesc target qualifier description
		 * @return qualifier id / "" if not found
		 */
		string getQualifierId(const string& pid, const string& qtype, const string& qdesc);

		/**
		 * update qualifier annotation
		 * @param qid qualifier id
		 * @param qtype qualifier type
		 * @param desc qualifier description
		 * @param emit_signal true to emit signalElementModified() for any updated element
		 * @return qualifier id
		 * @throw error message if another qualifier with same type, same desc and same anchors already exists
		 */
		std::string setQualifier(string qid, const string& qtype="noise", const string& desc="music", bool emit_signal=true);
		/**
		 * update qualifier annotation with normalized form (mainly for entities)
		 * @param qid qualifier id
		 * @param qtype qualifier type
		 * @param desc qualifier description
		 * @param norm qualifier normalized form
		 * @param emit_signal true to emit signalElementModified() for any updated element
		 * @return qualifier id
		 * @throw error message if another qualifier with same type, same desc and same anchors already exists
		 */
		std::string setQualifier(string qid, const string& qtype, const string& desc, const string& norm, bool emit_signal) ;

		/**
		 * Updates mainstream event element with given data
		 * @param id				Mainstream event element id
		 * @param value				Event type
		 * @param subvalue			Event subtype
		 * @param emit_signal 		True to emit signalElementModified()
		 * @return					True if modifications have been proceded, false otherwises
		 */
		bool setEventMainstreamElement(const string& id, const string& value, const string& subvalue, bool emit_signal) ;

		/**
		 * get element start / end offset
		 * @param id element id
		 * @param start if true, will get element's start anchor offset, else will get element's end anchor offset
		 * @return element start/end signal offset (in seconds)
		 */
		float getElementOffset(const string& id, bool start) ;

		/**
		 * Sets element start / end offset - no coherence check performed
		 * @param id 			Element id
		 * @param off 			New signal offset (in seconds)
		 * @param start 		If true, will set element's start anchor offset, else will set element's end anchor offset
		 * @param emitSignal	True for telling view to update, false otherwise
		 * @return				True for success, False for failure
		 */
		bool setElementOffset(const string& id, float off, bool start, bool emitSignal) ;

		/**
		 * Unsets element start / end offset - no coherence check performed
		 * @param id 			Element id
		 * @param start 		If true, will set element's start anchor offset, else will set element's end anchor offset
		 * @param emitSignal	True for telling view to update, false otherwise
		 * @return				True for success, False for failure
		 */
		bool unsetElementOffset(const string& id, bool start, bool emitSignal) ;

		/**
		 * check if current AGSet holds elements with given type
		 * @param type	checked type
		 * @param graphId	target AG id - let empty if id to be guessed from annotation type, with respect to current annotation conventions
		 * @return true if elements with given type exist, else false
		 */
		bool hasElementsWithType(const std::string& type, string graphId="");

		/**
		 * get elements ids with given type
		 * @param dest	(out) vector in which returned ids will be stored;
		 * @param type	target type
		 * @param notrack target track no (-1 for both)
		 * @param graphId	target AG id - let empty if id to be guessed from annotation type, with respect to current annotation conventions
		 * @return true if ok, false if no element of given type
		 */
		bool getElementsWithType(vector<string>& dest, const std::string& type, int notrack=-1, string graphId="");

		/**
		 * check element existence for given id
		 * @param id element id
		 * @return true if element with given id exist, else false
		 */
		bool existsElement(const std::string& id) ;


		/**
		 * Checks that resizing given element will not make neighbour elements anchoring inconsistent
		 * @param 		id 					Segment id
		 * @param 		start_offset 		New signal start offset
		 * @param 		end_offset			New signal end offset
		 * @param[out] 	diag (returned) 	Check diagnosis if any problem detected
		 * @param 		check_over 			If true, also checks eventual overlapping segments resizing
		 * @param		fromLink			True if resize action is called from anchor links action. Mainly set to false.
		 * @param		allowUnanchored		True if unanchored value (negative offset) must be ignored by checking
		 * @return 							true if valid resize, else false
		 * @note 							Overlapping may be constrained by annotation conventions.
		 *  								If resized segment has overlapping segments and conventions specify that
		 *   								overlapping segments must be aligned on overlapped ones, then resize rules
		 *   								are also checked on overlapping segments.
		 */
		bool checkResizeRules(const string& id, float start_offset, float end_offset, string& diag, bool check_over=true, bool fromLink=false, bool allowUnanchored=false);

		/**
		 * Checks element insertion rules
		 * @param 		type 				Segment type to be inserted
		 * @param 		notrack 			Corresponding signal track
		 * @param 		start 				Inserted segment start offset in signal / -1 if not to be checked
		 * @param 		stop 				Inserted segment end offset in signal / <= 0 if not to be checked
		 * @param 		prevId 				Id of existing annotation upon which new segment is to be aligned (type != mainstreamBaseType()) or after which it is to be inserted (type = mainstreamBaseType())
		 * 									prevId value may be updated by checkInsertionRules.
		 * @param[out]	order 				Segment order if overlapping allowed for segment type
		 * @param[out] 	diag 				Check diagnosis if any problem detected
		 * @param		alignCandidate		Annotation id. Use it for allowing creation on this existing element, empty otherwise
		 * @return 							True if segment can be inserted, else false
		 */
		bool checkInsertionRules(const string& type, int notrack, float start, float stop, const string& prevId, int& order, string alignCandidate, string& diag);

		/**
		 * Checks whether timestamp modification is allowed.
		 * @param start_offset		Inserted segment start offset in signal / -1 if not to be checked
		 * @param stop_offset 		Inserted segment end offset in signal / <= 0 if not to be checked
		 * @param id				Annotation beeing inserted
		 * @param diag				String to receive error message
		 * @return					True if modification is allowed, false otherwise
		 */
		bool checkTimestampRules(float start_offset, float stop_offset, const string& id, string& diag) ;

		//@}

		/**
		 * @name query / set annotations properties
		 */
		//@{
		/**
		 * gets annotation type
		 * @param id annotation id
		 * @return annotation type
		 */
		string getElementType(const std::string& id);

		/**
		 * check if given annotation has given property
		 * @param id	annotation id
		 * @param item	property name
		 * @return true if property exists for given annotation, else false
		 */
		bool hasElementProperty(const string& id, const string& item) ;
		/**
		 * get annotation property value as string value
		 * @param id annotation id
		 * @param item property item
		 * @param defaut default value if undefined property
		 * @return property value
		 */
		string getElementProperty(const std::string& id, const std::string& item, const std::string& defaut="");
		/**
		 * get annotation property value as int value
		 * @see above
		 */
		int getElementProperty(const std::string& id, const std::string& item, int defaut);
		/**
		 * get annotation property value as float value
		 * @see above
		 */
		float getElementProperty(const std::string& id, const std::string& item, float defaut);
		/**
		* set element property from string value for given annotation
		* @param id annotation id
		* @param item property item
		* @param value new property value
		* @param emit_signal if true emit signalElementModified(UPDATED) signal
		* @return	True for succes, False for failures
		*/
		bool setElementProperty(const std::string& id, const std::string& item, const std::string& value, bool emit_signal=true);

		/**
		 * set annotation property value as int value
		 * @see above
		 * @return	True for succes, False for failures
		 */
		bool setElementProperty(const std::string& id, const std::string& item, int value, bool emit_signal=true);
		/**
		 * set annotation property value as float value
		 * @see above
		 * @return	True for succes, False for failures
		 */
		bool setElementProperty(const std::string& id, const std::string& item, float value, bool emit_signal=true);

		/**
		* delete element property for given id
		* @param id annotation id
		* @param item property name
		* @param emit_signal if true and property deleted, emits signalElementModified(UPDATED)
		* @return true if property deleted, else false
		*/
		bool deleteElementProperty(const std::string& id, const std::string& item, bool emit_signal=true);

		/**
		 * set annotation confidence property value as float value
		 * @param id annotation id
		 * @param value confidence value
		 * @param emit_signal if true emits signalElementModified()
		 * @return	True for success, False for failures
		 */
		bool setConfidence(const std::string& id, float value, bool emit_signal=true) { setElementProperty(id, "score", value, emit_signal); }

		/**
		* check if given segment is a speech segment
		* @param id segment id
		* @param checkSpeaker if false and id is  mainstream base type, just check id has text subtype, else check speaker property in id's hierarchy (default),
		* @return  true if true if speech segment , else false
		*/

		bool isSpeechSegment(const std::string& id, bool checkSpeaker=true);
		/**
		 * check if given background is active or not (ie. background type != "none")
		 * @param id background id
		 * @return true if active, else false
		 */
		bool isActiveBackground(const string& id) ;
		/**
		* check if given annotation is instantaneous
		* @param id annotation id
		* @return  true if element start anchor = element end anchor, else false
		*/
		bool isInstantaneous(const std::string& id);
		/**
		* (overlapping annotations) return annotation order
		* @param id annotation id
		* @return annotation order (from 0 to n)
		*/
		int getOrder(const std::string& id);

		/**
		* check if some elements with given property exist in graph
		* @param agid graph id
		* @param name feature name
		* @param type element type
		* @return true if have some elements, else false.
		*/
		bool hasElementsWithProperty(const string& agid, const string& name, const string& type="");
		//@}


		/**
		 * @name browse through graph
		 */
		//@{

		/**
		 * get qualifier annotations of given type incoming / outgoing at start of segment with given id
		 * @param 		id 						Segment id
		 * @param[out]  v  						Vector in which qualifiers ids will be stored
		 * @param 		type 					Target qualifier type
		 * @param 		starting_at 			If true, get qualifiers starting at segment start, else get qualifiers ending at segment start
		 * @param 		onlyCheckExistence		True for only checking if qualifier annotations of given type incoming / outgoing at start
		 * 										of segment with given id
		 * @return								True if some elements have been found, False otherwise
		 */
		bool getQualifiers(const string& id, vector<string>& v, const string& type, bool starting_at, bool onlyCheckExistence) ;

		/**
		 * Checks whether at least one qualifier annotations of given type incoming / outgoing at start
		 * of segment with given id
		 * @param 		id 						Segment id
		 * @return								True if some elements have been found, False otherwise
		 */
		bool hasQualifiers(const string& id) ;

		/**
		 * get background annotations starting / ending / included during given segment time interval
		 * @param id segment id
		 * @param v (returned) vector in which background ids will be stored
		 * @param mode	0:starting in id time interval / 1:ending in id time interval / 2: both
		 */
		void getBackgroundsInSegment(const string& id, vector<string>& v, int mode) ;

		/**
		 * get segments of given type starting / ending / included during time interval
		 * @param graphId target graph id
		 * @param type	target annotation type
		 * @param notrack target track no
		 * @param start_offset	time interval start offset
		 * @param end_offset	time interval start offset
		 * @param v (returned) vector in which background ids will be stored
		 * @param mode 0:starting in time interval  1: ending in time interval 2: starting or ending in time interval, 3: included in time interval
		 */
		void getSegmentsInRange(const string& graphId, const string& type,
					int notrack, float start_offset, float end_offset, 	vector<string>& v, int mode) ;
		/**
		 * get alignment element id for given element id
		 * <br> alignment element is a mainstream element with (direct) lower precedence with same start anchor
		 * @param id element id
		 * @return alignment element id
		 */
		string getAlignmentId(const string& id);
		/**
		 * get next (signal-anchored) element id with same type for given element id
		 * @param id element id
		 * @return next element id
		 */
		string getNextAnchoredElementId(const string& id); // anchored only
		/**
		 * return next element with same type in graph
		 * @param id current element id
		 * @return next element id, or empty string if no next element found
		 * @note
		 *  in case 2 overlapping elements are starting at id end, then returns element with same order as current element id
		 */
		string getNextElementId(const string& id);
		/**
		 * return previous element with same type in graph
		 * @param id current element id
		 * @return previous element id, or empty string if no previous element found
		 * @note
		 *  in case 2 overlapping elements are ending at id start, then 0-order element will be returned
		 */
		string getPreviousElementId(const string& id);
		/**
		 * return "parent" element in graph, with respect to annotation conventions
		 * <br>returned element is a signal-anchored mainstream annotation. If current element is a qualifier, then by dafault the base segment it is attached to will be returned.
		 * @param id current element id
		 * @param type target mainstream parent type / "" (to target direct parent type)
		 * @param with_same_start if true, parent must be attached to element start anchor, else returns ""
		 * @param check	True for checking id existence, type availability, ...
		 * @return parent element id, or empty string  if no parent found
		 */
		string getParentElement(const string& id, const string& type, bool with_same_start=false, bool check=true);

		/**
		 * return "parent" element in graph, with respect to annotation conventions
		 * <br>returned element is a signal-anchored mainstream annotation. If current element is a qualifier, then by dafault the base segment it is attached to will be returned.
		 * @param id current element id
		 * @param with_same_start if true, parent must be attached to element start anchor, else returns ""
		 * @return parent element id, or empty string  if no parent found
		 */
		string getParentElement(const string& id, bool with_same_start=false);

		/**
		 * return "parent" property value
		 * @param id annotation id
		 * @param name target property name
		 * @param with_same_start if true, parent must be attached to element start anchor, else returns ""
		 * @return parent property value / "" if not found
		 * @note if current element doesn't own given property, try to get its value from parent elements in graph
		 *
		 */
		string getParentProperty(const string& id, const string& name, bool with_same_start=false);

		/**
		 * return ids of all elements of given type with same start anchor as given id
		 * <br>if id identifies an "overlapping" element (order > 0 ), then 0-order element
		 *    is returned as first entry in vector
		 * @param id current element id
		 * @param type target annotation type (defaults to current element's type)
		 * @return vector holding ids of elements found
		 */
		vector<string> getElementsWithSameStart(const string& id, string type="");
		/**
		 * return ids of all elements of given type with same end anchor as given id
		 *  <br> if id identifies an "overlapping" element (order > 0 ), then 0-order element
		 *    is returned as first entry in vector
		 * @param id current element id
		 * @param type target annotation type (defaults to current element's type)
		 * @return vector holding ids of elements found
		 */
		vector<string> getElementsWithSameEnd(const string& id, string type="");
		/**
		 * return id of mainstream start element for given annotation id
		 * <br> if id identifies a mainstream annotation, function will return id,
		 *   else function will return (base segment) mainstream annotation starting at id's start anchor
		 * @param id annotation id
		 * @return id of mainstream start element
		 */
		string getMainstreamStartElement(const string& id);
		/**
		 * return id of mainstream end element for given annotation id
		 * <br>  if id identifies a mainstream annotation, function will return id,
		 *   else function will return (base segment) mainstream annotation starting at id's end anchor
		 * @param id annotation id
		 * @return id of mainstream start element
		 */
		string getMainstreamEndElement(const string& id);
		/**
		 * For given annotation id, returns the  id of next mainstream element.
		 * @param id 		Annotation id
		 * @return 			Id of next mainstream element
		 *
		 * @note
		 *   If id identifies a mainstream annotation, function will return the next element of,
		 *   same type, otherwise it will return the base mainstream annotation starting at id's
		 *   end anchor.\n
		 *   If element belongs to an overlapping branch terminating at element's end,
		 *   will return "" (no next mainstream element).
		 */
		string getMainstreamNextElement(const string& id);
		/**
		 * get "signal-anchored" mainstream base-type element id for given annotation id
		 * <br>if annotation is qualifier type, or if annotation start is unanchored, browse back in
		 *  graph to retrieve anchored segment
		 * @param id annotation id
		 * @return id of mainstream start element
		 */
		string getAnchoredBaseTypeStartId(const std::string& id);
		/**
		 * get "signal-anchored" mainstream base-type element id id for given annotation id
		 * <br>if annotation is qualifier type, or if annotation start is unanchored, browse back in
		 *  graph to retrieve anchored segment
		 * @param id annotation id
		 * @return id of mainstream end element
		 */
		string getAnchoredBaseTypeEndId(const std::string& id);
		/**
		 * get next "signal-anchored" mainstream base-type element id id for given annotation id
		 * <br>if annotation is qualifier type, or if annotation start is unanchored, browse forth in
		 *  graph to retrieve anchored segment start
		 * @param id annotation id
		 * @return id of next mainstream element
		 */
		string getBaseTypeNextId(const std::string& id);
		/**
		 * locate mainstream annotation with given type which overlaps given time for given track
		 * @param type target annotation type
		 * @param startTime  signal offset
		 * @param notrack  signal track
		 * @param graphtype target graph type / "" to guess graph type from annotation type
		 * @return segment id / "" if no segment found
		 */
		string getByOffset(const string& type, float startTime, int notrack=-1, string graphtype="");


		/**
		 * locate next mainstream annotation with given type which starting after given time for given track
		 * @param type target annotation type
		 * @param startTime  signal offset
		 * @param notrack  signal track
		 * @return
		 */
		string getNextByOffset(const string& type, float startTime, int notrack=-1);

		/**
		 * retrieve all segments of given type with a start time within given time interval
		 * @param type searched type
		 * @param v (returned) signal segments found within time interval
		 * @param start time range start
		 * @param stop time range end
		 * @param notrack  if -1, retrieve segments for all signal tracks, else retrieve segments only for specified signal track no
		 * @param anchored_only if true, retrieve only segments with signal-anchored start node (default)
		 * @param follow_graph if true, follow graph links to retrieve end-anchored signal segments (default)
		 * @return true if ok, else false
		 *
		 * @note
		 *   if stop == 0.0, will retrieve all segments up to signal end.
		 *   returned segments are ordered by (start_offset / order / notrack)
		 */
		bool getSegments(string type, vector<SignalSegment>& v, float start=0.0, float stop=0.0, int notrack=-1, bool anchored_only=false, bool follow_graph=false);
		/**
		 * retrieve all segments of given type with a start time within parent segment time interval
		 * <br>returned segments are ordered by (start_offset / order / notrack)
		 * @param type searched type
		 * @param v (returned) signal segments found within parent time range
		 * @param parent parent signal segment
		 * @return true if ok, else false
		 */
		bool getSegments(string type, vector<SignalSegment>& v, const SignalSegment& parent);
		/**
		 * get all segments of given type, ordered by graph links, starting at parent start node, ending at parent end node
		 *  @param type searched type
		 *  @param v returned vector holding all elements found
		 *  @param parent parent signal segment
		 *  @return true if any child found, else false.
		 * @note
		 *  the retrieval is done by browsing though graph links.
		 *  If searched type is a mainstream or background type, then all type-arcs
		 *  belonging to the subgraph starting at parent start node and ending at
		 *  parent start node are retrieved
		 *  if searched type is an "event" type
		 *
		 *   other arc types, like events, are anchored  on mainstreamBaseType()-typed arcs.
		 *  thus, to retrieve arcs attached to a given graph, we must first retrieve corresponding
		 *   "segment" arcs
		 *  if parent end node is unanchored -> retrieve until end-anchored segment is found
		 *
		 */
		bool getChildSegments(string type, vector<SignalSegment>& v, const SignalSegment& parent);

		/**
		 * Gets all segments of given type, ordered by graph links, starting at parent start node, ending at parent end node
		 *  @param[out] dest 		Returned vector holding all elements found
		 *  @param 		type 		Searched type
		 *  @param 		parent		Parent signal segment
		 *  @param 		notrack		Track number
		 *  @return 				True if any child found, else false.
		 * @note
		 *  the retrieval is done by browsing though graph links.
		 *  If searched type is a mainstream or background type, then all type-arcs
		 *  belonging to the subgraph starting at parent start node and ending at
		 *  parent start node are retrieved
		 *  if searched type is an "event" type
		 *
		 *   other arc types, like events, are anchored  on mainstreamBaseType()-typed arcs.
		 *  thus, to retrieve arcs attached to a given graph, we must first retrieve corresponding
		 *   "segment" arcs
		 *  if parent end node is unanchored -> retrieve until end-anchored segment is found
		 *
		 */
		bool getChilds(vector<string>& dest, const string& type, const string& parent, int notrack=-1);

		/**
		 * 	retrieve signal segment properties for given (mainstream or background) annotation id;
		 * @param id annotation id
		 * @param s (returned) signal segment
		 * @param checktrack if != -1, check that annotation corresponds to given track
		 * @param anchored_only return false if annotation start not anchored in signal timeline
		 * @param follow_graph if annotation end not anchored, browse through graph to retrieve actual signal segment anchored end (may happen for splitted "text" segments)
		 * @param parent_order parent overlapping branch order / 0 (default)
		 * @return true if signal segment retrieved, else false.
		 */
		bool getSegment(const string& id, SignalSegment& s, int checktrack=-1, bool anchored_only=false, bool follow_graph=false, int parent_order=0);
		/**
		* get all elements of given type linked to start element,
		*    ending when new anchored element with same type as start element is found.
		*    (resulting vector includes start element if corresponds to seltype)
		*
		* @param start_id id of start element for graph browse
		* @param v (returned) vector in which annotation ids will be stored
		* @param type target annotation type / "" for all types
		* @param qualifiers_only if true return only qualifier-type annotations
		* @param only_at_start if true return only elements attached to same start anchor as start_id
		* @return end anchor or last segment-type element browsed
		*/
		string getLinkedElements(const string& start_id, vector<string>& v, const string& type="", bool qualifiers_only=false, bool only_at_start=false);

		/**
		 * return all annotations linked at given anchor
		 * @param anchor anchor id
		 * @param incoming	(returned) set of incoming annotations
		 * @param outgoing	(returned) set of outgoing annotations
		 */
		void getInAndOutAnnotationAtAnchor(const string anchor, std::set<string>& incoming, std::set<string>& outgoing) ;
		/**
		 * return annotations of given type incoming at given anchor id
		 * @param id anchor id
		 * @param type target annotation type
		 * @return incoming annotation set
		 */
		std::set<string> getIncomingAnnotations(const string& id, const string& type) ;

		//@}


		/**
		 * @name query anchors & signal offsets
		 */
		//@{
		/**
		 * get signal-anchored start node for given annotation id.
		 * If current annotation start is unanchored, browse back in mainstream base-type graph to retrieve anchored node.
		 * @param id annotation id
		 * @return start anchor id / "" if not found
		 */
		string getStartAnchor(const std::string& id);
		/**
		 * get signal-anchored end node for given annotation id.
		 * If current annotation end is unanchored, browse forth in mainstream base-type graph to retrieve anchored node.
		 * @param id annotation id
		 * @return end anchor id / "" if not found
		 */
		string getEndAnchor(const std::string& id);
		/**
		 * get signal-anchored start node offset for given annotation id.
		 * If current annotation start is unanchored, browse back in mainstream base-type graph following to retrieve anchored node.
		 * @param id annotation id
		 * @return start anchor  signal offset / -1 if not found
		 */
		float getStartOffset(const std::string& id);

		/**
		 * Returns the start anchor offset of given annotation if it is anchored.
		 * @param id		Annotation id
		 * @param start		True for check at start anchor, false for end
		 * @return			Offset of anchor if anchored, or -1 if it isn't.
		 */
		float isAnchoredElement(std::string id, bool start) ;

		/**
		 * CHecks whether an element is time anchored.
		 * @param id		Element id
		 * @param mode		0: checks if start anchor has time value\n
		 * 					1: checks if end anchor has time value\n
		 * 					2: checks if both anchors have time value\n
		 * 					3: checks if at least one anchor have time value\n
		 * @return			True if the element is time anchored respecting the mode,
		 * 					False otherwise
		 */
		bool isAnchoredElement(const std::string& id, int mode) ;

		/**
		 * get signal-anchored end node offset for given annotation id.
		 * If current annotation end is unanchored, browse forth in mainstream base-type graph to retrieve anchored node.
		 * @param id annotation id
		 * @return end anchor signal offset / -1 if not found
		 */
		float getEndOffset(const std::string& id);
		/**
		 *  returns signal track no corresponding to given annotation start anchor
		 *  @param id annotation id
		 *  @return signal track no  / -1 if annotation not found
		 *  @note element may be anchored or not. If not, then the signal track no of its anchored parent
		 *   element will be returned.
		 */
		int getElementSignalTrack(const std::string& id);
		/**
		 *  returns signal id corresponding to given annotation start anchor
		 *  @param id annotation id
		 *  @return signal id / "" if annotation not found
		 *  @note element may be anchored or not. If not, then the signal id of its anchored parent
		 *   element will be returned.
		 */
		const string& getElementSignalId(const std::string& id);
		/**
		 * returns signal track no corresponding to given anchor (where 0 identifies first signal track)
		 * @param id anchor id
		 * @return signal track no / 0 if anchor not found
		 */
		int getAnchorSignalTrack(const std::string& id);
		//@}


		/**
		 * @name Signals (sigc::signal)
		 */
		//@{
		/**
		 * Signal emitted when an element has been modified.\n
		 * <b>string parameter:</b>			Element type\n
		 * <b>string parameter:</b>			Element id\n
		 * <b>int	  parameter:</b>		Update action type\n
		 */
		sigc::signal<void, std::string, std::string, UpdateType > signalElementModified() { return m_signalElementModified ; }
		/**
		 * Signal emitted when model update state has changed (but not emitted when signalElementModified has already been emitted)\n
		 * <b>bool parameter:</b>			model updated state : true if updated, else false\n
		 */
		sigc::signal<void, bool > signalModelUpdated() { return m_signalModelUpdated ; }
		/**
		 * inhibition of signals signalModelUpdated  and signalElementModified upon data model modification
		 * @param b true to force inhibition of signal signalModelUpdated, else false
		 */
		void setInhibateSignals(bool b) { m_inhibateSignals = b; }
		/** @return inhibateSignals state true / false */
		bool getInhibateSignals() { return m_inhibateSignals; }

		/**
		 * inihibate undo-redo mechanism
		 * @param b true to inhibate, false to restore
		 */
		void setInhibUndoRedo(bool b) { m_inhibUndoRedo = b ; }
		/** @return inhibUndoRedo state true / false */
		bool getInhibUndoRedo() { return m_inhibUndoRedo ; }
		//@}


		/**
		 *  @name miscelleanous queries  : overlaps, stats, formatted text
		 */
		//@{
		/*! return true if segment id1 and id2 overlap each other */
		bool overlaps(const string& id1, const string& id2, float min_over=0.001);
		/*! return true if segment id1 and id2 overlap each other */
		bool overlaps(const string& id1, float so, float eo, float min_over=0.001);
		/*! @todo to be moved to stats helper - do not use  */
		void getOverlappingSegmentsIds(const SignalSegment& s, set<string>& v, const string& type, bool strict=false);


		/**
		 * trick to avoid AG graph deletion when using data model in agfio plugins
		 * @param k set to true to keep AGSet when deleting data model, false to delete it
		 * @note this function is normally only used only in formats handling plugins
		 */
		void setKeepAG(bool k) { m_keepAG=k; }
		/*! @return true if AGSet kept upon data model deletion, else false */
		bool getKeepAG() { return m_keepAG; }

		/*  for debug purpose 	 */
		string print_annotation(const string& id, bool with_offset) ;
		string print_anchor(const string& id) ;

		//@}

		/**
		 * Returns the first base type element from graphe graphtype with order 0 that starts at anchor id
		 * @param anchorid		Anchor id
		 * @param graphtype		Normalized graph name
		 * @return				first base type element from graphe with order 0
		 */
		std::string getMainstreamStartingAtAnchor(const string& anchorid, const string& graphtype) ;

		/**
		 * Return the first element of given type and given order starting at given anchor
		 * @param anchorid		Anchor id
		 * @param type			Element type
		 * @param order			Order (0 by default)
		 * @return				First element found in graphe
		 */
		std::string getElementStartingAtAnchor(const string& anchorid, const string& type, int order=0) ;

		// PROTECTED MEMBERS
	protected:
		/*!  object initialization */
		void init();

		/* add anchor to track map */
		/**
		 * add anchor to track correspondance map
		 * @param anchorId anchor id
		 * @param notrack corresponding track no
		 */
		void addAnchorToTrackMap(const string& anchorId, int notrack) ;


		/*!  wrapper for SplitAnchor method; also updates m_anchorTrack map */
		string splitAnchor(const string& anchorid);

	public:
		/**
		 * Sets anchor offset in signal
		 * @param anchorid 			Anchor id
		 * @param offset 			New signal offset
		 * @param use_epsilon 		Use epsilon value when checking new offset vs previous offset
		 * @param emit_signal 		If true, emit signalElementModified for all connected elements
		 * @return 					True if update performed, else false
		 */
		bool setAnchorOffset(const std::string& anchorid, float offset, bool use_epsilon=true, bool emit_signal=true);

		/**
		 * Sets anchor offset in signal
		 * @param anchorid 			Anchor id
		 * @param emit_signal 		If true, emit signalElementModified for all connected elements
		 * @return 					True if update performed, else false
		 */
		bool unsetAnchorOffset(const std::string& anchorid, bool emit_signal=true);


	protected:
		/**
		 * Emission of signal signalModelUpdated upon data model modification
		 * @param type		Updated type
		 * @param id		Updated id
		 * @param upd		Update type
		 */
		void emitSignal(const std::string& type, const std::string& id, UpdateType upd) ;
		/**
		 * Emission of signal signalModelUpdated upon data model modification
		 * @param id		Updated annotation id
		 * @param upd		Update type
		 */
		void emitSignal(const std::string& id, UpdateType upd) ;


	public:

		/**
		 * merge 2 annotations with same type by deleting one and overwritting / merging properties
		 * @param  baseId 				Id of annotation to be kept
		 * @param  mergedId 			Id of annotation to be merged
		 * @param  append  				If false, merged properties replace base properties values, else they are appended to base properties values
		 * @param  delayed_del 			If true, merged annotation will not be deleted
		 * @param  replaceAnchor		If true, will adjust attachments
		 * @param  emit_signal			If true, will emit an UPDATED signal for baseId
		 * @return 						deleted Annotation id
		 *
		 * @note if base annotation properties that do not exists in merged annotation are left unchanged
		 * @note merged annotation is deleted, as well as its start anchor if unused, and base annotation end is set to merged end.
		 */
		string mergeAnnotations(const string& baseId, const string& mergedId, bool append=true, bool delayed_del=false, bool replaceAnchor=false, bool emit_signal=false);

		/**
		 * merge 2 annotations with same type by deleting one and overwritting / merging properties
		 * @param  baseId 				Id of annotation to be kept
		 * @param  mergedId 			Id of annotation to be merged
		 * @param  lock_features		Vector containing all features that won't be merged
		 * @param  append  				If false, merged properties replace base properties values, else they are appended to base properties values
		 * @param  delayed_del 			If true, merged annotation will not be deleted
		 * @param  replace_anchor		If true, will adjust attachments
		 * @param  emit_signal			If true, will emit an UPDATED signal for baseId
		 * @return 						deleted Annotation id
		 */
		string mergeAnnotations(const string& baseId, const string& mergedId, const vector<string>& lock_features, bool append, bool delayed_del, bool replace_anchor, bool emit_signal) ;

		/**
		* add new mainstream element to a continuous graph; new element will split existing one at signal_offset
		* @param type new element type
		* @param prevId existing element to split
		* @param signal_offset new anchor offset in signal timeline / -1 if unanchored
		* @param needSplit	If false, base mainstream will be splitted only if required
		* @param emit_signal if true, emits signalElementModified signal for new inserted id, with signal type=INSERTED
		* @param forward_qual_attachments If true, will adjust attachments
		* @return new element id / "" if error occured
		*/
		std::string insertMainstreamElement(const string& type, const string& prevId, float signal_offset, bool needSplit=true, bool emit_signal=true, bool forward_qual_attachments=true);

		/**
		 * Adds a new non-text foreground element to a continuous graph between two new anchors.
		 * @param prevId				Existing mainstream base-type element to split
		 * @param start_offset			New start anchor offset in signal timeline / -1 if unanchored
		 * @param end_offset			New end anchor offset in signal timeline / -1 if unanchored
		 * @param value					Annotation value (type for an event)
		 * @param subvalue				Annotation subtype value (desc for an event)
		 * @param fwd_qualifier			Adjust qualifiers attached to mainstream
		 * @param emit_signal 			If true, emits signalElementModified signal for new inserted id, with signal type=INSERTED
		* @return 						New element id / "" if error occured
		 */
		std::string insertEventMainstreamElement(const string& prevId, float start_offset, float end_offset, const string& value, const string& subvalue, bool fwd_qualifier, bool emit_signal) ;

		/**
		 * Inserts a new mainstream event element
		 * @param prevId				Existing mainstream base-type element to split
		 * @param signal_offset			New start anchor offset in signal timeline / -1 if unanchored
		 * @param value					Type of foregournd event
		 * @param subvalue				Subtype of foreground event
		 * @param fwd_qualifier			Adjust qualifiers attached to mainstream
		 * @param emit_signal 			If true, emits signalElementModified signal for new inserted id, with signal type=INSERTED
     	 * @return 						New element id / "" if error occured
		 */
		std::string insertEventMainstreamElement(const string& prevId, float signal_offset, const string& value, const string& subvalue, bool fwd_qualifier, bool emit_signal) ;

		/**
		* Adds new anchored mainstream base-type element to a continuous graph; new element will split existing one at signal_offset
		* @param prevId 				Existing mainstream base-type element to split
		* @param signal_offset 			New anchor offset in signal timeline / -1 if unanchored
		* @param forward_attached_qualifiers 		If true, any qualifier attachments will be forwarded to new element's end
		* @param emit_signal 			If true, emits signalElementModified signal for new inserted id, with signal type=INSERTED
		* @return 						New element id / "" if error occured
		*/
		std::string insertMainstreamBaseElement(const string& prevId, float signal_offset, bool forward_attached_qualifiers=true, bool emit_signal=true);

		/**
		 * Deletes a mainstream event
		 * @param id				Mainstream event id
	 	 * @param emit_signal 		If true, emits signalElementModified signal for new inserted id, with signal type=DELETED
		 * @return					True for success, False otherwise
		 */
		bool deleteEventMainstreamElement(const string& id, bool emit_signal) ;

		/**
		* adds new overlapping element to a continuous graph upon existing one
		* @param overId existing mainstream element to overlap
		* @param emit_signal if true, emits signalElementModified signal for new inserted id, with signal type=INSERTED
		* @return new element id / "" if error occured
		*/
		std::string insertOverlappingElement(const string& overId, bool emit_signal=true);
		/**
		* add new mainstream text-type element to a continuous graph; new element will split existing one, its text contents will be split at given offset.
		* @param textId existing mainstream text-type element to split
		* @param text_offset text offset where to split
		* @param signal_offset 			New anchor offset in signal timeline / -1 if unanchored
		* @param forward_attached_qualifiers 		If true, any qualifier attachments will be forwarded to new element's end
		* @param emit_signal if true, emits signalElementModified signal for new inserted id, with signal type=INSERTED
		* @return new element id / "" if error occured
		*/
		std::string splitTextMainstreamElement(const string& textId, float text_offset, float signal_offset=-1, bool forward_attached_qualifiers=true, bool emit_signal=true);

		/**
		* add new mainstream parent element to a continuous graph; new element will split existing one at child element start
		* @param 	childId 				Existing mainstream child type element where to split
		* @param 	emit_signal 			If true, emits signalElementModified signal for new inserted id, with signal type=INSERTED
		* @return 							New element id / "" if error occured
		*/
		std::string insertParentElement(const string& childId, bool emit_signal=true);

		/**
		 * split text annotation contents between given annotation id and following element.
		 * @param prevId current text id
		 * @param text_offset offset where to split (in chars)
		 * @param emit_signal true if element modified signal to be emitted.
		 */
		void splitTextContent(const string& prevId, int text_offset, bool emit_signal);


		/**
		 * re-attach qualifier annotations to id start if cannot span over this type of element
		 * @param id 	annotation 				Id
		 * @param forward_attached_qualifier 	If true and cannot span over this type of element,
		 * 										then split annotation at inserted element start, else reattach end to id's start anchor
		 * @param emit_signal 					If true emit signalUpdated
		 */
		void applyQualifiersSpanRules(const string& id, bool forward_attached_qualifier=true, bool emit_signal=true);

		/**
		 * Gets all available annotation types for all graphes found in current model
		 * @param checkInModel		True for only getting types used in model, false for all types associated
									with graphes in conventions
		 * @return					All available annotation types for given graphes and option
		 * @warning					This method checks for graphes existing in model, so even if the
									conventions specify a graph, if this graph has not been created in
									model, the associated type won't be returned
		 */
		std::vector<string> getAnnotationTypes(bool checkInModel) ;

		/**
		 * returns true if id is first child of parent (have the same start anchor)
		 */
		bool isFirstChild(const string& id, const string& parent = "") ;

		/**
		 * returns true if id is last child of parent (have the same end anchor)
		 */
		bool isLastChild(const string& id, const string& parent = "") ;

		/**
		 * Checks if the given annotation has some parent
		 * @param id		Annotation id
		 * @param parent	If empty it will check that id is the unique children of
		 * 					another annotation, otherwise will check if id it the
		 * 					unique children of parent
		 * @return			True or false
		 */
		bool isUniqueChild(const string& id, const string& parent = "") ;

		/**
		 * Print method for annotation
		 * @param id		Annotation id
		 * @return			Annotation data
		 * @note			Debug purpose
		 */
		string toString(const string& id) ;

		/**
		 * Gets the start or end anchor id of the given annotation
		 * @param id		Annotation id
		 * @param start		True for start anchor, false for end anchor
		 * @return			Anchor id
		 */
		string getAnchor(const string& id, bool start) ;

		/**
		 * Sets the start or end anchor id of the given annotation
		 * @param id			AnnotationId
		 * @param anchorId		Anchor id
		 * @param start			True for start anchor, false for end anchor
		 */
		void setAnchor(const string& id, const string& anchorId, bool start) ;

		/**
		 * Creates a new anchor if none exist at same offset and same track, otherwise
		 * uses the existing one.
		 * @param graphId		Graph id
		 * @param notrack		Track number
		 * @param offset		Timecode
		 * @param force			True for forcing creation
		 * @return				Anchor id (new one if none existed or if force is set to True,
		 * 						existing one otherwise)
		 */
		string createAnchorIfNeeded(const string& graphId, int notrack, float offset, bool force=false) ;

		/**
		 * Returns the offset of the given anchor
		 * @param id		Anchor id
		 * @return			Timeocde
		 */
		float getAnchorOffset(const std::string& id);

		/**
		 * Gets the anchor matching the given track number and timecode
		 * @param graphId		Graph id
		 * @param notrack		Track number
		 * @param offset		Timecode
		 * @return				Anchor id or empty if nothing found
		 */
		string getAnchorAtOffset(const string& graphId, int notrack, float offset) ;

	/** PRIVATE METHODS **/
	private:
		typedef pair<string, DataModel::UpdateType> UpdateAction;

		/**
		 * Adds new anchored mainstream base-type element to a continuous graph; new element will split existing one at signal offsets
		 * @param prevId 				Existing mainstream base-type element to split
		 * @param start_offset			New start anchor offset / -1 if unanchored
		 * @param end_offset			New end anchor offser/ -1 if unanchored
		 * @param forward_attached_qualifiers 		If true, any qualifier attachments will be forwarded to new element's end
 		 * @param emit_signal 			If true, emits signalElementModified signal for new inserted id, with signal type=INSERTED
		 * @return 						New element id / "" if error occured
		 */
		std::string insertMainstreamBaseElement(const string& prevId, float start_offset, float end_offset, bool forward_attached_qualifiers, bool emit_signal) ;

		void mergeAnnotationAnchors(const string& baseId, const string& mergedId, bool withOffset) ;

		/* initialisation */
		void initGraphsMainstream(int start_track=0, const string& graphtype="");
		void setAGOptions(string format="TransAG", bool savemode=false);

		/* clear graph */
		void deleteAGElements();

		/* io utilities */
		/* to allow multiple file in memory */
		string fixAGSetId(const string& path, const string& old_id, string& new_id, const string& outpath="") throw (const char*);

		/* guess graph type */
		std::string guessGraphType(const string& agId) ;

		/* treatment specific to a type of graph - proceded after checking */
		void graphSpecificTreatment(const string& graphtype) ;

		/* get version information if exist */
		void loadVersionData() ;

		/* checks whether all graphs defined in convention exist in file - add them if needed */
		int actualizeGraphForConvention() ;

		/* store topics list as XML-formatted AGSet property */
		void storeTopicsList() throw (const char*) ;
		/* store speakers as XML-formatted AGSet property */
		void storeSpeakersDict(bool cleanup_unused=false);
		/* store anchorlinks list as XML-formatted AGSet property */
		void storeAnchorLinks() throw (const char*) ;

	    void loadSignalsFromFile(const set<SignalId>& sigIds) ;
		/* utility : update anchor-notrack associative table */
		void updateAnchorTrackMap(const string& graphId, std::map<string, int>& anchorTrack);

		/* set segment end anchor to new anchor */
		string setEndAnchor(const string& id, const string& new_end);
		bool checkRemainingMinsize(const string& type, int order, const string& start, float start_offset, string& diag);
		bool anchorHasOtherAttachedElements(const string& anchor, const string& id);

		/* links checking */
		bool checkResizeRulesForLinks(const string& id, float start_offset, float end_offset, string& diag, bool check_over, bool allowUnanchored) ;

		/* background segment management rules */
		std::string addBackground(string prevId, const string& subtype, const string& level, int notrack, float signal_offset, int text_offset, bool at_end, bool emit_signal, bool active, bool check_active);
		std::string addBackgroundBySelection(const string& subtype, const string& level, int notrack,float start, float stop_offset, bool emit_signal, bool active) ;
		bool checkElementFrontiersResizable(const string& id, float new_start, float new_end, string& err) ;

		/* browse through graph */
		void getAnchorAttachments(const string& anchorid, set<string>& att, bool with_qualifiers=true, bool with_incoming=true);
		string getAnnotationByOffset(const string& graphtype, float offset, const string& type) ;
		/* eventually create new anchor if no one exists at given offset for given graph */
//		string createAnchorIfNeeded(const string& graphId, int notrack, float offset, bool force=false);
		/* remove anchor if no more annotations linked to it */
		void deleteUnusedAnchor(const std::string& anchor);
		/* return next segment with start offset greater than or equal to given offset for given type and track	 */
		float getNextSegmentStartOffset(const std::string& type, int notrack, float offset);
		/* returns given anchor offset */

		/* overlapping annotations utilities */
		bool terminateOverlappingBranches(const string& parent_id, const string& align_id, vector<UpdateAction>& updates);
		bool resetOrderOnBranch(const string& startId, const string& endAnchor,  vector<UpdateAction>& updates);

		string getOverlapType(const string& id);

		Parameters* loadQualifierMapping(string format) ;
		void actualizeQualifierMapping(string format) ;

		/** If language of transcription graph exists, then use it and save it in AGSET language (for old files compatibility)
		 *  Else, returns AGSET language
		 * @return		file annotation language
		 * @attention	This method uses getGraphProperty and getTranscriptionProperty methods,
		 * 				so the AGID map of graphs should be initialized before calling method.
		 */
		string fixLangBackwardCompatibility() ;

		void applyHints(const string& id, string type) ;

		void getElementsBetweenAnchors(vector<string>& dest, const string& type, AnchorId astart, AnchorId aend, int order);


		// PRIVATE MEMBERS
	private:
		/* Track associated to each anchor
		 *  Only for timed offset anchor
		 */
		std::map<string, int> m_anchorTrack ;
		bool m_inhibateSignals; /** inhibates signalling upon data modification */
		bool m_inhibUndoRedo ; /** inhibates undoRedo processing */

		ModelChecker* 	m_checker ;		/**< Checker module */
		bool 	m_inhibateChecking; 	/**< if true then inhibate validation checks while updating model (eg qualifier span rules */
		string	m_graphType;		/**< default graph type */
		string m_import_format ;

		bool m_updated;      /** true if data model has been updated */

		Parameters* 	m_qualifierMapping ; /** mapping qualifier file **/

		std::string		m_path;			/** current file name */
		std::string		m_agsetId;		/** current corpus name */
		std::string		m_tmpId;		/** temporary corpus name (aglib duplicates) */
		std::string		m_savId;		/** ag file original corpus name (aglib duplicates) */
		std::string		m_defaultCorpusName;  /** default corpus name */
		std::string		m_timelineId;	/** current timelineId  */
		std::string 	m_localDTD;		/**< path to local DTD */
		std::map <std::string, std::string> m_agOptions;	/**< AG storage options */
		vector<string>	m_vlang;		/**< multi-track file -> per-track language info */

		int m_anchorTrackHint;			/**< for undo/redo handling */
		Conventions m_conventions;    /**< current annotation conventions */

		std::map<std::string, std::string> m_agIds; /** graph type - graph id  */
		std::map<std::string, std::string> m_agBaseType; /** graph id  - graph base type */
		std::map<int, std::map<std::string, std::string> > m_featureHints; /** feature hints per signal track */

		SignalConfiguration signalCfg ;

		SpeakerDictionary m_speakersDict;  	/**< associated speakers dictionnary */
		sigc::connection m_speakerUpdatedConnection ;

		AnchorLinks m_anchorLinks ; /**< Links between anchors from different graphes */

		float m_signalLength;	/** signal length */

		VersionList	    m_versions;  /**< version ids */
		string 		m_lastVersion;  /**< last version id */
		bool m_keepAG ; /** if set too true, then do not delete AG in DataModel destructor */

		// signals emitted when data structure updated
		sigc::signal<void, std::string, std::string, UpdateType > m_signalElementModified ;

		// signal emitted at model state changing
		sigc::signal<void, bool> m_signalModelUpdated ;

		 /** true when closing action has begun - used for prevent for dangerous signal to already closed parts */
		bool closing ;
		static bool _initEnvironDone;


	public:
		/**
		 *  initialize execution environment for current program so that AG plugins can be dynamically loaded
		 * @param progname current program path
		 *
		 * @note if LD_LIBRARY_PATH environment variable is not set, then set it to
		 *      - [program_parent_dir]/../libs/ag  if exists
		 *      - /usr/local/lib/ag if exists
		 */
		static void initEnviron(string progname);

		/**
		 * make unique temporary AGSet id
		 * @return AGSet id
		 */
		static string mktmpAGSetId();

		/**
		 * Removes anchor toBeRemoved from anchor anchorId links.
		 * This method is mainly dedicated to be used by AnchorLinks class.
		 * @param anchorId			Anchor identifier
		 * @param toBeRemoved		Anchor identifier to be removed from anchorId links
		 */
		virtual void tagRemoveFromAnchorLinks(const string& anchorId, const string& toBeRemoved) { m_anchorLinks.insertIntoLinks(anchorId,toBeRemoved); }

		/**
		 * Inserts anchor toBeInserted into anchor anchorId links.
		 * This method is mainly dedicated to be used by AnchorLinks class.
		 * @param anchorId			Anchor identifier
		 * @param toBeInserted		Anchor identifier to be inserted inside anchorId links
		 */
		virtual void tagInsertIntoAnchorLinks(const string& anchorId, const string& toBeInserted) { m_anchorLinks.removeFromLinks(anchorId,toBeInserted); }

	protected:

		/**
		 * @name Undoable Data Model interfaces
		 */
		//@{
		/**
		 * some AGAPI interfaces -> may be redefined for undo/redo
		 */
		virtual AnnotationId agCreateAnnotation(const Id& id, const AnchorId& start, const AnchorId& end, const string& type)
		{ return CreateAnnotation(id, start, end, type); }
		virtual void agDeleteAnnotation(const AnnotationId& id) { DeleteAnnotation(id); }
		virtual AnnotationId agCopyAnnotation(const AnnotationId& id, const AnnotationId& toBeCreated="") { return CopyAnnotation(id); }
		virtual list<AnnotationId> agSplitAnnotation(const AnnotationId& id, const AnnotationId& toBeCreated, AnchorId& anchorToBeCreated){ return SplitAnnotation(id); }
		virtual void agSetFeature(const AnnotationId& id, const string& key, const string& val) { SetFeature(id, key, val); }
		virtual void agDeleteFeature(const AnnotationId& id, const string& key)  { DeleteFeature(id, key); }
		virtual AnchorId agCreateAnchor(const Id& id) { return CreateAnchor(id); }
		virtual AnchorId agCreateAnchor(const Id& id, float offset, string unit, set<SignalId>& sigIds) { return CreateAnchor(id, offset, unit, sigIds); }
		virtual AnchorId agSplitAnchor(const AnchorId& id, const AnchorId& toBeCreated="") {
			const string& newid = SplitAnchor(id);
			m_anchorTrack[newid] = m_anchorTrack[id];
			return newid;
		}
		virtual void agDeleteAnchor(const Id& id) { return DeleteAnchor(id); }
		virtual void agSetStartAnchor(const AnnotationId& id, const AnchorId& a) { SetStartAnchor(id, a); }
		virtual void agSetEndAnchor(const AnnotationId& id, const AnchorId& a) { SetEndAnchor(id, a); }
		virtual void agSetAnchorOffset(const AnchorId& id, float f) { SetAnchorOffset(id, f); }
		virtual void agUnsetAnchorOffset(const AnchorId& id) { UnsetAnchorOffset(id); }
		virtual void agSetStartOffset(const AnnotationId& id, float offset) { SetStartOffset(id, offset); }

		/**
		 * set anchor track hint - undo/redo operations
		 * @param notrack notrack hint
		 */
		void setAnchorTrackHint(int notrack) { m_anchorTrackHint = notrack; }
		/**
		 * get anchor track hint - undo/redo operations
		 * @return notrack hint
		 */
		int getAnchorTrackHint() { return m_anchorTrackHint; }
		//@}

		// friend classes
		friend class DataModel_CPHelper;
//		friend class DataModel_IO;

		string _novalue;
		string _currentDTDVersion;	// current DTD version
};

} // namespace tag

#endif /* _HAVE_DATA_MODEL_H */
