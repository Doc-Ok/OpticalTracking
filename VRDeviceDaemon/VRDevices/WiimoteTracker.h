/***********************************************************************
WiimoteTracker - Class to use a Nintendo Wii controller and a special
infrared LED beacon as a 6-DOF tracking device.
Copyright (c) 2007-2012 Oliver Kreylos

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

#ifndef WIIMOTETRACKER_INCLUDED
#define WIIMOTETRACKER_INCLUDED

#include <VRDeviceDaemon/VRDevices/CameraFitter.h>
#include <VRDeviceDaemon/LevenbergMarquardtMinimizer.h>

#include <VRDeviceDaemon/VRDevice.h>

/* Forward declarations: */
namespace Misc {
class CallbackData;
}
class Wiimote;

class WiimoteTracker:public VRDevice
	{
	/* Embedded classes: */
	private:
	typedef CameraFitter::Scalar Scalar;
	typedef CameraFitter::Point Point;
	typedef CameraFitter::Vector Vector;
	typedef CameraFitter::Transform Transform;
	typedef CameraFitter::Pixel Pixel;
	typedef LevenbergMarquardtMinimizer<CameraFitter> LMCamera;
	typedef Vrui::VRDeviceState::TrackerState::PositionOrientation PositionOrientation; // Type for tracker position/orientation
	
	/* Elements: */
	Wiimote* wiimote; // Object representing the Wiimote device
	int ledMask; // LED array mask for the Wiimote device; set while Wiimote is active
	bool enableTracker; // Flag whether to enable position tracking using the accelerometers and camera. If disabled, will report only buttons and optional Nunchuck joystick
	CameraFitter wiiCamera; // Camera fitter object for the Wiimote
	Transform homeTransform; // "Home" position of the Wiimote; used when resetting tracking
	bool firstEvent; // Flag if the received status update is the first
	Vector acceleration; // Wiimote's current acceleration vector in device coordinates
	Vector lastAcceleration; // Previous instantaneous acceleration vector in device coordinates
	bool pixelValids[4]; // Valid flags for up to four targets tracked by the Wiimote's IR camera
	Pixel pixels[4]; // Positions of up to four targets tracked by the Wiimote's IR camera
	int pixelMap[4]; // Array mapping IR target indices to target point indices
	Transform wiipos; // Position and orientation of the virtual Wiimote
	bool reportEvents; // Flag whether to report wiimote events to the VR device manager
	
	/* Private methods: */
	void wiimoteEventCallbackNoTracker(Misc::CallbackData* cbData); // Callback when Wiimote receives a status update when tracking is disabled
	void wiimoteEventCallback(Misc::CallbackData* cbData); // Callback when Wiimote receives a status update when tracking is enabled
	
	/* Constructors and destructors: */
	public:
	WiimoteTracker(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile);
	virtual ~WiimoteTracker(void);
	
	/* Methods: */
	virtual void start(void);
	virtual void stop(void);
	};

#endif
