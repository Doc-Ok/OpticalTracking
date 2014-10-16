/***********************************************************************
AscensionFlockOfBirds - Class for tracking device of same name.
Copyright (c) 2000-2011 Oliver Kreylos

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

#ifndef ASCENSIONFLOCKOFBIRDS_INCLUDED
#define ASCENSIONFLOCKOFBIRDS_INCLUDED

#include <Misc/Timer.h>
#include <Comm/SerialPort.h>

#include <VRDeviceDaemon/VRDevice.h>

class AscensionFlockOfBirds:public VRDevice
	{
	/* Embedded classes: */
	private:
	typedef Vrui::VRDeviceState::TrackerState::PositionOrientation PositionOrientation; // Type for tracker position/orientation
	
	/* Elements: */
	Comm::SerialPort devicePort; // Serial port the tracker device hardware is connected to
	int masterId; // Device ID of FOB master
	int firstBirdId; // Device ID of first receiver
	int ercId; // Device ID of extended range controller; -1 if unused
	int ercTransmitterIndex; // Index of transmitter on extended range controller
	double trackerRange; // Maximum range of reported tracker coordinates
	Misc::Timer* timers; // Array of free-running timers for each tracker for velocity estimation
	bool* notFirstMeasurements; // Array of flags for each tracker if a measurement has already been delivered
	PositionOrientation* oldPositionOrientations; // Array of old tracker positions/orientations
	
	/* Private methods: */
	short int readShort(void); // Reads a 16-bit signed integer from the device port
	void writeShort(short int value); // Writes a 16-bit signed integer to the device port
	short int extractShort(const char* lsb) const; // Extracts a bit-stuffed 16-bit signed integer from two bytes of memory (lsb first)
	
	/* Protected methods: */
	protected:
	virtual void deviceThreadMethod(void);
	
	/* Constructors and destructors: */
	public:
	AscensionFlockOfBirds(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile);
	virtual ~AscensionFlockOfBirds(void);
	
	/* Methods: */
	virtual void start(void);
	virtual void stop(void);
	};

#endif
