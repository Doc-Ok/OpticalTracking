/***********************************************************************
PolhemusFastrak - Class for tracking device of same name.
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

#ifndef POLHEMUSFASTRAK_INCLUDED
#define POLHEMUSFASTRAK_INCLUDED

#include <Misc/Timer.h>
#include <Comm/SerialPort.h>

#include <VRDeviceDaemon/VRDevice.h>

/* Forward declarations: */
namespace Misc {
class Time;
}

class PolhemusFastrak:public VRDevice
	{
	/* Embedded classes: */
	private:
	typedef Vrui::VRDeviceState::TrackerState::PositionOrientation PositionOrientation; // Type for tracker position/orientation
	
	/* Elements: */
	Comm::SerialPort devicePort; // Serial port the tracker device hardware is connected to
	bool stylusEnabled; // Flag to enable reporting a stylus' button state
	Misc::Timer* timers; // Array of free-running timers for each tracker for velocity estimation
	bool* notFirstMeasurements; // Array of flags for each tracker if a measurement has already been delivered
	PositionOrientation* oldPositionOrientations; // Array of old tracker positions/orientations
	
	/* Private methods: */
	char* readLine(int lineBufferSize,char* lineBuffer,const Misc::Time& deadline); // Reads a CR/LF terminated line from the serial port and stores it as NUL-terminated string (without CR/LF bytes); terminates early if not completed by deadline
	bool readStatusReply(void); // Reads device's reply to a status request
	bool processRecord(void); // Reads and processes a record from the device; returns true if synchronization with tracker data stream was lost
	
	/* Protected methods: */
	virtual void deviceThreadMethod(void);
	
	/* Constructors and destructors: */
	public:
	PolhemusFastrak(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile);
	virtual ~PolhemusFastrak(void);
	
	/* Methods: */
	virtual void start(void);
	virtual void stop(void);
	};

#endif
