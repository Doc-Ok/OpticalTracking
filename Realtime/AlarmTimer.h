/***********************************************************************
AlarmTimer - Class to implement one-off alarm timers using the real-time
signal mechanism.
Copyright (c) 2005-2012 Oliver Kreylos
Mac OS X adaptation copyright (c) 2006 Braden Pellett

This file is part of the Realtime Processing Library (Realtime).

The Realtime Processing Library is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Realtime Processing Library is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Realtime Processing Library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef REALTIME_ALARMTIMER_INCLUDED
#define REALTIME_ALARMTIMER_INCLUDED

#include <Realtime/Config.h>

#include <time.h>
#if REALTIME_CONFIG_HAVE_POSIX_TIMERS
#include <signal.h>
#else
#include <Misc/Time.h>
#endif

/* Forward declarations: */
#if REALTIME_CONFIG_HAVE_POSIX_TIMERS
namespace Misc {
class Time;
}
#endif

namespace Realtime {

class AlarmTimer
	{
	/* Elements: */
	private:
	#if REALTIME_CONFIG_HAVE_POSIX_TIMERS
	static unsigned int numAlarmTimers; // Counts number of currently existing timer objects, to handle initialization
	timer_t timerId; // ID of per-process timer used by this object
	#endif
	volatile bool armed; // Flag if the timer is currently armed
	#if REALTIME_CONFIG_HAVE_POSIX_TIMERS
	volatile bool expired; // Flag if an armed timer has expired
	#else
	Misc::Time expireTime; // Time at which the timer expires
	#endif
	
	/* Private methods: */
	#if REALTIME_CONFIG_HAVE_POSIX_TIMERS
	static void signalHandler(int signal,siginfo_t* sigInfo,void* context); // Handler function for the timer signal
	#endif
	
	/* Constructors and destructors: */
	public:
	AlarmTimer(void); // Creates an unarmed timer
	~AlarmTimer(void); // Destroys a timer
	
	/* Methods: */
	bool isArmed(void) const // Returns true if the timer is currently armed
		{
		return armed;
		}
	bool isExpired(void) const // Returns true if an armed timer has expired
		{
		#if REALTIME_CONFIG_HAVE_POSIX_TIMERS
		return expired;
		#else
		return Misc::Time::now()>=expireTime;
		#endif
		}
	bool armTimer(const Misc::Time& expirationTime); // Arms the timer with the given timeout time vector; returns false if timer could not be armed
	};

}

#endif
