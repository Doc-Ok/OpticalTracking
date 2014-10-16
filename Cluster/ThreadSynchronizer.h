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

#ifndef CLUSTER_THREADSYNCHRONIZER_INCLUDED
#define CLUSTER_THREADSYNCHRONIZER_INCLUDED

/* Forward declarations: */
namespace Cluster {
class MulticastPipe;
}

namespace Cluster {

class ThreadSynchronizer
	{
	/* Elements: */
	private:
	MulticastPipe* pipe; // Multicast pipe used to synchronize the number of created threads
	unsigned int startChildThreadIndex; // Next child thread index of current thread when object is created
	
	/* Constructors and destructors: */
	public:
	ThreadSynchronizer(MulticastPipe* sPipe); // Creates a thread synchronizer using the given cluster pipe; cluster pipe's lifetime must bracket this object's lifetime
	~ThreadSynchronizer(void); // Synchronizes number of created child threads using the assigned cluster pipe
	
	/* Methods: */
	void sync(void); // Synchronizes number of created child threads using the assigned cluster pipe
	};

}

#endif
