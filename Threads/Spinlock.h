/***********************************************************************
Spinlock - Wrapper class for pthreads spinlocks, mostly providing
"resource allocation as creation" paradigm and lock objects. If the
host's pthreads library does not provide spinlocks, this class simulates
them using mutexes instead.
Copyright (c) 2011-2012 Oliver Kreylos

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

#ifndef THREADS_SPINLOCK_INCLUDED
#define THREADS_SPINLOCK_INCLUDED

#include <pthread.h>
#include <Threads/Config.h>

#if THREADS_CONFIG_DEBUG
#include <errno.h>
#include <iostream>
#endif

#if THREADS_CONFIG_HAVE_SPINLOCKS

/* Use spinlock types and functions: */
#define ThreadsSpinlockType pthread_spinlock_t
#define ThreadsSpinlockLockFunc pthread_spin_lock
#define ThreadsSpinlockTrylockFunc pthread_spin_trylock
#define ThreadsSpinlockUnlockFunc pthread_spin_unlock
#define ThreadsSpinlockDestroyFunc pthread_spin_destroy

#else

/* Fall back to using mutex types and functions: */
#define ThreadsSpinlockType pthread_mutex_t
#define ThreadsSpinlockLockFunc pthread_mutex_lock
#define ThreadsSpinlockTrylockFunc pthread_mutex_trylock
#define ThreadsSpinlockUnlockFunc pthread_mutex_unlock
#define ThreadsSpinlockDestroyFunc pthread_mutex_destroy

#endif

namespace Threads {

class Spinlock
	{
	/* Embedded classes: */
	public:
	class Lock // Class to obtain spinlock locks using construction mechanism
		{
		/* Elements: */
		private:
		ThreadsSpinlockType* spinlockPtr; // Pointer to spinlock that was locked
		
		/* Constructors and destructors: */
		public:
		Lock(Spinlock& spinlock) // Locks the given mutex
			:spinlockPtr(&spinlock.spinlock)
			{
			/* Lock the spinlock: */
			#if THREADS_CONFIG_DEBUG
			if(ThreadsSpinlockLockFunc(spinlockPtr)!=0)
				std::cerr<<"Error in Threads::Spinlock::Lock::Lock"<<std::endl;
			#else
			ThreadsSpinlockLockFunc(spinlockPtr);
			#endif
			}
		private:
		Lock(const Lock& source); // Prohibit copy constructor
		Lock& operator=(const Lock& source); // Prohibit assignment
		public:
		~Lock(void)
			{
			/* Unlock the spinlock: */
			#if THREADS_CONFIG_DEBUG
			if(ThreadsSpinlockUnlockFunc(spinlockPtr)!=0)
				std::cerr<<"Error in Threads::Spinlock::Lock::~Lock"<<std::endl;
			#else
			ThreadsSpinlockUnlockFunc(spinlockPtr);
			#endif
			}
		};
	
	friend class Lock;
	
	/* Elements: */
	private:
	ThreadsSpinlockType spinlock; // Low-level pthread spinlock (or mutex) handle
	
	/* Constructors and destructors: */
	public:
	Spinlock(bool processShared =false) // Creates spinlock with given share flag
		{
		#if THREADS_CONFIG_HAVE_SPINLOCKS
		#if THREADS_CONFIG_DEBUG
		if(pthread_spin_init(&spinlock,processShared)!=0)
			std::cerr<<"Error in Threads::Spinlock::Spinlock"<<std::endl;
		#else
		pthread_spin_init(&spinlock,processShared);
		#endif
		#else
		#if THREADS_CONFIG_DEBUG
		if(pthread_mutex_init(&spinlock,0)!=0)
			std::cerr<<"Error in Threads::Spinlock::Spinlock"<<std::endl;
		#else
		pthread_mutex_init(&spinlock,0);
		#endif
		#endif
		}
	private:
	Spinlock(const Spinlock& source); // Prohibit copy constructor
	Spinlock& operator=(const Spinlock& source); // Prohibit assignment
	public:
	~Spinlock(void)
		{
		#if THREADS_CONFIG_DEBUG
		if(ThreadsSpinlockDestroyFunc(&spinlock)!=0)
			std::cerr<<"Error in Threads::Spinlock::~Spinlock"<<std::endl;
		#else
		ThreadsSpinlockDestroyFunc(&spinlock);
		#endif
		}
	
	/* Methods: */
	void lock(void) // Locks spinlock; blocks until lock is held
		{
		#if THREADS_CONFIG_DEBUG
		if(ThreadsSpinlockLockFunc(&spinlock)!=0)
			std::cerr<<"Error in Threads::Spinlock::lock"<<std::endl;
		#else
		ThreadsSpinlockLockFunc(&spinlock);
		#endif
		}
	bool tryLock(void) // Locks spinlock and returns true if currently unlocked; returns false otherwise
		{
		int result=ThreadsSpinlockTrylockFunc(&spinlock);
		#if THREADS_CONFIG_DEBUG
		if(result!=0&&result!=EBUSY)
			std::cerr<<"Error in Threads::Spinlock::tryLock"<<std::endl;
		#endif
		return result==0;
		}
	void unlock(void) // Unlocks spinlock
		{
		#if THREADS_CONFIG_DEBUG
		if(ThreadsSpinlockUnlockFunc(&spinlock)!=0)
			std::cerr<<"Error in Threads::Spinlock::unlock"<<std::endl;
		#else
		ThreadsSpinlockUnlockFunc(&spinlock);
		#endif
		}
	};

}

#endif
