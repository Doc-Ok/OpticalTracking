/***********************************************************************
OpenPipe - Convenience function to open pipes of several types using the
Comm::Pipe abstraction and distribute among a cluster via a multicast
pipe.
Copyright (c) 2011 Oliver Kreylos

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

#ifndef CLUSTER_OPENPIPE_INCLUDED
#define CLUSTER_OPENPIPE_INCLUDED

#include <Comm/NetPipe.h>

namespace Cluster {
class Multiplexer;
}

namespace Cluster {

Comm::NetPipePtr openTCPPipe(Multiplexer* multiplexer,const char* hostName,int portId); // Opens a TCP pipe to the given host / port and distributes it over the given intra-cluster communication multiplexer

}

#endif
