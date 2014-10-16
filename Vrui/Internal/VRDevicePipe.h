/***********************************************************************
VRDevicePipe - Class defining the client-server protocol for remote VR
devices and VR applications.
Copyright (c) 2002-2014 Oliver Kreylos

This file is part of the Virtual Reality User Interface Library (Vrui).

The Virtual Reality User Interface Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Virtual Reality User Interface Library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality User Interface Library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef VRUI_INTERNAL_VRDEVICEPIPE_INCLUDED
#define VRUI_INTERNAL_VRDEVICEPIPE_INCLUDED

#include <Comm/TCPPipe.h>

namespace Vrui {

class VRDevicePipe:public Comm::TCPPipe
	{
	/* Embedded classes: */
	public:
	static const unsigned int protocolVersionNumber; // Version number of client/server protocol
	typedef unsigned short int MessageIdType; // Network type for protocol messages
	
	enum MessageId // Enumerated type for protocol messages
		{
		CONNECT_REQUEST, // Request to connect to server
		CONNECT_REPLY, // Positive connect reply with server layout
		DISCONNECT_REQUEST, // Polite request to disconnect from server
		ACTIVATE_REQUEST, // Request to activate server (prepare for sending packets)
		DEACTIVATE_REQUEST, // Request to deactivate server (no more packet requests)
		PACKET_REQUEST, // Requests a single packet with current device state
		PACKET_REPLY, // Sends a device state packet
		STARTSTREAM_REQUEST, // Requests entering stream mode (server sends packets automatically)
		STOPSTREAM_REQUEST, // Requests leaving stream mode
		STOPSTREAM_REPLY // Server's reply after last stream packet has been sent
		};
	
	/* Constructors and destructors: */
	VRDevicePipe(const char* hostName,int portId) // Creates a pipe connected to a remote host
		:Comm::TCPPipe(hostName,portId)
		{
		}
	VRDevicePipe(Comm::ListeningTCPSocket& listenSocket) // Creates a pipe for the next incoming TCP connection on the given listening socket
		:Comm::TCPPipe(listenSocket)
		{
		}
	
	/* New methods: */
	void writeMessage(MessageId messageId) // Writes a protocol message to the pipe
		{
		write<MessageIdType>(messageId);
		}
	MessageIdType readMessage(void) // Reads a protocol message from the pipe
		{
		return read<MessageIdType>();
		}
	};

}

#endif
