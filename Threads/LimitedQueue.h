/***********************************************************************
LimitedQueue - Simple limited-size queue to send data from one or more
producers to one or more consumers.
Copyright (c) 2012 Oliver Kreylos

This file is part of the Portable Threading Library (Threads).

The Portable Threading Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Portable Threading Library is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Portable Threading Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef THREADS_LIMITEDQUEUE_INCLUDED
#define THREADS_LIMITEDQUEUE_INCLUDED

#include <stddef.h>
#include <Threads/Mutex.h>
#include <Threads/Cond.h>

namespace Threads {

template <class ValueParam>
class LimitedQueue
	{
	/* Embedded classes: */
	public:
	typedef ValueParam Value; // Type of communicated data
	
	/* Elements: */
	private:
	Value* queue; // Pointer to beginning of queue area
	Value* queueEnd; // Pointer after end of queue area
	Mutex queueMutex; // Mutex to serialize access to the queue and condition variable
	Cond queueCond; // Condition variable to signal queue becoming non-empty or non-full
	Value* head; // Pointer to element at beginning of queue
	Value* tail; // Pointer to element after end of queue
	Value* nextTail; // New tail pointer after a new element is pushed into the queue
	
	/* Constructors and destructors: */
	public:
	LimitedQueue(size_t maxQueueLength) // Creates a queue that can hold at most the given number of elements
		:queue(new Value[maxQueueLength+1]),queueEnd(queue+maxQueueLength+1),
		 head(queue),tail(queue),nextTail(tail+1)
		{
		}
	private:
	LimitedQueue(const LimitedQueue& source); // Prohibit copy constructor
	LimitedQueue& operator=(const LimitedQueue& source); // Prohibit assignment operator
	public:
	~LimitedQueue(void) // Destroys the queue and its contents
		{
		delete[] queue;
		}
	
	/* Methods: */
	void push(const Value& value) // Pushes the given value into the queue; blocks if queue is full
		{
		Mutex::Lock queueLock(queueMutex);
		
		/* Block while the queue is full: */
		while(nextTail==head)
			queueCond.wait(queueMutex);
		
		/* Insert the new element into the queue: */
		*tail=value;
		
		/* Wake up a consumer if the queue just became non-empty: */
		if(tail==head)
			queueCond.broadcast();
		
		/* Advance the tail position: */
		tail=nextTail;
		if(++nextTail==queueEnd)
			nextTail=queue;
		}
	Value pop(void) // Returns and removes the first value from the queue; blocks if queue is empty
		{
		Mutex::Lock queueLock(queueMutex);
		
		/* Block while the queue is empty: */
		while(tail==head)
			queueCond.wait(queueMutex);
		
		/* Return the first element from the queue: */
		Value result=*head;
		
		/* Wake up a producer if the queue just became non-full: */
		if(nextTail==head)
			queueCond.broadcast();
		
		/* Advance the head position: */
		if(++head==queueEnd)
			head=queue;
		
		return result;
		}
	};

}

#endif
