/***********************************************************************
HIDDevice - VR device driver class for generic input devices supported
by the Linux or MacOS X HID event interface. Reports buttons and
absolute axes.
Copyright (c) 2004-2010 Oliver Kreylos
MacOS X additions copyright (c) 2006 Braden Pellett

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

#ifndef HIDDEVICE_INCLUDED
#define HIDDEVICE_INCLUDED

#ifdef __APPLE__
#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDLib.h>
#include <map>
#endif
#include <string>
#include <Threads/Mutex.h>
#include <Math/BrokenLine.h>

#include <VRDeviceDaemon/VRDevice.h>

class HIDDevice:public VRDevice
	{
	/* Embedded classes: */
	public:
	typedef Math::BrokenLine<float> AxisConverter; // Type to convert raw axis values to the [-1, 1] range
	
	/* Elements: */
	private:
	#ifdef __linux__
	int deviceFd; // File descriptor for the HID device
	int* keyMap; // Translation map from event key codes to device button indices
	int* absAxisMap; // Translation map from event absolute axis codes to device valuator indices
	int* relAxisMap; // Translation map from event relative axis codes to device valuator indices
	AxisConverter* axisConverters; // Array of converters from raw axis values to valuator values
	bool reportEvents; // Flag if device is started (HID device cannot be disabled in hardware)
	Threads::Mutex stateMutex; // Mutex to serialize access to the device state
	bool* buttonStates; // Array of current button states
	float* valuatorStates; // Array of current valuator values
	#endif
	#ifdef __APPLE__
	typedef std::map<IOHIDElementCookie,int> CookieIndexMap;
	struct AxisInfo
		{
		/* Elements: */
		public:
		int index;
		AxisConverter converter;
		};
	typedef std::map<IOHIDElementCookie,AxisInfo> CookieAxisInfoMap;
	CookieIndexMap buttonMap; // Translation map from button element cookies to device button indices
	CookieAxisInfoMap absAxisMap; // Translation map from axis element cookies to device valuator indices
	                              // and converters from raw absolute axis values to valuator values
	IOHIDDeviceInterface** hidDeviceInterface;
	IOHIDQueueInterface** hidQueueInterface;
	CFRunLoopSourceRef eventSource;
	CFRunLoopRef cfRunLoop;
	Threads::Mutex runLoopMutex; // Mutex to ensure completion of the CF run loop before stopping its thread
	static unsigned queueDepth; // Maximum number of elements in the queue before
	                            // the oldest elements in the queue begin to be lost.
	#endif
	
	/* Private methods: */
	#ifdef __linux__
	int findDevice(int vendorId,int productId); // Finds a HID device by vendor ID / product ID
	int findDevice(const char* deviceName); // Finds a HID device by name
	#endif
	#ifdef __APPLE__
	io_object_t findHIDDeviceByVendorIdAndProductId(int targetVendorId,int targetProductId);
	io_object_t findHIDDeviceByName(const char* targetDeviceName);
	io_object_t getHIDDevice(Misc::ConfigurationFile& configFile);
	IOHIDDeviceInterface** createHIDDeviceInterface(io_object_t hidDevice);
	void setupHIDDeviceInterface(Misc::ConfigurationFile& configFile);
	void setupButtonAndAxisMaps(Misc::ConfigurationFile& configFile);
	void setupEventQueue(void);
	void handleEvents(void);
	static void queueCallbackFunction(void* target,IOReturn result,void* refcon,void* sender);
	#endif
	
	/* Protected methods: */
	virtual void deviceThreadMethod(void);
	
	/* Constructors and destructors: */
	public:
	HIDDevice(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile);
	virtual ~HIDDevice(void);
	
	/* Methods: */
	virtual void start(void);
	virtual void stop(void);
	};

#endif
