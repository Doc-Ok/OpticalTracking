/***********************************************************************
RefCountedArray - Generic class for fixed-size arrays with copy-on-write
sharing and automatic garbage collection. Thread-safe version.
Copyright (c) 2010 Oliver Kreylos

This file is part of the Portable Threading Library (Threads).

The Portable Threading Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Portable Threading Library is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Portable Threading Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef THREADS_REFCOUNTEDARRAY_INCLUDED
#define THREADS_REFCOUNTEDARRAY_INCLUDED

#include <pthread.h>
#include <Threads/Config.h>

namespace Threads {

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
		#if !THREADS_CONFIG_HAVE_BUILTIN_ATOMICS
		#if THREADS_CONFIG_HAVE_SPINLOCKS
		pthread_spinlock_t refCountSpinlock; // Busy-wait mutual exclusion semaphore protecting the reference counter
		#else
		pthread_mutex_t refCountSpinlock; // Busy-wait mutual exclusion semaphore protecting the reference counter
		#endif
		#endif
		unsigned int refCount; // Number of RefCountedArray objects currently sharing the array
		size_t size; // Allocated size of the array
		Element elements[0]; // Beginning of actual array storage
		
		/* Methods: */
		void ref(void) // Increases the array's reference count
			{
			/* Increment the reference counter atomically: */
			#if THREADS_CONFIG_HAVE_BUILTIN_ATOMICS
			__sync_add_and_fetch(&refCount,1);
			#else
			#if THREADS_CONFIG_HAVE_SPINLOCKS
			pthread_spin_lock(&refCountSpinlock);
			++refCount;
			pthread_spin_unlock(&refCountSpinlock);
			#else
			pthread_mutex_lock(&refCountSpinlock);
			++refCount;
			pthread_mutex_unlock(&refCountSpinlock);
			#endif
			#endif
			}
		bool unref(void) // Decreases the array's reference count; returns true if array has become orphaned
			{
			/* Decrement the reference counter atomically: */
			#if THREADS_CONFIG_HAVE_BUILTIN_ATOMICS
			return __sync_sub_and_fetch(&refCount,1)==0;
			#else
			#if THREADS_CONFIG_HAVE_SPINLOCKS
			pthread_spin_lock(&refCountSpinlock);
			bool result=(--refCount)==0;
			pthread_spin_unlock(&refCountSpinlock);
			#else
			pthread_mutex_lock(&refCountSpinlock);
			bool result=(--refCount)==0;
			pthread_mutex_unlock(&refCountSpinlock);
			#endif
			return result;
			#endif
			}
		};
	
	/* Elements: */
	Header* header; // Pointer to header structure of shared array
	
	/* Private methods: */
	Header* create(size_t newSize); // Allocates a new array and returns a pointer to its header
	void destroy(Header* header);  // Destroys an orphaned array and its elements
	
	/* Constructors and destructors: */
	public:
	RefCountedArray(void) // Creates invalid array
		:header(0)
		{
		}
	RefCountedArray(size_t sSize) // Creates new array of the given size
		:header(create(sSize))
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
				destroy(header);
			
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
			destroy(header);
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

#ifndef THREADS_REFCOUNTEDARRAY_IMPLEMENTATION
#include <Threads/RefCountedArray.icpp>
#endif

#endif
