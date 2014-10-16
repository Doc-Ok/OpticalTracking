/***********************************************************************
StandardFile - Class for high-performance reading/writing from/to
standard operating system files.
Copyright (c) 2010-2013 Oliver Kreylos

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

#ifndef IO_STANDARDFILE_INCLUDED
#define IO_STANDARDFILE_INCLUDED

#include <IO/SeekableFile.h>

namespace IO {

class StandardFile:public SeekableFile
	{
	/* Elements: */
	private:
	int fd; // File descriptor of the underlying file
	Offset filePos; // Current position of the underlying file's read/write pointer
	
	/* Protected methods from File: */
	protected:
	virtual size_t readData(Byte* buffer,size_t bufferSize);
	virtual void writeData(const Byte* buffer,size_t bufferSize);
	
	/* Private methods: */
	void openFile(const char* fileName,AccessMode accessMode,int flags,int mode); // Opens a file and handles errors
	
	/* Constructors and destructors: */
	public:
	StandardFile(const char* fileName,AccessMode accessMode =ReadOnly); // Opens a standard file with "DontCare" endianness setting and default flags and permissions
	StandardFile(const char* fileName,AccessMode accessMode,int flags,int mode =0); // Opens a standard file with "DontCare" endianness setting
	StandardFile(int sFd,AccessMode accessMode); // Creates standard file for already opened file descriptor; inherits (does not duplicate) the given file descriptor
	virtual ~StandardFile(void);
	
	/* Methods from File: */
	virtual int getFd(void) const;
	
	/* Methods from SeekableFile: */
	virtual Offset getSize(void) const;
	};

}

#endif
