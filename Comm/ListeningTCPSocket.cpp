/***********************************************************************
ListeningTCPSocket - Class for TCP half-sockets that can accept incoming
connections.
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

#include <Comm/ListeningTCPSocket.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <Misc/ThrowStdErr.h>

#include <errno.h>
#include <unistd.h>
#include <sys/time.h>

namespace Comm {

/***********************************
Methods of class ListeningTCPSocket:
***********************************/

ListeningTCPSocket::ListeningTCPSocket(int portId,int backlog)
	:fd(-1)
	{
	/* Create the socket file descriptor: */
	fd=socket(PF_INET,SOCK_STREAM,0);
	if(fd<0)
		Misc::throwStdErr("Comm::ListeningTCPSocket: Unable to create socket");
	
	/* Bind the socket file descriptor to the port ID: */
	struct sockaddr_in socketAddress;
	socketAddress.sin_family=AF_INET;
	socketAddress.sin_port=portId>=0?htons(portId):0;
	socketAddress.sin_addr.s_addr=htonl(INADDR_ANY);
	if(bind(fd,(struct sockaddr*)&socketAddress,sizeof(struct sockaddr_in))==-1)
		{
		close(fd);
		fd=-1;
		Misc::throwStdErr("Comm::ListeningTCPSocket: Unable to bind socket to port %d",portId);
		}
	
	/* Start listening on the socket: */
	if(listen(fd,backlog)==-1)
		{
		close(fd);
		fd=-1;
		Misc::throwStdErr("Comm::ListeningTCPSocket: Unable to start listening on socket");
		}
	}

ListeningTCPSocket::~ListeningTCPSocket(void)
	{
	if(fd>=0)
		close(fd);
	}

int ListeningTCPSocket::getPortId(void) const
	{
	struct sockaddr_in socketAddress;
	#ifdef __SGI_IRIX__
	int socketAddressLen=sizeof(struct sockaddr_in);
	#else
	socklen_t socketAddressLen=sizeof(struct sockaddr_in);
	#endif
	if(getsockname(fd,(struct sockaddr*)&socketAddress,&socketAddressLen)==-1)
		Misc::throwStdErr("ListeningTCPSocket::getPortId: Unable to query socket's port ID");
	return ntohs(socketAddress.sin_port);
	}

std::string ListeningTCPSocket::getAddress(void) const
	{
	struct sockaddr_in socketAddress;
	#ifdef __SGI_IRIX__
	int socketAddressLen=sizeof(struct sockaddr_in);
	#else
	socklen_t socketAddressLen=sizeof(struct sockaddr_in);
	#endif
	if(getsockname(fd,(struct sockaddr*)&socketAddress,&socketAddressLen)==-1)
		Misc::throwStdErr("ListeningTCPSocket::getAddress: Unable to query socket's interface address");
	char resultBuffer[INET_ADDRSTRLEN];
	if(inet_ntop(AF_INET,&socketAddress.sin_addr,resultBuffer,INET_ADDRSTRLEN)==0)
		Misc::throwStdErr("ListeningTCPSocket::getAddress: Unable to convert socket's interface address to dotted notation");
	return std::string(resultBuffer);
	}

std::string ListeningTCPSocket::getInterfaceName(bool throwException) const
	{
	struct sockaddr_in socketAddress;
	#ifdef __SGI_IRIX__
	int socketAddressLen=sizeof(struct sockaddr_in);
	#else
	socklen_t socketAddressLen=sizeof(struct sockaddr_in);
	#endif
	if(getsockname(fd,(struct sockaddr*)&socketAddress,&socketAddressLen)==-1)
		Misc::throwStdErr("ListeningTCPSocket::getInterfaceName: Unable to query socket's interface address");
	
	/* Lookup interface's name: */
	std::string result;
	struct hostent* hostEntry=gethostbyaddr((const char*)&socketAddress.sin_addr,sizeof(struct in_addr),AF_INET);
	if(hostEntry==0)
		{
		/* Fall back to returning address in dotted notation or throwing exception: */
		char addressBuffer[INET_ADDRSTRLEN];
		if(inet_ntop(AF_INET,&socketAddress.sin_addr,addressBuffer,INET_ADDRSTRLEN)==0)
			Misc::throwStdErr("ListeningTCPSocket::getInterfaceName: Unable to convert socket's interface address to dotted notation");
		if(throwException)
			Misc::throwStdErr("ListeningTCPSocket::getInterfaceName: Cannot resolve interface address %s",addressBuffer);
		else
			result=std::string(addressBuffer);
		}
	else
		result=std::string(hostEntry->h_name);
	
	return result;
	}

}
