/***********************************************************************
FixedMemoryFile - Class to read/write from/to fixed-size memory blocks
using a File abstraction.
Copyright (c) 2011-2013 Oliver Kreylos

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

#ifndef IO_FIXEDMEMORYFILE_INCLUDED
#define IO_FIXEDMEMORYFILE_INCLUDED

#include <sys/types.h>
#include <IO/SeekableFile.h>

namespace IO {

class FixedMemoryFile:public SeekableFile
	{
	/* Elements: */
	private:
	size_t memSize; // Size of file's memory block
	Byte* memBlock; // Pointer to file's memory block
	
	/* Constructors and destructors: */
	public:
	FixedMemoryFile(size_t sMemSize); // Creates a memory block of the given size
	virtual ~FixedMemoryFile(void);
	
	/* Methods from File: */
	virtual size_t resizeReadBuffer(size_t newReadBufferSize);
	virtual void resizeWriteBuffer(size_t newWriteBufferSize);
	
	/* Methods from SeekableFile: */
	virtual Offset getSize(void) const;
	
	/* New methods: */
	const void* getMemory(void) const // Returns a pointer to the file's memory block
		{
		return memBlock;
		}
	void* getMemory(void) // Ditto
		{
		return memBlock;
		}
	size_t getWriteSize(void) const // Returns the amount of data that has been written into the memory block
		{
		return size_t(getWritePtr());
		}
	};

}

#endif
