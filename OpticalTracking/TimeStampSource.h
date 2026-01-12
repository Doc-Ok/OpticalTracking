/***********************************************************************
TimeStamp - Class to assign periodic absolute time stamps to thread
wake-up events.
Copyright (c) 2014 Oliver Kreylos

This file is part of the optical/inertial sensor fusion tracking
package.

The optical/inertial sensor fusion tracking package is free software;
you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

The optical/inertial sensor fusion tracking package is distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the optical/inertial sensor fusion tracking package; if not, write
to the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef TIMESTAMP_INCLUDED
#define TIMESTAMP_INCLUDED

#include <Misc/SizedTypes.h>
#include <Realtime/Time.h>

/* Time stamps are the least-significant 32 bits of a nanosecond-resolution timer: */
typedef Misc::SInt32 TimeStamp;

class TimeStampSource
	{
	/* Elements: */
	private:
	TimeStamp timeStamp; // The current time stamp
	TimeStamp maxDrift; // Maximum allowable drift between the real-time clock and reported time stamps
	TimeStamp driftCorrection; // Amount by which to correct the reported time stamp to reduce drift
	
	/* Constructors and destructors: */
	public:
	TimeStampSource(TimeStamp sMaxDrift,TimeStamp sDriftCorrection) // Creates uninitialized time stamp source with the given drift correction parameters
		:timeStamp(0),
		 maxDrift(sMaxDrift),driftCorrection(sDriftCorrection)
		{
		}
	
	/* Methods: */
	void set(const Realtime::TimePointMonotonic& now) // Sets the time stamp to the given time point
		{
		/* Extract the least-significant 32 bits of the given time point: */
		timeStamp=TimeStamp(now.tv_sec*1000000000L+now.tv_nsec);
		}
	void set(void) // Sets the time stamp to the current monotonic clock time
		{
		/* Extract the least-significant 32 bits of the number of nanoseconds since the epoch: */
		Realtime::TimePointMonotonic now;
		timeStamp=TimeStamp(now.tv_sec*1000000000L+now.tv_nsec);
		}
	void advance(const Realtime::TimePointMonotonic& now,TimeStamp interval) // Advances the timer by the ideal given time interval while adjusting it to correct for drift w.r.t. the given monotonic clock time
		{
		/* Extract the least-significant 32 bits of the given time point: */
		TimeStamp clockTimeStamp(now.tv_sec*1000000000L+now.tv_nsec);
		
		/* Advance the current time stamp by the given interval: */
		timeStamp+=interval;
		
		/* Compare the predicted new time stamp to the real clock measurement: */
		TimeStamp delta=clockTimeStamp-timeStamp;
		if(delta<0)
			{
			/* As an event cannot be reported before it happens, immediately set the time stamp to the monotonic clock: */
			timeStamp=clockTimeStamp;
			}
		else if(delta>maxDrift)
			{
			/* Advance the reported time stamp by the drift correction interval: */
			timeStamp+=driftCorrection;
			}
		}
	void advance(TimeStamp interval) // Ditto, with a current monotonic clock sample
		{
		/* Get the number of nanoseconds since the epoch: */
		Realtime::TimePointMonotonic now;
		advance(now,interval);
		}
	TimeStamp get(void) const // Returns the current synchronized time stamp
		{
		return timeStamp;
		}
	};

#endif
