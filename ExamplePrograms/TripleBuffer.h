/***********************************************************************
TripleBuffer - Class to allow one-way asynchronous non-blocking
communication between a producer and a consumer, in which the producer
writes a stream of data into a buffer, and the consumer can retrieve the
most recently written value at any time.
Version of same class in Threads library with different API to make it
actually useful in an application context.
Copyright (c) 2007 Oliver Kreylos

This file is part of the Virtual Jell-O interactive VR demonstration.

Virtual Jell-O is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

Virtual Jell-O is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with Virtual Jell-O; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef TRIPLEBUFFER_INCLUDED
#define TRIPLEBUFFER_INCLUDED

template <class ValueParam>
class TripleBuffer
	{
	/* Embedded classes: */
	public:
	typedef ValueParam Value; // Type of communicated data
	
	/* Elements: */
	private:
	Value values[3]; // The triple-buffer of values
	volatile int lockedIndex; // Buffer index currently locked by the consumer
	volatile int mostRecentIndex; // Buffer index of most recently produced value
	int nextIndex; // Index of buffer that is currently written to
	
	/* Constructors and destructors: */
	public:
	TripleBuffer(void) // Creates empty triple buffer
		:lockedIndex(0),mostRecentIndex(0),nextIndex(1)
		{
		};
	private:
	TripleBuffer(const TripleBuffer& source); // Prohibit copy constructor
	TripleBuffer& operator=(const TripleBuffer& source); // Prohibit assignment operator
	public:
	~TripleBuffer(void) // Destroys the triple buffer
		{
		};
	
	/* Methods: */
	Value& accessSlot(int slotIndex) // Allows applications to explicitly initialize triple buffer slots before communication starts
		{
		return values[slotIndex];
		};
	Value& startWrite(void) // Starts writing into an unused buffer slot
		{
		/* The order of the following operations is crucial to prevent race conditions: */
		nextIndex=(lockedIndex+1)%3;
		if(nextIndex==mostRecentIndex)
			nextIndex=(nextIndex+1)%3;
		return values[nextIndex];
		};
	Value& getNextValue(void) // Returns a reference to the buffer currently being written (only allowed between startWrite/finishWrite)
		{
		return values[nextIndex];
		};
	void finishWrite(void) // Finishes writing and marks the buffer currently being written as the most recent buffer
		{
		mostRecentIndex=nextIndex;
		};
	void write(const Value& newValue) // Writes a new value into an unused buffer slot
		{
		/* The order of the following operations is crucial to prevent race conditions: */
		nextIndex=(lockedIndex+1)%3;
		if(nextIndex==mostRecentIndex)
			nextIndex=(nextIndex+1)%3;
		
		values[nextIndex]=newValue;
		
		mostRecentIndex=nextIndex;
		};
	bool hasNewValue(void) const // Returns true if a new data value is available for the consumer
		{
		return mostRecentIndex!=lockedIndex;
		};
	const Value& lockNewValue(void) // Locks the most recently written buffer slot
		{
		lockedIndex=mostRecentIndex;
		return values[lockedIndex];
		};
	const Value& getLockedValue(void) const // Returns the currently locked value
		{
		return values[lockedIndex];
		};
	};

#endif
