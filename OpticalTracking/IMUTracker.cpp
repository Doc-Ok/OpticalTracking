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

#include "IMUTracker.h"

/***************************
Methods of class IMUTracker:
***************************/

IMUTracker::IMUTracker(const IMU& imu)
	:gravity(9.81),
	 driftCorrectionWeight(0.0001),
	 magnetometer(imu.getCalibrationData().magnetometer),
	 useMagnetometer(magnetometer),
	 numInitialSamples(10),initialAccel(Vector::zero),initialMag(Vector::zero)
	{
	}

void IMUTracker::setGravity(Scalar newGravity)
	{
	gravity=newGravity;
	}

void IMUTracker::setDriftCorrectionWeight(IMU::Scalar newDriftCorrectionWeight)
	{
	driftCorrectionWeight=newDriftCorrectionWeight;
	}

void IMUTracker::setUseMagnetometer(bool newUseMagnetometer)
	{
	/* Only use the magnetometer if the IMU has one: */
	useMagnetometer=newUseMagnetometer&&magnetometer;
	}

void IMUTracker::integrateSample(const IMU::CalibratedSample& sample)
	{
	if(numInitialSamples>0)
		{
		/* Accumulate the initial acceleration and magnetic flux vectors: */
		initialAccel+=sample.accelerometer;
		initialMag+=sample.magnetometer;
		
		--numInitialSamples;
		if(numInitialSamples==0)
			{
			/* Create the initial tracking state: */
			State& initial=states.startNewValue();
			initial.linearAcceleration=Vector::zero;
			initial.linearVelocity=Vector::zero;
			initial.translation=Vector::zero;
			initial.angularVelocity=Vector::zero;
			if(useMagnetometer)
				{
				/* Align acceleration vector with +Z and magnetic flux density vector with +X: */
				initialMag.orthogonalize(initialAccel);
				initial.rotation=Rotation::fromBaseVectors(initialMag,initialAccel);
				initial.rotation.leftMultiply(Rotation::rotateX(Math::rad(Scalar(90))));
				}
			else
				{
				/* Align acceleration vector with +Z: */
				Vector initialX(1,0,0);
				initialX.orthogonalize(initialAccel);
				initial.rotation=Rotation::fromBaseVectors(initialX,initialAccel);
				initial.rotation.leftMultiply(Rotation::rotateX(Math::rad(Scalar(90))));
				}
			states.postNewValue();
			}
		
		return;
		}
	
	/* Get the current and next tracking states: */
	const State& current=states.getMostRecentValue();
	State& next=states.startNewValue();
	
	/* Transform current linear acceleration, angular velocity, and magnetic flux density from current tracker frame to global space: */
	Vector gAccel=current.rotation.transform(sample.accelerometer);
	Vector gGyro=current.rotation.transform(sample.gyroscope);
	
	/* Subtract gravity from current linear acceleration in global space and integrate twice to update the current position: */
	next.linearAcceleration=gAccel;
	next.linearAcceleration[2]-=gravity;
	next.linearVelocity=current.linearVelocity+current.linearAcceleration*sample.timeStep;
	next.translation=current.translation+current.linearVelocity*sample.timeStep;
	
	/* Store current angular velocity: */
	next.angularVelocity=gGyro;
	
	/* Integrate the angular velocity into the current rotation: */
	next.rotation=current.rotation*Rotation::rotateScaledAxis(sample.gyroscope*sample.timeStep);
	
	if(driftCorrectionWeight>Scalar(0))
		{
		if(useMagnetometer)
			{
			/* Build a coordinate frame in global space where x points to magnetic north and z points up: */
			Vector gMag=current.rotation.transform(sample.magnetometer);
			gMag.orthogonalize(gAccel);
			Rotation globalFrame=Rotation::fromBaseVectors(gMag,gAccel);
			globalFrame*=Rotation::rotateX(Math::rad(Scalar(-90)));
			
			/* Nudge the current rotation towards the desired global frame: */
			globalFrame.doInvert();
			Vector gRotation=globalFrame.getScaledAxis();
			next.rotation.leftMultiply(Rotation::rotateScaledAxis(gRotation*driftCorrectionWeight));
			}
		else
			{
			/* Nudge the current rotation's z axis towards the direction of gravity: */
			Rotation gOffset=Rotation::rotateFromTo(gAccel,Vector(0,0,1));
			Vector gRotation=gOffset.getScaledAxis();
			next.rotation.leftMultiply(Rotation::rotateScaledAxis(gRotation*driftCorrectionWeight));
			}
		}
	
	/* Renormalize the next rotation: */
	next.rotation.renormalize();
	
	/* Post the new tracker state: */
	states.postNewValue();
	}
