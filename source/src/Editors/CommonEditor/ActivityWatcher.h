/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/* $Id */

#ifndef _HAVE_ACTIVITY_WATCHER_H
#define _HAVE_ACTIVITY_WATCHER_H

#include <glibmm.h>
#include <glibmm/objectbase.h>
#include <time.h>

namespace tag
{

#define IDLE_TIME_DELAY  180   //  if no activity since IDLE_TIME_DELAY -> isIdle


/**
 *  ActivityObject : base object acting as interface for
 * any object for which user activity time is to be tracked
 */
class ActivityObject
{
	public:
		ActivityObject() : a_lastActiveTime(time(0)) {}
		virtual ~ActivityObject() {}

		/*!  returns the time of last user action  (in seconds) */
		virtual time_t lastActiveTime() const { return a_lastActiveTime; }

	protected:
		time_t a_lastActiveTime;
};


class ActivityWatcher : public Glib::ObjectBase
{
	public:

		/*! Default constructor */
		ActivityWatcher(const ActivityObject* object, time_t idle_delay=IDLE_TIME_DELAY);
		~ActivityWatcher();

		/*! get overall user activity time for associated object */
		time_t getActivityTime();

		/*! set overall user activity time for associated object */
		void setActivityTime(time_t t) { a_activityTime = t; }

		/*! set time delay after which application is considered as idle if no user action */
		void setIdleTimeDelay(int delay) { a_idleTimeDelay = delay; }

		/*! get time delay after which application is considered as idle if no user action */
		int getIdleTimeDelay() { return a_idleTimeDelay; }

	private:
		bool trackUserActivity();


	private:
		const ActivityObject* a_object; /** associated active object */
		time_t a_idleTimeDelay; /** min delay after which considered as idle */
		time_t	a_activityTime ;  /** cumulated user activity time */
		time_t a_startEditTime;	/** start idle time */
		bool a_isIdle;   /** idle state */
		sigc::connection a_connect;  /** timeout signal connection id */
};

}
#endif /* _HAVE_ACTIVITY_WATCHER_H */
