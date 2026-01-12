/***********************************************************************
IMUTracker - Class to track the orientation and position of an inertial
measurement unit based on gravity and magnetometer drift correction and
positional dead reckoning.
Copyright (c) 2013-2015 Oliver Kreylos

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

#include <Threads/Spinlock.h>
#include <Geometry/Vector.h>
#include <Geometry/Rotation.h>
#include <Geometry/OrthonormalTransformation.h>

#include "IMU.h"

/* Forward declarations: */
namespace Misc {
template <class ParameterParam>
class FunctionCall;
}

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
		TimeStamp timeStamp; // Time stamp of tracking state
		Vector linearAcceleration; // Current linear acceleration in m/s^2
		Vector linearVelocity; // Current linear velocity in m/s
		Vector translation; // Current translation vector from origin in m
		Vector angularVelocity; // Current angular velocity in radians/s
		Rotation rotation; // Current rotation from identity orientation
		};
	
	typedef Misc::FunctionCall<const State&> TrackingCallback; // Type of callback called when a new tracking state has been calculated
	
	/* Elements: */
	private:
	Scalar gravity; // Magnitude of gravity in m/s^2
	bool magnetometer; // Flag whether the IMU device has a built-in magnetometer
	bool useMagnetometer; // Flag whether to use the IMU device's built-in magnetometer for orientation drift correction
	Scalar biasDriftGain; // Gain factor for gyroscope bias drift correction
	Scalar orientationDriftGain; // Gain factor for orientation drift correction based on gravity and optionally magnetic flux direction
	Vector initialAccel,initialGyro,initialMag; // Vectors to initialize tracking state by accumulating initial samples
	unsigned int numWarmupSamples; // Number of IMU samples collected during warm-up phase
	Vector gyroscopeBias; // Current gyroscope bias in radians/s
	TimeStamp lastTimeStamp; // Time stamp of most recently received IMU sample
	Vector lastTranslation; // Translation vector at last time step for Verlet integration
	TrackingCallback* trackingCallback; // Callback called when a new tracking state has been calculated
	unsigned int stateBufferSize; // Number of slots in the tracking state history buffer
	State* stateBuffer; // Tracking state history buffer
	mutable Threads::Spinlock indexMutex; // Mutex protecting the tracking state history buffer indices
	volatile unsigned int mostRecentState; // Index of most recent tracking state in the history buffer
	volatile unsigned int lockedState; // Index of currently locked tracking state in the history buffer
	
	/* Constructors and destructors: */
	public:
	IMUTracker(const IMU& imu,unsigned int sStateBufferSize =128U); // Creates a tracker for the given IMU object
	~IMUTracker(void); // Destroys the tracker
	
	/* Methods: */
	void setGravity(Scalar newGravity); // Sets new local gravity magnitude in m/s^2
	void setUseMagnetometer(bool newUseMagnetometer); // Enables or disables magnetometer-based drift correction
	void setBiasDriftGain(Scalar newBiasDriftGain); // Sets new gyroscope bias drift correction gain factor
	void setOrientationDriftGain(Scalar newOrientationDriftGain); // Sets new orientation drift correction gain factor
	void integrateSample(const IMU::CalibratedSample& newSample); // Integrates a new calibrated IMU sample into the tracker's current state; can be called from background thread
	void startStreaming(TrackingCallback* newTrackingCallback); // Starts streaming tracking states to the given tracking callback
	void stopStreaming(void); // Stops streaming tracking states
	bool hasNewState(void) const // Returns true if there is a tracker state in the history buffer that is newer than the currently locked state
		{
		Threads::Spinlock::Lock indexLock(indexMutex);
		return mostRecentState!=lockedState;
		}
	bool lockNewState(void) // Locks the most recently created tracker state; returns true if state has changed since last call
		{
		Threads::Spinlock::Lock indexLock(indexMutex);
		unsigned int oldLockedState=lockedState;
		lockedState=mostRecentState;
		return lockedState!=oldLockedState;
		}
	const State& getLockedState(void) const // Returns the currently locked tracker state
		{
		return stateBuffer[lockedState];
		}
	const State& getRecentState(TimeStamp timeStamp) const; // Returns the recent tracking state most closely matching the given absolute time stamp
	const Vector& getGyroscopeBias(void) const // Returns the current gyroscope bias correction vector
		{
		return gyroscopeBias;
		}
	void applyCorrection(const Vector& positionDelta,const Vector& velocityDelta); // Applies the given position and linear velocity correction vectors to the current tracking state
	void restart(void); // Resets linear and angular velocities of current tracking state
	void restart(const Vector& translation); // Re-initializes the current tracking state based on the given position; retains orientation
	void restart(const Vector& translation,const Rotation& rotation); // Re-initializes the current tracking state based on the given position and orientation
	};

#endif
