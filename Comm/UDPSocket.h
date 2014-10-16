/***********************************************************************
UDPSocket - Wrapper class for UDP sockets ensuring exception safety.
Copyright (c) 2004-2012 Oliver Kreylos

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

#ifndef COMM_UDPSOCKET_INCLUDED
#define COMM_UDPSOCKET_INCLUDED

#include <string>
#include <stdexcept>

namespace Comm {

class UDPSocket
	{
	/* Embedded classes: */
	public:
	class TimeOut:public std::runtime_error // Exception for time-outs when waiting for data
		{
		/* Constructors and destructors: */
		public:
		TimeOut(const std::string& what_arg)
			:std::runtime_error(what_arg)
			{
			}
		};
	
	/* Elements: */
	private:
	int socketFd; // Internal socket file descriptor
	
	/* Constructors and destructors: */
	public:
	UDPSocket(void) // Creates an invalid UDP socket
		:socketFd(-1)
		{
		}
	private:
	UDPSocket(int sSocketFd) // Creates a UDPSocket wrapper around an existing socket file descriptor (without copying)
		:socketFd(sSocketFd)
		{
		}
	public:
	UDPSocket(int localPortId,int backlog); // Creates an unconnected socket on the local host; if portId is negative, random free port is assigned
	UDPSocket(int localPortId,std::string hostname,int hostPortId); // Creates a socket connected to a remote host; if localPortId is negative, random free port is assigned
	UDPSocket(const UDPSocket& source); // Copy constructor
	~UDPSocket(void); // Closes a socket
	
	/* Methods: */
	int getFd(void) // Returns low-level socket file descriptor
		{
		return socketFd;
		}
	UDPSocket& operator=(const UDPSocket& source); // Assignment operator
	int getPortId(void) const; // Returns port ID assigned to a socket
	void connect(std::string hostname,int hostPortId); // Connects the socket to a remote host; throws exception (but does not close socket) on failure
	void accept(void); // Waits for a (short) incoming message on an unconnected socket and connects to the sender of the message; discards message
	
	/* I/O methods: */
	void sendMessage(const void* messageBuffer,size_t messageSize); // Sends a message on a connected socket
	size_t receiveMessage(void* messageBuffer,size_t messageBufferSize); // Receives a message; returns size of received message
	};

}

#endif
