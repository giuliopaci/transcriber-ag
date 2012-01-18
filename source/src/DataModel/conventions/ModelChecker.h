/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/*
 *  $Id$
 */

#ifndef MODELCHECKER_H_
#define MODELCHECKER_H_

#include <glibmm.h>
#include <set>

#define  MCHK_NO_ERROR						 		 1
#define  MCHK_ERROR_GRAPH_NOTINCONV					-1
#define  MCHK_ERROR_GRAPH_NOBASESEG					-2
#define  MCHK_ERROR_GRAPH_NOCONTINUOUS				-3
#define  MCHK_ERROR_GRAPH_INTERNAL					-4
#define  MCHK_ERROR_GRAPH_INVALIDTYPES				-5
#define  MCHK_ERROR_GRAPH_INVALIDSUBTYPES			-6
#define  MCHK_ERROR_IMPORT_WARNING					-7
#define  MCHK_ERROR_CONV_FILE						-8
#define  MCHK_ERROR_CONV_VERSION					-9
#define  MCHK_ERROR_GRAPH_INVALIDUNIT				-10
#define  MCHK_ERROR_GRAPH_INVALIDUNIT_VALUE			-11
#define  MCHK_ERROR_GRAPH_INVALIDUNIT_DESC			-12

namespace tag {

class DataModel ;

/**
 *  @class 		ModelChecker
 *  @ingroup 	DataModel
 *  Check tools and mechanisms for DataModel <br>
 *	Used for checking the correct use of conventions and correct graph form
 */
class ModelChecker
{
	public:
		/**
		 *  @class 		CheckGraphResult
		 *  @ingroup 	DataModel
		 *  Class used as a structure to keep graph checking results.\n
		 *  Tool of class ModelChecker.
		 */
		class CheckGraphResult
		{
			public:
				/**
				 * Constructor
				 * @param p_graphid		Graph id
				 * @param p_graphtype	Graph normalized type
				 * @param p_priority	priority:\n
				 * 						0: all is correct\n
				 * 						1: graph can be loaded but corrections can be necessary\n
				 * 						2: graph can't be loaded
				 */
				CheckGraphResult(const std::string& p_graphid, const std::string& p_graphtype, int p_priority) ;

				/**
				 * Indicates whether the graph can be loaded.\n
				 * Used to keep information about graphes with errors that can be corrected
				 * @return		0: all is correct\n
				 * 				1: graph can be loaded but corrections can be necessary\n
				 * 				2: graph can't be loaded
				 */
				int getPriority() { return status ;}

				/**
				 * Indicates whether errors has been encountered
				 * @return		True or False
				 */
				bool hasErrors() { return errors.size() > 0 ; }

				/**
				 * Insert error code for given graph.
				 * @param error_code
				 *						MCHK_ERROR_GRAPH_INTERNAL	  :   data model or graphtype is invalid\n
				 *						MCHK_ERROR_GRAPH_NOTINCONV	  :   graph is not in convention\n
				 *	 					MCHK_ERROR_GRAPH_NOBASESEG	  :   graph has no valid base type\n
			 	 *	 					MCHK_ERROR_GRAPH_NOCONTINUOUS :   graph is not continuous\n
				 *						MCHK_ERROR_GRAPH_INVALIDTYPES :   graph has unknown types
				 */
				void addError(int error_code) { errors[error_code] = 0 ; }

				/**
				 * Insert a set containing invalid types
				 * @param invalid		String vector
				 */
				void addInvalidTypes(const std::set<std::string> invalid) { invalid_types = invalid  ; }

				/**
				 * Insert a vector containing invalid types
				 * @param invalid		String vector
				 */
				void addInvalidSubtypes(const map<std::string,std::set<std::string> >& invalid) { invalid_subtypes = invalid  ; }

				/**
				 * Insert a set containing invalid types
				 * @param invalid		String vector
				 */
				void addInvalidSubmain(const std::set<std::string> invalid) { invalid_submain = invalid  ; }

				/**
				 * Insert a vector containing invalid types
				 * @param invalid		String vector
				 */
				void addInvalidSubmainValues(const map<std::string,std::set<std::string> >& invalid) { invalid_submain_value = invalid  ; }

				/**
				 * Insert a vector containing invalid types
				 * @param invalid		String vector
				 */
				void addInvalidSubmainDesc(const map<std::string,std::set<std::string> >& invalid) { invalid_submain_desc = invalid  ; }

				/**
				 * Changes error status
				 * @param error_code	Code error
				 * @param candidate		True for setting the error as candidate to fix, False otherwise
				 * @return				False if the error doesn't exist
				 */
				bool setErrorCodeCleanCandidate(int error_code, bool candidate) ;

				/**
				 * Tells the GUI that the error corresponding to the given code
				 * is in given state
				 * @param error_code		Error code
				 * @param state				Result state
				 */
				void emitDisplaySignal(int error_code, int state) { signalDisplayState().emit(error_code, state) ;  }

				/** Accessor **/
				const std::string& get_graphId() { return graphid ; }
				/** Accessor **/
				const std::string& get_graphType() { return graphtype ; }
				/** Accessor **/
				const std::map<int,int>& get_errorCodes() { return errors ; }
				/** Accessor **/
				const std::set<std::string>& get_invalidTypes() { return invalid_types ; }
				/** Accessor **/
				const std::map< std::string, std::set<std::string> >& get_invalidSubtypes() { return invalid_subtypes ; }
				/** Accessor **/
				const std::set<std::string>& get_invalidSubmain() { return invalid_submain ; }
				/** Accessor **/
				const std::map< std::string, std::set<std::string> >& get_invalidSubmainValues() { return invalid_submain_value ; }
				/** Accessor **/
				const std::map< std::string, std::set<std::string> >& get_invalidSubmainDesc() { return invalid_submain_desc ; }

				/** Signal teller gui **/
				sigc::signal<void, int, int>& signalDisplayState() { return m_signalDisplayState;}

			private :

				/**
				 *  ( error type - error status)
				 *  0: not fixed
				 *  1: candidate to clean
				 *  2: cleaned
				 * -1: clean failure
				 */
				map<int,int> errors ;

				/**
				 * @var status
				 * 1: warning (load possible, but some errors should be corrected)
				 * 2: error (load impossible)
				 */
				int status ;

				/**< graph id */
				std::string graphid ;

				/**< normalized graph type */
				std::string graphtype ;

				/**< types clean candidates **/
				set<std::string> invalid_types ;
				/**< subtypes clean candidates (type - all invalid subtypes)**/
				std::map< std::string, std::set<std::string> > invalid_subtypes ;

				/**< types clean candidates **/
				set<std::string> invalid_submain ;
				/** (submain - value) **/
				std::map< std::string, std::set<std::string> > invalid_submain_value ;
				/** (submain - value;desc) **/
				std::map< std::string, std::set<std::string> > invalid_submain_desc ;

				sigc::signal<void, int, int> m_signalDisplayState ;
		} ;

		/**
		 * Constructor
		 * @param model			DataModel the checker is linked to
		 */
		ModelChecker(DataModel* model);

		/**
		 * Destructor
		 * @return
		 */
		virtual ~ModelChecker();

		/**
		 * Check the graph corresponding to the given Id.
		 * Each time an error (fatal or not) is encountered, the corresponding code
		 * is kept in the CheckGraphResult corresponding to the graph.
		 * @param 	agId		Graph id
		 * @param	graphtype	Graph type
		 * @param 	only_check	In classic behaviour, the checker will keep information
		 * 						like the number of valid graphs loaded. If <em>only_check</em> is
		 * 						set to true, the checker won't keep the information, and will only
		 * 						test if the graph is valid. Therefore, if <em>only_check</em> is
		 * 						set to true, the graph won't be kept as loaded insede checker.\n
		 * 						Always set <em>only_check</em> at true when loading graphs.
		 * @return				-1 : graph can't be loaded\n
		 * 						 0 : graph can be loaded but some correction should be applied\n
		 * 						 1 : graph is correct
		 */
		int checkGraph(const std::string& agId, const std::string& graphtype, bool only_check=false) ;

		/**
		 *
		 * Checks all types for the given graphes
		 * @param 		agId					Graph id
		 * @param 		graphtype				Graph type
		 * @param[in] 	invalid_types			Set for receiving the invalid types
		 * @param[in] 	invalid_subtypes		Map for receiving the invalid types and theirs
		 * @param[in]	invalid_submain			Set for receiving invalid subtype of mainstream base type
		 * @param[in]	invalid_submain_value	Map for receiving invalid subtype of submain (given by submain)
		 * @param[in]	invalid_submain_desc	Map for receiving invalid desc of submain (given by subtype submain)
		 * @return								 1: all is OK\n
		 * 										-1: invalid types detected\n
		 *			 							-2: invalid subtypes detected\n
		 * 										-3: invalid types AND invalid subtypes detected
		 */
		int checkAnnotationTypes(const string& agId, const std::string& graphtype,
													std::set<std::string>& invalid_types,
													map< std::string, std::set<std::string> >& invalid_subtypes,
													set<std::string>& invalid_submain,
													std::map< std::string, std::set<std::string> >& invalid_submain_value,
													std::map< std::string, std::set<std::string> >& invalid_submain_desc ) ;

		/**
		 * Compares the given convention name and version with the conventions available
		 * for the application
		 * @param convention_name 	Name of the convention
		 * @param version			Version of the convention
		 * @return					MCHK_ERROR_CONV_FILE : 		error with convention file \n
		 * 							MCHK_ERROR_CONV_VERSION : 	error with convention version
		 */
		int checkConventions(const std::string& convention_name, const std::string& version) ;

		/**
		 * Checks whether the graph is continuous on base type
		 * @param agId			Graph id
		 * @param graphtype		Normalized graph type
		 * @param baseType		Base type of graph
		 * @return
		 */
		bool baseTypeIsContinuous(const std::string& agId, const std::string& graphtype, const std::string& baseType) ;

		/**
		 * Clear all checker data
		 */
		void clear() ;

		/**
		 * Sets the warnings message passed by plugin when importing a format
		 * @param warnings		Vector of string with all warning message
		 */
		void setImportWarnings(std::vector<std::string> warnings) { m_import_warn = warnings ; }

		/**
		 * Adds a graphtype in the list of all graphs added after loading
		 * @param graphtype		Graphtype
		 */
		void addAddedGraph(std::string graphtype) { added_graphs.insert(graphtype) ; }


		//----------------------------------------------------------------------
		//								ACCESSORS
		//----------------------------------------------------------------------

		/**
		 * Accessor to all import warning
		 * @return 				MCHK_ERROR_IMPORT_WARNING if some warnings were detected\n
		 * 						MCHK_NO_ERROR otherwise
		 */
		const std::vector<std::string>& getImportWarnings() { return m_import_warn ; }

		/**
		 * Accessor to the log of the given graph
		 * @param graphtype		Type of graph
		 * @return				Reference on log object (NULL if none)
		 */
		CheckGraphResult* getCheckResult(const string& graphtype) ;

		/**
		 * Accessor to all graphtypes that has been added (specified in conventions but not found in file)
		 * @return		Vector of string
		 */
		const std::set<std::string>& getAddedGraphs() { return added_graphs ;}

		/**
		 * Accessor to the incorrect graphs of the given priority
		 * @param priority		1: warning - 2 : fatal errors
		 * @return				Vector of string (graph types)
		 */
		const std::set<std::string>& getGraphesByPriority(int priority) { return (priority==1 ? graphs_priority_1 : graphs_priority_2 ) ;}

		//----------------------------------------------------------------------
		//								NUMBER ACCESSOR
		//----------------------------------------------------------------------

		/**
		 * Indicates whether the convention needed by files are correct
		 * @param[out] name			If convention file couldn't be found,
		 * 							receives the name needed by file.
		 * @param[out] version		If convention version doesn't match current version,
		 * 							receives the version needed by the file.
		 * @return
		 *  						0 							no convention error\n
		 *  						MCHK_ERROR_CONV_FILE		convention file can't be found\n
		 *  						MCHK_ERROR_CONV_VERSION		convention version doesn't mactch applied conventions\n
		 */
		int getConventionLog(string& name, string& version) ;

		/**
		 * Gets the number of error of the given priority
		 * @param priority		1 for warnings, 2 for fatal errors
		 * @return				Number of errors of given priority
		 */
		int getNbPriority(int priority) { return (priority==1 ? graphs_priority_1.size() : graphs_priority_2.size() ) ; }

		/**
		 * Gets number of added graphs.
		 * @return			The number of added graphs
		 */
		int getNbAddedGraphs() { return added_graphs.size() ; }

		/**
		 * Indicates whether the loading mechanism encountered import warning
		 * @return			True of False
		 */
		bool hasImportWarnings() { return (m_import_warn.size()>0) ; }

		/**
		 * Indicates whether the loading mechanism encountered warnings
		 * @return			True of False
		 */
		bool hasWarnings() ;

		/**
		 * Indicates whether the loading mechanism encountered import errors
		 * @return			True of False
		 */
		bool hasErrors() ;


		/**
		 * Indicates whether nothing was proceeded: graph(s) was/were OK.
		 * (i.e perfect file)
		 * @return			True or False
		 */
		bool isFullyCorrect() ;

		/**
		 * Indicates whether the loading only encountered information (added graphs for instance).
		 * @return			True or False
		 */
		bool isCorrect() ;

		//----------------------------------------------------------------------
		//								CLEAN BUSINESS
		//----------------------------------------------------------------------

		/**
		 *  Launches the clean process for all error codes that has previously marked
		 *  as candidate to clean.
		 *  @return		Number of modification done
		 */
		int applyCleanActions() ;

		/**
		 * Signal emitted when the main GTK loop should be flushed.\n
		 */
		sigc::signal<void>& signalFlushGUI() { return m_signalFlushGUI;}

	private:

		DataModel* model ;

		/** graph check results
		 *  (graphtype - results structure class)
		 */
		std::map<std::string, CheckGraphResult*> results ;

		//******* GRAPHS

		/** Correct or quite correct (convention and baseType OK) graphs: can be loaded  **/
		std::set<std::string> graphs_priority_1 ;
		/** Incorrect (no baseType, lack of indispensable conventions element): cannot be loaded  **/
		std::set<std::string> graphs_priority_2 ;
		/** All graphs that were added for fitting the applied convention **/
		std::set<std::string> added_graphs ;

		bool checkAnnotationSubtypes(const string& agId, const std::string& type,
										const std::string& graphtype,
										map<std::string,std::set<std::string> >& invalid_subtypes) ;

		bool checkMainstreamBasetype(const string& agId, const std::string& graphtype,
										set<std::string>& invalid_submain,
										std::map< std::string, std::set<std::string> >& invalid_submain_value,
										std::map< std::string, std::set<std::string> >& invalid_submain_desc ) ;

		//******* CONVENTIONS

		/** Keep convention file name if it couldn't be found */
		string bad_conv_file ;

		/** Keep convention version if it doesn't match the current conventions */
		string bad_conv_version ;

		/**
		 *  0: 							no convention error
		 *  MCHK_ERROR_CONV_FILE		convention file can't be found
		 *  MCHK_ERROR_CONV_VERSION		convention version doesn't match applied conventions
		 */
		int convention_error ;


		//******* IMPORT

		/** warning msg data for import **/
		std::vector<std::string> m_import_warn ;


		//******* CLEAN BUSINESS

		bool fixError(CheckGraphResult* checkResult, int errorCode) ;
		bool cleanType(ModelChecker::CheckGraphResult* checkResult) ;
		bool cleanType(const string& graphid, const string& graphtype, const string& type) ;
		bool cleanSubtype(ModelChecker::CheckGraphResult* checkResult) ;
		bool cleanSubtype(const string& graphid, const string& graphtype, const string& subtype, const string& type) ;


		sigc::signal<void> m_signalFlushGUI ;
};

}

#endif /* MODELCHECKER_H_ */
