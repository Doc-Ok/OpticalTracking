/***********************************************************************
MemMappedFile - Class for read/write access to memory-mapped files using
the File abstraction; mostly for simplified resource management.
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

#ifndef IO_MEMMAPPEDFILE_INCLUDED
#define IO_MEMMAPPEDFILE_INCLUDED

#include <sys/types.h>
#include <IO/SeekableFile.h>

namespace IO {

class MemMappedFile:public SeekableFile
	{
	/* Elements: */
	private:
	void* memBase; // Base address of file's memory space
	size_t memSize; // Size of file's memory space
	
	/* Private methods: */
	void openFile(const char* fileName,AccessMode accessMode,int flags,int mode); // Opens and memory-maps a file and handles errors
	
	/* Constructors and destructors: */
	public:
	MemMappedFile(const char* fileName,AccessMode accessMode = ReadOnly); // Opens a standard file with "DontCare" endianness setting and default flags and permissions
	MemMappedFile(const char* fileName,AccessMode accessMode,int flags,int mode =0); // Opens a standard file with "DontCare" endianness setting
	virtual ~MemMappedFile(void);
	
	/* Methods from File: */
	virtual size_t resizeReadBuffer(size_t newReadBufferSize);
	virtual void resizeWriteBuffer(size_t newWriteBufferSize);
	
	/* Methods from SeekableFile: */
	virtual Offset getSize(void) const;
	
	/* New methods: */
	const void* getMemory(void) const // Returns a pointer to the file's memory map
		{
		return memBase;
		}
	void* getMemory(void) // Ditto
		{
		return memBase;
		}
	};

}

#endif
