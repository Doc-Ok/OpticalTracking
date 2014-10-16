/***********************************************************************
MutexCond - Convenience class for condition variables that are protected
by their own mutual exclusion semaphores.
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

#ifndef THREADS_MUTEXCOND_INCLUDED
#define THREADS_MUTEXCOND_INCLUDED

#include <pthread.h>
#include <Misc/Time.h>
#include <Threads/Config.h>

#if THREADS_CONFIG_DEBUG
#include <errno.h>
#include <iostream>
#endif

namespace Threads {

class MutexCond
	{
	/* Embedded classes: */
	public:
	class Lock // Class to obtain mutex locks using construction mechanism
		{
		friend class MutexCond;
		
		/* Elements: */
		private:
		pthread_mutex_t* mutexPtr; // Pointer to mutex that was locked
		
		/* Constructors and destructors: */
		public:
		Lock(MutexCond& mutexCond) // Locks the given mutex-protected condition variable
			:mutexPtr(&mutexCond.mutex)
			{
			/* Lock the mutex: */
			#if THREADS_CONFIG_DEBUG
			if(pthread_mutex_lock(mutexPtr)!=0)
				std::cerr<<"Error in Threads::MutexCond::Lock::Lock"<<std::endl;
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
				std::cerr<<"Error in Threads::MutexCond::Lock::~Lock"<<std::endl;
			#else
			pthread_mutex_unlock(mutexPtr);
			#endif
			}
		};
	
	friend class Lock;
	
	/* Elements: */
	private:
	pthread_mutex_t mutex; // Low-level pthread mutex handle
	pthread_cond_t cond; // Low-level pthread condition variable handle
	
	/* Constructors and destructors: */
	public:
	MutexCond(void) // Creates default mutex and default condition variable
		{
		#if THREADS_CONFIG_DEBUG
		if(pthread_mutex_init(&mutex,0)!=0||pthread_cond_init(&cond,0)!=0)
			std::cerr<<"Error in Threads::MutexCond::MutexCond"<<std::endl;
		#else
		pthread_mutex_init(&mutex,0);
		pthread_cond_init(&cond,0);
		#endif
		}
	MutexCond(pthread_mutexattr_t* mutexAttributes) // Creates mutex with given attributes and default condition variable
		{
		#if THREADS_CONFIG_DEBUG
		if(pthread_mutex_init(&mutex,mutexAttributes)!=0||pthread_cond_init(&cond,0)!=0)
			std::cerr<<"Error in Threads::MutexCond::MutexCond"<<std::endl;
		#else
		pthread_mutex_init(&mutex,mutexAttributes);
		pthread_cond_init(&cond,0);
		#endif
		}
	MutexCond(pthread_mutexattr_t* mutexAttributes,pthread_condattr_t* condAttributes) // Creates mutex and condition variable with given attributes
		{
		#if THREADS_CONFIG_DEBUG
		if(pthread_mutex_init(&mutex,mutexAttributes)!=0||pthread_cond_init(&cond,condAttributes)!=0)
			std::cerr<<"Error in Threads::MutexCond::MutexCond"<<std::endl;
		#else
		pthread_mutex_init(&mutex,mutexAttributes);
		pthread_cond_init(&cond,condAttributes);
		#endif
		}
	private:
	MutexCond(const MutexCond& source); // Prohibit copy constructor
	MutexCond& operator=(const MutexCond& source); // Prohibit assignment operator
	public:
	~MutexCond(void)
		{
		#if THREADS_CONFIG_DEBUG
		if(pthread_mutex_destroy(&mutex)!=0||pthread_cond_destroy(&cond)!=0)
			std::cerr<<"Error in Threads::MutexCond::~MutexCond"<<std::endl;
		#else
		pthread_mutex_destroy(&mutex);
		pthread_cond_destroy(&cond);
		#endif
		}
	
	/* Methods: */
	void signal(void) // Signals the condition variable
		{
		#if THREADS_CONFIG_DEBUG
		if(pthread_cond_signal(&cond)!=0)
			std::cerr<<"Error in Threads::MutexCond::signal"<<std::endl;
		#else
		pthread_cond_signal(&cond);
		#endif
		}
	void broadcast(void) // Broadcasts the condition variable
		{
		#if THREADS_CONFIG_DEBUG
		if(pthread_cond_broadcast(&cond)!=0)
			std::cerr<<"Error in Threads::MutexCond::broadcast"<<std::endl;
		#else
		pthread_cond_broadcast(&cond);
		#endif
		}
	void wait(void) // Waits on condition variable; automatically obtains lock on mutex
		{
		Lock lock(*this);
		#if THREADS_CONFIG_DEBUG
		if(pthread_cond_wait(&cond,&mutex)!=0)
			std::cerr<<"Error in Threads::MutexCond::wait"<<std::endl;
		#else
		pthread_cond_wait(&cond,&mutex);
		#endif
		}
	bool timedWait(const Misc::Time& abstime) // Waits on condition variable; automatically obtains lock on mutex; returns true if signal occurred; returns false if time expires
		{
		Lock lock(*this);
		int result=pthread_cond_timedwait(&cond,&mutex,&abstime);
		#if THREADS_CONFIG_DEBUG
		if(result!=0&&result!=ETIMEDOUT)
			std::cerr<<"Error in Threads::MutexCond::timedWait"<<std::endl;
		#endif
		return result==0;
		}
	void wait(const Lock& lock) // Waits on condition variable when lock is already established
		{
		#if THREADS_CONFIG_DEBUG
		if(pthread_cond_wait(&cond,lock.mutexPtr)!=0)
			std::cerr<<"Error in Threads::MutexCond::wait"<<std::endl;
		#else
		pthread_cond_wait(&cond,lock.mutexPtr);
		#endif
		}
	bool timedWait(const Lock& lock,const Misc::Time& abstime) // Waits on condition variable when lock is already established; returns true if signal occurred; returns false if time expires
		{
		int result=pthread_cond_timedwait(&cond,lock.mutexPtr,&abstime);
		#if THREADS_CONFIG_DEBUG
		if(result!=0&&result!=ETIMEDOUT)
			std::cerr<<"Error in Threads::MutexCond::timedWait"<<std::endl;
		#endif
		return result==0;
		}
	};

}

#endif
