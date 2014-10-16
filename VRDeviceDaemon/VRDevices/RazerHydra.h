/***********************************************************************
RazerHydra - Class to represent a Razer / Sixense Hydra dual-sensor
desktop 6-DOF tracking device.
Copyright (c) 2011-2013 Oliver Kreylos

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

#ifndef RAZERHYDRA_INCLUDED
#define RAZERHYDRA_INCLUDED

#include <Misc/ReadBuffer.h>
#include <Threads/Thread.h>
#include <USB/Device.h>
#include <Geometry/Point.h>
#include <Geometry/Rotation.h>

/* Forward declarations: */
namespace Misc {
template <class ParameterParam>
class FunctionCall;
}
namespace USB {
class Context;
}

class RazerHydra
	{
	/* Embedded classes: */
	public:
	typedef float Scalar; // Scalar type for positions, orientations, and valuator (analog axis) states
	typedef Geometry::Point<Scalar,3> Point; // Type for sensor positions
	typedef Geometry::Rotation<Scalar,3> Orientation; // Type for sensor orientations
	
	struct SensorState // Structure defining the state of one sensor
		{
		/* Elements: */
		public:
		unsigned int sensorIndex; // Sensor index; 0 for left sensor, 1 for right sensor
		Point position; // Sensor position relative to base station
		Orientation orientation; // Sensor orientation relative to base station
		bool buttonStates[7]; // Array of button states, in order trigger, 3, 1, 2, 4, center, joystick
		Scalar valuatorStates[3]; // Array of valuator (analog axis) states, in order joystick x, joystick y, shoulder
		};
	
	typedef Misc::FunctionCall<const SensorState&> StreamingCallback; // Function call type for streaming state update callbacks
	
	/* Elements: */
	private:
	USB::Device device; // The USB device representing the Razer Hydra hardware
	bool wasInGamepadMode; // Flag whether the device was in gamepad emulation mode when opened
	Scalar positionConversionFactor; // Conversion factor from raw Razer Hydra units (mm) to user units
	Misc::ReadBuffer packetBuffer; // A buffer to help parsing device status update packets
	bool applyInterleaveFilter; // Flag whether to apply the 4x interleaving filter
	Scalar valueBuffers[14][4]; // A smoothing buffer for each raw position coordinate / quaternion component value
	int valueBufferIndex; // The current index into the smoothing buffers
	Scalar smoothingCoeffs[14][4][2]; // Coefficients for the current smoothing functions
	bool applyLowpassFilter; // Flag whether to apply the low-pass filter
	Scalar lowpassFilterStrength; // Strength of low-pass filter; reasonable values are around 32.0
	Scalar accumulators[14];// Accumulation buffers for the low-pass filter
	volatile bool streaming; // Flag whether the device is currently in streaming mode
	StreamingCallback* streamingCallback; // Callback to be called when a new update packet has been processed
	Threads::Thread streamingThread; // Background thread reading state update packets from the USB device
	
	/* Private methods: */
	bool processUpdatePacket(SensorState states[2]); // Reads the next update packet and extracts state values into the two given structures
	void* streamingThreadMethod(void); // Thread method for the streaming thread
	
	/* Constructors and destructors: */
	public:
	RazerHydra(USB::Context& usbContext,unsigned int index =0); // Connects to the index-th Razer Hydra device in the given USB context
	private:
	RazerHydra(const RazerHydra& source); // Prohibit copy constructor
	RazerHydra& operator=(const RazerHydra& source); // Prohibit assignment operator
	public:
	~RazerHydra(void); // Disconnects from this Razer Hydra device
	
	/* Methods: */
	Scalar getPositionConversionFactor(void) const // Returns the current position unit conversion factor
		{
		return positionConversionFactor;
		}
	void setPositionConversionFactor(Scalar newPositionConversionFactor); // Sets the position unit conversion factor to an arbitrary value
	void setPositionMMs(void) // Reports position in mms
		{
		setPositionConversionFactor(Scalar(1)); // Raw units are mm
		}
	void setPositionInches(void) // Reports position in inches
		{
		setPositionConversionFactor(Scalar(1)/Scalar(25.4)); // Raw units are mm
		}
	void setApplyInterleaveFilter(bool newApplyInterleaveFilter); // Turns the interleave filter on or off
	void setApplyLowpassFilter(bool newApplyLowpassFilter); // Turns the low-pass filter on or off
	void setLowpassFilterStrength(Scalar newLowpassFilterStrength); // Sets the strength of the low-pass filter
	
	/* Polling interface: */
	void pollSensors(SensorState states[2]); // Fills the given sensor state structures with values from the next update packet; blocks until next packet arrives
	
	/* Interrupt-driven interface: */
	void startStreaming(StreamingCallback* newStreamingCallback); // Starts streaming mode; the given callback function will be called from a background thread with each sensor state update as it is received, and deleted when streaming is stopped
	void stopStreaming(void); // Stops streaming mode
	};

#endif
