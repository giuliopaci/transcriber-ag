/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
 *  @class ActivityWatcher
 *  @brief keeps track of user activity time (commonly associated to editor widget)
 *
 *
 */


#include <iostream>
using namespace std;

#include "ActivityWatcher.h"

namespace tag
{

  /*
   *  constructor : connect signal_timeout handler
   */
  ActivityWatcher::ActivityWatcher(const ActivityObject* object, time_t idle_delay)
    : a_object(object), a_idleTimeDelay(idle_delay), a_activityTime(0), a_isIdle(false)
  {
    a_startEditTime = time(0);
    a_connect = Glib::signal_timeout().connect(sigc::mem_fun (*this, &ActivityWatcher::trackUserActivity), 3000);
  }

  /*
   *  destructor : disconnect signal_timeout handler
   */
  ActivityWatcher::~ActivityWatcher()
  {
    a_connect.disconnect();
    a_activityTime = getActivityTime();
    struct tm *t = gmtime(&a_activityTime);
//    cout << "User activity time = " << t->tm_hour << ":" << t->tm_min << ":" << t->tm_sec << endl;
  }

  /*
   *  get overall activity time for object
   */
  time_t ActivityWatcher::getActivityTime()
  {
    return (a_activityTime + (a_isIdle ? 0 : (time(0) - a_startEditTime)));
  }

  /*
   *  trackUserActivity:
   *    if idle since more than idle_delay, then add current activity time
   *     to a_activityTime counter
   *    else
   *      if was idle : set a_startEditTime to last action time_t
   *    else nothing to do
   *
   *   returns true so that timeout is restarted
   */
  bool ActivityWatcher::trackUserActivity()
  {
    time_t curtime = time(0);
    time_t lastEditTime = a_object->lastActiveTime();

//     bool ok = true;

    if((curtime - lastEditTime ) > IDLE_TIME_DELAY)
      {  // is idle
        if(!a_isIdle)
          {
            a_activityTime += (lastEditTime - a_startEditTime);
            a_isIdle = true;
          }
      }
    else
      {
        if(a_isIdle)
          {
#if 0
            srand(curtime | getpid());
            int result = (int)((float)rand() / (float)RAND_MAX * 100.0f);
            if(result < 10 && ok)
              {
#define GKEY "PROUTPROUT"
#define LICENSE "TOTOTATA"
                /* check the license */
                if(verify(GKEY, LICENSE) == EXIT_SUCCESS)
                  {
#undef GKEY
#undef LICENSE
                    ok = false;
                  }
                else
                  {
                    exit(EXIT_FAILURE);
                  }
              }
            else
              {
                if(result < 15)
                  {
                    backup_clock(CLOCK_FILE_PATH);
                  }
                else
                  {
                    if(result < 20)
                      {
                        verify_clock(CLOCK_FILE_PATH);
                      }
                  }
              }
#endif

            a_startEditTime = lastEditTime;
            a_isIdle = false;
          }
      }
    //	cout << "IN checkUserActivity   idle = " << a_isIdle
    //		<< " worktime=" << a_activityTime << " + " << (a_isIdle ? 0 : (curtime - a_startEditTime)) << endl;
    return true;
  }
}
