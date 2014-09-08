#include <ag/agfio.h>
#include <ag/agfio_plugin.h>
#include <ag/AGAPI.h>
#include <ag/AGException.h>
#include "ag_dlfcn.h"
#include "TranscriberAG-config.h"

/* (( BT Patch -- */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
  #define DIR_SEPARATOR "\\"
  #include <windows.h> // for GetModuleFileName
  #include <shlwapi.h>
#else
  #define DIR_SEPARATOR "/"
#endif
/* -- BT Patch )) */ 

agfio::plugin_map_t agfio::plugin_map;
agfio::destroy_func_map_t agfio::destroy_func_map;
agfio::loader_map_t agfio::loader_map;

agfio::LoadError::LoadError(const string& s)
: agfioError(s)
{}

agfio::StoreError::StoreError(const string& s)
: agfioError(s)
{}

agfio::~agfio()
{
/* (( BT Patch -- */
// Happening segfault
/*
	plugin_map_t::iterator pos;
	for (pos=plugin_map.begin(); pos != plugin_map.end(); ++pos)
		unplug(pos->first);
*/
/* -- BT Patch )) */
}

list<AGId>
agfio::load(const string& format,
	    const string& filename,
	    const Id& id,
	    map<string,string>* signalInfo,
	    map<string,string>* options)
  throw (LoadError)
{
  try {
    return plug(format)->load(filename, id, signalInfo, options);
  }
  catch (const string& msg) {
    throw LoadError(msg);
  }
  catch (const LoadError& e) {
    throw e;
  }
  catch (AGException& e) {
    throw LoadError(string("AGException occurred, ") + e.error());
  }
  catch (const exception& e) {
    throw LoadError(e.what());
  }
  catch (...) {
    throw LoadError("Unknown exception");
  }
}

string
agfio::store(const string& format,
	     const string& filename,
	     const Id& id,
	     map<string,string>* options)
  throw (StoreError)
{
  try {
    return plug(format)->store(filename, id, options);
  }
  catch (const string& msg) {
    throw StoreError(msg);
  }
  catch (const StoreError& e) {
    throw e;
  }
  catch (AGException& e) {
    throw StoreError(string("AGException occurred, ") + e.error());
  }
  catch (const exception& e) {
    throw StoreError(e.what());
  }
  catch (...) {
    throw StoreError("Unknown exception");
  }
}

string
agfio::store(const string& format,
	     const string& filename,
	     list<string>* const ids,
	     map<string,string>* options)
  throw (StoreError)
{
  try {
    return plug(format)->store(filename, ids, options);
  }
  catch (const string& msg) {
    throw StoreError(msg);
  }
  catch (const StoreError& e) {
    throw e;
  }
  catch (AGException& e) {
    throw StoreError(string("AGException occurred, ") + e.error());
  }
  catch (const exception& e) {
    throw StoreError(e.what());
  }
  catch (...) {
    throw StoreError("Unknown exception");
  }
}

agfio_plugin*
agfio::plug(const string& format)
  throw (const string&)
{
  loader_map_t::iterator pos = loader_map.find(format);
  if (pos != loader_map.end()) {
    return pos->second;
  }

/* (( BT Patch -- */
#ifdef WIN32
/* -- BT Patch )) */
	string plugin_name = "agfio_plugin_" + format + ".dll";
/* (( BT Patch -- */
	#define LD_LIBRARY_PATH "PATH"
	#define PATH_DELIM ";"
/* -- BT Patch )) */	
#else
/* (( BT Patch -- */
#ifdef __APPLE__
	string plugin_name = "agfio_plugin_" + format + ".dylib";
	#define LD_LIBRARY_PATH "LD_LIBRARY_PATH"
	#define PATH_DELIM ":"
/* -- BT Patch )) */
#else
	string plugin_name = "agfio_plugin_" + format + ".so";
/* (( BT Patch -- */
	#define LD_LIBRARY_PATH "LD_LIBRARY_PATH"
	#define PATH_DELIM ":"
#endif	// __APPLE__
#endif	// WIN32
/* -- BT Patch )) */

/* (( BT Patch -- */
    //void* plugin = dlopen(plugin_name.c_str(), RTLD_LAZY);
    void* plugin = NULL;
  	//if (getenv(LD_LIBRARY_PATH) != NULL )
  	if (false)
  	{
	 	// PATCH by BT-PLr 2008-06-04
	  	// HINT if  LD_LIBRARY_PATH set by calling program -> then it may be ignored by dlopen, so we scan
	  	// all libs set in LD_LIBRARY_PATH to find plugin
	  	string lib_path=getenv(LD_LIBRARY_PATH);
	  	while ( ! lib_path.empty() ) 
	  	{
		 	int pos = lib_path.find(PATH_DELIM);
		  	string cur ;
		  	if ( pos == string::npos ) 
		  	{
			  	cur = lib_path;
			  	lib_path.clear();
		  	}
		  	else 
		  	{
			 	cur = lib_path.substr(0, pos);
			  	lib_path=lib_path.substr(pos+1);
		  }
		  cur += DIR_SEPARATOR;
		  cur += plugin_name;
		  if ( access(cur.c_str(), 0) == 0 ) 
		  {
			  	cerr << " LOAD PLUGIN : " << cur.c_str() << endl;
			 	plugin = dlopen(cur.c_str(), RTLD_LAZY);
				break;
		  }
	  }
	  if ( plugin == NULL)
	  {
	      string err = string("Cannot load plugin: ") + plugin_name.c_str() + "\n" ;
	      throw err ;
	  }
  } 
  else 
  {
      #if defined(_WIN32) && !defined(__CYGWIN__)
      char cCurrentPath[FILENAME_MAX];
      GetModuleFileName(NULL, cCurrentPath, sizeof(cCurrentPath));
      PathRemoveFileSpec(cCurrentPath);
      string plugin_dir(cCurrentPath);
      plugin_name = plugin_dir + DIR_SEPARATOR + plugin_name;
      #else
	  plugin_name = AG_PLUGINDIR + plugin_name;
	  #endif
      cerr << " LOAD PLUGIN : " << plugin_name.c_str() << endl;
	  plugin = dlopen(plugin_name.c_str(), RTLD_LAZY);
	  if ( plugin == NULL)
	  {
	      throw string("Cannot load plugin: ") + dlerror() + '\n';
	  }
  }
/* -- BT Patch )) */ 

  // load the symbols
  create_func_t* create_func = (create_func_t*) dlsym(plugin, "create");
  destroy_func_t* destroy_func = (destroy_func_t*) dlsym(plugin, "destroy");
  if (!create_func || !destroy_func) {
    throw string("Cannot load symbols: ") + dlerror() + '\n';
  }

  // create an instance of the class
  agfio_plugin* loader = create_func();

  plugin_map[format] = plugin;
  destroy_func_map[format] = destroy_func;
  loader_map[format] = loader;

  return loader;
}

void
agfio::unplug(const string& format)
{
	/* (( BT Patch -- */
	#ifdef __APPLE__
	return;
	#endif
	  
	// -- Some Traces --
	printf("AGfio::unplug --> Format = %s\n", format.c_str());
	/* -- BT Patch )) */

	destroy_func_map_t::iterator pos = destroy_func_map.find(format);

	if (pos != destroy_func_map.end()){
		(pos->second)(loader_map[format]);
		/* (( BT Patch -- */
		//dlclose(plugin_map[format]);	// Avoids crashes...
		/* -- BT Patch )) */
		
		destroy_func_map.erase(pos);
		loader_map.erase(format);
		plugin_map.erase(format);
	}
	
	/* (( BT Patch -- */
	printf("AGfio::unplug --> AGLib successfully unloaded\n");
	/* -- BT Patch )) */
}

