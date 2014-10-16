/***********************************************************************
TCPSocket - Wrapper class for TCP sockets ensuring exception safety and
improved latency/throughput by supporting TCP_NODELAY and TCP_CORK where
available.
Copyright (c) 2002-2007 Oliver Kreylos

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

#ifndef COMM_TCPSOCKET_INCLUDED
#define COMM_TCPSOCKET_INCLUDED

#include <string>
#include <stdexcept>

/* Forward declarations: */
namespace Misc {
class Time;
}

namespace Comm {

class TCPSocket
	{
	/* Embedded classes: */
	public:
	class PipeError:public std::runtime_error // Exception for unexpected connection termination
		{
		/* Constructors and destructors: */
		public:
		PipeError(const std::string& what_arg)
			:std::runtime_error(what_arg)
			{
			}
		};
	
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
	TCPSocket(void) // Creates an invalid TCP socket
		:socketFd(-1)
		{
		}
	private:
	TCPSocket(int sSocketFd) // Creates a TCPSocket wrapper around an existing socket file descriptor (without copying)
		:socketFd(sSocketFd)
		{
		}
	public:
	TCPSocket(int portId,int backlog); // Creates a socket on the local host and starts listening; if portId is negative, random free port is assigned
	TCPSocket(std::string hostname,int portId); // Creates a socket connected to a remote host
	TCPSocket(const TCPSocket& source); // Copy constructor
	TCPSocket& operator=(const TCPSocket& source); // Assignment operator
	~TCPSocket(void); // Closes a socket
	
	/* Methods: */
	int getFd(void) // Returns low-level socket file descriptor
		{
		return socketFd;
		}
	TCPSocket& connect(std::string hostname,int portId); // Connects an existing socket to a remote host; closes previous connection
	int getPortId(void) const; // Returns port ID assigned to a socket
	std::string getAddress(void) const; // Returns internet address assigned to a socket in dotted notation
	std::string getHostname(bool throwException =true) const; // Returns host name of socket; throws exception if host name cannot be resolved and flag is true
	TCPSocket accept(void) const; // Waits for an incoming connection on a listening socket and returns a new socket connected to the initiator
	int getPeerPortId(void) const; // Returns port ID of remote socket
	std::string getPeerAddress(void) const; // Returns internet address of remote socket in dotted notation
	std::string getPeerHostname(bool throwException =true) const; // Returns host name of remote socket; throws exception if host name cannot be resolved and flag is true
	void shutdown(bool shutdownRead,bool shutdownWrite); // Shuts down the read or write part of a socket; further reads or writes are not allowed
	
	/* Options methods: */
	bool getNoDelay(void) const; // Returns true if TCP_NODELAY is enabled
	void setNoDelay(bool enable); // Enables/disables TCP_NODELAY (disabled by default)
	bool getCork(void) const; // Returns true if packet assembly using TCP_CORK is enabled
	void setCork(bool enable); // Enables/disables packet assembly using TCP_CORK (disabled by default); requires using flush() when enabled
	
	/* Raw I/O methods: */
	bool waitForData(long timeoutSeconds,long timeoutMicroseconds,bool throwException =true) const; // Waits for incoming data on TCP socket; returns true if data is ready; (optionally) throws exception if wait times out
	bool waitForData(const Misc::Time& timeout,bool throwException =true) const; // Waits for incoming data on TCP socket; returns true if data is ready; (optionally) throws exception if wait times out
	size_t read(void* buffer,size_t count); // Reads raw buffer from TCP socket; returns number of bytes read
	void blockingRead(void* buffer,size_t count); // Reads raw buffer from TCP socket; blocks until data completely read
	void readRaw(void* data,size_t dataSize) // Alternate name for blockingRead method to get APIs unified
		{
		blockingRead(data,dataSize);
		}
	void blockingWrite(const void* buffer,size_t count); // Writes raw buffer to TCP socket; blocks until data completely written
	void writeRaw(const void* data,size_t dataSize) // Alternate name for blockingWrite method to get APIs unified
		{
		blockingWrite(data,dataSize);
		}
	void flush(void); // Send any data in the socket's send buffer immediately (required at message boundaries when TCP_CORK is enabled)
	};

}

#endif
