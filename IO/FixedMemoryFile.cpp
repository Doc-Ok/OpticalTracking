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

#include <IO/FixedMemoryFile.h>

namespace IO {

/********************************
Methods of class FixedMemoryFile:
********************************/

FixedMemoryFile::FixedMemoryFile(size_t sMemSize)
	:SeekableFile(),
	 memSize(sMemSize),memBlock(new Byte[memSize])
	{
	/* Re-allocate the buffered file's buffers: */
	setReadBuffer(memSize,memBlock,false);
	canReadThrough=false;
	setWriteBuffer(memSize,memBlock,false);
	canWriteThrough=false;
	
	/* Assume that the memory block will be filled by the caller before any data is read: */
	appendReadBufferData(memSize);
	readPos=memSize;
	}

FixedMemoryFile::~FixedMemoryFile(void)
	{
	/* Release the buffered file's buffers: */
	setReadBuffer(0,0,false);
	setWriteBuffer(0,0,false);
	
	/* Delete the memory block: */
	delete[] memBlock;
	}

size_t FixedMemoryFile::resizeReadBuffer(size_t newReadBufferSize)
	{
	/* Ignore it and return the full memory size: */
	return memSize;
	}

void FixedMemoryFile::resizeWriteBuffer(size_t newWriteBufferSize)
	{
	/* Ignore it */
	}

SeekableFile::Offset FixedMemoryFile::getSize(void) const
	{
	/* Return the file size: */
	return memSize;
	}

}
