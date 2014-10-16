/***********************************************************************
GzipFilter - Class for read/write access to gzip-compressed files using
a IO::File abstraction.
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

#ifndef IO_GZIPFILTER_INCLUDED
#define IO_GZIPFILTER_INCLUDED

#include <zlib.h>
#include <IO/File.h>

namespace IO {

class GzipFilter:public IO::File
	{
	/* Elements: */
	private:
	FilePtr gzippedFile; // Underlying gzip-compressed file
	z_stream stream; // Zlib compression/decompression structure
	bool readEof; // Flag if the zlib decompressor has signaled end-of-file
	
	/* Methods from File: */
	protected:
	virtual size_t readData(Byte* buffer,size_t bufferSize);
	virtual void writeData(const Byte* buffer,size_t bufferSize);
	
	/* Private methods: */
	private:
	void init(void); // Initializes the compressor/decompressor
	
	/* Constructors and destructors: */
	public:
	GzipFilter(FilePtr sGzippedFile); // Creates a gzip filter for the given underlying gzip-compressed file; inherits access mode from compressed file
	GzipFilter(const char* gzippedFileName,File::AccessMode sAccessMode); // Opens the gzip-compressed file of the given name with the given access mode
	virtual ~GzipFilter(void); // Destroys the gzip filter
	};

}

#endif
