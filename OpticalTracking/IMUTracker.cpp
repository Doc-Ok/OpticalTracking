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

#include "IMUTracker.h"

// DEBUGGING
#include <iostream>
#include <Geometry/OutputOperators.h>

#include <stdexcept>
#include <Misc/FunctionCalls.h>

/***************************
Methods of class IMUTracker:
***************************/

IMUTracker::IMUTracker(const IMU& imu,unsigned int sStateBufferSize)
	:gravity(9.81),
	 magnetometer(imu.getCalibrationData().magnetometer),
	 useMagnetometer(magnetometer),
	 biasDriftGain(0),orientationDriftGain(0),
	 initialAccel(Vector::zero),initialGyro(Vector::zero),initialMag(Vector::zero),
	 numWarmupSamples(0),
	 gyroscopeBias(Vector::zero),
	 lastTimeStamp(0),
	 lastTranslation(Vector::zero),
	 trackingCallback(0),
	 stateBufferSize(sStateBufferSize),stateBuffer(new State[stateBufferSize]),
	 mostRecentState(0),lockedState(0)
	{
	/* Initialize the tracking state history buffer: */
	State* sPtr=stateBuffer;
	for(unsigned int i=0;i<stateBufferSize;++i,++sPtr)
		{
		sPtr->timeStamp=TimeStamp(0);
		sPtr->linearAcceleration=Vector::zero;
		sPtr->linearVelocity=Vector::zero;
		sPtr->translation=Vector::zero;
		sPtr->angularVelocity=Vector::zero;
		sPtr->rotation=Rotation::identity;
		}
	}

IMUTracker::~IMUTracker(void)
	{
	delete trackingCallback;
	delete[] stateBuffer;
	}

void IMUTracker::setGravity(Scalar newGravity)
	{
	gravity=newGravity;
	}

void IMUTracker::setUseMagnetometer(bool newUseMagnetometer)
	{
	/* Only use the magnetometer if the IMU has one: */
	useMagnetometer=newUseMagnetometer&&magnetometer;
	}

void IMUTracker::setBiasDriftGain(Scalar newBiasDriftGain)
	{
	biasDriftGain=newBiasDriftGain;
	}

void IMUTracker::setOrientationDriftGain(Scalar newOrientationDriftGain)
	{
	orientationDriftGain=newOrientationDriftGain;
	}

void IMUTracker::integrateSample(const IMU::CalibratedSample& sample)
	{
	if(sample.warmup)
		{
		/* Accumulate the initial acceleration, angular velocity, and magnetic flux vectors: */
		initialAccel+=sample.accelerometer;
		initialGyro+=sample.gyroscope;
		initialMag+=sample.magnetometer;
		++numWarmupSamples;
		
		/* Remember the first time stamp: */
		lastTimeStamp=sample.timeStamp;
		
		/* Create the initial tracking state: */
		State& initial=stateBuffer[mostRecentState];
		initial.timeStamp=sample.timeStamp;
		initial.linearAcceleration=Vector::zero;
		initial.linearVelocity=Vector::zero;
		initial.translation=Vector::zero;
		initial.angularVelocity=Vector::zero;
		if(useMagnetometer)
			{
			/* Align acceleration vector with +Z and magnetic flux density vector with +X: */
			Vector mag=initialMag;
			mag.orthogonalize(initialAccel);
			initial.rotation=Rotation::fromBaseVectors(mag,initialAccel);
			}
		else
			{
			/* Align acceleration vector with +Z: */
			Vector initialX(1,0,0);
			initialX.orthogonalize(initialAccel);
			initial.rotation=Rotation::fromBaseVectors(initialX,initialAccel);
			}
		initial.rotation*=Rotation::rotateX(Math::rad(Scalar(-90)));
		initial.rotation.doInvert();
		initial.rotation.renormalize();
		// std::cout<<"Initial orientation: "<<initial.rotation<<std::endl;
		
		/* Estimate the initial gyroscope bias: */
		gyroscopeBias=initialGyro/Scalar(numWarmupSamples);
		// std::cout<<"Gyro bias: "<<gyroscopeBias[0]<<", "<<gyroscopeBias[1]<<", "<<gyroscopeBias[2]<<std::endl;
		return;
		}
	
	{
	/* Lock the tracking state history buffer indices: */
	Threads::Spinlock::Lock indexLock(indexMutex);
	
	/* Get the current and next tracking states: */
	const State& current=stateBuffer[mostRecentState];
	unsigned int nextState=(mostRecentState+1)%stateBufferSize;
	State& next=stateBuffer[nextState];
	
	/* Calculate the current integration time step in seconds: */
	Scalar timeStep=Scalar(TimeStamp(sample.timeStamp-lastTimeStamp))*Scalar(1.0e-6);
	
	/* Set the next time stamp: */
	next.timeStamp=sample.timeStamp;
	
	// DEBUGGING
	// std::cout<<next.translation<<std::endl;
	
	/*****************************************************************************
	Calculate rotational state for next time point using improved Madgwick method:
	*****************************************************************************/
	
	/* Get the current orientation quaternion: */
	const Scalar* q=current.rotation.getQuaternion();
	
	/* Calculate the optimization target function for gravity correction (difference between estimated and measured gravity): */
	Scalar aLen=sample.accelerometer.mag();
	Scalar fgx=Scalar(2)*(q[0]*q[2]-q[1]*q[3])-sample.accelerometer[0]/aLen;
	Scalar fgy=Scalar(2)*(q[1]*q[2]+q[0]*q[3])-sample.accelerometer[1]/aLen;
	Scalar fgz=Scalar(2)*(Scalar(0.5)-q[0]*q[0]-q[1]*q[1])-sample.accelerometer[2]/aLen;
	
	/* Calculate the gradient descent step vector: */
	Scalar fnabla[4];
	
	/* Add the gravity correction component: */
	fnabla[0]=Scalar(2)*(q[2]*fgx+q[3]*fgy-Scalar(2)*q[0]*fgz);
	fnabla[1]=Scalar(2)*(q[2]*fgy-q[3]*fgx-Scalar(2)*q[1]*fgz);
	fnabla[2]=Scalar(2)*(q[0]*fgx+q[1]*fgy);
	fnabla[3]=Scalar(2)*(q[0]*fgy-q[1]*fgx);
	
	if(useMagnetometer)
		{
		/* Calculate the optimization target function for magnetic correction (angle between measured magnetic flux and (x, z) plane in sensor coordinates): */
		Scalar mLen2=sample.magnetometer.sqr();
		Scalar fb=(q[0]*q[1]+q[2]*q[3])*sample.magnetometer[0]+(Scalar(0.5)-q[0]*q[0]-q[2]*q[2])*sample.magnetometer[1]+(q[1]*q[2]-q[0]*q[3])*sample.magnetometer[2];
		
		/* Add the magnetic correction component: */
		Scalar magFactor=Scalar(4)*fb/mLen2;
		fnabla[0]+=(q[1]*sample.magnetometer[0]-Scalar(2)*q[0]*sample.magnetometer[1]-q[3]*sample.magnetometer[2])*magFactor;
		fnabla[1]+=(q[0]*sample.magnetometer[0]+q[2]*sample.magnetometer[2])*magFactor;
		fnabla[2]+=(q[3]*sample.magnetometer[0]-Scalar(2)*q[2]*sample.magnetometer[1]+q[1]*sample.magnetometer[2])*magFactor;
		fnabla[3]+=(q[2]*sample.magnetometer[0]-q[0]*sample.magnetometer[2])*magFactor;
		}
	
	Scalar fnablaLen=Math::sqrt(fnabla[0]*fnabla[0]+fnabla[1]*fnabla[1]+fnabla[2]*fnabla[2]+fnabla[3]*fnabla[3]);
	
	/* Transform the gradient descent step vector to an angular velocity: */
	Vector dBias;
	dBias[0]=Scalar(2)*(q[3]*fnabla[0]-q[0]*fnabla[3]-q[1]*fnabla[2]+q[2]*fnabla[1]);
	dBias[1]=Scalar(2)*(q[3]*fnabla[1]-q[1]*fnabla[3]+q[0]*fnabla[2]-q[2]*fnabla[0]);
	dBias[2]=Scalar(2)*(q[3]*fnabla[2]-q[2]*fnabla[3]-q[0]*fnabla[1]+q[1]*fnabla[0]);
	
	/* Update the bias compensation vector: */
	Scalar biasIntegrationFactor=fnablaLen>Scalar(0)?biasDriftGain*timeStep/fnablaLen:Scalar(0);
	for(int i=0;i<3;++i)
		gyroscopeBias[i]+=dBias[i]*biasIntegrationFactor;
	
	/* Apply the bias compensation vector to the gyroscope sample: */
	Vector omega;
	for(int i=0;i<3;++i)
		omega[i]=sample.gyroscope[i]-gyroscopeBias[i];
	
	/* Set the next angular velocity: */
	next.angularVelocity=current.rotation.transform(omega);
	
	/* Calculate the quaternion derivative of applying the gyroscope measurement to the current orientation: */
	Scalar qdo[4];
	qdo[0]=Scalar(0.5)*(q[3]*omega[0]+q[1]*omega[2]-q[2]*omega[1]);
	qdo[1]=Scalar(0.5)*(q[3]*omega[1]-q[0]*omega[2]+q[2]*omega[0]);
	qdo[2]=Scalar(0.5)*(q[3]*omega[2]+q[0]*omega[1]-q[1]*omega[0]);
	qdo[3]=Scalar(-0.5)*(q[0]*omega[0]+q[1]*omega[1]+q[2]*omega[2]);
	
	/* Integrate the current orientation: */
	Scalar driftCorrectionFactor=fnablaLen>Scalar(0)?orientationDriftGain/fnablaLen:Scalar(0);
	Scalar qp[4];
	for(int i=0;i<4;++i)
		qp[i]=q[i]+(qdo[i]-fnabla[i]*driftCorrectionFactor)*timeStep;
	
	/* Set the next orientation (qp will be normalized): */
	next.rotation=Rotation::fromQuaternion(qp);
	
	/******************************************
	Calculate linear state for next time point:
	******************************************/
	
	/* Transform current linear acceleration from current tracker frame to global space and subtract gravity: */
	next.linearAcceleration=next.rotation.transform(sample.accelerometer);
	next.linearAcceleration[2]-=gravity;
	
	#if 1 // Euler integration
	
	/* Integrate linear acceleration twice to update the current position: */
	next.linearVelocity=current.linearVelocity+current.linearAcceleration*timeStep;
	next.translation=current.translation+current.linearVelocity*timeStep;
	
	#else // Verlet integration
	
	next.translation=current.translation*2.0-lastTranslation+next.linearAcceleration*(timeStep*timeStep);
	lastTranslation=current.translation;
	
	#endif
	
	/* Post the new tracker state: */
	mostRecentState=nextState;
	} // Release the tracking state history buffer lock
	
	/* Call the tracking callback if streaming: */
	if(trackingCallback!=0)
		(*trackingCallback)(stateBuffer[mostRecentState]);
	
	/* Prepare for the next sample: */
	lastTimeStamp=sample.timeStamp;
	}

void IMUTracker::startStreaming(IMUTracker::TrackingCallback* newTrackingCallback)
	{
	if(trackingCallback!=0)
		throw std::runtime_error("IMUTracker::startStreaming: Already streaming");
	
	/* Install the new tracking callback: */
	trackingCallback=newTrackingCallback;
	}

void IMUTracker::stopStreaming(void)
	{
	/* Delete the tracking callback: */
	delete trackingCallback;
	trackingCallback=0;
	}

const IMUTracker::State& IMUTracker::getRecentState(TimeStamp timeStamp) const
	{
	/* Lock the tracking state history buffer indices: */
	Threads::Spinlock::Lock indexLock(indexMutex);
	
	/* Perform a binary search on the tracking state history buffer relative to the most recent time stamp: */
	unsigned int l=mostRecentState;
	TimeStamp tsBase=stateBuffer[l].timeStamp;
	timeStamp-=tsBase;
	++l;
	unsigned int r=l+stateBufferSize;
	while(r-l>1)
		{
		unsigned int m=(l+r)/2;
		TimeStamp ts=stateBuffer[m%stateBufferSize].timeStamp-tsBase;
		if(ts>=timeStamp)
			r=m;
		else
			l=m;
		}
	
	return stateBuffer[l%stateBufferSize];
	}

void IMUTracker::applyCorrection(const IMUTracker::Vector& positionDelta,const IMUTracker::Vector& velocityDelta)
	{
	/* Lock the tracking state history buffer indices: */
	Threads::Spinlock::Lock indexLock(indexMutex);
	
	/* Modify the position and linear velocity of the most recent tracking state: */
	stateBuffer[mostRecentState].translation+=positionDelta;
	stateBuffer[mostRecentState].linearVelocity+=velocityDelta;
	}

void IMUTracker::restart(void)
	{
	/* Lock the tracking state history buffer indices: */
	Threads::Spinlock::Lock indexLock(indexMutex);
	
	/* Override the most recent tracking state: */
	stateBuffer[mostRecentState].linearAcceleration=Vector::zero;
	stateBuffer[mostRecentState].linearVelocity=Vector::zero;
	stateBuffer[mostRecentState].angularVelocity=Vector::zero;
	}

void IMUTracker::restart(const IMUTracker::Vector& translation)
	{
	/* Lock the tracking state history buffer indices: */
	Threads::Spinlock::Lock indexLock(indexMutex);
	
	/* Override the most recent tracking state: */
	stateBuffer[mostRecentState].linearAcceleration=Vector::zero;
	stateBuffer[mostRecentState].linearVelocity=Vector::zero;
	stateBuffer[mostRecentState].translation=translation;
	stateBuffer[mostRecentState].angularVelocity=Vector::zero;
	}

void IMUTracker::restart(const IMUTracker::Vector& translation,const IMUTracker::Rotation& rotation)
	{
	/* Lock the tracking state history buffer indices: */
	Threads::Spinlock::Lock indexLock(indexMutex);
	
	/* Override the most recent tracking state: */
	stateBuffer[mostRecentState].linearAcceleration=Vector::zero;
	stateBuffer[mostRecentState].linearVelocity=Vector::zero;
	stateBuffer[mostRecentState].translation=translation;
	stateBuffer[mostRecentState].angularVelocity=Vector::zero;
	stateBuffer[mostRecentState].rotation=rotation;
	}
