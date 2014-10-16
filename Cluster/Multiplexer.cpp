/***********************************************************************
Multiplexer - Class to share several intra-cluster multicast pipes
across a single UDP socket connection.
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

#include <Cluster/Multiplexer.h>

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <Misc/ThrowStdErr.h>
#include <Cluster/Config.h>

#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER
#include <iostream>
#endif

namespace Cluster {

namespace {

/****************
Helper functions:
****************/

inline bool isMulticast(const struct in_addr& netAddress) // Returns true if the given IP address is in the defined multicast address range
	{
	int address=netAddress.s_addr;
	return address>=(0xe0<<24)&&address<(0xf0<<24);
	}

}

/***************************************************
Methods of class Multiplexer::PipeState::PacketList:
***************************************************/

Multiplexer::PipeState::PacketList::PacketList(void)
	:numPackets(0),head(0),tail(0)
	{
	}

Multiplexer::PipeState::PacketList::~PacketList(void)
	{
	/* Delete all packets in the queue: */
	while(head!=0)
		{
		Packet* succ=head->succ;
		delete head;
		head=succ;
		}
	}

void Multiplexer::PipeState::PacketList::push_back(Packet* packet)
	{
	/* Mark the packet as the last in the list: */
	packet->succ=0;
	
	/* Append it to the end of the list: */
	if(tail!=0)
		tail->succ=packet;
	else
		head=packet;
	
	/* Update the tail pointer: */
	tail=packet;
	
	/* Increase number of packets: */
	++numPackets;
	}

Packet* Multiplexer::PipeState::PacketList::pop_front(void)
	{
	/* Store pointer to first packet: */
	Packet* result=head;
	
	/* Update the head pointer: */
	head=result->succ;
	
	/* Reset the tail pointer if the list is now empty: */
	if(head==0)
		tail=0;
	
	/* Decrease number of packets: */
	--numPackets;
	
	/* Mark the result packet as singleton and return it: */
	result->succ=0;
	return result;
	}

/***************************************
Methods of class Multiplexer::PipeState:
***************************************/

Multiplexer::PipeState::PipeState(unsigned int nodeIndex,unsigned int numSlaves)
	:pipeId(0),
	 streamPos(0),packetLossMode(false),
	 headStreamPos(0),
	 slaveStreamPosOffsets(0),numHeadSlaves(0),
	 barrierId(0),slaveBarrierIds(0),minSlaveBarrierId(0),
	 slaveGatherValues(0)
	 #if CLUSTER_CONFIG_DEBUG_MULTIPLEXER
	 ,
	 numResentPackets(0),numResentBytes(0)
	 #endif
	{
	if(nodeIndex==0)
		{
		/* Initialize the slave stream position offset array: */
		slaveStreamPosOffsets=new unsigned int[numSlaves];
		for(unsigned int i=0;i<numSlaves;++i)
			slaveStreamPosOffsets[i]=0;
		numHeadSlaves=numSlaves;
		
		/* Initialize the slave barrier ID array: */
		slaveBarrierIds=new unsigned int[numSlaves];
		for(unsigned int i=0;i<numSlaves;++i)
			slaveBarrierIds[i]=0;
		
		/* Initialize the slave gather value array: */
		slaveGatherValues=new unsigned int[numSlaves];
		for(unsigned int i=0;i<numSlaves;++i)
			slaveBarrierIds[i]=0;
		}
	}

Multiplexer::PipeState::~PipeState(void)
	{
	{
	Threads::Mutex::Lock stateLock(stateMutex);
	
	/* Destroy stream position array: */
	delete[] slaveStreamPosOffsets;
	
	/* Destroy barrier ID array: */
	delete[] slaveBarrierIds;
	
	/* Destroy slave gather value array: */
	delete[] slaveGatherValues;
	}
	}

namespace {

/**************
Helper classes:
**************/

struct Message // Structure for protocol messages exchanged between master and slaves
	{
	/* Embedded classes: */
	public:
	enum MessageId // Enumerated type for message IDs
		{
		CONNECTION, // Signal that slave joined multicast group
		PING, // Ping request from slave to master
		CREATEPIPE1,CREATEPIPE2, // Signal that slave completed pipe creation's first and second stage, respectively
		ACKNOWLEDGMENT, // Signal that slave has received some stream packets
		PACKETLOSS, // Signal that slave lost a stream packet
		BARRIER, // Barrier message sent from slaves to master
		GATHER // Message conveying a slave's gather value in a gather operation
		};
	
	/* Elements: */
	unsigned int nodeIndex; // Index of node that sent this message, with the MSB set if the message is from a slave to the master
	int messageId; // ID of message
	
	/* Constructors and destructors: */
	Message(unsigned int sNodeIndex,int sMessageId)
		:nodeIndex(sNodeIndex),messageId(sMessageId)
		{
		}
	};

struct PipeMessage:public Message
	{
	/* Elements: */
	public:
	unsigned int pipeId; // ID of pipe for which this message is intended
	
	/* Constructors and destructors: */
	PipeMessage(unsigned int sNodeIndex,int sMessageId,unsigned int sPipeId)
		:Message(sNodeIndex,sMessageId),
		 pipeId(sPipeId)
		{
		}
	};

struct CreatePipe1Message:public PipeMessage
	{
	/* Elements: */
	public:
	unsigned int idNumParts; // Number of partial IDs in the sending thread's global thread ID
	
	/* Constructors and destructors: */
	CreatePipe1Message(unsigned int sNodeIndex,unsigned int sPipeId,unsigned int sIdNumParts)
		:PipeMessage(sNodeIndex,CREATEPIPE1,sPipeId),
		 idNumParts(sIdNumParts)
		{
		}
	};

struct StreamMessage:public PipeMessage
	{
	/* Elements: */
	public:
	unsigned int streamPos; // Current stream position of slave when packet loss is detected
	unsigned int packetPos; // Stream position of packet after packet loss
	
	/* Constructors and destructors: */
	StreamMessage(unsigned int sNodeIndex,int sMessageId,unsigned int sPipeId,unsigned int sStreamPos,unsigned int sPacketPos)
		:PipeMessage(sNodeIndex,sMessageId,sPipeId),
		 streamPos(sStreamPos),packetPos(sPacketPos)
		{
		}
	};

struct BarrierMessage:public PipeMessage
	{
	/* Elements: */
	public:
	unsigned int barrierId; // ID of current barrier in barrier message or gather operation in gather message
	
	/* Constructors and destructors: */
	BarrierMessage(unsigned int sNodeIndex,int sMessageId,unsigned int sPipeId,unsigned int sBarrierId)
		:PipeMessage(sNodeIndex,sMessageId,sPipeId),
		 barrierId(sBarrierId)
		{
		}
	};

struct GatherMessage:public BarrierMessage
	{
	/* Elements: */
	public:
	unsigned int value; // Slave's gather value in a gather operation
	
	/* Constructors and destructors: */
	GatherMessage(unsigned int sNodeIndex,int sMessageId,unsigned int sPipeId,unsigned int sBarrierId,unsigned int sValue)
		:BarrierMessage(sNodeIndex,sMessageId,sPipeId,sBarrierId),
		 value(sValue)
		{
		}
	};

}

/****************************
Methods of class Multiplexer:
****************************/

Packet* Multiplexer::allocatePacket(void)
	{
	return new Packet;
	}

void Multiplexer::processAcknowledgment(Multiplexer::LockedPipe& pipeState,int slaveIndex,unsigned int streamPos)
	{
	/* Check if the reported stream position points into the packet queue: */
	unsigned int streamPosOffset=streamPos-pipeState->headStreamPos;
	if(streamPosOffset>0)
		{
		/* Check if the slave had not yet acknowledged the head of the packet list: */
		if(pipeState->slaveStreamPosOffsets[slaveIndex]==0)
			{
			/* Update the slave's stream position offset: */
			pipeState->slaveStreamPosOffsets[slaveIndex]=streamPosOffset;
			
			/* Reduce the number of slaves that are still pending acknowledgment for the head of the packet list: */
			--pipeState->numHeadSlaves;
			
			/* Check if the last acknowledgment for the head of the packet list has come in: */
			if(pipeState->numHeadSlaves==0)
				{
				/* Calculate the minimal stream position offset: */
				unsigned int minStreamPosOffset=pipeState->slaveStreamPosOffsets[0];
				for(unsigned int i=1;i<numSlaves;++i)
					if(minStreamPosOffset>pipeState->slaveStreamPosOffsets[i])
						minStreamPosOffset=pipeState->slaveStreamPosOffsets[i];
				
				#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER_VERBOSE
				std::cerr<<"Attempting to discard "<<minStreamPosOffset<<" bytes from beginning of packet list"<<std::endl;
				#endif
				
				/* Discard all acknowledged packets from the head of the packet list: */
				unsigned int numDiscarded=0;
				Packet* firstAcknowledged=pipeState->packetList.head;
				Packet* lastAcknowledged=0;
				for(Packet* pPtr=pipeState->packetList.head;pPtr!=0&&minStreamPosOffset>=pPtr->packetSize;lastAcknowledged=pPtr,pPtr=pPtr->succ)
					{
					--pipeState->packetList.numPackets;
					numDiscarded+=pPtr->packetSize;
					minStreamPosOffset-=pPtr->packetSize;
					}
				if(lastAcknowledged!=0)
					{
					pipeState->packetList.head=lastAcknowledged->succ;
					if(lastAcknowledged->succ==0)
						pipeState->packetList.tail=0;
					{
					Threads::Spinlock::Lock packetPoolLock(packetPoolMutex);
					lastAcknowledged->succ=packetPoolHead;
					packetPoolHead=firstAcknowledged;
					}
					}
				
				#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER_VERBOSE
				std::cerr<<"Discarded "<<numDiscarded<<" bytes from beginning of packet list"<<std::endl;
				#endif
				
				/* Update the stream position of the head of the packet list: */
				pipeState->headStreamPos+=numDiscarded;
				
				/* Update all slaves' stream position offsets: */
				for(unsigned int i=0;i<numSlaves;++i)
					{
					pipeState->slaveStreamPosOffsets[i]-=numDiscarded;
					if(pipeState->slaveStreamPosOffsets[i]==0)
						++pipeState->numHeadSlaves;
					}
				
				/* Wake up any callers that might be blocking on a full send queue: */
				pipeState->receiveCond.broadcast();
				}
			}
		else
			{
			/* Remember the slave's stream position offset: */
			pipeState->slaveStreamPosOffsets[slaveIndex]=streamPosOffset;
			}
		}
	}

void* Multiplexer::packetHandlingThreadMaster(void)
	{
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	// Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	/* Handle message exchange during multiplexer initialization: */
	bool* slaveConnecteds=new bool[numSlaves];
	for(unsigned int i=0;i<numSlaves;++i)
		slaveConnecteds[i]=false;
	unsigned int numConnectedSlaves=0;
	while(numConnectedSlaves<numSlaves)
		{
		/* Wait for a connection initialization packet: */
		ssize_t numBytesReceived=recv(socketFd,messageBuffer,Packet::maxRawPacketSize,0);
		if(numBytesReceived==sizeof(Message))
			{
			Message* msg=static_cast<Message*>(messageBuffer);
			if(msg->nodeIndex&0x80000000U) // Check if the message is from a slave
				{
				unsigned int slaveIndex=(msg->nodeIndex&0x7fffffffU)-1;
				if(msg->messageId==Message::CONNECTION&&slaveIndex<numSlaves&&!slaveConnecteds[slaveIndex])
					{
					/* Mark the slave as connected: */
					slaveConnecteds[slaveIndex]=true;
					++numConnectedSlaves;
					}
				}
			}
		}
	delete[] slaveConnecteds;
	
	/* Send connection message to slaves: */
	Message msg(0,Message::CONNECTION);
	{
	// SocketMutex::Lock socketLock(socketMutex);
	for(int i=0;i<masterMessageBurstSize;++i)
		sendto(socketFd,&msg,sizeof(Message),0,(const sockaddr*)otherAddress,sizeof(sockaddr_in));
	}
	
	/* Signal connection establishment: */
	{
	Threads::MutexCond::Lock connectionCondLock(connectionCond);
	connected=true;
	connectionCond.broadcast();
	}
	
	/* Handle messages from the slaves: */
	while(true)
		{
		/* Wait for a message from any slave: */
		ssize_t numBytesReceived=recv(socketFd,messageBuffer,Packet::maxRawPacketSize,0);
		if(numBytesReceived>0&&size_t(numBytesReceived)>=sizeof(Message))
			{
			/* Check that the message is not the echo of a server message: */
			if(static_cast<Message*>(messageBuffer)->nodeIndex&0x80000000U)
				{
				/* Remove the slave message indicator bit from the message's node index: */
				unsigned int msgNodeIndex=static_cast<Message*>(messageBuffer)->nodeIndex&0x7fffffffU;
				
				switch(static_cast<Message*>(messageBuffer)->messageId)
					{
					case Message::CONNECTION:
						{
						/* One slave must have missed the connection establishment packet; send another one: */
						Message msg(0,Message::CONNECTION);
						{
						// SocketMutex::Lock socketLock(socketMutex);
						sendto(socketFd,&msg,sizeof(Message),0,(const sockaddr*)otherAddress,sizeof(sockaddr_in));
						}
						break;
						}
					
					case Message::PING:
						{
						/* Broadcast a ping reply to all slaves: */
						Message msg(0,Message::PING);
						{
						// SocketMutex::Lock socketLock(socketMutex);
						sendto(socketFd,&msg,sizeof(Message),0,(const sockaddr*)otherAddress,sizeof(sockaddr_in));
						}
						break;
						}
					
					case Message::CREATEPIPE1:
						{
						CreatePipe1Message* msg=static_cast<CreatePipe1Message*>(messageBuffer);
						if(size_t(numBytesReceived)>=sizeof(CreatePipe1Message)&&size_t(numBytesReceived)==sizeof(CreatePipe1Message)+msg->idNumParts*sizeof(unsigned int))
							{
							/* Extract the originating thread's ID from the message: */
							Threads::Thread::ID senderId(msg->idNumParts,reinterpret_cast<unsigned int*>(msg+1));
							
							/* Find the new pipe state corresponding to the thread ID: */
							PipeState* newPipeState;
							{
							Threads::Mutex::Lock pipeStateTableLock(pipeStateTableMutex);
							NewPipeHasher::Iterator npIt=newPipes.findEntry(senderId);
							if(npIt.isFinished())
								{
								/* If the new pipe state hasn't been created already, do it here: */
								newPipeState=new PipeState(nodeIndex,numSlaves);
								
								/* Add the new pipe state to the new pipe map: */
								newPipes[senderId]=newPipeState;
								}
							else
								newPipeState=npIt->getDest();
							}
							
							/* Lock the new pipe: */
							LockedPipe pipeState(newPipeState);
							
							/* Check the pipe's barrier state for first-stage completion: */
							bool sendReply=false;
							if(pipeState->barrierId<1)
								{
								/* Remember the slave's barrier completion: */
								pipeState->slaveBarrierIds[msgNodeIndex-1]=1;
								
								/* Check if the current barrier is complete: */
								pipeState->minSlaveBarrierId=pipeState->slaveBarrierIds[0];
								for(unsigned int i=1;i<numSlaves;++i)
									if(pipeState->minSlaveBarrierId>pipeState->slaveBarrierIds[i])
										pipeState->minSlaveBarrierId=pipeState->slaveBarrierIds[i];
								if(pipeState->minSlaveBarrierId>=1)
									{
									/* Complete the first barrier: */
									pipeState->barrierId=1;
									
									/* Assign a pipe ID to the new pipe and store it in the pipe state table: */
									Threads::Mutex::Lock pipeStateTableLock(pipeStateTableMutex);
									do
										{
										++lastPipeId;
										if(lastPipeId==0x80000000U) // Ensure that pipeId never has the MSB set
											lastPipeId=1;
										}
									while(pipeStateTable.isEntry(lastPipeId));
									pipeState->pipeId=lastPipeId;
									pipeStateTable[lastPipeId]=newPipeState;
									
									/* Wake up the thread blocked on the new pipe: */
									pipeState->barrierCond.signal();
									
									/* Send a stage-one pipe creation completion message: */
									sendReply=true;
									}
								}
							else
								{
								/* One slave must have missed a stage-one pipe creation completion message; send another one: */
								sendReply=true;
								}
							
							if(sendReply)
								{
								CreatePipe1Message* msg2=static_cast<CreatePipe1Message*>(messageBuffer);
								msg2->nodeIndex=0;
								msg2->messageId=Message::CREATEPIPE1;
								msg2->pipeId=pipeState->pipeId;
								msg2->idNumParts=senderId.getNumParts();
								for(unsigned int i=0;i<msg2->idNumParts;++i)
									reinterpret_cast<unsigned int*>(msg2+1)[i]=senderId.getPart(i);
								{
								// SocketMutex::Lock socketLock(socketMutex);
								sendto(socketFd,messageBuffer,sizeof(CreatePipe1Message)+msg2->idNumParts*sizeof(unsigned int),0,(const sockaddr*)otherAddress,sizeof(sockaddr_in));
								}
								}
							}
						#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER
						else
							std::cerr<<"Node "<<nodeIndex<<": received CREATEPIPE1 message of wrong size "<<numBytesReceived<<std::endl;
						#endif
						break;
						}
					
					case Message::CREATEPIPE2:
						{
						if(numBytesReceived==sizeof(PipeMessage))
							{
							PipeMessage* msg=static_cast<PipeMessage*>(messageBuffer);
							
							/* Get a handle on the state object of the pipe the packet is meant for: */
							LockedPipe pipeState(pipeStateTable,pipeStateTableMutex,msg->pipeId);
							
							if(pipeState.isValid())
								{
								/* Check the pipe's barrier state for second-stage completion: */
								if(pipeState->barrierId<2)
									{
									/* Remember the slave's barrier completion: */
									pipeState->slaveBarrierIds[msgNodeIndex-1]=2;
									
									/* Check if the current barrier is complete: */
									pipeState->minSlaveBarrierId=pipeState->slaveBarrierIds[0];
									for(unsigned int i=1;i<numSlaves;++i)
										if(pipeState->minSlaveBarrierId>pipeState->slaveBarrierIds[i])
											pipeState->minSlaveBarrierId=pipeState->slaveBarrierIds[i];
									if(pipeState->minSlaveBarrierId>=2)
										{
										/* Complete the second barrier: */
										pipeState->barrierId=2;

										/* Wake up the thread blocked on the new pipe: */
										pipeState->barrierCond.signal();
										}
									}
								}
							#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER
							else
								std::cerr<<"Node "<<nodeIndex<<": received CREATEPIPE2 message for non-existent pipe "<<msg->pipeId<<std::endl;
							#endif
							}
						#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER
						else
							std::cerr<<"Node "<<nodeIndex<<": received CREATEPIPE2 message of wrong size "<<numBytesReceived<<std::endl;
						#endif
						break;
						}
					
					case Message::ACKNOWLEDGMENT:
						{
						if(numBytesReceived==sizeof(StreamMessage))
							{
							StreamMessage* msg=static_cast<StreamMessage*>(messageBuffer);
							
							/* Get a handle on the state object of the pipe the packet is meant for: */
							LockedPipe pipeState(pipeStateTable,pipeStateTableMutex,msg->pipeId);
							
							if(pipeState.isValid())
								{
								/* Process the acknowledgment packet: */
								processAcknowledgment(pipeState,msgNodeIndex-1,msg->streamPos);
								}
							#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER
							else
								std::cerr<<"Node "<<nodeIndex<<": received ACKNOWLEDGMENT message for non-existent pipe "<<msg->pipeId<<std::endl;
							#endif
							}
						#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER
						else
							std::cerr<<"Node "<<nodeIndex<<": received ACKNOWLEDGMENT message of wrong size "<<numBytesReceived<<std::endl;
						#endif
						break;
						}
					
					case Message::PACKETLOSS:
						{
						if(numBytesReceived==sizeof(StreamMessage))
							{
							StreamMessage* msg=static_cast<StreamMessage*>(messageBuffer);
							
							/* Get a handle on the state object of the pipe the packet is meant for: */
							LockedPipe pipeState(pipeStateTable,pipeStateTableMutex,msg->pipeId);
							
							if(pipeState.isValid())
								{
								/* Use the stream position reported by the client as positive acknowledgment: */
								processAcknowledgment(pipeState,msgNodeIndex-1,msg->streamPos);
								
								/* Resend requested packets if there are any; otherwise, do nothing because master is busy: */
								if(msg->streamPos!=pipeState->streamPos)
									{
									#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER_VERBOSE
									std::cerr<<"Packet loss of "<<msg->packetPos-msg->streamPos<<" bytes from "<<msg->streamPos<<" detected by node "<<msgNodeIndex<<", stream pos is "<<pipeState->streamPos<<", buffer starts at "<<pipeState->headStreamPos<<std::endl;
									#endif
									
									/* Find the recently-sent packet starting at the slave's current stream position: */
									Packet* packet;
									for(packet=pipeState->packetList.front();packet!=0&&packet->streamPos!=msg->streamPos;packet=packet->succ)
										;
									
									/* Signal a fatal error if the required packet has already been discarded: */
									if(packet==0)
										Misc::throwStdErr("Cluster::Multiplexer: Node %u: Fatal packet loss detected at stream position %u",msgNodeIndex,msg->streamPos);
									
									{
									/* Resend all recent packets in order: */
									// SocketMutex::Lock socketLock(socketMutex);
									for(;packet!=0;packet=packet->succ)
										{
										sendto(socketFd,&packet->pipeId,packet->packetSize+2*sizeof(unsigned int),0,(const sockaddr*)otherAddress,sizeof(sockaddr_in));
										#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER
										++pipeState->numResentPackets;
										pipeState->numResentBytes+=packet->packetSize;
										#endif
										}
									}
									}
								}
							#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER
							else
								std::cerr<<"Node "<<nodeIndex<<": received PACKETLOSS message for non-existent pipe "<<msg->pipeId<<std::endl;
							#endif
							}
						#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER
						else
							std::cerr<<"Node "<<nodeIndex<<": received PACKETLOSS message of wrong size "<<numBytesReceived<<std::endl;
						#endif
						break;
						}
					
					case Message::BARRIER:
						{
						if(numBytesReceived==sizeof(BarrierMessage))
							{
							BarrierMessage* msg=static_cast<BarrierMessage*>(messageBuffer);
							
							/* Get a handle on the state object of the pipe the packet is meant for: */
							LockedPipe pipeState(pipeStateTable,pipeStateTableMutex,msg->pipeId);
							
							if(pipeState.isValid())
								{
								/* Update the barrier ID array: */
								if(pipeState->barrierId>=msg->barrierId)
									{
									/* One slave must have missed a barrier completion message; send another one: */
									BarrierMessage msg2(0,Message::BARRIER,msg->pipeId,msg->barrierId);
									{
									// SocketMutex::Lock socketLock(socketMutex);
									sendto(socketFd,&msg2,sizeof(BarrierMessage),0,(const sockaddr*)otherAddress,sizeof(sockaddr_in));
									}
									}
								else
									{
									pipeState->slaveBarrierIds[msgNodeIndex-1]=msg->barrierId;
									
									/* Check if the current barrier is complete: */
									pipeState->minSlaveBarrierId=pipeState->slaveBarrierIds[0];
									for(unsigned int i=1;i<numSlaves;++i)
										if(pipeState->minSlaveBarrierId>pipeState->slaveBarrierIds[i])
											pipeState->minSlaveBarrierId=pipeState->slaveBarrierIds[i];
									if(pipeState->minSlaveBarrierId>pipeState->barrierId)
										{
										/* Wake up thread waiting on barrier: */
										pipeState->barrierCond.signal();
										}
									}
								}
							else
								{
								/* One slave must have missed the completion message for a pipe-closing barrier; send another one: */
								BarrierMessage msg2(0,Message::BARRIER,msg->pipeId,msg->barrierId);
								{
								// SocketMutex::Lock socketLock(socketMutex);
								sendto(socketFd,&msg2,sizeof(BarrierMessage),0,(const sockaddr*)otherAddress,sizeof(sockaddr_in));
								}
								}
							}
						#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER
						else
							std::cerr<<"Node "<<nodeIndex<<": received BARRIER message of wrong size "<<numBytesReceived<<std::endl;
						#endif
						break;
						}
					
					case Message::GATHER:
						{
						if(numBytesReceived==sizeof(GatherMessage))
							{
							GatherMessage* msg=static_cast<GatherMessage*>(messageBuffer);
							
							/* Get a handle on the state object of the pipe the packet is meant for: */
							LockedPipe pipeState(pipeStateTable,pipeStateTableMutex,msg->pipeId);
							
							if(pipeState.isValid())
								{
								/* Update the barrier ID array: */
								if(pipeState->barrierId>=msg->barrierId)
									{
									/* One slave must have missed a gather completion message; send another one: */
									GatherMessage msg2(0,Message::GATHER,msg->pipeId,msg->barrierId,pipeState->masterGatherValue);
									{
									// SocketMutex::Lock socketLock(socketMutex);
									sendto(socketFd,&msg2,sizeof(GatherMessage),0,(const sockaddr*)otherAddress,sizeof(sockaddr_in));
									}
									}
								else
									{
									pipeState->slaveBarrierIds[msgNodeIndex-1]=msg->barrierId;
									pipeState->slaveGatherValues[msgNodeIndex-1]=msg->value;
									
									/* Check if the current gather operation is complete: */
									pipeState->minSlaveBarrierId=pipeState->slaveBarrierIds[0];
									for(unsigned int i=1;i<numSlaves;++i)
										if(pipeState->minSlaveBarrierId>pipeState->slaveBarrierIds[i])
											pipeState->minSlaveBarrierId=pipeState->slaveBarrierIds[i];
									if(pipeState->minSlaveBarrierId>pipeState->barrierId)
										{
										/* Wake up thread waiting on barrier: */
										pipeState->barrierCond.signal();
										}
									}
								}
							#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER
							else
								std::cerr<<"Node "<<nodeIndex<<": received GATHER message for non-existent pipe "<<msg->pipeId<<std::endl;
							#endif
							}
						#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER
						else
							std::cerr<<"Node "<<nodeIndex<<": received GATHER message of wrong size "<<numBytesReceived<<std::endl;
						#endif
						break;
						}
					}
				}
			}
		#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER
		else
			std::cerr<<"Node "<<nodeIndex<<": received short message of size "<<numBytesReceived<<std::endl;
		#endif
		}
	
	return 0;
	}

void* Multiplexer::packetHandlingThreadSlave(void)
	{
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	// Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	/* Set the MSB on the nodeIndex to identify a slave-originating message: */
	unsigned int sendNodeIndex=nodeIndex|0x80000000U;
	
	/* Keep sending connection initiation packets to the master until connection is established: */
	while(true)
		{
		/* Send connection initiation packet to master: */
		Message msg(sendNodeIndex,Message::CONNECTION);
		{
		// SocketMutex::Lock socketLock(socketMutex);
		for(int i=0;i<slaveMessageBurstSize;++i)
			sendto(socketFd,&msg,sizeof(Message),0,(const sockaddr*)otherAddress,sizeof(struct sockaddr_in));
		}
		
		/* Wait for a connection packet from the master (but don't wait for too long): */
		fd_set readFdSet;
		FD_ZERO(&readFdSet);
		FD_SET(socketFd,&readFdSet);
		struct timeval timeout=connectionWaitTimeout;
		if(select(socketFd+1,&readFdSet,0,0,&timeout)>=0&&FD_ISSET(socketFd,&readFdSet))
			break;
		}
	
	unsigned int sendAckIn=nodeIndex-1;
	
	/* Handle messages from the master: */
	while(true)
		{
		/* Wait for the next packet, and request a ping packet if no data arrives during the timeout: */
		bool havePacket=false;
		for(int i=0;i<maxPingRequests&&!havePacket;++i)
			{
			/* Wait until the "silence period" is over: */
			fd_set readFdSet;
			FD_ZERO(&readFdSet);
			FD_SET(socketFd,&readFdSet);
			struct timeval timeout=pingTimeout;
			if(select(socketFd+1,&readFdSet,0,0,&timeout)>=0&&FD_ISSET(socketFd,&readFdSet))
				havePacket=true;
			else
				{
				/* Send a ping request packet: */
				Message msg(sendNodeIndex,Message::PING);
				{
				// SocketMutex::Lock socketLock(socketMutex);
				for(int i=0;i<slaveMessageBurstSize;++i)
					sendto(socketFd,&msg,sizeof(Message),0,(const sockaddr*)otherAddress,sizeof(struct sockaddr_in));
				}
				}
			}
		if(!havePacket)
			{
			/* Signal an error: */
			Misc::throwStdErr("Cluster::Multiplexer: Node %u: Communication error",nodeIndex);
			}
		
		/* Read the waiting packet: */
		ssize_t numBytesReceived=recv(socketFd,&slaveThreadPacket->pipeId,Packet::maxRawPacketSize,0);
		if(numBytesReceived<0)
			{
			/* Try to recover from this error: */
			#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER
			std::cerr<<"Node "<<nodeIndex<<": Error "<<errno<<" on receive, slaveThreadPacket="<<slaveThreadPacket<<std::endl;
			#endif
			delete slaveThreadPacket;
			slaveThreadPacket=newPacket();
			}
		else if(size_t(numBytesReceived)>=2*sizeof(unsigned int))
			{
			slaveThreadPacket->packetSize=size_t(numBytesReceived-2*sizeof(unsigned int));
			
			if(slaveThreadPacket->pipeId==0)
				{
				/* It's a message for the pipe multiplexer itself: */
				void* messageBuffer=&slaveThreadPacket->pipeId;
				switch(static_cast<Message*>(messageBuffer)->messageId)
					{
					case Message::CONNECTION:
						/* Signal connection establishment: */
						{
						Threads::MutexCond::Lock connectionCondLock(connectionCond);
						if(!connected)
							{
							connected=true;
							connectionCond.broadcast();
							}
						}
						break;
					
					case Message::PING:
						/* Just ignore the packet... */
						break;
					
					case Message::CREATEPIPE1:
						{
						CreatePipe1Message* msg=static_cast<CreatePipe1Message*>(messageBuffer);
						if(size_t(numBytesReceived)>=sizeof(CreatePipe1Message)&&size_t(numBytesReceived)==sizeof(CreatePipe1Message)+msg->idNumParts*sizeof(unsigned int))
							{
							{
							Threads::Mutex::Lock pipeStateTableLock(pipeStateTableMutex);
							
							/* Check if the pipe is not yet in the pipe state table: */
							if(!pipeStateTable.isEntry(msg->pipeId))
								{
								/* Extract the originating thread's ID from the message: */
								Threads::Thread::ID senderId(msg->idNumParts,reinterpret_cast<unsigned int*>(msg+1));
								
								/* Find the new pipe state corresponding to the thread ID: */
								NewPipeHasher::Iterator npIt=newPipes.findEntry(senderId);
								PipeState* newPipeState=npIt->getDest();
								
								/* Remove the new pipe state from the new pipe map and insert it into the pipe state table: */
								newPipes.removeEntry(npIt);
								pipeStateTable[msg->pipeId]=newPipeState;
								
								/* Signal pipe creation completion: */
								{
								Threads::Mutex::Lock pipeStateLock(newPipeState->stateMutex);
								newPipeState->pipeId=msg->pipeId;
								newPipeState->barrierId=2;
								newPipeState->barrierCond.signal();
								}
								}
							}
							
							/* Send a stage-two pipe creation message to the master: */
							PipeMessage msg2(sendNodeIndex,Message::CREATEPIPE2,msg->pipeId);
							{
							// SocketMutex::Lock socketLock(socketMutex);
							for(int i=0;i<slaveMessageBurstSize;++i)
								sendto(socketFd,&msg2,sizeof(PipeMessage),0,(const sockaddr*)otherAddress,sizeof(struct sockaddr_in));
							}
							}
						#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER
						else
							std::cerr<<"Node "<<nodeIndex<<": received CREATEPIPE1 message of wrong size "<<numBytesReceived<<std::endl;
						#endif
						break;
						}
					
					case Message::BARRIER:
						{
						if(numBytesReceived==sizeof(BarrierMessage))
							{
							BarrierMessage* msg=static_cast<BarrierMessage*>(messageBuffer);
							
							/* Get a handle on the state object of the pipe the packet is meant for: */
							LockedPipe pipeState(pipeStateTable,pipeStateTableMutex,msg->pipeId);
							
							if(pipeState.isValid())
								{
								/* Signal barrier completion if the completion message is for the current barrier: */
								if(pipeState->barrierId<msg->barrierId)
									{
									pipeState->barrierId=msg->barrierId;
									pipeState->barrierCond.signal();
									}
								}
							#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER
							else
								std::cerr<<"Node "<<nodeIndex<<": received BARRIER message for non-existent pipe "<<msg->pipeId<<std::endl;
							#endif
							}
						#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER
						else
							std::cerr<<"Node "<<nodeIndex<<": received BARRIER message of wrong size "<<numBytesReceived<<std::endl;
						#endif
						break;
						}
					
					case Message::GATHER:
						{
						if(numBytesReceived==sizeof(GatherMessage))
							{
							GatherMessage* msg=static_cast<GatherMessage*>(messageBuffer);
							
							/* Get a handle on the state object of the pipe the packet is meant for: */
							LockedPipe pipeState(pipeStateTable,pipeStateTableMutex,msg->pipeId);
							
							if(pipeState.isValid())
								{
								/* Signal barrier completion if the completion message is for the current barrier: */
								if(pipeState->barrierId<msg->barrierId)
									{
									pipeState->barrierId=msg->barrierId;
									pipeState->masterGatherValue=msg->value;
									pipeState->barrierCond.signal();
									}
								}
							#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER
							else
								std::cerr<<"Node "<<nodeIndex<<": received GATHER message for non-existent pipe "<<msg->pipeId<<std::endl;
							#endif
							}
						#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER
						else
							std::cerr<<"Node "<<nodeIndex<<": received GATHER message of wrong size "<<numBytesReceived<<std::endl;
						#endif
						break;
						}
					}
				}
			else
				{
				/* Get a handle on the state object of the pipe the packet is meant for: */
				LockedPipe pipeState(pipeStateTable,pipeStateTableMutex,slaveThreadPacket->pipeId);
				
				if(pipeState.isValid())
					{
					/* Check if the received packet is the next expected one: */
					if(pipeState->streamPos==slaveThreadPacket->streamPos)
						{
						/* Disable packet loss mode: */
						pipeState->packetLossMode=false;
						
						++sendAckIn;
						if(sendAckIn==numSlaves)
							{
							/* Send positive acknowledgment to the master: */
							StreamMessage msg(sendNodeIndex,Message::ACKNOWLEDGMENT,slaveThreadPacket->pipeId,pipeState->streamPos,slaveThreadPacket->streamPos);
							{
							// SocketMutex::Lock socketLock(socketMutex);
							sendto(socketFd,&msg,sizeof(StreamMessage),0,(const sockaddr*)otherAddress,sizeof(struct sockaddr_in));
							}
							sendAckIn=0;
							}
						
						/* Wake up sleeping receivers if the delivery queue is currently empty: */
						if(pipeState->packetList.empty())
							pipeState->receiveCond.signal();
						
						/* Append the packet to the pipe state's delivery queue: */
						pipeState->streamPos+=slaveThreadPacket->packetSize;
						pipeState->packetList.push_back(slaveThreadPacket);
						
						/* Get a new packet: */
						slaveThreadPacket=newPacket();
						}
					else
						{
						/* Check if there is data missing between the packet's stream position and the pipe's stream position; watch for stream position wrap-around: */
						if(!pipeState->packetLossMode&&slaveThreadPacket->streamPos-pipeState->streamPos<=0x80000000U)
							{
							/* At least one packet must have been lost; send negative acknowledgment to the master: */
							StreamMessage msg(sendNodeIndex,Message::PACKETLOSS,slaveThreadPacket->pipeId,pipeState->streamPos,slaveThreadPacket->streamPos);
							{
							// SocketMutex::Lock socketLock(socketMutex);
							for(int i=0;i<slaveMessageBurstSize;++i)
								sendto(socketFd,&msg,sizeof(StreamMessage),0,(const sockaddr*)otherAddress,sizeof(struct sockaddr_in));
							}

							/* Enable packet loss mode to prohibit sending further loss messages until the missing packet arrives: */
							pipeState->packetLossMode=true;
							}
						}
					}
				#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER
				else
					std::cerr<<"Node "<<nodeIndex<<": received stream packet for non-existent pipe "<<slaveThreadPacket->pipeId<<std::endl;
				#endif
				}
			}
		#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER
		else
			std::cerr<<"Node "<<nodeIndex<<": received short message of size "<<numBytesReceived<<std::endl;
		#endif
		}
	
	return 0;
	}

Multiplexer::Multiplexer(unsigned int sNumSlaves,unsigned int sNodeIndex,std::string masterHostName,int masterPortNumber,std::string slaveMulticastGroup,int slavePortNumber)
	:numSlaves(sNumSlaves),nodeIndex(sNodeIndex),
	 masterAddress(new sockaddr_in),
	 otherAddress(new sockaddr_in),
	 socketFd(0),
	 connected(false),
	 newPipes(17),
	 lastPipeId(0),
	 pipeStateTable(17),
	 messageBuffer(0),
	 slaveThreadPacket(0),
	 masterMessageBurstSize(1),slaveMessageBurstSize(1),
	 connectionWaitTimeout(0.5),
	 pingTimeout(10.0),maxPingRequests(3),
	 receiveWaitTimeout(0.25),
	 barrierWaitTimeout(0.1),
	 sendBufferSize(20),
	 packetPoolHead(0)
	{
	/* Lookup master's IP address: */
	struct hostent* masterEntry=gethostbyname(masterHostName.c_str());
	if(masterEntry==0)
		{
		close(socketFd);
		Misc::throwStdErr("Cluster::Multiplexer: Node %u: Unable to resolve master %s",nodeIndex,masterHostName.c_str());
		}
	struct in_addr masterNetAddress;
	masterNetAddress.s_addr=ntohl(((struct in_addr*)masterEntry->h_addr_list[0])->s_addr);
	
	/* Store the master's address: */
	memset(masterAddress,0,sizeof(sockaddr_in));
	masterAddress->sin_family=AF_INET;
	masterAddress->sin_port=htons(masterPortNumber);
	masterAddress->sin_addr.s_addr=htonl(masterNetAddress.s_addr);
	
	/* Lookup slave multicast group's IP address: */
	struct hostent* slaveEntry=gethostbyname(slaveMulticastGroup.c_str());
	if(slaveEntry==0)
		{
		close(socketFd);
		Misc::throwStdErr("Cluster::Multiplexer: Node %u: Unable to resolve slave multicast group %s",nodeIndex,slaveMulticastGroup.c_str());
		}
	struct in_addr slaveNetAddress;
	slaveNetAddress.s_addr=ntohl(((struct in_addr*)slaveEntry->h_addr_list[0])->s_addr);
	
	/* Create a UDP socket: */
	socketFd=socket(PF_INET,SOCK_DGRAM,0);
	if(socketFd<0)
		Misc::throwStdErr("Cluster::Multiplexer: Node %u: Unable to create socket",nodeIndex);
	
	/* Bind the socket to the local address/port number: */
	int localPortNumber=nodeIndex==0?masterPortNumber:slavePortNumber;
	struct sockaddr_in socketAddress;
	socketAddress.sin_family=AF_INET;
	socketAddress.sin_port=htons(localPortNumber);
	socketAddress.sin_addr.s_addr=htonl(INADDR_ANY);
	if(bind(socketFd,(struct sockaddr*)&socketAddress,sizeof(struct sockaddr_in))==-1)
		{
		close(socketFd);
		Misc::throwStdErr("Cluster::Multiplexer: Node %u: Unable to bind socket to port number %d",nodeIndex,localPortNumber);
		}
	
	if(!isMulticast(slaveNetAddress))
		{
		/* Enable broadcast handling for the socket: */
		int broadcastFlag=1;
		setsockopt(socketFd,SOL_SOCKET,SO_BROADCAST,&broadcastFlag,sizeof(int));
		}
	
	/* Connect the socket to the other end: */
	if(nodeIndex==0)
		{
		if(isMulticast(slaveNetAddress))
			{
			/* Set the outgoing network interface for the slave multicast group: */
			struct in_addr multicastInterfaceAddress;
			multicastInterfaceAddress.s_addr=htonl(masterNetAddress.s_addr);
			if(setsockopt(socketFd,IPPROTO_IP,IP_MULTICAST_IF,&multicastInterfaceAddress,sizeof(struct in_addr))<0)
				{
				int myerrno=errno;
				close(socketFd);
				Misc::throwStdErr("Cluster::Multiplexer: Node %u: error %s during setsockopt",nodeIndex,strerror(myerrno));
				}
			}
		
		/* Store the slaves' address: */
		memset(otherAddress,0,sizeof(sockaddr_in));
		otherAddress->sin_family=AF_INET;
		otherAddress->sin_port=htons(slavePortNumber);
		otherAddress->sin_addr.s_addr=htonl(slaveNetAddress.s_addr);
		}
	else
		{
		if(isMulticast(slaveNetAddress))
			{
			/* Join the slave multicast group: */
			struct ip_mreq addGroupRequest;
			addGroupRequest.imr_multiaddr.s_addr=htonl(slaveNetAddress.s_addr);
			addGroupRequest.imr_interface.s_addr=htonl(INADDR_ANY);
			if(setsockopt(socketFd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&addGroupRequest,sizeof(struct ip_mreq))<0)
				{
				int myerrno=errno;
				close(socketFd);
				Misc::throwStdErr("Cluster::Multiplexer: Node %u: error %s during setsockopt",nodeIndex,strerror(myerrno));
				}
			}
		
		/* Store the master's address: */
		memset(otherAddress,0,sizeof(sockaddr_in));
		otherAddress->sin_family=AF_INET;
		otherAddress->sin_port=htons(masterPortNumber);
		otherAddress->sin_addr.s_addr=htonl(masterNetAddress.s_addr);
		}
	
	/* Create the packet handling thread: */
	if(nodeIndex==0)
		{
		messageBuffer=new unsigned char[Packet::maxRawPacketSize];
		packetHandlingThread.start(this,&Multiplexer::packetHandlingThreadMaster);
		}
	else
		{
		slaveThreadPacket=newPacket();
		packetHandlingThread.start(this,&Multiplexer::packetHandlingThreadSlave);
		}
	}

Multiplexer::~Multiplexer(void)
	{
	/* Stop the packet handling thread: */
	packetHandlingThread.cancel();
	packetHandlingThread.join();
	
	/* Delete the packet handling thread's receive packet: */
	delete slaveThreadPacket;
	delete[] static_cast<unsigned char*>(messageBuffer);
	
	/* Close all leftover pipes: */
	for(PipeHasher::Iterator psIt=pipeStateTable.begin();psIt!=pipeStateTable.end();++psIt)
		delete psIt->getDest();
	
	/* Close the UDP socket: */
	close(socketFd);
	
	/* Delete address of multicast connection's other end: */
	delete masterAddress;
	delete otherAddress;
	
	/* Delete all multicast packets in the packet pool: */
	while(packetPoolHead!=0)
		{
		Packet* succ=packetPoolHead->succ;
		delete packetPoolHead;
		packetPoolHead=succ;
		}
	}

int Multiplexer::getLocalPortNumber(void) const
	{
	/* Query the communication socket's bound address: */
	struct sockaddr_in socketAddress;
	#ifdef __SGI_IRIX__
	int socketAddressLen=sizeof(struct sockaddr_in);
	#else
	socklen_t socketAddressLen=sizeof(struct sockaddr_in);
	#endif
	getsockname(socketFd,(struct sockaddr*)&socketAddress,&socketAddressLen);
	
	/* Return the socket's port number: */
	return ntohs(socketAddress.sin_port);
	}

void Multiplexer::setConnectionWaitTimeout(Misc::Time newConnectionWaitTimeout)
	{
	connectionWaitTimeout=newConnectionWaitTimeout;
	}

void Multiplexer::setPingTimeout(Misc::Time newPingTimeout,int newMaxPingRequests)
	{
	pingTimeout=newPingTimeout;
	maxPingRequests=newMaxPingRequests;
	if(maxPingRequests<2) // Need at least two
		maxPingRequests=2;
	}

void Multiplexer::setReceiveWaitTimeout(Misc::Time newReceiveWaitTimeout)
	{
	receiveWaitTimeout=newReceiveWaitTimeout;
	}

void Multiplexer::setBarrierWaitTimeout(Misc::Time newBarrierWaitTimeout)
	{
	barrierWaitTimeout=newBarrierWaitTimeout;
	}

void Multiplexer::setSendBufferSize(unsigned int newSendBufferSize)
	{
	sendBufferSize=newSendBufferSize;
	}

void Multiplexer::waitForConnection(void)
	{
	{
	Threads::MutexCond::Lock connectionCondLock(connectionCond);
	while(!connected)
		{
		/* Sleep until connection is established: */
		connectionCond.wait(connectionCondLock);
		}
	}
	}

unsigned int Multiplexer::openPipe(void)
	{
	/* Get the current thread's global ID: */
	const Threads::Thread::ID& threadId=Threads::Thread::getThreadObject()->getId();
	
	/* Check if the configured multicast packet size can handle the current thread's ID: */
	if(sizeof(CreatePipe1Message)+threadId.getNumParts()*sizeof(unsigned int)>Packet::maxRawPacketSize)
		Misc::throwStdErr("Cluster::Multiplexer: Threads nested too deply to open new multicast pipe");
	
	/* Add a new pipe state to the new pipe map: */
	PipeState* newPipeState;
	{
	Threads::Mutex::Lock pipeStateTableLock(pipeStateTableMutex);
	NewPipeHasher::Iterator npIt=newPipes.findEntry(threadId);
	if(npIt.isFinished())
		{
		/* If the new pipe state hasn't been created already, do it here: */
		newPipeState=new PipeState(nodeIndex,numSlaves);
		
		/* Add the new pipe state to the new pipe map: */
		newPipes[threadId]=newPipeState;
		}
	else
		newPipeState=npIt->getDest();
	}
	
	/* Execute the pipe creation protocol: */
	Threads::Mutex::Lock pipeStateLock(newPipeState->stateMutex);
	if(nodeIndex==0)
		{
		#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER
		std::cerr<<"Opening pipe for thread ";
		if(threadId.getNumParts()==0)
			std::cerr<<"root";
		else
			{
			std::cerr<<threadId.getPart(0);
			for(unsigned int i=1;i<threadId.getNumParts();++i)
				std::cerr<<'.'<<threadId.getPart(i);
			}
		std::cerr<<std::flush;
		#endif
		
		/* Wait for the first pipe creation stage to complete: */
		while(newPipeState->barrierId<1)
			{
			/* Wait until the next barrier message: */
			newPipeState->barrierCond.wait(newPipeState->stateMutex);
			}
		
		#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER
		std::cerr<<", pipe ID "<<newPipeState->pipeId<<"..."<<std::flush;
		#endif
		
		/* Wait for the second pipe creation stage to complete: */
		Misc::Time waitTimeout=Misc::Time::now();
		while(true)
			{
			/* Wait until the next barrier message or timeout: */
			waitTimeout+=barrierWaitTimeout;
			newPipeState->barrierCond.timedWait(newPipeState->stateMutex,waitTimeout);
			if(newPipeState->barrierId>=2)
				break;
			
			/* Send another pipe creation message in case all previous ones got lost: */
			size_t msgSize=sizeof(CreatePipe1Message)+threadId.getNumParts()*sizeof(unsigned int);
			unsigned char* msgBuffer=new unsigned char[msgSize];
			CreatePipe1Message* msg=reinterpret_cast<CreatePipe1Message*>(msgBuffer);
			msg->nodeIndex=0;
			msg->messageId=Message::CREATEPIPE1;
			msg->pipeId=newPipeState->pipeId;
			msg->idNumParts=threadId.getNumParts();
			for(unsigned int i=0;i<threadId.getNumParts();++i)
				reinterpret_cast<unsigned int*>(msg+1)[i]=threadId.getPart(i);
			{
			// SocketMutex::Lock socketLock(socketMutex);
			for(int i=0;i<masterMessageBurstSize;++i)
				sendto(socketFd,msg,msgSize,0,(const sockaddr*)otherAddress,sizeof(struct sockaddr_in));
			}
			delete[] msgBuffer;
			}
		
		/* Remove the just-opened pipe from the new pipe map: */
		{
		Threads::Mutex::Lock pipeStateTableLock(pipeStateTableMutex);
		NewPipeHasher::Iterator npIt=newPipes.findEntry(threadId);
		newPipes.removeEntry(npIt);
		}
		
		#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER
		std::cerr<<" done"<<std::endl;
		#endif
		}
	else
		{
		/* Send pipe creation messages to master until pipe creation completion message is received: */
		size_t msgSize=sizeof(CreatePipe1Message)+threadId.getNumParts()*sizeof(unsigned int);
		unsigned char* msgBuffer=new unsigned char[msgSize];
		CreatePipe1Message* msg=reinterpret_cast<CreatePipe1Message*>(msgBuffer);
		msg->nodeIndex=nodeIndex|0x80000000U;
		msg->messageId=Message::CREATEPIPE1;
		msg->pipeId=0;
		msg->idNumParts=threadId.getNumParts();
		for(unsigned int i=0;i<threadId.getNumParts();++i)
			reinterpret_cast<unsigned int*>(msg+1)[i]=threadId.getPart(i);
		
		Misc::Time waitTimeout=Misc::Time::now();
		while(newPipeState->barrierId<2)
			{
			/* Send pipe creation message to master: */
			{
			// SocketMutex::Lock socketLock(socketMutex);
			for(int i=0;i<slaveMessageBurstSize;++i)
				sendto(socketFd,msg,msgSize,0,(const sockaddr*)otherAddress,sizeof(struct sockaddr_in));
			}
			
			/* Wait for arrival of pipe creation completion message: */
			waitTimeout+=barrierWaitTimeout;
			newPipeState->barrierCond.timedWait(newPipeState->stateMutex,waitTimeout);
			}
		delete[] msgBuffer;
		}
	
	/* Return new pipe's ID: */
	return newPipeState->pipeId;
	}

void Multiplexer::closePipe(unsigned int pipeId)
	{
	/* Execute a barrier to synchronize and flush the pipe before closing it: */
	barrier(pipeId);
	
	/* Remove the pipe's state from the state table: */
	PipeState* pipeState;
	{
	Threads::Mutex::Lock pipeStateTableLock(pipeStateTableMutex);
	PipeHasher::Iterator psIt=pipeStateTable.findEntry(pipeId);
	if(psIt.isFinished())
		Misc::throwStdErr("Cluster::Multiplexer: Node %u: Attempt to close already-closed pipe",nodeIndex);
	pipeState=psIt->getDest();
	pipeStateTable.removeEntry(psIt);
	}
	
	#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER
	if(nodeIndex==0)
		{
		std::cerr<<"Closing pipe "<<pipeId;
		std::cerr<<". Re-sent "<<pipeState->numResentPackets<<" packets, "<<pipeState->numResentBytes<<" bytes"<<std::endl;
		}
	#endif
	
	/* Add all packets in the list to the list of free packets: */
	{
	Threads::Mutex::Lock pipeStateLock(pipeState->stateMutex);
	if(pipeState->packetList.numPackets>0)
		{
		{
		Threads::Spinlock::Lock packetPoolLock(packetPoolMutex);
		pipeState->packetList.tail->succ=packetPoolHead;
		packetPoolHead=pipeState->packetList.head;
		}
		pipeState->packetList.numPackets=0;
		pipeState->packetList.head=0;
		pipeState->packetList.tail=0;
		}
	}
	
	/* Destroy the pipe state: */
	delete pipeState;
	}

void Multiplexer::sendPacket(unsigned int pipeId,Packet* packet)
	{
	/* Get a handle on the state object for the given pipe: */
	LockedPipe pipeState(pipeStateTable,pipeStateTableMutex,pipeId);
	if(!pipeState.isValid())
		Misc::throwStdErr("Cluster::Multiplexer: Node %u: Attempt to write to closed pipe",nodeIndex);
	
	/* Block if the pipe's send queue is full: */
	#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER_VERBOSE
	bool amBlocking=pipeState->packetList.size()==sendBufferSize;
	if(amBlocking)
		std::cerr<<"Pipe "<<pipeId<<": Blocking on full send buffer"<<std::endl;
	#endif
	while(pipeState->packetList.size()==sendBufferSize)
		pipeState->receiveCond.wait(pipeState->stateMutex);
	
	#if CLUSTER_CONFIG_DEBUG_MULTIPLEXER_VERBOSE
	if(amBlocking)
		std::cerr<<"Pipe "<<pipeId<<": Woke up after blocking on full send buffer"<<std::endl;
	#endif
	
	/* Append the packet to the pipe's "recently sent" list: */
	packet->pipeId=pipeId;
	packet->streamPos=pipeState->streamPos;
	pipeState->streamPos+=packet->packetSize;
	pipeState->packetList.push_back(packet);
	
	/* It's safe to unlock the pipe state now: */
	pipeState.unlock();
	
	/* Send the packet across the UDP connection: */
	{
	// SocketMutex::Lock socketLock(socketMutex);
	sendto(socketFd,&packet->pipeId,packet->packetSize+2*sizeof(unsigned int),0,(const sockaddr*)otherAddress,sizeof(sockaddr_in));
	}
	}

Packet* Multiplexer::receivePacket(unsigned int pipeId)
	{
	/* Get a handle on the state object for the given pipe: */
	LockedPipe pipeState(pipeStateTable,pipeStateTableMutex,pipeId);
	if(!pipeState.isValid())
		Misc::throwStdErr("Cluster::Multiplexer: Node %u: Attempt to read from closed pipe",nodeIndex);
	
	/* Wait until there is a packet in the delivery queue: */
	if(pipeState->packetList.empty())
		{
		Misc::Time waitTimeout=Misc::Time::now();
		while(true)
			{
			/* Wait for arrival of the next packet: */
			waitTimeout+=receiveWaitTimeout;
			pipeState->receiveCond.timedWait(pipeState->stateMutex,waitTimeout);
			if(!pipeState->packetList.empty())
				break;
			
			/* Send a packet loss message to the master, just to be sure: */
			StreamMessage msg(nodeIndex|0x80000000U,Message::PACKETLOSS,pipeId,pipeState->streamPos,pipeState->streamPos);
			{
			// SocketMutex::Lock socketLock(socketMutex);
			for(int i=0;i<slaveMessageBurstSize;++i)
				sendto(socketFd,&msg,sizeof(StreamMessage),0,(const sockaddr*)otherAddress,sizeof(struct sockaddr_in));
			}
			}
		}
	
	/* Remove and return the first packet from the queue: */
	return pipeState->packetList.pop_front();
	}

void Multiplexer::barrier(unsigned int pipeId)
	{
	/* Get a handle on the state object for the given pipe: */
	LockedPipe pipeState(pipeStateTable,pipeStateTableMutex,pipeId);
	if(!pipeState.isValid())
		Misc::throwStdErr("Cluster::Multiplexer: Node %u: Attempt to synchronize closed pipe",nodeIndex);
		
	/* Bump up barrier ID: */
	unsigned int nextBarrierId=pipeState->barrierId+1;
	
	if(nodeIndex==0)
		{
		/* Wait until barrier messages from all slaves have been received: */
		while(pipeState->minSlaveBarrierId<nextBarrierId)
			{
			/* Wait until the next barrier message: */
			pipeState->barrierCond.wait(pipeState->stateMutex);
			}
		
		/* Mark the barrier as completed: */
		pipeState->barrierId=pipeState->minSlaveBarrierId;
		
		/* Send barrier completion message to all slaves: */
		BarrierMessage msg(0,Message::BARRIER,pipeId,nextBarrierId);
		{
		// SocketMutex::Lock socketLock(socketMutex);
		sendto(socketFd,&msg,sizeof(BarrierMessage),0,(const sockaddr*)otherAddress,sizeof(sockaddr_in));
		}
		
		/* Reset the pipe's flow control state: */
		pipeState->headStreamPos=pipeState->streamPos;
		for(unsigned int i=0;i<numSlaves;++i)
			pipeState->slaveStreamPosOffsets[i]=0;
		pipeState->numHeadSlaves=numSlaves;
		
		/* Add all packets in the list to the list of free packets: */
		if(pipeState->packetList.numPackets>0)
			{
			{
			Threads::Spinlock::Lock packetPoolLock(packetPoolMutex);
			pipeState->packetList.tail->succ=packetPoolHead;
			packetPoolHead=pipeState->packetList.head;
			}
			pipeState->packetList.numPackets=0;
			pipeState->packetList.head=0;
			pipeState->packetList.tail=0;
			}
		}
	else
		{
		/* Send barrier messages to master until barrier completion message is received: */
		Misc::Time waitTimeout=Misc::Time::now();
		while(pipeState->barrierId<nextBarrierId)
			{
			/* Send barrier message to master: */
			BarrierMessage msg(nodeIndex|0x80000000U,Message::BARRIER,pipeId,nextBarrierId);
			{
			// SocketMutex::Lock socketLock(socketMutex);
			sendto(socketFd,&msg,sizeof(BarrierMessage),0,(const sockaddr*)otherAddress,sizeof(struct sockaddr_in));
			}
			
			/* Wait for arrival of barrier completion message: */
			waitTimeout+=barrierWaitTimeout;
			pipeState->barrierCond.timedWait(pipeState->stateMutex,waitTimeout);
			}
		}
	}

unsigned int Multiplexer::gather(unsigned int pipeId,unsigned int value,GatherOperation::OpCode op)
	{
	/* Get a handle on the state object for the given pipe: */
	LockedPipe pipeState(pipeStateTable,pipeStateTableMutex,pipeId);
	if(!pipeState.isValid())
		Misc::throwStdErr("Cluster::Multiplexer: Node %u: Attempt to gather on closed pipe",nodeIndex);
	
	/* Bump up barrier ID: */
	unsigned int nextBarrierId=pipeState->barrierId+1;
	
	if(nodeIndex==0)
		{
		/* Wait until gather messages from all slaves have been received: */
		while(pipeState->minSlaveBarrierId<nextBarrierId)
			{
			/* Wait until the next barrier message: */
			pipeState->barrierCond.wait(pipeState->stateMutex);
			}
		
		/* Mark the gathering operation as completed: */
		pipeState->barrierId=nextBarrierId;
		
		/* Calculate the final gather value: */
		pipeState->masterGatherValue=value;
		switch(op)
			{
			case GatherOperation::AND:
				for(unsigned int i=0;i<numSlaves;++i)
					pipeState->masterGatherValue=pipeState->masterGatherValue&&pipeState->slaveGatherValues[i];
				break;
			
			case GatherOperation::OR:
				for(unsigned int i=0;i<numSlaves;++i)
					pipeState->masterGatherValue=pipeState->masterGatherValue||pipeState->slaveGatherValues[i];
				break;
			
			case GatherOperation::MIN:
				for(unsigned int i=0;i<numSlaves;++i)
					if(pipeState->masterGatherValue>pipeState->slaveGatherValues[i])
						pipeState->masterGatherValue=pipeState->slaveGatherValues[i];
				break;
			
			case GatherOperation::MAX:
				for(unsigned int i=0;i<numSlaves;++i)
					if(pipeState->masterGatherValue<pipeState->slaveGatherValues[i])
						pipeState->masterGatherValue=pipeState->slaveGatherValues[i];
				break;
			
			case GatherOperation::SUM:
				for(unsigned int i=0;i<numSlaves;++i)
					pipeState->masterGatherValue+=pipeState->slaveGatherValues[i];
				break;
			
			case GatherOperation::PRODUCT:
				for(unsigned int i=0;i<numSlaves;++i)
					pipeState->masterGatherValue*=pipeState->slaveGatherValues[i];
				break;
			}
		
		/* Send gather completion message to all slaves: */
		GatherMessage msg(0,Message::GATHER,pipeId,nextBarrierId,pipeState->masterGatherValue);
		{
		// SocketMutex::Lock socketLock(socketMutex);
		sendto(socketFd,&msg,sizeof(GatherMessage),0,(const sockaddr*)otherAddress,sizeof(sockaddr_in));
		}
		
		/* Reset the pipe's flow control state: */
		pipeState->headStreamPos=pipeState->streamPos;
		for(unsigned int i=0;i<numSlaves;++i)
			pipeState->slaveStreamPosOffsets[i]=0;
		pipeState->numHeadSlaves=numSlaves;
		
		/* Add all packets in the list to the list of free packets: */
		if(pipeState->packetList.numPackets>0)
			{
			{
			Threads::Spinlock::Lock packetPoolLock(packetPoolMutex);
			pipeState->packetList.tail->succ=packetPoolHead;
			packetPoolHead=pipeState->packetList.head;
			}
			pipeState->packetList.numPackets=0;
			pipeState->packetList.head=0;
			pipeState->packetList.tail=0;
			}
		}
	else
		{
		/* Send barrier messages to master until barrier completion message is received: */
		Misc::Time waitTimeout=Misc::Time::now();
		while(pipeState->barrierId<nextBarrierId)
			{
			/* Send gather message to master: */
			GatherMessage msg(nodeIndex|0x80000000U,Message::GATHER,pipeId,nextBarrierId,value);
			{
			// SocketMutex::Lock socketLock(socketMutex);
			sendto(socketFd,&msg,sizeof(GatherMessage),0,(const sockaddr*)otherAddress,sizeof(struct sockaddr_in));
			}
			
			/* Wait for arrival of barrier completion message: */
			waitTimeout+=barrierWaitTimeout;
			pipeState->barrierCond.timedWait(pipeState->stateMutex,waitTimeout);
			}
		}
	
	/* Return the master gather value: */
	return pipeState->masterGatherValue;
	}

}
