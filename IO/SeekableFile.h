/***********************************************************************
SeekableFile - Base class for files whose read and write positions can
be changed independently.
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

#ifndef IO_SEEKABLEFILE_INCLUDED
#define IO_SEEKABLEFILE_INCLUDED

#include <sys/types.h>
#include <Misc/Autopointer.h>
#include <IO/File.h>

namespace IO {

class SeekableFile:public File
	{
	/* Embedded classes: */
	public:
	#ifdef __APPLE__
	typedef off_t Offset; // Type for 64-bit file offsets
	#else
	typedef off64_t Offset; // Type for 64-bit file offsets
	#endif
	
	class SeekError:public Error // Exception class to report file seeking errors
		{
		/* Elements: */
		public:
		Offset failedOffset; // Offset that caused seek to fail
		
		/* Constructors and destructors: */
		SeekError(Offset sFailedOffset);
		};
	
	/* Elements: */
	protected:
	Offset readPos; // Absolute file position for next read access (end of data in read buffer)
	Offset writePos; // Absolute file position for next write access (beginning of write buffer)
	
	/* Constructors and destructors: */
	public:
	SeekableFile(void)
		:File(),
		 readPos(0),writePos(0)
		{
		}
	SeekableFile(AccessMode sAccessMode)
		:File(sAccessMode),
		 readPos(0),writePos(0)
		{
		}
	
	/* New methods: */
	virtual Offset getSize(void) const =0; // Returns the absolute position of the end of the file's data
	
	/* Methods to get/set the current read position: */
	Offset getReadPos(void) const // Returns the absolute position of the read pointer
		{
		return readPos-getUnreadDataSize();
		}
	void setReadPosAbs(Offset newOffset); // Sets the read position relative to the beginning of the file
	void setReadPosRel(Offset newOffset) // Sets the read position relative to the current read position
		{
		setReadPosAbs(newOffset+readPos-getUnreadDataSize());
		}
	
	/* Methods to get/set the current write position: */
	Offset getWritePos(void) const // Returns the absolute position of the write pointer
		{
		return writePos+getWritePtr();
		}
	void setWritePosAbs(Offset newOffset); // Sets the write position relative to the beginning of the file
	void setWritePosRel(Offset newOffset) // Sets the write position relative to the current write position
		{
		setWritePosAbs(newOffset+writePos+getWritePtr());
		}
	};

typedef Misc::Autopointer<SeekableFile> SeekableFilePtr; // Type for pointers to reference-counted seekable file objects

}

#endif
