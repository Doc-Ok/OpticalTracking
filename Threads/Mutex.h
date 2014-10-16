/***********************************************************************
Mutex - Wrapper class for pthreads mutual exclusion semaphores, mostly
providing "resource allocation as creation" paradigm and lock objects.
Copyright (c) 2005-2012 Oliver Kreylos

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

#ifndef THREADS_MUTEX_INCLUDED
#define THREADS_MUTEX_INCLUDED

#include <pthread.h>
#include <Threads/Config.h>

#if THREADS_CONFIG_DEBUG
#include <errno.h>
#include <iostream>
#endif

/* Forward declarations: */
namespace Threads {
class Cond;
}

namespace Threads {

class Mutex
	{
	/* Embedded classes: */
	public:
	class Lock // Class to obtain mutex locks using construction mechanism
		{
		/* Elements: */
		private:
		pthread_mutex_t* mutexPtr; // Pointer to mutex that was locked
		
		/* Constructors and destructors: */
		public:
		Lock(Mutex& mutex) // Locks the given mutex
			:mutexPtr(&mutex.mutex)
			{
			/* Lock the mutex: */
			#if THREADS_CONFIG_DEBUG
			if(pthread_mutex_lock(mutexPtr)!=0)
				std::cerr<<"Error in Threads::Mutex::Lock::Lock"<<std::endl;
			#else
			pthread_mutex_lock(mutexPtr);
			#endif
			}
		private:
		Lock(const Lock& source); // Prohibit copy constructor
		Lock& operator=(const Lock& source); // Prohibit assignment
		public:
		~Lock(void)
			{
			/* Unlock the mutex: */
			#if THREADS_CONFIG_DEBUG
			if(pthread_mutex_unlock(mutexPtr)!=0)
				std::cerr<<"Error in Threads::Mutex::Lock::~Lock"<<std::endl;
			#else
			pthread_mutex_unlock(mutexPtr);
			#endif
			}
		};
	
	friend class Lock;
	friend class Cond;
	
	/* Elements: */
	private:
	pthread_mutex_t mutex; // Low-level pthread mutex handle
	
	/* Constructors and destructors: */
	public:
	Mutex(const pthread_mutexattr_t* mutexAttribute =0) // Creates mutex with given attributes (or default mutex)
		{
		#if THREADS_CONFIG_DEBUG
		if(pthread_mutex_init(&mutex,mutexAttribute)!=0)
			std::cerr<<"Error in Threads::Mutex::Mutex"<<std::endl;
		#else
		pthread_mutex_init(&mutex,mutexAttribute);
		#endif
		}
	private:
	Mutex(const Mutex& source); // Prohibit copy constructor
	Mutex& operator=(const Mutex& source); // Prohibit assignment
	public:
	~Mutex(void)
		{
		#if THREADS_CONFIG_DEBUG
		if(pthread_mutex_destroy(&mutex)!=0)
			std::cerr<<"Error in Threads::Mutex::~Mutex"<<std::endl;
		#else
		pthread_mutex_destroy(&mutex);
		#endif
		}
	
	/* Methods: */
	void lock(void) // Locks mutex; blocks until lock is held
		{
		#if THREADS_CONFIG_DEBUG
		if(pthread_mutex_lock(&mutex)!=0)
			std::cerr<<"Error in Threads::Mutex::lock"<<std::endl;
		#else
		pthread_mutex_lock(&mutex);
		#endif
		}
	bool tryLock(void) // Locks mutex and returns true if currently unlocked; returns false otherwise
		{
		int result=pthread_mutex_trylock(&mutex);
		#if THREADS_CONFIG_DEBUG
		if(result!=0&&result!=EBUSY)
			std::cerr<<"Error in Threads::Mutex::tryLock"<<std::endl;
		#endif
		return result==0;
		}
	void unlock(void) // Unlocks mutex
		{
		#if THREADS_CONFIG_DEBUG
		if(pthread_mutex_unlock(&mutex)!=0)
			std::cerr<<"Error in Threads::Mutex::unlock"<<std::endl;
		#else
		pthread_mutex_unlock(&mutex);
		#endif
		}
	};

}

#endif
