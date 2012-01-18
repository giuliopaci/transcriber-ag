/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>

#ifndef _HAVE_FILEINFO_HH
#define _HAVE_FILEINFO_HH

using namespace std;

/**
 * @class 		FileInfo
 * @ingroup		Common
 *
 *	Class with useful methods for manipulating file paths.
 */
class FileInfo
{

	public:
		/**
		 * Constructor
		 * @param path	File path
		 */
		FileInfo(const char* path)
		: _path(path), _dirname(NULL), _basename(NULL) {}

		/**
		 * Constructor
		 * @param path	File path
		 */
		FileInfo(const string& path)
		: _path(path), _dirname(NULL), _basename(NULL)  {}

		~FileInfo() {
			if ( _dirname != NULL ) free(_dirname);
			if ( _basename != NULL ) free(_basename);
		}

		/**
		 * Checks existence
		 * @return		True if the file exists
		 */
		bool exists();

		/**
		 * Checks existence and given type
		 * @param type	"f" for file, "d" for directory
		 * @return		True if the file exists and corresponds to given type
		 */
		bool exists(const char *type);

		/**
		 * Checks the write permission
		 * @return		True if the write permissions are ok.
		 */
		bool canWrite();

		/**
		 * Checks if the file is a regular file
		 * @return		True if the file is a regular file, i.e not a symlink or directory
		 * @see 		exists(const char*)
		 */
		bool isFile() { return exists("f"); }

		/**
		 * Checks if the file is a directory
		 * @return		True if the file is a directory
		 * @see 		exists(const char*)
		 */
		bool isDirectory() { return exists("d"); }

		/**
		 * Gets the file size
		 * @return		File size
		 */
		int size();

		/**
		 * Gets the file modification time
		 * @return		The file modification time
		 */
		time_t mtime();

		/**
		 * Compute the basename of the file path given to the constructor
		 * @return		File basename
		 */
		const char* Basename();

		/**
		 * Compute the dirname of the file path given to the constructor
		 * @param uplevels		Level of backward research inside the path
		 * @note				For getting the direct dirname, set uplevels to 1 (default value)
		 * @return				Computed directory path
		 */
		const char* dirname(int uplevels=1);

		/**
		 * @return File mimetype
		 */
		const char* mimetype();

		/**
		 * @return File mimeclass
		 */
		const char* mimeclass();

		/**
		 * @return File extension
		 */
		const char* tail();

		/**
		 * @return File name without extension
		 */
		const char* rootname();

		/**
		 * Defines file extension
		 * @param ext	File extension to be added
		 */
		void setTail(string ext);

		/**
		 * Gets the file path in char* format
		 * @return		File path
		 */
		const char* path() { return _path.c_str(); }

		/**
		 * Resolves relative path such as ~/path or ./path into real path
		 * @return		Real absolute path
		 */
		const char* realpath();

		/**
		 * Joins the given extension to the file name
		 * @param ext	Extension to be added
		 * @return		The resulted path
		 */
		const char* join(const char* ext);

		/**
		 * Joins the given extension to the file name
		 * @param ext	Extension to be added
		 * @return		The resulted path
		 */
		const char* join(const string& ext) { return join(ext.c_str()); }

	private:
		string _path;
		char* _dirname;
		char* _basename;
		string _realpath;
		string _rootname;
		string _joined;
		string _mimetype;
		string _mimeclass;

};

#endif  // _HAVE_FILEINFO_H
