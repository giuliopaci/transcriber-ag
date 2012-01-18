/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef __HAVE_PARAMETERS__
#define __HAVE_PARAMETERS__

#include <map>
#include <string>

#include "CommonXMLReader.h"

using namespace std;

namespace tag {

/**
* @defgroup 	Common
*/

/**
* @class 		Parameters
* @ingroup		Common
*
* Wrapper for XML configuration files IO.\n
* It provides a specific XML data representation for all TranscriberAG configuration files.\n\n
*
* This representation uses a three level skeleton, 1 Component => N Section => M parameters\n
* Therefore 1 Component is loaded into a map where the key is the Component name and the value is a map (let's call it SecParam).\n
* A SecParam is loaded into a map where the key is the Section name and the Parameter name, separated with "," ,
* and the value is the Parameter value.
*
*/
class Parameters
{
	public:

		/**
		* @struct 		KeyLookup
		* @ingroup		Common
		*
		* Functor for defining a prefix to be looked for inside a string
		*/
		struct KeyLookup : public std::unary_function< pair<string, string>, bool >
		{
			private:
				string m_prefix;
			public:
				/**
				 * Constructor
				 * @param prefix	Prefix to look for
				 */
				KeyLookup(const string& prefix) : m_prefix(prefix) {};

				/**
				 * Functor operator
				 * @param it	Iterator
				 * @return		True if prefix could be found, False otherwise
				 */
				bool operator () (const pair<string, string>& it) {
					return (it.first.compare(0, m_prefix.size(), m_prefix) == 0 && it.first.find(",#label") == string::npos);
				}
		};

		/**
		 * Constructor
		 * @return
		 */
		Parameters();
		virtual ~Parameters();

		/**
		 * Adds all parameters specified in <em>users</em> map into <em>system</em> map. Enables to choose
		 * if the resulting map should be saved into <em>system</em> file.
		 * @param users		User Parameters object
		 * @param system	System Parameters object
		 * @param save		True for saving the resulting mapping into the file
		 * 					from where <em>system</em> were loaded, false otherwise
		 */
		static void mergeUserParameters(Parameters* users, Parameters* system, bool save) ;

		/**
		 * Update a Parameters instance with values of old Parameters instance.\n
		 * More precisely, for each parameter of <em>old</em>, checks if it exists in <em>newp</em>
		 * and copy the old value if it exists.
		 *
		 * @param old		Old Parameters instance
		 * @param newp		New Parameters instance to update
		 * @param save		True for saving the resulting mapping into the file
		 * 					from where <em>newp</em> were loaded, false otherwise
		 */
		static void updateUserParameters(Parameters* old, Parameters* newp, bool save) ;

		/**
		 * @return	All Components and the corresponding value.
		 */
		std::map<string, std::map<string, string> >& getMap() { return a_components ; }

		/**
		 * Adds a component in the parameters map
		 * @param a_comp	Component name
		 * @param map		SecParam map
		 */
		void setMap(string a_comp, std::map<string, string> map) { a_components[a_comp]=map ; }

		/**
		 * Accessor to a Component map.
		 * @param p_idComponent		Component name
		 * @return					Pointer on the corresponding SecParam map
		 * @note					Available for modification
		 * @see						getParametersMap() for read-only access
		 */
		std::map<string, string>* getAndModifyParametersMap(const string& p_idComponent);

		/**
		 * Print all the parameters.
		 * @param only_param		True for skipping all label information, False otherwise
		 */
		void print(bool only_param) ;

		/**
		 * Load parameters from given file
		 * @param p_path	Path of the file that describes all parameters
		 */
		void load(const string& p_path) throw (const char*);

		/**
		 * Refresh parameters value by loading from the file again
		 */
		void reload() throw (const char*);

		/**
		 * Checks a component existence.
		 * @param p_idComponent		Component name
		 * @return					True if the component exists, False otherwise
		 */
		bool existsParametersMap(const string& p_idComponent);

		/**
		 * Accessor to a Component map.
		 * @param p_idComponent		Component name
		 * @return					Reference on the corresponding SecParam map
		 * @note					For access in modification see getAndModifyParametersMap()
		 */
		const std::map<string, string>& getParametersMap(const string& p_idComponent);

		/**
		 * Checks the existence of the given parameter
		 * @param p_idComponent			Component to which the parameter must belong
		 * @param p_idSectionParam		SecParam to be checked
		 * @return						True if the parameter could be found, False otherwise
		 */
		bool existsParameter(const string& p_idComponent, const string& p_idSectionParam);

		/**
		 * Accessor to the label of a given Parameter
		 * @param p_idComponent			Component to which the parameter belongs
		 * @param p_idSectionParam		SecParam key
		 * @see							getParameterLabel(const string&,const string&,const string&) for more convenient method
		 * @return						The corresponding label if it exists, empty value otherwise
		 */
		const string& getParameterLabel(const string& p_idComponent, const string& p_idSectionParam);

		/**
		 * Accessor to the label of a given Parameter
		 * @param p_idComponent			Component key
		 * @param p_idSection			Section key
		 * @param p_idParam				Parameter key
		 * @return						The corresponding label if it exists, empty value otherwise
		 */
		const string& getParameterLabel(const string& p_idComponent, const string& p_idSection,  const string& p_idParam)
		{
			return getParameterLabel(p_idComponent, p_idSection+","+p_idParam) ;
		}

		/**
		 * Accessor to the value of a given Parameter
		 * @param p_idComponent			Component key
		 * @param p_idSectionParam		SecParam key
		 * @see							getParameterValue(const string&,const string&,const string&) for more convenient method
		 * @return						The corresponding value if it exists, empty value otherwise
		 */
		const string& getParameterValue(const string& p_idComponent, const string& p_idSectionParam);

		/**
		 * Accessor to the value of a given Parameter
		 * @param p_idComponent			Component key
		 * @param p_idSection			Section key
		 * @param p_idParam				Parameter key
		 * @return						The corresponding value if it exists, empty value otherwise
		 */
		const string& getParameterValue(const string& p_idComponent, const string& p_idSection,  const string& p_idParam)
		{
			return getParameterValue(p_idComponent, p_idSection+","+p_idParam) ;
		}

		/**
		 * Sets the label of the given parameter. Provides the possibility to create the Parameter if
		 * it doesn't exist
		 * @param p_idComponent			Component key
		 * @param p_idSectionParam		SecParam key
		 * @param p_label				Label to be set
		 * @param p_create				If set to True and if Parameter doesn't exist, will create the Parameter
		 * @see							setParameterLabel(const string&,const string&,const string&,const string&,bool) for more convenient method
		 * @return						True for success, False for failure
		 */
		bool setParameterLabel(const string& p_idComponent, const string& p_idSectionParam, const string& p_label, bool p_create=false);

		/**
		 * Sets the label of the given parameter. Provides the possibility to create the Parameter if
		 * it doesn't exist
		 * @param p_idComponent			Component key
		 * @param p_idSection			Section key
		 * @param p_idParam				Parameter key
		 * @param p_label				Label to be set
		 * @param p_create				If set to True and if Parameter doesn't exist, will create the Parameter
		 * @return						True for success, False for failure
		 */
		bool setParameterLabel(const string& p_idComponent, const string& p_idSection,  const string& p_idParam, const string& p_label, bool p_create=false)
		{
			return setParameterLabel(p_idComponent, p_idSection+","+p_idParam, p_label, p_create);
		}

		/**
		 * Sets the value of the given parameter. Provides the possibility to create the Parameter if
		 * it doesn't exist
		 * @param p_idComponent			Component key
		 * @param p_idSectionParam		SecParam key
		 * @param p_value				Value to be set
		 * @param p_create				If set to True and if Parameter doesn't exist, will create the Parameter
		 * @see							setParameterValue(const string&,const string&,const string&,const string&,bool) for more convenient method
		 * @return						True for success, False for failure
		 */
		bool setParameterValue(const string& p_idComponent, const string& p_idSectionParam, const string& p_value, bool p_create=false);

		/**
		 * Sets the value of the given parameter. Provides the possibility to create the Parameter if
		 * it doesn't exist
		 * @param p_idComponent			Component key
		 * @param p_idSection			Section key
		 * @param p_idParam				Parameter key
		 * @param p_value				Value to be set
		 * @param p_create				If set to True and if Parameter doesn't exist, will create the Parameter
		 * @return						True for success, False for failure
		 */
		bool setParameterValue(const string& p_idComponent, const string& p_idSection,  const string& p_idParam, const string& p_value, bool p_create=false)
		{
			return setParameterValue(p_idComponent, p_idSection+","+p_idParam, p_value, p_create);
		}

		/**
		 * Adds a new Section into a Component
		 * @param p_idComponent			Key of the component the section will belong too
		 * @param p_idSection			Section to be added
		 * @param p_label				Label of the new section
		 */
		void addSection(const string& p_idComponent, const string& p_idSection, const string& p_label);

		/**
		 * Save parameters into file.
		 * @return		True for success, False is file couldn't be found
		 */
		bool save();

		private:
			string a_path;
			std::map<string, std::map<string, string> > a_components;
};

} //namespace

#ifndef __HAVE_PROPERTY__
#define __HAVE_PROPERTY__
/**
 * @struct	Property
 * Structure used for representing a dynamic property
 */
struct Property
{
	string label;			/**< Property label */
	int type;				/**< Property type */
	string choiceList;		/**< Property choice list */
};

const int PROPERTY_TEXT = 0;			/**< Property text type */
const int PROPERTY_CHOICELIST = 1;		/**< Property choice list type */
#endif

/**
* @defgroup 	Common
*/

/**
* @class 		SAXParametersHandler
* @ingroup		Common
*
* Utility class for SAX parsing
* Used by Parameters class
*/
class SAXParametersHandler : public CommonXMLHandler
{
	public :
		/**
		 * Constructor
		 * @param p_parameters	Pointer on a parameter map
		 */
		SAXParametersHandler(std::map<string, std::map<string, string> >* p_parameters) {
			a_parameters = p_parameters;
		}
		virtual ~SAXParametersHandler() { };

		/**
		 * Preser starts element behaviour
		 * @param uri			---
		 * @param localname		---
		 * @param qname			---
		 * @param attrs			---
		 */
		void startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const Attributes& attrs);

	private :
		std::map<string, std::map<string, string> >* a_parameters;
		std::map<string, string>* a_tmpComponent;
		string a_tmpSectionID;

};


#endif // __HAVE_PARAMETERS__
