/***********************************************************************
PSMove - Class to represent a PlayStation Move game controller as an
inertially-tracked input device.
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

#include "PSMove.h"

#include <string.h>
#include <string>
#include <stdexcept>
#include <iostream>
#include <Misc/SizedTypes.h>
#include <Misc/FunctionCalls.h>
#include <IO/File.h>
#include <IO/OpenFile.h>
#include <RawHID/BusType.h>
#include <Math/Constants.h>

/***********************
Methods of class PSMove:
***********************/

void PSMove::initialize(void)
	{
	/* Initialize the calibration data structure: */
	calibrationData.accelerometerFactor=Scalar(0.001);
	calibrationData.gyroscopeFactor=Scalar(0.001);
	calibrationData.magnetometer=true;
	calibrationData.magnetometerFactor=Scalar(0.001);
	calibrationData.timeStepFactor=Scalar(0.00001);
	
	/* Try loading calibration data from a calibration file: */
	std::string calibrationFileName="Calibration-PSMove-";
	calibrationFileName.append(RawHID::Device::getSerialNumber());
	try
		{
		IO::FilePtr calibFile=IO::openFile(calibrationFileName.c_str());
		loadCalibrationData(*calibFile);
		}
	catch(std::runtime_error)
		{
		/* Ignore the error and reset calibration data to the default: */
		calibrationData.accelerometerMatrix=Matrix::one;
		calibrationData.gyroscopeMatrix=Matrix::one;
		calibrationData.magnetometerMatrix=Matrix::one;
		}
	
	/* Negate the magnetometer's x and z axes: */
	for(int j=0;j<4;++j)
		{
		calibrationData.magnetometerMatrix(0,j)=-calibrationData.magnetometerMatrix(0,j);
		calibrationData.magnetometerMatrix(2,j)=-calibrationData.magnetometerMatrix(2,j);
		}
	
	/* Initialize the LED ball color: */
	for(int i=0;i<3;++i)
		ledColor[i]=0x00U;
	ledColorChanged=true;
	}

void* PSMove::samplingThreadMethod(void)
	{
	/* Enable thread cancellation: */
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	// Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	try
		{
		/* Read the first input report from the device's raw HID node to start sampling: */
		unsigned char getInputReport[49];
		getInputReport[0]=0x01U;
		readReport(getInputReport,sizeof(getInputReport));
		
		/* Initialize time step calculation and packet loss detection: */
		unsigned int lastTimeStamp=((unsigned int)(getInputReport[11])<<8)|(unsigned int)(getInputReport[43]);
		unsigned int lastSequenceNumber=getInputReport[4]&0x0fU;
		
		/* Create an update timer to refresh the LED ball color at regular intervals: */
		int timeToLedUpdate=0;
		
		/* Read from the device's raw HID node until interrupted: */
		RawSample rawSample;
		while(keepSampling)
			{
			if(ledColorChanged||timeToLedUpdate<=0)
				{
				/* Send a setLED report: */
				unsigned char setLedReport[49];
				memset(setLedReport,0,sizeof(setLedReport));
				setLedReport[0]=0x02U;
				for(int i=0;i<3;++i)
					setLedReport[2+i]=ledColor[i];
				setLedReport[5]=0U;
				setLedReport[6]=0U;
				writeReport(setLedReport,sizeof(setLedReport));
				
				/* Send another LED update in 2 seconds: */
				if(ledColorChanged)
					timeToLedUpdate=200000;
				else
					timeToLedUpdate+=200000;
				ledColorChanged=false;
				}
			
			/* Read the next input report: */
			getInputReport[0]=0x01U;
			readReport(getInputReport,sizeof(getInputReport));
			
			/*******************************************************************
			Create one sample callback structure for each of the two samples
			reported in a single input report. Use the time stamps in the input
			reports to account for dropped packets or uneven sample rates.
			Assume that the first sample comes 1/177s before the second one.
			*******************************************************************/
			
			/* Calculate the number of lost packets: */
			unsigned int sequenceNumber=getInputReport[4]&0x0fU;
			unsigned int numLostPackets=(sequenceNumber-(lastSequenceNumber+1))&0x0fU;
			
			/* Calculate the time difference: */
			unsigned int timeStamp=((unsigned int)(getInputReport[11])<<8)|(unsigned int)(getInputReport[43]);
			int fullTimeStep=int((timeStamp-lastTimeStamp)&0xffffU);
			
			/* Fill in the raw sample callback structure for the first half-sample: */
			for(int i=0;i<3;++i)
				rawSample.accelerometer[i]=((int(getInputReport[14+i*2])<<8)|int(getInputReport[13+i*2]))-0x8000;
			for(int i=0;i<3;++i)
				rawSample.gyroscope[i]=((int(getInputReport[26+i*2])<<8)|int(getInputReport[25+i*2]))-0x8000;
			rawSample.magnetometer[0]=((int(getInputReport[38])<<8)|int(getInputReport[39]))&0x0fff;
			rawSample.magnetometer[1]=((int(getInputReport[40])<<4)|(int(getInputReport[41])>>4))&0x0fff;
			rawSample.magnetometer[2]=((int(getInputReport[41])<<8)|int(getInputReport[42]))&0x0fff;
			for(int i=0;i<3;++i)
				if(rawSample.magnetometer[i]&0x0800)
					rawSample.magnetometer[i]|=~int(0x0fff);
			rawSample.timeStep=fullTimeStep-565; // First half-sample occurs 1/2 average time step (1130us) before second
			
			/* Call the sample callback with the first half-sample: */
			sendSample(rawSample);
			
			/* Fill in the raw sample callback structure for the second half-sample: */
			for(int i=0;i<3;++i)
				rawSample.accelerometer[i]=((int(getInputReport[14+i*2])<<8)|int(getInputReport[13+i*2]))-0x8000;
			for(int i=0;i<3;++i)
				rawSample.gyroscope[i]=((int(getInputReport[26+i*2])<<8)|int(getInputReport[25+i*2]))-0x8000;
			
			/* Magnetometer data is only reported once per sample, so keep the previous values */
			
			rawSample.timeStep=565; // Second half-sample occurs 1/2 average time step (1130) after first
			
			/* Call the sample callback with the second half-sample: */
			sendSample(rawSample);
			
			/* Update sampling state for next sample: */
			lastSequenceNumber=sequenceNumber;
			lastTimeStamp=timeStamp;
			timeToLedUpdate-=fullTimeStep;
			}
		}
	catch(std::runtime_error err)
		{
		std::cerr<<"PSMove::samplingThreadMethod: Terminating due to exception "<<err.what()<<std::endl;
		}
	
	return 0;
	}

PSMove::PSMove(unsigned int deviceIndex)
	:RawHID::Device(RawHID::BUSTYPE_BLUETOOTH,0x054cU,0x03d5U,deviceIndex),
	 keepSampling(false)
	{
	initialize();
	}

PSMove::PSMove(const std::string& deviceSerialNumber)
	:RawHID::Device(RawHID::BUSTYPE_BLUETOOTH,0x054cU,0x03d5U,deviceSerialNumber),
	 keepSampling(false)
	{
	initialize();
	}

PSMove::~PSMove(void)
	{
	/* Shut down the background sampling thread if it is still active: */
	if(keepSampling)
		{
		keepSampling=false;
		samplingThread.join();
		}
	}

std::string PSMove::getSerialNumber(void) const
	{
	/* Prefix the HID device's serial number with the device class: */
	std::string result="PSMove-";
	result.append(RawHID::Device::getSerialNumber());
	return result;
	}

void PSMove::startStreamingRaw(IMU::RawSampleCallback* newRawSampleCallback)
	{
	/* Install the new raw sample callback: */
	IMU::startStreamingRaw(newRawSampleCallback);
	
	/* Start the background sampling thread: */
	keepSampling=true;
	samplingThread.start(this,&PSMove::samplingThreadMethod);
	}

void PSMove::startStreamingCalibrated(IMU::CalibratedSampleCallback* newCalibratedSampleCallback)
	{
	/* Install the new calibrated sample callback: */
	IMU::startStreamingCalibrated(newCalibratedSampleCallback);
	
	/* Start the background sampling thread: */
	keepSampling=true;
	samplingThread.start(this,&PSMove::samplingThreadMethod);
	}

void PSMove::stopStreaming(void)
	{
	if(!keepSampling)
		return;
	
	/* Shut down the background sampling thread: */
	keepSampling=false;
	samplingThread.join();
	
	/* Delete the streaming callback: */
	IMU::stopStreaming();
	}

void PSMove::setLedColor(unsigned char red,unsigned char green,unsigned char blue)
	{
	/* Set the new LED color and notify the sampling thread: */
	ledColor[0]=red;
	ledColor[1]=green;
	ledColor[2]=blue;
	ledColorChanged=true;
	}
