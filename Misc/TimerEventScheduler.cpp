/***********************************************************************
TimerEventScheduler - Base class for schedulers that allow clients to
register timer event callbacks.
Copyright (c) 2008 Oliver Kreylos

This file is part of the Miscellaneous Support Library (Misc).

The Miscellaneous Support Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Miscellaneous Support Library is distributed in the hope that it
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Miscellaneous Support Library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#include <typeinfo>
#include <Misc/Time.h>

#include <Misc/TimerEventScheduler.h>

namespace Misc {

/******************************************************
Methods of class TimerEventScheduler::FunctionCallback:
******************************************************/

bool TimerEventScheduler::FunctionCallback::operator==(const TimerEventScheduler::Callback& other) const
	{
	if(typeid(other)!=typeid(FunctionCallback))
		return false;
	const FunctionCallback* other2=static_cast<const FunctionCallback*>(&other);
	return callbackFunction==other2->callbackFunction&&userData==other2->userData;
	}

void TimerEventScheduler::FunctionCallback::call(CallbackData* callbackData) const
	{
	/* Call the callback function: */
	callbackFunction(callbackData,userData);
	}

/************************************
Methods of class TimerEventScheduler:
************************************/

TimerEventScheduler::TimerEventScheduler(void)
	:timers(10)
	{
	/* Initialize the timer to the current time-of-day: */
	Misc::Time time=Misc::Time::now();
	currentTime=double(time.tv_sec)+double(time.tv_nsec)/1000000000.0;
	}

TimerEventScheduler::~TimerEventScheduler(void)
	{
	/* Delete the callback structures of all pending timer events: */
	for(TimerQueue::Iterator tIt=timers.begin();tIt!=timers.end();++tIt)
		delete tIt->callback;
	}

void TimerEventScheduler::scheduleEvent(double eventTime,TimerEventScheduler::Callback* callback)
	{
	/* Insert the new timer event into the queue: */
	timers.insert(Timer(eventTime,callback));
	}

void TimerEventScheduler::removeEvent(double eventTime,const TimerEventScheduler::Callback& callback)
	{
	/* Find a matching event in the timer queue: */
	for(TimerQueue::Iterator tIt=timers.begin();tIt!=timers.end();++tIt)
		if(tIt->time==eventTime&&*tIt->callback==callback)
			{
			/* Remove the timer event: */
			delete tIt->callback;
			timers.remove(tIt);
			
			/* Bail out: */
			break;
			}
	}

void TimerEventScheduler::removeAllEvents(const TimerEventScheduler::Callback& callback)
	{
	/* Keep scanning through the heap until no more matching timer events are found: */
	bool removedOne;
	do
		{
		/* Assume there won't be a match on this pass through: */
		removedOne=false;
		
		/* Scan for a matching timer event: */
		for(TimerQueue::Iterator tIt=timers.begin();tIt!=timers.end();++tIt)
			if(*tIt->callback==callback)
				{
				/* Remove the timer event: */
				delete tIt->callback;
				timers.remove(tIt);
				
				/* Bail out, but do another pass: */
				removedOne=true;
				break;
				}
		}
	while(removedOne);
	}

void TimerEventScheduler::triggerEvents(void)
	{
	/* Update the current time: */
	Misc::Time time=Misc::Time::now();
	currentTime=double(time.tv_sec)+double(time.tv_nsec)/1000000000.0;
	
	if(timers.isEmpty())
		return;
	
	/* Create the callback data structure: */
	CallbackData cbData(currentTime);
	
	/* Process expired timer events from the head of the queue: */
	while(timers.getSmallest().time<=currentTime)
		{
		/* Call the callback: */
		timers.getSmallest().callback->call(&cbData);
		
		/* Remove the callback from the queue: */
		delete timers.getSmallest().callback;
		timers.removeSmallest();
		
		if(timers.isEmpty())
			break;
		}
	}

void TimerEventScheduler::triggerEvents(double time)
	{
	/* Update the current time: */
	currentTime=time;
	
	if(timers.isEmpty())
		return;
	
	/* Create the callback data structure: */
	CallbackData cbData(currentTime);
	
	/* Process expired timer events from the head of the queue: */
	while(timers.getSmallest().time<=currentTime)
		{
		/* Call the callback: */
		timers.getSmallest().callback->call(&cbData);
		
		/* Remove the callback from the queue: */
		delete timers.getSmallest().callback;
		timers.removeSmallest();
		
		if(timers.isEmpty())
			break;
		}
	}

}
