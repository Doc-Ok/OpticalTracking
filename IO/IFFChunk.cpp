/***********************************************************************
IFFChunk - Class to represent component chunks of IFF (Interchange File
Format) as read-only File abstractions.
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

#include <IO/IFFChunk.h>

#include <Misc/SizedTypes.h>
#include <Misc/ThrowStdErr.h>

namespace IO {

/*************************
Methods of class IFFChunk:
*************************/

size_t IFFChunk::readData(File::Byte* buffer,size_t bufferSize)
	{
	/* Check for end-of-chunk: */
	if(chunkSizeLeft==0)
		return 0;
	
	/* Read at most as much data as is left in the chunk: */
	void* resultBuffer;
	size_t result=source->readInBuffer(resultBuffer,chunkSizeLeft);
	
	/* Install a fake read buffer pointing into the source's read buffer: */
	setReadBuffer(result,static_cast<Byte*>(resultBuffer),false);
	
	/* Reduce the left-over chunk size and return the amount of read data: */
	chunkSizeLeft-=result;
	return result;
	}

IFFChunk::IFFChunk(FilePtr sSource,bool subChunk)
	:File(),
	 source(sSource)
	{
	/* Read the chunk ID: */
	source->read(chunkId,4);
	chunkId[4]='\0';
	
	/* Guess the endianness of the source based on the chunk type: */
	if(isChunk("FORM"))
		source->setEndianness(Misc::BigEndian);
	else if(isChunk("RIFF"))
		source->setEndianness(Misc::LittleEndian);
	
	/* Read the chunk size: */
	chunkSize=subChunk?source->read<Misc::UInt16>():source->read<Misc::UInt32>();
	
	/* Initialize the leftover data size: */
	chunkSizeLeft=chunkSize;
	
	/* Copy the source's endianness setting: */
	setSwapOnRead(source->mustSwapOnRead());
	
	/* Disable read-through: */
	canReadThrough=false;
	}

IFFChunk::~IFFChunk(void)
	{
	/* Skip any unread chunk data: */
	source->skip<Misc::UInt8>(chunkSizeLeft);
	if((chunkSize&0x1U)&&!source->eof())
		source->skip<Misc::UInt8>(1);
	
	/* Release the read buffer: */
	setReadBuffer(0,0,false);
	}

size_t IFFChunk::getReadBufferSize(void) const
	{
	/* Return the source's read buffer size, since we're sharing it: */
	return source->getReadBufferSize();
	}

size_t IFFChunk::resizeReadBuffer(size_t newReadBufferSize)
	{
	/* Ignore the request and return the source's read buffer size, since we're sharing it: */
	return source->getReadBufferSize();
	}

}
