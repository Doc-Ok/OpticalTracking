/***********************************************************************
IMUTracker - Class to track the orientation and position of an inertial
measurement unit based on gravity and magnetometer drift correction and
positional dead reckoning.
Copyright (c) 2013-2014 Oliver Kreylos

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

#ifndef IMUTRACKER_INCLUDED
#define IMUTRACKER_INCLUDED

#include <Threads/TripleBuffer.h>
#include <Geometry/Vector.h>
#include <Geometry/Rotation.h>
#include <Geometry/OrthonormalTransformation.h>

#include "IMU.h"

class IMUTracker
	{
	/* Embedded classes: */
	public:
	typedef double Scalar;
	typedef Geometry::Vector<Scalar,3> Vector;
	typedef Geometry::Rotation<Scalar,3> Rotation;
	typedef Geometry::OrthonormalTransformation<Scalar,3> ONTransform;
	
	struct State // Structure encapsulating a tracking state
		{
		/* Elements: */
		public:
		Vector linearAcceleration; // Current linear acceleration in m/s^2
		Vector linearVelocity; // Current linear velocity in m/s
		Vector translation; // Current translation vector from origin in m
		Vector angularVelocity; // Current angular velocity in radians/s
		Rotation rotation; // Current rotation from identity orientation
		};
	
	/* Elements: */
	private:
	Scalar gravity; // Magnitude of gravity in m/s^2
	Scalar driftCorrectionWeight; // Weight for orientation drift correction based on gravity and magnetic flux direction
	bool magnetometer; // Flag whether the IMU device has a built-in magnetometer
	bool useMagnetometer; // Flag whether to use the IMU device's built-in magnetometer for orientation drift correction
	Threads::TripleBuffer<State> states; // Triple buffer of tracking states communicated to the main thread
	
	/* Constructors and destructors: */
	public:
	IMUTracker(const IMU& imu); // Creates a tracker for the given IMU object
	
	/* Methods: */
	void setGravity(Scalar newGravity); // Sets new local gravity magnitude in m/s^2
	void setDriftCorrectionWeight(Scalar newDriftCorrectionWeight); // Sets a new weight for gravity and magnetometer-based drift correction
	void setUseMagnetometer(bool newUseMagnetometer); // Enables or disables magnetometer-based drift correction
	void integrateSample(const IMU::CalibratedSample& newSample); // Integrates a new calibrated IMU sample into the tracker's current state; can be called from background thread
	bool lockNewState(void) // Locks the most recent tracker state; returns true if state has been updated since last call
		{
		return states.lockNewValue();
		}
	const State& getLockedState(void) const // Returns the most recently locked tracker state
		{
		return states.getLockedValue();
		}
	};

#endif
