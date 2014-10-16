/***********************************************************************
VRPNConnection - Class to maintain connections to remote VRPN virtual
reality device servers.
Copyright (c) 2008-2010 Oliver Kreylos

This file is part of the Vrui VR Device Driver Daemon (VRDeviceDaemon).

The Vrui VR Device Driver Daemon is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Vrui VR Device Driver Daemon is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Vrui VR Device Driver Daemon; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef VRPNCONNECTION_INCLUDED
#define VRPNCONNECTION_INCLUDED

#include <utility>
#include <vector>
#include <Misc/HashTable.h>
#include <Comm/UDPSocket.h>
#include <Comm/TCPSocket.h>
#include <Vrui/Internal/VRDeviceState.h>

/* Forward declarations: */
namespace Misc {
class Time;
}

class VRPNConnection
	{
	/* Embedded classes: */
	public:
	typedef Vrui::VRDeviceState::TrackerState TrackerState;
	typedef TrackerState::PositionOrientation PositionOrientation;
	typedef TrackerState::LinearVelocity LinearVelocity;
	typedef TrackerState::AngularVelocity AngularVelocity;
	typedef Vrui::VRDeviceState::ButtonState ButtonState;
	typedef Vrui::VRDeviceState::ValuatorState ValuatorState;
	
	private:
	enum ServiceType // Enumerated type for VRPN service types
		{
		RELIABLE=0x1,
		FIXED_LATENCY=0x2,
		LOW_LATENCY=0x4,
		FIXED_THROUGHPUT=0x8,
		HIGH_THROUGHPUT=0x10
		};
	
	enum SystemMessageType // Enumerated type for VRPN system message types
		{
		SENDER_DESCRIPTION=-1,
		TYPE_DESCRIPTION=-2,
		UDP_DESCRIPTION=-3,
		LOG_DESCRIPTION=-4,
		DISCONNECT_MESSAGE=-5
		};
	
	struct SenderDescriptor // Structure storing the data associations of a VRPN sender
		{
		/* Elements: */
		public:
		int trackerIndexBase; // Base index of all trackers on this sender
		int numTrackers; // Number of trackers on this sender
		int buttonIndexBase; // Base index of all buttons on this sender
		int numButtons; // Number of buttons on this sender
		int valuatorIndexBase; // Base index of all valuators on this sender
		int numValuators; // Number of valuators on this sender
		
		/* Constructors and destructors: */
		SenderDescriptor(void)
			:trackerIndexBase(-1),numTrackers(0),
			 buttonIndexBase(-1),numButtons(0),
			 valuatorIndexBase(-1),numValuators(0)
			{
			}
		};
	
	typedef std::pair<std::string,SenderDescriptor> SenderRequest; // Type to store senders requested by VRPN clients
	typedef Misc::HashTable<unsigned int,SenderDescriptor> SenderHasher; // Map from sender tags to sender descriptors
	
	/* Elements: */
	static const int alignment=8; // Alignment size for all atomic data sent across a VRPN connection
	static const size_t alignmentMask=~size_t(alignment-1); // Bit mask to efficiently pad data sizes
	static const char* versionMessage; // Text string identifying a VRPN client and server
	static const int versionMessageLength; // Length of the version message
	Comm::TCPSocket tcpSocket; // TCP socket connected to the VRPN server for reliable messages
	Comm::UDPSocket udpSocket; // UDP socket connected to the VRPN server for low-latency unreliable messages
	bool udpSocketConnected; // Flag if the UDP socket is connected to the VRPN server
	size_t messageBufferSize; // Current size of the temporary buffer to assemble messages
	char* messageBuffer; // The current message buffer
	std::vector<SenderRequest> requestedSenders; // List of senders requested by VRPN clients
	SenderHasher senders; // Map from VRPN server's sender tags to device descriptions
	
	/* Relevant message types: */
	int trackerPosQuatMessage; // Position/orientation update for a tracker
	int trackerVelocityMessage; // Velocity update for a tracker
	int buttonChangeMessage; // Button status update for a set of buttons
	int analogChannelMessage; // Analog value update for a set of valuators
	
	bool flipZAxis; // Flag whether to negate the z components of incoming position data
	
	/* Private methods: */
	static size_t pad(size_t size) // Pads a message size to the data alignment
		{
		return (size+alignment-1)&alignmentMask;
		}
	void sendMessage(size_t messageSize,const Misc::Time& time,int messageType,unsigned int sender,const char* message,int serviceType); // Sends a message to the VRPN server
	void handleMessage(const Misc::Time& messageTime,int messageType,unsigned int sender,size_t messageSize,char* message); // Processes a single message from the VRPN server
	
	/* Protected methods: */
	protected:
	virtual void updateTrackerPosition(int trackerIndex,const PositionOrientation& positionOrientation) =0; // Sets a tracker's position and orientation
	virtual void updateTrackerVelocity(int trackerIndex,const LinearVelocity& linearVelocity,const AngularVelocity& angularVelocity) =0; // Sets a tracker's velocities
	virtual void updateButtonState(int buttonIndex,ButtonState newState) =0; // Sets a button's state
	virtual void updateValuatorState(int valuatorIndex,ValuatorState newState) =0; // Sets a valuator's state
	virtual void finalizePacket(void); // Called when an entire packet of VRPN messages has been processed
	
	/* Constructors and destructors: */
	public:
	VRPNConnection(const char* serverName,int serverPort =3883); // Opens a connection to the given server host and port
	virtual ~VRPNConnection(void);
	
	/* Methods: */
	void requestTrackers(const char* senderName,int trackerIndexBase,int numTrackers); // Requests tracker data
	void setFlipZAxis(bool newFlipZAxis); // Sets the z axis flipping flag, to convert left-handed into right-handed coordinate systems
	void requestButtons(const char* senderName,int buttonIndexBase,int numButtons); // Requests button data
	void requestValuators(const char* senderName,int valuatorIndexBase,int numValuators); // Requests valuator data
	void readNextMessages(void); // Reads the next batch of messages from either the TCP or the UDP socket
	};

#endif
