/***********************************************************************
Clusterize - Helper functions to distribute an application across a
cluster, and establish communications between nodes using a multicast
pipe multiplexer.
Copyright (c) 2009-2011 Oliver Kreylos

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

#ifndef CLUSTER_CLUSTERIZE_INCLUDED
#define CLUSTER_CLUSTERIZE_INCLUDED

/* Forward declarations: */
namespace Cluster {
class Multiplexer;
}

namespace Cluster {

Multiplexer* clusterize(int& argc,char**& argv); // Distributes the running application across a cluster and returns an intra-cluster multiplexer connecting node instances. Clusterization parameters are parsed and removed from the given command line
void unclusterize(Multiplexer* multiplexer); // Waits until all node instances have terminated, and shuts down and deletes the multiplexer

}

#endif
