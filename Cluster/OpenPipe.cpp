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

#include <Cluster/OpenPipe.h>

#include <Comm/TCPPipe.h>
#include <Cluster/Multiplexer.h>
#include <Cluster/TCPPipe.h>

namespace Cluster {

Comm::NetPipePtr openTCPPipe(Multiplexer* multiplexer,const char* hostName,int portId)
	{
	if(multiplexer!=0)
		{
		if(multiplexer->isMaster())
			{
			/* Open a master-side distributed TCP pipe: */
			return new TCPPipeMaster(multiplexer,hostName,portId);
			}
		else
			{
			/* Open a slave-side distributed TCP pipe: */
			return new TCPPipeSlave(multiplexer,hostName,portId);
			}
		}
	else
		{
		/* Return a non-distributed TCP pipe: */
		return new Comm::TCPPipe(hostName,portId);
		}
	}

}
