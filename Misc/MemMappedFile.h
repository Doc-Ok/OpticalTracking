/***********************************************************************
MemMappedFile - Wrapper class to provide a file-like interface for
blocks of memory or memory-mapped files with exception safety, typed
data I/O, and automatic endianness conversion.
Copyright (c) 2007-2010 Oliver Kreylos

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

#ifndef MISC_MEMMAPPEDFILE_INCLUDED
#define MISC_MEMMAPPEDFILE_INCLUDED

#include <string.h>
#include <stdexcept>
#include <Misc/ThrowStdErr.h>
#include <Misc/Endianness.h>

namespace Misc {

class MemMappedFile
	{
	/* Embedded classes: */
	public:
	typedef ptrdiff_t Offset; // Type for file offsets
	
	enum Endianness // Enumerated type to enforce file endianness
		{
		DontCare,LittleEndian,BigEndian
		};
	
	class ReadError:public std::runtime_error // Exception class to report file reading errors
		{
		/* Constructors and destructors: */
		public:
		ReadError(size_t numBytes,size_t numBytesRead)
			:std::runtime_error(printStdErrMsg("Misc::MemMappedFile: Error reading %u bytes from file, read %u bytes instead",numBytes,numBytesRead))
			{
			}
		};
	
	class WriteError:public std::runtime_error // Exception class to report file writing errors
		{
		/* Constructors and destructors: */
		public:
		WriteError(size_t numBytes,size_t numBytesWritten)
			:std::runtime_error(printStdErrMsg("Misc::MemMappedFile: Error writing %u bytes to file, wrote %u bytes instead",numBytes,numBytesWritten))
			{
			}
		};
	
	/* Elements: */
	private:
	unsigned char* blockStart; // Start of the file's memory block
	size_t blockSize; // Size of the file's memory block in bytes
	unsigned char* blockEnd; // Pointer behind the end of the file's memory block
	unsigned char* ioPtr; // Current read/write pointer
	bool writeProtected; // Flag if the class is allowed to write into the memory block
	Endianness endianness; // Endianness of the represented file
	bool mustSwapEndianness; // Flag if current file endianness is different from machine endianness
	
	/* Constructors and destructors: */
	public:
	MemMappedFile(unsigned char* sBlockStart,size_t sBlockSize,Endianness sEndianness =DontCare) // Opens file for given memory block
		:blockStart(sBlockStart),blockSize(sBlockSize),
		 blockEnd(blockStart+blockSize),
		 ioPtr(blockStart),
		 writeProtected(false)
		{
		setEndianness(sEndianness);
		}
	MemMappedFile(const unsigned char* sBlockStart,size_t sBlockSize,Endianness sEndianness =DontCare) // Opens file for given write-protected memory block
		:blockStart(const_cast<unsigned char*>(sBlockStart)),blockSize(sBlockSize),
		 blockEnd(blockStart+blockSize),
		 ioPtr(blockStart),
		 writeProtected(true)
		{
		setEndianness(sEndianness);
		}
	MemMappedFile(const MemMappedFile& source) // Copy constructor
		:blockStart(source.blockStart),blockSize(source.blockSize),
		 blockEnd(source.blockEnd),
		 ioPtr(source.ioPtr),
		 writeProtected(source.writeProtected),
		 endianness(source.endianness),mustSwapEndianness(source.mustSwapEndianness)
		{
		}
	
	/* Methods: */
	Endianness getEndianness(void) // Returns current endianness setting of file
		{
		return endianness;
		}
	void setEndianness(Endianness newEndianness) // Sets current endianness setting of file
		{
		endianness=newEndianness;
		#if __BYTE_ORDER==__LITTLE_ENDIAN
		mustSwapEndianness=endianness==BigEndian;
		#endif
		#if __BYTE_ORDER==__BIG_ENDIAN
		mustSwapEndianness=endianness==LittleEndian;
		#endif
		}
	void rewind(void)
		{
		ioPtr=blockStart;
		}
	void seekSet(Offset offset)
		{
		ioPtr=blockStart+offset;
		}
	void seekCurrent(Offset offset)
		{
		ioPtr+=offset;
		}
	void seekEnd(Offset offset)
		{
		ioPtr=blockEnd-offset;
		}
	Offset tell(void)
		{
		return Offset(ioPtr-blockStart);
		}
	bool eof(void)
		{
		return ioPtr==blockEnd;
		}
	
	/* Methods for text file I/O: */
	int getc(void)
		{
		return ioPtr==blockEnd?-1:int(*(ioPtr++));
		}
	int ungetc(int c)
		{
		if(ioPtr>blockStart)
			{
			--ioPtr; // Safe to ignore the passed character; must be the same as the last read
			return c;
			}
		else
			return -1;
		}
	#if 0
	void putc(int c)
		{
		fputc(c,filePtr);
		}
	#endif
	char* gets(char* stringBuffer,int stringBufferSize)
		{
		/* Copy data from the memory block: */
		char* sbPtr=stringBuffer;
		while(stringBufferSize>1&&ioPtr!=blockEnd)
			{
			if((*(sbPtr++)=(char)(*(ioPtr++)))=='\n')
				break;
			--stringBufferSize;
			}
		*sbPtr='\0'; // Add the terminating NUL character
		return stringBuffer;
		}
	int puts(const char* string)
		{
		/* Check if writing is allowed: */
		if(!writeProtected)
			throw WriteError(1,0);
		
		/* Copy data into the memory block: */
		while(*string!='\0'&&ioPtr!=blockEnd)
			*(ioPtr++)=(unsigned char)(*(string++));
		
		/* Append the newline character: */
		if(ioPtr==blockEnd)
			return -1;
		*(ioPtr++)='\n';
		
		return 1;
		}
	
	/* Endianness-safe binary I/O interface: */
	bool mustSwapOnRead(void) // Retusn true if the file must endianness-swap data on read
		{
		return mustSwapEndianness;
		}
	void readRaw(void* buffer,size_t size)
		{
		/* Check if the memory block contains enough data: */
		if(ioPtr+size>blockEnd)
			throw ReadError(size,blockEnd-ioPtr);
		
		/* Copy data from the memory block: */
		memcpy(buffer,ioPtr,size);
		ioPtr+=size;
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
	size_t read(DataParam* data,size_t numItems) // Reads array of values
		{
		size_t numReadItems=numItems;
		size_t readSize=numReadItems*sizeof(DataParam);
		if(ioPtr+readSize>blockEnd)
			{
			numReadItems=(blockEnd-ioPtr)/sizeof(DataParam);
			readSize=numReadItems*sizeof(DataParam);
			}
		readRaw(data,readSize);
		if(mustSwapEndianness)
			swapEndianness(data,numReadItems);
		return numReadItems;
		}
	bool mustSwapOnWrite(void) // Returns true if the file must endianness-swap data on write
		{
		return mustSwapEndianness;
		}
	void writeRaw(const void* buffer,size_t size)
		{
		/* Check if writing is allowed: */
		if(!writeProtected)
			throw WriteError(size,0);
		
		/* Check if the memory block can hold enough data: */
		if(ioPtr+size>blockEnd)
			throw WriteError(size,blockEnd-ioPtr);
		
		/* Copy data into the memory block: */
		memcpy(ioPtr,buffer,size);
		ioPtr+=size;
		}
	template <class DataParam>
	void write(const DataParam& data) // Writes single value
		{
		if(mustSwapEndianness)
			{
			DataParam temp=data;
			swapEndianness(temp);
			writeRaw(&temp,sizeof(DataParam));
			}
		else
			writeRaw(&data,sizeof(DataParam));
		}
	template <class DataParam>
	void write(const DataParam* data,size_t numItems) // Writes array of values
		{
		if(mustSwapEndianness)
			{
			for(size_t i=0;i<numItems;++i)
				{
				DataParam temp=data[i];
				swapEndianness(temp);
				writeRaw(&temp,sizeof(DataParam));
				}
			}
		else
			writeRaw(data,sizeof(DataParam)*numItems);
		}
	};

}

#endif
