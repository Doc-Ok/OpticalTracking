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

#include <Cluster/TCPPipe.h>

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/StringMarshaller.h>
#include <Misc/FdSet.h>

namespace Cluster {

namespace {

/****************
Helper functions:
****************/

void handleConstructionError(int errorType,const char* hostName,int portId)
	{
	switch(errorType)
		{
		case 1:
			Misc::throwStdErr("Cluster::TCPPipe::TCPPipe: Unable to create socket");
		
		case 2:
			Misc::throwStdErr("Cluster::TCPPipe::TCPPipe: Unable to bind socket to port");
		
		case 3:
			Misc::throwStdErr("Cluster::TCPPipe::TCPPipe: Unable to resolve host name %s",hostName);
		
		case 4:
			Misc::throwStdErr("Cluster::TCPPipe::TCPPipe: Unable to connect to host %s on port %d",hostName,portId);
		
		case 5:
			Misc::throwStdErr("Cluster::TCPPipe::TCPPipe: Unable to disable Nagle's algorithm on socket");
		}
	}

void handleReadError(int errorCode)
	{
	Misc::throwStdErr("Comm::TCPPipe: Fatal error %d while reading from source",errorCode);
	}

void handleWriteError(int errorType,int errorCode)
	{
	switch(errorType)
		{
		case 1:
			Misc::throwStdErr("Cluster::TCPPipe: Connection terminated by peer");
		
		case 2:
			throw IO::File::WriteError(errorCode);
		
		case 3:
			Misc::throwStdErr("Cluster::TCPPipe: Fatal error %d while writing to sink",errorCode);
		}
	}

}

/******************************
Methods of class TCPPipeMaster:
******************************/

size_t TCPPipeMaster::readData(IO::File::Byte* buffer,size_t bufferSize)
	{
	/* Read more data from source: */
	ssize_t readResult;
	do
		{
		readResult=::read(fd,buffer,bufferSize);
		}
	while(readResult<0&&(errno==EAGAIN||errno==EWOULDBLOCK||errno==EINTR));
	
	/* Handle the result from the read call: */
	if(readResult>=0)
		{
		size_t readSize=size_t(readResult);
		
		if(isReadCoupled())
			{
			/* Forward the just-read data to the slaves: */
			Packet* packet=multiplexer->newPacket();
			packet->packetSize=readSize;
			memcpy(packet->packet,buffer,readSize);
			multiplexer->sendPacket(pipeId,packet);
			}
		
		return readSize;
		}
	else
		{
		int errorCode=errno;
		if(isReadCoupled())
			{
			/* Send an error indicator (empty packet followed by status packet) to the slaves: */
			Packet* packet=multiplexer->newPacket();
			packet->packetSize=0;
			multiplexer->sendPacket(pipeId,packet);
			packet=multiplexer->newPacket();
			{
			Packet::Writer writer(packet);
			writer.write<int>(errorCode);
			}
			multiplexer->sendPacket(pipeId,packet);
			}
		
		/* Throw an exception: */
		handleReadError(errorCode);
		}
	
	/* Never reached; just to make compiler happy: */
	return 0;
	}

void TCPPipeMaster::writeData(const IO::File::Byte* buffer,size_t bufferSize)
	{
	/* Collect error codes: */
	int errorType=0;
	int errorCode=0;
	
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
			errorType=1;
			break;
			}
		else if(writeResult<0&&(errno==EAGAIN||errno==EWOULDBLOCK||errno==EINTR))
			{
			/* Do nothing and try again */
			}
		else if(writeResult==0)
			{
			/* Sink has reached end-of-file: */
			errorType=2;
			errorCode=int(bufferSize);
			break;
			}
		else
			{
			/* Unknown error; probably a bad thing: */
			errorType=3;
			errorCode=errno;
			break;
			}
		}
	
	if(isWriteCoupled())
		{
		/* Send a status packet to the slaves: */
		Packet* packet=multiplexer->newPacket();
		{
		Packet::Writer writer(packet);
		writer.write<int>(errorType);
		writer.write<int>(errorCode);
		}
		multiplexer->sendPacket(pipeId,packet);
		}
	
	if(errorType!=0)
		{
		/* Throw an exception: */
		handleWriteError(errorType,errorCode);
		}
	}

TCPPipeMaster::TCPPipeMaster(Multiplexer* sMultiplexer,const char* hostName,int portId)
	:Comm::NetPipe(WriteOnly),ClusterPipe(sMultiplexer),
	 fd(-1)
	{
	/* Collect error indicators: */
	int errorType=0;
	
	/* Create the socket file descriptor: */
	fd=socket(PF_INET,SOCK_STREAM,0);
	if(fd<0)
		errorType=1;
	
	if(errorType==0)
		{
		/* Bind the socket file descriptor: */
		struct sockaddr_in mySocketAddress;
		mySocketAddress.sin_family=AF_INET;
		mySocketAddress.sin_port=0;
		mySocketAddress.sin_addr.s_addr=htonl(INADDR_ANY);
		if(bind(fd,(struct sockaddr*)&mySocketAddress,sizeof(struct sockaddr_in))==-1)
			errorType=2;
		}
	
	struct in_addr hostNetAddress;
	if(errorType==0)
		{
		/* Lookup host's IP address: */
		struct hostent* hostEntry=gethostbyname(hostName);
		if(hostEntry!=0)
			hostNetAddress.s_addr=ntohl(((struct in_addr*)hostEntry->h_addr_list[0])->s_addr);
		else
			errorType=3;
		}
	
	if(errorType==0)
		{
		/* Connect to the remote host: */
		struct sockaddr_in hostAddress;
		hostAddress.sin_family=AF_INET;
		hostAddress.sin_port=htons(portId);
		hostAddress.sin_addr.s_addr=htonl(hostNetAddress.s_addr);
		if(connect(fd,(const struct sockaddr*)&hostAddress,sizeof(struct sockaddr_in))==-1)
			errorType=4;
		}
	
	if(errorType==0)
		{
		/* Set the TCP_NODELAY socket option: */
		int flag=1;
		if(setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,&flag,sizeof(flag))==-1)
			errorType=5;
		}
	
	/* Send a status message to the slaves: */
	Packet* statusPacket=multiplexer->newPacket();
	{
	Packet::Writer writer(statusPacket);
	writer.write<int>(errorType);
	}
	multiplexer->sendPacket(pipeId,statusPacket);
	
	if(errorType>1)
		{
		/* Close the socket again: */
		close(fd);
		fd=-1;
		}
	
	if(errorType!=0)
		{
		/* Throw an exception: */
		handleConstructionError(errorType,hostName,portId);
		}
	
	/* Install a read buffer the size of a multicast packet: */
	Comm::Pipe::resizeReadBuffer(Packet::maxPacketSize);
	canReadThrough=false;
	}

TCPPipeMaster::~TCPPipeMaster(void)
	{
	/* Flush the write buffer, and then close the socket: */
	flush();
	if(fd>=0)
		close(fd);
	}

int TCPPipeMaster::getFd(void) const
	{
	Misc::throwStdErr("Cluster::TCPPipe::getFd: Cannot query file descriptor");
	
	/* Just to make compiler happy: */
	return -1;
	}

size_t TCPPipeMaster::resizeReadBuffer(size_t newReadBufferSize)
	{
	/* Ignore the change and return the size of a multicast packet: */
	return Packet::maxPacketSize;
	}

bool TCPPipeMaster::waitForData(void) const
	{
	/* Check if there is unread data in the buffer: */
	if(getUnreadDataSize()>0)
		return true;
	
	/* Wait for data on the socket and return whether data is available: */
	Misc::FdSet readFds(fd);
	bool result=Misc::pselect(&readFds,0,0,0)>=0&&readFds.isSet(fd);
	
	if(isReadCoupled())
		{
		/* Send a status message to the slaves: */
		Packet* statusPacket=multiplexer->newPacket();
		{
		Packet::Writer writer(statusPacket);
		writer.write<int>(result?1:0);
		}
		multiplexer->sendPacket(pipeId,statusPacket);
		}
	
	return result;
	}

bool TCPPipeMaster::waitForData(const Misc::Time& timeout) const
	{
	/* Check if there is unread data in the buffer: */
	if(getUnreadDataSize()>0)
		return true;
	
	/* Wait for data on the socket and return whether data is available: */
	Misc::FdSet readFds(fd);
	bool result=Misc::pselect(&readFds,0,0,timeout)>=0&&readFds.isSet(fd);
	
	if(isReadCoupled())
		{
		/* Send a status message to the slaves: */
		Packet* statusPacket=multiplexer->newPacket();
		{
		Packet::Writer writer(statusPacket);
		writer.write<int>(result?1:0);
		}
		multiplexer->sendPacket(pipeId,statusPacket);
		}
	
	return result;
	}

void TCPPipeMaster::shutdown(bool read,bool write)
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

int TCPPipeMaster::getPortId(void) const
	{
	struct sockaddr_in socketAddress;
	#ifdef __SGI_IRIX__
	int socketAddressLen=sizeof(struct sockaddr_in);
	#else
	socklen_t socketAddressLen=sizeof(struct sockaddr_in);
	#endif
	getsockname(fd,(struct sockaddr*)&socketAddress,&socketAddressLen);
	int result=ntohs(socketAddress.sin_port);
	
	if(isReadCoupled())
		{
		/* Send a status message to the slaves: */
		Packet* statusPacket=multiplexer->newPacket();
		{
		Packet::Writer writer(statusPacket);
		writer.write<int>(result);
		}
		multiplexer->sendPacket(pipeId,statusPacket);
		}
	
	return result;
	}

std::string TCPPipeMaster::getAddress(void) const
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
	std::string result(resultBuffer);
	
	if(isReadCoupled())
		{
		/* Send a status message to the slaves: */
		Packet* statusPacket=multiplexer->newPacket();
		{
		Packet::Writer writer(statusPacket);
		Misc::writeCppString(result,writer);
		}
		multiplexer->sendPacket(pipeId,statusPacket);
		}
	
	return result;
	}

std::string TCPPipeMaster::getHostName(void) const
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
	
	if(isReadCoupled())
		{
		/* Send a status message to the slaves: */
		Packet* statusPacket=multiplexer->newPacket();
		{
		Packet::Writer writer(statusPacket);
		Misc::writeCppString(result,writer);
		}
		multiplexer->sendPacket(pipeId,statusPacket);
		}
	
	return result;
	}

int TCPPipeMaster::getPeerPortId(void) const
	{
	struct sockaddr_in peerAddress;
	#ifdef __SGI_IRIX__
	int peerAddressLen=sizeof(struct sockaddr_in);
	#else
	socklen_t peerAddressLen=sizeof(struct sockaddr_in);
	#endif
	getpeername(fd,(struct sockaddr*)&peerAddress,&peerAddressLen);
	int result=ntohs(peerAddress.sin_port);
	
	if(isReadCoupled())
		{
		/* Send a status message to the slaves: */
		Packet* statusPacket=multiplexer->newPacket();
		{
		Packet::Writer writer(statusPacket);
		writer.write<int>(result);
		}
		multiplexer->sendPacket(pipeId,statusPacket);
		}
	
	return result;
	}

std::string TCPPipeMaster::getPeerAddress(void) const
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
	std::string result(resultBuffer);
	
	if(isReadCoupled())
		{
		/* Send a status message to the slaves: */
		Packet* statusPacket=multiplexer->newPacket();
		{
		Packet::Writer writer(statusPacket);
		Misc::writeCppString(result,writer);
		}
		multiplexer->sendPacket(pipeId,statusPacket);
		}
	
	return result;
	}

std::string TCPPipeMaster::getPeerHostName(void) const
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
	
	if(isReadCoupled())
		{
		/* Send a status message to the slaves: */
		Packet* statusPacket=multiplexer->newPacket();
		{
		Packet::Writer writer(statusPacket);
		Misc::writeCppString(result,writer);
		}
		multiplexer->sendPacket(pipeId,statusPacket);
		}
	
	return result;
	}

/*****************************
Methods of class TCPPipeSlave:
*****************************/

size_t TCPPipeSlave::readData(IO::File::Byte* buffer,size_t bufferSize)
	{
	if(isReadCoupled())
		{
		/* Receive a data packet from the master: */
		Packet* newPacket=multiplexer->receivePacket(pipeId);
		
		/* Check for error conditions: */
		if(newPacket->packetSize!=0)
			{
			/* Install the new packet as the pipe's read buffer: */
			if(packet!=0)
				multiplexer->deletePacket(packet);
			packet=newPacket;
			setReadBuffer(Packet::maxPacketSize,reinterpret_cast<Byte*>(packet->packet),false);
			
			return packet->packetSize;
			}
		else
			{
			/* Read the status packet: */
			multiplexer->deletePacket(newPacket);
			newPacket=multiplexer->receivePacket(pipeId);
			Packet::Reader reader(newPacket);
			int errorCode=reader.read<int>();
			multiplexer->deletePacket(newPacket);
			
			/* Throw an exception: */
			handleReadError(errorCode);
			}
		
		/* Never reached; just to make compiler happy: */
		return 0;
		}
	else
		{
		/* Return end-of-file; slave shouldn't have been reading in decoupled state: */
		return 0;
		}
	}

void TCPPipeSlave::writeData(const IO::File::Byte* buffer,size_t bufferSize)
	{
	if(isWriteCoupled())
		{
		/* Receive a status packet from the master: */
		Packet* statusPacket=multiplexer->receivePacket(pipeId);
		Packet::Reader reader(statusPacket);
		int errorType=reader.read<int>();
		int errorCode=reader.read<int>();
		multiplexer->deletePacket(statusPacket);
		
		if(errorType!=0)
			{
			/* Throw an exception: */
			handleWriteError(errorType,errorCode);
			}
		}
	}

TCPPipeSlave::TCPPipeSlave(Multiplexer* sMultiplexer,const char* hostName,int portId)
	:Comm::NetPipe(WriteOnly),ClusterPipe(sMultiplexer),
	 packet(0)
	{
	/* Read the status packet from the master node: */
	Packet* statusPacket=multiplexer->receivePacket(pipeId);
	Packet::Reader reader(statusPacket);
	int errorType=reader.read<int>();
	multiplexer->deletePacket(statusPacket);
	
	if(errorType!=0)
		{
		/* Throw an exception: */
		handleConstructionError(errorType,hostName,portId);
		}
	
	/* Disable read-through: */
	canReadThrough=false;
	}

TCPPipeSlave::~TCPPipeSlave(void)
	{
	/* Delete the current multicast packet: */
	if(packet!=0)
		{
		multiplexer->deletePacket(packet);
		setReadBuffer(0,0,false);
		}
	}

int TCPPipeSlave::getFd(void) const
	{
	Misc::throwStdErr("Cluster::TCPPipe::getFd: Cannot query file descriptor");
	
	/* Just to make compiler happy: */
	return -1;
	}

size_t TCPPipeSlave::getReadBufferSize(void) const
	{
	/* Return the size of a multicast packet: */
	return Packet::maxPacketSize;
	}

size_t TCPPipeSlave::resizeReadBuffer(size_t newReadBufferSize)
	{
	/* Ignore the change and return the size of a multicast packet: */
	return Packet::maxPacketSize;
	}

bool TCPPipeSlave::waitForData(void) const
	{
	if(isReadCoupled())
		{
		/* Check if there is unread data in the buffer: */
		if(getUnreadDataSize()>0)
			return true;
		
		/* Read the status packet from the master node: */
		Packet* statusPacket=multiplexer->receivePacket(pipeId);
		Packet::Reader reader(statusPacket);
		int result=reader.read<int>();
		multiplexer->deletePacket(statusPacket);
		
		return result!=0;
		}
	else
		{
		/* Return no new data; slave shouldn't be reading in decoupled state: */
		return false;
		}
	}

bool TCPPipeSlave::waitForData(const Misc::Time& timeout) const
	{
	if(isReadCoupled())
		{
		/* Check if there is unread data in the buffer: */
		if(getUnreadDataSize()>0)
			return true;
		
		/* Read the status packet from the master node: */
		Packet* statusPacket=multiplexer->receivePacket(pipeId);
		Packet::Reader reader(statusPacket);
		int result=reader.read<int>();
		multiplexer->deletePacket(statusPacket);
		
		return result!=0;
		}
	else
		{
		/* Return no new data; slave shouldn't be reading in decoupled state: */
		return false;
		}
	}

void TCPPipeSlave::shutdown(bool read,bool write)
	{
	/* Do nothing */
	}

int TCPPipeSlave::getPortId(void) const
	{
	if(isReadCoupled())
		{
		/* Read the status packet from the master node: */
		Packet* statusPacket=multiplexer->receivePacket(pipeId);
		Packet::Reader reader(statusPacket);
		int result=reader.read<int>();
		multiplexer->deletePacket(statusPacket);
		
		return result;
		}
	else
		{
		/* Return bogus port ID; slave shouldn't be reading in decoupled state: */
		return -1;
		}
	}

std::string TCPPipeSlave::getAddress(void) const
	{
	if(isReadCoupled())
		{
		/* Read the status packet from the master node: */
		Packet* statusPacket=multiplexer->receivePacket(pipeId);
		Packet::Reader reader(statusPacket);
		std::string result=Misc::readCppString(reader);
		multiplexer->deletePacket(statusPacket);
		
		return result;
		}
	else
		{
		/* Return bogus address; slave shouldn't be reading in decoupled state: */
		return std::string();
		}
	}

std::string TCPPipeSlave::getHostName(void) const
	{
	if(isReadCoupled())
		{
		/* Read the status packet from the master node: */
		Packet* statusPacket=multiplexer->receivePacket(pipeId);
		Packet::Reader reader(statusPacket);
		std::string result=Misc::readCppString(reader);
		multiplexer->deletePacket(statusPacket);
		
		return result;
		}
	else
		{
		/* Return bogus host name; slave shouldn't be reading in decoupled state: */
		return std::string();
		}
	}

int TCPPipeSlave::getPeerPortId(void) const
	{
	if(isReadCoupled())
		{
		/* Read the status packet from the master node: */
		Packet* statusPacket=multiplexer->receivePacket(pipeId);
		Packet::Reader reader(statusPacket);
		int result=reader.read<int>();
		multiplexer->deletePacket(statusPacket);
		
		return result;
		}
	else
		{
		/* Return bogus port ID; slave shouldn't be reading in decoupled state: */
		return -1;
		}
	}

std::string TCPPipeSlave::getPeerAddress(void) const
	{
	if(isReadCoupled())
		{
		/* Read the status packet from the master node: */
		Packet* statusPacket=multiplexer->receivePacket(pipeId);
		Packet::Reader reader(statusPacket);
		std::string result=Misc::readCppString(reader);
		multiplexer->deletePacket(statusPacket);
		
		return result;
		}
	else
		{
		/* Return bogus address; slave shouldn't be reading in decoupled state: */
		return std::string();
		}
	}

std::string TCPPipeSlave::getPeerHostName(void) const
	{
	if(isReadCoupled())
		{
		/* Read the status packet from the master node: */
		Packet* statusPacket=multiplexer->receivePacket(pipeId);
		Packet::Reader reader(statusPacket);
		std::string result=Misc::readCppString(reader);
		multiplexer->deletePacket(statusPacket);
		
		return result;
		}
	else
		{
		/* Return bogus host name; slave shouldn't be reading in decoupled state: */
		return std::string();
		}
	}

}
