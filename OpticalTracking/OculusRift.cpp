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

#include "OculusRift.h"

#include <stdexcept>
#include <iostream>
#include <IO/File.h>
#include <IO/OpenFile.h>
#include <RawHID/BusType.h>

#include "OculusRiftHIDReports.h"

// DEBUGGING
// #include <Misc/Timer.h>

/***************************
Methods of class OculusRift:
***************************/

void OculusRift::initialize(void)
	{
	/* Determine the device type: */
	switch(getProductId())
		{
		case 0x0001U:
			deviceType=DK1;
			break;
		
		case 0x0021U:
			deviceType=DK2;
		
		default:
			deviceType=UNKNOWN;
		}
	
	/* Initialize other state: */
	opticalTracking=false;
	keepSampling=false;
	
	/* Initialize the calibration data structure: */
	calibrationData.accelerometerFactor=Scalar(0.0001);
	calibrationData.gyroscopeFactor=Scalar(0.0001);
	calibrationData.magnetometer=true;
	calibrationData.magnetometerFactor=Scalar(0.0001);
	calibrationData.timeStepFactor=Scalar(0.001);
	
	/* Try loading calibration data from a calibration file: */
	std::string calibrationFileName="Calibration-OculusRift-";
	calibrationFileName.append(RawHID::Device::getSerialNumber());
	try
		{
		IO::FilePtr calibFile=IO::openFile(calibrationFileName.c_str());
		loadCalibrationData(*calibFile);
		}
	catch(std::runtime_error)
		{
		/* Ignore the error and reset calibration data to the default: */
		calibrationData.accelerometerMatrix=Matrix(Scalar(0.0001));
		calibrationData.gyroscopeMatrix=Matrix(Scalar(0.0001));
		calibrationData.magnetometerMatrix=Matrix(Scalar(0.0001));
		}
	
	if(deviceType==DK1)
		{
		/* Swap the last two rows of the magnetometer matrix to transform to HMD frame: */
		for(int j=0;j<4;++j)
			{
			double t=calibrationData.magnetometerMatrix(1,j);
			calibrationData.magnetometerMatrix(1,j)=calibrationData.magnetometerMatrix(2,j);
			calibrationData.magnetometerMatrix(2,j)=t;
			}
		}
	}

namespace {

/****************
Helper functions:
****************/

inline int unpackSInt16(const Misc::UInt8 raw[2]) // Unpacks a signed integer from 2 bytes
	{
	union // Helper union to assemble 2 bytes into a signed 16-bit integer
		{
		Misc::UInt8 b[2];
		Misc::SInt16 i;
		} p;
	
	/* Assemble the integer's components: */
	p.b[0]=raw[0];
	p.b[1]=raw[1];
	
	/* Return the signed integer: */
	return p.i;
	}

inline void unpackVector(const Misc::UInt8 raw[8],int vector[3]) // Unpacks a 3D vector from 8 bytes
	{
	union // Helper union to assemble 3 or 4 bytes into a signed 32-bit integer
		{
		Misc::UInt8 b[4];
		Misc::SInt32 i;
		} p;
	struct // Helper structure to sign-extend a 21-bit signed integer value
		{
		signed int si:21;
		} s;

	/* Assemble the vector's x component: */
	p.b[0]=raw[2];
	p.b[1]=raw[1];
	p.b[2]=raw[0];
	// p.b[3]=0U; // Not needed because it's masked out below anyway
	vector[0]=s.si=(p.i>>3)&0x001fffff;

	/* Assemble the vector's y component: */
	p.b[0]=raw[5];
	p.b[1]=raw[4];
	p.b[2]=raw[3];
	p.b[3]=raw[2];
	vector[1]=s.si=(p.i>>6)&0x001fffff;

	/* Assemble the vector's z component: */
	p.b[0]=raw[7];
	p.b[1]=raw[6];
	p.b[2]=raw[5];
	// p.b[3]=0U; // Not needed because it's masked out below anyway
	vector[2]=s.si=(p.i>>1)&0x001fffff;
	}

}

void* OculusRift::samplingThreadMethod(void)
	{
	/* Enable thread cancellation: */
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	// Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	// DEBUGGING
	// Misc::Timer packetTimer;
	
	try
		{
		/* Set the keep-alive interval for streaming to 10 seconds: */
		Misc::UInt16 keepAliveInterval=10000U;
		
		/* Create an update timer to send keep-alive feature reports at regular intervals: */
		int timeToKeepAlive=0;
		
		while(keepSampling)
			{
			if(timeToKeepAlive<=0)
				{
				/* Send a keep-alive feature report to start streaming sample data: */
				if(deviceType==DK1)
					{
					KeepAliveDK1 ka(keepAliveInterval);
					ka.set(*this,0x0000U);
					}
				else
					{
					KeepAliveDK2 ka(opticalTracking,keepAliveInterval);
					ka.set(*this,0x0000U);
					}
				
				/* Reset the keep-alive timer: */
				timeToKeepAlive+=int(keepAliveInterval)-1000;
				}
			
			/* Read the next input report: */
			unsigned char inputReport[62];
			memset(inputReport,0x00U,sizeof(inputReport));
			readReport(inputReport,sizeof(inputReport));
			Misc::UInt16 numSamples=Misc::UInt16(inputReport[1]);
			
			// DEBUGGING
			// std::cout<<packetTimer.peekTime()*1000.0<<", "<<numSamples<<std::endl;
			
			/* Process up to three samples included in the current input report: */
			RawSample rawSample;
			
			/* Read the magnetometer reading valid for all samples: */
			for(int i=0;i<3;++i)
				rawSample.magnetometer[i]=unpackSInt16(inputReport+56+i*2);
			
			/* Set the time step for all samples: */
			rawSample.timeStep=1;
			
			/* Unpack and send off each sample: */
			unsigned int sendSamples=numSamples;
			if(sendSamples>3)
				sendSamples=3;
			for(unsigned int sampleIndex=0;sampleIndex<sendSamples;++sampleIndex)
				{
				/* Read the accelerometer and gyroscope readings for all samples: */
				unpackVector(inputReport+8+sampleIndex*16,rawSample.accelerometer);
				unpackVector(inputReport+16+sampleIndex*16,rawSample.gyroscope);
				
				/* Send the sample: */
				sendSample(rawSample);
				}
			
			/* Prepare for the next sample: */
			timeToKeepAlive-=numSamples;
			}
		}
	catch(std::runtime_error err)
		{
		std::cerr<<"OculusRift::samplingThreadMethod: Terminating due to exception "<<err.what()<<std::endl;
		}
	
	return 0;
	}

namespace {

/**************
Helper classes:
**************/

class OculusRiftMatcher:public RawHID::Device::DeviceMatcher
	{
	/* Methods from RawHID::Device::DeviceMatcher: */
	public:
	virtual bool operator()(int busType,unsigned short vendorId,unsigned short productId) const
		{
		return busType==RawHID::BUSTYPE_USB&&vendorId==0x2833U&&(productId==0x0001U||productId==0x0021U);
		}
	};

}

OculusRift::OculusRift(unsigned int deviceIndex)
	:RawHID::Device(OculusRiftMatcher(),deviceIndex)
	{
	initialize();
	}

OculusRift::OculusRift(const std::string& deviceSerialNumber)
	:RawHID::Device(OculusRiftMatcher(),deviceSerialNumber)
	{
	initialize();
	}

OculusRift::~OculusRift(void)
	{
	/* Shut down the background sampling thread if it is still active: */
	if(keepSampling)
		{
		keepSampling=false;
		samplingThread.join();
		}
	}

std::string OculusRift::getSerialNumber(void) const
	{
	/* Prefix the HID device's serial number with the device class: */
	std::string result="OculusRift-";
	result.append(RawHID::Device::getSerialNumber());
	return result;
	}

void OculusRift::startStreamingRaw(IMU::RawSampleCallback* newRawSampleCallback)
	{
	/* Install the new raw sample callback: */
	IMU::startStreamingRaw(newRawSampleCallback);
	
	/* Start the background sampling thread: */
	keepSampling=true;
	samplingThread.start(this,&OculusRift::samplingThreadMethod);
	}

void OculusRift::startStreamingCalibrated(IMU::CalibratedSampleCallback* newCalibratedSampleCallback)
	{
	/* Install the new calibrated sample callback: */
	IMU::startStreamingCalibrated(newCalibratedSampleCallback);
	
	/* Start the background sampling thread: */
	keepSampling=true;
	samplingThread.start(this,&OculusRift::samplingThreadMethod);
	}

void OculusRift::stopStreaming(void)
	{
	/* Bail out if not streaming: */
	if(!keepSampling)
		return;
	
	/* Shut down the background sampling thread: */
	keepSampling=false;
	samplingThread.join();
	
	/* Delete the streaming callback: */
	IMU::stopStreaming();
	}

void OculusRift::startOpticalTracking(void)
	{
	if(deviceType==DK1&&!opticalTracking)
		{
		/* Run the initialization sequence of unknown semantics: */
		Unknown0x02 unknown0x02(0x01U);
		unknown0x02.get(*this);
		unknown0x02.value=0x01U;
		unknown0x02.set(*this,0x0000U);
		
		/* Turn on the LEDs: */
		LEDControl ledControl;
		ledControl.get(*this);
		ledControl.pattern=1; // This should have been 0, but I ran the wrong pattern when noting down LED IDs
		ledControl.enable=true;
		ledControl.autoIncrement=true;
		ledControl.useCarrier=true;
		ledControl.syncInput=false;
		ledControl.vsyncLock=false;
		ledControl.customPattern=false;
		ledControl.exposureLength=350U;
		ledControl.frameInterval=16666U;
		ledControl.vsyncOffset=0U;
		ledControl.dutyCycle=127U;
		ledControl.set(*this,0x0000U);
		
		/* Remember that optical tracking is on to send the appropriate keep-alive report: */
		opticalTracking=true;
		}
	}

void OculusRift::stopOpticalTracking(void)
	{
	if(deviceType==DK1&&opticalTracking)
		{
		/* Turn off the LEDs: */
		LEDControl ledControl;
		ledControl.get(*this);
		ledControl.pattern=0;
		ledControl.enable=false;
		ledControl.autoIncrement=false;
		ledControl.useCarrier=false;
		ledControl.syncInput=false;
		ledControl.vsyncLock=false;
		ledControl.customPattern=false;
		ledControl.exposureLength=350U;
		ledControl.frameInterval=16666U;
		ledControl.vsyncOffset=0U;
		ledControl.dutyCycle=127U;
		ledControl.set(*this,0x0000U);
		
		/* Run the shutdown sequence of unknown semantics: */
		Unknown0x02 unknown0x02(0x01U);
		unknown0x02.get(*this);
		unknown0x02.value=0x13U;
		unknown0x02.set(*this,0x0000U);
		
		/* Remember that optical tracking is off to send the appropriate keep-alive report: */
		opticalTracking=false;
		}
	}
