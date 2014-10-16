/***********************************************************************
TCPPipe - Class for high-performance reading/writing from/to connected
TCP sockets.
Copyright (c) 2010-2013 Oliver Kreylos

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

#include <Comm/TCPPipe.h>

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/FdSet.h>
#include <Comm/ListeningTCPSocket.h>

namespace Comm {

/************************
Methods of class TCPPipe:
************************/

size_t TCPPipe::readData(IO::File::Byte* buffer,size_t bufferSize)
	{
	/* Read more data from source: */
	ssize_t readResult;
	do
		{
		readResult=::read(fd,buffer,bufferSize);
		}
	while(readResult<0&&(errno==EAGAIN||errno==EWOULDBLOCK||errno==EINTR));
	
	/* Handle the result from the read call: */
	if(readResult<0)
		{
		/* Unknown error; probably a bad thing: */
		int errorCode=errno;
		Misc::throwStdErr("Comm::TCPPipe: Fatal error %d while reading from source",errorCode);
		}
	
	return size_t(readResult);
	}

void TCPPipe::writeData(const IO::File::Byte* buffer,size_t bufferSize)
	{
	while(bufferSize>0)
		{
		ssize_t writeResult=::write(fd,buffer,bufferSize);
		if(writeResult>0)
			{
			/* Prepare to write more data: */
			buffer+=writeResult;
			bufferSize-=writeResult;
			}
		else if(errno==EPIPE)
			{
			/* Other side hung up: */
			Misc::throwStdErr("Comm::TCPPipe: Connection terminated by peer");
			}
		else if(writeResult<0&&(errno==EAGAIN||errno==EWOULDBLOCK||errno==EINTR))
			{
			/* Do nothing and try again */
			}
		else if(writeResult==0)
			{
			/* Sink has reached end-of-file: */
			throw WriteError(bufferSize);
			}
		else
			{
			/* Unknown error; probably a bad thing: */
			int errorCode=errno;
			Misc::throwStdErr("Comm::TCPPipe: Fatal error %d while writing to sink",errorCode);
			}
		}
	}

TCPPipe::TCPPipe(const char* hostName,int portId)
	:NetPipe(ReadWrite),
	 fd(-1)
	{
	/* Create the socket file descriptor: */
	fd=socket(PF_INET,SOCK_STREAM,0);
	if(fd<0)
		Misc::throwStdErr("Comm::TCPPipe::TCPPipe: Unable to create socket");
	
	/* Bind the socket file descriptor: */
	struct sockaddr_in mySocketAddress;
	mySocketAddress.sin_family=AF_INET;
	mySocketAddress.sin_port=0;
	mySocketAddress.sin_addr.s_addr=htonl(INADDR_ANY);
	if(bind(fd,(struct sockaddr*)&mySocketAddress,sizeof(struct sockaddr_in))==-1)
		{
		close(fd);
		fd=-1;
		Misc::throwStdErr("Comm::TCPPipe::TCPPipe: Unable to bind socket to port");
		}
	
	/* Lookup host's IP address: */
	struct hostent* hostEntry=gethostbyname(hostName);
	if(hostEntry==0)
		{
		close(fd);
		fd=-1;
		Misc::throwStdErr("Comm::TCPPipe::TCPPipe: Unable to resolve host name %s",hostName);
		}
	struct in_addr hostNetAddress;
	hostNetAddress.s_addr=ntohl(((struct in_addr*)hostEntry->h_addr_list[0])->s_addr);
	
	/* Connect to the remote host: */
	struct sockaddr_in hostAddress;
	hostAddress.sin_family=AF_INET;
	hostAddress.sin_port=htons(portId);
	hostAddress.sin_addr.s_addr=htonl(hostNetAddress.s_addr);
	if(connect(fd,(const struct sockaddr*)&hostAddress,sizeof(struct sockaddr_in))==-1)
		{
		close(fd);
		fd=-1;
		Misc::throwStdErr("Comm::TCPPipe::TCPPipe: Unable to connect to host %s on port %d",hostName,portId);
		}
	
	/* Set the TCP_NODELAY socket option: */
	int flag=1;
	if(setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,&flag,sizeof(flag))==-1)
		{
		close(fd);
		fd=-1;
		Misc::throwStdErr("Cluster::TCPPipe::TCPPipe: Unable to disable Nagle's algorithm on socket");
		}
	}

TCPPipe::TCPPipe(ListeningTCPSocket& listenSocket)
	:NetPipe(ReadWrite),
	 fd(-1)
	{
	/* Wait for a connection attempt on the listening socket: */
	fd=accept(listenSocket.getFd(),0,0);
	if(fd==-1)
		Misc::throwStdErr("Comm::TCPPipe::TCPPipe: Unable to accept connection");
	
	/* Set the TCP_NODELAY socket option: */
	int flag=1;
	setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,&flag,sizeof(flag));
	}

TCPPipe::~TCPPipe(void)
	{
	/* Flush the write buffer: */
	flush();
	
	/* Close the socket: */
	if(fd>=0)
		close(fd);
	}

int TCPPipe::getFd(void) const
	{
	return fd;
	}

bool TCPPipe::waitForData(void) const
	{
	/* Check if there is unread data in the buffer: */
	if(getUnreadDataSize()>0)
		return true;
	
	/* Wait for data on the socket and return whether data is available: */
	Misc::FdSet readFds(fd);
	return Misc::pselect(&readFds,0,0,0)>=0&&readFds.isSet(fd);
	}

bool TCPPipe::waitForData(const Misc::Time& timeout) const
	{
	/* Check if there is unread data in the buffer: */
	if(getUnreadDataSize()>0)
		return true;
	
	/* Wait for data on the socket and return whether data is available: */
	Misc::FdSet readFds(fd);
	return Misc::pselect(&readFds,0,0,timeout)>=0&&readFds.isSet(fd);
	}

void TCPPipe::shutdown(bool read,bool write)
	{
	/* Flush the write buffer: */
	flush();
	
	/* Shut down the socket: */
	if(read&&write)
		::shutdown(fd,SHUT_RDWR);
	else if(read)
		::shutdown(fd,SHUT_RD);
	else if(write)
		::shutdown(fd,SHUT_WR);
	}

int TCPPipe::getPortId(void) const
	{
	struct sockaddr_in socketAddress;
	#ifdef __SGI_IRIX__
	int socketAddressLen=sizeof(struct sockaddr_in);
	#else
	socklen_t socketAddressLen=sizeof(struct sockaddr_in);
	#endif
	getsockname(fd,(struct sockaddr*)&socketAddress,&socketAddressLen);
	return ntohs(socketAddress.sin_port);
	}

std::string TCPPipe::getAddress(void) const
	{
	struct sockaddr_in socketAddress;
	#ifdef __SGI_IRIX__
	int socketAddressLen=sizeof(struct sockaddr_in);
	#else
	socklen_t socketAddressLen=sizeof(struct sockaddr_in);
	#endif
	getsockname(fd,(struct sockaddr*)&socketAddress,&socketAddressLen);
	char resultBuffer[INET_ADDRSTRLEN];
	inet_ntop(AF_INET,&socketAddress.sin_addr,resultBuffer,INET_ADDRSTRLEN);
	return std::string(resultBuffer);
	}

std::string TCPPipe::getHostName(void) const
	{
	struct sockaddr_in socketAddress;
	#ifdef __SGI_IRIX__
	int socketAddressLen=sizeof(struct sockaddr_in);
	#else
	socklen_t socketAddressLen=sizeof(struct sockaddr_in);
	#endif
	getsockname(fd,(struct sockaddr*)&socketAddress,&socketAddressLen);
	
	/* Lookup host's name: */
	std::string result;
	struct hostent* hostEntry=gethostbyaddr((const char*)&socketAddress.sin_addr,sizeof(struct in_addr),AF_INET);
	if(hostEntry==0)
		{
		/* Fall back to returning address in dotted notation: */
		char addressBuffer[INET_ADDRSTRLEN];
		inet_ntop(AF_INET,&socketAddress.sin_addr,addressBuffer,INET_ADDRSTRLEN);
		result=std::string(addressBuffer);
		}
	else
		result=std::string(hostEntry->h_name);
	
	return result;
	}

int TCPPipe::getPeerPortId(void) const
	{
	struct sockaddr_in peerAddress;
	#ifdef __SGI_IRIX__
	int peerAddressLen=sizeof(struct sockaddr_in);
	#else
	socklen_t peerAddressLen=sizeof(struct sockaddr_in);
	#endif
	getpeername(fd,(struct sockaddr*)&peerAddress,&peerAddressLen);
	return ntohs(peerAddress.sin_port);
	}

std::string TCPPipe::getPeerAddress(void) const
	{
	struct sockaddr_in peerAddress;
	#ifdef __SGI_IRIX__
	int peerAddressLen=sizeof(struct sockaddr_in);
	#else
	socklen_t peerAddressLen=sizeof(struct sockaddr_in);
	#endif
	getpeername(fd,(struct sockaddr*)&peerAddress,&peerAddressLen);
	char resultBuffer[INET_ADDRSTRLEN];
	inet_ntop(AF_INET,&peerAddress.sin_addr,resultBuffer,INET_ADDRSTRLEN);
	return std::string(resultBuffer);
	}

std::string TCPPipe::getPeerHostName(void) const
	{
	struct sockaddr_in peerAddress;
	#ifdef __SGI_IRIX__
	int peerAddressLen=sizeof(struct sockaddr_in);
	#else
	socklen_t peerAddressLen=sizeof(struct sockaddr_in);
	#endif
	getpeername(fd,(struct sockaddr*)&peerAddress,&peerAddressLen);
	
	/* Lookup host's name: */
	std::string result;
	struct hostent* hostEntry=gethostbyaddr((const char*)&peerAddress.sin_addr,sizeof(struct in_addr),AF_INET);
	if(hostEntry==0)
		{
		/* Fall back to returning address in dotted notation: */
		char addressBuffer[INET_ADDRSTRLEN];
		inet_ntop(AF_INET,&peerAddress.sin_addr,addressBuffer,INET_ADDRSTRLEN);
		result=std::string(addressBuffer);
		}
	else
		result=std::string(hostEntry->h_name);
	
	return result;
	}

}
