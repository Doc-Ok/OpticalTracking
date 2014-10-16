/***********************************************************************
PacketBuffer - Class to read/write arbitrary data types from/into memory
buffers, as intermediate storage for file access or network
transmission.
Copyright (c) 2010 Oliver Kreylos

This file is part of the Vrui calibration utility package.

The Vrui calibration utility package is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Vrui calibration utility package is distributed in the hope that it
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Vrui calibration utility package; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef PACKETBUFFER_INCLUDED
#define PACKETBUFFER_INCLUDED

#include <string.h>
#include <stdexcept>
#include <Misc/Endianness.h>
#include <Misc/ThrowStdErr.h>

class PacketBuffer
	{
	/* Embedded classes: */
	public:
	enum Endianness // Enumerated type to enforce buffer endianness
		{
		DontCare,LittleEndian,BigEndian
		};
	
	class ReadError:public std::runtime_error // Exception class to report file reading errors
		{
		/* Constructors and destructors: */
		public:
		ReadError(size_t numBytes,size_t numBytesRead)
			:std::runtime_error(Misc::printStdErrMsg("PacketBuffer: Error reading %u bytes from packet, read %u bytes instead",numBytes,numBytesRead))
			{
			}
		};
	
	/* Elements: */
	private:
	Endianness endianness; // Endianness of the buffer
	bool mustSwapEndianness; // Flag if current buffer endianness is different from machine endianness
	size_t allocatedSize; // Current allocated size of packet buffer
	unsigned char* buffer; // Pointer to allocated buffer
	unsigned char* bufferEnd; // Pointer to end of allocated buffer
	unsigned char* dataEnd; // Pointer to end of data in buffer
	unsigned char* readPtr; // Current read position
	
	/* Private methods: */
	void resizeBuffer(size_t newSize) // Resizes the buffer to at least the given size
		{
		size_t newAllocatedSize=allocatedSize;
		while(newAllocatedSize<newSize)
			newAllocatedSize=(newAllocatedSize*3+2)/2;
		
		unsigned char* newBuffer=new unsigned char[newAllocatedSize];
		memcpy(newBuffer,buffer,dataEnd-buffer);
		
		allocatedSize=newAllocatedSize;
		bufferEnd=newBuffer+allocatedSize;
		dataEnd=newBuffer+(dataEnd-buffer);
		readPtr=newBuffer+(readPtr-buffer);
		buffer=newBuffer;
		}
	
	/* Constructors and destructors: */
	public:
	PacketBuffer(void) // Creates an unallocated packet buffer
		:endianness(DontCare),mustSwapEndianness(false),
		 allocatedSize(0),buffer(0),
		 bufferEnd(0),dataEnd(0),
		 readPtr(0)
		{
		}
	PacketBuffer(size_t sAllocatedSize,Endianness sEndianness =DontCare) // Creates a buffer with the given initial size
		:endianness(DontCare),mustSwapEndianness(false),
		 allocatedSize(sAllocatedSize),buffer(new unsigned char[allocatedSize]),
		 bufferEnd(buffer+allocatedSize),dataEnd(buffer),
		 readPtr(buffer)
		{
		setEndianness(sEndianness);
		}
	private:
	PacketBuffer(const PacketBuffer& source); // Prohibit copy constructor
	PacketBuffer& operator=(const PacketBuffer& source); // Prohibit assignment operator
	public:
	~PacketBuffer(void)
		{
		delete[] buffer;
		}
	
	/* Methods: */
	Endianness getEndianness(void) // Returns current endianness setting of buffer
		{
		return endianness;
		}
	void setEndianness(Endianness newEndianness) // Sets current endianness setting of buffer
		{
		endianness=newEndianness;
		#if __BYTE_ORDER==__LITTLE_ENDIAN
		mustSwapEndianness=endianness==BigEndian;
		#endif
		#if __BYTE_ORDER==__BIG_ENDIAN
		mustSwapEndianness=endianness==LittleEndian;
		#endif
		}
	const void* getPacket(void) const // Returns the buffered data
		{
		return buffer;
		}
	void* getPacket(void) // Ditto
		{
		return buffer;
		}
	size_t getMaxPacketSize(void) const // Returns the allocated size of the packet buffer
		{
		return allocatedSize;
		}
	void setPacketSize(size_t newPacketSize) // Explicitly sets the packet size of the buffer
		{
		dataEnd=buffer+newPacketSize;
		}
	size_t getPacketSize(void) const // Returns the size of the buffered data in bytes
		{
		return dataEnd-buffer;
		}
	void rewind(void) // Resets the reading position to the beginning of the buffer
		{
		readPtr=buffer;
		}
	template <class DataParam>
	DataParam read(void) // Reads single value
		{
		/* Check if there is enough data in the buffer: */
		if(readPtr+sizeof(DataParam)>dataEnd)
			throw ReadError(sizeof(DataParam),dataEnd-readPtr);
		
		/* Read the next value: */
		DataParam result;
		memcpy(&result,readPtr,sizeof(DataParam));
		readPtr+=sizeof(DataParam);
		if(mustSwapEndianness)
			Misc::swapEndianness(result);
		return result;
		}
	template <class DataParam>
	DataParam& read(DataParam& data) // Reads single value through reference
		{
		/* Check if there is enough data in the buffer: */
		if(readPtr+sizeof(DataParam)>dataEnd)
			throw ReadError(sizeof(DataParam),dataEnd-readPtr);
		
		/* Read the next value: */
		memcpy(&data,readPtr,sizeof(DataParam));
		readPtr+=sizeof(DataParam);
		if(mustSwapEndianness)
			Misc::swapEndianness(data);
		return data;
		}
	template <class DataParam>
	size_t read(DataParam* data,size_t numItems) // Reads array of values
		{
		/* Check if there is enough data in the buffer: */
		if(readPtr+sizeof(DataParam)*numItems>dataEnd)
			throw ReadError(sizeof(DataParam)*numItems,dataEnd-readPtr);
		
		/* Read the next values: */
		memcpy(data,readPtr,sizeof(DataParam)*numItems);
		readPtr+=sizeof(DataParam)*numItems;
		if(mustSwapEndianness)
			Misc::swapEndianness(data,numItems);
		return numItems;
		}
	void clear(void) // Clears buffer; shorthand for setDataSize(0)
		{
		dataEnd=buffer;
		}
	template <class DataParam>
	void write(const DataParam& data) // Writes single value
		{
		/* Check if there is enough space in the buffer: */
		if(dataEnd+sizeof(DataParam)>bufferEnd)
			resizeBuffer((dataEnd+sizeof(DataParam))-buffer);
		
		/* Write the next value: */
		if(mustSwapEndianness)
			{
			DataParam temp=data;
			Misc::swapEndianness(temp);
			memcpy(dataEnd,&temp,sizeof(DataParam));
			}
		else
			memcpy(dataEnd,&data,sizeof(DataParam));
		dataEnd+=sizeof(DataParam);
		}
	template <class DataParam>
	void write(const DataParam* data,size_t numItems) // Writes array of values
		{
		/* Check if there is enough space in the buffer: */
		if(dataEnd+sizeof(DataParam)*numItems>bufferEnd)
			resizeBuffer((dataEnd+sizeof(DataParam)*numItems)-buffer);
		
		if(mustSwapEndianness)
			{
			for(size_t i=0;i<numItems;++i)
				{
				DataParam temp=data[i];
				Misc::swapEndianness(temp);
				memcpy(dataEnd,&temp,sizeof(DataParam));
				dataEnd+=sizeof(DataParam);
				}
			}
		else
			{
			memcpy(dataEnd,&data,sizeof(DataParam)*numItems);
			dataEnd+=sizeof(DataParam)*numItems;
			}
		}
	};

#endif
