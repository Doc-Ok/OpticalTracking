/***********************************************************************
FakespacePinchGlove - Class for pinch glove device of same name.
Copyright (c) 1998-2011 Oliver Kreylos

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

#ifndef FAKESPACEPINCHGLOVE_INCLUDED
#define FAKESPACEPINCHGLOVE_INCLUDED

#include <Threads/Mutex.h>
#include <Comm/SerialPort.h>

#include <VRDeviceDaemon/VRDevice.h>

class FakespacePinchGlove:public VRDevice
	{
	/* Elements: */
	private:
	Comm::SerialPort devicePort; // Serial port the pinch glove device hardware is connected to
	bool reportEvents; // Flag if device is started (pinch glove device cannot be disabled in hardware)
	Threads::Mutex pinchMaskMutex; // Mutex to serialize access to the pinch masks
	int pinchMasks[2]; // Bit mask for current pinch states of all gloves
	
	/* Private methods: */
	void writeCommand(const char* command); // Writes command to device port
	char* readReply(int stringBufferLen,char* stringBuffer); // Reads reply string from device port
	void ignoreReply(void); // Ignores reply string from device port
	
	/* Protected methods: */
	virtual void deviceThreadMethod(void);
	
	/* Constructors and destructors: */
	public:
	FakespacePinchGlove(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile);
	virtual ~FakespacePinchGlove(void);
	
	/* Methods: */
	virtual void start(void);
	virtual void stop(void);
	};

#endif
