/***********************************************************************
Barrier - Class implementing synchronization points where a fixed number
of threads have to come together before any can proceed.
Copyright (c) 2006-2011 Oliver Kreylos

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

#ifndef THREADS_BARRIER_INCLUDED
#define THREADS_BARRIER_INCLUDED

#include <pthread.h>
#include <stdexcept>

namespace Threads {

class Barrier
	{
	/* Embedded classes: */
	public:
	class Error:public std::runtime_error // Base exception class for barrier-related errors
		{
		/* Constructors and destructors: */
		public:
		Error(const char* msg)
			:std::runtime_error(msg)
			{
			}
		};
	
	class BarrierBusy:public Error // Exception class thrown when trying to resize an active barrier
		{
		/* Constructors and destructors: */
		public:
		BarrierBusy(void)
			:Error("Barrier::setNumSynchronizingThreads: Barrier is busy")
			{
			}
		};
	
	/* Elements: */
	private:
	pthread_mutex_t mutex; // A mutex serializing access to the barrier structure
	volatile unsigned int numSynchronizingThreads; // Number of threads that have to synchronize before they can proceed
	volatile unsigned int frame; // A frame counter to catch spurious wake-ups of waiting threads
	volatile unsigned int numWaitingThreads; // Number of threads that are already waiting at the barrier
	pthread_cond_t cond; // A condition variable to wake up waiting threads if the synchronization point is complete
	
	/* Constructors and destructors: */
	public:
	Barrier(unsigned int sNumSynchronizingThreads =1) // Creates a barrier to synchronize the given number of threads
		:numSynchronizingThreads(sNumSynchronizingThreads),
		 frame(0),numWaitingThreads(0)
		{
		/* Initialize the mutex and the condition variable: */
		pthread_mutex_init(&mutex,0);
		pthread_cond_init(&cond,0);
		}
	private:
	Barrier(const Barrier& source); // Prohibit copy constructor
	Barrier& operator=(const Barrier& source); // Prohibit assignment operator
	public:
	~Barrier(void)
		{
		/* Destroy the mutex and the condition variable: */
		pthread_mutex_destroy(&mutex);
		pthread_cond_destroy(&cond);
		}
	
	/* Methods: */
	unsigned int getNumSynchronizingThreads(void) const // Returns the number of threads that have to synchronize
		{
		return numSynchronizingThreads;
		}
	void setNumSynchronizingThreads(unsigned int newNumSynchronizingThreads) // Sets the number of threads that have to synchronize; blocks the calling thread until any ongoing synchronization is complete
		{
		/* Lock the mutex: */
		pthread_mutex_lock(&mutex);
		
		/* Check if the barrier is in the middle of a synchronization: */
		if(numWaitingThreads!=0)
			{
			/* Unlock the mutex and throw an exception: */
			pthread_mutex_unlock(&mutex);
			throw BarrierBusy();
			}
		
		/* Set the number of synchronizing threads: */
		numSynchronizingThreads=newNumSynchronizingThreads;
		
		/* Unlock the mutex: */
		pthread_mutex_unlock(&mutex);
		}
	bool synchronize(void) // Enters the synchronization point; blocks the calling thread until synchronization is complete; returns true for exactly one of the callers upon wakeup
		{
		bool result=false;
		
		/* Lock the mutex: */
		pthread_mutex_lock(&mutex);
		
		/* Enter the synchronization point: */
		++numWaitingThreads;
		
		/* Check if the synchronization is complete: */
		if(numWaitingThreads==numSynchronizingThreads)
			{
			/* Mark the synchronization as complete: */
			++frame;
			numWaitingThreads=0;
			
			/* Wake up all waiting threads: */
			pthread_cond_broadcast(&cond);
			
			/* This is the one call returning true: */
			result=true;
			}
		else
			{
			/* Wait until the current synchronization is complete: */
			unsigned int currentFrame=frame;
			while(currentFrame==frame)
				pthread_cond_wait(&cond,&mutex);
			}
		
		/* Unlock the mutex: */
		pthread_mutex_unlock(&mutex);
		
		return result;
		}
	};

}

#endif
