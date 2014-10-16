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

#include <IO/SeekableFile.h>

#include <Misc/ThrowStdErr.h>

namespace IO {

/****************************************
Methods of class SeekableFile::SeekError:
****************************************/

SeekableFile::SeekError::SeekError(SeekableFile::Offset sFailedOffset)
	:Error(Misc::printStdErrMsg("IO::SeekableFile: Failed seek to position %ld",(long int)sFailedOffset)),
	 failedOffset(sFailedOffset)
	{
	}

/*****************************
Methods of class SeekableFile:
*****************************/

void SeekableFile::setReadPosAbs(SeekableFile::Offset newOffset)
	{
	if(newOffset<0)
		throw SeekError(newOffset);
	
	/* Check if the new read position is inside the current read buffer: */
	Offset bufferStart=readPos-getReadBufferDataSize();
	if(newOffset>=bufferStart&&newOffset<=readPos)
		{
		/* Seek inside the current buffer: */
		setReadPtr(newOffset-bufferStart);
		}
	else
		{
		/* Flush the read buffer and reposition the read pointer: */
		flushReadBuffer();
		readPos=newOffset;
		}
	}

void SeekableFile::setWritePosAbs(SeekableFile::Offset newOffset)
	{
	if(newOffset<0)
		throw SeekError(newOffset);
	
	/* Check if the new write position is inside the current write buffer: */
	if(newOffset>=writePos&&newOffset<=writePos+getWritePtr())
		{
		/* Seek inside the current buffer: */
		setWritePtr(newOffset-writePos);
		}
	else
		{
		/* Flush the write buffer and reposition the write pointer: */
		flush();
		writePos=newOffset;
		}
	}

}
