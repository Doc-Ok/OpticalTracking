/***********************************************************************
DropoutBuffer - Class implementing a generalization of the triple buffer
communication pattern, where the buffer retains a given number of most
recent items for a consumer to read.
Copyright (c) 2009-2010 Oliver Kreylos

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

#ifndef THREADS_DROPOUTBUFFER_INCLUDED
#define THREADS_DROPOUTBUFFER_INCLUDED

#include <Threads/MutexCond.h>

namespace Threads {

template <class ValueParam>
class DropoutBuffer
	{
	/* Embedded classes: */
	public:
	typedef ValueParam Value; // Type of communicated data
	
	/* Values: */
	private:
	size_t segmentSize; // Size of each buffer segment
	size_t queueSize; // Maximum number of segments in the queue
	Value* buffer; // Pointer to the buffer containing all segments
	MutexCond queueMutex; // Mutex protecting the segment queue and condition variable signaling pushing of a new segment
	Value** readyQueue; // Queue of segments ready for reading, in writing order
	size_t readyQueueSize; // Number of segments in ready queue
	Value** freeQueue; // Queue of unused segments ready for writing
	size_t freeQueueSize; // Number of segments in free queue
	Value* readSegment; // Segment currently being read from
	Value* writeSegment; // Segment currently being written to
	Value** lockedQueue; // Queue of locked segments, in writing order
	size_t lockedQueueSize; // Number of segments in locked queue
	
	/* Constructors and destructors: */
	public:
	DropoutBuffer(size_t sSegmentSize,size_t sQueueSize) // Creates buffer for given segment and queue size (max number of retained elements)
		:segmentSize(sSegmentSize),queueSize(sQueueSize),
		 buffer(new Value[segmentSize*(queueSize+2)]),
		 readyQueue(new Value*[queueSize]),readyQueueSize(0),
		 freeQueue(new Value*[queueSize]),freeQueueSize(queueSize),
		 readSegment(buffer),writeSegment(buffer+segmentSize),
		 lockedQueue(new Value*[queueSize]),lockedQueueSize(0)
		{
		/* Initialize free queue: */
		for(size_t i=0;i<queueSize;++i)
			freeQueue[i]=buffer+segmentSize*(i+2);
		}
	~DropoutBuffer(void)
		{
		delete[] buffer;
		delete[] readyQueue;
		delete[] freeQueue;
		delete[] lockedQueue;
		}
	
	/* Methods: */
	size_t getSegmentSize(void) const // Returns size of a queue segment
		{
		return segmentSize;
		}
	size_t getMaxQueueSize(void) const // Returns maximum number of segments in the queue
		{
		return queueSize;
		}
	void resize(size_t newSegmentSize,size_t newQueueSize) // Clears and then resizes the queue
		{
		Threads::MutexCond::Lock queueLock(queueMutex);
		
		/* Delete the old queue structures: */
		delete[] buffer;
		delete[] readyQueue;
		delete[] freeQueue;
		delete[] lockedQueue;
		
		/* Allocate new queue structures: */
		segmentSize=newSegmentSize;
		queueSize=newQueueSize;
		buffer=new Value[segmentSize*(queueSize+2)];
		readyQueue=new Value*[queueSize];
		freeQueue=new Value*[queueSize];
		lockedQueue=new Value*[queueSize];
		
		/* Initialize queue state: */
		readyQueueSize=0;
		freeQueueSize=queueSize;
		for(size_t i=0;i<queueSize;++i)
			freeQueue[i]=buffer+segmentSize*(i+2);
		readSegment=buffer;
		writeSegment=buffer+segmentSize;
		lockedQueueSize=0;
		}
	Value* getWriteSegment(void) // Returns pointer to segment ready for writing
		{
		return writeSegment;
		}
	void pushSegment(void) // Marks a segment as complete after writing
		{
		Threads::MutexCond::Lock queueLock(queueMutex);
		
		if(freeQueueSize>0)
			{
			/* Push the finished segment onto the ready queue, and grab another one from the free queue: */
			readyQueue[readyQueueSize]=writeSegment;
			++readyQueueSize;
			--freeQueueSize;
			writeSegment=freeQueue[freeQueueSize];
			}
		else
			{
			/* Discard the oldest ready segment and reuse it for writing: */
			Value* newWriteSegment=readyQueue[0];
			for(size_t i=1;i<readyQueueSize;++i)
				readyQueue[i-1]=readyQueue[i];
			readyQueue[readyQueueSize-1]=writeSegment;
			writeSegment=newWriteSegment;
			}
		
		/* Signal arrival of a new segment: */
		queueMutex.broadcast();
		}
	size_t getQueueSize(void) const // Returns the current size of the queue
		{
		return readyQueueSize;
		}
	const Value* testPopSegment(void) // Removes and returns the oldest segment from the queue; returns NULL on empty queue
		{
		Threads::MutexCond::Lock queueLock(queueMutex);
		
		/* Check for empty queue: */
		if(readyQueueSize==0)
			return 0;
		
		/* Add the current read segment to the free queue: */
		freeQueue[freeQueueSize]=readSegment;
		++freeQueueSize;
		
		/* Remove the oldest element from the ready queue and make it the new read segment: */
		readSegment=readyQueue[0];
		--readyQueueSize;
		for(size_t i=0;i<readyQueueSize;++i)
			readyQueue[i]=readyQueue[i+1];
		
		return readSegment;
		}
	const Value* popSegment(void) // Removes and returns the oldest segment from the queue; blocks on empty queue
		{
		Threads::MutexCond::Lock queueLock(queueMutex);
		
		/* Check for empty queue: */
		while(readyQueueSize==0)
			queueMutex.wait(queueLock);
		
		/* Add the current read segment to the free queue: */
		freeQueue[freeQueueSize]=readSegment;
		++freeQueueSize;
		
		/* Remove the oldest element from the ready queue and make it the new read segment: */
		readSegment=readyQueue[0];
		--readyQueueSize;
		for(size_t i=0;i<readyQueueSize;++i)
			readyQueue[i]=readyQueue[i+1];
		
		return readSegment;
		}
	size_t lockQueue(void) // Locks all ready segments in the queue to be read and discarded at once
		{
		Threads::MutexCond::Lock queueLock(queueMutex);
		
		/* Get the number of ready segments in the queue, but leave at least one segment unlocked: */
		lockedQueueSize=readyQueueSize;
		if(lockedQueueSize==queueSize)
			--lockedQueueSize;
		
		/* Copy the ready segments into the locked queue: */
		for(size_t i=0;i<lockedQueueSize;++i)
			lockedQueue[i]=readyQueue[i];
		
		/* Remove all locked segments from the ready queue: */
		readyQueueSize-=lockedQueueSize;
		for(size_t i=0;i<readyQueueSize;++i)
			readyQueue[i]=readyQueue[i+lockedQueueSize];
		
		return lockedQueueSize;
		}
	size_t getLockedQueueSize(void) const // Returns the number of elements in the locked queue
		{
		return lockedQueueSize;
		}
	const Value* getLockedSegment(size_t segmentIndex) const // Returns one of the locked segments in writing order
		{
		return lockedQueue[segmentIndex];
		}
	void unlockQueue(void) // Removes all locked segments from the queue
		{
		Threads::MutexCond::Lock queueLock(queueMutex);
		
		/* Move all segments from the locked queue to the free queue: */
		for(size_t i=0;i<lockedQueueSize;++i)
			freeQueue[freeQueueSize+i]=lockedQueue[i];
		freeQueueSize+=lockedQueueSize;
		
		/* Clear the locked queue: */
		lockedQueueSize=0;
		}
	};

}

#endif
