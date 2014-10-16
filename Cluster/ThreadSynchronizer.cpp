/***********************************************************************
ClusterSynchronizer - Helper class to synchronize the number of created
child threads of the current thread across a cluster, in order to ensure
multi-threaded pipe creation consistency.
Copyright (c) 2012-2013 Oliver Kreylos

This file is part of the Cluster Abstraction Library (Cluster).

The Cluster Abstraction Library is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Cluster Abstraction Library is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Cluster Abstraction Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <Cluster/ThreadSynchronizer.h>

#include <Threads/Thread.h>
#include <Cluster/MulticastPipe.h>

namespace Cluster {

/***********************************
Methods of class ThreadSynchronizer:
***********************************/

ThreadSynchronizer::ThreadSynchronizer(MulticastPipe* sPipe)
	:pipe(sPipe),startChildThreadIndex(0)
	{
	if(pipe!=0)
		{
		/* Get the index of the next created child thread: */
		startChildThreadIndex=Threads::Thread::getThreadObject()->getNextChildIndex();
		}
	}

ThreadSynchronizer::~ThreadSynchronizer(void)
	{
	if(pipe!=0)
		{
		/* Get the number of child threads created since initialization: */
		unsigned int numChildThreads=Threads::Thread::getThreadObject()->getNextChildIndex()-startChildThreadIndex;
		
		/* Get the maximum number of created child threads for each cluster node: */
		unsigned int maxNumChildThreads=pipe->gather(numChildThreads,Cluster::GatherOperation::MAX);
		
		/* Synchronize the next child thread index: */
		Threads::Thread::getThreadObject()->advanceNextChildIndex(maxNumChildThreads-numChildThreads);
		}
	}

void ThreadSynchronizer::sync(void)
	{
	if(pipe!=0)
		{
		/* Get the number of child threads created since initialization: */
		unsigned int numChildThreads=Threads::Thread::getThreadObject()->getNextChildIndex()-startChildThreadIndex;
		
		/* Get the maximum number of created child threads for each cluster node: */
		unsigned int maxNumChildThreads=pipe->gather(numChildThreads,Cluster::GatherOperation::MAX);
		
		/* Synchronize the next child thread index: */
		Threads::Thread::getThreadObject()->advanceNextChildIndex(maxNumChildThreads-numChildThreads);
		
		/* Update the index of the next created child thread: */
		startChildThreadIndex=Threads::Thread::getThreadObject()->getNextChildIndex();
		}
	}

}
