/***********************************************************************
ClusterSupport - Helper functions to simplify programming cluster-aware
Vrui applications. All these functions are NOPs if the passed pipe
pointer is NULL. This allows application programmers to always use pipe
communications in a cluster-aware application, even when it is not
running on a cluster.
Copyright (c) 2007-2011 Oliver Kreylos

This file is part of the Virtual Reality User Interface Library (Vrui).

The Virtual Reality User Interface Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Virtual Reality User Interface Library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality User Interface Library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef VRUI_CLUSTERSUPPORT_INCLUDED
#define VRUI_CLUSTERSUPPORT_INCLUDED

#include <Misc/ThrowStdErr.h>
#include <Cluster/MulticastPipe.h>

namespace Vrui {

/***********************************************************************
barrier: Blocks the calling thread until all nodes on the multicast pipe
have reached the same barrier.
***********************************************************************/

inline
void
barrier(
	Cluster::MulticastPipe* clusterPipe)
	{
	if(clusterPipe!=0)
		clusterPipe->barrier();
	}

/***********************************************************************
broadcast: Broadcasts a single value of an arbitrary data type by
writing on the master node and reading on the slave nodes.
***********************************************************************/

template <class DataParam>
inline
void
broadcast(
	Cluster::MulticastPipe* clusterPipe,
	DataParam& data)
	{
	if(clusterPipe!=0)
		clusterPipe->broadcast<DataParam>(data);
	}

/***********************************************************************
broadcast: Broadcasts an array of values of an arbitrary data type by
writing on the master node and reading on the slave nodes.
***********************************************************************/

template <class DataParam>
inline
void
broadcast(
	Cluster::MulticastPipe* clusterPipe,
	DataParam* data,
	size_t numItems)
	{
	if(clusterPipe!=0)
		clusterPipe->broadcast<DataParam>(data,numItems);
	}

/***********************************************************************
write: Writes a single value of an arbitrary data type on the master
node, causes an exception when called on a slave node.
***********************************************************************/

template <class DataParam>
inline
void
write(
	Cluster::MulticastPipe* clusterPipe,
	const DataParam& data)
	{
	if(clusterPipe!=0)
		clusterPipe->write<DataParam>(data);
	}

/***********************************************************************
write: Writes an array of values of an arbitrary data type on the master
node, causes an exception when called on a slave node.
***********************************************************************/

template <class DataParam>
inline
void
write(
	Cluster::MulticastPipe* clusterPipe,
	const DataParam* data,
	size_t numItems)
	{
	if(clusterPipe!=0)
		clusterPipe->write<DataParam>(data,numItems);
	}

/***********************************************************************
read: Reads a single value of an arbitrary data type on a slave node,
causes an exception when called on the master node or in a non-cluster
environment.
***********************************************************************/

template <class DataParam>
inline
void
read(
	Cluster::MulticastPipe* clusterPipe,
	DataParam& data)
	{
	if(clusterPipe==0)
		Misc::throwStdErr("Vrui::read: Called in non-cluster environment");
	clusterPipe->read<DataParam>(data);
	}

/***********************************************************************
read: Reads a single value of an arbitrary data type on a slave node,
causes an exception when called on the master node.
***********************************************************************/

template <class DataParam>
inline
DataParam
read(
	Cluster::MulticastPipe* clusterPipe)
	{
	if(clusterPipe==0)
		Misc::throwStdErr("Vrui::read: Called in non-cluster environment");
	return clusterPipe->read<DataParam>();
	}

/***********************************************************************
read: Reads an array of values of an arbitrary data type on a slave
node, causes an exception when called on the master node or in a non-
cluster environment.
***********************************************************************/

template <class DataParam>
inline
void
read(
	Cluster::MulticastPipe* clusterPipe,
	DataParam* data,
	size_t numItems)
	{
	if(clusterPipe==0)
		Misc::throwStdErr("Vrui::read: Called in non-cluster environment");
	clusterPipe->read<DataParam>(data,numItems);
	}

/***********************************************************************
flush: Flushes the pipe's output buffer on the master node.
***********************************************************************/

inline
void
flush(
	Cluster::MulticastPipe* clusterPipe)
	{
	if(clusterPipe!=0)
		{
		clusterPipe->flush();
		}
	}

}

#endif
