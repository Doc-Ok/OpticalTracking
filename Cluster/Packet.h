/***********************************************************************
Packet - Structure for packets sent and received by a cluster
multiplexer.
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

#ifndef CLUSTER_PACKET_INCLUDED
#define CLUSTER_PACKET_INCLUDED

#include <string.h>
#include <Cluster/Config.h>

namespace Cluster {

struct Packet
	{
	/* Embedded classes: */
	public:
	static const size_t maxRawPacketSize=CLUSTER_CONFIG_MTU_SIZE-CLUSTER_CONFIG_IP_HEADER_SIZE-CLUSTER_CONFIG_UDP_HEADER_SIZE; // Configured MTU size minus IP header size minus UDP header size
	static const size_t maxPacketSize=CLUSTER_CONFIG_MTU_SIZE-CLUSTER_CONFIG_IP_HEADER_SIZE-CLUSTER_CONFIG_UDP_HEADER_SIZE-2*sizeof(unsigned int); // Maximum size of multicast packet data payload in bytes
	
	class Reader // Simple class to read data from packets
		{
		/* Elements: */
		private:
		const char* end; // End-of-data pointer
		const char* rPtr; // Current read pointer
		
		/* Constructors and destructors: */
		public:
		Reader(const Packet* packet) // Creates reader for given packet
			:end(packet->packet+packet->packetSize),
			 rPtr(packet->packet)
			{
			}
		
		/* Methods: */
		bool eof(void) const // Returns true after all data in the packet has been read
			{
			return rPtr==end;
			}
		template <class DataParam>
		DataParam read(void) // Reads data item from packet
			{
			DataParam result;
			memcpy(&result,rPtr,sizeof(DataParam));
			rPtr+=sizeof(DataParam);
			return result;
			}
		template <class DataParam>
		void read(DataParam* data,size_t numItems) // Reads several data items from packet
			{
			memcpy(data,rPtr,numItems*sizeof(DataParam));
			rPtr+=numItems*sizeof(DataParam);
			}
		};
	
	class Writer // Simple class to write data into packets
		{
		/* Elements: */
		private:
		Packet* packet; // Pointer to packet
		char* wPtr; // Current write pointer
		
		/* Constructors and destructors: */
		public:
		Writer(Packet* sPacket) // Creates writer for given packet
			:packet(sPacket),wPtr(packet->packet)
			{
			/* Initialize the packet: */
			packet->packetSize=0;
			}
		~Writer(void)
			{
			/* Finalize the packet: */
			packet->packetSize=wPtr-packet->packet;
			}
		
		/* Methods: */
		template <class DataParam>
		void write(const DataParam& value) // Writes data item into packet
			{
			memcpy(wPtr,&value,sizeof(DataParam));
			wPtr+=sizeof(DataParam);
			}
		template <class DataParam>
		void write(const DataParam* data,size_t numItems) // Writes several data items into packet
			{
			memcpy(wPtr,data,numItems*sizeof(DataParam));
			wPtr+=numItems*sizeof(DataParam);
			}
		};
	
	/* Elements: */
	Packet* succ; // Pointer to successor in packet queues
	size_t packetSize; // Actual size of packet
	unsigned int pipeId; // ID of the pipe this packet is intended for
	unsigned int streamPos; // Position of packet data in entire stream that has been sent on pipe so far
	char packet[maxPacketSize]; // Packet data
	
	/* Constructors and destructors: */
	Packet(void) // Creates empty packet
		:succ(0),packetSize(0)
		{
		}
	};

}

#endif
