/***********************************************************************
OculusRift - Class represent the Oculus Rift HMD's built-in orientation
tracker.
Copyright (c) 2013-2014 Oliver Kreylos

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

#ifndef OCULUSRIFT_INCLUDED
#define OCULUSRIFT_INCLUDED

#include <Misc/SizedTypes.h>
#include <USB/Context.h>
#include <USB/Device.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Rotation.h>
#include <Geometry/Matrix.h>

#include <VRDeviceDaemon/VRDevice.h>

class OculusRift:public VRDevice
	{
	/* Embedded classes: */
	private:
	enum DeviceModel // Enumerated type for different Oculus Rift models
		{
		UNKNOWN,DK1,DK2
		};
	typedef float Scalar;
	typedef Geometry::Point<Scalar,3> Point;
	typedef Geometry::Vector<Scalar,3> Vector;
	typedef Geometry::Rotation<Scalar,3> Rotation;
	typedef Geometry::Matrix<Scalar,3,4> Correction; // Type for sensor correction matrices (affine transformations)
	typedef Vrui::VRDeviceState::TrackerState TrackerState; // Type for tracker states
	typedef TrackerState::PositionOrientation PositionOrientation; // Type for tracker position/orientation
	
	/* Elements: */
	private:
	USB::Context usbContext; // A USB context for the Oculus Rift device
	USB::Device oculus; // The Oculus Rift device
	DeviceModel deviceModel; // Model of the connected Oculus Rift device
	Correction accelCorrect; // Correction transformation to rectify the three-axis accelerometer
	Correction magCorrect; // Correction transformation to rectify the three-axis magnetometer ("hard" and "soft iron" correction)
	Point neckPivot; // Position of rotation pivot point in HMD-relative coordinates
	Scalar driftCorrectionWeight; // Weight factor for drift correction; 0.0001 is good value
	bool useMagnetometer; // Flag whether to use the built-in magnetometer for yaw drift correction (requires mag correction transformation)
	Scalar motionPredictionDelta; // Time interval for motion prediction in seconds
	unsigned int updateRate; // Rate at which new tracking data is sent to the device manager (and clients)
	bool reportEvents; // Flag if device is started (its best to keep the tracker running at all times)
	Rotation currentOrientation; // Current accumulated orientation
	Vector currentLinearAcceleration; // Current filtered linear acceleration vector
	Vector currentAngularVelocity; // Current filtered angular velocity vector
	Vector currentMagneticFlux; // Current filtered magnetic flux density vector
	Misc::UInt16 nextTimeStamp; // Expected time stamp of next sensor data packet
	volatile bool keepRunning; // Flag to shut down the device thread without lock-ups in the USB stack
	
	/* Protected methods: */
	virtual void deviceThreadMethod(void);
	
	/* Constructors and destructors: */
	public:
	OculusRift(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile);
	virtual ~OculusRift(void);
	
	/* Methods: */
	virtual void start(void);
	virtual void stop(void);
	};

#endif
