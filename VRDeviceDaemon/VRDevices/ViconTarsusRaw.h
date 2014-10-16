/***********************************************************************
ViconTarsusRaw - Class for Vicon optical trackers using the raw
real-time streaming protocol.
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

#ifndef VICONTARSUSRAW_INCLUDED
#define VICONTARSUSRAW_INCLUDED

#include <Comm/TCPPipe.h>

#include <VRDeviceDaemon/VRDevice.h>

class ViconTarsusRaw:public VRDevice
	{
	/* Embedded classes: */
	private:
	typedef Vrui::VRDeviceState::TrackerState::PositionOrientation PositionOrientation;
	typedef PositionOrientation::Point Point;
	typedef Point::Scalar Scalar;
	typedef PositionOrientation::Vector Vector;
	typedef PositionOrientation::Rotation Rotation;
	
	struct MarkerState // Structure for currently tracked markers
		{
		/* Elements: */
		public:
		Point position; // Current marker position
		Vector velocity; // Current estimated marker velocity in Vicon units per frame
		unsigned int lastVisibleFrame; // Index of the frame this marker was last matched
		};
	
	/* Elements: */
	Comm::TCPPipe pipe; // TCP pipe connected to remote tracking host
	int maxNumMarkers; // Maximum number of tracked markers
	Scalar matchTolerance2; // Squared maximum deviation for a marker from its predicted position for a match
	unsigned int predictionLimit; // Maximum number of frames for which a marker's position will be predicted and the button will be held
	unsigned int markerTimeout; // Number of frames a marker is held in the "off" position before it can be reused
	Point defaultPosition; // Position to assign to currently invisible markers (stored as vector to be used as translation)
	Vrui::VRDeviceState::TrackerState trackerStateTemplate; // Template to communicate tracker states to the VR device manager
	MarkerState* markerStates; // Array of marker states
	
	/* Protected methods: */
	virtual void deviceThreadMethod(void);
	
	/* Constructors and destructors: */
	public:
	ViconTarsusRaw(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile);
	virtual ~ViconTarsusRaw(void);
	
	/* Methods: */
	virtual void start(void);
	virtual void stop(void);
	};

#endif
