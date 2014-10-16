/***********************************************************************
ReadBuffer - Class to read from a memory buffer using an endianness-safe
pipe-like interface.
Copyright (c) 2010-2011 Oliver Kreylos

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

#ifndef MISC_READBUFFER_INCLUDED
#define MISC_READBUFFER_INCLUDED

#include <string.h>
#include <stdexcept>
#include <Misc/Endianness.h>

namespace Misc {

class ReadBuffer
	{
	/* Embedded classes: */
	public:
	class ReadError:public std::runtime_error // Exception class to report buffer reading errors
		{
		/* Constructors and destructors: */
		public:
		ReadError(size_t numBytes,size_t numBytesRead); // Error where numBytesRead were read instead of numBytes
		};
	
	private:
	typedef unsigned char Byte; // Type for raw bytes
	
	/* Elements: */
	private:
	size_t bufferSize; // Size of the buffer
	Byte* buffer; // Pointer to the buffer
	Byte* bufferEnd; // Pointer behind the end of the buffer
	bool mustSwapEndianness; // Flag if current buffer endianness is different from machine endianness
	Byte* readPtr; // Pointer to the current read position
	
	/* Constructors and destructors: */
	public:
	ReadBuffer(size_t sBufferSize); // Creates a read buffer of the given size
	private:
	ReadBuffer(const ReadBuffer& source); // Prohibit copy constructor
	ReadBuffer& operator=(const ReadBuffer& source); // Prohibit assignment operator
	public:
	~ReadBuffer(void); // Destroys the read buffer
	
	/* Methods: */
	void setEndianness(Endianness newEndianness); // Sets the endianness of the data in the buffer for all following read accesses
	size_t getBufferSize(void) const // Returns the size of the buffer
		{
		return bufferSize;
		}
	void* getBuffer(void) // Returns the buffer
		{
		return buffer;
		}
	void setDataSize(size_t newDataSize); // Sets the amount of data in the buffer after it has been filled in by the caller and rewinds the buffer
	template <class SourceParam>
	void readFromSource(SourceParam& source) // Fills the entire buffer by reading from a binary data source
		{
		/* Read the entire buffer: */
		source.readRaw(buffer,bufferSize);
		
		/* Rewind the buffer: */
		bufferEnd=buffer+bufferSize;
		readPtr=buffer;
		}
	size_t getUnread(void) const // Returns the amount of unread data left in the buffer
		{
		return bufferEnd-readPtr;
		}
	bool eof(void) const // Returns true if the entire buffer has been read
		{
		return readPtr==bufferEnd;
		}
	
	/* Endianness-safe binary I/O interface: */
	bool mustSwapOnRead(void) const // Returns true if data must be endianness-swapped on read
		{
		return mustSwapEndianness;
		}
	void setSwapOnRead(bool newSwapOnRead); // Enables or disables endianness swapping
	void readRaw(void* data,size_t dataSize) // Reads a chunk of data from the buffer
		{
		/* Check if there is enough unread data in the buffer: */
		size_t unread=bufferEnd-readPtr;
		if(unread>=dataSize)
			{
			/* Read data from the buffer: */
			memcpy(data,readPtr,dataSize);
			readPtr+=dataSize;
			}
		else
			throw ReadError(dataSize,unread);
		}
	template <class DataParam>
	DataParam read(void) // Reads single value
		{
		DataParam result;
		readRaw(&result,sizeof(DataParam));
		if(mustSwapEndianness)
			swapEndianness(result);
		return result;
		}
	template <class DataParam>
	DataParam& read(DataParam& data) // Reads single value through reference
		{
		readRaw(&data,sizeof(DataParam));
		if(mustSwapEndianness)
			swapEndianness(data);
		return data;
		}
	template <class DataParam>
	void read(DataParam* data,size_t numItems) // Reads array of values
		{
		readRaw(data,numItems*sizeof(DataParam));
		if(mustSwapEndianness)
			swapEndianness(data,numItems);
		}
	template <class DataParam>
	void skip(size_t numItems) // Skips array of values
		{
		size_t dataSize=numItems*sizeof(DataParam);
		size_t unread=bufferEnd-readPtr;
		if(unread>=dataSize)
			readPtr+=dataSize;
		else
			throw ReadError(dataSize,unread);
		}
	};

}

#endif
