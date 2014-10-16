/***********************************************************************
Local - Wrapper class for per-thread global variables using the pthreads
interface. Thread-local storage provided by the local compiler and run-
time environment (such as the gcc __thread extension) are generally
preferable to the pthreads interface.
Copyright (c) 2006 Oliver Kreylos

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

#ifndef THREADS_LOCAL_INCLUDED
#define THREADS_LOCAL_INCLUDED

#include <pthread.h>

namespace Threads {

template <class ValueParam>
class Local
	{
	/* Embedded classes: */
	public:
	typedef ValueParam Value; // Actual type of thread-local variable
	
	/* Elements: */
	private:
	pthread_key_t key; // Process-wide key for thread-local values
	
	/* Private methods: */
	static void destructor(void* value) // Destroys a thread-local value
		{
		delete static_cast<Value*>(value);
		}
	
	/* Constructors and destructors: */
	public:
	Local(void) // Initializes process-wide key
		{
		pthread_key_create(&key,destructor);
		}
	private:
	Local(const Local& source); // Prohibit copy constructor
	Local& operator=(const Local& source); // Prohibit assignment operator
	public:
	~Local(void) // Destroys process-wide key
		{
		pthread_key_delete(key);
		}
	
	/* Methods: */
	Value& operator=(const Value& newValue) // Assignment of new thread-local value
		{
		/* Get a pointer to the thread-local value: */
		Value* valuePtr=static_cast<Value*>(pthread_getspecific(key));
		if(valuePtr==0)
			{
			/* Create a new thread-local value: */
			valuePtr=new Value(newValue);
			pthread_setspecific(key,valuePtr);
			}
		else
			{
			/* Set the thread-local value: */
			*valuePtr=newValue;
			}
		
		return *valuePtr;
		}
	operator Value(void) const // Returns the thread-local value
		{
		return *static_cast<const Value*>(pthread_getspecific(key));
		}
	const Value& value(void) const // Explicit function to return the thread-local value
		{
		return *static_cast<const Value*>(pthread_getspecific(key));
		}
	}

}

#endif
