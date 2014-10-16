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

#ifndef MISC_TIMEREVENTSCHEDULER_INCLUDED
#define MISC_TIMEREVENTSCHEDULER_INCLUDED

#include <typeinfo>
#include <Misc/PriorityHeap.h>
#include <Misc/CallbackData.h>

namespace Misc {

class TimerEventScheduler
	{
	/* Embedded classes: */
	public:
	struct CallbackData:public Misc::CallbackData // Callback data structure for timer events
		{
		/* Elements: */
		public:
		double time; // Time at which the timer event was triggered
		
		/* Constructors and destructors: */
		CallbackData(double sTime)
			:time(sTime)
			{
			}
		};
	
	/* Abstract base class for callbacks: */
	private:
	class Callback
		{
		friend class TimerEventScheduler;
		
		/* Constructors and destructors: */
		public:
		virtual ~Callback(void) // Virtual destructor
			{
			}
		
		/* Methods: */
		virtual bool operator==(const Callback& other) const =0; // Virtual method to compare callbacks
		virtual void call(CallbackData* callbackData) const =0; // Virtual method to invoke callback
		};
	
	/* Class to call C functions with an additional void* parameter (traditional C-style callback): */
	class FunctionCallback:public Callback
		{
		/* Elements: */
		private:
		CallbackType callbackFunction; // Pointer to callback function
		void* userData; // Additional callback function parameter
		
		/* Constructors and destructors: */
		public:
		FunctionCallback(CallbackType sCallbackFunction,void* sUserData) // Creates callback for given function with given additional parameter
			:callbackFunction(sCallbackFunction),userData(sUserData)
			{
			}
		
		/* Methods: */
		virtual bool operator==(const Callback& other) const;
		virtual void call(CallbackData* cbData) const;
		};
	
	/* Class to call arbitrary methods on objects of arbitrary type: */
	template <class CallbackClassParam>
	class MethodCallback:public Callback
		{
		/* Embedded classes: */
		public:
		typedef CallbackClassParam CallbackClass; // Class of called objects
		typedef void (CallbackClass::*CallbackMethod)(CallbackData*); // Type of called callback method
		
		/* Elements: */
		private:
		CallbackClass* callbackObject; // Pointer to callback object
		CallbackMethod callbackMethod; // Pointer to callback method
		
		/* Constructors and destructors: */
		public:
		MethodCallback(CallbackClass* sCallbackObject,CallbackMethod sCallbackMethod) // Creates callback for given method on given object
			:callbackObject(sCallbackObject),callbackMethod(sCallbackMethod)
			{
			}
		
		/* Methods: */
		virtual bool operator==(const Callback& other) const
			{
			if(typeid(other)!=typeid(MethodCallback))
				return false;
			const MethodCallback* other2=static_cast<const MethodCallback*>(&other);
			return callbackObject==other2->callbackObject&&callbackMethod==other2->callbackMethod;
			}
		virtual void call(CallbackData* callbackData) const
			{
			/* Call the callback method on the callback object: */
			(callbackObject->*callbackMethod)(callbackData);
			}
		};
	
	/* Class to call arbitrary methods taking a parameter derived from CallbackData on objects of arbitrary type: */
	template <class CallbackClassParam,class DerivedCallbackDataParam>
	class MethodCastCallback:public Callback
		{
		/* Embedded classes: */
		public:
		typedef CallbackClassParam CallbackClass; // Class of called objects
		typedef DerivedCallbackDataParam DerivedCallbackData; // Class of callback data (must be derived from CallbackData)
		typedef void (CallbackClass::*CallbackMethod)(DerivedCallbackData*); // Type of called callback method
		
		/* Elements: */
		private:
		CallbackClass* callbackObject; // Pointer to callback object
		CallbackMethod callbackMethod; // Pointer to callback method
		
		/* Constructors and destructors: */
		public:
		MethodCastCallback(CallbackClass* sCallbackObject,CallbackMethod sCallbackMethod) // Creates callback for given method on given object
			:callbackObject(sCallbackObject),callbackMethod(sCallbackMethod)
			{
			}
		
		/* Methods: */
		virtual bool operator==(const Callback& other) const
			{
			if(typeid(other)!=typeid(MethodCastCallback))
				return false;
			const MethodCastCallback* other2=static_cast<const MethodCastCallback*>(&other);
			return callbackObject==other2->callbackObject&&callbackMethod==other2->callbackMethod;
			}
		virtual void call(CallbackData* callbackData) const
			{
			/* Call the callback method on the callback object with downcasted callback data: */
			(callbackObject->*callbackMethod)(static_cast<DerivedCallbackData*>(callbackData));
			}
		};
	
	struct Timer // Structure to store a time point and its associated callback
		{
		/* Elements: */
		public:
		double time; // Time at which the callback is supposed to happen
		Callback* callback; // The callback to call when the time comes
		
		/* Constructors and destructors: */
		Timer(double sTime,Callback* sCallback) // Elementwise initialization
			:time(sTime),callback(sCallback)
			{
			}
		
		/* Methods: */
		static bool lessEqual(const Timer& t1,const Timer& t2) // Comparison function required for PriorityHeap
			{
			return t1.time<=t2.time;
			}
		};
	
	typedef PriorityHeap<Timer,Timer> TimerQueue; // Type for ordered queues of timer events
	
	/* Elements: */
	private:
	TimerQueue timers; // List of pending timer events, in increasing order of time
	double currentTime; // The current time; actually the last time point for which events were triggered
	
	/* Constructors and destructors: */
	public:
	TimerEventScheduler(void); // Creates an empty event scheduler
	private:
	TimerEventScheduler(const TimerEventScheduler& source); // Prohibit copy constructor
	TimerEventScheduler& operator=(const TimerEventScheduler& source); // Prohibit assignment operator
	public:
	virtual ~TimerEventScheduler(void); // Virtual destructor
	
	/* Methods: */
	
	/* Methods to schedule events for different types of callbacks: */
	void scheduleEvent(double eventTime,CallbackType newCallbackFunction,void* newUserData) // Schedules an event for a C-style callback at the given time
		{
		scheduleEvent(eventTime,new FunctionCallback(newCallbackFunction,newUserData));
		}
	template <class CallbackClassParam>
	void scheduleEvent(double eventTime,CallbackClassParam* newCallbackObject,void (CallbackClassParam::*newCallbackMethod)(CallbackData*)) // Schedules an event for a method callback at the given time
		{
		scheduleEvent(eventTime,new MethodCallback<CallbackClassParam>(newCallbackObject,newCallbackMethod));
		}
	template <class CallbackClassParam,class DerivedCallbackDataParam>
	void scheduleEvent(double eventTime,CallbackClassParam* newCallbackObject,void (CallbackClassParam::*newCallbackMethod)(DerivedCallbackDataParam*)) // Schedules an event for a method callback with downcast at the given time
		{
		scheduleEvent(eventTime,new MethodCastCallback<CallbackClassParam,DerivedCallbackDataParam>(newCallbackObject,newCallbackMethod));
		}
	void scheduleEvent(double eventTime,Callback* callback); // The actual scheduling method
	
	/* Methods to remove previously scheduled events: */
	void removeEvent(double eventTime,CallbackType callbackFunction,void* userData) // Removes a previously scheduled event for a C-style callback
		{
		removeEvent(eventTime,FunctionCallback(callbackFunction,userData));
		}
	template <class CallbackClassParam>
	void removeEvent(double eventTime,CallbackClassParam* callbackObject,void (CallbackClassParam::*callbackMethod)(CallbackData*)) // Removes a previously scheduled event for a method callback
		{
		removeEvent(eventTime,MethodCallback<CallbackClassParam>(callbackObject,callbackMethod));
		}
	template <class CallbackClassParam,class DerivedCallbackDataParam>
	void removeEvent(double eventTime,CallbackClassParam* callbackObject,void (CallbackClassParam::*callbackMethod)(DerivedCallbackDataParam*)) // Removes a previously scheduled event for a method callback with downcast
		{
		removeEvent(eventTime,MethodCastCallback<CallbackClassParam,DerivedCallbackDataParam>(callbackObject,callbackMethod));
		}
	void removeEvent(double eventTime,const Callback& callback); // The actual event removal method
	
	/* Methods to remove all previously scheduled events for a given callback (to clean up before callback recipient's deletion): */
	void removeAllEvents(CallbackType callbackFunction,void* userData) // Removes all previously scheduled events for a C-style callback
		{
		removeAllEvents(FunctionCallback(callbackFunction,userData));
		}
	template <class CallbackClassParam>
	void removeAllEvents(CallbackClassParam* callbackObject,void (CallbackClassParam::*callbackMethod)(CallbackData*)) // Removes all previously scheduled events for a method callback
		{
		removeAllEvents(MethodCallback<CallbackClassParam>(callbackObject,callbackMethod));
		}
	template <class CallbackClassParam,class DerivedCallbackDataParam>
	void removeAllEvents(double eventTime,CallbackClassParam* callbackObject,void (CallbackClassParam::*callbackMethod)(DerivedCallbackDataParam*)) // Removes all previously scheduled events for a method callback with downcast
		{
		removeAllEvents(MethodCastCallback<CallbackClassParam,DerivedCallbackDataParam>(callbackObject,callbackMethod));
		}
	void removeAllEvents(const Callback& callback); // The actual event removal method
	
	/* Methods to query and trigger scheduled timer events: */
	double getCurrentTime(void) const // Returns the scheduler's current time
		{
		return currentTime;
		}
	bool hasPendingEvents(void) const // Returns true if the scheduler has any scheduled events
		{
		return !timers.isEmpty();
		}
	double getNextEventTime(void) const // Returns the time of the next scheduled event
		{
		return timers.getSmallest().time;
		}
	void triggerEvents(void); // Triggers all timer events that were scheduled before or on the current time-of-day
	void triggerEvents(double time); // Triggers all timer events that were scheduled before or on the given time
	};

}

#endif
