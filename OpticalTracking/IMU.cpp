/***********************************************************************
IMU - Abstract base class for inertial measurement units.
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

#include "IMU.h"

// DEBUGGING
#include <iostream>
#include <Geometry/OutputOperators.h>

#include <Misc/SizedTypes.h>
#include <Misc/FunctionCalls.h>
#include <Misc/ThrowStdErr.h>
#include <IO/File.h>

/********************
Methods of class IMU:
********************/

void IMU::loadCalibrationData(IO::File& calibrationFile)
	{
	calibrationFile.setEndianness(Misc::LittleEndian);
	
	/* Read the accelerometer matrix: */
	for(int i=0;i<3;++i)
		for(int j=0;j<4;++j)
			calibrationData.accelerometerMatrix(i,j)=Scalar(calibrationFile.read<Misc::Float64>());
	
	// DEBUGGING
	// std::cout<<"Accelerometer matrix: "<<calibrationData.accelerometerMatrix<<std::endl;
	
	/* Read the gyroscope matrix: */
	for(int i=0;i<3;++i)
		for(int j=0;j<4;++j)
			calibrationData.gyroscopeMatrix(i,j)=Scalar(calibrationFile.read<Misc::Float64>());
	
	// DEBUGGING
	// std::cout<<"Gyroscope matrix: "<<calibrationData.gyroscopeMatrix<<std::endl;
	
	if(calibrationData.magnetometer)
		{
		/* Read the magnetometer matrix: */
		for(int i=0;i<3;++i)
			for(int j=0;j<4;++j)
				calibrationData.magnetometerMatrix(i,j)=Scalar(calibrationFile.read<Misc::Float64>());
		
		// DEBUGGING
		// std::cout<<"Magnetometer matrix: "<<calibrationData.magnetometerMatrix<<std::endl;
		}
	}

void IMU::sendSample(const IMU::RawSample& rawSample)
	{
	if(rawSampleCallback!=0)
		{
		/* Send the raw sample: */
		(*rawSampleCallback)(rawSample);
		}
	else
		{
		/* Send a calibrated sample: */
		CalibratedSample calibratedSample;
		calibrationData.calibrate(rawSample,calibratedSample);
		(*calibratedSampleCallback)(calibratedSample);
		}
	}

IMU::IMU(void)
	:rawSampleCallback(0),
	 calibratedSampleCallback(0)
	{
	}

IMU::~IMU(void)
	{
	delete rawSampleCallback;
	delete calibratedSampleCallback;
	}

void IMU::startStreamingRaw(IMU::RawSampleCallback* newRawSampleCallback)
	{
	if(rawSampleCallback!=0||calibratedSampleCallback!=0)
		Misc::throwStdErr("IMU::startStreamingRaw: Streaming still active");
	if(newRawSampleCallback==0)
		Misc::throwStdErr("IMU::startStreamingRaw: No streaming callback provided");
	
	/* Set the new raw sample callback: */
	rawSampleCallback=newRawSampleCallback;
	}

void IMU::startStreamingCalibrated(IMU::CalibratedSampleCallback* newCalibratedSampleCallback)
	{
	if(rawSampleCallback!=0||calibratedSampleCallback!=0)
		Misc::throwStdErr("IMU::startStreamingCalibrated: Streaming still active");
	if(newCalibratedSampleCallback==0)
		Misc::throwStdErr("IMU::startStreamingCalibrated: No streaming callback provided");
	
	/* Set the new calibrated sample callback: */
	calibratedSampleCallback=newCalibratedSampleCallback;
	}

void IMU::stopStreaming(void)
	{
	/* Delete the current sample callbacks: */
	delete rawSampleCallback;
	rawSampleCallback=0;
	delete calibratedSampleCallback;
	calibratedSampleCallback=0;
	}
