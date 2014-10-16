/***********************************************************************
InterSense - Class for InterSense IS-900 hybrid inertial/sonic 6-DOF
tracking devices.
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

#ifndef INTERSENSE_INCLUDED
#define INTERSENSE_INCLUDED

#include <Misc/Timer.h>
#include <Comm/Pipe.h>

#include <VRDeviceDaemon/VRDevice.h>

/* Forward declarations: */
namespace Misc {
class Time;
}

class InterSense:public VRDevice
	{
	/* Embedded classes: */
	private:
	struct Station // Structure to hold configuration of a tracked station
		{
		/* Elements: */
		int id; // Identifier number of the station
		int numButtons; // Number of buttons on station
		int firstButtonIndex; // Index of first button on station
		bool joystick; // Flag if the station has an analog joystick attached to it
		int firstValuatorIndex; // Index of first valuator on station
		};
	
	typedef Vrui::VRDeviceState::TrackerState::PositionOrientation PositionOrientation; // Type for tracker position/orientation
	
	/* Elements: */
	Comm::PipePtr devicePort; // Port to which the tracker device hardware is connected (serial port or TCP socket)
	Station* stations; // Array of tracked stations
	int stationIdToIndex[32]; // Array mapping from station IDs to tracker indices
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
	InterSense(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile);
	virtual ~InterSense(void);
	
	/* Methods: */
	virtual void start(void);
	virtual void stop(void);
	};

#endif
