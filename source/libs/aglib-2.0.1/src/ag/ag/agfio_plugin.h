// agfio_plugin.h: AG file I/O plugin interface
// Haejoong Lee
// Copyright (C) 2001-2003 Linguistic Data Consortium, U Penn.
// Web: http://www.ldc.upenn.edu/; Email: ldc@ldc.upenn.edu
// For license information, see the file `LICENSE' included
// with the distribution.


#ifndef _AGFIO_PLUGIN_H_
#define _AGFIO_PLUGIN_H_

#include <ag/agfio.h>
#include <ag/AGTypes.h>
#include <list>
#include <map>

using namespace std;

#define AGFIO_PLUGIN(t) \
    extern "C" {\
      DllExport agfio_plugin* create() {return new t;}\
      DllExport void destroy(agfio_plugin* m) {delete (t*) m;}\
    }

/**
 * @brief %AG file I/O plugin interface.
 *
 * agfio_plugin is an interface to %AG file I/O plugins. Plugins should
 * inherit this class and implement <tt>%load()</tt> and <tt>%store()</tt>
 * functions.
 */
class agfio_plugin
{
protected:
  /**
   * This function is used to get appropriate AGSetId, AGId, TimelineId and
   * SignalId for loading, using an algorithm described in the following
   * table:
   *
   * <table>
   * <tr>
   * <td>%AGSet<sub><tt>id</tt></sub><br>exists</td>
   * <td>%AG<sub><tt>id</tt></sub><br>exists</td>
   * <td>signalInfo<br>is NULL</td>
   * <td>Result</td>
   * </tr>
   *
   * <tr>
   * <td>Y</td><td>Y</td><td>Y</td>
   * <td>
   * <li><tt>agsetId</tt>=id of %AGSet<sub><tt>id</tt></sub> containing
   * %AG<sub><tt>id</tt></sub>
   * <li><tt>timelineId</tt>=id of %AG<sub><tt>id</tt></sub>'s %Timeline
   * <li><tt>signalId</tt>=empty string
   * <li><tt>agId</tt>=<tt>id</tt>
   * </td>
   * </tr>
   *
   * <tr>
   * <td>Y</td><td>Y</td><td>N</td>
   * <td>
   * <li><tt>agsetId</tt>=id of %AGSet<sub><tt>id</tt></sub> containing
   * %AG<sub><tt>id</tt></sub>
   * <li><tt>timelineId</tt>=id of %AG<sub><tt>id</tt></sub>'s %Timeline
   * <li><tt>signalId</tt>=id of matched (with <tt>signalInfo</tt>) %Signal or
   * a new %Signal if no %Signal is matched
   * <li><tt>agId</tt>=<tt>id</tt>
   * </td>
   * </tr>
   *
   * <tr>
   * <td>Y</td><td>N</td><td>Y</td>
   * <td>
   * <li><tt>agsetId</tt>=<tt>id</tt>
   * <li><tt>timelineId</tt>=id of a new %Timeline
   * <li><tt>signalId</tt>=empty string
   * <li><tt>agId</tt>=id of a new %AG
   * </td>
   * </tr>
   *
   * <tr>
   * <td>Y</td><td>N</td><td>N</td>
   * <td>
   * <li><tt>agsetId</tt>=<tt>id</tt>
   * <li><tt>timelineId</tt>=id of a new %Timeline
   * <li><tt>signalId</tt>=id of a new %Signal
   * <li><tt>agId</tt>=id of a new %AG
   * </td>
   * </tr>
   *
   * <tr>
   * <td>N</td><td>N</td><td>Y</td>
   * <td>
   * <li><tt>agsetId</tt>=id of a new %AGSet
   * <li><tt>timelineId</tt>=id of a new %Timeline
   * <li><tt>signalId</tt>=empty string
   * <li><tt>agId</tt>=id of a new %AG
   * </td>
   * </tr>
   *
   * <tr>
   * <td>N</td><td>N</td><td>N</td>
   * <td>
   * <li><tt>agsetId</tt>=id of a new %AGSet
   * <li><tt>timelineId</tt>=id of a new %Timeline
   * <li><tt>signalId</tt>=id of a new %Signal
   * <li><tt>agId</tt>=id of a new %AG
   * </td>
   * </tr>
   * </table>
   *
   * Note that %AGSet<sub><tt>id</tt></sub> is an AGSet which is identified by
   * <tt>id</tt>. Similarly, %AG<sub><tt>id</tt></sub> is an AG which is
   * identified by <tt>id</tt>.
   * 
   * @param id
   *   <tt>id</tt> that is given to <tt>%load()</tt>.
   * @param signalInfo
   *   <tt>signalInfo</tt> that is given to <tt>%load()</tt>.
   * @param agsetId
   *   Output parameter for an AGSetId
   * @param timelineId
   *   Output parameter for a TimelineId
   * @param signalId
   *   Output parameter for a SignalId
   * @param agsetId
   *   Output parameter for an AGId
   */
  DllExport void
  auto_init(const Id& id,
	    map<string,string>* signalInfo,
	    AGSetId& agsetId,
	    TimelineId& timelineId,
	    SignalId& signalId,
	    AGId& agId)
    throw (const string&);

public:
  /**
   * Load a file, converting it to AG's.
   *
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
  virtual list<AGId>
  load(const string& filename,
       const Id& id = "",
       map<string,string>* signalInfo = NULL,
       map<string,string>* options = NULL)
    throw (agfio::LoadError)
  { throw agfio::LoadError("agfio_plugin::load:Not supported by this format"); }

  /**
   * Store AG's to a file.
   *
   * @param filename
   *   The name of the file to be written.
   * @param id
   *   The id of AG or AGSet to be stored.
   * @param options
   *   A option-value pair list that is used to change the behavior of
   *   <tt>%store()</tt> of the I/O module(plugin).
   * @return
   *   An optional string.
   */
  virtual string
  store(const string& filename,
	const Id& id,
	map<string,string>* options = NULL)
    throw (agfio::StoreError)
  { list<AGId> l; l.push_back(id); return store(filename,&l,options); }

  /**
   * Same as <tt>%store()</tt> below except that this one accepts
   * a list of AGId's instead of just one AGId/AGSetId.
   */
  virtual string
  store(const string& filename,
	list<string>* const ids,
	map<string,string>* options = NULL)
    throw (agfio::StoreError)
  { throw agfio::StoreError("agfio_plugin::store:Not supported by this format"); }

  
};

#endif
