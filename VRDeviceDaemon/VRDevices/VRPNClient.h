/***********************************************************************
VRPNClient - Class to receive input device data from a remote VRPN
server.
Copyright (c) 2008-2010 Oliver Kreylos

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

#ifndef VRPNCLIENT_INCLUDED
#define VRPNCLIENT_INCLUDED

#include <Threads/Mutex.h>

#include <VRDeviceDaemon/VRDevice.h>
#include <VRDeviceDaemon/VRDevices/VRPNConnection.h>

class VRPNClient:public VRDevice,private VRPNConnection
	{
	/* Elements: */
	private:
	bool reportEvents; // Flag if device is started (VRPNConnection does not yet support drop/restart)
	Threads::Mutex stateMutex; // Mutex to serialize access to the local device state
	TrackerState* trackerStates; // Array of local tracker states; to assemble them before updating device state en bloc
	int* trackerFlags; // Array of completion flags per tracker; denote whether both position and velocity updates have been received
	ButtonState* buttonStates; // Ditto
	ValuatorState* valuatorStates; // Ditto
	
	/* Protected methods from VRDevice: */
	protected:
	virtual void deviceThreadMethod(void);
	
	/* Protected methods from VRPNConnection: */
	virtual void updateTrackerPosition(int trackerIndex,const PositionOrientation& positionOrientation);
	virtual void updateTrackerVelocity(int trackerIndex,const LinearVelocity& linearVelocity,const AngularVelocity& angularVelocity);
	virtual void updateButtonState(int buttonIndex,ButtonState newState);
	virtual void updateValuatorState(int valuatorIndex,ValuatorState newState);
	virtual void finalizePacket(void);
	
	/* Constructors and destructors: */
	public:
	VRPNClient(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile);
	virtual ~VRPNClient(void);
	
	/* Methods from VRDevice: */
	virtual void start(void);
	virtual void stop(void);
	};

#endif
