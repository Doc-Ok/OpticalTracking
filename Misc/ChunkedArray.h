/***********************************************************************
ChunkedArray - Data structure to store lists of elements in page-aligned
chunks of memory.
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

#ifndef MISC_CHUNKEDARRAY_INCLUDED
#define MISC_CHUNKEDARRAY_INCLUDED

#include <stddef.h>
#include <new>

namespace Misc {

template <class ContentParam,size_t pageSizeParam =8192>
class ChunkedArray
	{
	/* Embedded classes: */
	public:
	typedef ContentParam Content; // Type of array contents
	static const size_t pageSize=pageSizeParam; // Size of memory page in bytes
	
	private:
	struct Chunk;
	
	struct ChunkHeader // Structure for array chunk headers
		{
		/* Elements: */
		public:
		Chunk* succ; // Pointer to next chunk in list
		size_t numElements; // Number of array elements in chunk
		
		/* Constructors and destructors: */
		ChunkHeader(void)
			:succ(0),numElements(0)
			{
			}
		};
	
	static const size_t chunkSize=(pageSize-sizeof(ChunkHeader))/sizeof(Content); // Number of array elements per chunk
	
	struct Chunk // Structure for array chunks
		{
		/* Elements: */
		public:
		ChunkHeader header; // Chunk header
		char mem[pageSize-sizeof(ChunkHeader)];
		
		/* Methods: */
		const Content& operator[](size_t index) const
			{
			return reinterpret_cast<const Content*>(mem)[index];
			}
		Content& operator[](size_t index)
			{
			return reinterpret_cast<Content*>(mem)[index];
			}
		const Content* getAddress(size_t index) const
			{
			return reinterpret_cast<const Content*>(mem)+index;
			}
		Content* getAddress(size_t index)
			{
			return reinterpret_cast<Content*>(mem)+index;
			}
		};
	
	public:
	class const_iterator;
	
	class iterator
		{
		friend class ChunkedArray;
		friend class const_iterator;
		
		/* Elements: */
		private:
		Chunk* chunk; // Pointer to chunk containing the currently indexed element
		size_t index; // Index of the currently indexed element in the current chunk
		
		/* Constructors and destructors: */
		public:
		iterator(void) // Creates invalid iterator
			:chunk(0),index(0)
			{
			}
		private:
		iterator(Chunk* sChunk) // Creates iterator to first element in chunk
			:chunk(sChunk),index(0)
			{
			}
		
		/* Methods: */
		public:
		bool operator==(const iterator& other) const
			{
			return chunk==other.chunk&&index==other.index;
			}
		bool operator!=(const iterator& other) const
			{
			return chunk!=other.chunk||index!=other.index;
			}
		Content& operator*(void) const
			{
			return (*chunk)[index];
			}
		Content* operator->(void) const
			{
			return chunk->getAddress(index);
			}
		iterator& operator++(void)
			{
			++index;
			if(index==chunk->header.numElements)
				{
				chunk=chunk->header.succ;
				index=0;
				}
			return *this;
			}
		iterator operator++(int)
			{
			iterator result(*this);
			++index;
			if(index==chunk->header.numElements)
				{
				chunk=chunk->header.succ;
				index=0;
				}
			return result;
			}
		};
	
	class const_iterator
		{
		friend class ChunkedArray;
		
		/* Elements: */
		private:
		const Chunk* chunk; // Pointer to chunk containing the currently indexed element
		size_t index; // Index of the currently indexed element in the current chunk
		
		/* Constructors and destructors: */
		public:
		const_iterator(void) // Creates invalid iterator
			:chunk(0),index(0)
			{
			}
		const_iterator(const iterator& source) // Creates constant iterator from regular iterator
			:chunk(source.chunk),index(source.index)
			{
			}
		private:
		const_iterator(const Chunk* sChunk) // Creates iterator to first element in chunk
			:chunk(sChunk),index(0)
			{
			}
		
		/* Methods: */
		public:
		bool operator==(const const_iterator& other) const
			{
			return chunk==other.chunk&&index==other.index;
			}
		bool operator==(const iterator& other) const
			{
			return chunk==other.chunk&&index==other.index;
			}
		bool operator!=(const const_iterator& other) const
			{
			return chunk!=other.chunk||index!=other.index;
			}
		bool operator!=(const iterator& other) const
			{
			return chunk!=other.chunk||index!=other.index;
			}
		const Content& operator*(void) const
			{
			return (*chunk)[index];
			}
		const Content* operator->(void) const
			{
			return chunk->getAddress(index);
			}
		const_iterator& operator++(void)
			{
			++index;
			if(index==chunk->header.numElements)
				{
				chunk=chunk->header.succ;
				index=0;
				}
			return *this;
			}
		const_iterator operator++(int)
			{
			const_iterator result(*this);
			++index;
			if(index==chunk->header.numElements)
				{
				chunk=chunk->header.succ;
				index=0;
				}
			return result;
			}
		};
	
	/* Elements: */
	private:
	Chunk* firstChunk; // Pointer to first chunk in list
	Chunk* lastChunk; // Pointer to last chunk in list
	
	/* Constructors and destructors: */
	public:
	ChunkedArray(void) // Creates empty array
		:firstChunk(0),lastChunk(0)
		{
		}
	private:
	ChunkedArray(const ChunkedArray& source); // Prohibit copy constructor
	ChunkedArray& operator=(const ChunkedArray& source); // Prohibit assignment operator
	public:
	~ChunkedArray(void) // Destroys array
		{
		clear();
		}
	
	/* Methods: */
	bool empty(void) const // Returns true if chunked array is empty
		{
		return firstChunk==0;
		}
	size_t size(void) const // Returns total number of elements in chunked array
		{
		size_t result=0;
		for(const Chunk* chPtr=firstChunk;chPtr!=0;chPtr=chPtr->header.succ)
			result+=chPtr->header.numElements;
		return result;
		}
	const Content& back(void) const // Returns reference to the last element in the array
		{
		return (*lastChunk)[lastChunk->header.numElements-1];
		}
	Content& back(void) // Ditto
		{
		return (*lastChunk)[lastChunk->header.numElements-1];
		}
	void push_back(const Content& newElement) // Pushes the new element to the back of the array
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
	void pop_back(void) // Removes the last element of the array
		{
		/* Decrease the element count of the last chunk: */
		--lastChunk->header.numElements;
		
		/* Destroy the last element: */
		(*lastChunk)[lastChunk->header.numElements].~Content();
		
		/* Remove last chunk if it is empty: */
		if(lastChunk->header.numElements==0)
			{
			/* Find pointer to but-last chunk: */
			Chunk* chPtr1=0;
			for(Chunk* chPtr2=firstChunk;chPtr2!=lastChunk;chPtr1=chPtr2,chPtr2=chPtr2->header.succ)
				;
			
			/* Remove last chunk: */
			delete lastChunk;
			lastChunk=chPtr1;
			
			/* Check for empty array: */
			if(lastChunk==0)
				firstChunk=0;
			}
		}
	void clear(void) // Clears all elements from the array
		{
		while(firstChunk!=0)
			{
			Chunk* succ=firstChunk->header.succ;
			
			/* Destroy all elements in the chunk: */
			for(size_t i=0;i<firstChunk->header.numElements;++i)
				(*firstChunk)[i].~Content();
			
			/* Delete the chunk: */
			delete firstChunk;
			
			firstChunk=succ;
			}
		lastChunk=0;
		}
	const_iterator begin(void) const // Returns iterator to the first element
		{
		return const_iterator(firstChunk);
		}
	iterator begin(void) // Ditto
		{
		return iterator(firstChunk);
		}
	const_iterator end(void) const // Returns iterator one past the last element
		{
		return const_iterator();
		}
	iterator end(void) // Ditto
		{
		return iterator();
		}
	template <class FunctorParam>
	void forEach(FunctorParam& functor) const // Applies given functor to each element in the array in order
		{
		/* Iterate through all chunks: */
		for(const Chunk* chunkPtr=firstChunk;chunkPtr!=0;chunkPtr=chunkPtr->header.succ)
			{
			/* Iterate through all elements in the chunk: */
			for(size_t i=0;i<chunkPtr->header.numElements;++i)
				{
				/* Apply the functor to the element: */
				functor((*chunkPtr)[i]);
				}
			}
		}
	template <class FunctorParam>
	void forEach(FunctorParam& functor) // Ditto
		{
		/* Iterate through all chunks: */
		for(Chunk* chunkPtr=firstChunk;chunkPtr!=0;chunkPtr=chunkPtr->header.succ)
			{
			/* Iterate through all elements in the chunk: */
			for(size_t i=0;i<chunkPtr->header.numElements;++i)
				{
				/* Apply the functor to the element: */
				functor((*chunkPtr)[i]);
				}
			}
		}
	template <class FunctorParam>
	void forEachChunk(FunctorParam& functor) const // Applies given functor to each chunk in the array in order
		{
		/* Iterate through all chunks: */
		for(const Chunk* chunkPtr=firstChunk;chunkPtr!=0;chunkPtr=chunkPtr->header.succ)
			{
			/* Call the functor for all elements in the chunk at once: */
			functor(reinterpret_cast<const Content*>(chunkPtr->mem),chunkPtr->header.numElements);
			}
		}
	};

}

#endif
