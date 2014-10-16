/***********************************************************************
TCPSocket - Wrapper class for TCP sockets ensuring exception safety.
Copyright (c) 2002-2005 Oliver Kreylos

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

#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/Time.h>

#include <Comm/TCPSocket.h>

namespace Comm {

/**************************
Methods of class TCPSocket:
**************************/

TCPSocket::TCPSocket(int portId,int backlog)
	:socketFd(-1)
	{
	/* Create the socket file descriptor: */
	socketFd=socket(PF_INET,SOCK_STREAM,0);
	if(socketFd<0)
		Misc::throwStdErr("TCPSocket: Unable to create socket");
	
	/* Bind the socket file descriptor to the port ID: */
	struct sockaddr_in socketAddress;
	socketAddress.sin_family=AF_INET;
	socketAddress.sin_port=portId>=0?htons(portId):0;
	socketAddress.sin_addr.s_addr=htonl(INADDR_ANY);
	if(bind(socketFd,(struct sockaddr*)&socketAddress,sizeof(struct sockaddr_in))==-1)
		{
		close(socketFd);
		socketFd=-1;
		Misc::throwStdErr("TCPSocket: Unable to bind socket to port %d",portId);
		}
	
	/* Start listening on the socket: */
	if(listen(socketFd,backlog)==-1)
		{
		close(socketFd);
		socketFd=-1;
		Misc::throwStdErr("TCPSocket: Unable to start listening on socket");
		}
	}

TCPSocket::TCPSocket(std::string hostname,int portId)
	:socketFd(-1)
	{
	/* Create the socket file descriptor: */
	socketFd=socket(PF_INET,SOCK_STREAM,0);
	if(socketFd<0)
		Misc::throwStdErr("TCPSocket: Unable to create socket");
	
	/* Bind the socket file descriptor: */
	struct sockaddr_in mySocketAddress;
	mySocketAddress.sin_family=AF_INET;
	mySocketAddress.sin_port=0;
	mySocketAddress.sin_addr.s_addr=htonl(INADDR_ANY);
	if(bind(socketFd,(struct sockaddr*)&mySocketAddress,sizeof(struct sockaddr_in))==-1)
		{
		close(socketFd);
		socketFd=-1;
		Misc::throwStdErr("TCPSocket: Unable to bind socket to port");
		}
	
	/* Lookup host's IP address: */
	struct hostent* hostEntry=gethostbyname(hostname.c_str());
	if(hostEntry==0)
		{
		close(socketFd);
		socketFd=-1;
		Misc::throwStdErr("TCPSocket: Unable to resolve host name %s",hostname.c_str());
		}
	struct in_addr hostNetAddress;
	hostNetAddress.s_addr=ntohl(((struct in_addr*)hostEntry->h_addr_list[0])->s_addr);
	
	/* Connect to the remote host: */
	struct sockaddr_in hostAddress;
	hostAddress.sin_family=AF_INET;
	hostAddress.sin_port=htons(portId);
	hostAddress.sin_addr.s_addr=htonl(hostNetAddress.s_addr);
	if(::connect(socketFd,(const struct sockaddr*)&hostAddress,sizeof(struct sockaddr_in))==-1)
		{
		close(socketFd);
		socketFd=-1;
		Misc::throwStdErr("TCPSocket: Unable to connect to host %s on port %d",hostname.c_str(),portId);
		}
	}

TCPSocket::TCPSocket(const TCPSocket& source)
	:socketFd(dup(source.socketFd))
	{
	}

TCPSocket& TCPSocket::operator=(const TCPSocket& source)
	{
	if(this!=&source)
		{
		if(socketFd>=0)
			close(socketFd);
		socketFd=dup(source.socketFd);
		}
	return *this;
	}

TCPSocket::~TCPSocket(void)
	{
	if(socketFd>=0)
		close(socketFd);
	}

TCPSocket& TCPSocket::connect(std::string hostname,int portId)
	{
	/* Close a previous connection: */
	if(socketFd>=0)
		{
		close(socketFd);
		socketFd=-1;
		}
	
	/* Create a new socket file descriptor: */
	socketFd=socket(PF_INET,SOCK_STREAM,0);
	if(socketFd<0)
		Misc::throwStdErr("TCPSocket::connect: Unable to create socket");
	
	/* Bind the socket file descriptor: */
	struct sockaddr_in mySocketAddress;
	mySocketAddress.sin_family=AF_INET;
	mySocketAddress.sin_port=0;
	mySocketAddress.sin_addr.s_addr=htonl(INADDR_ANY);
	if(bind(socketFd,(struct sockaddr*)&mySocketAddress,sizeof(struct sockaddr_in))==-1)
		{
		close(socketFd);
		socketFd=-1;
		Misc::throwStdErr("TCPSocket::connect: Unable to bind socket to port");
		}
	
	/* Lookup host's IP address: */
	struct hostent* hostEntry=gethostbyname(hostname.c_str());
	if(hostEntry==0)
		{
		close(socketFd);
		socketFd=-1;
		Misc::throwStdErr("TCPSocket::connect: Unable to resolve host name %s",hostname.c_str());
		}
	struct in_addr hostNetAddress;
	hostNetAddress.s_addr=ntohl(((struct in_addr*)hostEntry->h_addr_list[0])->s_addr);
	
	/* Connect to the remote host: */
	struct sockaddr_in hostAddress;
	hostAddress.sin_family=AF_INET;
	hostAddress.sin_port=htons(portId);
	hostAddress.sin_addr.s_addr=htonl(hostNetAddress.s_addr);
	if(::connect(socketFd,(const struct sockaddr*)&hostAddress,sizeof(struct sockaddr_in))==-1)
		{
		close(socketFd);
		socketFd=-1;
		Misc::throwStdErr("TCPSocket::connect: Unable to connect to host %s on port %d",hostname.c_str(),portId);
		}
	
	return *this;
	}

int TCPSocket::getPortId(void) const
	{
	struct sockaddr_in socketAddress;
	#ifdef __SGI_IRIX__
	int socketAddressLen=sizeof(struct sockaddr_in);
	#else
	socklen_t socketAddressLen=sizeof(struct sockaddr_in);
	#endif
	getsockname(socketFd,(struct sockaddr*)&socketAddress,&socketAddressLen);
	return ntohs(socketAddress.sin_port);
	}

std::string TCPSocket::getAddress(void) const
	{
	struct sockaddr_in socketAddress;
	#ifdef __SGI_IRIX__
	int socketAddressLen=sizeof(struct sockaddr_in);
	#else
	socklen_t socketAddressLen=sizeof(struct sockaddr_in);
	#endif
	getsockname(socketFd,(struct sockaddr*)&socketAddress,&socketAddressLen);
	char resultBuffer[INET_ADDRSTRLEN];
	inet_ntop(AF_INET,&socketAddress.sin_addr,resultBuffer,INET_ADDRSTRLEN);
	return std::string(resultBuffer);
	}

std::string TCPSocket::getHostname(bool throwException) const
	{
	struct sockaddr_in socketAddress;
	#ifdef __SGI_IRIX__
	int socketAddressLen=sizeof(struct sockaddr_in);
	#else
	socklen_t socketAddressLen=sizeof(struct sockaddr_in);
	#endif
	getsockname(socketFd,(struct sockaddr*)&socketAddress,&socketAddressLen);
	
	/* Lookup host's name: */
	std::string result;
	struct hostent* hostEntry=gethostbyaddr((const char*)&socketAddress.sin_addr,sizeof(struct in_addr),AF_INET);
	if(hostEntry==0)
		{
		/* Fall back to returning address in dotted notation or throwing exception: */
		char addressBuffer[INET_ADDRSTRLEN];
		inet_ntop(AF_INET,&socketAddress.sin_addr,addressBuffer,INET_ADDRSTRLEN);
		if(throwException)
			Misc::throwStdErr("TCPSocket::getHostname: Cannot resolve host address %s",addressBuffer);
		else
			result=std::string(addressBuffer);
		}
	else
		result=std::string(hostEntry->h_name);
	
	return result;
	}

TCPSocket TCPSocket::accept(void) const
	{
	/* Wait for connection attempts: */
	int newSocketFd=::accept(socketFd,0,0);
	if(newSocketFd==-1)
		Misc::throwStdErr("TCPSocket: Unable to accept connection");
	return TCPSocket(newSocketFd);
	}

int TCPSocket::getPeerPortId(void) const
	{
	struct sockaddr_in peerAddress;
	#ifdef __SGI_IRIX__
	int peerAddressLen=sizeof(struct sockaddr_in);
	#else
	socklen_t peerAddressLen=sizeof(struct sockaddr_in);
	#endif
	getpeername(socketFd,(struct sockaddr*)&peerAddress,&peerAddressLen);
	return ntohs(peerAddress.sin_port);
	}

std::string TCPSocket::getPeerAddress(void) const
	{
	struct sockaddr_in peerAddress;
	#ifdef __SGI_IRIX__
	int peerAddressLen=sizeof(struct sockaddr_in);
	#else
	socklen_t peerAddressLen=sizeof(struct sockaddr_in);
	#endif
	getpeername(socketFd,(struct sockaddr*)&peerAddress,&peerAddressLen);
	char resultBuffer[INET_ADDRSTRLEN];
	inet_ntop(AF_INET,&peerAddress.sin_addr,resultBuffer,INET_ADDRSTRLEN);
	return std::string(resultBuffer);
	}

std::string TCPSocket::getPeerHostname(bool throwException) const
	{
	struct sockaddr_in peerAddress;
	#ifdef __SGI_IRIX__
	int peerAddressLen=sizeof(struct sockaddr_in);
	#else
	socklen_t peerAddressLen=sizeof(struct sockaddr_in);
	#endif
	getpeername(socketFd,(struct sockaddr*)&peerAddress,&peerAddressLen);
	
	/* Lookup host's name: */
	std::string result;
	struct hostent* hostEntry=gethostbyaddr((const char*)&peerAddress.sin_addr,sizeof(struct in_addr),AF_INET);
	if(hostEntry==0)
		{
		/* Fall back to returning address in dotted notation or throwing exception: */
		char addressBuffer[INET_ADDRSTRLEN];
		inet_ntop(AF_INET,&peerAddress.sin_addr,addressBuffer,INET_ADDRSTRLEN);
		if(throwException)
			Misc::throwStdErr("TCPSocket::getPeerHostname: Cannot resolve host address %s",addressBuffer);
		else
			result=std::string(addressBuffer);
		}
	else
		result=std::string(hostEntry->h_name);
	
	return result;
	}

void TCPSocket::shutdown(bool shutdownRead,bool shutdownWrite)
	{
	if(shutdownRead&&shutdownWrite)
		{
		if(::shutdown(socketFd,SHUT_RDWR)!=0)
			Misc::throwStdErr("TCPSocket:: Error while shutting down read and write");
		}
	else if(shutdownRead)
		{
		if(::shutdown(socketFd,SHUT_RD)!=0)
			Misc::throwStdErr("TCPSocket:: Error while shutting down read");
		}
	else if(shutdownWrite)
		{
		if(::shutdown(socketFd,SHUT_WR)!=0)
			Misc::throwStdErr("TCPSocket:: Error while shutting down write");
		}
	}

bool TCPSocket::getNoDelay(void) const
	{
	/* Get the TCP_NODELAY socket option: */
	int flag;
	socklen_t flagLen=sizeof(int);
	getsockopt(socketFd,IPPROTO_TCP,TCP_NODELAY,&flag,&flagLen);
	return flag!=0;
	}

void TCPSocket::setNoDelay(bool enable)
	{
	/* Set the TCP_NODELAY socket option: */
	int flag=enable?1:0;
	setsockopt(socketFd,IPPROTO_TCP,TCP_NODELAY,&flag,sizeof(int));
	}

bool TCPSocket::getCork(void) const
	{
	#ifdef __linux__
	/* Get the TCP_CORK socket option: */
	int flag;
	socklen_t flagLen=sizeof(int);
	getsockopt(socketFd,IPPROTO_TCP,TCP_CORK,&flag,&flagLen);
	return flag!=0;
	#else
	return false;
	#endif
	}

void TCPSocket::setCork(bool enable)
	{
	#ifdef __linux__
	/* Set the TCP_CORK socket option: */
	int flag=enable?1:0;
	setsockopt(socketFd,IPPROTO_TCP,TCP_CORK,&flag,sizeof(int));
	#endif
	}

bool TCPSocket::waitForData(long timeoutSeconds,long timeoutMicroseconds,bool throwException) const
	{
	fd_set readFdSet;
	FD_ZERO(&readFdSet);
	FD_SET(socketFd,&readFdSet);
	struct timeval tv;
	tv.tv_sec=timeoutSeconds;
	tv.tv_usec=timeoutMicroseconds;
	bool dataWaiting=select(socketFd+1,&readFdSet,0,0,&tv)>=0&&FD_ISSET(socketFd,&readFdSet);
	if(throwException&&!dataWaiting)
		throw TimeOut("TCPSocket: Time-out while waiting for data");
	return dataWaiting;
	}

bool TCPSocket::waitForData(const Misc::Time& timeout,bool throwException) const
	{
	fd_set readFdSet;
	FD_ZERO(&readFdSet);
	FD_SET(socketFd,&readFdSet);
	struct timeval tv=timeout;
	bool dataWaiting=select(socketFd+1,&readFdSet,0,0,&tv)>=0&&FD_ISSET(socketFd,&readFdSet);
	if(throwException&&!dataWaiting)
		throw TimeOut("TCPSocket: Time-out while waiting for data");
	return dataWaiting;
	}

size_t TCPSocket::read(void* buffer,size_t count)
	{
	char* byteBuffer=reinterpret_cast<char*>(buffer);
	ssize_t numBytesRead=::read(socketFd,byteBuffer,count);
	if(numBytesRead>0||errno==EAGAIN)
		return size_t(numBytesRead);
	else if(numBytesRead==0)
		{
		/* Other end terminated connection: */
		throw PipeError("TCPSocket: Connection terminated by peer during read");
		}
	else
		{
		/* Consider this a fatal error: */
		Misc::throwStdErr("TCPSocket: Fatal error during read");
		}
	
	/* Dummy statement to make compiler happy: */
	return 0;
	}

void TCPSocket::blockingRead(void* buffer,size_t count)
	{
	char* byteBuffer=reinterpret_cast<char*>(buffer);
	while(count>0)
		{
		ssize_t numBytesRead=::read(socketFd,byteBuffer,count);
		if(numBytesRead!=ssize_t(count))
			{
			if(numBytesRead>0)
				{
				/* Advance result pointer and retry: */
				count-=numBytesRead;
				byteBuffer+=numBytesRead;
				}
			else if(errno==EAGAIN||errno==EINTR)
				{
				/* Do nothing */
				}
			else if(numBytesRead==0)
				{
				/* Other end terminated connection: */
				throw PipeError("TCPSocket: Connection terminated by peer during read");
				}
			else
				{
				/* Consider this a fatal error: */
				Misc::throwStdErr("TCPSocket: Fatal error during read");
				}
			}
		else
			count=0;
		}
	}

void TCPSocket::blockingWrite(const void* buffer,size_t count)
	{
	const char* byteBuffer=reinterpret_cast<const char*>(buffer);
	while(count>0)
		{
		ssize_t numBytesWritten=::write(socketFd,byteBuffer,count);
		if(numBytesWritten!=ssize_t(count))
			{
			/* Check error condition: */
			if(numBytesWritten>=0||errno==EAGAIN||errno==EINTR)
				{
				/* Advance data pointer and try again: */
				count-=numBytesWritten;
				byteBuffer+=numBytesWritten;
				}
			else if(errno==EPIPE)
				{
				/* Other end terminated connection: */
				throw PipeError("TCPSocket: Connection terminated by peer during write");
				}
			else
				{
				/* Consider this a fatal error: */
				Misc::throwStdErr("TCPSocket: Fatal error during write");
				}
			}
		else
			count=0;
		}
	}

void TCPSocket::flush(void)
	{
	#ifdef __linux__
	/* Twiddle the TCP_CORK socket option to flush half-assembled packets: */
	int flag=0;
	setsockopt(socketFd,IPPROTO_TCP,TCP_CORK,&flag,sizeof(int));
	flag=1;
	setsockopt(socketFd,IPPROTO_TCP,TCP_CORK,&flag,sizeof(int));
	#endif
	}

}
