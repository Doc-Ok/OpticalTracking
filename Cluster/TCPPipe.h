/***********************************************************************
TCPPipe - Pair of classes for high-performance cluster-transparent
reading/writing from/to TCP sockets.
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

#ifndef CLUSTER_TCPPIPE_INCLUDED
#define CLUSTER_TCPPIPE_INCLUDED

#include <Comm/NetPipe.h>
#include <Cluster/ClusterPipe.h>

namespace Cluster {

class TCPPipeMaster:public Comm::NetPipe,public ClusterPipe // Class to represent cluster-transparent TCP pipes on the master node
	{
	/* Elements: */
	private:
	int fd; // File descriptor of the underlying TCP socket
	
	/* Protected methods from IO::File: */
	virtual size_t readData(Byte* buffer,size_t bufferSize);
	virtual void writeData(const Byte* buffer,size_t bufferSize);
	
	/* Constructors and destructors: */
	public:
	TCPPipeMaster(Multiplexer* sMultiplexer,const char* hostName,int portId); // Opens a TCP socket connected to the given port on the given host with "DontCare" endianness setting and shares it across the given intra-cluster communication multiplexer
	virtual ~TCPPipeMaster(void);
	
	/* Methods from IO::File: */
	virtual int getFd(void) const;
	virtual size_t resizeReadBuffer(size_t newReadBufferSize);
	
	/* Methods from Comm::Pipe: */
	virtual bool waitForData(void) const;
	virtual bool waitForData(const Misc::Time& timeout) const;
	virtual void shutdown(bool read,bool write);
	
	/* Methods from Comm::NetPipe: */
	virtual int getPortId(void) const;
	virtual std::string getAddress(void) const;
	virtual std::string getHostName(void) const;
	virtual int getPeerPortId(void) const;
	virtual std::string getPeerAddress(void) const;
	virtual std::string getPeerHostName(void) const;
	};

class TCPPipeSlave:public Comm::NetPipe,public ClusterPipe // Class to represent cluster-transparent TCP pipes on the slave nodes
	{
	/* Elements: */
	private:
	Packet* packet; // Pointer to most recently received multicast packet; doubles as pipe's read buffer
	
	/* Protected methods from IO::File: */
	virtual size_t readData(Byte* buffer,size_t bufferSize);
	virtual void writeData(const Byte* buffer,size_t bufferSize);
	
	/* Constructors and destructors: */
	public:
	TCPPipeSlave(Multiplexer* sMultiplexer,const char* hostName,int portId); // Opens a TCP socket connected to the given port on the given host with "DontCare" endianness setting and shares it across the given intra-cluster communication multiplexer
	virtual ~TCPPipeSlave(void);
	
	/* Methods from IO::File: */
	virtual int getFd(void) const;
	virtual size_t getReadBufferSize(void) const;
	virtual size_t resizeReadBuffer(size_t newReadBufferSize);
	
	/* Methods from Comm::Pipe: */
	virtual bool waitForData(void) const;
	virtual bool waitForData(const Misc::Time& timeout) const;
	virtual void shutdown(bool read,bool write);
	
	/* Methods from Comm::NetPipe: */
	virtual int getPortId(void) const;
	virtual std::string getAddress(void) const;
	virtual std::string getHostName(void) const;
	virtual int getPeerPortId(void) const;
	virtual std::string getPeerAddress(void) const;
	virtual std::string getPeerHostName(void) const;
	};

}

#endif
