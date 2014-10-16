/***********************************************************************
Timer - Class to provide high-resolution timers using POSIX clocks.
Copyright (c) 2014 Oliver Kreylos

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

#ifndef REALTIME_TIMER_INCLUDED
#define REALTIME_TIMER_INCLUDED

#include <time.h>

namespace Realtime {

class Timer
	{
	/* Elements: */
	private:
	struct timespec lastMeasured; // Last time point at which the timer was started or reset
	struct timespec elapsed; // Time vector between the last two measurement points
	
	/* Constructors and destructors: */
	public:
	Timer(void) // Creates and starts a timer for the monotonic system clock
		{
		/* Initialize the first timing interval: */
		elapsed.tv_sec=0;
		elapsed.tv_nsec=0;
		
		/* Get the current monotonic clock time: */
		clock_gettime(CLOCK_MONOTONIC,&lastMeasured);
		}
	
	/* Methods: */
	void elapse(void) // Measures time since last measurement, and resets timer
		{
		/* Get the current monotonic clock time: */
		struct timespec measured;
		clock_gettime(CLOCK_MONOTONIC,&measured);
		
		/* Calculate the elapsed time interval: */
		elapsed.tv_sec=measured.tv_sec-lastMeasured.tv_sec;
		elapsed.tv_nsec=measured.tv_nsec-lastMeasured.tv_nsec;
		if(elapsed.tv_nsec<0L)
			{
			--elapsed.tv_sec;
			elapsed.tv_nsec+=1000000000L;
			}
		
		lastMeasured=measured;
		}
	double getElapsed(void) const // Returns the last time interval in seconds
		{
		return double(elapsed.tv_sec)+double(elapsed.tv_nsec)*1.0e-9;
		}
	double getCurrent(void) const // Returns the current time interval since the last measurement without resetting the timer
		{
		/* Get the current monotonic clock time: */
		struct timespec measured;
		clock_gettime(CLOCK_MONOTONIC,&measured);
		
		/* Calculate the elapsed time interval (conversion to double takes care of nanosecond underflow): */
		return double(measured.tv_sec-lastMeasured.tv_sec)+double(measured.tv_nsec-lastMeasured.tv_nsec)*1.0e-9;
		}
	void sleep(time_t seconds,long nanoseconds) // Resets timer and sleeps for the given number of seconds and nanoseconds
		{
		/* Get the current monotonic clock time: */
		clock_gettime(CLOCK_MONOTONIC,&lastMeasured);
		
		/* Calculate the wakey-upy time: */
		struct timespec wakeup;
		wakeup.tv_sec=lastMeasured.tv_sec+seconds;
		wakeup.tv_nsec=lastMeasured.tv_nsec+nanoseconds;
		if(wakeup.tv_nsec>=1000000000L)
			{
			++wakeup.tv_sec;
			wakeup.tv_nsec-=1000000000L;
			}
		
		/* Go to sleep: */
		while(clock_nanosleep(CLOCK_MONOTONIC,TIMER_ABSTIME,&wakeup,0))
			;
		}
	};

}

#endif
