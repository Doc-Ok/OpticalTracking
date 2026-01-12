/***********************************************************************
OculusRift - Class to represent the tracking subsystem of an Oculus Rift
head-mounted display as an inertially-tracked input device.
Copyright (c) 2014-2018 Oliver Kreylos

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

#include <unistd.h>
#include <stdexcept>
#include <iostream>
#include <IO/File.h>
#include <IO/OpenFile.h>
#include <RawHID/BusType.h>

#include "TimeStampSource.h"
#include "OculusRiftHIDReports.h"

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
			break;
		
		case 0x0031U:
			deviceType=CV1;
			break;
		
		default:
			deviceType=UNKNOWN;
		}
	
	/* Read sensor measurement ranges: */
	SensorRange sensorRange;
	sensorRange.get(*this);
	
	/* Read display information: */
	DisplayInfo displayInfo;
	displayInfo.get(*this);
	
	/* Read sensor configuration: */
	SensorConfig sensorConfig;
	sensorConfig.get(*this);
	
	/* Set sensors to raw mode: */
	sensorConfig.flags|=SensorConfig::UseCalibFlags;
	sensorConfig.flags|=SensorConfig::AutoCalibFlags;
	sensorConfig.set(*this,0x0000U);
	
	/* Double-check sensor configuration: */
	sensorConfig.get(*this);
	
	/* Initialize other state: */
	opticalTracking=false;
	keepSampling=false;
	
	/* Initialize the calibration data structure: */
	calibrationData.magnetometer=true;
	
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
		initCalibrationData(getAccelerometerScale(),getGyroscopeScale(),getMagnetometerScale());
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

void* OculusRift::samplingThreadMethod(void)
	{
	/* Enable thread cancellation: */
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	// Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	try
		{
		/* Set the keep-alive interval for streaming to 10 seconds: */
		Misc::UInt16 keepAliveInterval=10000U;
		
		/*******************************************************************
		Warm-up period: Collect an initial set of samples to avoid bad time
		stamps at the beginning of the stream, and establish an initial
		offset between the Rift's internal clock and the CPU's wall clock.
		*******************************************************************/
		
		// DEBUGGING
		std::cout<<"OculusRift: Sending first keep-alive"<<std::endl;
		
		/* Send a keep-alive feature report to start streaming sample data: */
		if(deviceType==DK1)
			{
			KeepAliveDK1 ka(keepAliveInterval);
			ka.set(*this,0x0000U);
			}
		else
			{
			// KeepAliveDK2 ka(opticalTracking,keepAliveInterval);
			KeepAliveDK2 ka(false,keepAliveInterval);
			ka.set(*this,0x0000U);
			}
		
		/* Create an update timer to send keep-alive feature reports at regular intervals: */
		int timeToKeepAlive=int(keepAliveInterval)-1000;
		
		// DEBUGGING
		std::cout<<"OculusRift: Reading initial batch of input reports"<<std::endl;
		
		/* Read the first batch of input reports until raw time stamps stabilize: */
		SensorData sensorData;
		TimeStampSource timeStampSource(1000000,1000);
		temperature=0.0f;
		unsigned int numWarmupReports=0;
		for(int numReports=0;numReports<2;++numReports,++numWarmupReports)
			{
			/* Read the next input report and initialize the time stamp source: */
			sensorData.get(*this);
			timeStampSource.set();
			temperature=(temperature*float(numWarmupReports)+float(sensorData.temperature))/float(numWarmupReports+1);
			
			/* Check if the report was over-full: */
			if(sensorData.numSamples>3)
				{
				/* Wait for two more reports: */
				numReports=-1;
				}
			
			timeToKeepAlive-=int(sensorData.numSamples);
			}
		
		// DEBUGGING
		std::cout<<"OculusRift: Stabilizing time stamps"<<std::endl;
		
		/* Read some more input reports until the offset between raw and CPU time stamps stabilizes: */
		RawSample rawSamples[3];
		for(int i=0;i<3;++i)
			rawSamples[i].warmup=true;
		for(int numReports=0;numReports<10;++numReports,++numWarmupReports)
			{
			/* Read the next input report and advance the time stamp source: */
			unsigned int numRawSamples=sensorData.get(*this,rawSamples,timeStampSource);
			
			/* Adjust the running temperature average: */
			temperature=temperature*(1023.0f/1024.0f)+float(sensorData.temperature)*(1.0f/1024.0f);
			
			/* Send off each raw sample: */
			TimeStamp sampleTimeStamp=timeStampSource.get()-TimeStamp(sensorData.numSamples-1)*SensorData::sampleInterval;
			for(unsigned int sample=0;sample<numRawSamples;++sample,sampleTimeStamp+=SensorData::sampleInterval)
				{
				/* Attach a time stamp to this sample and send it: */
				rawSamples[sample].timeStamp=sampleTimeStamp;
				sendSample(rawSamples[sample]);
				}
			
			/* Prepare for the next input report: */
			timeToKeepAlive-=int(sensorData.numSamples);
			}
		
		/*******************************************************************
		Main tracking loop: Collect and distribute samples while keeping the
		two timers synchronized.
		*******************************************************************/
		
		// DEBUGGING
		std::cout<<"OculusRift: Starting sampling loop"<<std::endl;
		
		/* Process further samples in "regular mode" until interrupted: */
		for(int i=0;i<3;++i)
			rawSamples[i].warmup=false;
		while(keepSampling)
			{
			if(timeToKeepAlive<=0)
				{
				// DEBUGGING
				std::cout<<"OculusRift: Sending keep-alive"<<std::endl;
		
				/* Send a keep-alive feature report to start streaming sample data: */
				if(deviceType==DK1)
					{
					KeepAliveDK1 ka(keepAliveInterval);
					ka.set(*this,0x0000U);
					}
				else
					{
					// KeepAliveDK2 ka(opticalTracking,keepAliveInterval);
					KeepAliveDK2 ka(false,keepAliveInterval);
					ka.set(*this,0x0000U);
					}
				
				/* Reset the keep-alive timer: */
				timeToKeepAlive+=int(keepAliveInterval)-1000;
				}
			
			/* Read the next input report and advance the time stamp source: */
			unsigned int numRawSamples=sensorData.get(*this,rawSamples,timeStampSource);
			
			/* Adjust the running temperature average: */
			temperature=temperature*(1023.0f/1024.0f)+float(sensorData.temperature)*(1.0f/1024.0f);
			
			TimeStamp sampleTimeStamp=timeStampSource.get()-TimeStamp(sensorData.numSamples-1)*SensorData::sampleInterval;
			for(unsigned int sample=0;sample<numRawSamples;++sample,sampleTimeStamp+=SensorData::sampleInterval)
				{
				/* Attach a time stamp to this sample and send it: */
				rawSamples[sample].timeStamp=sampleTimeStamp;
				sendSample(rawSamples[sample]);
				}
			
			/* Prepare for the next input report: */
			timeToKeepAlive-=int(sensorData.numSamples);
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
		return busType==RawHID::BUSTYPE_USB&&vendorId==0x2833U&&(productId==0x0001U||productId==0x0021U||productId==0x0031U);
		}
	};

}

OculusRift::OculusRift(unsigned int deviceIndex)
	:RawHID::Device(OculusRiftMatcher(),deviceIndex)
	{
	initialize();
	
	#if 0
	
	/* Send a keep-alive feature report to start streaming sample data: */
	Misc::UInt16 keepAliveInterval=10000U;
	if(deviceType==DK1)
		{
		KeepAliveDK1 ka(keepAliveInterval);
		ka.set(*this,0x0000U);
		}
	else
		{
		// KeepAliveDK2 ka(opticalTracking,keepAliveInterval);
		KeepAliveDK2 ka(false,keepAliveInterval);
		ka.set(*this,0x0000U);
		}
	
	/* Create an update timer to send keep-alive feature reports at regular intervals: */
	int timeToKeepAlive=int(keepAliveInterval)-1000;
	
	/* Read 500 sample packets to clear any buffers etc.: */
	SensorData sensorData;
	for(int i=0;i<500;++i)
		{
		sensorData.get(*this);
		timeToKeepAlive-=sensorData.numSamples;
		}
	
	/* Start timing: */
	Realtime::TimePointMonotonic timer;
	unsigned int totalNumSamples=0;
	for(int i=0;i<100*500;++i)
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
				// KeepAliveDK2 ka(opticalTracking,keepAliveInterval);
				KeepAliveDK2 ka(false,keepAliveInterval);
				ka.set(*this,0x0000U);
				}
			timeToKeepAlive+=int(keepAliveInterval)-1000;
			}
		
		/* Read the next sample: */
		sensorData.get(*this);
		totalNumSamples+=sensorData.numSamples;
		timeToKeepAlive-=sensorData.numSamples;
		}
	double time=double(timer.setAndDiff());
	std::cout<<totalNumSamples<<" samples in "<<time<<"s, "<<time*1000000000.0/double(totalNumSamples)<<" ns/sample"<<std::endl;
	
	#endif
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
	
	if(deviceType==DK2)
		{
		#if 0
		
		/* Run the shutdown sequence of unknown semantics: */
		Unknown0x02 unknown0x02(0x01U);
		unknown0x02.get(*this);
		unknown0x02.value=0x13U;
		unknown0x02.set(*this,0x0000U);
		
		#endif
		}
	}

std::string OculusRift::getSerialNumber(void) const
	{
	/* Prefix the HID device's serial number with the device class: */
	std::string result="OculusRift-";
	result.append(RawHID::Device::getSerialNumber());
	return result;
	}

IMU::Scalar OculusRift::getAccelerometerScale(void) const
	{
	return Scalar(0.0001);
	}

IMU::Scalar OculusRift::getGyroscopeScale(void) const
	{
	return Scalar(0.0001);
	}

IMU::Scalar OculusRift::getMagnetometerScale(void) const
	{
	return Scalar(0.0001);
	}

void OculusRift::startStreamingRaw(IMU::RawSampleCallback* newRawSampleCallback)
	{
	// DEBUGGING
	std::cout<<"OculusRift: Starting raw streaming"<<std::endl;
	
	/* Install the new raw sample callback: */
	IMU::startStreamingRaw(newRawSampleCallback);
	
	/* Start the background sampling thread: */
	keepSampling=true;
	samplingThread.start(this,&OculusRift::samplingThreadMethod);
	}

void OculusRift::startStreamingCalibrated(IMU::CalibratedSampleCallback* newCalibratedSampleCallback)
	{
	// DEBUGGING
	std::cout<<"OculusRift: Starting calibrated streaming"<<std::endl;
	
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
	
	// DEBUGGING
	std::cout<<"OculusRift: Stopping streaming"<<std::endl;
	
	/* Shut down the background sampling thread: */
	keepSampling=false;
	samplingThread.join();
	
	/* Delete the streaming callback: */
	IMU::stopStreaming();
	}

void OculusRift::enableComponents(bool enableDisplay,bool enableAudio,bool enableLeds)
	{
	if(deviceType==CV1)
		{
		/* Switch components on or off: */
		ComponentStatus componentStatus(enableDisplay,enableAudio,enableLeds);
		componentStatus.set(*this,0x0000U);
		}
	}

void OculusRift::startOpticalTracking(void)
	{
	if((deviceType==DK2||deviceType==CV1)&&!opticalTracking)
		{
		// DEBUGGING
		std::cout<<"OculusRift: Turning on LEDs"<<std::endl;
		
		/* Turn on the LEDs: */
		usleep(16666);
		LEDControl ledControl;
		ledControl.get(*this);
		ledControl.pattern=deviceType==CV1?0xffU:0x00U;
		ledControl.enable=true;
		ledControl.autoIncrement=false;
		ledControl.useCarrier=true;
		ledControl.syncInput=false;
		ledControl.vsyncLock=false;
		ledControl.customPattern=false;
		ledControl.exposureLength=deviceType==CV1?399U:350U;
		ledControl.frameInterval=deviceType==CV1?19200U:16666U;
		ledControl.vsyncOffset=0U;
		ledControl.dutyCycle=127U;
		ledControl.set(*this,0x0000U);
		ledControl.get(*this);
		
		/* Remember that optical tracking is on to send the appropriate keep-alive report: */
		opticalTracking=true;
		}
	}

void OculusRift::stopOpticalTracking(void)
	{
	if((deviceType==DK2||deviceType==CV1)&&opticalTracking)
		{
		// DEBUGGING
		std::cout<<"OculusRift: Turning off LEDs"<<std::endl;
		
		/* Turn off the LEDs: */
		LEDControl ledControl;
		ledControl.get(*this);
		ledControl.pattern=deviceType==CV1?0xffU:0x00U;
		ledControl.enable=false;
		ledControl.autoIncrement=false;
		ledControl.useCarrier=false;
		ledControl.syncInput=false;
		ledControl.vsyncLock=false;
		ledControl.customPattern=false;
		ledControl.exposureLength=deviceType==CV1?399U:350U;
		ledControl.frameInterval=deviceType==CV1?19200U:16666U;
		ledControl.vsyncOffset=0U;
		ledControl.dutyCycle=127U;
		ledControl.set(*this,0x0000U);
		ledControl.get(*this);
		usleep(16666);
		
		/* Remember that optical tracking is off to send the appropriate keep-alive report: */
		opticalTracking=false;
		}
	}
