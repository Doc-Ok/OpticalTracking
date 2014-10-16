/***********************************************************************
RefCountedArray - Generic class for fixed-size arrays with copy-on-write
sharing and automatic garbage collection. Not safe to share among
multiple threads.
Copyright (c) 2010 Oliver Kreylos

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

#ifndef MISC_REFCOUNTEDARRAY_INCLUDED
#define MISC_REFCOUNTEDARRAY_INCLUDED

namespace Misc {

template <class ElementParam>
class RefCountedArray
	{
	/* Embedded classes: */
	public:
	typedef ElementParam Element; // Type of elements stored in the array
	
	private:
	struct Header // Header structure stored before actual array
		{
		/* Elements: */
		public:
		unsigned int refCount; // Number of RefCountedArray objects currently sharing the array
		size_t size; // Allocated size of the array
		Element elements[0]; // Beginning of actual array storage
		
		/* Methods: */
		void ref(void) // Increases the array's reference count
			{
			++refCount;
			}
		bool unref(void) // Decreases the array's reference count; returns true if array has become orphaned
			{
			return --refCount==0;
			}
		};
	
	/* Elements: */
	Header* header; // Pointer to header structure of shared array
	
	/* Private methods: */
	Header* allocate(size_t newSize); // Allocates a new array and returns a pointer to its header
	void free(Header* header);  // Destroys an orphaned array and its elements
	
	/* Constructors and destructors: */
	public:
	RefCountedArray(void) // Creates invalid array
		:header(0)
		{
		}
	RefCountedArray(size_t sSize) // Creates new array of the given size
		:header(allocate(sSize))
		{
		}
	RefCountedArray(const RefCountedArray& source) // Copy constructor; shares array
		:header(source.header)
		{
		/* Reference the shared array: */
		if(header!=0)
			header->ref();
		}
	RefCountedArray& operator=(const RefCountedArray& source) // Assignment operator; shares array
		{
		if(header!=source.header)
			{
			/* Unreference and potentially destroy the current shared array: */
			if(header!=0&&header->unref())
				free(header);
			
			/* Share the source's array: */
			header=source.header;
			
			/* Reference the new shared array: */
			if(header!=0)
				header->ref();
			}
		
		return *this;
		}
	~RefCountedArray(void)
		{
		/* Unreference and potentially destroy the current shared array: */
		if(header!=0&&header->unref())
			free(header);
		}
	
	/* Methods: */
	bool isValid(void) const // Returns true if the object has a valid array representation; precondition for all other methods
		{
		return header!=0;
		}
	size_t getSize(void) const // Returns allocated size of array
		{
		return header->size;
		}
	const Element* getArray(void) const // Returns a C-style pointer to the shared array
		{
		return header->elements;
		}
	const Element& operator[](size_t index) const // Returns an array element
		{
		return header->elements[index];
		}
	void set(size_t index,const Element& newElement) // Un-shares array and sets an array element
		{
		modify();
		header->elements[index]=newElement;
		}
	void modify(void); // Prepares the shared array for modification; precondition for all following methods
	Element* getArray(void) // Returns a C-style pointer to the shared array
		{
		return header->elements;
		}
	Element& operator[](size_t index) // Returns an array element
		{
		return header->elements[index];
		}
	};

}

#ifndef MISC_REFCOUNTEDARRAY_IMPLEMENTATION
#include <Misc/RefCountedArray.icpp>
#endif

#endif
