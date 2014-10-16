/***********************************************************************
SeekableFilter - Class to convert a streaming file into a seekable
file by caching its contents in memory.
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

#ifndef IO_SEEKABLEFILTER_INCLUDED
#define IO_SEEKABLEFILTER_INCLUDED

#include <IO/File.h>
#include <IO/SeekableFile.h>

namespace IO {

class SeekableFilter:public SeekableFile
	{
	/* Embedded classes: */
	private:
	struct BufferHeader // Header structure prepended to each in-memory buffer
		{
		/* Elements: */
		public:
		BufferHeader* succ; // Pointer to next buffer in list
		size_t size; // Amount of data in the buffer
		
		/* Constructors and destructors: */
		BufferHeader(void)
			:succ(0),size(0)
			{
			}
		};
	
	/* Elements: */
	private:
	FilePtr source; // The streaming source file
	size_t memBufferSize; // Size for in-memory buffers
	BufferHeader* head; // Pointer to the first filled in-memory buffer
	BufferHeader* tail; // Pointer to last buffer in list
	Offset totalReadSize; // Total amount of data already read from source file
	Offset filePos; // Current read position in buffer chain
	BufferHeader* current; // Buffer containing the current read position
	Offset currentPos; // Offset of read position within current buffer
	
	/* Protected methods from File: */
	virtual size_t readData(Byte* buffer,size_t bufferSize);
	
	/* Private methods: */
	void readFromSource(void); // Reads a chunk of data from the source file
	
	/* Constructors and destructors: */
	public:
	SeekableFilter(FilePtr sSource,size_t sMemBufferSize =8192-sizeof(BufferHeader)); // Creates an adapter for the given file with the given buffer size
	~SeekableFilter(void); // Destroys the adapter
	
	/* Methods from File: */
	virtual size_t getReadBufferSize(void) const;
	virtual size_t resizeReadBuffer(size_t newReadBufferSize);
	virtual void resizeWriteBuffer(size_t newWriteBufferSize);
	
	/* Methods from SeekableFile: */
	virtual Offset getSize(void) const;
	};

}

#endif
