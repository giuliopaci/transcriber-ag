/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 *  $Id$
 *
 *  @file 	ModelChecker
 *  @brief  Validation mechanism and tools
 */

#include <ag/AGAPI.h>

#include "ModelChecker.h"

#include "DataModel/DataModel.h"
#include "DataModel/conventions/Conventions.h"

#include "Common/FileInfo.h"


namespace tag {

//******************************************************************************
//							      Check Result
//******************************************************************************

ModelChecker::CheckGraphResult::CheckGraphResult(const std::string& p_graphid, const std::string& p_graphtype, int p_priority)
{
	graphid = p_graphid ;
	graphtype = p_graphtype ;
	status = p_priority ;
}


bool ModelChecker::CheckGraphResult::setErrorCodeCleanCandidate(int error_code, bool candidate)
{
	/*** actualize status ***/
	std::map<int,int>::iterator it = errors.find(error_code) ;
	if (it==errors.end())
		return false ;
	else if (candidate)
		it->second = 1 ;
	else
		it->second = 0 ;
}

//******************************************************************************
//							      Model Checker
//******************************************************************************


ModelChecker::ModelChecker(DataModel* datamodel)
{
	model = datamodel ;
	convention_error = 1 ;
}

ModelChecker::~ModelChecker()
{
	clear() ;
}

void ModelChecker::clear()
{
	std::map<std::string, CheckGraphResult*>::iterator it ;
	for (it=results.begin(); it!=results.end(); it++) {
		if (it->second)
			delete (it->second) ;
	}
	results.clear() ;

	graphs_priority_1.clear() ;
	graphs_priority_2.clear() ;

	bad_conv_file.clear() ;
	bad_conv_version.clear() ;
	convention_error = 0 ;
}

ModelChecker::CheckGraphResult* ModelChecker::getCheckResult(const string& graphtype)
{
	std::map<std::string, CheckGraphResult*>::iterator it = results.find(graphtype) ;
	if ( it != results.end() )
		return it->second ;
	else
		return NULL ;
}


//------------------------------------------------------------------------------
//							      Graphes
//------------------------------------------------------------------------------

/*
 *  -1 : graph can't be loaded
 * 	 0 : graph can be loaded but some correction should be applied
 * 	 1 : graph is correct
 */
int ModelChecker::checkGraph(const std::string& agId, const std::string& graphtype, bool only_check)
{
	if (!model || graphtype.empty())
		return -1 ;

	//> 1: check presence in conventions
	bool graph_in_conv = model->conventions().hasGraphType(graphtype) ;
	if (!graph_in_conv)
	{
		if (!only_check)
		{
			results[graphtype] = new CheckGraphResult(agId, graphtype, 2) ;
			results[graphtype]->addError(MCHK_ERROR_GRAPH_NOTINCONV) ;
			graphs_priority_2.insert(graphtype) ;
		}
		return -1 ;
	}

	//> 2 : check presence of baseSegment in loaded graph
	std::string mainstreamBaseType = model->mainstreamBaseType(graphtype) ;
	bool has_base_type = model->hasElementsWithType(mainstreamBaseType, agId) ;
	if (!has_base_type)
	{
		if (!only_check)
		{
			results[graphtype] = new CheckGraphResult(agId, graphtype, 2) ;
			results[graphtype]->addError(MCHK_ERROR_GRAPH_NOBASESEG) ;
			graphs_priority_2.insert(graphtype) ;
		}
		// error is important, can't keep on loading: exit.
		return -1 ;
	}

	//> 3 : if graph is continuous, check if base segment type is continuous
	bool base_continuous = true ;
	//TODO empty method, implement it
	if (model->conventions().isContinuousGraph(graphtype))
		base_continuous = baseTypeIsContinuous(agId, graphtype, mainstreamBaseType) ;
	if ( !base_continuous )
	{
		if (!only_check)
		{
			results[graphtype] = new CheckGraphResult(agId, graphtype, 2) ;
			results[graphtype]->addError(MCHK_ERROR_GRAPH_NOCONTINUOUS) ;
			graphs_priority_2.insert(graphtype) ;
		}
		// error is important, can't keep on loading: exit.
		return -1 ;
	}

	//> 4 : check annotation types
	// -- prepare our container
	std::set<std::string> invalid_types ;
	map<std::string,std::set<std::string> > invalid_subtypes ;
	set<std::string> invalid_submain ;
	std::map< std::string, std::set<std::string> > invalid_submain_value ;
	std::map< std::string, std::set<std::string> > invalid_submain_desc ;

	int invalid_return = checkAnnotationTypes(agId, graphtype, invalid_types, invalid_subtypes,
												invalid_submain, invalid_submain_value, invalid_submain_desc) ;
	if ( invalid_return!=1 && !only_check)
	{
		results[graphtype] = new CheckGraphResult(agId, graphtype, 1) ;
		// invalid types detected
		if ( ! invalid_types.empty() )
		{
			results[graphtype]->addError(MCHK_ERROR_GRAPH_INVALIDTYPES ) ;
			results[graphtype]->addInvalidTypes(invalid_types);
		}
		// invalid subtypes detected
		if ( ! invalid_subtypes.empty() )
		{
			results[graphtype]->addError(MCHK_ERROR_GRAPH_INVALIDSUBTYPES) ;
			results[graphtype]->addInvalidSubtypes(invalid_subtypes);
		}
		// invalid subtypes detected
		if ( ! invalid_submain.empty() )
		{
			results[graphtype]->addError(MCHK_ERROR_GRAPH_INVALIDUNIT) ;
			results[graphtype]->addInvalidSubmain(invalid_submain);
		}
		// invalid subtypes detected
		if ( ! invalid_submain_value.empty() )
		{
			results[graphtype]->addError(MCHK_ERROR_GRAPH_INVALIDUNIT_VALUE) ;
			results[graphtype]->addInvalidSubmainValues(invalid_submain_value);
		}
		// invalid subtypes detected
		if ( ! invalid_submain_desc.empty() )
		{
			results[graphtype]->addError(MCHK_ERROR_GRAPH_INVALIDUNIT_DESC) ;
			results[graphtype]->addInvalidSubmainDesc(invalid_submain_desc);
		}
	}

	//> 5 : deep clean (//TODO clean bad annotations)
	bool deep_cleaned = true ;

	// determine whether all is OK or if there were some invalid states
	if ( !deep_cleaned || invalid_return!=1 )
	{
		if (!only_check)
			graphs_priority_1.insert(graphtype) ;
		return 0 ;
	}
	else
		return 1 ;
}

bool ModelChecker::baseTypeIsContinuous(const string& agId, const std::string& graphtype, const string& baseType)
{
	//TODO do it
	return true ;
}


/*
 *  1: all is OK
 * -1: invalid types detected
 * -2: invalid subtypes detected
 * -3: invalid types AND invalid subtypes detected
 */
int ModelChecker::checkAnnotationTypes(const string& agId, const std::string& graphtype,
											std::set<std::string>& invalid_types,
											map< std::string, std::set<std::string> >& invalid_subtypes,
											set<std::string>& invalid_submain,
											std::map< std::string, std::set<std::string> >& invalid_submain_value,
											std::map< std::string, std::set<std::string> >& invalid_submain_desc )
{
	int cpt_error = 0 ;

	//-- Get all annotations of graphe
	const std::set<AnnotationType>& l = GetAnnotationTypes(agId) ;
	std::set<AnnotationType>::const_iterator it  ;

	//-- For each annotation
	for (it=l.begin(); it!=l.end(); it++)
	{
		//-- Check if type is allowed
		bool valid = model->conventions().isValidType(*it, graphtype) ;
		if (!valid)
		{
			cpt_error++ ;
			invalid_types.insert(*it) ;
		}
		//-- Specific check for mainstream base type
		else if ( *it == model->conventions().mainstreamBaseType(graphtype) )
		{
			if ( checkMainstreamBasetype(agId, graphtype, invalid_submain, invalid_submain_value, invalid_submain_desc) )
				cpt_error++ ;
		}
		//-- Then check if subtype is Valid
		else if ( checkAnnotationSubtypes(agId, graphtype, *it, invalid_subtypes) )
			cpt_error++ ;
	}

	if (cpt_error==0)
		return 1 ;
	else
		return -1 ;
}


bool ModelChecker::checkMainstreamBasetype(const string& agId, const std::string& graphtype,
													set<std::string>& invalid_submain,
													std::map< std::string, std::set<std::string> >& invalid_submain_value,
													std::map< std::string, std::set<std::string> >& invalid_submain_desc )
{
	bool invalid = false ;
	string submain, value, desc ;

	// Get all annotationsId for given type
	const std::set<AnnotationId>& l = GetAnnotationSet(agId, model->conventions().mainstreamBaseType(graphtype)) ;
	std::set<AnnotationId>::const_iterator it ;

	// For all id, get the value of the  feature that represents the substype
	// (if current annotation can have subtype)
	for ( it=l.begin(); it!=l.end(); it++ )
	{
		submain = model->getElementProperty(*it, "subtype", "") ;
		value = model->getElementProperty(*it, "value", "") ;
		desc = model->getElementProperty(*it, "desc", "") ;

		int res = model->conventions().isValidMainstreamBaseType(graphtype, submain, value, desc) ;
		if ( res == -1 )
		{
			invalid = true ;
			invalid_submain.insert(submain) ;
		}
		else if ( res == -2 )
		{
			invalid = true ;
			invalid_submain_value[submain].insert(value) ;
		}
		else if ( res == -2 )
		{
			invalid = true ;
			invalid_submain_desc[submain].insert(value + ";" + desc) ;
		}
	}

	return invalid ;
}

/**
 * Checks subtype for a given type
 * @param 	   agId					Graph id
 * @param 	   type					Annotation type
 * @param[out] invalid_subtypes		Set for receiving the invalid subtypes
 * @return							True if at least 1 invalid subtype has been found
 * @attention						This method doesn't check if the type is available,
 * 									mainly because it has done to be called inside checkAnnotationTypes.
 * 									If you want to use it out of that method, check type first.
 */
bool ModelChecker::checkAnnotationSubtypes(const string& agId, const std::string& graphtype,
												const std::string& type,
												map<std::string,std::set<std::string> >& invalid_subtypes)
{
	bool invalidsubtypes = false ;

	// get all annotationsId for given type
	const std::set<AnnotationId>& l = GetAnnotationSet(agId, type) ;
	std::set<FeatureName>::const_iterator it  ;
	string subtypeName, subtype ;

	// For all id, get the value of the  feature that represents the substype
	// (if current annotation can have subtype)
	for (it=l.begin(); it!=l.end(); it++)
	{
		// find the feature name corresponding to subtype
		subtypeName = model->conventions().getSubtypeFeatureName(type, graphtype) ;
		if (!subtypeName.empty())
		{
			try 
			{
				subtype = GetFeature(*it, subtypeName) ;
				if ( ! subtype.empty())
				{
					bool valid = model->conventions().isValidSubtype(subtype, type, graphtype) ;
					if (!valid)
					{
						invalidsubtypes = true ;
						invalid_subtypes[type].insert(subtype) ;
					}
				}
			} 
			catch (...) 
			{
				// warn -> missing subtype
			}
		}
	}
	return invalidsubtypes ;
}

//------------------------------------------------------------------------------
//							      Conventions
//------------------------------------------------------------------------------

int ModelChecker::checkConventions(const std::string& convention_name, const std::string& version)
{
	bool convOK = false ;
	bool versionOK = false ;

	string dir = model->conventions().getDirectory() ;
	FileInfo infodir(dir) ;
	string path = infodir.join(convention_name) ;
	path = path + ".rc" ;
	string file_version = "" ;

	Log::trace() << "<<<\nchecking conventions: {" << convention_name << " ~ " << version << "} \n" ;

	//> Convention file exists, keep on going
	if (Glib::file_test(path, Glib::FILE_TEST_EXISTS)
			&& !Glib::file_test(path, Glib::FILE_TEST_IS_DIR)
			&& !Glib::file_test(path, Glib::FILE_TEST_IS_EXECUTABLE))
	{
		convOK = true ;

		if (version.empty())
			versionOK = false ;
		else
		{
			//> Temporary load the required conventions
			Conventions convention ;
			convention.configure(path, "", false) ;
			if (convention.loaded())
			{
				file_version = convention.version() ;
				// Version required and version in conventions are different, BAD
				if (file_version != version)
					versionOK = false ;
				// Otherwise, GOOD
				else
					versionOK = true ;
			}
			else
				versionOK = false ;
		}

		Log::trace() << "\t convention: {" << path << " - " << file_version << "} -> check version: [" << versionOK << "]\n>>>" << std::endl ;
	}
	//> Convention file doesn't exist, STOP
	else {
		convOK = false ;
		Log::trace() << "\t convention file couldn't be found: " << path << "]\n>>>" << std::endl ;
	}

	//> Update information
	if (!convOK) {
		bad_conv_file = path ;
		convention_error = MCHK_ERROR_CONV_FILE ;
		return MCHK_ERROR_CONV_FILE ;
	}
	else if (!versionOK) {
		bad_conv_version = version ;
		convention_error = MCHK_ERROR_CONV_VERSION ;
		return MCHK_ERROR_CONV_VERSION ;
	}
	else
		return 1 ;
}


int ModelChecker::getConventionLog(string& name, string& version)
{
	name = bad_conv_file ;
	version =  bad_conv_version ;
	return convention_error ;
}


//------------------------------------------------------------------------------
//							      Quick test
//------------------------------------------------------------------------------


bool ModelChecker::isFullyCorrect()
{
	bool graph = (graphs_priority_1.size()==0) && (graphs_priority_2.size()==0) && (added_graphs.size()==0) ;
	bool conv = (convention_error==MCHK_NO_ERROR) ;
	bool import = (m_import_warn.size()==0) ;
	return (graph && conv && import) ;
}

bool ModelChecker::isCorrect()
{
	bool warngraph = (graphs_priority_1.size()==0) && (graphs_priority_2.size()==0) ;
	bool conv = (convention_error==MCHK_NO_ERROR) ;
	bool import = (m_import_warn.size()==0) ;
	return (warngraph && conv && import) ;
}

bool ModelChecker::hasWarnings()
{
	return (graphs_priority_1.size()!=0) || (convention_error!=MCHK_NO_ERROR || (m_import_warn.size()!=0));
}

bool ModelChecker::hasErrors()
{
	return (graphs_priority_2.size()!=0) ;
}


//------------------------------------------------------------------------------
//							      Clean business
//------------------------------------------------------------------------------

bool ModelChecker::cleanSubtype(ModelChecker::CheckGraphResult* checkResult)
{
	if (!checkResult)
		return false ;

	bool result = true ;

	//> Recover graph information
	string graphtype = checkResult->get_graphType() ;
	string graphid = checkResult->get_graphId() ;

	//> Get all types and theirs invalid subtypes
	std::map<std::string, std::set<std::string> > invalids =  checkResult->get_invalidSubtypes() ;
	std::map<std::string, std::set<std::string> >::iterator it ;
	std::set<string> subtypes ;
	std::set<string>::iterator it_sub ;

	//> Clean all
	for (it=invalids.begin(); it!=invalids.end(); it++)
	{
		subtypes =  it->second ;
		for (it_sub=subtypes.begin(); it_sub!=subtypes.end(); it_sub++)
			result = result && cleanSubtype(graphid, graphtype, *it_sub, it->first) ;
	}
	return result ;
}

bool ModelChecker::cleanSubtype(const string& graphid, const string& graphtype, const string& subtype, const string& type)
{
	bool result = true ;
	int cpt = 0 ;

	// adjust GUI update frequency
	int flush_max = 10 ;

	//> Recover the TAG feature name corresponding to subtype for the given type
	string featureName = model->conventions().getSubtypeFeatureName(type, graphtype) ;

	//> Get all annotations of given type with corresponding feature
	std::set<std::string> annotations = GetAnnotationSetByFeature (graphid, featureName, subtype, type) ;
	std::set<std::string>::iterator it ;

	//> For all, clean
	for(it=annotations.begin(); it!= annotations.end(); it++)
	{
		result = result && model->deleteElement(*it, false) ;
		// time to time, let the GUI breathing ;)
		if (cpt%flush_max==0)
			m_signalFlushGUI.emit() ;
		cpt++ ;
	}
	return result ;
}


bool ModelChecker::cleanType(ModelChecker::CheckGraphResult* checkResult)
{
	if (!checkResult)
		return false ;

	bool result = true ;

	//> Recover graph information
	string graphtype = checkResult->get_graphType() ;
	string graphid = checkResult->get_graphId() ;

	//> Get all invalid types
	std::set<std::string> invalids =  checkResult->get_invalidTypes() ;
	std::set<std::string>::iterator it ;

	//> For all, clean
	for (it=invalids.begin(); it!=invalids.end(); it++)
		result = result && cleanType(graphid, graphtype, *it) ;

	return result ;
}

bool ModelChecker::cleanType(const string& graphid, const string& graphtype, const string& type)
{
	//> Cannot clean baseType (means graph totally invalid, cannot be cleaned)
	if (model->conventions().isMainstreamType(type, graphtype))
		return false ;

	bool result = true ;

	// Adjust GUI update frequency
	int flush_max = 10 ;
	int cpt = 0 ;

	//> Get all invalid annotations and delete them
	std::set<std::string> annotations = GetAnnotationSet(graphid, type) ;
	std::set<std::string>::iterator it ;
	for(it=annotations.begin(); it!= annotations.end(); it++) {
		result = result && model->deleteElement(*it, false) ;
		if (cpt%flush_max==0)
			m_signalFlushGUI.emit() ;
		cpt++ ;
	}

	return result ;
}

int ModelChecker::applyCleanActions()
{
	int cpt = 0 ;

	//> For each clean action procedeed
	std::map<std::string, CheckGraphResult*>::iterator it ;
	for (it=results.begin(); it!=results.end(); it++)
	{
		CheckGraphResult* result = it->second ;
		//> Get all error codes
		std::map<int,int> errors = result->get_errorCodes() ;
		std::map<int,int>::iterator it_error ;
		for (it_error=errors.begin(); it_error!=errors.end(); it_error++)
		{
			//> For candidate to clean, proceed
			if (it_error->second==1)
			{
				// clean
				bool fixed = fixError(result, it_error->first) ;
				// update clean action state
				if (fixed) {
					it_error->second = 2 ;
					cpt++ ;
				}
				else
					it_error->second = -1 ;
				// tell GUI to update state
				result->emitDisplaySignal(it_error->first, it_error->second) ;
			}
			// let GUI breathing ;)
			m_signalFlushGUI.emit() ;
		}
	}

	return cpt ;
}

bool ModelChecker::fixError(CheckGraphResult* checkResult, int errorCode)
{
	switch (errorCode)
	{
		case MCHK_ERROR_GRAPH_INVALIDTYPES : {
			TRACE << "fixing error Invalid Type" << std::endl ;
			return cleanType(checkResult) ;
			break ;
		}
		case MCHK_ERROR_GRAPH_INVALIDSUBTYPES : {
			TRACE << "fixing error Invalid Subtype" << std::endl ;
			return cleanSubtype(checkResult) ;
			break ;
		}
		case MCHK_ERROR_GRAPH_NOBASESEG : {
			TRACE << "fixing error no bas seg" << std::endl ;
			break ;
		}
		case MCHK_ERROR_GRAPH_NOCONTINUOUS : {
			TRACE << "fixing error not continuous" << std::endl ;
			break ;
		}
		case MCHK_ERROR_GRAPH_NOTINCONV : {
			TRACE << "fixing error graph not in conv" << std::endl ;
			break ;
		}
		case MCHK_ERROR_CONV_FILE : {
			TRACE << "fixing error conv file" << std::endl ;
			break;
		}
		case MCHK_ERROR_CONV_VERSION : {
			TRACE << "fixing error conv version" << std::endl ;
			break;
		}
		case MCHK_ERROR_IMPORT_WARNING : {
			TRACE << "fixing error import warning" << std::endl ;
			break ;
		}
		default :
			TRACE << "fixing error: error type unknown" << std::endl ;
			return false ;
	}
	return false ;
}

} // namespace
