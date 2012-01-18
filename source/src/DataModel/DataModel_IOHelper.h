/*
 * DataModel_IOHelper.h
 *
 *  Created on: 24 mai 2010
 *      Author: lecuyer
 */

#ifndef DATAMODEL_IOHELPER_H_
#define DATAMODEL_IOHELPER_H_

#include <set>
#include <vector>
#include <string>
using namespace std;
#include "DataModel/DataModel.h"

namespace tag {

/**
 * @class DataModel_IOHelper
 * @ingroup DataModel
 * DataModel IO operations helper
 */
class DataModel_IOHelper
{

public:

	/**
	 * constructor
	 * @param data target data model
	 */
	DataModel_IOHelper(DataModel& data) : m_data(data) {};


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
	 * @param replaceIfExists		If set to true and given key already exists, replaces the existing value
	 */
	void addAGOption(string key, string value, bool replaceIfExists=true);

	/**
	 * formats import - get conversion convention file
	 * @param format
	 * @param param
	 * @return convention file path
	 * @todo to move to configurator object
	 */
	Glib::ustring getConversionConventionFile(Glib::ustring format, Parameters* param=NULL) ;
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

private:

	DataModel& m_data;
};

};	// namespace tag


#endif /* DATAMODEL_IOHELPER_H_ */
