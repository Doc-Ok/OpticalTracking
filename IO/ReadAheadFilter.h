/***********************************************************************
ReadAheadFilter - Class to add background read-ahead to other IO::File
abstractions to improve read throughput.
Copyright (c) 2011 Oliver Kreylos

This file is part of the I/O Support Library (IO).

The I/O Support Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

The I/O Support Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the I/O Support Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef IO_READAHEADFILTER_INCLUDED
#define IO_READAHEADFILTER_INCLUDED

#include <Threads/Mutex.h>
#include <Threads/Cond.h>
#include <Threads/Thread.h>
#include <IO/File.h>

namespace IO {

class ReadAheadFilter:public File
	{
	/* Elements: */
	private:
	FilePtr source; // The source file
	Threads::Thread readAheadThread; // The background read-ahead thread
	Threads::Mutex bufferMutex; // Mutex serializing access to the read-ahead ring buffer
	Threads::Cond bufferCond; // Condition variable to signal a change in ring buffer state
	size_t halfBufferSize; // Size of each half of the double buffer
	Byte* buffers[2]; // Pointers to double buffer halves
	unsigned int inBuffer; // Index of buffer currently read into
	unsigned int outBuffer; // Index of buffer currently read from
	unsigned int numFullBuffers; // Number of filled double buffer halves
	unsigned int bufferSizes[2]; // Amount of data in the two buffer halves; amount less than full size indicates source was read completely
	bool haveReadOnce; // Flag true if readData has consumed at least one buffer half
	
	/* Protected methods from IO::File: */
	protected:
	virtual size_t readData(Byte* buffer,size_t bufferSize);
	
	/* Private methods: */
	private:
	void* readAheadThreadMethod(void); // The background read-ahead thread's method
	
	/* Constructors and destructors: */
	public:
	ReadAheadFilter(FilePtr sSource);
	virtual ~ReadAheadFilter(void);
	
	/* Methods from File: */
	virtual size_t getReadBufferSize(void) const;
	virtual size_t resizeReadBuffer(size_t newReadBufferSize);
	};

}

#endif
