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

#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <Misc/ThrowStdErr.h>

#include <Comm/UDPSocket.h>

namespace Comm {

/**************************
Methods of class UPDSocket:
**************************/

UDPSocket::UDPSocket(int localPortId,int)
	{
	/* Create the socket file descriptor: */
	socketFd=socket(PF_INET,SOCK_DGRAM,0);
	if(socketFd<0)
		Misc::throwStdErr("Comm::UDPSocket: Unable to create socket");
	
	/* Bind the socket file descriptor to the local port ID: */
	struct sockaddr_in socketAddress;
	socketAddress.sin_family=AF_INET;
	socketAddress.sin_port=localPortId>=0?htons(localPortId):0;
	socketAddress.sin_addr.s_addr=htonl(INADDR_ANY);
	if(bind(socketFd,(struct sockaddr*)&socketAddress,sizeof(struct sockaddr_in))==-1)
		{
		close(socketFd);
		Misc::throwStdErr("Comm::UDPSocket: Unable to bind socket to port %d",localPortId);
		}
	}

UDPSocket::UDPSocket(int localPortId,std::string hostname,int hostPortId)
	{
	/* Create the socket file descriptor: */
	socketFd=socket(PF_INET,SOCK_DGRAM,0);
	if(socketFd<0)
		Misc::throwStdErr("Comm::UDPSocket: Unable to create socket");
	
	/* Bind the socket file descriptor to the local port ID: */
	struct sockaddr_in mySocketAddress;
	mySocketAddress.sin_family=AF_INET;
	mySocketAddress.sin_port=localPortId>=0?htons(localPortId):0;
	mySocketAddress.sin_addr.s_addr=htonl(INADDR_ANY);
	if(bind(socketFd,(struct sockaddr*)&mySocketAddress,sizeof(struct sockaddr_in))==-1)
		{
		close(socketFd);
		Misc::throwStdErr("Comm::UDPSocket: Unable to bind socket to port %d",localPortId);
		}
	
	/* Lookup host's IP address: */
	struct hostent* hostEntry=gethostbyname(hostname.c_str());
	if(hostEntry==0)
		{
		close(socketFd);
		Misc::throwStdErr("Comm::UDPSocket: Unable to resolve host name %s",hostname.c_str());
		}
	struct in_addr hostNetAddress;
	hostNetAddress.s_addr=ntohl(((struct in_addr*)hostEntry->h_addr_list[0])->s_addr);
	
	/* Connect to the remote host: */
	struct sockaddr_in hostAddress;
	hostAddress.sin_family=AF_INET;
	hostAddress.sin_port=htons(hostPortId);
	hostAddress.sin_addr.s_addr=htonl(hostNetAddress.s_addr);
	if(::connect(socketFd,(const struct sockaddr*)&hostAddress,sizeof(struct sockaddr_in))==-1)
		{
		close(socketFd);
		Misc::throwStdErr("Comm::UDPSocket: Unable to connect to host %s on port %d",hostname.c_str(),hostPortId);
		}
	}

UDPSocket::UDPSocket(const UDPSocket& source)
	:socketFd(dup(source.socketFd))
	{
	}

UDPSocket::~UDPSocket(void)
	{
	close(socketFd);
	}

UDPSocket& UDPSocket::operator=(const UDPSocket& source)
	{
	if(this!=&source)
		{
		close(socketFd);
		socketFd=dup(source.socketFd);
		}
	return *this;
	}

int UDPSocket::getPortId(void) const
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

void UDPSocket::connect(std::string hostname,int hostPortId)
	{
	/* Lookup host's IP address: */
	struct hostent* hostEntry=gethostbyname(hostname.c_str());
	if(hostEntry==0)
		Misc::throwStdErr("Comm::UDPSocket: Unable to resolve host name %s",hostname.c_str());
	struct in_addr hostNetAddress;
	hostNetAddress.s_addr=ntohl(((struct in_addr*)hostEntry->h_addr_list[0])->s_addr);
	
	/* Connect to the remote host: */
	struct sockaddr_in hostAddress;
	hostAddress.sin_family=AF_INET;
	hostAddress.sin_port=htons(hostPortId);
	hostAddress.sin_addr.s_addr=htonl(hostNetAddress.s_addr);
	if(::connect(socketFd,(const struct sockaddr*)&hostAddress,sizeof(struct sockaddr_in))==-1)
		Misc::throwStdErr("Comm::UDPSocket: Unable to connect to host %s on port %d",hostname.c_str(),hostPortId);
	}

void UDPSocket::accept(void)
	{
	/* Wait for an incoming message: */
	char buffer[256];
	struct sockaddr_in senderAddress;
	socklen_t senderAddressLen=sizeof(struct sockaddr_in);
	ssize_t numBytesReceived=recvfrom(socketFd,buffer,sizeof(buffer),0,(struct sockaddr*)&senderAddress,&senderAddressLen);
	if(numBytesReceived<0||size_t(numBytesReceived)>sizeof(buffer))
		Misc::throwStdErr("Comm::UDPSocket: Fatal error during accept");
	
	/* Connect to the sender: */
	if(::connect(socketFd,(const struct sockaddr*)&senderAddress,sizeof(struct sockaddr_in))==-1)
		Misc::throwStdErr("Comm::UDPSocket: Unable to connect to message sender");
	}

void UDPSocket::sendMessage(const void* messageBuffer,size_t messageSize)
	{
	ssize_t sendResult;
	do
		{
		sendResult=send(socketFd,messageBuffer,messageSize,0);
		}
	while(sendResult<0&&errno==EINTR);
	if(sendResult<0)
		{
		/* Consider this a fatal error: */
		int errorCode=errno;
		Misc::throwStdErr("Comm::UDPSocket: Fatal error %d while sending message",errorCode);
		}
	else if(size_t(sendResult)!=messageSize)
		{
		/* Message was truncated during send: */
		Misc::throwStdErr("Comm::UDPSocket: Truncation from %u to %u while sending message",(unsigned int)messageSize,(unsigned int)sendResult);
		}
	}

size_t UDPSocket::receiveMessage(void* messageBuffer,size_t messageSize)
	{
	/* Receive a message: */
	ssize_t recvResult;
	do
		{
		recvResult=recv(socketFd,messageBuffer,messageSize,0);
		}
	while(recvResult<0&&(errno==EAGAIN||errno==EWOULDBLOCK||errno==EINTR));
	
	/* Handle the result from the recv call: */
	if(recvResult<0)
		{
		/* Unknown error; probably a bad thing: */
		int errorCode=errno;
		Misc::throwStdErr("Comm::UDPSocket: Fatal error %d while receiving message",errorCode);
		}
	
	return size_t(recvResult);
	}

}
