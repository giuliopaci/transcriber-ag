/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/


#include <sstream>
#include <ag/AGException.h>
#include <ag/RE.h>
#include <ag/AGAPI.h>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/util/UTFDataFormatException.hpp>
#include "SGML.h"
#include "SAX_SGMLHandler.h"
#include "agfXercesUtils.h"
#include "Common/iso639.h"
#include "Common/util/FormatTime.h"
#include "Common/util/Utils.h"
#include "Common/util/StringOps.h"
#include "Common/Explorer_filter.h"
#include "Common/util/Log.h"

#define GAP_TOLERANCE 3
#define EPSILON_ADJUST 0.05

#define DEFAULT_CONVENTIONS "transag_default"
#define DEFAULT_LANGUAGE "eng"

#define WITH_TRACE false


//******************************************************************************
//********************************** IMPORT ************************************
//******************************************************************************


list<AGId> SGML::load(const string& filename, const Id& id,
						map<string,string>* signalInfo, map<string,string>* options)
throw (agfio::LoadError)
{
	m_duration = -1 ;
	m_fromScratch = false ;

	Log::setTraceLevel(Log::OFF) ;

	try {
		xercesc_open();
	}
	catch (const agfioError& e) {
		Glib::ustring err = Glib::ustring(_("Error while importing format:")) + "\n" + Glib::ustring(e.what()) ;
		Log::setTraceLevel(Log::OFF) ;
		throw agfio::LoadError(err);
	}

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>> PREPARE OPTIONS >>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	bool val_opt = true;
	string encoding("");
	string localDTD("");
	string addr("");
	string agsetId ("");
	string conventions=DEFAULT_CONVENTIONS;
	string lang = DEFAULT_LANGUAGE;

	if (options != NULL)
	{
		if ((*options)["dtd"] != "")
			localDTD = (*options)["dtd"];
		if ((*options)["encoding"] != "")
			encoding = (*options)["encoding"];
		if ((*options)["DTDvalidation"] == "false")
			val_opt = false;

		map<string,string>::iterator it;
		string item;

		if ( (it = options->find("&datamodel")) != options->end() )  addr = it->second;
		if ( (it = options->find("corpusName")) != options->end() )  agsetId = it->second;


		if ( (it = options->find("lang")) != options->end() )  item = it->second;
		if ( item.length() == 2 ) {
			const char* pl = ISO639::get3LetterCode(item.c_str());
			if ( pl != NULL ) lang = pl;
		} else if (item.length() == 3) { // check validity
			const char* pl = ISO639::get2LetterCode(item.c_str());
			if ( pl != NULL ) lang=item;
		}

		if ( (it = options->find("conventions")) != options->end() ) {
			conventions = it->second;
		}
	}


	//>>>>>>>>>>>>>>>>>>>>>>>>>>>> DATA MODEL >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	tag::DataModel* data = NULL;
	bool do_del = false;

	if (  addr != "" ) {
		// data model adress passed by caller -> use it
		Log::err() << "data model adress passed by caller -> use it" << endl;
		data = (tag::DataModel*)strtoul(addr.c_str(), NULL, 16);
	}
	else {
		// make a unique Set ID
		string prefix = "txml_";
		int no = 0;
		do {
			++no;
			ostringstream os;
			os << prefix << no;
			agsetId = os.str();
		}
		while ( ExistsAGSet(agsetId) );

		data = new tag::DataModel();
		data->setKeepAG(true); // do not delete graph upon datamodel deletion
		do_del = true;
	}

	if ( data->getAGPrefix() == "" )
		data->initAGSet(agsetId);

	Log::trace() << "BEFORE CONFIGURE=" << data->getAGPrefix() << " agtrans="<< data->getAGTrans() << endl;

	// If conventions haven't been previously set let's configure
	// (could have been set before plugin call when identifying import format)
	if ( !data->conventions().loaded() )
	{
		try {
			data->setConventions(conventions, lang);
		}
		catch ( const char* msg ) {
			Log::err() << msg << endl;
			data->setConventions(conventions);
		}
	}

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>> PARSE FILE >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	bool ok = true ;
	std::string input_encoding = encoding ;

	if (input_encoding.empty())
	{
		Log::trace() << "no encoding specified, trying to guess" << std::endl ;
		input_encoding = tag::FormatToUTF8::guessFileEncoding(filename) ;
		if (input_encoding.empty())
			ok = false ;
		else
			Log::trace() << "using encoding [" << input_encoding << "]" << std::endl ;
	}

	list<AGId> result ;

	if (ok)
	{
		SGMLobjects SGML_objects ;
		SAX_SGMLHandler handler(*data, &SGML_objects, "UTF-8") ;

		try {
			SGMLagfSAXParse(&handler, filename, val_opt, input_encoding);
		}
		catch (const XMLException& e) {
			Glib::ustring err = Glib::ustring(_("Error while importing format:")) + "\n" + Glib::ustring(trans(e.getMessage())) ;
			Log::setTraceLevel(Log::OFF) ;
			throw agfio::LoadError(err);
		}
		catch (AGException& e) {
			Glib::ustring err = Glib::ustring(_("Error while importing format:")) + "\n" + Glib::ustring(e.error()) ;
			Log::setTraceLevel(Log::OFF) ;
			throw agfio::LoadError(err) ;
		}

		//>>>>>>>>>>>>>>>>>>>>>>>>>>>> FILL MODEL >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

		try
		{
			getFiles(data, filename) ;

			//> return AgId used for special treatments in datamodel loading
			if ( data != NULL)
			{
				string err ;
				bool success = fillModels(data, &SGML_objects, err) ;
				if (success)
				{
					if (m_fromScratch)
						result.push_back(data->getAG("transcription_graph"));
					result.push_back(data->getAG("alignmentREF_graph"));
					result.push_back(data->getAG("alignmentHYP_graph"));
				}
				else
				{
					Log::setTraceLevel(Log::OFF) ;
					data->cleanModelChecker() ;
					throw agfio::LoadError(err) ;
				}
			}
		}
		catch (AGException& e)
		{
			Glib::ustring err = Glib::ustring(_("Error while importing format:")) + "\n" + Glib::ustring(e.error()) ;
			Log::setTraceLevel(Log::OFF) ;
			throw agfio::LoadError(err) ;
		}
	}
	else
	{
		string error = Glib::ustring(_("Error while importing format:")) + "\n"
						+ Glib::ustring(_("Cannot determinate file encoding")) ;
		Log::setTraceLevel(Log::OFF) ;
		throw agfio::LoadError(error);
	}

	xercesc_close() ;

	Log::setTraceLevel(Log::OFF) ;

	return result ;
}

void SGML::getFiles(tag::DataModel* data, string filepath)
{
	//> Prepare file
	string path = tag::Explorer_filter::cut_extension(filepath) ;
	string directory = Glib::path_get_dirname(filepath) ;
	string name = Glib::path_get_basename(path) ;

	m_audioFile = tag::Explorer_filter::lookForMediaFile(directory, name, "audio") ;
	m_sgmlFile = filepath ;

	m_displayFile = tag::Explorer_filter::lookForFile(directory, name, "trs") ;
	m_displayType = "TRS" ;
	if (m_displayFile.empty()) {
		m_displayFile = tag::Explorer_filter::lookForFile(directory, name, "stm") ;
		m_displayType = "STM" ;
	}
	if (m_displayFile.empty()) {
		m_displayFile = tag::Explorer_filter::lookForFile(directory, name, "ctm") ;
		m_displayType = "CTM" ;
	}

	Log::trace() << "FILE audio=" << m_audioFile << std::endl ;
	Log::trace() << "FILE display=" << m_displayFile << std::endl ;
	Log::trace() << "FILE data=" << m_sgmlFile << std::endl ;
}

bool SGML::fillDisplayModel(tag::DataModel* data, string& err)
{
	//> -- UGLY call to another plugin, do it it other way ?
	bool success = data->loadFromFile(m_displayFile, m_displayType, true) ;
	if (!success)
		err = "SGML error: display graphes preparation failed." ;
	return success ;
}

bool SGML::fillAlignModel(tag::DataModel* data, SGMLobjects* structure, string& err)
{
	string GRAPH_HYP = "alignmentHYP_graph" ;
	string GRAPH_REF = "alignmentREF_graph" ;
	string LANG = "fra" ;
	string SCRIBE = "SGMLtoTAG" ;
	string FORMAT = "SGML" ;
	string ANNOTYPE_REF = "alignmentREF" ;
	string ANNOTYPE_HYP = "alignmentHYP" ;
	string GRAPH_TRANS = "transcription_graph" ;
	string ANNOTYPE_TRANS = "unit" ;

	string curr_unit = "" ;
	string curr_segment = "" ;
	string curr_turn = "" ;
	string curr_section = "" ;

	bool with_sect =  ( data->conventions().isMainstreamType("section", GRAPH_TRANS));

	if (m_fromScratch) {
		curr_unit = data->getByOffset(ANNOTYPE_TRANS, 0.0, 0, GRAPH_TRANS) ;
		curr_segment = data->getParentElement(curr_unit) ;
		curr_turn = data->getParentElement(curr_segment) ;
		if ( with_sect ) {
			curr_section = data->getParentElement(curr_turn) ;
			data->setElementProperty(curr_section, "type", "nontrans");
		}
		data->deleteElementProperty(curr_turn, "speaker");
		Log::trace() << "\t loading data from " << m_sgmlFile << " too" << endl ;
	}

	bool ok ;
	int cpt_empty_path = 0 ;
	int cpt_path = 0 ;
	int cpt_entry = 0 ;

	ok = addGraph(data, FORMAT, GRAPH_REF, LANG, SCRIBE) ;
	ok = ok && addGraph(data, FORMAT, GRAPH_HYP, LANG, SCRIBE) ;

	if (!ok)
	{
		Log::trace() << "\t adding graphes for alignement: failed... aborted." << endl ;
		err = "SGML error: model graphes preparation failed." ;
		return false ;
	}

	int last = structure->getLastSequence() ;

	string last_was_d_str = "" ;
	string last_elem_ref = "" ;
	string last_elem_hyp = "" ;
	string last_trans_segment = "" ;

	Log::trace() << "NUMBER OF PATHS= " << structure->getNBpaths() << std::endl ;
	Log::trace() << "last_seq= " << last << std::endl ;


	// for all sequence number (i.e all path)
	for (guint i=0 ; i < last+1 ; i++)
	{
		SGMLobjects::SGMLpath* current_path = structure->getPathN(i) ;

		if (!current_path || current_path->entries.size()==0) {
//			Log::trace() << "\n ~~~~~~~~~~~~~~~~~~~~~~~~~~ PATH null. skip !" << endl ;
			cpt_empty_path++ ;
			continue ;
		}
		else
			cpt_path++ ;

		Log::trace() << "\n ~~~~~~~~~~~~~~~~~~~~~~~~~~ PATH start=" << current_path->R_T1 << " - end=" << current_path->R_T2 << endl ;

		float start_path = current_path->R_T1 ;
		float end_path = current_path->R_T2 ;
		float last_start = start_path ;
		float last_end = -1 ;
		string last_type = "" ;
		string concat_txt = "" ;
		int first_entry = 1 ;
		bool last_was_invalid ;

		//> for each entries of path fill ag model
		std::vector<SGMLobjects::SGMLentry*> entries = current_path->entries ;
		std::vector<SGMLobjects::SGMLentry*>::iterator itent ;
		for (itent = entries.begin(); itent != entries.end(); itent++)
		{
			cpt_entry++ ;

			std::vector<SGMLobjects::SGMLentry*>::iterator tmp = itent ;
			tmp ++ ;
			bool last_entry = (tmp==entries.end()) ;

			//> current entry data
			SGMLobjects::SGMLentry* current_entry = *itent ;
			float start_t = current_entry->start_time ;
			float end_t = current_entry->end_time ;
			string type = current_entry->type ;
			string hyp_w = current_entry->hyp_word ;
			string ref_w = current_entry->ref_word ;

			Log::trace() << "\n (Entry " << start_t << " - " << end_t << " : " << type << " : " << ref_w << endl ;

			bool timeOK = checkTime(current_entry, current_path) ;

			//> REFERENCE CASE
			if (  isHandledType(type, "REF") )
			{
				//> FIRST ENTRY
				if (first_entry==1)
				{
					//> DELETION and SUBSTITUTION have not (D) or bad (S) timecode,
					//  we use path timecode instead
					if ( type.compare("D")==0 || type.compare("S")==0) {
						last_start = start_path ;
						last_end = end_path ;
					}
					else {
						last_start = start_t ;
						last_end = end_t ;
					}

					last_type = type ;
					concat_txt = ref_w ;

					if (timeOK)
					{
						// If element is valid, first is passed
						first_entry = 0 ;
						// *TIME ADJUSTMENT*
						//> check Special case: current start offset is smaller than
						//  previous element end offset
						adjustPreviousEnd(last_elem_ref, last_start, data) ;
					}

					// if it's unique entry, let's create it
					if (last_entry) {
						last_elem_ref = createAlignment(type, last_start, last_end, concat_txt, GRAPH_REF, ANNOTYPE_REF, data) ;
						Log::trace() << "REF : unique created " << last_elem_ref << " between " << last_start << " and " << last_end << std::endl ;
					}
				} /*end first*/
				//> OTHER CASES
				else
				{
					//> TIME IS OK
					if (timeOK)
					{
						bool different_type_case = true ;

						//> DELETION doesn't have timecode and align its timecode with previous
						//  or next entry, but only if next ISN'T
						bool force_concat = ( (last_type.compare("D")==0) && (type.compare("S")==0) ) ;

						//> Same type than previous one : actualize information and
						//  iterate
						if (type.compare(last_type)==0 || force_concat)
						{
							different_type_case = false ;

							// actualize information for next iteration
							if ( type.compare("D")!=0 )
							{
								//> if there's a gap between previous end and current start
								//  let's force a separation
								//  (CAUTION: even if current start < last_end - i.e invalid order -
								//  we let the same element )
								//> If we're processing special case DELETION - SUBSTITUTION,
								//  concat anyway
								if ( (start_t - last_end) > 0 && !force_concat)
									different_type_case = true ;
								//> Otherwise, stay on the same element
								else
									last_end = end_t ;
							}

							if (force_concat)
								last_type = type ;

							if (!different_type_case)
							{
								concat_txt = concat_txt + " " + ref_w ;

								// if it's last entry, let's create the previous/current element
								if (last_entry)
								{
									//> DELETION and SUBSTITUTION have not (D) or bad (S) timecode,
									//  we use path timecode instead
									if ( (type.compare("D")==0) || (type.compare("S")==0) )
										last_end = end_path ;

									last_elem_ref = createAlignment(type, last_start, last_end, concat_txt, GRAPH_REF, ANNOTYPE_REF, data) ;
									Log::trace() << "REF : last created " << last_elem_ref << " between " << last_start << " and " << last_end << std::endl ;
								}
							}
						}
						//> change of type : create previous and deals with current
						if (different_type_case)
						{
							float start, end ;

//							adjustPreviousEnd(last_elem_ref, start_t, data) ;

							// STEP 1 : create previous
							// Start from last start of same type we found
							start = last_start ;
							//> DELETION and SUBSTITUTION have not (D) or bad (S) timecode,
							//  we use time code of next and previous element.
							//  In that case, the next element is the current
							if ( last_type.compare("D")==0 || last_type.compare("S")==0)
								end = start_t ;
							//   Otherwise, we end at current element start
							else
								end = last_end ;

							//> TIMECODE SGML are not precise and sometimes a DELETION
							//  get start and end from previous and next element that
							//	are the same. In this case, let's adjust previous element
							//  to finish a little earlier for inserting the deletion.
							if (start>=end && last_type.compare("D")==0)
							{
								start = start - (start-end) - EPSILON_ADJUST ;
								Log::trace() << "REF : start==end => adjust previous end " << last_elem_ref << " at " << start << std::endl ;
								adjustPreviousEnd(last_elem_ref, start, data) ;
							}

							last_elem_ref = createAlignment(last_type, start, end, concat_txt, GRAPH_REF, ANNOTYPE_REF, data) ;
							Log::trace() << "REF : previous created " << last_elem_ref << " between " << start << " and " << end << " - type:" << last_type << std::endl ;

							concat_txt = ref_w ;

							// STEP-2 : actualize information for next iteration
							//> DELETION and SUBSTITUTION have not (D) or bad (S) timecode,
							//  so the last start is the end of the previous we have created,
							//  the end is the end we have created
							if (type.compare("D")==0 || type.compare("S")==0) {
								last_start = end ;
								last_end = end ;
							}
							// If current isn't a D or S, last start becomes current start,
							// and last end becomes current end
							else {
								last_start = start_t ;
								last_end = end_t ;
							}

							last_type = type ;

							// STEP-3 : if it's last element, create it !
							if (last_entry)
							{
								float start, end ;
								//> DELETION and SUBSTITUTION have not (D) or bad (S) timecode,
								//  let's use path or previous element information
								if ( type.compare("D")==0 || type.compare("S")==0 ) {
									start = last_end ;
									end = end_path ;
								}
								else {
									start = start_t ;
									end = end_t ;
								}
								last_elem_ref = createAlignment(type, start, end, concat_txt, GRAPH_REF, ANNOTYPE_REF, data) ;
								Log::trace() << "REF : last created " << last_elem_ref << " between " << start << " and " << end << std::endl ;
							}
						} /*end lasttype != type*/
					} /*end timeOK*/
					//> Time KO and we're the last entry: don't forget to create the previous one
					else if (last_entry)
					{
						float start, end ;

						start = last_start ;

						//> DELETION and SUBSTITUTION have not (D) or bad (S) timecode,
						//  let's use path or previous element information
						if ( last_type.compare("D")==0 || last_type.compare("S")==0 )
							end = start_t ;
						else
							end = last_end ;

						last_elem_ref = createAlignment(last_type, start, end, concat_txt, GRAPH_REF, ANNOTYPE_REF, data) ;
						Log::trace() << "REF : last isn't timeOK: previous created " << last_elem_ref << " between " << start << " and " << end << std::endl ;

						last_was_invalid = true ;

					} /*end timeKO && last*/
				} /*end !first*/
			} /*end REF case*/

			//> HYPOTHESIS CASE
			if ( isHandledType(type, "HYP") )
			{
				float start, end ;
				if (timeOK)
				{
					start = start_t ;
					end = end_t ;

					//> adjust previous frontier in case previous end would be
					//  greater than current start (adjust previous end)
					adjustPreviousEnd(last_elem_hyp, start, data) ;

					//> Create current element
					last_elem_hyp = createAlignment(type, start, end, hyp_w, GRAPH_HYP, ANNOTYPE_HYP, data) ;
					Log::trace() << "HYP : create " << last_elem_hyp << " between " << start << " and " << end << std::endl ;
				}

				//> If we are the last entry of the path and an insertion, don't forget to
				// create the previous element of reference
				// (cause reference case doesn't handle type Insertion)
				if (last_entry && type.compare("I") == 0 && first_entry!=0 && first_entry!=1)
				{
					start = last_start ;
					if ( last_end > 0 )
						end = last_end ;
					else if (timeOK)
						end = start_t ;
					else
						end = end_path ;

					last_elem_ref = createAlignment(last_type, start, end, concat_txt, GRAPH_REF, ANNOTYPE_REF, data) ;
					Log::trace() << "REF : last from HYP: create previous from ref" << last_elem_ref << " between " << start << " and " << end << std::endl ;
				}
			} /* end of HYP */

			//> FROM SCRATCH: fill transcription model
			if ( m_fromScratch)
			{
				if (timeOK && type.compare("D")!=0 )
				{
					if (!curr_unit.empty())
					{
						float last_end = data->getElementOffset(curr_unit, false) ;
						if (start_t!=last_end && last_end!=m_duration) {
							curr_unit = createTranscriptionAnnotation(curr_unit, last_end, hyp_w, data) ;
							Log::trace() << "TRANS : closing creating =" << curr_unit << " at " << start_t << std::endl ;
						}
					}
					curr_unit = createTranscriptionAnnotation(curr_unit, start_t, hyp_w, data) ;
					Log::trace() << "TRANS : created prevId=" << curr_unit << " at " << start_t << std::endl ;
				}
				else if (last_entry)
				{
					if (!curr_unit.empty())
					{
						float last_end = data->getElementOffset(curr_unit, false) ;
						if (last_end!=m_duration) {
							curr_unit = createTranscriptionAnnotation(curr_unit, last_end, hyp_w, data) ;
							Log::trace() << "TRANS : LAST closing creating =" << curr_unit << " at " << start_t << std::endl ;
						}
					}
				}
			}

			// remove first entry flag if passed
			if (first_entry==0)
				first_entry = -1 ;

		} /*end loop on entries*/
	} /*end all sequences*/

	Log::trace() << "empty paths detected: " << cpt_empty_path << std::endl ;
	Log::trace() << "valid paths detected: " << cpt_path << std::endl ;
	Log::trace() << "entries detected: " << cpt_entry << std::endl ;
	return true ;
}

bool SGML::adjustPreviousEnd(const string last_id, float current_start, tag::DataModel* data)
{
	bool res = false ;
	if (!last_id.empty() && current_start != -1)
	{
		float previous_end = data->getElementOffset(last_id,false) ;
		if (previous_end > current_start) {
//			Log::trace() << "\t\t(adjusting end offset for " << last_id << " from " << previous_end << " to " << current_start << std::endl ;
			data->setElementOffset(last_id,current_start,false, false) ;
			res = true ;
		}
	}
	return res ;
}


bool SGML::initializeFromScratch(SGMLobjects* pars, tag::DataModel* data)
{
	//> Get duration
	int last = pars->getLastSequence() ;
	SGMLobjects::SGMLpath* lastPath = pars->getPathN(last) ;
	if (lastPath)
	{
		m_duration = lastPath->getLastTime() ;
		data->setSignalDuration(m_duration);
		Log::trace() << "signal duration = " << m_duration << std::endl ;
	}

	//> Indicates signal to model
	// TODO EMPTY SIGNAL
	string path = tag::Explorer_filter::cut_extension(m_sgmlFile) ;
	string directory = Glib::path_get_dirname(m_sgmlFile) ;
	string name = Glib::path_get_basename(path) ;
	m_audioFile = tag::Explorer_filter::lookForMediaFile(directory, name, "audio") ;
	data->addSignal(m_audioFile, "audio", "", "", 1, 1, false) ;

	//> Init !
	data->initAnnotationGraphs("", "fra", "SGMLtoTAG", true) ;
	return true ;
}

string SGML::createAlignment(const string& alignType, float start, float end, const string& text, const string& graphType, const string& annotType, tag::DataModel* data)
{
	string id = data->createParseElement("", graphType, annotType, 0, start, end) ;

	if (!id.empty())
	{
		data->setElementProperty(id, "value", text, false) ;
		data->setElementProperty(id, "type", alignType, false) ;
	}

	return id ;
}

string SGML::createTranscriptionAnnotation(const string& prevId, float start, const string& text, tag::DataModel* data)
{
	m_lastOffset = start;

	string id = data->insertMainstreamElement("segment", prevId, start);
	if (!id.empty()) {
		string baseid = data->getAlignmentId(id);
		data->setElementProperty(baseid, "subtype", "unit_text", false) ;
		data->setElementProperty(baseid, "value", text, false) ;
		return baseid ;
	}
	else
		return "" ;
}

bool SGML::fillModels(tag::DataModel* data, SGMLobjects* structure, string& err)
{
	bool success = false ;

	if (m_displayFile.empty())
	{
		m_fromScratch = true ;
		m_displayFile = m_sgmlFile ;
		success = initializeFromScratch(structure, data) ;
	}
	//> fill display
	else
	{
		string fail ;
		Log::trace() << "\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> agfioSGML: preparing display from " << m_displayFile << endl ;
		success = fillDisplayModel(data, fail) ;
		if (!success)
			err = fail ;
		Log::trace() << "\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< display done: {" << success << "}\n" << endl ;
	}

	//> fill align
	Log::trace() << "\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> agfioSGML: preparing model from " << m_sgmlFile << endl ;

	string fail ;
	success = success && fillAlignModel(data, structure, fail) ;
	if (!success && err.empty())
		err = fail ;
	else if (!success)
		err = err + "\n" + fail ;

	Log::trace() << "\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> model done: {" << success << "} \n"  << endl ;
	return success ;
}

bool SGML::isHandledType(const string& type, const string& mode)
{
	bool res = false ;
	if ( mode.compare("REF") == 0 ) {
		if ( type.compare("D") ==0 || type.compare("C")==0 || type.compare("S")==0 )
			res = true ;
	}
	else if ( mode.compare("HYP") == 0 ) {
		if ( type.compare("I")==0 || type.compare("C")==0 || type.compare("S")==0 )
			res = true ;
	}
	return res ;
}

bool SGML::checkTime(SGMLobjects::SGMLentry* entry, SGMLobjects::SGMLpath* path)
{
	if (!entry || !path)
		return false ;

	float start_e = entry->start_time ;
	float end_e = entry->end_time ;
	float start_p = path->R_T1 ;
	float end_p = path->R_T2 ;

	if (start_e ==-1 && end_e == -1)
		return true ;

	bool start_in_range = start_p <= start_e && start_e <= end_p ;
	float diff_start = start_p - start_e ;
	bool start_ok = start_in_range || diff_start<=GAP_TOLERANCE ;

	bool end_in_range = start_p <= end_e && end_e <= end_p ;
	float diff_end = end_e - end_p ;
	bool end_ok = end_in_range || diff_end<=GAP_TOLERANCE ;

	if (start_ok && end_ok)
		return true ;
	else {
		Log::trace() << "CHECK-TIME failure: start=" << start_ok << " - end=" << end_ok << std::endl ;
		Log::trace() << "\tstartGAP=" << diff_start << std::endl ;
		Log::trace() << "\tendGAP=" << diff_end << std::endl ;
		Log::trace() << "\t entry =" << entry->toString() << std::endl ;
		Log::trace() << "\t path =" << path->toString() << std::endl ;
		return false ;
	}
}

//******************************************************************************
//********************************** initialize graph ************************************
//******************************************************************************


bool SGML::addGraph(tag::DataModel* data, const string& format, const string& graphtype, const string& lang, const string& scribe)
{
	tag::Parameters* SGMLconv = data->getConventionsFromFormat(format) ;
	if (!SGMLconv)
		return false ;

	bool res = data->conventions().addGraphDescription(graphtype, SGMLconv) ;

	TRACE << "DM:: adding additional graph " << graphtype  << " [" << res << "]" << std::endl ;

	if (res) {
		data->initAnnotationGraphs(graphtype, lang, scribe) ;
	}
	else if (SGMLconv)
		delete SGMLconv ;
	return res ;
}

//******************************************************************************
//********************************** EXPORT ************************************
//******************************************************************************

string SGML::store(const string& filename, const string& id, map<string,string>* options)
throw (agfio::StoreError)
{
	ofstream out(filename.c_str());
	if (! out.good())
		throw agfio::StoreError("SGML::store():can't open "+filename+"for writing");
	string encoding, dtd;
	if (options) {
		encoding = (*options)["encoding"];
		dtd = (*options)["dtd"];
	}
	if (encoding.empty())
		out << "<?xml version=\"1.0\"?>" << endl;
	else
		out << "<?xml version=\"1.0\" encoding=\""
		<< encoding << "\"?>" << endl;
	if (dtd.empty())
		out << "<!DOCTYPE AGSet SYSTEM \"http://agtk.sf.net/doc/xml/ag-1.1.dtd\">" << endl;
	else
		out << "<!DOCTYPE AGSet SYSTEM \""
		<< dtd << "\">" << endl ;
	out << toXML(id).substr(55) ;
	return "";
}


