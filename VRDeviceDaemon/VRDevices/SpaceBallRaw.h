/***********************************************************************
SpaceBallRaw - VR device driver class exposing the "raw" interface of a
6-DOF joystick as a collection of buttons and valuators. The conversion
from the raw values into 6-DOF states is done at the application end.
Copyright (c) 2004-2011 Oliver Kreylos

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

#ifndef SPACEBALLRAW_INCLUDED
#define SPACEBALLRAW_INCLUDED

#include <Comm/SerialPort.h>
#include <Math/BrokenLine.h>

#include <VRDeviceDaemon/VRDevice.h>

/* Forward declarations: */
namespace Misc {
class Time;
}

class SpaceBallRaw:public VRDevice
	{
	/* Embedded classes: */
	private:
	typedef Math::BrokenLine<double> AxisConverter; // Type to convert raw axis values to the [-1, 1] range
	
	/* Elements: */
	private:
	Comm::SerialPort devicePort; // Serial port the tracking device hardware is connected to
	AxisConverter axisConverters[6]; // Converters for the device's axes
	
	/* Private methods: */
	bool readLine(int lineBufferSize,char* lineBuffer,const Misc::Time& deadline); // Reads a line of text from the space ball with timeout
	int readPacket(int packetBufferSize,unsigned char* packetBuffer); // Reads a space ball status packet from the serial port; returns number of read characters
	
	/* Protected methods: */
	virtual void deviceThreadMethod(void);
	
	/* Constructors and destructors: */
	public:
	SpaceBallRaw(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile);
	
	/* Methods: */
	virtual void start(void);
	virtual void stop(void);
	};

#endif
