/***********************************************************************
MulticastPipe - Class to represent data streams between a single master
and several slaves, with the bulk of communication from the master to
all the slaves in parallel.
Copyright (c) 2005-2012 Oliver Kreylos

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

#ifndef CLUSTER_MULTICASTPIPE_INCLUDED
#define CLUSTER_MULTICASTPIPE_INCLUDED

#include <stddef.h>
#include <IO/File.h>
#include <Cluster/ClusterPipe.h>

/* Forward declarations: */
namespace Cluster {
struct Packet;
}

namespace Cluster {

class MulticastPipe:public IO::File,public ClusterPipe
	{
	/* Elements: */
	private:
	Packet* packet; // Pointer to current packet
	size_t packetPos; // Data position in current packet
	
	/* Protected methods from IO::File: */
	protected:
	virtual size_t readData(Byte* buffer,size_t bufferSize);
	virtual void writeData(const Byte* buffer,size_t bufferSize);
	
	/* Protected methods from ClusterPipe: */
	virtual void flushPipe(void);
	
	/* Constructors and destructors: */
	public:
	MulticastPipe(Multiplexer* sMultiplexer); // Creates new pipe for the given multiplexer
	private:
	MulticastPipe(const MulticastPipe& source); // Prohibit copy constructor
	MulticastPipe& operator=(const MulticastPipe& source); // Prohibit assignment operato
	public:
	virtual ~MulticastPipe(void); // Closes the pipe
	
	/* Methods from IO::File: */
	virtual size_t getReadBufferSize(void) const;
	virtual size_t resizeReadBuffer(size_t newReadBufferSize);
	virtual size_t getWriteBufferSize(void) const;
	virtual void resizeWriteBuffer(size_t newWriteBufferSize);
	
	/* New methods: */
	template <class DataParam>
	void broadcast(DataParam& data) // Sends single value of arbitrary type from master to all slaves; does not change value on master
		{
		/* Read or write, depending on whether on slave nodes or master node: */
		if(isMaster())
			writeRaw(&data,sizeof(DataParam));
		else
			readRaw(&data,sizeof(DataParam));
		}
	template <class DataParam>
	void broadcast(DataParam* data,size_t numItems) // Sends array of values of arbitrary type from master to all slaves; does not change values on master
		{
		/* Read or write, depending on whether on slave nodes or master node: */
		if(isMaster())
			writeRaw(data,sizeof(DataParam)*numItems);
		else
			readRaw(data,sizeof(DataParam)*numItems);
		}
	};

}

#endif
