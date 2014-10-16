/***********************************************************************
SpaceBall - Class for 6-DOF joysticks (Spaceball 4000FLX).
Copyright (c) 2002-2011 Oliver Kreylos

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

#ifndef SPACEBALL_INCLUDED
#define SPACEBALL_INCLUDED

#include <Comm/SerialPort.h>

#include <VRDeviceDaemon/VRDevice.h>

/* Forward declarations: */
namespace Misc {
class Time;
}

class SpaceBall:public VRDevice
	{
	/* Embedded classes: */
	private:
	typedef Vrui::VRDeviceState::TrackerState::PositionOrientation PositionOrientation; // Type for tracker position/orientation
	
	/* Elements: */
	Comm::SerialPort devicePort; // Serial port the tracking device hardware is connected to
	double linearGain; // Multiplication factor for linear velocities
	double angularGain; // Multiplication factor for angular velocities
	PositionOrientation currentPositionOrientation; // Current position/orientation of space ball device
	
	/* Private methods: */
	bool readLine(int lineBufferSize,char* lineBuffer,const Misc::Time& deadline); // Reads a line of text from the space ball with timeout
	int readPacket(int packetBufferSize,unsigned char* packetBuffer); // Reads a space ball status packet from the serial port; returns number of read characters
	
	/* Protected methods: */
	virtual void deviceThreadMethod(void);
	
	/* Constructors and destructors: */
	public:
	SpaceBall(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile);
	
	/* Methods: */
	virtual void start(void);
	virtual void stop(void);
	};

#endif
