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

#ifndef IO_IFFCHUNK_INCLUDED
#define IO_IFFCHUNK_INCLUDED

#include <IO/File.h>

namespace IO {

class IFFChunk:public File
	{
	/* Elements: */
	private:
	FilePtr source; // The data source from which the chunk header and data are read
	char chunkId[5]; // NUL-terminated four-character chunk ID
	size_t chunkSize; // Size of this chunk in bytes without padding
	size_t chunkSizeLeft; // Number of unread bytes in this chunk
	
	/* Protected methods from IO::File: */
	virtual size_t readData(Byte* buffer,size_t bufferSize);
	
	/* Constructors and destructors: */
	public:
	IFFChunk(FilePtr sSource,bool subChunk =false); // Creates a chunk from the given data source; if subChunk flag is true, chunkSize is only 2 bytes long
	~IFFChunk(void); // Closes the chunk, skipping all unread data and padding
	
	/* Methods from IO::File: */
	virtual size_t getReadBufferSize(void) const;
	virtual size_t resizeReadBuffer(size_t newReadBufferSize);
	
	/* New methods: */
	const char* getChunkId(void) const // Returns the NUL-terminated chunk ID
		{
		return chunkId;
		}
	bool isChunk(const char* testChunkId) const // Returns true if the chunk ID matches the given ID
		{
		return chunkId[0]==testChunkId[0]&&chunkId[1]==testChunkId[1]&&chunkId[2]==testChunkId[2]&&chunkId[3]==testChunkId[3];
		}
	size_t getChunkSize(void) const // Returns the unpadded chunk size
		{
		return chunkSize;
		}
	size_t getChunkSizeLeft(void) const // Returns the number of bytes left to read in this chunk
		{
		return getReadBufferDataSize()+chunkSizeLeft;
		}
	};

typedef Misc::Autopointer<IFFChunk> IFFChunkPtr;

}

#endif
