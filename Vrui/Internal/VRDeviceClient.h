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

#ifndef VRUI_INTERNAL_VRDEVICECLIENT_INCLUDED
#define VRUI_INTERNAL_VRDEVICECLIENT_INCLUDED

#include <utility>
#include <vector>
#include <stdexcept>
#include <Misc/FunctionCalls.h>
#include <Threads/Thread.h>
#include <Threads/Mutex.h>
#include <Threads/MutexCond.h>
#include <Vrui/Internal/VRDeviceState.h>
#include <Vrui/Internal/VRDevicePipe.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}
namespace Vrui {
class VRDeviceDescriptor;
}

namespace Vrui {

class VRDeviceClient
	{
	/* Embedded classes: */
	public:
	class ProtocolError:public std::runtime_error // Exception when unexpected protocol messages are received
		{
		/* Elements: */
		public:
		VRDeviceClient* deviceClient; // Pointer to the device client that encountered the error
		
		/* Constructors and destructors: */
		ProtocolError(const std::string& what_arg,VRDeviceClient* sDeviceClient)
			:std::runtime_error(what_arg),
			 deviceClient(sDeviceClient)
			{
			}
		};
	
	typedef Misc::FunctionCall<VRDeviceClient*> Callback; // Type for callback functions
	typedef Misc::FunctionCall<const ProtocolError&> ErrorCallback; // Type for error callback functions called from background thread to signal errors in streaming mode
	
	/* Elements: */
	private:
	VRDevicePipe pipe; // Pipe connected to device server
	unsigned int serverProtocolVersionNumber; // Version number of server protocol
	bool serverHasTimeStamps; // Flag whether the connected device server sends tracker state time stamps
	std::vector<VRDeviceDescriptor*> virtualDevices; // List of virtual input devices managed by the server
	Threads::Mutex stateMutex; // Mutex to serialize access to current state
	VRDeviceState state; // Shadow of server's current state
	bool active; // Flag if client is active
	bool streaming; // Flag if client is in streaming mode
	volatile bool connectionDead; // Flag whether the connection to the server was interrupted while in streaming mode
	Threads::Thread streamReceiveThread; // Packet receiving thread in stream mode
	Threads::MutexCond packetSignalCond; // Condition variable to signal packet reception in streaming mode
	Callback* packetNotificationCallback; // Function called when a new state packet arrives from the server in streaming mode (called from background thread)
	ErrorCallback* errorCallback; // Function called when a protocol error occurs in streaming mode (called from background thread)
	
	/* Private methods: */
	void* streamReceiveThreadMethod(void); // Stream packet receiving thread method
	void initClient(void); // Initializes communication between device server and client
	
	/* Constructors and destructors: */
	public:
	VRDeviceClient(const char* deviceServerName,int deviceServerPort); // Connects client to given server
	VRDeviceClient(const Misc::ConfigurationFileSection& configFileSection); // Connects client to server listed in current configuration file section
	~VRDeviceClient(void); // Disconnects client from server
	
	/* Methods: */
	int getNumVirtualDevices(void) const // Returns the number of managed virtual input devices
		{
		return int(virtualDevices.size());
		}
	const VRDeviceDescriptor& getVirtualDevice(int deviceIndex) const // Returns the virtual input device of the given index
		{
		return *(virtualDevices[deviceIndex]);
		}
	void lockState(void) // Locks current server state
		{
		stateMutex.lock();
		}
	void unlockState(void) // Unlocks current server state
		{
		stateMutex.unlock();
		}
	const VRDeviceState& getState(void) const // Returns current server state (state must be locked while being used)
		{
		return state;
		}
	void activate(void); // Prepares the server for sending state packets
	void deactivate(void); // Deactivates server
	void getPacket(void); // Requests state packet from server; blocks until arrival
	void startStream(Callback* newPacketNotificationCallback,ErrorCallback* newErrorCallback =0); // Installs given callback functions (device client adopts function objects) and starts streaming mode
	void stopStream(void); // Stops streaming mode
	};

}

#endif
