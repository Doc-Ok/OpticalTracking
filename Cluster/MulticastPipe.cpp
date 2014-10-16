/***********************************************************************
MulticastPipe - Class to represent data streams between a single master
and several slaves, with the bulk of communication from the master to
all the slaves in parallel.
Copyright (c) 2005-2012 Oliver Kreylos

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

#include <Cluster/MulticastPipe.h>

#include <Misc/ThrowStdErr.h>
#include <Cluster/Packet.h>
#include <Cluster/Multiplexer.h>

namespace Cluster {

/******************************
Methods of class MulticastPipe:
******************************/

size_t MulticastPipe::readData(IO::File::Byte* buffer,size_t bufferSize)
	{
	/* Delete the current (completely read) packet: */
	if(packet!=0)
		{
		Packet* oldPacket=packet;
		packet=0;
		multiplexer->deletePacket(oldPacket);
		}
	
	/* Get the next packet from the multiplexer: */
	packet=multiplexer->receivePacket(pipeId);
	
	/* Install the new packet as the buffered file's read buffer: */
	setReadBuffer(Packet::maxPacketSize,reinterpret_cast<Byte*>(packet->packet),false);
	
	return packet->packetSize;
	}

void MulticastPipe::writeData(const IO::File::Byte* buffer,size_t bufferSize)
	{
	/* Pass the current packet to the multiplexer: */
	{
	Packet* sendPacket=packet;
	packet=0;
	sendPacket->packetSize=bufferSize;
	multiplexer->sendPacket(pipeId,sendPacket);
	}
	
	/* Install a fresh cluster packet as the write buffer: */
	packet=multiplexer->newPacket();
	setWriteBuffer(Packet::maxPacketSize,reinterpret_cast<Byte*>(packet->packet),false);
	}

void MulticastPipe::flushPipe(void)
	{
	/* Call IO::File's flush method: */
	flush();
	}

MulticastPipe::MulticastPipe(Multiplexer* sMultiplexer)
	:IO::File(),ClusterPipe(sMultiplexer),
	 packet(0)
	{
	/* Set up the master or slave buffers: */
	if(isMaster())
		{
		/* Install a fresh cluster packet as the write buffer: */
		packet=multiplexer->newPacket();
		setWriteBuffer(Packet::maxPacketSize,reinterpret_cast<Byte*>(packet->packet),false);
		
		/* Disable direct writes: */
		canWriteThrough=false;
		}
	else
		{
		/* Disable direct reads: */
		canReadThrough=false;
		}
	}

MulticastPipe::~MulticastPipe(void)
	{
	if(isMaster())
		{
		/* Check if there is unsent data in the write buffer: */
		size_t unwrittenSize=getWritePtr();
		if(unwrittenSize!=0)
			{
			/* Pass the final packet to the multiplexer: */
			{
			Packet* sendPacket=packet;
			packet=0;
			sendPacket->packetSize=unwrittenSize;
			multiplexer->sendPacket(pipeId,sendPacket);
			}
			}
		
		/* Uninstall the buffered file's write buffer: */
		setWriteBuffer(0,0,false);
		}
	else
		{
		/* Uninstall the buffered file's read buffer: */
		setReadBuffer(0,0,false);
		}
	
	/* Delete the current cluster packet: */
	if(packet!=0)
		multiplexer->deletePacket(packet);
	}

size_t MulticastPipe::getReadBufferSize(void) const
	{
	/* Return the maximum cluster packet size: */
	return Packet::maxPacketSize;
	}

size_t MulticastPipe::getWriteBufferSize(void) const
	{
	/* Return the maximum cluster packet size: */
	return Packet::maxPacketSize;
	}

size_t MulticastPipe::resizeReadBuffer(size_t newReadBufferSize)
	{
	/* Ignore the request and return the maximum cluster packet size: */
	return Packet::maxPacketSize;
	}

void MulticastPipe::resizeWriteBuffer(size_t newWriteBufferSize)
	{
	/* Ignore the request */
	}

}
