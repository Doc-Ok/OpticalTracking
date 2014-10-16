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

#include <Realtime/AlarmTimer.h>

#include <Realtime/Config.h>

#include <string.h>
#include <sys/time.h>
#include <Misc/Time.h>

namespace Realtime {

/***********************************
Static elements of class AlarmTimer:
***********************************/

#if REALTIME_CONFIG_HAVE_POSIX_TIMERS
unsigned int AlarmTimer::numAlarmTimers=0;
#endif

/***************************
Methods of class AlarmTimer:
***************************/

#if REALTIME_CONFIG_HAVE_POSIX_TIMERS
void AlarmTimer::signalHandler(int,siginfo_t* sigInfo,void*)
	{
	/* Get pointer to the alarm timer object: */
	AlarmTimer* at=static_cast<AlarmTimer*>(sigInfo->si_value.sival_ptr);
	
	/* Mark the timer as expired: */
	at->expired=true;
	at->armed=false;
	}
#endif

#if REALTIME_CONFIG_HAVE_POSIX_TIMERS
AlarmTimer::AlarmTimer(void)
	:timerId(0),armed(false),expired(false)
	{
	/* Initialize the alarm timer class if this is the first one: */
	if(numAlarmTimers==0)
		{
		/* Install the signal handler: */
		struct sigaction sigAction;
		memset(&sigAction,0,sizeof(sigAction));
		sigAction.sa_sigaction=signalHandler;
		sigemptyset(&sigAction.sa_mask);
		sigAction.sa_flags=SA_SIGINFO;
		sigaction(SIGRTMIN,&sigAction,NULL);
		}
	++numAlarmTimers;
	
	/* Create a per-process timer: */
	struct sigevent timerEvent;
	memset(&timerEvent,0,sizeof(timerEvent));
	timerEvent.sigev_notify=SIGEV_SIGNAL;
	timerEvent.sigev_signo=SIGRTMIN;
	timerEvent.sigev_value.sival_ptr=this;
	timer_create(CLOCK_REALTIME,&timerEvent,&timerId);
	}
#else
AlarmTimer::AlarmTimer(void)
	:armed(false),expireTime(0,0)
	{
	}
#endif

AlarmTimer::~AlarmTimer(void)
	{
	#if REALTIME_CONFIG_HAVE_POSIX_TIMERS
	/* Destroy the per-process timer: */
	timer_delete(timerId);
	
	/* Deinitialize the alarm timer class if this is the last one: */
	--numAlarmTimers;
	if(numAlarmTimers==0)
		{
		/* Remove the signal handler: */
		struct sigaction sigAction;
		sigAction.sa_handler=SIG_DFL;
		sigemptyset(&sigAction.sa_mask);
		sigAction.sa_flags=0;
		sigaction(SIGRTMIN,&sigAction,NULL);
		}
	#endif
	}

bool AlarmTimer::armTimer(const Misc::Time& expirationTime)
	{
	#if REALTIME_CONFIG_HAVE_POSIX_TIMERS
	/* Arm the timer signal: */
	struct itimerspec timerInterval;
	timerInterval.it_interval.tv_sec=0;
	timerInterval.it_interval.tv_nsec=0;
	timerInterval.it_value=expirationTime;
	if(timer_settime(timerId,0,&timerInterval,0)==0)
		{
		expired=false;
		armed=true;
		return true;
		}
	else
		return false;
	#else
	expireTime=Misc::Time::now()+expirationTime;
	return true;
	#endif
	}

}
