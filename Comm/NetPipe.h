/***********************************************************************
NetPipe - Base class for pipes connected to remote hosts using an
Internet protocol.
Copyright (c) 2011 Oliver Kreylos

This file is part of the Portable Communications Library (Comm).

The Portable Communications Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Portable Communications Library is distributed in the hope that it
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Portable Communications Library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef COMM_NETPIPE_INCLUDED
#define COMM_NETPIPE_INCLUDED

#include <string>
#include <Comm/Pipe.h>

namespace Comm {

class NetPipe:public Pipe
	{
	/* Constructors and destructors: */
	public:
	NetPipe(void)
		{
		}
	NetPipe(AccessMode sAccessMode)
		:Pipe(sAccessMode)
		{
		}
	
	/* New methods: */
	virtual int getPortId(void) const =0; // Returns port ID assigned to the socket
	virtual std::string getAddress(void) const =0; // Returns internet address assigned to the socket in dotted notation
	virtual std::string getHostName(void) const =0; // Returns host name of socket, or internet address in dotted notation if host name cannot be resolved
	virtual int getPeerPortId(void) const =0; // Returns port ID of remote socket
	virtual std::string getPeerAddress(void) const =0; // Returns internet address of remote socket in dotted notation
	virtual std::string getPeerHostName(void) const =0; // Returns host name of remote socket, or internet address in dotted notation if host name cannot be resolved
	};

typedef Misc::Autopointer<NetPipe> NetPipePtr; // Type for pointers to reference-counted Internet pipe objects

}

#endif
