/***********************************************************************
ChunkedQueue - Data structure to store queues of elements in page-
aligned chunks of memory.
Copyright (c) 2005 Oliver Kreylos

This file is part of the Miscellaneous Support Library (Misc).

The Miscellaneous Support Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Miscellaneous Support Library is distributed in the hope that it
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Miscellaneous Support Library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef MISC_CHUNKEDQUEUE_INCLUDED
#define MISC_CHUNKEDQUEUE_INCLUDED

#include <new>

namespace Misc {

template <class ContentParam,size_t pageSizeParam =8192>
class ChunkedQueue
	{
	/* Embedded classes: */
	public:
	typedef ContentParam Content; // Type of array contents
	typedef size_t size_type; // Type for sizes
	static const size_t pageSize=pageSizeParam; // Size of memory page in bytes
	
	private:
	struct Chunk;
	
	struct ChunkHeader // Structure for array chunk headers
		{
		/* Elements: */
		public:
		Chunk* succ; // Pointer to next chunk in list
		size_type numElements; // Number of array elements in chunk
		
		/* Constructors and destructors: */
		ChunkHeader(void)
			:succ(0),numElements(0)
			{
			}
		}
	
	static const size_type chunkSize=(pageSize-sizeof(ChunkHeader))/sizeof(Content); // Number of array elements per chunk
	
	struct Chunk // Structure for array chunks
		{
		/* Elements: */
		public:
		ChunkHeader header; // Chunk header
		char mem[pageSize-sizeof(ChunkHeader)];
		
		/* Methods: */
		const Content& operator[](size_type index) const
			{
			return reinterpret_cast<const Content*>(mem)[index];
			}
		Content& operator[](size_type index)
			{
			return reinterpret_cast<Content*>(mem)[index];
			}
		const Content* getAddress(size_type index) const
			{
			return reinterpret_cast<const Content*>(mem)+index;
			}
		Content* getAddress(size_type index)
			{
			return reinterpret_cast<Content*>(mem)+index;
			}
		}
	
	/* Elements: */
	private:
	Chunk* firstChunk; // Pointer to first chunk in list
	size_type frontIndex; // Index of first queue element in first chunk
	Chunk* lastChunk; // Pointer to last chunk in list
	
	/* Constructors and destructors: */
	public:
	ChunkedQueue(void) // Creates empty queue
		:firstChunk(0),frontIndex(0),lastChunk(0)
		{
		}
	private:
	ChunkedQueue(const ChunkedQueue& source); // Prohibit copy constructor
	ChunkedQueue& operator=(const ChunkedQueue& source); // Prohibit assignment operator
	public:
	~ChunkedQueue(void) // Destroys array
		{
		clear();
		}
	
	/* Methods: */
	bool empty(void) const // Returns true if chunked queue is empty
		{
		return firstChunk==0||frontIndex==firstChunk->header.numElements;
		}
	size_type size(void) const // Returns total number of elements in chunked array
		{
		size_type result=0;
		for(const Chunk* chPtr=firstChunk;chPtr!=0;chPtr=chPtr->header.succ)
			result+=chPtr->header.numElements;
		result-=frontIndex;
		return result;
		}
	const Content& front(void) const // Returns reference to the first element in the queue
		{
		return (*firstChunk)[frontIndex];
		}
	Content& front(void) // Ditto
		{
		return (*firstChunk)[frontIndex];
		}
	const Content& back(void) const // Returns reference to the last element in the array
		{
		return (*lastChunk)[lastChunk->header.numElements-1];
		}
	Content& back(void) // Ditto
		{
		return (*lastChunk)[lastChunk->header.numElements-1];
		}
	void push(const Content& newElement) // Pushes the new element to the back of the queue
		{
		/* Check for empty array first: */
		if(lastChunk==0)
			{
			/* Create the first chunk: */
			firstChunk=new Chunk;
			lastChunk=firstChunk;
			}
		
		/* Check if there is room in the last chunk: */
		if(lastChunk->header.numElements==chunkSize)
			{
			/* Create a new chunk: */
			lastChunk->header.succ=new Chunk;
			lastChunk=lastChunk->header.succ;
			}
		
		/* Append the new element to the last chunk: */
		new(lastChunk->getAddress(lastChunk->header.numElements)) Content(newElement);
		
		/* Increase the element count of the last chunk: */
		++lastChunk->header.numElements;
		}
	void pop(void) // Removes the first element of the queue
		{
		/* Destroy the first element: */
		(*firstChunk)[frontIndex].~Content();
		
		/* Increase the index of the first element in the first chunk: */
		++frontIndex;
		
		/* Remove the first chunk if it is empty: */
		if(frontIndex==firstChunk->header.numElements)
			{
			Chunk* succ=firstChunk->header.succ;
			/* Optimization: If this is the only chunk, don't remove it yet: */
			if(succ!=0)
				{
				/* Remove the chunk: */
				delete firstChunk;
				firstChunk=succ;
				}
			else
				{
				/* Re-use the chunk: */
				firstChunk->header.numElements=0;
				}
			frontIndex=0;
			}
		}
	void clear(void) // Clears all elements from the queue
		{
		while(firstChunk!=0)
			{
			Chunk* succ=firstChunk->header.succ;
			
			/* Destroy all elements in the chunk: */
			for(size_type i=0;i<firstChunk->header.numElements;++i)
				(*firstChunk)[i].~Content();
			
			/* Delete the chunk: */
			delete firstChunk;
			
			firstChunk=succ;
			}
		lastChunk=0;
		}
	}

}

#endif
