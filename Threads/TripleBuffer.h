/***********************************************************************
TripleBuffer - Class to allow one-way asynchronous non-blocking
communication between a producer and a consumer, in which the producer
writes a stream of data into a buffer, and the consumer can retrieve the
most recently written value at any time.
Copyright (c) 2005-2011 Oliver Kreylos

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

#ifndef THREADS_TRIPLEBUFFER_INCLUDED
#define THREADS_TRIPLEBUFFER_INCLUDED

#include <Threads/Spinlock.h>

namespace Threads {

template <class ValueParam>
class TripleBuffer
	{
	/* Embedded classes: */
	public:
	typedef ValueParam Value; // Type of communicated data
	
	/* Elements: */
	private:
	Value buffer[3]; // The triple-buffer of values
	Spinlock indexSpinlock; // Spinlock protecting the buffer index fields
	volatile int lockedIndex; // Buffer index currently locked by the consumer
	volatile int mostRecentIndex; // Buffer index of most recently produced value
	int nextIndex; // Buffer index of value currently being written into buffer
	
	/* Constructors and destructors: */
	public:
	TripleBuffer(void) // Creates empty triple buffer
		:lockedIndex(0),mostRecentIndex(0)
		{
		}
	private:
	TripleBuffer(const TripleBuffer& source); // Prohibit copy constructor
	TripleBuffer& operator=(const TripleBuffer& source); // Prohibit assignment operator
	public:
	~TripleBuffer(void) // Destroys the triple buffer
		{
		}
	
	/* Low-level methods: */
	Value& getBuffer(int bufferIndex) // Low-level method to access triple buffer contents
		{
		return buffer[bufferIndex];
		}
	
	/* Producer-side methods: */
	Value& startNewValue(void) // Prepares buffer to receive a new value
		{
		/* Determine the index of the currently unused buffer: */
		nextIndex=mostRecentIndex+1;
		if(nextIndex==3)
			nextIndex=0;
		if(nextIndex==lockedIndex)
			{
			if(++nextIndex==3)
				nextIndex=0;
			}
		
		/* Return a reference to the value: */
		return buffer[nextIndex];
		}
	void postNewValue(void) // Marks a new buffer value as most recent after data has been written
		{
		/* Mark the written buffer as most recent: */
		Spinlock::Lock indexLock(indexSpinlock);
		mostRecentIndex=nextIndex;
		}
	void postNewValue(const Value& newValue) // Pushes a new data value into the buffer
		{
		/* Determine the index of the currently unused buffer: */
		nextIndex=mostRecentIndex+1;
		if(nextIndex==3)
			nextIndex=0;
		if(nextIndex==lockedIndex)
			{
			if(++nextIndex==3)
				nextIndex=0;
			}
		
		/* Write the new value: */
		buffer[nextIndex]=newValue;
		
		/* Mark the written buffer as most recent: */
		{
		Spinlock::Lock indexLock(indexSpinlock);
		mostRecentIndex=nextIndex;
		}
		}
	const Value& getMostRecentValue(void) const // Returns the last posted value; must not be called in cases where consumer might change locked value
		{
		return buffer[mostRecentIndex];
		}
	
	/* Consumer-side methods: */
	bool hasNewValue(void) const // Returns true if a new data value is available for the consumer
		{
		return mostRecentIndex!=lockedIndex;
		}
	bool lockNewValue(void) // Locks the most recently written value; returns true if the value is new
		{
		Spinlock::Lock indexLock(indexSpinlock);
		int mri=mostRecentIndex;
		bool result=lockedIndex!=mri;
		lockedIndex=mri;
		return result;
		}
	const Value& getLockedValue(void) const // Returns the currently locked value
		{
		return buffer[lockedIndex];
		}
	Value& getLockedValue(void) // Ditto
		{
		return buffer[lockedIndex];
		}
	};

}

#endif
