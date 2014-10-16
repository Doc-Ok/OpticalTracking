/***********************************************************************
File - Base class for high-performance buffered binary read/write access
to file-like objects.
Copyright (c) 2010-2011 Oliver Kreylos

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

#ifndef IO_FILE_INCLUDED
#define IO_FILE_INCLUDED

#include <stddef.h>
#include <string.h>
#include <stdexcept>
#include <Misc/Endianness.h>
#include <Misc/RefCounted.h>
#include <Misc/Autopointer.h>

namespace IO {

class File:public Misc::RefCounted
	{
	/* Embedded classes: */
	public:
	enum AccessMode // Enumerated type for file access modes
		{
		NoAccess,ReadOnly,WriteOnly,ReadWrite
		};
	
	class Error:public std::runtime_error // Base exception class for file-related errors
		{
		/* Constructors and destructors: */
		public:
		Error(const char* msg)
			:std::runtime_error(msg)
			{
			}
		};
	
	class OpenError:public Error // Exception class to report file opening errors; typically extended by derived classes
		{
		/* Constructors and destructors: */
		public:
		OpenError(const char* message);
		};
	
	class ReadError:public Error // Exception class to report file reading errors
		{
		/* Elements: */
		public:
		size_t numMissingBytes; // Number of bytes that could not be read in failed call
		
		/* Constructors and destructors: */
		public:
		ReadError(size_t sNumMissingBytes);
		};
	
	class UngetCharError:public Error // Exception class to report errors while putting characters back into the source
		{
		/* Constructors and destructors: */
		public:
		UngetCharError(void);
		};
	
	class WriteError:public Error // Exception class to report file writing errors
		{
		/* Elements: */
		public:
		size_t numMissingBytes; // Number of bytes that could not be written in failed call
		
		/* Constructors and destructors: */
		public:
		WriteError(size_t sNumMissingBytes);
		};
	
	protected:
	typedef unsigned char Byte; // Type for 8-bit raw data stored in buffers and read/written from sources/sinks
	
	/* Elements: */
	private:
	size_t readBufferSize; // Size of read buffer in bytes
	Byte* readBuffer; // Pointer to read buffer
	Byte* readDataEnd; // Pointer to end of data currently in buffer
	bool haveEof; // Flag whether the source signaled end-of-source during the last readData call
	Byte* readPtr; // Pointer to next byte available for reading
	protected:
	bool canReadThrough; // Flag whether the concrete implementation supports out-of-buffer reads
	bool readMustSwapEndianness; // Flag if data has to be endianness-swapped after reading
	
	private:
	size_t writeBufferSize; // Size of write buffer in bytes
	Byte* writeBuffer; // Pointer to write buffer
	Byte* writeBufferEnd; // Pointer to end of write buffer
	Byte* writePtr; // Pointer to next byte available for writing
	protected:
	bool canWriteThrough; // Flag whether the concrete implementation supports out-of-buffer writes
	bool writeMustSwapEndianness; // Flag if data has to be endianness-swapped before writing
	
	/* Protected methods: */
	protected:
	static AccessMode disableRead(AccessMode accessMode); // Disables reading in the given access mode
	static AccessMode disableWrite(AccessMode accessMode); // Disables writing in the given access mode
	static const char* getAccessModeName(AccessMode accessMode); // Returns a string describing the given access mode
	void flushReadBuffer(void) // Clears the read buffer so that the next read access has to go to the data source
		{
		/* Reset the read buffer pointers: */
		readDataEnd=readBuffer;
		readPtr=readBuffer;
		}
	void setReadBuffer(size_t newReadBufferSize,Byte* newReadBuffer,bool deleteOldBuffer =true); // Allows derived class to set a new read buffer while deleting or releasing the previous buffer; discards unread data in read buffer
	size_t getReadBufferDataSize(void) const // Returns current amount of data in the read buffer
		{
		return readDataEnd-readBuffer;
		}
	void appendReadBufferData(size_t newDataSize) // Adds to the amount of readable data in the read buffer; does not check for buffer bounds
		{
		readDataEnd+=newDataSize;
		}
	ptrdiff_t getReadPtr(void) const // Returns the current position of the read pointer inside the read buffer
		{
		return readPtr-readBuffer;
		}
	void setReadPtr(ptrdiff_t newReadPos) // Sets the current position of the read pointer; does not check for buffer bounds
		{
		readPtr=readBuffer+newReadPos;
		}
	void setWriteBuffer(size_t newWriteBufferSize,Byte* newWriteBuffer,bool deleteOldBuffer =true); // Allows derived class to set a new write buffer while deleting or releasing the previous buffer; discards unwritten data in write buffer
	ptrdiff_t getWritePtr(void) const // Returns the current position of the write pointer inside the write buffer
		{
		return writePtr-writeBuffer;
		}
	void setWritePtr(ptrdiff_t newWritePos) // Sets the current position of the write pointer; does not check for buffer bounds
		{
		writePtr=writeBuffer+newWritePos;
		}
	
	/* Protected low-level methods to be implemented by concrete derived classes; default implementations return EOF or throw error: */
	virtual size_t readData(Byte* buffer,size_t bufferSize); // Method to read data into the given buffer; must block until at least one byte is read; returns number of bytes read; zero return value signals end-of-source condition
	virtual void writeData(const Byte* buffer,size_t bufferSize); // Method to write all data contained in the write buffer to a sink; should throw appropriate exception in case of errors
	
	/* Private methods: */
	private:
	void fillReadBuffer(void) // Reads more data from the source, and updates the read buffer's state
		{
		size_t readSize=readData(readBuffer,readBufferSize);
		readDataEnd=readBuffer+readSize;
		haveEof=readDataEnd==readBuffer;
		readPtr=readBuffer;
		}
	void bufferedRead(void* buffer,size_t bufferSize); // Reads exactly given amount of data into the given buffer
	void bufferedSkip(size_t skipSize); // Skips exactly given amount of data from the read data stream
	void bufferedWrite(const void* buffer,size_t bufferSize); // Writes exactly given amount of data from the given buffer
	
	/* Constructors and destructors: */
	public:
	File(void); // Creates a buffer-less file
	File(AccessMode sAccessMode); // Creates a buffered file for the given access mode with "DontCare" endianness
	private:
	File(const File& source); // Prohibit copy constructor
	File& operator=(const File& source); // Prohibit assignment operator
	public:
	virtual ~File(void); // Destroys buffers, but does not flush
	
	/* Methods: */
	virtual int getFd(void) const; // Returns the concrete file's OS file descriptor, if applicable
	virtual size_t getReadBufferSize(void) const; // Returns the (nominal) size of the read buffer in bytes
	virtual size_t getWriteBufferSize(void) const; // Returns the (nominal) size of the write buffer in bytes
	virtual size_t resizeReadBuffer(size_t newReadBufferSize); // Resizes the read buffer; increases given size if unread data in buffer would not fit into new buffer; returns actual read buffer size
	virtual void resizeWriteBuffer(size_t newWriteBufferSize); // Flushes and resizes the write buffer
	bool canReadImmediately(void) const // Returns true if there is unread data in the read buffer
		{
		return readPtr!=readDataEnd;
		}
	size_t getUnreadDataSize(void) const // Returns the amount of unread data in the read buffer
		{
		return readDataEnd-readPtr;
		}
	bool canWriteImmediately(void) const // Returns true if there is room in the write buffer
		{
		return writePtr!=writeBufferEnd;
		}
	size_t getWriteBufferSpace(void) const // Returns amount of space left in write buffer
		{
		return writeBufferEnd-writePtr;
		}
	bool eof(void) // Returns true if all available data has been read; might block until more data becomes available to read
		{
		/* Check for unread data first: */
		if(readPtr!=readDataEnd)
			return false;
		
		/* Check if end-of-file was already seen: */
		if(haveEof)
			return true;
		
		/* Try reading more data: */
		fillReadBuffer();
		return haveEof&&readPtr==readDataEnd;
		}
	int getChar(void) // Returns the next character from the input file, or EOF (-1) if the entire file has been read
		{
		/* Check if there is data left in the read buffer: */
		if(readPtr!=readDataEnd)
			return int(*(readPtr++));
		
		/* Check if end-of-file was already seen: */
		if(haveEof)
			return -1;
		
		/* Try reading more data: */
		fillReadBuffer();
		
		/* Try returning data again: */
		if(readPtr!=readDataEnd)
			return int(*(readPtr++));
		else
			return -1;
		}
	void ungetChar(int character) // Puts the given character back into the read buffer; throws exception if there is no room
		{
		/* Check if the unget buffer is full: */
		if(readPtr==readBuffer)
			throw UngetCharError();
		
		/* Put the character back into the read buffer: */
		*(--readPtr)=Byte(character);
		}
	size_t readUpTo(void* buffer,size_t bufferSize) // Reads up to the given amount of data into the provided buffer; returns amount of data read; returns zero when called at end-of-file
		{
		/* Read more data if the buffer is empty: */
		if(readPtr==readDataEnd)
			fillReadBuffer();
		
		/* Copy whatever is in the buffer, up to the provided size: */
		size_t copySize=readDataEnd-readPtr;
		if(copySize>bufferSize)
			copySize=bufferSize;
		memcpy(buffer,readPtr,copySize);
		
		/* Advance the read position: */
		readPtr+=copySize;
		
		return copySize;
		}
	size_t readInBuffer(void*& buffer,size_t bufferSize =~size_t(0)) // Ditto, but returns a pointer into the file's internal buffer, which will stay valid until the next read
		{
		/* Read more data if the buffer is empty: */
		if(readPtr==readDataEnd)
			fillReadBuffer();
		
		/* Return any unread buffer content, up to the provided size: */
		size_t result=readDataEnd-readPtr;
		if(result>bufferSize)
			result=bufferSize;
		buffer=readPtr;
		readPtr+=result;
		
		return result;
		}
	void readRaw(void* buffer,size_t bufferSize) // Reads exactly the given amount of data into the provided buffer; blocks until read complete
		{
		/* Check if there is enough data in the read buffer: */
		if(bufferSize<=size_t(readDataEnd-readPtr))
			{
			/* Copy data into the provided buffer: */
			memcpy(buffer,readPtr,bufferSize);
			
			/* Advance the read pointer: */
			readPtr+=bufferSize;
			}
		else
			{
			/* Use the buffered reading method: */
			bufferedRead(buffer,bufferSize);
			}
		}
	void putChar(int character) // Writes a single character to the file
		{
		/* Check if the write buffer is full: */
		if(writePtr==writeBufferEnd)
			{
			/* Write the full write buffer and reset the write pointer: */
			writeData(writeBuffer,writeBufferSize);
			writePtr=writeBuffer;
			}
		
		/* Append the character to the write buffer: */
		*writePtr=Byte(character);
		++writePtr;
		}
	size_t writeInBufferPrepare(void*& buffer) // Prepares to write data directly into the write buffer
		{
		/* Check if the write buffer is full: */
		if(writePtr==writeBufferEnd)
			{
			/* Write the full write buffer and reset the write pointer: */
			writeData(writeBuffer,writeBufferSize);
			writePtr=writeBuffer;
			}
		
		/* Return the buffer pointer and amount of space left in buffer: */
		buffer=writePtr;
		return writeBufferEnd-writePtr;
		}
	void writeInBufferFinish(size_t writeSize) // Finishes writing data directly into the write buffer by providing the exact amount of data written
		{
		/* Update the write pointer to the end of the written data: */
		writePtr+=writeSize;
		}
	void writeRaw(const void* buffer,size_t bufferSize) // Writes exactly the given amount of data from the provided buffer; blocks until write complete
		{
		/* Check if there is enough room in the write buffer: */
		if(bufferSize<=size_t(writeBufferEnd-writePtr))
			{
			/* Copy data from the provided buffer: */
			memcpy(writePtr,buffer,bufferSize);
			
			/* Advance the write pointer: */
			writePtr+=bufferSize;
			}
		else
			{
			/* Use the buffered writing method: */
			bufferedWrite(buffer,bufferSize);
			}
		}
	void flush(void) // Flushes the write buffer to the data sink
		{
		/* Write the entire write buffer if there is any data in it: */
		if(writePtr!=writeBuffer)
			writeData(writeBuffer,writePtr-writeBuffer);
		
		/* Reset the write buffer: */
		writePtr=writeBuffer;
		}
	void setEndianness(Misc::Endianness newEndianness); // Sets the endianness of the source and/or sink
	
	/* Endianness-safe binary read interface: */
	bool mustSwapOnRead(void) // Returns true if the file must endianness-swap data on read
		{
		return readMustSwapEndianness;
		}
	void setSwapOnRead(bool newSwapOnRead); // Enables or disables endianness swapping after reading
	template <class DataParam>
	DataParam read(void) // Reads single value
		{
		DataParam result;
		if(sizeof(DataParam)<=size_t(readDataEnd-readPtr))
			{
			memcpy(&result,readPtr,sizeof(DataParam));
			readPtr+=sizeof(DataParam);
			}
		else
			bufferedRead(&result,sizeof(DataParam));
		if(readMustSwapEndianness)
			Misc::swapEndianness(result);
		return result;
		}
	template <class DataParam>
	DataParam& read(DataParam& data) // Reads single value through reference
		{
		if(sizeof(DataParam)<=size_t(readDataEnd-readPtr))
			{
			memcpy(&data,readPtr,sizeof(DataParam));
			readPtr+=sizeof(DataParam);
			}
		else
			bufferedRead(&data,sizeof(DataParam));
		if(readMustSwapEndianness)
			Misc::swapEndianness(data);
		return data;
		}
	template <class DataParam>
	void read(DataParam* data,size_t numItems) // Reads array of values
		{
		if(numItems*sizeof(DataParam)<=size_t(readDataEnd-readPtr))
			{
			memcpy(data,readPtr,numItems*sizeof(DataParam));
			readPtr+=numItems*sizeof(DataParam);
			}
		else
			bufferedRead(data,numItems*sizeof(DataParam));
		if(readMustSwapEndianness)
			Misc::swapEndianness(data,numItems);
		}
	template <class DataParam>
	void skip(size_t numItems) // Skips array of values
		{
		if(numItems*sizeof(DataParam)<=size_t(readDataEnd-readPtr))
			readPtr+=numItems*sizeof(DataParam);
		else
			bufferedSkip(numItems*sizeof(DataParam));
		}
	
	/* Endianness-safe binary write interface: */
	bool mustSwapOnWrite(void) // Returns true if the file must endianness-swap data on write
		{
		return writeMustSwapEndianness;
		}
	void setSwapOnWrite(bool newSwapOnWrite); // Enables or disables endianness swapping before writing
	template <class DataParam>
	void write(const DataParam& data) // Writes single value
		{
		if(writeMustSwapEndianness)
			{
			DataParam temp=data;
			Misc::swapEndianness(temp);
			if(sizeof(DataParam)<=size_t(writeBufferEnd-writePtr))
				{
				memcpy(writePtr,&temp,sizeof(DataParam));
				writePtr+=sizeof(DataParam);
				}
			else
				bufferedWrite(&temp,sizeof(DataParam));
			}
		else
			{
			if(sizeof(DataParam)<=size_t(writeBufferEnd-writePtr))
				{
				memcpy(writePtr,&data,sizeof(DataParam));
				writePtr+=sizeof(DataParam);
				}
			else
				bufferedWrite(&data,sizeof(DataParam));
			}
		}
	template <class DataParam>
	void write(const DataParam* data,size_t numItems) // Writes array of values
		{
		if(writeMustSwapEndianness)
			{
			for(size_t i=0;i<numItems;++i)
				{
				DataParam temp=data[i];
				Misc::swapEndianness(temp);
				if(sizeof(DataParam)<=size_t(writeBufferEnd-writePtr))
					{
					memcpy(writePtr,&temp,sizeof(DataParam));
					writePtr+=sizeof(DataParam);
					}
				else
					bufferedWrite(&temp,sizeof(DataParam));
				}
			}
		else
			{
			if(numItems*sizeof(DataParam)<=size_t(writeBufferEnd-writePtr))
				{
				memcpy(writePtr,data,numItems*sizeof(DataParam));
				writePtr+=numItems*sizeof(DataParam);
				}
			else
				bufferedWrite(data,numItems*sizeof(DataParam));
			}
		}
	};

typedef Misc::Autopointer<File> FilePtr; // Type for pointers to reference-counted file objects

}

#endif
