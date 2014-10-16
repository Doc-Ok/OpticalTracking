/***********************************************************************
Client to read tracking data from a NaturalPoint OptiTrack tracking
system.
Copyright (c) 2010-2012 Oliver Kreylos

This file is part of the Vrui calibration utility package.

The Vrui calibration utility package is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Vrui calibration utility package is distributed in the hope that it
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Vrui calibration utility package; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#include "NaturalPointClient.h"

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string>
#include <iostream>
#include <Misc/ThrowStdErr.h>

namespace {

enum MessageIds
	{
	NAT_PING=0,NAT_PINGRESPONSE=1,
	NAT_REQUEST=2,NAT_RESPONSE=3,
	NAT_REQUEST_MODELDEF=4,NAT_MODELDEF=5,
	NAT_REQUEST_FRAMEOFDATA=6,NAT_FRAMEOFDATA=7,
	NAT_MESSAGESTRING=8,
	NAT_UNRECOGNIZED_REQUEST=100
	};

std::string readString(PacketBuffer& packet)
	{
	std::string result;
	char c;
	while((c=packet.read<char>())!='\0')
		result.push_back(c);
	return result;
	}

}

/***********************************
Methods of class NaturalPointClient:
***********************************/

void NaturalPointClient::handlePacket(PacketBuffer& packet)
	{
	/* Parse the packet: */
	packet.rewind();
	unsigned int messageId=packet.read<unsigned short>();
	packet.read<unsigned short>();
	switch(messageId)
		{
		case NAT_PINGRESPONSE:
			{
			/* Read the server's application name: */
			char appName[257];
			packet.read<char>(appName,256);
			appName[256]='\0';
			serverName=appName;
			
			/* Read the server's application version number: */
			unsigned char appVersion[4];
			packet.read<unsigned char>(appVersion,4);
			for(int i=0;i<4;++i)
				serverVersion[i]=int(appVersion[i]);
			
			/* Read the protocol version: */
			unsigned char protVersion[4];
			packet.read<unsigned char>(protVersion,4);
			for(int i=0;i<4;++i)
				protocolVersion[i]=int(protVersion[i]);
			
			/* Notify anyone waiting for a ping: */
			pingCond.broadcast();
			break;
			}
		
		case NAT_MODELDEF:
			/* Check if there is a model definition structure waiting to be filled in: */
			if(nextModelDef!=0)
				{
				/* Clear the model definition structure: */
				nextModelDef->markerSets.clear();
				nextModelDef->rigidBodies.clear();
				nextModelDef->skeletons.clear();
				
				/* Read number of data sets: */
				int numDataSets=packet.read<int>();
				for(int dataSet=0;dataSet<numDataSets;++dataSet)
					{
					int dataSetType=packet.read<int>();
					switch(dataSetType)
						{
						case 0: // Marker set
							{
							MarkerSetDef ms;
							
							/* Read the marker set's name: */
							ms.name=readString(packet);
							
							/* Read all markers: */
							int numMarkers=packet.read<int>();
							for(int i=0;i<numMarkers;++i)
								{
								/* Read the marker's name: */
								ms.markerNames.push_back(readString(packet));
								}
							
							/* Store the marker set: */
							nextModelDef->markerSets.push_back(ms);
							break;
							}
						
						case 1: // Rigid body
							{
							RigidBodyDef rb;
							
							if(protocolVersion[0]>=2)
								{
								/* Read the rigid body's name: */
								rb.name=readString(packet);
								}
							
							/* Read the rigid body's ID, parent ID, and parent offset: */
							rb.id=packet.read<int>();
							rb.parentId=packet.read<int>();
							packet.read<Scalar>(rb.offset.getComponents(),3);
							
							/* Store the rigid body: */
							nextModelDef->rigidBodies.push_back(rb);
							break;
							}
						
						case 2: // Skeleton
							{
							SkeletonDef s;
							
							/* Read the skeleton's name: */
							s.name=readString(packet);
							
							/* Read the skeleton's ID: */
							s.id=packet.read<int>();
							
							/* Read all rigid bodies: */
							int numRigidBodies=packet.read<int>();
							for(int i=0;i<numRigidBodies;++i)
								{
								RigidBodyDef rb;
								
								if(protocolVersion[0]>=2)
									{
									/* Read the rigid body's name: */
									rb.name=readString(packet);
									}
								
								/* Read the rigid body's ID, parent ID, and parent offset: */
								rb.id=packet.read<int>();
								rb.parentId=packet.read<int>();
								packet.read<Scalar>(rb.offset.getComponents(),3);
								
								/* Store the rigid body: */
								s.rigidBodies.push_back(rb);
								}
							
							/* Store the skeleton: */
							nextModelDef->skeletons.push_back(s);
							break;
							}
						}
					}
				
				/* Protect the model definition: */
				nextModelDef=0;
				}
			
			/* Notify anyone waiting for a model definition: */
			modelDefCond.broadcast();
			break;
		
		case NAT_FRAMEOFDATA:
			{
			/* Start a new frame in the triple buffer: */
			Frame& frame=frames.startNewValue();
			
			/* Read the frame number: */
			frame.number=packet.read<int>();
			
			/* Read all marker sets: */
			int numMarkerSets=packet.read<int>();
			std::vector<MarkerSet>::iterator msIt=frame.markerSets.begin();
			for(int markerSet=0;markerSet<numMarkerSets;++markerSet,++msIt)
				{
				/* Check if all allocated marker sets have been used: */
				if(msIt==frame.markerSets.end())
					{
					/* Extend the marker set array: */
					frame.markerSets.push_back(MarkerSet());
					msIt=frame.markerSets.end()-1;
					}
				
				/* Read the marker set's name: */
				msIt->name=readString(packet);
				
				/* Read all markers in the set: */
				int numMarkers=packet.read<int>();
				std::vector<Point>::iterator mIt=msIt->markers.begin();
				for(int i=0;i<numMarkers;++i,++mIt)
					{
					/* Check if all allocated markers have been used: */
					if(mIt==msIt->markers.end())
						{
						/* Extend the marker array: */
						msIt->markers.push_back(Point());
						mIt=msIt->markers.end()-1;
						}
					
					/* Read the marker's position: */
					packet.read<Scalar>(mIt->getComponents(),3);
					}
				
				/* Delete all unused markers: */
				msIt->markers.erase(mIt,msIt->markers.end());
				}
			
			/* Delete all unused marker sets: */
			frame.markerSets.erase(msIt,frame.markerSets.end());
			
			/* Read all unidentified markers: */
			int numOtherMarkers=packet.read<int>();
			std::vector<Point>::iterator omIt=frame.otherMarkers.begin();
			for(int i=0;i<numOtherMarkers;++i,++omIt)
				{
				/* Check if all allocated markers have been used: */
				if(omIt==frame.otherMarkers.end())
					{
					/* Extend the marker array: */
					frame.otherMarkers.push_back(Point());
					omIt=frame.otherMarkers.end()-1;
					}
				
				/* Read the marker's position: */
				packet.read<Scalar>(omIt->getComponents(),3);
				}
			
			/* Delete all unused unidentified markers: */
			frame.otherMarkers.erase(omIt,frame.otherMarkers.end());
			
			/* Read all rigid bodies: */
			int numRigidBodies=packet.read<int>();
			std::vector<RigidBody>::iterator rbIt=frame.rigidBodies.begin();
			for(int i=0;i<numRigidBodies;++i,++rbIt)
				{
				/* Check if all allocated rigid bodies have been used: */
				if(rbIt==frame.rigidBodies.end())
					{
					/* Extend the rigid body array: */
					frame.rigidBodies.push_back(RigidBody());
					rbIt=frame.rigidBodies.end()-1;
					}
				
				/* Read the body ID and position/orientation: */
				rbIt->id=packet.read<int>();
				packet.read<Scalar>(rbIt->position.getComponents(),3);
				Scalar quat[4];
				packet.read<Scalar>(quat,4);
				rbIt->orientation=Rotation::fromQuaternion(quat);
				
				/* Read the body's associated markers: */
				int numBodyMarkers=packet.read<int>();
				std::vector<Point>::iterator mIt=rbIt->markers.begin();
				for(int j=0;j<numBodyMarkers;++j,++mIt)
					{
					/* Check if all allocated markers have been used: */
					if(mIt==rbIt->markers.end())
						{
						/* Extend the marker array: */
						rbIt->markers.push_back(Point());
						mIt=rbIt->markers.end()-1;
						}
					
					/* Read the marker position: */
					packet.read<Scalar>(mIt->getComponents(),3);
					}
				
				/* Erase all unused markers: */
				rbIt->markers.erase(mIt,rbIt->markers.end());
				
				if(protocolVersion[0]>=2)
					{
					/* Read marker IDs: */
					std::vector<int>::iterator midIt=rbIt->markerIds.begin();
					for(int j=0;j<numBodyMarkers;++j,++midIt)
						{
						/* Check if all allocated marker IDs have been used: */
						if(midIt==rbIt->markerIds.end())
							{
							/* Extend the marker ID array: */
							rbIt->markerIds.push_back(0);
							midIt=rbIt->markerIds.end()-1;
							}
						
						/* Read the marker ID: */
						*midIt=packet.read<int>();
						}
					
					/* Erase all unused marker IDs: */
					rbIt->markerIds.erase(midIt,rbIt->markerIds.end());
					
					/* Read marker sizes: */
					std::vector<Scalar>::iterator msIt=rbIt->markerSizes.begin();
					for(int j=0;j<numBodyMarkers;++j,++msIt)
						{
						/* Check if all allocated marker sizes have been used: */
						if(msIt==rbIt->markerSizes.end())
							{
							/* Extend the marker size array: */
							rbIt->markerSizes.push_back(0);
							msIt=rbIt->markerSizes.end()-1;
							}
						
						/* Read the marker size: */
						*msIt=packet.read<Scalar>();
						}
					
					/* Erase all unused marker sizes: */
					rbIt->markerSizes.erase(msIt,rbIt->markerSizes.end());
					
					/* Read mean marker error: */
					rbIt->meanMarkerError=packet.read<Scalar>();
					}
				}
			
			/* Delete all unused rigid bodies: */
			frame.rigidBodies.erase(rbIt,frame.rigidBodies.end());
			
			if(protocolVersion[0]>2||(protocolVersion[0]==2&&protocolVersion[1]>0))
				{
				frame.skeletons.clear();
				
				/* Read all skeletons: */
				int numSkeletons=packet.read<int>();
				for(int i=0;i<numSkeletons;++i)
					{
					Skeleton s;
					s.id=packet.read<int>();
					int numSkeletonRigidBodies=packet.read<int>();
					for(int j=0;j<numSkeletonRigidBodies;++j)
						{
						RigidBody rb;
						
						/* Read the body ID and position/orientation: */
						rb.id=packet.read<int>();
						packet.read<Scalar>(rb.position.getComponents(),3);
						Scalar quat[4];
						packet.read<Scalar>(quat,4);
						rb.orientation=Rotation::fromQuaternion(quat);
						
						/* Read the body's associated markers: */
						int numBodyMarkers=packet.read<int>();
						for(int k=0;k<numBodyMarkers;++k)
							{
							Point marker;
							packet.read<Scalar>(marker.getComponents(),3);
							rb.markers.push_back(marker);
							}
						
						/* Read marker IDs: */
						for(int k=0;k<numBodyMarkers;++k)
							rb.markerIds.push_back(packet.read<int>());
						
						/* Read marker sizes: */
						for(int k=0;k<numBodyMarkers;++k)
							rb.markerSizes.push_back(packet.read<Scalar>());
						
						/* Read mean marker error: */
						rb.meanMarkerError=packet.read<Scalar>();
						
						/* Store the rigid body: */
						s.rigidBodies.push_back(rb);
						}
					
					/* Store the skeleton: */
					frame.skeletons.push_back(s);
					}
				}
			
			/* Read latency: */
			frame.latency=packet.read<int>();
			
			/* Read end-of-data tag: */
			packet.read<int>();
			
			/* Notify anyone waiting for a frame: */
			frames.postNewValue();
			frameCond.broadcast();
			break;
			}
		
		default:
			;
		}
	}

void* NaturalPointClient::commandHandlingThreadMethod(void)
	{
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	// Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	while(true)
		{
		/* Read the next data packet from the server: */
		size_t numBytesReceived=commandSocket.receiveMessage(commandReplyBuffer.getPacket(),commandReplyBuffer.getMaxPacketSize());
		commandReplyBuffer.setPacketSize(numBytesReceived);
		
		/* Handle the packet: */
		handlePacket(commandReplyBuffer);
		}
	
	return 0;
	}

void* NaturalPointClient::dataHandlingThreadMethod(void)
	{
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	// Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	while(true)
		{
		/* Read the next data packet from the server: */
		ssize_t numBytesReceived=recv(dataSocketFd,dataBuffer.getPacket(),dataBuffer.getMaxPacketSize(),0);
		if(numBytesReceived>0)
			{
			/* Handle the packet: */
			dataBuffer.setPacketSize(numBytesReceived);
			handlePacket(dataBuffer);
			}
		}
	
	return 0;
	}

NaturalPointClient::NaturalPointClient(const char* serverHostName,int commandPort,const char* dataMulticastGroup,int dataPort)
	:commandSocket(-1,serverHostName,commandPort),commandBuffer(1024,PacketBuffer::LittleEndian),
	 commandReplyBuffer(65536,PacketBuffer::LittleEndian),
	 dataSocketFd(0),dataBuffer(65536,PacketBuffer::LittleEndian),
	 nextModelDef(0)
	{
	/* Create the data multicast UDP socket: */
	dataSocketFd=socket(PF_INET,SOCK_DGRAM,0);
	if(dataSocketFd<0)
		Misc::throwStdErr("NaturalPointClient: Unable to create data socket");
	
	/* Bind the data socket to the local address/port number: */
	struct sockaddr_in dataSocketAddress;
	dataSocketAddress.sin_family=AF_INET;
	dataSocketAddress.sin_port=htons(dataPort);
	dataSocketAddress.sin_addr.s_addr=htonl(INADDR_ANY);
	if(bind(dataSocketFd,(struct sockaddr*)&dataSocketAddress,sizeof(struct sockaddr_in))==-1)
		{
		close(dataSocketFd);
		Misc::throwStdErr("NaturalPointClient: Unable to bind data socket to port %d",dataPort);
		}
	
	/* Lookup server's IP address: */
	struct hostent* serverEntry=gethostbyname(serverHostName);
	if(serverEntry==0)
		{
		close(dataSocketFd);
		Misc::throwStdErr("NaturalPointClient: Unable to resolve server %s",serverHostName);
		}
	struct in_addr serverAddress;
	serverAddress.s_addr=ntohl(((struct in_addr*)serverEntry->h_addr_list[0])->s_addr);
	
	/* Lookup data multicast group's IP address: */
	struct hostent* dataEntry=gethostbyname(dataMulticastGroup);
	if(dataEntry==0)
		{
		close(dataSocketFd);
		Misc::throwStdErr("NaturalPointClient: Unable to resolve data multicast group %s",dataMulticastGroup);
		}
	struct in_addr dataMulticastAddress;
	dataMulticastAddress.s_addr=ntohl(((struct in_addr*)dataEntry->h_addr_list[0])->s_addr);
	
	#if 0
	/* Connect the data socket to the server's multicast socket: */
	struct sockaddr_in dataServerAddress;
	dataServerAddress.sin_family=AF_INET;
	dataServerAddress.sin_port=htons(dataPort);
	dataServerAddress.sin_addr.s_addr=htonl(serverAddress.s_addr);
	if(connect(dataSocketFd,(const struct sockaddr*)&dataServerAddress,sizeof(struct sockaddr_in))<0)
		{
		close(dataSocketFd);
		Misc::throwStdErr("NaturalPointClient: Unable to connect data socket to multicast address %s",dataMulticastGroup);
		}
	
	/* Get the data socket's local IP address: */
	struct sockaddr_in dataLocalAddress;
	socklen_t dataLocalAddressLen=sizeof(struct sockaddr_in);
	if(getsockname(dataSocketFd,(struct sockaddr*)&dataLocalAddress,&dataLocalAddressLen)<0)
		{
		close(dataSocketFd);
		Misc::throwStdErr("NaturalPointClient: Unable to query data socket's local address");
		}
	#endif
	
	/* Enable broadcast handling for the data socket: */
	int broadcastFlag=1;
	if(setsockopt(dataSocketFd,SOL_SOCKET,SO_BROADCAST,&broadcastFlag,sizeof(int))<0)
		{
		int myerrno=errno;
		close(dataSocketFd);
		Misc::throwStdErr("NaturalPointClient: error %s during setsockopt",strerror(myerrno));
		}
	
	/* Join the data multicast group: */
	struct ip_mreq addGroupRequest;
	addGroupRequest.imr_multiaddr.s_addr=htonl(dataMulticastAddress.s_addr);
	addGroupRequest.imr_interface.s_addr=htonl(INADDR_ANY); // dataLocalAddress.sin_addr.s_addr;
	if(setsockopt(dataSocketFd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&addGroupRequest,sizeof(struct ip_mreq))<0)
		{
		int myerrno=errno;
		close(dataSocketFd);
		Misc::throwStdErr("NaturalPointClient: error %s during setsockopt",strerror(myerrno));
		}
	
	/* Start the command response receiving thread: */
	commandHandlingThread.start(this,&NaturalPointClient::commandHandlingThreadMethod);
	
	/* Start the data receiving thread: */
	dataHandlingThread.start(this,&NaturalPointClient::dataHandlingThreadMethod);
	
	int numPingTries;
	for(numPingTries=0;numPingTries<5;++numPingTries)
		{
		/* Send a ping request: */
		commandBuffer.clear();
		commandBuffer.write<unsigned short>(NAT_PING); // Message ID
		commandBuffer.write<unsigned short>(0); // Data size
		commandSocket.sendMessage(commandBuffer.getPacket(),commandBuffer.getPacketSize());

		/* Block until the server replies: */
		if(pingCond.timedWait(Misc::Time::now()+Misc::Time(1.0)))
			break;
		}
	if(numPingTries>=5)
		{
		/* Clean up and signal a connection error: */
		commandHandlingThread.cancel();
		commandHandlingThread.join();
		dataHandlingThread.cancel();
		dataHandlingThread.join();
		close(dataSocketFd);
		Misc::throwStdErr("NaturalPointClient: Unable to connect to server %s",serverHostName);
		}
	}

NaturalPointClient::~NaturalPointClient(void)
	{
	commandHandlingThread.cancel();
	commandHandlingThread.join();
	dataHandlingThread.cancel();
	dataHandlingThread.join();
	close(dataSocketFd);
	}

NaturalPointClient::ModelDef& NaturalPointClient::queryModelDef(NaturalPointClient::ModelDef& modelDef)
	{
	/* Store a pointer to the model definition structure: */
	nextModelDef=&modelDef;
	
	/* Send a model definition request: */
	commandBuffer.clear();
	commandBuffer.write<unsigned short>(NAT_REQUEST_MODELDEF); // Message ID
	commandBuffer.write<unsigned short>(0); // Data size
	commandSocket.sendMessage(commandBuffer.getPacket(),commandBuffer.getPacketSize());
	
	/* Wait for the server's response: */
	modelDefCond.wait();
	
	/* Return the changed structure: */
	return modelDef;
	}

const NaturalPointClient::Frame& NaturalPointClient::requestFrame(void)
	{
	/* Send a data frame request: */
	commandBuffer.clear();
	commandBuffer.write<unsigned short>(NAT_REQUEST_FRAMEOFDATA); // Message ID
	commandBuffer.write<unsigned short>(0); // Data size
	commandSocket.sendMessage(commandBuffer.getPacket(),commandBuffer.getPacketSize());
	
	/* Wait for the server's response: */
	frameCond.wait();
	
	/* Lock and return the most recent frame: */
	frames.lockNewValue();
	return frames.getLockedValue();
	}

const NaturalPointClient::Frame& NaturalPointClient::waitForNextFrame(void)
	{
	/* Check if the most recent frame has already been returned: */
	if(!frames.lockNewValue())
		{
		/* Wait for the next data frame from the server: */
		frameCond.wait();
		
		/* Lock it: */
		frames.lockNewValue();
		}
	
	/* Return the most recent frame: */
	return frames.getLockedValue();
	}
