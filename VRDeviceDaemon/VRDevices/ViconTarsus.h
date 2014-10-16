/***********************************************************************
ViconTarsus - Class for Vicon optical trackers using the real-time
streaming protocol.
Copyright (c) 2007-2011 Oliver Kreylos

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

#ifndef VICONTARSUS_INCLUDED
#define VICONTARSUS_INCLUDED

#include <utility>
#include <Comm/TCPPipe.h>

#include <VRDeviceDaemon/VRDevice.h>

class ViconTarsus:public VRDevice
	{
	/* Embedded classes: */
	private:
	typedef Vrui::VRDeviceState::TrackerState::PositionOrientation PositionOrientation;
	typedef PositionOrientation::Vector Vector;
	typedef Vector::Scalar VScalar;
	typedef PositionOrientation::Rotation Rotation;
	typedef Rotation::Scalar RScalar;
	
	/* Elements: */
	Comm::TCPPipe pipe; // TCP pipe connected to remote tracking host
	int* trackerChannelIndices; // 2D array of channel indices for each component (position, rotation axis) of each tracker
	bool* trackerSixDofs; // Array of flags whether each tracker is 3-DOF (single marker, position only) or 6-DOF
	int numChannels; // Number of channels maximally reported by the server
	std::pair<int,int>* trackerMap; // Array mapping from channel indices to (tracker, parameter) pairs
	double* channelPacketBuffer; // Buffer for channel data packets received from the server
	Vrui::VRDeviceState::TrackerState* trackerStates; // Local copy of all tracker states to fill in missing data
	
	/* Protected methods: */
	virtual void deviceThreadMethod(void);
	
	/* Constructors and destructors: */
	public:
	ViconTarsus(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile);
	virtual ~ViconTarsus(void);
	
	/* Methods: */
	virtual void start(void);
	virtual void stop(void);
	};

#endif
