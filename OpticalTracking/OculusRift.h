/***********************************************************************
OculusRift - Class to represent the tracking subsystem of an Oculus Rift
head-mounted display as an inertially-tracked input device.
Copyright (c) 2014 Oliver Kreylos

This file is part of the optical/inertial sensor fusion tracking
package.

The optical/inertial sensor fusion tracking package is free software;
you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

The optical/inertial sensor fusion tracking package is distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the optical/inertial sensor fusion tracking package; if not, write
to the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef OCULUSRIFT_INCLUDED
#define OCULUSRIFT_INCLUDED

#include <string>
#include <Threads/Thread.h>
#include <RawHID/Device.h>

#include "IMU.h"

class OculusRift:public RawHID::Device,public IMU
	{
	/* Embedded classes: */
	public:
	enum DeviceType // Enumerated type for Oculus Rift device types
		{
		UNKNOWN,
		DK1,
		DK2
		};
	
	/* Elements: */
	private:
	DeviceType deviceType; // Type of Oculus Rift HMD
	bool opticalTracking; // Flag whether optical tracking is currently enabled
	Threads::Thread samplingThread; // Thread object for the background sampling thread
	volatile bool keepSampling; // Flag to shut down the background sampling thread
	
	/* Private methods: */
	void initialize(void); // Initializes the Oculus Rift tracker after the raw HID device has been opened
	void* samplingThreadMethod(void); // Thread method for the background sampling thread
	
	/* Constructors and destructors: */
	public:
	OculusRift(unsigned int deviceIndex); // Connects to the Oculus Rift tracker of the given zero-based index on the local HID bus
	OculusRift(const std::string& deviceSerialNumber); // Connects to the Oculus Rift tracker of the given serial number on the local HID bus
	virtual ~OculusRift(void);
	
	/* Methods from IMU: */
	virtual std::string getSerialNumber(void) const;
	virtual void startStreamingRaw(RawSampleCallback* newRawSampleCallback);
	virtual void startStreamingCalibrated(CalibratedSampleCallback* newCalibratedSampleCallback);
	virtual void stopStreaming(void);
	
	/* New methods: */
	DeviceType getDeviceType(void) const // Returns the type of this Oculus Rift device
		{
		return deviceType;
		}
	void startOpticalTracking(void); // Configures the device for optical tracking if it has the capability; called before startStreaming
	void stopOpticalTracking(void); // Configures the device for standard non-optically tracked operation; called after stopStreaming
	};

#endif
