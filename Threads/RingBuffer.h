/***********************************************************************
RingBuffer - Class to allow one-way synchronous communication between a
producer and a consumer.
Copyright (c) 2009 Oliver Kreylos

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

#ifndef THREADS_RINGBUFFER_INCLUDED
#define THREADS_RINGBUFFER_INCLUDED

#include <Threads/Mutex.h>
#include <Threads/Cond.h>

namespace Threads {

template <class ValueParam>
class RingBuffer
	{
	/* Embedded classes: */
	public:
	typedef ValueParam Value; // Type of communicated data
	
	class ReadLock // Helper class to lock a region in the ring buffer for reading
		{
		friend class RingBuffer;
		
		/* Elements: */
		private:
		const Value* values; // Base pointer in ring buffer
		size_t numValues; // Number of locked values
		
		/* Constructors and destructors: */
		public:
		ReadLock(void) // Creates invalid lock
			:values(0),numValues(0)
			{
			}
		private:
		ReadLock(const Value* sValues,size_t sNumValues) // Creates valid lock for the given buffer and value region
			:values(sValues),numValues(sNumValues)
			{
			}
		
		/* Methods: */
		public:
		const Value* getValues(void) const
			{
			return values;
			}
		size_t getNumValues(void) const
			{
			return numValues;
			}
		};
	
	class WriteLock // Helper class to lock a region in the ring buffer for writing
		{
		friend class RingBuffer;
		
		/* Elements: */
		private:
		Value* values; // Base pointer in ring buffer
		size_t numValues; // Number of locked values
		
		/* Constructors and destructors: */
		public:
		WriteLock(void) // Creates invalid lock
			:values(0),numValues(0)
			{
			}
		private:
		WriteLock(Value* sValues,size_t sNumValues) // Creates valid lock for the given buffer and value region
			:values(sValues),numValues(sNumValues)
			{
			}
		
		/* Methods: */
		public:
		Value* getValues(void) const
			{
			return values;
			}
		size_t getNumValues(void) const
			{
			return numValues;
			}
		};
	
	/* Elements: */
	private:
	size_t bufferSize; // Size of the ring buffer
	Value* buffer; // The ring buffer
	Value* bufferEnd; // The end of the ring buffer
	Mutex bufferMutex; // Mutex protecting the buffer's read and write positions
	const Value* readPtr; // Current reading position in buffer
	Value* writePtr; // Current writing position in buffer
	size_t bufferUsed; // Current actual number of values in buffer
	Cond bufferCond; // Condition variable to signal a buffer write or read
	
	/* Constructors and destructors: */
	public:
	RingBuffer(size_t sBufferSize) // Creates empty ring buffer of given size
		:bufferSize(sBufferSize),buffer(new Value[bufferSize]),bufferEnd(buffer+bufferSize),
		 readPtr(buffer),writePtr(buffer),bufferUsed(0)
		{
		}
	private:
	RingBuffer(const RingBuffer& source); // Prohibit copy constructor
	RingBuffer& operator=(const RingBuffer& source); // Prohibit assignment operator
	public:
	~RingBuffer(void) // Destroys the ring buffer
		{
		delete[] buffer;
		}
	
	/* Methods: */
	void resize(size_t newBufferSize) // Resizes the buffer, discarding all data
		{
		delete[] buffer;
		bufferSize=newBufferSize;
		buffer=new Value[bufferSize];
		bufferEnd=buffer+bufferSize;
		readPtr=writePtr=buffer;
		bufferUsed=0;
		}
	bool empty(void) const // Returns true if there is no data to be read in the ring buffer
		{
		/* No need to lock: */
		return bufferUsed==0;
		}
	bool full(void) const // Returns true if there is no room to write data in the ring buffer
		{
		/* No need to lock: */
		return bufferUsed==bufferSize;
		}
	ReadLock getReadLock(size_t maxNumValues) // Blocks until at least one value can be read from the buffer; returns lock on number of values
		{
		/* Wait until data becomes available: */
		size_t numValues;
		{
		Mutex::Lock bufferLock(bufferMutex);
		while((numValues=bufferUsed)==0)
			{
			/* Wait until data becomes available: */
			bufferCond.wait(bufferMutex);
			}
		}
		
		/* Adjust the result value for ring buffer wrap-around and requested number of values: */
		size_t bufferEnd=bufferSize-(readPtr-buffer);
		if(numValues>bufferEnd)
			numValues=bufferEnd;
		if(numValues>maxNumValues)
			numValues=maxNumValues;
		
		/* Return a read lock: */
		return ReadLock(readPtr,numValues);
		}
	void releaseReadLock(const ReadLock& readLock) // Releases a read lock; assumes that all data in the locked region has been read
		{
		Mutex::Lock bufferLock(bufferMutex);
		if((readPtr+=readLock.numValues)==bufferEnd)
			readPtr=buffer;
		if(bufferUsed==bufferSize)
			{
			/* Wake up blocked writers: */
			bufferCond.signal();
			}
		bufferUsed-=readLock.numValues;
		}
	WriteLock getWriteLock(size_t maxNumValues) // Blocks until at least one value can be written to the buffer; returns lock on number of values
		{
		/* Wait until space becomes available: */
		size_t numValues;
		{
		Mutex::Lock bufferLock(bufferMutex);
		while((numValues=bufferSize-bufferUsed)==0)
			{
			/* Wait until space becomes available: */
			bufferCond.wait(bufferMutex);
			}
		}
		
		/* Adjust the result value for ring buffer wrap-around and requested number of values: */
		size_t bufferEnd=bufferSize-(writePtr-buffer);
		if(numValues>bufferEnd)
			numValues=bufferEnd;
		if(numValues>maxNumValues)
			numValues=maxNumValues;
		
		/* Return a write lock: */
		return WriteLock(writePtr,numValues);
		}
	void releaseWriteLock(const WriteLock& writeLock) // Releases a write lock; assumes that all data in the locked region has been written
		{
		Mutex::Lock bufferLock(bufferMutex);
		if((writePtr+=writeLock.numValues)==bufferEnd)
			writePtr=buffer;
		if(bufferUsed==0)
			{
			/* Wake up blocked readers: */
			bufferCond.signal();
			}
		bufferUsed+=writeLock.numValues;
		}
	size_t read(Value* values,size_t numValues) // Reads between one and numValues from buffer; returns number read; blocks if no data is available
		{
		/* Lock the buffer: */
		Mutex::Lock bufferLock(bufferMutex);
		
		/* Determine how much can be read from the buffer in one go: */
		size_t chunkSize;
		while((chunkSize=bufferUsed)==0)
			{
			/* Block until more data becomes available: */
			bufferCond.wait(bufferMutex);
			}
		if(chunkSize>numValues)
			chunkSize=numValues;
		
		/* Read from the buffer: */
		for(size_t i=0;i<chunkSize;++i)
			{
			/* Read a value: */
			*values=*readPtr;
			
			/* Go to the next value: */
			if(++readPtr==bufferEnd)
				readPtr=buffer;
			++values;
			}
		
		/* Update the buffer state: */
		if(bufferUsed==bufferSize)
			{
			/* Wake up blocked writers: */
			bufferCond.signal();
			}
		bufferUsed-=chunkSize;
		numValues-=chunkSize;
		
		return chunkSize;
		}
	void blockingRead(Value* values,size_t numValues) // Reads the given array from buffer; blocks until everything is read
		{
		/* Lock the buffer: */
		Mutex::Lock bufferLock(bufferMutex);
		
		/* Read values from the buffer until everything is read: */
		while(numValues>0)
			{
			/* Determine how much can be read from the buffer in one go: */
			size_t chunkSize;
			while((chunkSize=bufferUsed)==0)
				{
				/* Block until more data becomes available: */
				bufferCond.wait(bufferMutex);
				}
			if(chunkSize>numValues)
				chunkSize=numValues;
			
			/* Read from the buffer: */
			for(size_t i=0;i<chunkSize;++i)
				{
				/* Read a value: */
				*values=*readPtr;
				
				/* Go to the next value: */
				if(++readPtr==bufferEnd)
					readPtr=buffer;
				++values;
				}
			
			/* Update the buffer state: */
			if(bufferUsed==bufferSize)
				{
				/* Wake up blocked writers: */
				bufferCond.signal();
				}
			bufferUsed-=chunkSize;
			numValues-=chunkSize;
			}
		}
	void blockingWrite(const Value* values,size_t numValues) // Writes the given array into buffer; blocks until everything is written
		{
		/* Lock the buffer: */
		Mutex::Lock bufferLock(bufferMutex);
		
		/* Write values into the buffer until everything is written: */
		while(numValues>0)
			{
			/* Determine how much can be written into the buffer in one go: */
			size_t chunkSize;
			while((chunkSize=bufferSize-bufferUsed)==0)
				{
				/* Block until more space becomes available: */
				bufferCond.wait(bufferMutex);
				}
			if(chunkSize>numValues)
				chunkSize=numValues;
			
			/* Write into the buffer: */
			for(size_t i=0;i<chunkSize;++i)
				{
				/* Write a value: */
				*writePtr=*values;
				
				/* Go to the next value: */
				if(++writePtr==bufferEnd)
					writePtr=buffer;
				++values;
				}
			
			/* Update the buffer state: */
			if(bufferUsed==0)
				{
				/* Wake up blocked readers: */
				bufferCond.signal();
				}
			bufferUsed+=chunkSize;
			numValues-=chunkSize;
			}
		}
	};

}

#endif
