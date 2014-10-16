/***********************************************************************
VariableMemoryFile - Class to write to variable-sized in-memory files as
temporary file storage.
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

#ifndef IO_VARIABLEMEMORYFILE_INCLUDED
#define IO_VARIABLEMEMORYFILE_INCLUDED

#include <IO/File.h>

namespace IO {

class VariableMemoryFile:public File
	{
	/* Embedded classes: */
	private:
	struct BufferHeader // Header structure prepended to each in-memory buffer
		{
		/* Elements: */
		public:
		BufferHeader* succ; // Pointer to next buffer in list
		size_t size; // Amount of data in the buffer
		
		/* Constructors and destructors: */
		BufferHeader(void)
			:succ(0),size(0)
			{
			}
		};
	
	public:
	class BufferChain // Class to represent a chain of in-memory buffers from a buffered memory file
		{
		friend class VariableMemoryFile;
		
		/* Elements: */
		private:
		BufferHeader* head; // First buffer in the chain
		
		/* Constructors and destructors: */
		public:
		BufferChain(void) // Creates an empty buffer chain
			:head(0)
			{
			}
		~BufferChain(void); // Destroys a buffer chain
		
		/* Methods: */
		size_t getDataSize(void) const; // Returns the total size of data stored in the buffer chain
		template <class SinkParam>
		void writeToSink(SinkParam& sink) const // Writes all data in the buffer chain to the given sink
			{
			/* Write all buffers to the sink: */
			for(const BufferHeader* bhPtr=head;bhPtr!=0;bhPtr=bhPtr->succ)
				sink.writeRaw(bhPtr+1,bhPtr->size); // bhPtr+1 points to actual data in buffer
			}
		};
	
	/* Elements: */
	private:
	size_t writeBufferSize; // Size for in-memory buffers
	BufferHeader* head; // Pointer to the first filled in-memory buffer
	BufferHeader* tail; // Pointer to last buffer in list
	BufferHeader* current; // Pointer to the buffer currently being filled by the base class
	
	/* Protected methods from File: */
	virtual void writeData(const Byte* buffer,size_t bufferSize);
	
	/* Constructors and destructors: */
	public:
	VariableMemoryFile(size_t sWriteBufferSize =8192-sizeof(BufferHeader)); // Creates a new temporary file with the given buffer size
	~VariableMemoryFile(void); // Destroys the buffered file
	
	/* Methods from File: */
	virtual size_t getWriteBufferSize(void) const;
	virtual void resizeWriteBuffer(size_t newWriteBufferSize);
	
	/* New methods: */
	size_t getDataSize(void) const; // Returns the total size of data currently in the file
	void storeBuffers(BufferChain& chain); // Stores all data currently in the file in a chain of buffers
	template <class SinkParam>
	void writeToSink(SinkParam& sink) const // Writes all data currently in the file to the given sink
		{
		/* Write all filled buffers to the sink: */
		for(const BufferHeader* bhPtr=head;bhPtr!=0;bhPtr=bhPtr->succ)
			sink.writeRaw(bhPtr+1,bhPtr->size); // bhPtr+1 points to actual data in buffer
		
		/* Write all data in the current write buffer to the sink: */
		sink.writeRaw(current+1,getWritePtr());
		}
	void clear(void); // Deletes all data in the file
	};

}

#endif
