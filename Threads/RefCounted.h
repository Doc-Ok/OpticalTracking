/***********************************************************************
RefCounted - Base class for objects with automatic destruction based on
thread-safe reference counting. Reference-counted objects must be
created using the single-object new operator.
Copyright (c) 2007-2011 Oliver Kreylos

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

#ifndef THREADS_REFCOUNTED_INCLUDED
#define THREADS_REFCOUNTED_INCLUDED

#include <pthread.h>
#include <Threads/Config.h>

namespace Threads {

class RefCounted
	{
	/* Elements: */
	private:
	#if !THREADS_CONFIG_HAVE_BUILTIN_ATOMICS
	#if THREADS_CONFIG_HAVE_SPINLOCKS
	pthread_spinlock_t refCountSpinlock; // Busy-wait mutual exclusion semaphore protecting the reference counter
	#else
	pthread_mutex_t refCountSpinlock; // Busy-wait mutual exclusion semaphore protecting the reference counter
	#endif
	#endif
	unsigned int refCount; // Current number of autopointers referencing this object
	
	/* Constructors and destructors: */
	public:
	RefCounted(void) // Creates an unreferenced object
		:refCount(0)
		{
		#if !THREADS_CONFIG_HAVE_BUILTIN_ATOMICS
		/* Initialize the spinlock: */
		#if THREADS_CONFIG_HAVE_SPINLOCKS
		pthread_spin_init(&refCountSpinlock,0);
		#else
		pthread_mutex_init(&refCountSpinlock,0);
		#endif
		#endif
		}
	RefCounted(const RefCounted& source) // Copy constructor; creates unreferenced object
		:refCount(0)
		{
		#if !THREADS_CONFIG_HAVE_BUILTIN_ATOMICS
		#if THREADS_CONFIG_HAVE_SPINLOCKS
		pthread_spin_init(&refCountSpinlock,0);
		#else
		pthread_mutex_init(&refCountSpinlock,0);
		#endif
		#endif
		}
	RefCounted& operator=(const RefCounted& source) // Assignment operator; assigning does not change reference count
		{
		return *this;
		}
	virtual ~RefCounted(void) // Virtual destructor; called when the reference count reaches zero
		{
		#if !THREADS_CONFIG_HAVE_BUILTIN_ATOMICS
		/* Destroy the spinlock: */
		#if THREADS_CONFIG_HAVE_SPINLOCKS
		pthread_spin_destroy(&refCountSpinlock);
		#else
		pthread_mutex_destroy(&refCountSpinlock);
		#endif
		#endif
		}
	
	/* Methods: */
	void ref(void) // Method called when an autopointer starts referencing this object
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
	void unref(void) // Method called when an autopointer stops referencing an object; destroys object when reference count reaches zero
		{
		bool mustDelete=false;
		
		/* Decrement the reference counter atomically: */
		#if THREADS_CONFIG_HAVE_BUILTIN_ATOMICS
		mustDelete=__sync_sub_and_fetch(&refCount,1)==0;
		#else
		#if THREADS_CONFIG_HAVE_SPINLOCKS
		pthread_spin_lock(&refCountSpinlock);
		mustDelete=(--refCount)==0;
		pthread_spin_unlock(&refCountSpinlock);
		#else
		pthread_mutex_lock(&refCountSpinlock);
		mustDelete=(--refCount)==0;
		pthread_mutex_unlock(&refCountSpinlock);
		#endif
		#endif
		
		/* Delete the object if the reference count reached zero: */
		if(mustDelete)
			delete this;
		}
	};

}

#endif
