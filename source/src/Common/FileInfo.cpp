/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
//
//  classe FileInfo : utilitaires de manipulation de fichiers
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifndef WIN32
#include <sys/wait.h>
#endif
#include <sys/param.h>
#include <sys/file.h>
#include <utime.h>

#include "Common/FileInfo.h"
#include "util/Log.h"

#ifdef WIN32
  #define _path_delim "\\"
#else
  #define _path_delim "/"
#endif

bool FileInfo::exists(const char *type)
{
  struct	stat	buf;

  if ( stat(_path.c_str(), &buf) != 0 ) return false;
  switch ( *type ) {
  case 'd':	return ((buf.st_mode & S_IFDIR) != 0);
  case 'f':	return ((buf.st_mode & S_IFREG) != 0);
  default:      return false;
  }
}

bool FileInfo::exists()
{
  if ( (access (_path.c_str(), R_OK) == 0) ) return true;
  else return false;
}

bool FileInfo::canWrite()
{
	if ( exists("d") ) { // test if writable directory
		char buf[512];
		sprintf(buf, "%s%stmp.%x", _path.c_str(), _path_delim , (unsigned int)time(0));
		FILE* fic = fopen(buf, "w");
		if ( fic != NULL ) {
			fclose(fic);
			unlink(buf);
			return true;
		}
	} else { // test file can be opened in write mode
		FILE* fic = fopen(_path.c_str(), "rb+");
		if ( fic != NULL ) { fclose(fic); return true; }
	}
  return false;
}

int FileInfo::size()
{
  struct stat buf ;

  if ( (stat (_path.c_str(), &buf) < 0) ) 	return -1;
  return buf.st_size ;
}

time_t FileInfo::mtime()
{
  struct stat buf ;

  if ( (stat (_path.c_str(), &buf) < 0) ) 	return (time_t)0;
  return buf.st_mtime ;
}


const char* FileInfo::Basename()
{
  char* pt  = strdup(_path.c_str());
  if ( _basename != NULL ) { free(_basename); _basename = NULL; }
  _basename= strdup(::basename(pt));
  free(pt);
  return _basename;
}

const char* FileInfo::dirname(int uplevels)
{
  char* pt;
  if ( _dirname != NULL ) { free(_dirname); _dirname = NULL; }
  pt = ( exists() ? strdup(realpath()) : strdup(_path.c_str()));
  while ( uplevels > 0 ) {
    _dirname= strdup(::dirname(pt));
    free(pt);
    pt = _dirname;
    uplevels--;
  }
  return _dirname;
}



const char* FileInfo::realpath()
{

	char* pt = strdup( _path.c_str());
    string dir = ::dirname(pt);
	free(pt);

	_realpath = _path;

	if ( dir[0] == '~' )
	{
//		const char* pt = getenv("HOME");
		const gchar* pt = g_get_home_dir() ;
		if ( pt != NULL )
			dir = pt + dir.substr(1);
	}
	else if ( dir[0] == '$' )
	{
		char varname[80];
		const char* pt;
		int i;
		for ( i=0, pt = dir.c_str()+1;
			*pt && *pt != ')' && strncmp(pt,_path_delim,1) ; ++pt )
			if ( *pt == '(' ) continue;
			else varname[i++] = *pt;
		varname[i] = 0;
		if ( *pt == ')' ) ++pt;

		if ( varname[0] ) {
			const char* pt2 = getenv(varname);
			if ( pt2 != NULL ) {
				dir = string(pt2) + string(pt);
			} else
				Log::err() << "Error in FileInfo::realpath : undefined environment variable " << varname << endl;
		}
	}
	else if ( dir[0] == '.' )
	{
  		char resolved[MAXPATHLEN];
		char current[MAXPATHLEN];
		if ( getcwd(current, sizeof(current)) == NULL )
			return "";  // rep courant indefini
		if ( chdir(dir.c_str()) != 0 )
			return ""; // path invalide
		getcwd(resolved, sizeof(resolved));
		chdir (current);
		dir=resolved;
	}

	_realpath = FileInfo(dir).join(Basename());

	return _realpath.c_str();
}

const char* FileInfo::join(const char* ext)
{
	if (_path!="/")
		_joined = _path + string(_path_delim) + ext;
	else
		_joined = _path + ext;
	return _joined.c_str();
}

const char* FileInfo::mimetype()
{
  _mimetype = "data";
	unsigned long pos = _path.find_last_of(".");
	if ( pos != string::npos ) {
		_mimetype = _path.substr(pos+1);
		for ( unsigned int i=0; i < _mimetype.length(); ++i )
			_mimetype[i] = tolower(_mimetype[i]);
	}

  return _mimetype.c_str();
}

const char* FileInfo::mimeclass()
{

  mimetype();

	if ( _mimetype == "wav" )  _mimeclass = "audio";
	if ( _mimetype == "mp3" )  _mimeclass = "audio";
	if ( _mimetype == "trs" )  _mimeclass = "transcript";
	if ( _mimetype == "tag" )  _mimeclass = "transcript";

  return _mimeclass.c_str();
}

const char* FileInfo::tail()
{
	const char* name = Basename();
	const char* pt =  strrchr(name, '.');

	return (pt == NULL ? "" : pt);
}

void FileInfo::setTail(string ext)
{
	string prev=tail();
	if ( prev.length() > 0 )
		_path = _path.substr(0, _path.length()-prev.length());
	if ( ext[0] != '.' ) _path += ".";
	_path += ext ;
}


/**
 * return file root name (ie without suffix)
 */
const char* FileInfo::rootname()
{
	_rootname = Basename();
	unsigned long pos = _rootname.rfind(".");
	if ( pos != string::npos )
		_rootname.erase(pos,  string::npos);

	return _rootname.c_str();
}



#ifdef TEST_MODULE
#include <iostream>

int main(int argc, const char* argv[])
{
  FileInfo info(argv[1]);

  cout << "1 -- " << endl;
  cout << " Basename = " << info.Basename() << endl;
  cout << " dirname = " << info.dirname() << endl;
  cout << " realpath = " << info.realpath() << endl;
  cout << endl << "2 -- " << endl;
  cout << " Basename = " << FileInfo(argv[1]).Basename() << endl;
  cout << " dirname = " << FileInfo(argv[1]).dirname() << endl;
  cout << " realpath = " << FileInfo(argv[1]).realpath() << endl;
}
#endif
