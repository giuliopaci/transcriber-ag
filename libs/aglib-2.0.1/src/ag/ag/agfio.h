// agfio.h: AG file I/O interface
// Haejoong Lee, Xiaoyi Ma, Steven Bird
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _AGFIO_H_
#define _AGFIO_H_

#include <ag/AGTypes.h>
#include <ag/agfioError.h>
#include <list>
#include <map>

using namespace std;

class agfio_plugin;

/**
 * @brief %AG file I/O interface.
 *
 * Agfio defines an interface to file I/O modules for application programs.
 * Two file I/O operations (<tt>load</tt> and <tt>store</tt>) and exceptions
 * during those operations are defined in agfio. Internally, agfio also
 * performs file I/O plugin management.
 */
class agfio
{
private:
  typedef agfio_plugin* create_func_t();
  typedef void destroy_func_t(agfio_plugin*);
  // module name => module itself
  typedef map<string,void*> plugin_map_t;
  // module name => agfio_plugin instance destroy function
  typedef map<string,destroy_func_t*> destroy_func_map_t;
  // module name => agfio_plugin instance
  typedef map<string,agfio_plugin*> loader_map_t;
  
  static plugin_map_t plugin_map;
  static destroy_func_map_t destroy_func_map;
  static loader_map_t loader_map;

  agfio_plugin* plug(const string& format) throw (const string&);
  void unplug(const string& format);

public:
  /// Error class for any error during load
  class DllExport LoadError : public agfioError
  {
  public:
    /**
     * @param s
     *   Error message.
     */
    LoadError(const string& s);
  };

  /// Error class for any error during store
  class DllExport StoreError : public agfioError
  {
  public:
    /**
     * @param s
     *   Error message.
     */
    StoreError(const string& s);
  };

  agfio() {}
  ~agfio();

  /**
   * Load a file, converting it to AG's. To be more specific, <tt>load()</tt>
   * locates the plugin for the given <i>format</i> and calls the
   * <tt>%load()</tt> function of the plugin with the given parameters.
   *
   * The way how the function parameters are used varies over file I/O
   * modules(plugins).  For example, some modules don't use <tt>id</tt>,
   * some modules don't use <tt>signalInfo</tt>.  Also available options,
   * which are specified using <tt>options</tt> parameter, different for
   * different file I/O formats.  See <a href="../formats.html>File I/O
   * plugins</a> for details of each file I/O module(plugin).
   *
   * @param format
   *   Name of the file format.
   * @param filename
   *   A path (absolute or relative) to the file.
   * @param id
   *   AGSetId or AGId into which the file will be loaded.
   * @param signalInfo
   *   A feature-value pair list for the signal information. Features that
   *   should be used are:
   *   <ul>
   *   <li><tt>uri</tt></li>
   *   <li><tt>mimeClass</tt></li>
   *   <li><tt>mimeType</tt></li>
   *   <li><tt>encoding</tt></li>
   *   <li><tt>unit</tt></li>
   *   <li><tt>track</tt> (optional)</li>
   *   </ul>
   *   The values shouldn't be empty.
   * @param options
   *   A option-value pair list that is used to change the behavior of
   *   <tt>%load()</tt> of the I/O module(plugin).
   * @return
   *   List of ids of created AG's
   */
  list<AGId>
  load(const string& format,
       const string& filename,
       const Id& id = "",
       map<string,string>* signalInfo = NULL,
       map<string,string>* options = NULL)
    throw (LoadError);

  /**
   * Store AG's to a file in the specified format. Note that it is usually
   * impossible to store the loaded AG's using a different format (except
   * for <tt>%AG</tt> and <tt>CAG</tt> formats) than the one used for loading.
   *
   * @param format
   *   Target format in which the given annotaiton graphs are stored.
   * @param filename
   *   The name of the file to be written.
   * @param id
   *   The id of AG or AGSet to be stored. Some formats require AGId but
   *   some formats require AGSetId. Some formats can accept both.  See
   *   <a href="../formats.html">File I/O plugins</a> for details.
   * @param options
   *   A option-value pair list that is used to change the behavior of
   *   <tt>%store()</tt> of the I/O module(plugin).  See <a href=
   *   "../formats.html">File I/O plugins</a> for available options for
   *   different formats.
   * @return
   *   An optional string. Usually you can ignore this, but for some formats
   *   it might be meaningfull.
   */
  string
  store(const string& format,
	const string& filename,
	const Id& id,
	map<string,string>* options = NULL)
    throw (StoreError);

  /**
   * Same as <tt>%store()</tt> below except that this one accepts
   * a list of AGId's instead of just one AGId/AGSetId.
   */
  string
  store(const string& format,
	const string& filename,
	list<string>* const ids,
	map<string,string>* options = NULL)
    throw (StoreError);
  
};

#endif
