/***********************************************************************
ClusterPipe - Base class providing a 1-to-n intra-cluster communication
pattern using a cluster multiplexer.
Copyright (c) 2011-2012 Oliver Kreylos

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

#include <Cluster/ClusterPipe.h>

#include <Cluster/Multiplexer.h>

namespace Cluster {

/****************************
Methods of class ClusterPipe:
****************************/

void ClusterPipe::flushPipe(void)
	{
	}

ClusterPipe::ClusterPipe(Multiplexer* sMultiplexer)
	:multiplexer(sMultiplexer),
	 pipeId(multiplexer->openPipe()),
	 readCoupled(true),writeCoupled(true)
	{
	}

ClusterPipe::~ClusterPipe(void)
	{
	/* Close the pipe: */
	multiplexer->closePipe(pipeId);
	}

void ClusterPipe::couple(bool newReadCoupled,bool newWriteCoupled)
	{
	if(readCoupled!=newReadCoupled||writeCoupled!=newWriteCoupled)
		{
		/* Send any unsent data and execute a barrier to synchronize: */
		flushPipe();
		multiplexer->barrier(pipeId);
		}
	
	/* Set the coupling flags: */
	readCoupled=newReadCoupled;
	writeCoupled=newWriteCoupled;
	}

void ClusterPipe::barrier(void)
	{
	/* Send any unsent data: */
	flushPipe();
	
	/* Pass call through to multicast pipe multiplexer: */
	multiplexer->barrier(pipeId);
	}

unsigned int ClusterPipe::gather(unsigned int value,GatherOperation::OpCode op)
	{
	/* Send any unsent data: */
	flushPipe();
	
	/* Pass call through to multicast pipe multiplexer: */
	return multiplexer->gather(pipeId,value,op);
	}

}
