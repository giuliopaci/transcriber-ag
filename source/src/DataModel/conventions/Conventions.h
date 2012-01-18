/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/


#ifndef _HAVE_CONVENTIONS_H
#define _HAVE_CONVENTIONS_H 1

#include <string>
#include <vector>
#include <set>
#include <list>
#include <glibmm.h>
#include <glibmm/objectbase.h>

using namespace std;

#include "DataModel/conventions/WordList.h"
#include "DataModel/conventions/Topics.h"
#include "DataModel/speakers/SpeakerDictionary.h"
#include "DataModel/signals/SignalSegment.h"
#include "DataModel/versions/VersionList.h"

#include "Common/Encoding.h"
#include "Common/Parameters.h"

extern const utils::Encoding* utf8;

namespace tag {

#define WORD_TYPE "unit"

/**
 *  @class Conventions
 *  @ingroup DataModel
 *  annotation conventions manager for transcriber DataModel
 */

class Conventions
{

	public:
		/**
		*  @class 		TypeDescriptor
		*  @ingroup		DataModel
		*
		*  GraphType structure.\n
		*  Used by Conventions class
		*/
		class TypeDescriptor : public string
		{
			public:
				/**
				 * Constructor
				 */
				TypeDescriptor() : string("") { init(); };
				/**
				 * Copy constructor
				 * @param s	Type string
				 */
				TypeDescriptor(const string& s) : string(s) {init(); }
				/**
				 * Copy constructor
				 * @param t TypeDescriptor reference
				 */
				TypeDescriptor(const TypeDescriptor& t) : string(t)
				{
					isMainstream = t.isMainstream;
					maxOverlap = t.maxOverlap;
					maxOrder = t.maxOrder;
					isAnchored = t.isAnchored;
					parentType = t.parentType;
					childType = t.childType;
					typeClass = t.typeClass;
					canSpanOver = t.canSpanOver;
					features = t.features;
					submain = t.submain;
				}
				/**
				 * Init type structure
				 */
				void init ()
				{
					isMainstream = false;
					maxOverlap = 0;
					maxOrder = 0;
					isAnchored = false;
					parentType="";
					childType = "";
					typeClass = "";
					canSpanOver = "";
				}

				TypeDescriptor& operator = (const string& s) { *(string*)this = s; return *this; }

				bool isMainstream;	/**< is mainstream type */
				bool isAnchored;	/**< is anchored */
				int maxOverlap;   	/**< >0 if overlap allowed for given type, else 0 */
				int maxOrder ;		/**< >0 if can be in overlapped branch, else 0 */
				string parentType;	/**< type of parent elements */
				string childType;	/**< type of child elements */
				string typeClass;	/**< mainstream/event/entity */
				string canSpanOver;	/**< qualifiers -> can span over mainstream type / "" */
				std::map<std::string, std::string> features; /**< type features */
				map<string, string>  submain ; /**< mainstream subtypes**/
		};

		/**
		 * @class GraphDescriptor
		 * annotation graph descriptor items
		 */
		class GraphDescriptor
		{
			public:
				vector<string>	mainstream ; /**< mainstream types */
				set<string> spannables; /** < mainstream types that can be spanned by qualifiers */
				vector<string> qualifiers; /**< qualifier types */
				vector<string> signalClass; /**< signal mime classes on which graph applies */
				map<string, vector<string> > qualifierClass;  /**< qualifiers classes if any */
				string type;				/**< graph type */
				string segmentationBaseType; /**< timeline segmentation base type */
				string speakerType;			/**< type to which speaker meta is attached (transcription_graph) */
				bool isContinuous;			/**< is continuous graph true / false */
				bool resizeKeepAttachments;	/**< keep attachments when resizing segments */
				bool overlapAlignment;		/**< force alignment of overlapping segments on existing ones */
				int maxOverlap;			/**< max overlap within all mainstream types */
				map<string, TypeDescriptor> types;


				/* DEBUG
				static string toString(GraphDescriptor g, string gtype)
				{
					string res = "\nGRAPH " + gtype ;
					res.append("\n type=" + g.type) ;
					res.append("\n baseSegment=" + g.mainstreamBaseType) ;
//					res.append("\n continuous=" + g.isContinuous) ;
//					res.append("\n resizeKeepAttachements=" + g.resizeKeepAttachments) ;
//					res.append("\n overlapAlignment=" + g.overlapAlignment) ;
//					res.append("\n") ;
//					res.append("\nMainstreams:") ;
//					std::vector<string>::iterator it ;
//					for (it=g.mainstream.begin(); it!=g.mainstream.end(); it++) {
//						res.append("\n\t" + *it) ;
//					}
//					res.append("\n") ;
//					res.append("\nQualifiers:") ;
//					for (it=g.qualifiers.begin(); it!=g.qualifiers.end(); it++) {
//						res.append("\n\t" + *it) ;
//					}
//					res.append("\n") ;
//					res.append("\nSignalTypes:") ;
//					for (it=g.signals.begin(); it!=g.signals.end(); it++) {
//						res.append("\n\t" + *it) ;
//					}
					return res ;
				}
				 */
		};

		typedef map<string, GraphDescriptor>::const_iterator GraphDescriptorIter;
		typedef map<string, TypeDescriptor>::const_iterator TypeDescriptorIter;


	public:

		/*! @name Constructor / Destructor  	 */
		//@{
		Conventions(); /**< default constructor */

		~Conventions() {} /**< destructor */
		//@}


		const string& name() { return m_conventions ; } /**< @return conventions name */
		const string& path() { return m_currentFile ; } /**< @return conventions file path */
		const string& version() { return m_version ; } /**< @return conventions version */
		const string& getDirectory() { return m_configDir ; }  /**< @return conventions configuration directory */
		const string& getCorpusName() { return m_corpusName ; } /**< @return conventions default corpus name */
		const string& getCorpusVersion() { return m_corpusVersion ; }  /**< @return conventions default corpus vesrion */


		/**
		 * add graph description to current conventions (mainly for alignment data management purpose)
		 * @param graphtype graph type
		 * @param param graph description
		 * @return true if added, else false
		 */
		bool addGraphDescription(const string& graphtype, Parameters* param) ;

		/**
	    * 	Configure conventions for current annotation file
		* @param conventions conventions file used for annotation
		* @param lang  transcription language
		* @param fullmode true for complete loading (i.e labels menu, labels translation, etc..)
		* 		In most case should be set to true
		*/
		void configure(string conventions="", string lang="", bool fullmode=true);

		/**
		 * @return true if conventions already loaded
		 */
		bool loaded() { return ! m_currentFile.empty() ; }
		/**
		 * check if conventions contain given graph type definition
		 * @param graphtype checked graph type
		 * @return true if conventions contain given graph type, else false
		 */
		bool hasGraphType(const string& graphtype) { return  m_graphDesc.find(graphtype) !=  m_graphDesc.end(); }

		/**
		 * Indicates whether a graph uses a video signal, or if the conventions only
		 * use audio signal
		 * @return		True if at least one graph is applied on a video signal, False otherwise
		 */
		bool usesVideoSignal() { return m_usesVideoSignal; }

		/**
		 * @return graph types defined in conventions
		 */
		const vector<string>& getGraphTypes() { return m_graphTypes; }
		/**
		 * @return graph description map
		 */
		const map<string, GraphDescriptor>& getGraphDescriptors() { return m_graphDesc; }

		/**
		 * get qualifier annotation types for given graph type for current conventions
		 * @param qclass	 qualifier class
		 * @param graphtype	 graph type
		 * @return vector of mainstream annotation types
		 */
		const vector<string>& getQualifierTypes(const string& qclass="", const string& graphtype="transcription_graph");

		/**
		 * return base mainstream annotation type for current graph type
		 * @param graphtype
		 * @return base segment type
		 */
		const string& mainstreamBaseType(const string& graphtype="transcription_graph");

		/**
		 * Checks whether the given subtype is a valid subtype for the mainstream basetype
		 * of the given graphtype
		 * @param		subtype			Subtype
		 * @param		graphtype		Graphtype
		 * @return		True or false
		 */
		bool isMainstreamBaseSubtype(const string& subtype, const string& graphtype) ;

		/**
		 * get mainstream annotation types for given graph type for current conventions
		 * @param graphtype	 graph type
		 * @return vector of mainstream annotation types
		 */
		const vector<string>& getMainstreamTypes(const string& graphtype="transcription_graph") ;

		/**
		 * return base segmentation annotation type (ie defining a timeline segmentation) for current graph type
		 * @param graphtype
		 * @return base segment type
		 */
		const string& segmentationBaseType(const string& graphtype="transcription_graph") ;
		/**
		 * check if graph type is defined as continuous (all annotations must link together) or discrete
		 * @param graphtype
		 * @return true if continuous, else false
		 */
		bool isContinuousGraph(const std::string& graphtype) ;

		/**
		 * get qualifier class name (eg "qualifier" / "entity") for given qualifier type in given graph type
		 * @param type qualifier type
		 * @param graphtype graph type
		 * @return qualifier class name
		 */
		const std::string& getQualifierClass(const string& type, const std::string& graphtype="transcription_graph");

		/**
		 * @return configuration parameters map
		 */
		const std::map<std::string, std::string>& getConfiguration() const { return m_config; }

		/**
		 * get configuration option value
		 * @param key option key
		 * @return option value
		 */
		const std::string& getConfiguration(const std::string& key) const ;

		/**
		 * set configuration option value
		 * @param key option key
		 * @param value option value
		 */
		void setConfiguration(const std::string& key, const std::string& value) { m_config[key]=value; }
		/**
		 * @return list of predefined words lists (eg. onomatopoeia)
		 */
		const std::list<WordList>& getPredefinedWordlists() { return m_wordLists; }
		/**
		 * @return topics map
		 */
		std::map<Glib::ustring,Topics*>& getTopics() { return m_Topics ; }
		/**
		 * get localized label for given convention item
		 * @param item item name
		 * @return localized label
		 */
		string getLocalizedLabel(const string& item);
		/**
		 * get items for given annotation type and subtype
		 * @param[out] 	items 		Vector of items names
		 * @param 		type 		Target annotation type
		 * @param 		itemtype 	Target item type
		 */
		void getAnnotationItems(vector<string>& items, const string& type, const string& itemtype="subtypes");

		/**
		 * get layout definition for given layout component.
		 * Layouts allow to customize( and localize) some conventions elements display.
		 * They come with conventions definitions, and are optional.
		 * <br> Layout component can be
		 * - "Labels" : labels for annotation items (mainly for messages display)
		 * - "Menu" : menu labels for annotation items
		 * - "Layout" : layout format for textual display in annotation editor.
		 * - "Colors" : foreground and background colors for textual display in annotation editor.
		 * @param component layout component
		 * @return layout component definition map
		 */
		const std::map<std::string, std::string>& getLayout(const string& component)
			{ return m_layout.getParametersMap(component); }
		/**
		 * get layout item value (see above)
		 * @param component layout component
		 * @param item annotation item
		 * @param suffix layout item suffix (eg "fg" or "bg" for colors)
		 * @return
		 */
		const std::string& getLayoutItem(const string& component, const string& item, const string& suffix="");

		/**
		 * get item type on which given item type must be aligned
		 * (eg. "turn" is aligned on "segment", which means it must start and stop on a segment start)
		 * @param type item type
		 * @return alignment item type / "" if no alignment
		 */
		const string& getAlignmentType(const string& type) ;

		/**
		 * get item type on which given item type must be aligned
		 * @param type			Item type
		 * @param graphtype		Graphtype
		 * @return alignment item type / "" if no alignment
		 */
		const string& getAlignmentType(const string& type, const string& graphtype) ;

		/**
		 * get parent type for given annotation type :
		 * - mainstream type : upper level mainstream type
		 * - qualifier type : base segment type for graph type.
		 * @param type item type
		 * @param graphtype  graph type
		 * @return  parent type
		 */
		const string& getParentType(const string& type, const string& graphtype="transcription_graph");
		/**
		 * get direct child type for given annotation type :
		 * - mainstream type : lower level mainstream type
		 * - qualifier type : ""
		 * @param type item type
		 * @param graphtype  graph type
		 * @return  child type
		 */
		const string& getChildType(const string& type, const string& graphtype="transcription_graph");

		/**
		 * Checks if given annotation type is a mainstream type for the given graph type.
		 * (if no graph type is specified, the method will check in all conventions graphs)
		 * @param type 			Annotation type
		 * @param graphtype 	Graph type, or empty for all graphes
		 * @return 				True if it is mainstream type, false otherwises
		 */
		bool isMainstreamType(const string& type, const string& graphtype);

		/**
		 * Checks if some qualifier annotations may span over this type
		 * @param type 			Annotation type
		 * @param graphtype 	Graph type
		 * @return 				True if can be spanned over, else false
		 */
		bool isSpannableType(const string& type, const string& graphtype="transcription_graph");

		/**
		 * check if given annotation type is defined as qualifier type for given graph type
		 * @param type annotation type
		 * @param graphtype graph type
		 * @return true if is qualifier, else false
		 */
		bool isQualifierType(const string& type, const string& graphtype="transcription_graph");

		/**
		 * Checks whether the given type is a valid type in conventions
		 * @param type			Candidate type
		 * @param graphtype		Graph type
		 * @return				True if the given type exists in convention, false otherwise
		 */
		bool isValidType(const string& type, const string& graphtype) ;

		/**
		 * Checks whether the given subtype is a valid subtype for the given type
		 * @param subtype		Candidate subtype
		 * @param type			Annotation type
		 * @param graphtype		Graph type
		 * @return				True if the given subtype is correct, False otherwise
		 */
		bool isValidSubtype(const string& subtype, const string& type, const string& graphtype) ;

		/**
		 * Checks whether the given mainstream type has subtypes.
		 * @param 		type			Mainstream type
		 * @param 		graphtype		Graphe type
		 * @param[out] 	subtypes		Vector of subtypes
		 * @return						False if no subtype were found or if the given type is not a mainstream
		 */
		bool mainstreamHasSubtypes(const string& type, const string& graphtype, std::vector<std::string>& subtypes) ;

		/**
		 * Checks whether the annotation defined by the given data is a valid mainstream base
		 * @param graphtype		Graphe type
		 * @param submain		Subtype of mainstream basetype
		 * @param submain		Annotation main subtype
		 * @param value			Annotation value
		 * @param desc			Annotation description
		 * @return				 1: annotation is valid.\n
		 * 						-1:	submain is not valid for given type\n
		 * 						-2:	value is not valid for given submain\n
		 * 						-3:	desc is not valid for given value\n
		 */
		int isValidMainstreamBaseType(const string& graphtype, const string& submain, const string& value, const string& desc) ;

		/**
		 * check if given annotation type is defined as qualifier type of given qualifier class for given graph type
		 * @param qclass qualifier class ("qualifier" / "entity")
		 * @param type annotation type
		 * @param graphtype graph type
		 * @return true if is qualifier of given qualifier class, else false
		 */
		bool isQualifierClassType(const string& qclass, const string& type, const string& graphtype="transcription_graph") ;

		/**
		 *   (conventions validation) check if the qualifier defined by given type and subtype exists in conventions
		 *	 @param type type of qualifier
		 *	 @param desc subtype of qualifier
		 *	 @param graphtype graph type of annotation
		 *	 @param typeOK (returned) true if the type has been checked, false otherwise
		 *	 @param descOK (returned) true if the subtype has been checked, false otherwise. Note that
		 *	 	if type check fails, this returned value will always be false
		 *	 @return true if qualifier defined for current conventions, else false
		 */
		bool isQualifier(const string& type, const string& desc, const string& graphtype, bool& typeOK, bool& descOK) ;
		/**
		 * check if qualifier type is instantaneous
		 * <br> this is mainly for fast-transcription conventions
		 * @param type annotation type
		 * @return true if can auto-inserted, else false.
		 */
		bool isInstantaneous(const string& type);

		/**
		 * check if annotation type requires that annotations be signal anchored or not
		 * @param type annotation type
		 * @return true if requires that annotations start and end nodes are signal anchored, else false
		 */
		bool isAnchoredType(const std::string& type);

		/**
		 * Checks if annotation type requires that annotations be signal anchored or not
		 * @param type 			Annotation type
		 * @param graphtype		Graphtype
		 * @return 				True if requires that annotations start and end nodes are signal anchored, else false
		 */
		bool isAnchoredType(const std::string& type, const std::string& graphtype);

		/**
		 * compare 2 mainstream annotation types
		 * @param type annotation type
		 * @param ref reference annotation type
		 * @param graphtype graph type
		 * @return positive value if type has higher level than reference type, ; else negative
		 */
		int compareMainstreamLevel(const string& type, const string& ref, const string& graphtype="transcription_graph");
		/**
		 * compare 2 annotation types
		 * @param type annotation type
		 * @param ref reference annotation type
		 * @param graphtype graph type
		 * @return true if type has higher precedence than reference type, ; else false.
		 * A qualifier type has always lower precedence than a mainstream type
		 */
		bool isHigherPrecedence(const string& type, const string& ref, const string& graphtype="transcription_graph");
		/**
		 * checks if overlapping allowed for given annotation type
		 * @param type 			annotation type
		 * @param graphtype		annotation type
		 * @return true if can overlap, else false
		 */
		bool canOverlap(const string& type, const string& graphtype="");
		/**
		*  max overlapping layers allowed for given annotation type
		* @param type annotation type  / "" for all mainstream types
		* @param graphtype annotation graphtype  / "" to guess from type
		* @return max overlapping layers
		*/
		int maxOverlap(const string& type, const string& graphtype="");
		/**
		*  max possible order for given annotation type
		* @param type annotation type  / "" for all mainstream types
		* @param graphtype annotation graphtype  / "" to guess from type
		* @return max order
		*/
		int maxOrder(const string& type, const string& graphtype="");
		/**
		 * check if span over multiple mainstream elements allowed for given annotation qualifiers type
		 * @param qtype qualifier type
		 * @param mtype potentially spanned mainstream type type
		 * @return true if can span, else false.
		 */
		bool canSpanOverType(const string& qtype, const string& mtype);

		/**
		 * check if annotation type allows input of a normalized form of the tagged item (this concerns mainly entities)
		 * @param type annotation type
		 * @return true if normalized form input requested.
		 */
		bool typeCanBeNormalized(const string& type) ;
		/**
		 * check if subtype can be modified by user for given annotation type (qualifiers)
		 * @param type annotation type
		 * @return true if can be modified, false if only subtypes stated in conventions are allowed
		 */
		bool typeCanBeEdited(const string& type) ;

		/**
		 * Some menu options are configured from items defined in conventions.
		 * Presentation order is by default items alphabetical order, but corpus manager has the possibility
		 * to impose that presentation order is that of conventions definitions.
		 * @return true if qualifier items may be sorted by alphabetical order, else false
		 */
		bool sortQualifiersForLayout() { return m_sortQualifiersForLayout; }

		/**
		 * Returns the minimal segment size
		 * @param type 			annotation type / graph type
		 * @param defval		default value
		 * @return 				minimal size for given segment time
		 */
		float minSegmentSize(const string& type, float defval=0.001);

		/**
		 * check if graph type applies to given signal mime class
		 * @param graphtype graph type
		 * @param signaltype signal mime class (audio/video)
		 * @return true if graph type applies to given signal mime class, else false
		 */
		bool appliesToSignalClass(const string& graphtype, const string& signaltype) ;

		/**
		 * @return true if conventions set for fast annotation mode
		 */
		bool fastAnnotationMode();

		/**
		 * automatic space handling
		 * @param c checked character
		 * @return true if check character must be surrounded by spaces, else false
		 */
		bool needSpaceBorders(char c) ;
		/**
		 * automatic space handling
		 * @return set of chars that must be surrounded by chars
		 */
		std::set<gunichar> getSpaceBorderedChars() { return m_borderedChars; }
		/**
		 * automatic space handling
		 * @return true if some chars require surrounding spaces,
		 */
		bool spaceBorderingForced() ;
		/**
		 * automatic space handling
		 * @return true if automatic space handling is on, else false.
		 */
		bool automaticSpaceHandling() ;


		/**
		 * add mainstream type to current graph type
		 * @param type new type
		 * @param graphtype graph type
		 */
		void appendMainstreamType(string type, string graphtype="transcription_graph");

		/**
		 * Checks whether the given value for the corresponding parameter exists in
		 * the applied conventions.
		 * @param type			Type of element
		 * @param parameter		Parameter
		 * @param value			Value
		 * @return				True if the value is listed in conventions files, false otherwise
		 */
		bool checkValue(const string& type, const string& parameter, const string& value) ;

		/**
		 * Searches into conventions the graph corresponding to the given types.
		 * @param types		Annotation types
		 * @return			The type of the graph that could correspond to the given annotation
		 * @remarks			Use this method only if you are unable to correctly identify the graph type
		 */
		const std::string& getGraphtypeFromTypes(const set<string>& types) ;

		/**
		 * Searches into conventions the graph corresponding to the given type.
		 * @param 	type		Annotation type
		 * @return				The type of the graph that could correspond to the given annotation
		 * @remarks				Use this method only if you are unable to correctly identify the graph type
		 */
		const string& getGraphtypeFromType(const string& type) ;

		/**
		 * Returns the AGAPI Feature name corresponding to the subtype of the given type.
		 * @param type			The type we want to find the subtype
		 * @param graphtype		Graphtype
		 * @return				Feature name if found, empty if the given type has no subtype.
		 */
		string getSubtypeFeatureName(const string& type, const string& graphtype) ;

		/**
		 * get features with default value for given annotation type
		 * @param type annotation type
		 * @param graphtype	graphtype
		 * @return	All features in map
		 */
		const std::map< std::string, std::string> & getTypeFeatures(const std::string& type, const string& graphtype="");

		/**
		 * Returns whether a feature is valid for a given type of a given graphtype
		 * @param feature		Feature name
		 * @param type			Annotation type
		 * @param graphtype		Graphe type
		 * @return				True if the given annotation type uses the feature, False otherwise.
		 * @note				Will return false if the type is not a valide type for the given graphe
		 */
		bool isValidFeature(const std::string& feature, const std::string& type, const std::string& graphtype="transcription_graph") ;

		/**
		 *
		 * @param subtype
		 * @param graphtype
		 * @return
		 */
		string normalizeSubmain(const string& subtype, const string& graphtype) ;

		/**
		 * Indicates whether the given submain (subtype of mainstream base type) holds text data
		 * @param type			Type
		 * @param subtype		Subtype
		 * @param graphtype		Graphtype
		 * @return
		 */
		bool submainHasText(const string& type, const string& subtype, const string& graphtype) ;

		/**
		 * Gets type corresponding to a given feature
		 * @param feature		Feature name
		 * @param graphtype		Graphtype
		 * @return				Type
		 */
		const string& getTypeFromFeature(const std::string& feature, const string& graphtype="") ;

		/**
		 * getSpeakerType -> returns annotation type to which "speaker" feature is attached
		 */
		const string& getSpeakerType(const string& graphtype) ;

	public:
		/**
		 * set conventions definition files default directory
		 * @param path default directory path
		 */
		static void setConfigDir(const string& path) { _configDir = path; }

		friend class DataModel;

	private:
		/*** General configuration ***/
		static string _configDir;	/**< conventions definition files default directory */

		std::string m_conventions;    /**< current annotation conventions name **/

		string m_configDir;	/**< current conventions definition files directory */
		string m_currentFile;	/**< current loaded convention file */
		string m_currentLang;	/**< current language configuration  */
		string m_version ;
		string m_corpusName ;
		string m_corpusVersion ;

		/*** Graphes ***/
		map<string, GraphDescriptor> m_graphDesc;
		vector<string> m_graphTypes;	/** graphtypes  **/
		std::map <std::string, float> m_minSegSize;	/**< min size for any graph type */
		map<string, Parameters*> m_graphToAdd ; /** save graph type and graph information for adding them after loading **/
		bool m_usesVideoSignal;  // true if one of the graphs applies to video signal

		/*** Data ***/
		std::map <std::string, std::string> m_config;	/**< data model config options */

		/*** Layout ***/
		std::map <std::string, std::string> m_labels;	/**< segtypes labels */
		Parameters m_layout;
		bool m_sortQualifiersForLayout;		/**< sort seg qualifiers (enclosing first) */

		/*** Extra files ***/
		std::list<WordList> m_wordLists ; /**< predefined words lists (input support) */
		std::map<Glib::ustring,Topics*> m_Topics ; /**< predefined topics list (input support) */

		/*** Space handling ***/
		bool m_spaceHandling ; /**check for space handling **/
		set<gunichar> m_borderedChars ; /** for space bordered chars **/

		std::string _undef;
		std::map< std::string, std::string> _emptyFeat;

		/* for returning empty value as ref */
		std::vector<string> _no_type_v ;
		std::string _no_type_s ;

	private:

		const std::string& getConfiguration(const std::string& key, std::map<std::string, std::string> configuration) const;

		int addExtraGraphs() ;
		void updateGraphDescriptor(const string& graphtype, Parameters* param, bool reset_mode);
		// set mainstream type vector
		void updMainstreamTypes(const string& graphtype, Parameters* param, bool reset_mode);
	    // set signals types
	    void updSignalTypes(const string& graphtype, Parameters* param=NULL) ;
		// set qualifiers type vector
		void updQualifierTypes(const string& types, Parameters* param=NULL);
		// set base mainstream subtype map
		void updMainstreamBaseSubtype(const string& graphtype, Parameters* param) ;

		string loadLocalizedLayout(const string& path);
		void loadWordLists(const string& path, string lang="");
		void loadTopics(const string& path, string lang="");

};

} // namespace tag

#endif /* _HAVE_CONVENTIONS_H */
