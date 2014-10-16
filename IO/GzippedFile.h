/***********************************************************************
GzippedFile - Class for high-performance reading from gzip-compressed
standard operating system files.
Copyright (c) 2011-2012 Oliver Kreylos

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

#ifndef IO_GZIPPEDFILE_INCLUDED
#define IO_GZIPPEDFILE_INCLUDED

#include <zlib.h>
#include <IO/File.h>

namespace IO {

class GzippedFile:public File
	{
	/* Elements: */
	private:
	gzFile inputFile; // File descriptor of gzipped input file
	
	/* Protected methods from File: */
	protected:
	virtual size_t readData(Byte* buffer,size_t bufferSize);
	
	/* Constructors and destructors: */
	public:
	GzippedFile(const char* inputFileName); // Opens the given compressed input file
	virtual ~GzippedFile(void); // Closes the file
	};

}

#endif
