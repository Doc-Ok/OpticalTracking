/***********************************************************************
VRDeviceClient - Class encapsulating the VR device protocol's client
side.
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

#include <Vrui/Internal/VRDeviceClient.h>

#include <Misc/Time.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Realtime/Time.h>
#include <Vrui/Internal/VRDeviceDescriptor.h>

namespace Vrui {

namespace {

/****************
Helper functions:
****************/

void setTrackerStateTimeStamps(VRDeviceState& state) // Sets tracker state time stamps to current monotonic time
	{
	/* Get the current monotonic time: */
	Realtime::TimePointMonotonic now;
	
	/* Get the lower-order bits of the microsecond time: */
	VRDeviceState::TimeStamp ts=VRDeviceState::TimeStamp(now.tv_sec*1000000+(now.tv_nsec+500)/1000);
	
	/* Set all tracker state time stamps to the curren time: */
	for(int i=0;i<state.getNumTrackers();++i)
		state.setTrackerTimeStamp(i,ts);
	}

}

/*******************************
Methods of class VRDeviceClient:
*******************************/

void* VRDeviceClient::streamReceiveThreadMethod(void)
	{
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	
	while(true)
		{
		/* Wait for next packet reply message: */
		try
			{
			VRDevicePipe::MessageIdType message=pipe.readMessage();
			if(message==VRDevicePipe::PACKET_REPLY)
				{
				/* Read server's state: */
				{
				Threads::Mutex::Lock stateLock(stateMutex);
				state.read(pipe,serverHasTimeStamps);
				if(!serverHasTimeStamps)
					setTrackerStateTimeStamps(state);
				}
				
				/* Signal packet reception: */
				packetSignalCond.broadcast();
				
				/* Invoke packet notification callback: */
				if(packetNotificationCallback!=0)
					(*packetNotificationCallback)(this);
				}
			else if(message==VRDevicePipe::STOPSTREAM_REPLY)
				break;
			else
				{
				/* Signal a protocol error and shut down: */
				if(errorCallback!=0)
					(*errorCallback)(ProtocolError("VRDeviceClient: Mismatching message while waiting for PACKET_REPLY",this));
				connectionDead=true;
				packetSignalCond.broadcast();
				break;
				}
			}
		catch(std::runtime_error err)
			{
			/* Signal an error and shut down: */
			if(errorCallback!=0)
				{
				std::string msg="VRDeviceClient: Caught exception ";
				msg.append(err.what());
				(*errorCallback)(ProtocolError(msg,this));
				}
			connectionDead=true;
			packetSignalCond.broadcast();
			break;
			}
		}
	
	return 0;
	}

void VRDeviceClient::initClient(void)
	{
	/* Initiate connection: */
	pipe.writeMessage(VRDevicePipe::CONNECT_REQUEST);
	pipe.write<unsigned int>(VRDevicePipe::protocolVersionNumber);
	pipe.flush();
	
	/* Wait for server's reply: */
	if(!pipe.waitForData(Misc::Time(30,0)))
		throw ProtocolError("VRDeviceClient: Timeout while waiting for CONNECT_REPLY",this);
	if(pipe.readMessage()!=VRDevicePipe::CONNECT_REPLY)
		throw ProtocolError("VRDeviceClient: Mismatching message while waiting for CONNECT_REPLY",this);
	serverProtocolVersionNumber=pipe.read<unsigned int>();
	
	/* Check server version number for compatibility: */
	if(serverProtocolVersionNumber<1U||serverProtocolVersionNumber>VRDevicePipe::protocolVersionNumber)
		throw ProtocolError("VRDeviceClient: Unsupported server protocol version",this);
	
	/* Read server's layout and initialize current state: */
	state.readLayout(pipe);
	
	/* Check if the server will send virtual input device descriptors: */
	if(serverProtocolVersionNumber>=2U)
		{
		/* Read the list of virtual devices managed by the server: */
		int numVirtualDevices=pipe.read<int>();
		for(int deviceIndex=0;deviceIndex<numVirtualDevices;++deviceIndex)
			{
			/* Create a new virtual input device and read its layout from the server: */
			VRDeviceDescriptor* newDevice=new VRDeviceDescriptor;
			newDevice->read(pipe);
			
			/* Store the virtual input device: */
			virtualDevices.push_back(newDevice);
			}
		}
	
	/* Check if the server will send tracker state time stamps: */
	serverHasTimeStamps=serverProtocolVersionNumber>=3U;
	}

VRDeviceClient::VRDeviceClient(const char* deviceServerName,int deviceServerPort)
	:pipe(deviceServerName,deviceServerPort),
	 serverProtocolVersionNumber(0),serverHasTimeStamps(false),
	 active(false),streaming(false),connectionDead(false),
	 packetNotificationCallback(0),errorCallback(0)
	{
	initClient();
	}

VRDeviceClient::VRDeviceClient(const Misc::ConfigurationFileSection& configFileSection)
	:pipe(configFileSection.retrieveString("./serverName").c_str(),configFileSection.retrieveValue<int>("./serverPort")),
	 serverProtocolVersionNumber(0),serverHasTimeStamps(false),
	 active(false),streaming(false),connectionDead(false),
	 packetNotificationCallback(0),errorCallback(0)
	{
	initClient();
	}

VRDeviceClient::~VRDeviceClient(void)
	{
	/* Leave streaming mode: */
	if(streaming)
		stopStream();
	
	/* Deactivate client: */
	if(active)
		deactivate();
	
	/* Disconnect from server: */
	pipe.writeMessage(VRDevicePipe::DISCONNECT_REQUEST);
	pipe.flush();
	
	/* Delete all virtual input devices: */
	for(std::vector<VRDeviceDescriptor*>::iterator vdIt=virtualDevices.begin();vdIt!=virtualDevices.end();++vdIt)
		delete *vdIt;
	}

void VRDeviceClient::activate(void)
	{
	if(!active&&!connectionDead)
		{
		pipe.writeMessage(VRDevicePipe::ACTIVATE_REQUEST);
		pipe.flush();
		active=true;
		}
	}

void VRDeviceClient::deactivate(void)
	{
	if(active)
		{
		active=false;
		if(!connectionDead)
			{
			pipe.writeMessage(VRDevicePipe::DEACTIVATE_REQUEST);
			pipe.flush();
			}
		}
	}

void VRDeviceClient::getPacket(void)
	{
	if(active)
		{
		if(streaming)
			{
			if(connectionDead)
				throw ProtocolError("VRDeviceClient: Server disconnected",this);
			
			/* Wait for arrival of next packet: */
			packetSignalCond.wait();
			if(connectionDead)
				throw ProtocolError("VRDeviceClient: Server disconnected",this);
			}
		else
			{
			/* Send packet request message: */
			pipe.writeMessage(VRDevicePipe::PACKET_REQUEST);
			pipe.flush();
			
			/* Wait for packet reply message: */
			if(!pipe.waitForData(Misc::Time(10,0))) // Throw exception if reply does not arrive in time
				{
				connectionDead=true;
				throw ProtocolError("VRDeviceClient: Timout while waiting for PACKET_REPLY",this);
				}
			if(pipe.readMessage()!=VRDevicePipe::PACKET_REPLY)
				{
				connectionDead=true;
				throw ProtocolError("VRDeviceClient: Mismatching message while waiting for PACKET_REPLY",this);
				}
			
			/* Read server's state: */
			try
				{
				Threads::Mutex::Lock stateLock(stateMutex);
				state.read(pipe,serverHasTimeStamps);
				if(!serverHasTimeStamps)
					setTrackerStateTimeStamps(state);
				}
			catch(std::runtime_error err)
				{
				/* Mark the connection as dead and re-throw the original exception: */
				connectionDead=true;
				throw;
				}
			}
		}
	}

void VRDeviceClient::startStream(VRDeviceClient::Callback* newPacketNotificationCallback,VRDeviceClient::ErrorCallback* newErrorCallback)
	{
	if(active&&!streaming&&!connectionDead)
		{
		/* Install the new callback functions: */
		packetNotificationCallback=newPacketNotificationCallback;
		errorCallback=newErrorCallback;
		
		/* Start the packet receiving thread: */
		streamReceiveThread.start(this,&VRDeviceClient::streamReceiveThreadMethod);
		
		/* Send start streaming message and wait for first state packet to arrive: */
		{
		Threads::MutexCond::Lock packetSignalLock(packetSignalCond);
		pipe.writeMessage(VRDevicePipe::STARTSTREAM_REQUEST);
		pipe.flush();
		packetSignalCond.wait(packetSignalLock);
		streaming=true;
		}
		}
	else
		{
		/* Just delete the new callback functions: */
		delete newPacketNotificationCallback;
		delete newErrorCallback;
		}
	}

void VRDeviceClient::stopStream(void)
	{
	if(streaming)
		{
		streaming=false;
		if(!connectionDead)
			{
			/* Send stop streaming message: */
			pipe.writeMessage(VRDevicePipe::STOPSTREAM_REQUEST);
			pipe.flush();
			
			/* Wait for packet receiving thread to die: */
			streamReceiveThread.join();
			}
		
		/* Delete the callback functions: */
		delete packetNotificationCallback;
		packetNotificationCallback=0;
		delete errorCallback;
		errorCallback=0;
		}
	}

}
