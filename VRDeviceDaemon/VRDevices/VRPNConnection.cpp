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

#define VRPNCONNECTION_USE_UDP 0

#include <VRDeviceDaemon/VRDevices/VRPNConnection.h>

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <string>
#include <Misc/Endianness.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/Time.h>
#include <Misc/FdSet.h>

namespace {

/****************
Helper functions:
****************/

template <class DataParam>
inline
void
buffer(
	char*& bufferPtr,
	const DataParam& data)
	{
	DataParam* ptr=reinterpret_cast<DataParam*>(bufferPtr);
	bufferPtr+=sizeof(DataParam);
	*ptr=data;
	
	#if __BYTE_ORDER!=__BIG_ENDIAN
	/* Byte-swap the just-buffered data: */
	Misc::swapEndianness(*ptr);
	#endif
	}

template <class DataParam>
inline
DataParam
unbuffer(
	char*& bufferPtr)
	{
	DataParam* ptr=reinterpret_cast<DataParam*>(bufferPtr);
	bufferPtr+=sizeof(DataParam);
	
	#if __BYTE_ORDER!=__BIG_ENDIAN
	/* Byte-swap the data in the buffer: */
	Misc::swapEndianness(*ptr);
	#endif
	return *ptr;
	}

}

/***************************************
Static elements of class VRPNConnection:
***************************************/

const char* VRPNConnection::versionMessage="vrpn: ver. 07.15";
const int VRPNConnection::versionMessageLength=16;

/*******************************
Methods of class VRPNConnection:
*******************************/

void VRPNConnection::sendMessage(size_t messageSize,const Misc::Time& time,int messageType,unsigned int sender,const char* message,int serviceType)
	{
	/* Compute the total message length, including padding: */
	size_t headerLen=pad(4*sizeof(u_int32_t)+sizeof(int32_t));
	size_t messageLen=pad(messageSize);
	
	/* Adjust the message buffer's size: */
	if(messageBufferSize<headerLen+messageLen)
		{
		delete[] messageBuffer;
		messageBufferSize=headerLen+messageLen;
		messageBuffer=new char[messageBufferSize];
		}
	
	/* Assemble the message header in the send buffer: */
	char* messageBufferPtr=messageBuffer;
	buffer(messageBufferPtr,u_int32_t(headerLen+messageSize)); // Use unpadded message size here!
	buffer(messageBufferPtr,u_int32_t(time.tv_sec));
	buffer(messageBufferPtr,u_int32_t((time.tv_nsec+500)/1000));
	buffer(messageBufferPtr,u_int32_t(sender));
	buffer(messageBufferPtr,int32_t(messageType));
	
	/* Copy the message into the send buffer: */
	messageBufferPtr=messageBuffer+headerLen;
	memcpy(messageBufferPtr,message,messageSize);
	
	/* Send the message: */
	if(!udpSocketConnected||(serviceType&RELIABLE))
		{
		/* Send data via TCP socket: */
		tcpSocket.blockingWrite(messageBuffer,headerLen+messageLen);
		}
	else
		{
		/* Send data via UDP socket: */
		udpSocket.sendMessage(messageBuffer,headerLen+messageLen);
		}
	}

void VRPNConnection::handleMessage(const Misc::Time& messageTime,int messageType,unsigned int sender,size_t messageSize,char* message)
	{
	switch(messageType)
		{
		case UDP_DESCRIPTION:
			{
			/* Read the address and port of the VRPN server's UDP socket: */
			std::string serverAddress=std::string(message,messageSize);
			
			/* Connect the UDP socket to the server: */
			// udpSocket.connect(serverAddress,sender+1); // VRPN server sends totally bogus UDP port addresses...
			udpSocketConnected=true;
			
			break;
			}
		
		case SENDER_DESCRIPTION:
			{
			/* Extract the sender name from the message: */
			char* namePtr=message;
			namePtr+=sizeof(u_int32_t); // Skip redundant name length
			
			/* Match the new sender with any pending sender requests: */
			unsigned int index;
			for(index=0;index<requestedSenders.size()&&requestedSenders[index].first!=namePtr;++index)
				;
			if(index<requestedSenders.size())
				{
				/* Store the sender's device descriptor in the sender map: */
				senders.setEntry(SenderHasher::Entry(sender,requestedSenders[index].second));
				}
			
			break;
			}
		
		case TYPE_DESCRIPTION:
			{
			/* Extract the message type name from the message: */
			char* namePtr=message;
			namePtr+=sizeof(u_int32_t); // Skip redundant name length
			
			/* Check if the message type is of interest: */
			if(strcmp(namePtr,"vrpn_Tracker Pos_Quat")==0)
				trackerPosQuatMessage=int(sender);
			else if(strcmp(namePtr,"vrpn_Tracker Velocity")==0)
				trackerVelocityMessage=int(sender);
			else if(strcmp(namePtr,"vrpn_Button Change")==0)
				buttonChangeMessage=int(sender);
			else if(strcmp(namePtr,"vrpn_Analog Channel")==0)
				analogChannelMessage=int(sender);
			
			break;
			}
		
		case LOG_DESCRIPTION:
		case DISCONNECT_MESSAGE:
			/* Just ignore these silently... */
			break;
		
		default:
			{
			/* Handle device messages: */
			if(messageType==trackerPosQuatMessage)
				{
				/* Get the sender's device descriptor: */
				SenderHasher::Iterator shIt=senders.findEntry(sender);
				if(!shIt.isFinished())
					{
					/* Get the device descriptor: */
					const SenderDescriptor& sd=shIt->getDest();
					
					/* Extract the sensor number from the message: */
					char* messagePtr=message;
					int sensorNumber=int(unbuffer<u_int32_t>(messagePtr));
					
					/* Check if the client is interested in this sensor: */
					if(sensorNumber<sd.numTrackers)
						{
						/* Extract the tracker's position and orientation: */
						messagePtr=message+pad(sizeof(u_int32_t));
						double pos[3];
						for(int i=0;i<3;++i)
							pos[i]=unbuffer<double>(messagePtr);
						double quat[4];
						for(int i=0;i<4;++i)
							quat[i]=unbuffer<double>(messagePtr);
						
						/* Flip the z axis if necessary: */
						if(flipZAxis)
							{
							/* Flip the position: */
							pos[2]=-pos[2];
							
							/* Flip the orientation: */
							quat[0]=-quat[0];
							quat[1]=-quat[1];
							}
						
						/* Update the tracker data structure: */
						PositionOrientation po(PositionOrientation::Vector(pos),PositionOrientation::Rotation::fromQuaternion(quat));
						updateTrackerPosition(sd.trackerIndexBase+sensorNumber,po);
						}
					}
				}
			else if(messageType==trackerVelocityMessage)
				{
				/* Get the sender's device descriptor: */
				SenderHasher::Iterator shIt=senders.findEntry(sender);
				if(!shIt.isFinished())
					{
					/* Get the device descriptor: */
					const SenderDescriptor& sd=shIt->getDest();
					
					/* Extract the sensor number from the message: */
					char* messagePtr=message;
					int sensorNumber=int(unbuffer<u_int32_t>(messagePtr));
					
					/* Check if the client is interested in this sensor: */
					if(sensorNumber<sd.numTrackers)
						{
						/* Extract the tracker's linear and angular velocities: */
						messagePtr=message+pad(sizeof(u_int32_t));
						double linearVel[3];
						for(int i=0;i<3;++i)
							linearVel[i]=unbuffer<double>(messagePtr);
						double angularVel[4];
						for(int i=0;i<4;++i)
							angularVel[i]=unbuffer<double>(messagePtr);
						
						/* Flip the z axis if necessary: */
						if(flipZAxis)
							{
							/* Flip the linear velocity: */
							linearVel[2]=-linearVel[2];
							
							/* Flip the angular velocity (represented as a quaternion): */
							angularVel[0]=-angularVel[0];
							angularVel[1]=-angularVel[1];
							}
						
						/* Update the tracker data structure: */
						LinearVelocity l(linearVel);
						AngularVelocity a=PositionOrientation::Rotation::fromQuaternion(angularVel).getScaledAxis();
						updateTrackerVelocity(sd.trackerIndexBase+sensorNumber,l,a);
						}
					}
				}
			else if(messageType==buttonChangeMessage)
				{
				/* Get the sender's device descriptor: */
				SenderHasher::Iterator shIt=senders.findEntry(sender);
				if(!shIt.isFinished())
					{
					/* Get the device descriptor: */
					const SenderDescriptor& sd=shIt->getDest();
					
					/* Extract the button number from the message: */
					char* messagePtr=message;
					int buttonNumber=int(unbuffer<u_int32_t>(messagePtr));
					
					/* Check if the client is interested in this button: */
					if(buttonNumber<sd.numButtons)
						{
						/* Extract the button state: */
						int state=int(unbuffer<u_int32_t>(messagePtr));
						
						/* Update the button data structure: */
						updateButtonState(sd.buttonIndexBase+buttonNumber,state!=0);
						}
					}
				}
			else if(messageType==analogChannelMessage)
				{
				/* Get the sender's device descriptor: */
				SenderHasher::Iterator shIt=senders.findEntry(sender);
				if(!shIt.isFinished())
					{
					/* Get the device descriptor: */
					const SenderDescriptor& sd=shIt->getDest();
					
					/* Extract the number of channels from the message: */
					char* messagePtr=message;
					int numChannels=int(unbuffer<double>(messagePtr)); // What the hell?
					
					/* Copy up to the requested number of channels: */
					for(int channelIndex=0;channelIndex<numChannels&&channelIndex<sd.numValuators;++channelIndex)
						{
						/* Extract the valuator value: */
						double value=unbuffer<double>(messagePtr);
						
						/* Update the valuator data structure: */
						updateValuatorState(sd.valuatorIndexBase+channelIndex,value);
						}
					}
				}
			}
		}
	}

void VRPNConnection::finalizePacket(void)
	{
	/* Do nothing... */
	}

VRPNConnection::VRPNConnection(const char* serverName,int serverPort)
	:udpSocket(-1,-1),udpSocketConnected(false),
	 messageBufferSize(8192),messageBuffer(new char[messageBufferSize]),
	 senders(17),
	 trackerPosQuatMessage(-1),trackerVelocityMessage(-1),
	 buttonChangeMessage(-1),
	 analogChannelMessage(-1),
	 flipZAxis(false)
	{
	#if VRPNCONNECTION_USE_UDP
	
	/*********************************************************************
	Old-style connection initiation step 1: Send messages to the VRPN
	server until it connects back to a listening TCP socket. Retarded idea
	because it won't work across a firewall.
	*********************************************************************/
	
	{
	#ifdef VERBOSE
	printf("VRPNConnection: Waiting for server to back-connect to TCP socket...");
	fflush(stdout);
	#endif
	
	/* Set up a listening TCP socket to retrieve connection replies from the VRPN server: */
	Comm::TCPSocket listenSocket(-1,1);
	std::string listenSocketAddress=listenSocket.getAddress(); // My IP address in dotted notation; needed later (ouch)
	int listenSocketPortId=listenSocket.getPortId(); // My TCP port ID; needed later (ouch again)
	
	/* Create a UDP socket to send connection requests to the VRPN server: */
	Comm::UDPSocket serverSocket(-1,serverName,serverPort);
	bool connected=false;
	for(int attempt=0;attempt<15&&!connected;++attempt)
		{
		/* Send a connection request to the VRPN server's UDP listening socket: */
		char connectMessage[256];
		snprintf(connectMessage,sizeof(connectMessage),"%s %d",listenSocketAddress.c_str(),listenSocketPortId);
		serverSocket.sendMessage(connectMessage,strlen(connectMessage)+1);
		
		/* Wait for a connection request on the listening TCP socket, but not for too long: */
		Misc::FdSet requestFds(listenSocket.getFd());
		Misc::Time timeout(1,0); // Wait for at most 1 second
		int result=Misc::pselect(&requestFds,0,0,&timeout);
		if(result>0&&requestFds.isSet(listenSocket.getFd()))
			{
			/* Accept the connection from the VRPN server and set the TCP socket to TCP_NODELAY: */
			tcpSocket=listenSocket.accept();
			tcpSocket.setNoDelay(true);
			
			/* Done here: */
			connected=true;
			}
		}
	if(!connected)
		{
		#ifdef VERBOSE
		printf(" aborted\n");
		fflush(stdout);
		#endif
		Misc::throwStdErr("VRPNConnection::VRPNConnection: Unable to connect to VRPN server on host %s, port %d",serverName,serverPort);
		}
	
	#ifdef VERBOSE
	printf(" done\n");
	fflush(stdout);
	#endif
	}
	
	#else
	
	/*********************************************************************
	New-style connection initiation step 1: Directly open a TCP connection
	to the VRPN server's port:
	*********************************************************************/
	
	tcpSocket.connect(serverName,serverPort);
	tcpSocket.setNoDelay(true);
	
	#endif
	
	/*********************************************************************
	Step 2: Check if the VRPN server is of a compatible version.
	*********************************************************************/
	
	{
	#ifdef VERBOSE
	printf("VRPNConnection: Checking server version number...");
	fflush(stdout);
	#endif
	
	/* Initiate the connection by sending the VRPN "magic cookie" to the server: */
	char message[versionMessageLength+alignment+1];
	snprintf(message,sizeof(message),"%s  0",versionMessage);
	tcpSocket.blockingWrite(message,versionMessageLength+alignment);
	
	/* Wait for the VRPN server's reply for at most 3 seconds: */
	if(!tcpSocket.waitForData(3,0))
		{
		#ifdef VERBOSE
		printf(" aborted\n");
		fflush(stdout);
		#endif
		Misc::throwStdErr("VRPNConnection::VRPNConnection: Unable to connect to VRPN server on host %s, port %d",serverName,serverPort);
		}
	
	/* Read the VRPN server's reply: */
	tcpSocket.blockingRead(message,versionMessageLength+alignment);
	message[versionMessageLength+alignment]='\0';
	
	/* Find the dot separating the major and minor version numbers encoded in the version message: */
	const char* dotPtr=0;
	for(const char* mPtr=message;*mPtr!='\0';++mPtr)
		if(*mPtr=='.')
			dotPtr=mPtr;
	if(dotPtr!=0)
		{
		/* Compare only up to the major protocol version number: */
		if(strncmp(message,versionMessage,dotPtr-message)!=0)
			{
			#ifdef VERBOSE
			printf(" incompatible version\n");
			fflush(stdout);
			#endif
			Misc::throwStdErr("VRPNConnection::VRPNConnection: VRPN server on host %s, port %d sent incompatible version message %s",serverName,serverPort,message);
			}
		}
	else
		{
		/* Compare the entire cookie: */
		if(strncmp(message,versionMessage,versionMessageLength)!=0)
			{
			#ifdef VERBOSE
			printf(" incompatible version\n");
			fflush(stdout);
			#endif
			Misc::throwStdErr("VRPNConnection::VRPNConnection: VRPN server on host %s, port %d sent incompatible version message %s",serverName,serverPort,message);
			}
		}
	
	#ifdef VERBOSE
	printf(" done\n");
	message[versionMessageLength]='\0';
	printf("VRPNConnection: Server version is %s\n",message);
	fflush(stdout);
	#endif
	}
	
	#if VRPNCONNECTION_USE_UDP
	
	/*********************************************************************
	Step 3: Send the address and port number of our UDP socket to the VRPN
	server.
	*********************************************************************/
	
	{
	#ifdef VERBOSE
	printf("VRPNConnection: Waiting for VRPN server to connect to UPD socket...");
	fflush(stdout);
	#endif
	
	/* Send the UPD socket's address to the VRPN server: */
	std::string socketAddress=tcpSocket.getAddress();
	sendMessage(socketAddress.length()+1,Misc::Time::now(),UDP_DESCRIPTION,(unsigned int)(udpSocket.getPortId()),socketAddress.c_str(),RELIABLE);
	
	/* Receive the address of the VRPN server's UDP socket: */
	while(!udpSocketConnected)
		readNextMessages();
	
	#ifdef VERBOSE
	printf(" done\n");
	fflush(stdout);
	#endif
	}
	
	#endif
	}

VRPNConnection::~VRPNConnection(void)
	{
	delete[] messageBuffer;
	}

void VRPNConnection::requestTrackers(const char* senderName,int trackerIndexBase,int numTrackers)
	{
	/* Check if the same sender has already been requested: */
	unsigned int index;
	for(index=0;index<requestedSenders.size()&&requestedSenders[index].first!=senderName;++index)
		;
	if(index<requestedSenders.size())
		{
		/* Add tracker device information to the existing sender descriptor: */
		requestedSenders[index].second.trackerIndexBase=trackerIndexBase;
		requestedSenders[index].second.numTrackers=numTrackers;
		}
	else
		{
		/* Add another sender descriptor: */
		SenderDescriptor sd;
		sd.trackerIndexBase=trackerIndexBase;
		sd.numTrackers=numTrackers;
		requestedSenders.push_back(SenderRequest(senderName,sd));
		
		/* Send a sender request to the VRPN server: */
		char nameBuffer[sizeof(u_int32_t)+256];
		char* namePtr=nameBuffer;
		size_t nameLen=strlen(senderName)+1;
		buffer<u_int32_t>(namePtr,nameLen);
		memcpy(namePtr,senderName,nameLen);
		sendMessage(sizeof(u_int32_t)+nameLen,Misc::Time::now(),SENDER_DESCRIPTION,requestedSenders.size()-1,nameBuffer,RELIABLE);
		}
	}

void VRPNConnection::setFlipZAxis(bool newFlipZAxis)
	{
	/* Set the flipping flag: */
	flipZAxis=newFlipZAxis;
	}

void VRPNConnection::requestButtons(const char* senderName,int buttonIndexBase,int numButtons)
	{
	/* Check if the same sender has already been requested: */
	unsigned int index;
	for(index=0;index<requestedSenders.size()&&requestedSenders[index].first!=senderName;++index)
		;
	if(index<requestedSenders.size())
		{
		/* Add button device information to the existing sender descriptor: */
		requestedSenders[index].second.buttonIndexBase=buttonIndexBase;
		requestedSenders[index].second.numButtons=numButtons;
		}
	else
		{
		/* Add another sender descriptor: */
		SenderDescriptor sd;
		sd.buttonIndexBase=buttonIndexBase;
		sd.numButtons=numButtons;
		requestedSenders.push_back(SenderRequest(senderName,sd));
		
		/* Send a sender request to the VRPN server: */
		char nameBuffer[sizeof(u_int32_t)+256];
		char* namePtr=nameBuffer;
		size_t nameLen=strlen(senderName)+1;
		buffer<u_int32_t>(namePtr,nameLen);
		memcpy(namePtr,senderName,nameLen);
		sendMessage(sizeof(u_int32_t)+nameLen,Misc::Time::now(),SENDER_DESCRIPTION,requestedSenders.size()-1,nameBuffer,RELIABLE);
		}
	}

void VRPNConnection::requestValuators(const char* senderName,int valuatorIndexBase,int numValuators)
	{
	/* Check if the same sender has already been requested: */
	unsigned int index;
	for(index=0;index<requestedSenders.size()&&requestedSenders[index].first!=senderName;++index)
		;
	if(index<requestedSenders.size())
		{
		/* Add valuator device information to the existing sender descriptor: */
		requestedSenders[index].second.valuatorIndexBase=valuatorIndexBase;
		requestedSenders[index].second.numValuators=numValuators;
		}
	else
		{
		/* Add another sender descriptor: */
		SenderDescriptor sd;
		sd.valuatorIndexBase=valuatorIndexBase;
		sd.numValuators=numValuators;
		requestedSenders.push_back(SenderRequest(senderName,sd));
		
		/* Send a sender request to the VRPN server: */
		char nameBuffer[sizeof(u_int32_t)+256];
		char* namePtr=nameBuffer;
		size_t nameLen=strlen(senderName)+1;
		buffer<u_int32_t>(namePtr,nameLen);
		memcpy(namePtr,senderName,nameLen);
		sendMessage(sizeof(u_int32_t)+nameLen,Misc::Time::now(),SENDER_DESCRIPTION,requestedSenders.size()-1,nameBuffer,RELIABLE);
		}
	}

void VRPNConnection::readNextMessages(void)
	{
	/* Wait for the next message on either the UDP or the TCP socket: */
	Misc::FdSet readFds;
	readFds.add(tcpSocket.getFd());
	if(udpSocketConnected)
		readFds.add(udpSocket.getFd());
	Misc::pselect(&readFds,0,0,0);
	
	/* Read the next message(s): */
	if(udpSocketConnected&&readFds.isSet(udpSocket.getFd()))
		{
		/* Read a message from the UDP socket: */
		size_t packetSize=udpSocket.receiveMessage(messageBuffer,messageBufferSize);
		
		/* Process all messages contained in the packet: */
		char* packetPtr=messageBuffer;
		while(packetSize>0)
			{
			/* Extract the message header: */
			char* messagePtr=packetPtr;
			size_t totalLen=unbuffer<u_int32_t>(messagePtr);
			Misc::Time messageTime;
			messageTime.tv_sec=unbuffer<u_int32_t>(messagePtr);
			messageTime.tv_nsec=unbuffer<u_int32_t>(messagePtr)*1000;
			unsigned int sender=unbuffer<u_int32_t>(messagePtr);
			int messageType=unbuffer<int32_t>(messagePtr);
			
			/* Skip the padding: */
			size_t headerLen=pad(4*sizeof(u_int32_t)+sizeof(int32_t));
			messagePtr=packetPtr+headerLen;
			
			/* Read the message payload: */
			size_t messageSize=totalLen-headerLen;
			char* message=messagePtr;
			
			/* Handle the message: */
			handleMessage(messageTime,messageType,sender,messageSize,message);
			
			/* Go to the next message: */
			totalLen=pad(totalLen);
			packetPtr+=totalLen;
			packetSize-=totalLen;
			}
		}
	else if(readFds.isSet(tcpSocket.getFd()))
		{
		/* Read all available data from the TCP socket: */
		size_t packetSize=tcpSocket.read(messageBuffer,messageBufferSize);
		
		/* Make sure we read at least one message header: */
		size_t headerLen=pad(4*sizeof(u_int32_t)+sizeof(int32_t));
		if(packetSize<headerLen)
			{
			/* Read the rest of the padded header: */
			tcpSocket.blockingRead(messageBuffer+packetSize,headerLen-packetSize);
			packetSize=headerLen;
			}
		
		/* Process all messages contained in the packet: */
		char* packetPtr=messageBuffer;
		while(packetSize>0)
			{
			/* Extract the message header: */
			char* headerPtr=packetPtr;
			size_t totalLen=unbuffer<u_int32_t>(headerPtr);
			Misc::Time messageTime;
			messageTime.tv_sec=unbuffer<u_int32_t>(headerPtr);
			messageTime.tv_nsec=unbuffer<u_int32_t>(headerPtr)*1000;
			unsigned int sender=unbuffer<u_int32_t>(headerPtr);
			int messageType=unbuffer<int32_t>(headerPtr);
			
			/* Determine the length and padded length of the message payload: */
			size_t messageSize=totalLen-pad(4*sizeof(u_int32_t)+sizeof(int32_t));
			size_t messageLen=pad(messageSize);
			
			/* Check if the entire message payload has been read: */
			packetPtr+=headerLen;
			packetSize-=headerLen;
			if(packetSize<messageLen)
				{
				/* Check if the message buffer is big enough to hold the entire message: */
				if(messageBufferSize<messageLen)
					{
					/* Re-allocate the message buffer: */
					messageBufferSize=(messageLen*5)/4;
					char* newMessageBuffer=new char[messageBufferSize];
					memcpy(newMessageBuffer,packetPtr,packetSize);
					delete[] messageBuffer;
					messageBuffer=newMessageBuffer;
					}
				else
					{
					/* Move the already read message part to the beginning of the buffer: */
					memcpy(messageBuffer,packetPtr,packetSize);
					}
				
				/* Read the rest of the message payload: */
				packetPtr=messageBuffer;
				tcpSocket.blockingRead(packetPtr+packetSize,messageLen-packetSize);
				packetSize=messageLen;
				}
			
			/* Handle the message: */
			handleMessage(messageTime,messageType,sender,messageSize,packetPtr);
			
			/* Go to the next message: */
			packetPtr+=messageLen;
			packetSize-=messageLen;
			}
		}
	
	/* Finish processing the packet: */
	finalizePacket();
	}
