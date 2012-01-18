/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
// TRS.cc: TRS loader class implementation
// Paule Lecuyer
// Copyright (C) 2007 Bertin Technologies
// Web: http://www.bertin.fr/
// For license information, see the file `LICENSE' included
// with the distribution.


#include <fstream>
#include <ag/AGException.h>
#include <ag/RE.h>
#include <ag/AGAPI.h>
#include "TRS.h"
#include "agfXercesUtils.h"

//------------------------------------------------------------------------------
//										READ
//------------------------------------------------------------------------------

list<AGId> TRS::load(const string& filename, const Id& id, map<string,string>* signalInfo, map<string,string>* options)
throw (agfio::LoadError)
{
	try
	{
		xercesc_open();
	}
	catch (const agfioError& e)
	{
		throw agfio::LoadError(string("TRS:") + e.what());
	}

	bool val_opt = true;
	string encoding = "";
	dataModel = NULL ;
	mapping = NULL ;

	// -- Load and configure parsing options
	SAX_TRSHandlers handler("UTF-8");
	setFormatOptions(options, handler, val_opt, encoding) ;

	try
	{
		TRSagfSAXParse(&handler, filename, val_opt, encoding);
	}
	catch (const XMLException& e)
	{
		throw agfio::LoadError(string("TRS:loading failed due to the following error\n") + trans(e.getMessage()));
	}
	catch (AGException& e)
	{
		throw agfio::LoadError("TRS:loading failed due to the following error\n" + 	e.error());
	}

//	list<AGId> result = handler.get_agids();

	//> -- Get created elements id
	list<AGId> result  ;
	if ( dataModel != NULL  )
	{
		result.push_back(dataModel->getAG("transcription_graph"));
		result.push_back(dataModel->getAG("background_graph"));
	}

	//> -- Throw warnings message to upper layer
	prepare_error_user(dataModel, &handler, mapping) ;

	xercesc_close();

	return result;
}

void TRS::setFormatOptions(map<string,string>* options, SAX_TRSHandlers& handler, bool& val_opt, string& encoding)
{
	if ( !options )
		return ;

	string agsetId = "" ;

	// -- Agid (corpus)
	map<string,string>::iterator it = options->find("corpusName") ;
	if ( it != options->end() )
		agsetId = it->second;

	it = options->find("&datamodel") ;
	if ( it != options->end() )
	{
		string addr = it->second ;
		if (  addr != "" )
			dataModel = (tag::DataModel*)strtoul(addr.c_str(), NULL, 16);
	}
	initializeDataModel(agsetId) ;
	handler.setTAGmodel(dataModel) ;

	// -- Dtd
	it = options->find("dtd") ;
	if ( it != options->end() )
	{
		string value = it->second ;
		if (!value.empty())
			handler.set_localDTD(value) ;
	}

	// -- Validation
	it = options->find("DTDvalidation") ;
	if ( it != options->end() )
	{
		string value = it->second ;
		if (value=="false")
			val_opt = false ;
	}

	// -- Encoding
	it = options->find("encoding") ;
	if ( it != options->end() )
	{
		string value = it->second ;
		if (!value.empty())
			encoding = it->second ;
	}

	// -- Mapping
	it = options->find("&qualifierMapping") ;
	if ( it != options->end() )
	{
		string addr = it->second ;
		if (  addr != "" )
		{
			mapping = (tag::Parameters*)strtoul(addr.c_str(), NULL, 16);
			handler.set_mapping(mapping) ;
		}
	}
}

void TRS::initializeDataModel(string agsetId)

{
	if (!dataModel)
	{
		// make a unique Set ID
		string prefix = "trs_";
		int no = 0;
		do
		{
			++no;
			ostringstream os;
			os << prefix << no;
			agsetId = os.str();
		}
		while ( ExistsAGSet(agsetId) ) ;
		dataModel = new tag::DataModel();
		// do not delete graph upon datamodel deletion
		dataModel->setKeepAG(true);
	}

	// -- Check datamodel prefix
	if ( dataModel->getAGPrefix() == "" )
		dataModel->initAGSet(agsetId);

	if (dataModel)
		dataModel->anchorLinks().setModel(dataModel) ;
}


//------------------------------------------------------------------------------
//										FEEDBACK
//------------------------------------------------------------------------------

void TRS::prepare_error_user(tag::DataModel* model, SAX_TRSHandlers* handler, tag::Parameters* mapping)
{
	string head = "" ;
	std::vector<std::string> warns = handler->getMappingWarnings() ;

	if (model && mapping)
	{
		// check if convention are specified for import
		string convention_path = model->getConversionConventionFile(model->getLoadedFileFormat()) ;
		// if none look msg
		if (convention_path.empty()) {
			head = head + ">> " + string(_("NO CONVENTION FILE for TRS import specified !"))  ;
			head = head + "\n   " + string(_("Display can be altered.")) + "\n" ;
		}
		if (!head.empty())
			warns.insert(warns.begin(), head) ;
		model->setImportWarnings(warns) ;
	}
	//> if no mapping found, tell the upper layer
	else if (model)
	{
		head = head + ">> " + string(_("NO MAPPING FILE for TRS import !")) ;
		head = head + "\n   " + string(_("Format conversion can be incomplete.")) + "\n" ;
		if (!head.empty())
			warns.insert(warns.begin(), head) ;
		model->setImportWarnings(warns) ;
	}
}

//------------------------------------------------------------------------------
//										WRITE
//------------------------------------------------------------------------------

string TRS::store(const string& filename, const string& id, map<string,string>* options)
throw (agfio::StoreError)
{
	ofstream out(filename.c_str());

	if (! out.good())
		throw agfio::StoreError("TRS::store():can't open "+filename+"for writing");

	string encoding, dtd;

	if (options)
	{
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
		<< dtd << "\">" << endl;

	out << toXML(id).substr(55);
	return "";
}
