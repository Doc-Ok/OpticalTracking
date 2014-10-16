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

#include <IO/ReadAheadFilter.h>

#include <stdexcept>
#include <Misc/Utility.h>

namespace IO {

/********************************
Methods of class ReadAheadFilter:
********************************/

size_t ReadAheadFilter::readData(File::Byte* buffer,size_t bufferSize)
	{
	{
	Threads::Mutex::Lock bufferLock(bufferMutex);
	
	if(haveReadOnce)
		{
		/* Release the just-finished buffer half: */
		--numFullBuffers;
		bufferCond.signal();
		}
	
	/* Check if the double buffer is empty: */
	while(numFullBuffers==0)
		{
		/* Wait for more data: */
		bufferCond.wait(bufferMutex);
		}
	}
	
	/* Read from the next double buffer half: */
	outBuffer=1-outBuffer;
	setReadBuffer(bufferSizes[outBuffer],buffers[outBuffer],false);
	haveReadOnce=true;
	
	return bufferSizes[outBuffer];
	}

void* ReadAheadFilter::readAheadThreadMethod(void)
	{
	/* Enable immediate cancellation: */
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	// Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	do
		{
		/* Fill the next double-buffer half: */
		inBuffer=1-inBuffer;
		Byte* bufPtr=buffers[inBuffer];
		size_t bufSize=halfBufferSize;
		try
			{
			while(bufSize>0)
				{
				/* Read into the buffer: */
				size_t readSize=source->readUpTo(bufPtr,bufSize);
				
				/* Check for end-of-file: */
				if(readSize==0)
					break;
				
				bufPtr+=readSize;
				bufSize-=readSize;
				}
			bufferSizes[inBuffer]=halfBufferSize-bufSize;
			}
		catch(std::runtime_error)
			{
			/* Ignore the error; reader thread will treat it as end-of-file: */
			bufferSizes[inBuffer]=0;
			}
		
		{
		Threads::Mutex::Lock bufferLock(bufferMutex);
		
		/* Hand the filled buffer to the reader: */
		++numFullBuffers;
		bufferCond.signal();
		
		/* Check if the double buffer is full: */
		while(numFullBuffers==2)
			{
			/* Wait for room in the buffer: */
			bufferCond.wait(bufferMutex);
			}
		}
		}
	while(bufferSizes[inBuffer]!=0);
	
	return 0;
	}

ReadAheadFilter::ReadAheadFilter(FilePtr sSource)
	:File(),
	 source(sSource),
	 halfBufferSize(Misc::max(source->getReadBufferSize(),size_t(8192))),
	 inBuffer(1),outBuffer(1),numFullBuffers(0),
	 haveReadOnce(false)
	{
	/* Initialize the double buffer halves: */
	buffers[0]=new Byte[halfBufferSize*2];
	buffers[1]=buffers[0]+halfBufferSize;
	for(int i=0;i<2;++i)
		bufferSizes[i]=0;
	
	/* Start the read-ahead thread: */
	readAheadThread.start(this,&ReadAheadFilter::readAheadThreadMethod);
	
	/* Disable read-through: */
	canReadThrough=false;
	}

ReadAheadFilter::~ReadAheadFilter(void)
	{
	/* Shut down the read-ahead thread: */
	readAheadThread.cancel();
	readAheadThread.join();
	
	/* Release the file's read buffer: */
	setReadBuffer(0,0,false);
	
	/* Delete the double buffer: */
	delete[] buffers[0];
	}

size_t ReadAheadFilter::getReadBufferSize(void) const
	{
	/* Return the size of a half buffer: */
	return halfBufferSize;
	}

size_t ReadAheadFilter::resizeReadBuffer(size_t newReadBufferSize)
	{
	/* Ignore the request and return the current read buffer size: */
	return halfBufferSize;
	}

}
