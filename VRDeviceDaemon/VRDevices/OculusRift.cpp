/***********************************************************************
OculusRift - Class to represent the Oculus Rift HMD's built-in
orientation tracker.
Copyright (c) 2013-2014 Oliver Kreylos

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

#include <VRDeviceDaemon/VRDevices/OculusRift.h>

#include <iostream>
#include <iomanip>
#include <Geometry/OutputOperators.h>
#include <utility>
#include <Misc/ThrowStdErr.h>
#include <Misc/Timer.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <IO/File.h>
#include <IO/FixedMemoryFile.h>
#include <IO/OpenFile.h>
#include <libusb-1.0/libusb.h>
#include <USB/DeviceList.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/GeometryValueCoders.h>
#include <Vrui/Internal/VRDeviceDescriptor.h>

#include <VRDeviceDaemon/VRDeviceManager.h>

namespace {

/**************
Helper classes:
**************/

struct SensorRange // Structure to get and set the maximum value ranges of the Oculus Rift's three sensors
	{
	/* Elements: */
	public:
	static unsigned int accelRanges[4]; // Range scale factors supported by linear accelerometers
	static unsigned int gyroRanges[4]; // Range scale factors supported by rate gyroscopes
	static unsigned int magRanges[4]; // Range scale factors supported by magnetometers
	float accelFactor; // Scale factor to convert raw linear accelerometer measurements to m/(s^2)
	float gyroFactor; // Scale factor to convert raw rate gyroscope measurements to radians/s
	float magFactor; // Scale factor to convert raw magnetometer measurements to gauss
	
	/* Private methods: */
	private:
	static unsigned int findRangeMatch(float requested,float scale,const unsigned int* range,unsigned int rangeSize)
		{
		unsigned int value=(unsigned int)(Math::floor(requested/scale+0.5f));
		unsigned int i;
		for(i=0;i<rangeSize-1&&range[i]<value;++i)
			;
		return range[i];
		}
	
	/* Constructors and destructors: */
	public:
	SensorRange(void) // Creates maximum range values
		:accelFactor(float(accelRanges[3])*9.81f),
		 gyroFactor(float(gyroRanges[3])*Math::Constants<float>::pi/250.0f),
		 magFactor(float(magRanges[3])*0.001f)
		{
		}
	
	/* Methods: */
	void get(USB::Device& device) // Reads sensor ranges from given USB device
		{
		/* Set up the feature report packet buffer: */
		IO::FixedMemoryFile pktBuffer(8);
		pktBuffer.setEndianness(Misc::LittleEndian);
		Misc::UInt8* buf=static_cast<Misc::UInt8*>(pktBuffer.getMemory());
		memset(buf,0U,pktBuffer.getSize());
		buf[0]=0x04U;
		
		/* Read the sensor range feature report: */
		size_t readResult=device.readControl(LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE,
		                                     0x01U, // HID_GET_REPORT
		                                     (0x03U<<8)|buf[0], // HID_REPORT_TYPE_FEATURE
		                                     0x00U, // interface number
		                                     buf,
		                                     pktBuffer.getSize());
		if(readResult!=size_t(pktBuffer.getSize()))
			Misc::throwStdErr("SensorRange::get: Received %u bytes instead of %u",readResult,size_t(pktBuffer.getSize()));
		
		/* Unpack the packet buffer: */
		pktBuffer.skip<Misc::UInt8>(1); // Skip report ID
		pktBuffer.skip<Misc::UInt16>(1); // Skip command ID
		accelFactor=float(pktBuffer.read<Misc::UInt8>())*9.81f;
		gyroFactor=float(pktBuffer.read<Misc::UInt16>())*Math::Constants<float>::pi/250.0f;
		magFactor=float(pktBuffer.read<Misc::UInt16>())*0.001f;
		}
	void set(USB::Device& device) const // Writes sensor ranges to given USB device
		{
		/* Set up the feature report packet buffer: */
		IO::FixedMemoryFile pktBuffer(8);
		pktBuffer.setEndianness(Misc::LittleEndian);
		
		/* Pack the packet buffer: */
		pktBuffer.write<Misc::UInt8>(0x04U); // Report ID
		pktBuffer.write<Misc::UInt16>(0x0000U); // Command ID
		pktBuffer.write<Misc::UInt8>(findRangeMatch(accelFactor,9.81f,accelRanges,4));
		pktBuffer.write<Misc::UInt16>(findRangeMatch(gyroFactor,Math::Constants<float>::pi/250.0f,gyroRanges,4));
		pktBuffer.write<Misc::UInt16>(findRangeMatch(magFactor,0.001f,magRanges,4));
		
		/* Write the sensor range feature report: */
		Misc::UInt8* buf=static_cast<Misc::UInt8*>(pktBuffer.getMemory());
		device.writeControl(LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE,
		                    0x09U, // HID_SET_REPORT
		                    (0x03U<<8)|buf[0], // HID_REPORT_TYPE_FEATURE
		                    0x00U, // interface number
		                    buf,
		                    pktBuffer.getSize());
		}
	};

/*************************************
Static elements of struct SensorRange:
*************************************/

unsigned int SensorRange::accelRanges[4]={2,4,8,16};
unsigned int SensorRange::gyroRanges[4]={250,500,1000,2000};
unsigned int SensorRange::magRanges[4]={880,1300,1900,2500};

struct KeepAlive // Structure to query the Oculus Rift's keep-alive interval, and wake up the device
	{
	/* Elements: */
	public:
	unsigned int interval; // Keep-alive interval in ms
	
	/* Constructors and destructors: */
	KeepAlive(unsigned int sInterval =10000U)
		:interval(sInterval)
		{
		}
	
	/* Methods: */
	void get(USB::Device& device) // Reads keep-alive interval from given device
		{
		/* Set up the feature report packet buffer: */
		IO::FixedMemoryFile pktBuffer(5);
		pktBuffer.setEndianness(Misc::LittleEndian);
		Misc::UInt8* buf=static_cast<Misc::UInt8*>(pktBuffer.getMemory());
		memset(buf,0U,pktBuffer.getSize());
		buf[0]=0x08U;
		
		/* Read the keep-alive interval feature report: */
		size_t readResult=device.readControl(LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE,
		                                     0x01U, // HID_GET_REPORT
		                                     (0x03U<<8)|buf[0], // HID_REPORT_TYPE_FEATURE
		                                     0x00U, // interface number
		                                     buf,
		                                     pktBuffer.getSize());
		if(readResult!=size_t(pktBuffer.getSize()))
			Misc::throwStdErr("KeepAlive::get: Received %u bytes instead of %u",readResult,size_t(pktBuffer.getSize()));
		
		/* Unpack the packet buffer: */
		pktBuffer.skip<Misc::UInt8>(1); // Skip report ID
		pktBuffer.skip<Misc::UInt16>(1); // Skip command ID
		interval=pktBuffer.read<Misc::UInt16>();
		}
	void set(USB::Device& device) const // Writes keep-alive interval to given USB device
		{
		/* Set up the feature report packet buffer: */
		IO::FixedMemoryFile pktBuffer(5);
		pktBuffer.setEndianness(Misc::LittleEndian);
		
		/* Pack the packet buffer: */
		pktBuffer.write<Misc::UInt8>(0x08U); // Report ID
		pktBuffer.write<Misc::UInt16>(0x0000U); // Command ID
		pktBuffer.write<Misc::UInt16>(interval);
		
		/* Write the keep-alive interval feature report: */
		Misc::UInt8* buf=static_cast<Misc::UInt8*>(pktBuffer.getMemory());
		device.writeControl(LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE,
		                    0x09U, // HID_SET_REPORT
		                    (0x03U<<8)|buf[0], // HID_REPORT_TYPE_FEATURE
		                    0x00U, // interface number
		                    buf,
		                    pktBuffer.getSize());
		}
	};

struct SensorData // Structure to retrieve raw sensor measurements from the Oculus Rift
	{
	/* Embedded classes: */
	public:
	struct SensorSample
		{
		/* Elements: */
		public:
		int accel[3]; // Raw linear accelerometer measurements
		int gyro[3]; // Raw rate gyroscope measurements
		};
	
	/* Elements: */
	private:
	IO::FixedMemoryFile pktBuffer; // Buffer to unpack sensor data messages
	public:
	Vrui::VRDeviceState::TimeStamp arrivalTimeStamp; // Time stamp for this sample's arrival from USB
	unsigned int numSamples; // Number of samples in this packet (can be more than three, but only three are contained)
	Misc::UInt16 timeStamp; // Rolling index of first sample in this packet, to detect data loss
	unsigned int temperature; // Raw sensor temperature
	SensorSample samples[3]; // Up to three raw linear accelerometer and rate gyroscope samples
	int mag[3]; // Raw magnetic flux density sample
	
	/* Private methods: */
	private:
	static void unpackVector(const Misc::UInt8 raw[8],int vector[3])
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
	
	/* Constructors and destructors: */
	public:
	SensorData(void)
		:pktBuffer(64),
		 numSamples(0U),
		 timeStamp(0U),
		 temperature(0)
		{
		for(int i=0;i<3;++i)
			{
			for(int j=0;j<3;++j)
				samples[i].accel[j]=0;
			for(int j=0;j<3;++j)
				samples[i].gyro[j]=0;
			}
		for(int i=0;i<3;++i)
			mag[i]=0;
		}
	
	/* Methods: */
	void get(USB::Device& device) // Reads next sensor data packet from given USB device
		{
		/* Read next message from USB interrupt pipe: */
		size_t readSize=device.interruptTransfer(0x81U,static_cast<unsigned char*>(pktBuffer.getMemory()),size_t(pktBuffer.getSize()));
		if(readSize==62U)
			{
			/* Create an arrival-time time stamp: */
			Realtime::TimePointMonotonic now;
			arrivalTimeStamp=Vrui::VRDeviceState::TimeStamp(now.tv_sec*1000000+(now.tv_nsec+500)/1000);
			
			/* Unpack the message: */
			pktBuffer.setReadPosAbs(0);
			if(pktBuffer.read<Misc::UInt8>()==0x01U)
				{
				numSamples=pktBuffer.read<Misc::UInt8>();
				timeStamp=pktBuffer.read<Misc::UInt16>();
				pktBuffer.skip<Misc::UInt16>(1);
				temperature=pktBuffer.read<Misc::SInt16>();
				
				/* Read up to three sensor samples: */
				for(unsigned int sample=0;sample<numSamples&&sample<3;++sample)
					{
					/* Read the packed byte buffer containing the two vectors: */
					Misc::UInt8 bytes[16];
					pktBuffer.read<Misc::UInt8>(bytes,16);
					
					/* Unpack the two vectors: */
					unpackVector(bytes,samples[sample].accel);
					unpackVector(bytes+8,samples[sample].gyro);
					}
				
				/* Skip any unused samples: */
				for(unsigned int sample=numSamples;sample<3;++sample)
					pktBuffer.skip<Misc::UInt8>(16);
				
				/* Read magnetometer measurements: */
				for(int i=0;i<3;++i)
					mag[i]=pktBuffer.read<Misc::SInt16>();
				}
			}
		}
	};

}

/***************************
Methods of class OculusRift:
***************************/

void OculusRift::deviceThreadMethod(void)
	{
	/* Set the device's keep-alive interval to the default (10s): */
	KeepAlive ka;
	double keepAliveInterval=double(ka.interval)*0.001-1.0; // Send wake-up call 1 second before keep-alive interval expires
	
	/* Start receiving sensor data from the device: */
	ka.set(oculus);
	Misc::Timer sampleTimer;
	double nextKeepAliveTime=keepAliveInterval;
	
	/* Start with fast drift correction to quickly initialize the device's orientation; then back off: */
	Scalar dcw=driftCorrectionWeight*Scalar(100);
	int numFastSamples=1000; // Go back to slow drift correction after 1000 samples (1 second)
	
	/* Receive and process sensor data until interrupted: */
	unsigned int numProcessedSamples=0U;
	SensorData sensorData;
	while(keepRunning)
		{
		/* Check if the sensor needs waking up: */
		double now=sampleTimer.peekTime();
		if(now>=nextKeepAliveTime)
			{
			/* Send a keep-alive feature request: */
			ka.set(oculus);
			
			/* Advance the keep-alive timeout: */
			nextKeepAliveTime=now+keepAliveInterval;
			}
		
		/* Read a sensor message from the device and create a time stamp: */
		sensorData.get(oculus);
		
		/*******************************************************************
		Perform orientation integration and drift correction:
		*******************************************************************/
		
		/* Transform magnetic flux density from magnetometer frame to HMD frame: */
		Vector mag;
		for(int i=0;i<3;++i)
			mag[i]=magCorrect(i,0)*Scalar(sensorData.mag[0])
			      +magCorrect(i,1)*Scalar(sensorData.mag[1])
			      +magCorrect(i,2)*Scalar(sensorData.mag[2])
			      +magCorrect(i,3);
		
		/* Filter magnetic flux density: */
		for(int i=0;i<3;++i)
			currentMagneticFlux[i]=(currentMagneticFlux[i]*Scalar(15)+mag[i])/Scalar(16);
		
		/* Update the tracker state with all new samples: */
		unsigned int numSamples=sensorData.numSamples;
		if(numSamples>3)
			numSamples=3;
		for(unsigned int s=0;s<numSamples;++s)
			{
			/* Convert raw linera accelerometer measurements to m/s^2 and filter linear accelerations: */
			Vector accel;
			for(int i=0;i<3;++i)
				accel[i]=accelCorrect(i,0)*Scalar(sensorData.samples[s].accel[0])
				        +accelCorrect(i,1)*Scalar(sensorData.samples[s].accel[1])
				        +accelCorrect(i,2)*Scalar(sensorData.samples[s].accel[2])
				        +accelCorrect(i,3);
			currentLinearAcceleration=(currentLinearAcceleration*Scalar(15)+accel)/Scalar(16);
			
			/* Convert raw rate gyro measurements into an angular velocity vector in radians/s: */
			for(int i=0;i<3;++i)
				currentAngularVelocity[i]=Scalar(sensorData.samples[s].gyro[i])*Scalar(0.0001);
			
			/* Integrate the angular velocity into the current rotation using a fixed 1ms time step: */
			currentOrientation*=Rotation::rotateScaledAxis(currentAngularVelocity*Scalar(0.001));
			
			/* Only perform drift correction if the acceleration vector's magnitude is close to gravity: */
			Scalar accelLen2=accel.sqr();
			if(accelLen2>=Math::sqr(9.75)&&accelLen2<=Math::sqr(9.85))
				{
				if(useMagnetometer)
					{
					/* Transform linear acceleration and magnetic flux density vectors from HMD frame to current global frame: */
					Vector gAccel=currentOrientation.transform(accel);
					Vector gMag=currentOrientation.transform(mag);
					
					/* Build a coordinate frame in global space: */
					gMag.orthogonalize(gAccel);
					Rotation globalFrame=Rotation::fromBaseVectors(gMag,gAccel);
					
					/* Nudge the current global frame towards the desired global frame, where x points north and y points up: */
					globalFrame.doInvert();
					Vector globalRotation=globalFrame.getScaledAxis();
					currentOrientation.leftMultiply(Rotation::rotateScaledAxis(globalRotation*dcw));
					}
				else
					{
					/* Transform linear acceleration from HMD frame to current global frame: */
					Vector gAccel=currentOrientation.transform(accel);
					
					/* Rotate the current orientation's Y axis towards the true vertical: */
					Rotation globalOffset=Rotation::rotateFromTo(gAccel,Vector(0,1,0));
					Vector globalRotation=globalOffset.getScaledAxis();
					currentOrientation.leftMultiply(Rotation::rotateScaledAxis(globalRotation*dcw));
					}
				}
			}
		currentOrientation.renormalize();
		
		if(numFastSamples>0)
			{
			/* Count back until it's time to return to slow drift correction: */
			numFastSamples-=numSamples;
			if(numFastSamples<=0)
				dcw=driftCorrectionWeight;
			}
		
		/* Check if it is time to send new tracker data to the device manager: */
		numProcessedSamples+=numSamples;
		if(numProcessedSamples>=updateRate)
			{
			if(reportEvents)
				{
				/* Turn the current orientation into a tracker state: */
				TrackerState ts;
				PositionOrientation::Rotation r=currentOrientation;
				ts.linearVelocity=r.transform(TrackerState::LinearVelocity((neckPivot-Point::origin)^currentAngularVelocity));
				ts.angularVelocity=r.transform(TrackerState::AngularVelocity(currentAngularVelocity));
				if(motionPredictionDelta!=Scalar(0))
					r*=Rotation::rotateScaledAxis(currentAngularVelocity*motionPredictionDelta);
				ts.positionOrientation=PositionOrientation::rotateAround(neckPivot,r);
				
				/* Send the tracker state to the device manager: */
				setTrackerState(0,ts,sensorData.arrivalTimeStamp);
				}
			numProcessedSamples-=updateRate;
			}
		}
	}

namespace {

class OculusRiftDeviceMatcher // Helper class to match all supported models of Oculus Rift devices
	{
	/* Methods: */
	public:
	bool operator()(const libusb_device_descriptor& descriptor) const
		{
		return descriptor.idVendor==0x2833U&&(descriptor.idProduct==0x0001U||descriptor.idProduct==0x0021U);
		}
	};

}

OculusRift::OculusRift(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile)
	:VRDevice(sFactory,sDeviceManager,configFile),
	 deviceModel(UNKNOWN),
	 accelCorrect(1),magCorrect(1),
	 neckPivot(0.0,-6.0,8.0),
	 driftCorrectionWeight(0.0001),useMagnetometer(true),
	 motionPredictionDelta(0),
	 updateRate(10U),
	 reportEvents(false),
	 currentOrientation(Rotation::identity),
	 currentLinearAcceleration(Vector::zero),
	 currentAngularVelocity(Vector::zero),
	 currentMagneticFlux(Vector::zero),
	 nextTimeStamp(0U),
	 keepRunning(true)
	{
	/* Set device configuration: */
	setNumTrackers(1,configFile);
	
	/* Open the requested connected Oculus Rift device: */
	{
	USB::DeviceList deviceList(usbContext);
	if(configFile.hasTag("./deviceSerialNumber"))
		{
		/* Open an Oculus Rift device by serial number: */
		std::string serialNumber=configFile.retrieveString("./deviceSerialNumber");
		size_t numOculusDevices=deviceList.getNumDevices(OculusRiftDeviceMatcher());
		for(size_t i=0;i<numOculusDevices;++i)
			{
			USB::Device tempOculus=deviceList.getDevice(OculusRiftDeviceMatcher(),i);
			if(tempOculus.getSerialNumber()==serialNumber)
				{
				oculus=tempOculus;
				break;
				}
			}
		if(!oculus.isValid())
			Misc::throwStdErr("OculusRift::OculusRift: Oculus Rift device with serial number %s not found",serialNumber.c_str());
		
		/* Determine the device model: */
		switch(oculus.getVendorProductId().productId)
			{
			case 0x0001U:
				deviceModel=DK1;
				break;
			
			case 0x0021U:
				deviceModel=DK2;
				break;
			
			default:
				Misc::throwStdErr("OculusRift::OculusRift: Oculus Rift device with serial number %s has unsupported product ID %u",serialNumber.c_str(),oculus.getVendorProductId().productId);
			}
		}
	else
		{
		/* Open an Oculus Rift device by enumeration index: */
		int oculusIndex=configFile.retrieveValue<int>("./deviceIndex",0);
		oculus=deviceList.getDevice(OculusRiftDeviceMatcher(),oculusIndex);
		if(!oculus.isValid())
			Misc::throwStdErr("OculusRift::OculusRift: Oculus Rift device %d not found",oculusIndex);
		
		/* Determine the device model: */
		switch(oculus.getVendorProductId().productId)
			{
			case 0x0001U:
				deviceModel=DK1;
				break;
			
			case 0x0021U:
				deviceModel=DK2;
				break;
			
			default:
				Misc::throwStdErr("OculusRift::OculusRift: Oculus Rift device %d has unsupported product ID %u",oculusIndex,oculus.getVendorProductId().productId);
			}
		}
	}
	
	/* Open the device and claim the first (and only) interface: */
	oculus.open();
	// oculus.reset();
	// oculus.setConfiguration(1);
	oculus.claimInterface(0,true); // Disconnect the kernel's generic HID driver
	
	#ifdef VERBOSE
	std::cout<<"OculusRift: Connected to Oculus Rift ";
	switch(deviceModel)
		{
		case DK1:
			std::cout<<"DK1";
			break;
		
		case DK2:
			std::cout<<"DK2";
			break;
		
		default:
			std::cout<<"(unkown)";
		}
	std::cout<<" with serial number "<<oculus.getSerialNumber()<<std::endl;
	#endif
	
	/* Check if there is a binary calibration data file for the opened Oculus Rift device: */
	try
		{
		/* Assemble the calibration file name based on the device's serial number: */
		std::string calibFileName=VRDEVICEDAEMON_CONFIG_CONFIGDIR;
		calibFileName.append("/OculusRift-");
		calibFileName.append(oculus.getSerialNumber());
		calibFileName.append(".calib");
		
		/* Read the binary calibration data file: */
		IO::FilePtr calibFile=IO::openFile(calibFileName.c_str());
		#ifdef VERBOSE
		std::cout<<"OculusRift: Loading calibration data from "<<calibFileName<<std::endl;
		#endif
		calibFile->setEndianness(Misc::LittleEndian);
		for(int i=0;i<3;++i)
			for(int j=0;j<4;++j)
				accelCorrect(i,j)=Scalar(calibFile->read<Misc::Float64>());
		for(int i=0;i<3;++i)
			for(int j=0;j<4;++j)
				magCorrect(i,j)=Scalar(calibFile->read<Misc::Float64>());
		}
	catch(std::runtime_error)
		{
		/* Read the accelerometer and magnetometer correction matrices: */
		accelCorrect=configFile.retrieveValue<Correction>("./accelCorrection",accelCorrect);
		magCorrect=configFile.retrieveValue<Correction>("./magCorrection",magCorrect);
		
		/* Scale the correction matrices to convert from raw integer to floating-point measurements: */
		accelCorrect*=Scalar(0.0001);
		magCorrect*=Scalar(0.0001);
		
		if(deviceModel==DK1)
			{
			/* Flip the last two rows of the magnetometer correction matrix to transform to HMD frame: */
			for(int j=0;j<4;++j)
				std::swap(magCorrect(1,j),magCorrect(2,j));
			}
		}
	
	neckPivot=configFile.retrieveValue<Point>("./neckPivot",neckPivot);
	driftCorrectionWeight=configFile.retrieveValue<Scalar>("./driftCorrectionWeight",driftCorrectionWeight);
	useMagnetometer=configFile.retrieveValue<bool>("./useMagnetometer",useMagnetometer);
	motionPredictionDelta=configFile.retrieveValue<Scalar>("./motionPredictionDelta",motionPredictionDelta);
	updateRate=configFile.retrieveValue<unsigned int>("./updateRate",updateRate);
	
	/* Create a virtual device: */
	Vrui::VRDeviceDescriptor* vd=new Vrui::VRDeviceDescriptor(0,0);
	vd->name=configFile.retrieveString("./deviceName","OculusRift");
	vd->trackType=Vrui::VRDeviceDescriptor::TRACK_POS|Vrui::VRDeviceDescriptor::TRACK_DIR|Vrui::VRDeviceDescriptor::TRACK_ORIENT;
	vd->rayDirection=Vrui::VRDeviceDescriptor::Vector(0,1,0);
	vd->rayStart=0.0f;
	vd->trackerIndex=getTrackerIndex(0);
	addVirtualDevice(vd);
	
	/* Start device thread (its best to keep the tracker running at all times): */
	startDeviceThread();
	}

OculusRift::~OculusRift(void)
	{
	/* Stop device thread (its best to keep the tracker running at all times): */
	keepRunning=false;
	stopDeviceThread(false);
	}

void OculusRift::start(void)
	{
	/* Start reporting events to the device manager: */
	reportEvents=true;
	}

void OculusRift::stop(void)
	{
	/* Stop reporting events to the device manager: */
	reportEvents=false;
	}

/*************************************
Object creation/destruction functions:
*************************************/

extern "C" VRDevice* createObjectOculusRift(VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager,Misc::ConfigurationFile& configFile)
	{
	VRDeviceManager* deviceManager=static_cast<VRDeviceManager::DeviceFactoryManager*>(factoryManager)->getDeviceManager();
	return new OculusRift(factory,deviceManager,configFile);
	}

extern "C" void destroyObjectOculusRift(VRDevice* device,VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager)
	{
	delete device;
	}
