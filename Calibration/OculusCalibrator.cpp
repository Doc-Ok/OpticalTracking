/***********************************************************************
OculusCalibrator - Simple utility to retrieve calibration data from an Oculus
Rift head-mounted display, and visualize the 3-DOF and inertial 6-DOF
tracking driver's results.
Copyright (c) 2013-2014 Oliver Kreylos

This file is part of the Vrui calibration utility package.

The Vrui calibration utility package is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Vrui calibration utility package is distributed in the hope that it
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Vrui calibration utility package; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#define FITRAWMEASUREMENTS 1

#include <iostream>
#include <iomanip>
#include <Misc/SizedTypes.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/Timer.h>
#include <Misc/ChunkedArray.h>
#include <Threads/Thread.h>
#include <Threads/TripleBuffer.h>
#include <IO/SeekableFile.h>
#include <IO/FixedMemoryFile.h>
#include <libusb-1.0/libusb.h>
#include <USB/Context.h>
#include <USB/DeviceList.h>
#include <USB/Device.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#if FITRAWMEASUREMENTS
#include <Math/Matrix.h>
#endif
#include <Geometry/Vector.h>
#include <Geometry/Rotation.h>
#include <Geometry/OutputOperators.h>
#include <GL/gl.h>
#include <GL/GLMaterialTemplates.h>
#include <GL/GLModels.h>
#include <GL/GLGeometryWrappers.h>
#include <Vrui/Application.h>
#include <Vrui/OpenFile.h>

#include "../VRDeviceDaemon/Config.h"

/**************************************************************
Helper structures to communicate with the Oculus Rift over USB:
**************************************************************/

namespace {

struct DisplayInfo
	{
	/* Elements: */
	public:
	unsigned int distortionType;
	unsigned int screenResolution[2];
	float screenSize[2];
	float screenCenterY;
	float lensDistanceX;
	float eyePos[2];
	float distortionCoeffs[6];
	
	/* Constructors and destructors: */
	DisplayInfo(USB::Device& device) // Reads display info data from given USB device
		{
		/* Set display info to default values for 7" screen: */
		screenResolution[0]=1280;
		screenResolution[1]=800;
		screenSize[0]=0.14976f;
		screenSize[1]=0.0936f;
		screenCenterY=screenSize[1]*0.5f;
		lensDistanceX=0.064f;
		eyePos[0]=eyePos[1]=0.041f;
		distortionCoeffs[0]=1.0f;
		distortionCoeffs[1]=0.22f;
		distortionCoeffs[2]=0.24f;
		distortionCoeffs[3]=0.0f;
		distortionCoeffs[4]=0.0f;
		distortionCoeffs[5]=0.0f;
		
		/* Set up the feature report packet buffer: */
		IO::FixedMemoryFile pktBuffer(56);
		pktBuffer.setEndianness(Misc::LittleEndian);
		Misc::UInt8* buf=static_cast<Misc::UInt8*>(pktBuffer.getMemory());
		memset(buf,0U,pktBuffer.getSize());
		buf[0]=0x09U;
		
		/* Read the display info feature report: */
		size_t readResult=device.readControl(LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE,
		                                     0x01U, // HID_GET_REPORT
		                                     (0x03U<<8)|buf[0], // HID_REPORT_TYPE_FEATURE
		                                     0x00U, // interface number
		                                     buf,
		                                     pktBuffer.getSize());
		if(readResult!=size_t(pktBuffer.getSize()))
			Misc::throwStdErr("DisplayInfo::DisplayInfo: Received %u bytes instead of %u",readResult,size_t(pktBuffer.getSize()));
		
		/* Unpack the packet buffer: */
		pktBuffer.skip<Misc::UInt8>(1); // Skip report number
		pktBuffer.skip<Misc::UInt16>(1); // Skip command ID
		distortionType=pktBuffer.read<Misc::UInt8>();
		if((distortionType&0x0fU)>=0x01U)
			{
			/* Read basic screen geometry: */
			for(int i=0;i<2;++i)
				screenResolution[i]=pktBuffer.read<Misc::UInt16>();
			for(int i=0;i<2;++i)
				screenSize[i]=float(pktBuffer.read<Misc::UInt32>())/1000000.0f;
			screenCenterY=float(pktBuffer.read<Misc::UInt32>())/1000000.0f;
			lensDistanceX=float(pktBuffer.read<Misc::UInt32>())/1000000.0f;
			for(int i=0;i<2;++i)
				eyePos[i]=float(pktBuffer.read<Misc::UInt32>())/1000000.0f;
			if((distortionType&0x0fU)>=0x02U)
				{
				/* Read lens distortion correction coefficients: */
				for(int i=0;i<6;++i)
					distortionCoeffs[i]=pktBuffer.read<Misc::Float32>();
				}
			}
		}
	};

struct SensorRange
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

unsigned int SensorRange::accelRanges[4]={2,4,8,16};
unsigned int SensorRange::gyroRanges[4]={250,500,1000,2000};
unsigned int SensorRange::magRanges[4]={880,1300,1900,2500};

struct KeepAlive
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

struct SensorData
	{
	/* Embedded classes: */
	public:
	struct SensorSample
		{
		/* Elements: */
		public:
		int accel[3];
		int gyro[3];
		};
	
	/* Elements: */
	private:
	IO::FixedMemoryFile pktBuffer; // Buffer to unpack sensor data messages
	public:
	unsigned int numSamples;
	Misc::UInt16 timeStamp;
	unsigned int temperature;
	SensorSample samples[3];
	int mag[3];
	
	/* Private methods: */
	private:
	#if 0
	static void unpackVector(const Misc::UInt8 raw[8],int vector[3])
		{
		vector[0]=(Misc::UInt32(raw[0])<<13)|(Misc::UInt32(raw[1])<<5)|(Misc::UInt32(raw[2])>>3);
		if(vector[0]&0x00100000)
			vector[0]|=~0x001fffff;
		vector[1]=(Misc::UInt32(raw[2]&0x07U)<<18)|(Misc::UInt32(raw[3])<<10)|(Misc::UInt32(raw[4])<<2)|(Misc::UInt32(raw[5])>>6);
		if(vector[1]&0x00100000)
			vector[1]|=~0x001fffff;
		vector[2]=(Misc::UInt32(raw[5]&0x3fU)<<15)|(Misc::UInt32(raw[6])<<7)|(Misc::UInt32(raw[7])>>1);
		if(vector[2]&0x00100000)
			vector[2]|=~0x001fffff;
		}
	#else
	static void unpackVector(const Misc::UInt8 raw[8],int vector[3])
		{
		union
			{
			Misc::UInt8 b[4];
			Misc::SInt32 i;
			} p;
		struct
			{
			signed int si:21;
			} s;
		p.b[0]=raw[2];
		p.b[1]=raw[1];
		p.b[2]=raw[0];
		vector[0]=s.si=p.i>>3;
		p.b[0]=raw[5];
		p.b[1]=raw[4];
		p.b[2]=raw[3];
		p.b[3]=raw[2];
		vector[1]=s.si=(p.i>>6)&0x001fffff;
		p.b[0]=raw[7];
		p.b[1]=raw[6];
		p.b[2]=raw[5];
		vector[2]=s.si=(p.i>>1)&0x001fffff;
		}
	#endif
	
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
			/* Unpack the message: */
			pktBuffer.setReadPosAbs(0);
			if(pktBuffer.read<Misc::UInt8>()==0x01U)
				{
				numSamples=pktBuffer.read<Misc::UInt8>();
				timeStamp=pktBuffer.read<Misc::UInt16>();
				pktBuffer.skip<Misc::UInt16>(1);
				temperature=pktBuffer.read<Misc::SInt16>();
				for(unsigned int sample=0;sample<numSamples&&sample<3;++sample)
					{
					Misc::UInt8 bytes[16];
					pktBuffer.read<Misc::UInt8>(bytes,16);
					unpackVector(bytes,samples[sample].accel);
					unpackVector(bytes+8,samples[sample].gyro);
					}
				for(unsigned int sample=numSamples;sample<3;++sample)
					pktBuffer.skip<Misc::UInt8>(16);
				for(int i=0;i<3;++i)
					mag[i]=pktBuffer.read<Misc::SInt16>();
				}
			}
		}
	};

}

#if FITRAWMEASUREMENTS

namespace std {

/****************************************
Helper function to pretty-print a matrix:
****************************************/

inline ostream& operator<<(ostream& os,const Math::Matrix& m)
	{
	streamsize width=os.width();
	os<<setw(1)<<'/'<<setw(width)<<m(0,0);
	for(unsigned int j=1;j<m.getNumColumns();++j)
		os<<setw(2)<<", "<<setw(width)<<m(0,j);
	os<<setw(1)<<'\\'<<endl;
	for(unsigned int i=1;i<m.getNumRows()-1;++i)
		{
		os<<setw(1)<<'|'<<setw(width)<<m(i,0);
		for(unsigned int j=1;j<m.getNumColumns();++j)
			os<<setw(2)<<", "<<setw(width)<<m(i,j);
		os<<setw(1)<<'|'<<endl;
		}
	os<<setw(1)<<'\\'<<setw(width)<<m(m.getNumRows()-1,0);
	for(unsigned int j=1;j<m.getNumColumns();++j)
		os<<setw(2)<<", "<<setw(width)<<m(m.getNumRows()-1,j);
	os<<setw(1)<<'/'<<endl;
	return os;
	}

}

#endif

namespace {

/*****************************************************************
Class encapsulating an Oculus Rift inertial 6-DOF tracking driver:
*****************************************************************/

class Tracker
	{
	/* Embedded classes: */
	public:
	typedef float Scalar;
	typedef Geometry::Point<Scalar,3> Point;
	typedef Geometry::Vector<Scalar,3> Vector;
	typedef Geometry::Rotation<Scalar,3> Rotation;
	
	#if FITRAWMEASUREMENTS
	
	class RawRenderFunctor
		{
		/* Methods: */
		public:
		void operator()(const Vector* vectors,size_t numVectors)
			{
			glVertexPointer(3,GL_FLOAT,0,vectors);
			glDrawArrays(GL_POINTS,0,numVectors);
			}
		};
	
	#endif
	
	/* Elements: */
	private:
	Scalar accelCalib[3][4];
	Scalar magCalib[3][4];
	Scalar driftCorrectionWeight; // Weight factor for drift correction; good value is 0.0001
	Vector globalAccelAverage; // Long-term average global acceleration vector (approximately (0, 9.81, 0))
	Point currentPosition; // Current accumulated position
	Vector currentLinearVelocity; // Current accumulated linear velocity
	Rotation currentOrientation; // Current accumulated orientation
	Scalar linearAccelerationFilterWeight; // Filter weight for acceleration vector filtering
	Vector currentLinearAcceleration; // Current filtered linear acceleration vector
	Vector currentAngularVelocity; // Current filtered angular velocity vector
	Vector currentMagneticFlux; // Current filtered magnetic flux density vector
	Misc::UInt16 nextTimeStamp; // Expected time stamp of next sensor data packet
	SensorData::SensorSample lastSample; // Most recent linear accelerations and angular velocities
	#if FITRAWMEASUREMENTS
	Vector currentRawAccel,currentRawMag;
	volatile bool recordRawMeasurements; // Flag whether raw accelerometer and magnetometer measurements are collected
	Misc::ChunkedArray<Vector> rawAccels; // List of raw accelerometer measurements for calibration
	Misc::ChunkedArray<Vector> rawMags; // List of raw magnetometer measurements for calibration
	volatile bool rawMeasurementsDirty; // Flag whether there are raw measurements that have not yet been used for calibration
	#endif
	
	#if FITRAWMEASUREMENTS
	
	/* Private methods: */
	Math::Matrix fitQuadric(const Misc::ChunkedArray<Vector>& vectors) const
		{
		/* Create the least-squares system: */
		Math::Matrix a(10,10,0.0);
		
		/* Process all vectors: */
		for(Misc::ChunkedArray<Vector>::const_iterator vIt=vectors.begin();vIt!=vectors.end();++vIt)
			{
			/* Create the vector's associated linear equation: */
			double eq[10];
			eq[0]=(*vIt)[0]*(*vIt)[0];
			eq[1]=2.0*(*vIt)[0]*(*vIt)[1];
			eq[2]=2.0*(*vIt)[0]*(*vIt)[2];
			eq[3]=2.0*(*vIt)[0];
			eq[4]=(*vIt)[1]*(*vIt)[1];
			eq[5]=2.0*(*vIt)[1]*(*vIt)[2];
			eq[6]=2.0*(*vIt)[1];
			eq[7]=(*vIt)[2]*(*vIt)[2];
			eq[8]=2.0*(*vIt)[2];
			eq[9]=1.0;
			
			/* Insert the equation into the least-squares system: */
			for(unsigned int i=0;i<10;++i)
				for(unsigned int j=0;j<10;++j)
					a(i,j)+=eq[i]*eq[j];
			}
		
		/* Find the least-squares system's smallest eigenvalue: */
		std::pair<Math::Matrix,Math::Matrix> qe=a.jacobiIteration();
		unsigned int minEIndex=0;
		double minE=Math::abs(qe.second(0,0));
		for(unsigned int i=1;i<10;++i)
			{
			if(minE>Math::abs(qe.second(i,0)))
				{
				minEIndex=i;
				minE=Math::abs(qe.second(i,0));
				}
			}
		
		/* Create the quadric's defining matrices: */
		Math::Matrix qa(3,3);
		qa(0,0)=qe.first(0,minEIndex);
		qa(0,1)=qe.first(1,minEIndex);
		qa(0,2)=qe.first(2,minEIndex);
		qa(1,0)=qe.first(1,minEIndex);
		qa(1,1)=qe.first(4,minEIndex);
		qa(1,2)=qe.first(5,minEIndex);
		qa(2,0)=qe.first(2,minEIndex);
		qa(2,1)=qe.first(5,minEIndex);
		qa(2,2)=qe.first(7,minEIndex);
		Math::Matrix qb(3,1);
		qb(0)=qe.first(3,minEIndex);
		qb(1)=qe.first(6,minEIndex);
		qb(2)=qe.first(8,minEIndex);
		double qc=qe.first(9,minEIndex);
		
		/* Calculate the quadric's principal axes: */
		qe=qa.jacobiIteration();
		std::cout<<std::fixed<<std::setprecision(6);
		std::cout<<std::setw(9)<<qe.first<<std::endl;
		std::cout<<std::setw(9)<<qe.second<<std::endl<<std::endl;
		std::cout.unsetf(std::ios_base::floatfield);
		
		/* "Complete the square" to calculate the quadric's centroid and radii: */
		Math::Matrix qbp=qb.divideFullPivot(qe.first);
		Math::Matrix cp(3,1);
		for(int i=0;i<3;++i)
			cp(i)=-qbp(i)/qe.second(i);
		Math::Matrix c=qe.first*cp;
		std::cout<<"Centroid: "<<c(0)<<", "<<c(1)<<", "<<c(2)<<std::endl;
		double rhs=-qc;
		for(int i=0;i<3;++i)
			rhs+=Math::sqr(qbp(i))/qe.second(i);
		double radii[3];
		for(int i=0;i<3;++i)
			radii[i]=Math::sqrt(rhs/qe.second(i));
		std::cout<<"Radii: "<<radii[0]<<", "<<radii[1]<<", "<<radii[2]<<std::endl;
		Scalar averageRadius=Math::pow(radii[0]*radii[1]*radii[2],1.0/3.0);
		std::cout<<"Average radius: "<<averageRadius<<std::endl;
		
		/* Calculate the calibration matrix: */
		Math::Matrix ellP(4,4,1.0);
		for(int i=0;i<3;++i)
			for(int j=0;j<3;++j)
				ellP(i,j)=qe.first(i,j);
		Math::Matrix ellScale(4,4,1.0);
		for(int i=0;i<3;++i)
			ellScale(i,i)=averageRadius/radii[i];
		Math::Matrix ell=ellP;
		ell.makePrivate();
		for(int i=0;i<3;++i)
			ell(i,3)=c(i);
		Math::Matrix ellInv=ell.inverseFullPivot();
		Math::Matrix calib=ellP*ellScale*ellInv;
		
		/* Calculate the calibration residual: */
		double rms=0.0;
		for(Misc::ChunkedArray<Vector>::const_iterator vIt=vectors.begin();vIt!=vectors.end();++vIt)
			{
			Math::Matrix c(4,1);
			for(int i=0;i<3;++i)
				c(i)=(*vIt)[i];
			c(3)=1.0;
			Math::Matrix cc=calib*c;
			rms+=Math::sqr(Math::sqrt(Math::sqr(cc(0))+Math::sqr(cc(1))+Math::sqr(cc(2)))-averageRadius);
			}
		rms=Math::sqrt(rms/double(vectors.size()));
		std::cout<<"Calibration residual: "<<rms<<std::endl;
		
		return calib;
		}
	
	#endif
	
	/* Constructors and destructors: */
	public:
	Tracker(void)
		:driftCorrectionWeight(0.0001),
		 globalAccelAverage(0.0,9.8083,0.0),
		 currentPosition(Point::origin),currentLinearVelocity(Vector::zero),
		 currentOrientation(Rotation::identity),
		 linearAccelerationFilterWeight(15),currentLinearAcceleration(Vector::zero),
		 currentAngularVelocity(Vector::zero),
		 currentMagneticFlux(Vector::zero),
		 nextTimeStamp(0U)
		 #if FITRAWMEASUREMENTS
		,currentRawAccel(Vector::zero),currentRawMag(Vector::zero),
		 recordRawMeasurements(false),rawMeasurementsDirty(false)
		 #endif
		{
		for(int i=0;i<3;++i)
			lastSample.accel[i]=0;
		for(int i=0;i<3;++i)
			lastSample.gyro[i]=0;
		}
	~Tracker(void)
		{
		}
	
	/* Methods: */
	void loadCalibration(const std::string& calibFileName)
		{
		try
			{
			/* Load the accelerometer and magnetometer calibration matrices: */
			IO::FilePtr calibFile=IO::openFile(calibFileName.c_str());
			calibFile->setEndianness(Misc::LittleEndian);
			for(int i=0;i<3;++i)
				for(int j=0;j<4;++j)
					accelCalib[i][j]=Scalar(calibFile->read<Misc::Float64>());
			for(int i=0;i<3;++i)
				for(int j=0;j<4;++j)
					magCalib[i][j]=Scalar(calibFile->read<Misc::Float64>());
			}
		catch(std::runtime_error err)
			{
			/* Create a default calibration: */
			std::cerr<<"Error "<<err.what()<<" while loading calibration file "<<calibFileName<<"; generating default calibration"<<std::endl;
			for(int i=0;i<3;++i)
				for(int j=0;j<4;++j)
					accelCalib[i][j]=i==j?Scalar(0.0001):Scalar(0);
			for(int i=0;i<3;++i)
				for(int j=0;j<4;++j)
					magCalib[i][j]=i==j?Scalar(0.0001):Scalar(0);
			for(int j=0;j<4;++j)
				std::swap(magCalib[1][j],magCalib[2][j]);
			}
		
		#if 0
		
		std::cout<<"accelCorrection (";
		for(int j=0;j<4;++j)
			{
			if(j>0)
				std::cout<<", ";
			std::cout<<"("<<accelCalib[0][j]*Scalar(10000);
			for(int i=1;i<3;++i)
				std::cout<<", "<<accelCalib[i][j]*Scalar(10000);
			std::cout<<")";
			}
		std::cout<<")"<<std::endl;
		std::cout<<"magCorrection (";
		for(int j=0;j<4;++j)
			{
			if(j>0)
				std::cout<<", ";
			std::cout<<"("<<magCalib[0][j]*Scalar(10000);
			for(int i=1;i<3;++i)
				std::cout<<", "<<magCalib[i][j]*Scalar(10000);
			std::cout<<")";
			}
		std::cout<<")"<<std::endl;
		
		#endif
		}
	void setDriftCorrectionWeight(Scalar newDriftCorrectionWeight)
		{
		driftCorrectionWeight=newDriftCorrectionWeight;
		}
	void init(const SensorData& sensorData)
		{
		nextTimeStamp=sensorData.timeStamp+sensorData.numSamples;
		}
	void update(const SensorData& sensorData)
		{
		/* Check if any samples were missed since the last update: */
		Misc::UInt16 missingSamples=sensorData.timeStamp-nextTimeStamp;
		if(missingSamples!=0U)
			{
			/* Apply the last sample multiple times to make up for missing data: */
			/* On second thought, no, really don't: */
			// std::cout<<"Missed "<<missingSamples<<" samples of tracker data"<<std::endl;
			// ...
			}
		
		/* Transform magnetic flux density from magnetometer frame to HMD frame: */
		Vector mag;
		for(int i=0;i<3;++i)
			mag[i]=magCalib[i][0]*Scalar(sensorData.mag[0])+magCalib[i][1]*Scalar(sensorData.mag[1])+magCalib[i][2]*Scalar(sensorData.mag[2])+magCalib[i][3];
		#if FITRAWMEASUREMENTS
		currentRawMag=Vector(sensorData.mag[0],sensorData.mag[1],sensorData.mag[2]);
		if(recordRawMeasurements)
			rawMags.push_back(currentRawMag);
		#endif
		
		/* Filter magnetic flux density: */
		for(int i=0;i<3;++i)
			currentMagneticFlux[i]=(currentMagneticFlux[i]*Scalar(15)+mag[i])/Scalar(16);
		
		/* Update the tracker state with all new samples: */
		for(unsigned int s=0;s<3&&s<sensorData.numSamples;++s)
			{
			/* Filter linear accelerations: */
			Vector accel;
			for(int i=0;i<3;++i)
				{
				accel[i]=accelCalib[i][0]*Scalar(sensorData.samples[s].accel[0])+accelCalib[i][1]*Scalar(sensorData.samples[s].accel[1])+accelCalib[i][2]*Scalar(sensorData.samples[s].accel[2])+accelCalib[i][3];
				currentLinearAcceleration[i]=(currentLinearAcceleration[i]*linearAccelerationFilterWeight+accel[i])/(linearAccelerationFilterWeight+Scalar(1));
				}
			#if FITRAWMEASUREMENTS
			currentRawAccel=Vector(sensorData.samples[s].accel[0],sensorData.samples[s].accel[1],sensorData.samples[s].accel[2]);
			if(recordRawMeasurements)
				rawAccels.push_back(currentRawAccel);
			#endif
			
			/* Convert raw rate gyro measurements into an angular velocity vector in radians/s: */
			for(int i=0;i<3;++i)
				currentAngularVelocity[i]=Scalar(sensorData.samples[s].gyro[i])*Scalar(0.0001);
			
			/* Integrate the angular velocity into the current rotation using a fixed 1ms time step: */
			currentOrientation*=Rotation::rotateScaledAxis(currentAngularVelocity*Scalar(0.001));
			
			/* Transform linear acceleration and magnetic flux density vectors from HMD frame to current global frame: */
			Vector gAccel=currentOrientation.transform(accel);
			Vector gMag=currentOrientation.transform(mag);
			
			/* Subtract gravity from current linear acceleration and integrate twice to update the current position: */
			currentPosition+=currentLinearVelocity*Scalar(0.001);
			currentLinearVelocity+=(gAccel-globalAccelAverage)*Scalar(0.001);
			#if 0
			globalAccelAverage=(globalAccelAverage*Scalar(8191)+gAccel)/Scalar(8192);
			currentLinearVelocity*=Scalar(0.99);
			#endif
			
			/* Build a coordinate frame in global space where x points to magnetic north and y points up: */
			gMag.orthogonalize(gAccel);
			Rotation globalFrame=Rotation::fromBaseVectors(gMag,gAccel);
			
			/* Nudge the current global frame towards the desired global frame: */
			globalFrame.doInvert();
			Vector globalRotation=globalFrame.getScaledAxis();
			currentOrientation.leftMultiply(Rotation::rotateScaledAxis(globalRotation*driftCorrectionWeight));
			}
		currentOrientation.renormalize();
		
		/* Wrap current position to a box around the origin: */
		for(int i=0;i<3;++i)
			{
			while(currentPosition[i]>Scalar(10))
				currentPosition[i]-=Scalar(20);
			while(currentPosition[i]<Scalar(-10))
				currentPosition[i]+=Scalar(20);
			}
		
		/* Prepare for the next update: */
		nextTimeStamp=sensorData.timeStamp+sensorData.numSamples;
		if(sensorData.numSamples!=0U)
			lastSample=sensorData.samples[sensorData.numSamples-1];
		
		#if FITRAWMEASUREMENTS
		if(recordRawMeasurements)
			rawMeasurementsDirty=true;
		#endif
		}
	void resetPosition(void)
		{
		currentPosition=Point::origin;
		currentLinearVelocity=Vector::zero;
		}
	const Point& getPosition(void) const
		{
		return currentPosition;
		}
	const Rotation& getOrientation(void) const
		{
		return currentOrientation;
		}
	const Vector& getLinearAcceleration(void) const
		{
		return currentLinearAcceleration;
		}
	const Vector& getAngularVelocity(void) const
		{
		return currentAngularVelocity;
		}
	const Vector& getMagneticFlux(void) const
		{
		return currentMagneticFlux;
		}
	const Vector& getGlobalAccelerationAverage(void) const
		{
		return globalAccelAverage;
		}
	#if FITRAWMEASUREMENTS
	void setRecordRawMeasurements(bool newRecordRawMeasurements)
		{
		recordRawMeasurements=newRecordRawMeasurements;
		}
	void render(void) const
		{
		glPushAttrib(GL_ENABLE_BIT|GL_POINT_BIT);
		glDisable(GL_LIGHTING);
		glPointSize(3.0f);
		glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
		glEnableClientState(GL_VERTEX_ARRAY);
		
		glPushMatrix();
		glScaled(0.0002,0.0002,0.0002);
		
		RawRenderFunctor rrf;
		
		/* Render all acceleration measurements: */
		glColor3f(1.0f,0.0f,0.0f);
		rawAccels.forEachChunk(rrf);
		glBegin(GL_POINTS);
		glVertex(Point::origin+currentRawAccel);
		glEnd();
		
		glScaled(10.0,10.0,10.0);
		
		/* Render all magnetometer measurements: */
		glColor3f(0.0f,1.0f,0.0f);
		rawMags.forEachChunk(rrf);
		glBegin(GL_POINTS);
		glVertex(Point::origin+currentRawMag);
		glEnd();
		
		glPopMatrix();
		
		glPopClientAttrib();
		glPopAttrib();
		}
	bool isDirty(void) const
		{
		return rawMeasurementsDirty;
		}
	void updateCalibration(const std::string& calibFileName)
		{
		rawMeasurementsDirty=false;
		
		if(!rawAccels.empty()&&!rawMags.empty())
			{
			/* Fit a quadric to the raw accelerometer and magnetometer measurements: */
			std::cout<<"Accelerometer calibration:"<<std::endl;
			Math::Matrix accelCal=fitQuadric(rawAccels);
			
			/* Align the calibrated Y axis with the (0, 1, 0) vector: */
			Geometry::Vector<double,3> yAxis;
			for(int i=0;i<3;++i)
				yAxis[i]=accelCal(i,1);
			Geometry::Rotation<double,3> rot=Geometry::Rotation<double,3>::rotateFromTo(yAxis,Geometry::Vector<double,3>(0,1,0));
			
			/* Align the calibrated X axis with the z=0 plane: */
			Geometry::Vector<double,3> xAxis;
			for(int i=0;i<3;++i)
				xAxis[i]=accelCal(i,0);
			xAxis=rot.transform(xAxis);
			double xAngle=Math::atan2(xAxis[2],xAxis[0]);
			rot.leftMultiply(Geometry::Rotation<double,3>::rotateY(xAngle));
			rot.renormalize();
			Geometry::Matrix<double,3,3> rotMatTemp;
			rot.writeMatrix(rotMatTemp);
			Math::Matrix rotMat(3,3);
			for(int i=0;i<3;++i)
				for(int j=0;j<3;++j)
					rotMat(i,j)=rotMatTemp(i,j);
			accelCal=rotMat*accelCal;
			
			std::cout<<"accelCorrection (";
			for(int j=0;j<4;++j)
				{
				if(j>0)
					std::cout<<", ";
				std::cout<<"("<<accelCal(0,j);
				for(int i=1;i<3;++i)
					std::cout<<", "<<accelCal(i,j);
				std::cout<<")";
				}
			std::cout<<")"<<std::endl;
			std::cout<<std::endl<<"Magnetometer calibration:"<<std::endl;
			Math::Matrix magCal=fitQuadric(rawMags);
			std::cout<<"magCorrection (";
			for(int j=0;j<4;++j)
				{
				if(j>0)
					std::cout<<", ";
				std::cout<<"("<<magCal(0,j);
				for(int i=1;i<3;++i)
					std::cout<<", "<<magCal(i,j);
				std::cout<<")";
				}
			std::cout<<")"<<std::endl;
			
			/* Normalize the calibration matrices: */
			for(int i=0;i<3;++i)
				for(int j=0;j<4;++j)
					accelCal(i,j)*=0.0001;
			for(int i=0;i<3;++i)
				for(int j=0;j<4;++j)
					magCal(i,j)*=0.0001;
			
			/* Flip the last two rows of the magnetometer calibration matrix to align with HMD frame: */
			for(int j=0;j<4;++j)
				std::swap(magCal(1,j),magCal(2,j));
			
			/* Update the current tracker calibration matrices: */
			for(int i=0;i<3;++i)
				for(int j=0;j<4;++j)
					accelCalib[i][j]=Scalar(accelCal(i,j));
			for(int i=0;i<3;++i)
				for(int j=0;j<4;++j)
					magCalib[i][j]=Scalar(magCal(i,j));
			
			try
				{
				std::cout<<"Saving calibration data to file "<<calibFileName<<std::endl;
				
				/* Write the calibration matrices to a binary calibration file: */
				IO::FilePtr calibFile=IO::openFile(calibFileName.c_str(),IO::File::WriteOnly);
				calibFile->setEndianness(Misc::LittleEndian);
				for(int i=0;i<3;++i)
					for(int j=0;j<4;++j)
						calibFile->write<Misc::Float64>(accelCal(i,j));
				for(int i=0;i<3;++i)
					for(int j=0;j<4;++j)
						calibFile->write<Misc::Float64>(magCal(i,j));
				}
			catch(std::runtime_error err)
				{
				std::cerr<<"Error "<<err.what()<<" while writing calibration data to calibration file "<<calibFileName<<std::endl;
				}
			}
		}
	#endif
	};

}

/**********************
Main application class:
**********************/

class OculusCalibrator:public Vrui::Application
	{
	/* Embedded classes: */
	private:
	struct TrackerState
		{
		/* Elements: */
		public:
		Tracker::Point position; // Current position
		Tracker::Rotation orientation; // Current orientation
		Tracker::Vector acceleration; // Current acceleration vector
		Tracker::Vector magneticFlux; // Current magnetic flux vector
		Tracker::Vector gAccelAv; // Long-term average of acceleration vector in global space
		
		/* Constructors and destructors: */
		TrackerState(void)
			:position(Tracker::Point::origin),
			 orientation(Tracker::Rotation::identity),
			 acceleration(Tracker::Vector::zero),
			 magneticFlux(Tracker::Vector::zero),
			 gAccelAv(Tracker::Vector::zero)
			{
			}
		};
	
	/* Elements: */
	private:
	USB::Context usbContext;
	USB::Device oculus;
	Tracker tracker;
	Threads::Thread trackingThread;
	volatile bool receiveSamples;
	Threads::TripleBuffer<TrackerState> orientations;
	bool showRawMeasurements; // Flag whether to show raw measurements for calibration
	bool showTracker; // Flag whether to show the tracking data visualization
	bool lockPosition; // Flag whether to lock the tracker's position in the display
	
	/* Private methods: */
	void* trackingThreadMethod(void);
	
	/* Constructors and destructors: */
	public:
	OculusCalibrator(int& argc,char**& argv);
	virtual ~OculusCalibrator(void);
	
	/* Methods from Vrui::Application: */
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	virtual void eventCallback(EventID eventId,Vrui::InputDevice::ButtonCallbackData* cbData);
	};

void* OculusCalibrator::trackingThreadMethod(void)
	{
	/* Read the device's keep-alive interval: */
	KeepAlive ka;
	ka.get(oculus);
	double keepAliveInterval=double(ka.interval)*0.001-1.0; // Send next keep-alive packet one second before time runs out
	
	/* Start receiving sensor data from the device: */
	ka.set(oculus);
	Misc::Timer sampleTimer;
	double nextKeepAliveTime=keepAliveInterval;
	
	/* Receive the first sensor measurement to initialize the tracker: */
	SensorData sensorData;
	sensorData.get(oculus);
	tracker.init(sensorData);
	
	/* Start with fast drift correction to initialize the Oculus Rift's orientation: */
	tracker.setDriftCorrectionWeight(0.01);
	unsigned int slowDriftCountdown=1000U; // Switch back to slow drift correction after 1000 packets, i.e., one second
	
	/* Receive sensor data until interrupted: */
	unsigned int numSamples=0;
	while(receiveSamples)
		{
		/* Check if the sensor needs wakin' up: */
		double now=sampleTimer.peekTime();
		if(now>=nextKeepAliveTime)
			{
			/* Send a keep-alive feature request: */
			ka.set(oculus);
			
			/* Advance the keep-alive timeout: */
			nextKeepAliveTime=now+keepAliveInterval;
			}
		
		/* Read a sensor message from the device: */
		sensorData.get(oculus);
		
		/* Update the tracker: */
		tracker.update(sensorData);
		numSamples+=sensorData.numSamples;
		
		/* Update the Oculus' orientation on every tenth sample, i.e., at 100 Hz: */
		if(numSamples>=10U)
			{
			TrackerState& newTs=orientations.startNewValue();
			newTs.position=tracker.getPosition();
			newTs.orientation=tracker.getOrientation();
			newTs.acceleration=tracker.getLinearAcceleration();
			newTs.magneticFlux=tracker.getMagneticFlux();
			newTs.gAccelAv=tracker.getGlobalAccelerationAverage();
			orientations.postNewValue();
			numSamples-=10U;
			Vrui::requestUpdate();
			}
		
		if(slowDriftCountdown!=0U)
			{
			if(--slowDriftCountdown==0U)
				{
				/* Switch the tracker to slow drift correction: */
				tracker.setDriftCorrectionWeight(0.0001);
				}
			}
		}
	
	return 0;
	}

OculusCalibrator::OculusCalibrator(int& argc,char**& argv)
	:Vrui::Application(argc,argv),
	 showRawMeasurements(false),
	 showTracker(true),
	 lockPosition(true)
	{
	/* Parse the command line: */
	int oculusIndex=0;
	for(int i=1;i<argc;++i)
		{
		if(argv[i][0]=='-')
			{
			if(strcasecmp(argv[i]+1,"o")==0)
				{
				++i;
				if(i<argc)
					oculusIndex=atoi(argv[i]);
				else
					std::cerr<<"Ignoring dangling -o argument"<<std::endl;
				}
			}
		}
	
	/* Open one of the connected Oculus Rift devices: */
	{
	USB::DeviceList deviceList(usbContext);
	oculus=deviceList.getDevice(0x2833U,0x0001U,oculusIndex);
	if(!oculus.isValid())
		Misc::throwStdErr("Oculus Rift device with index %d not found",oculusIndex);
	}
	
	/* Open the device and claim the first (and only) interface: */
	oculus.open();
	// oculus.reset();
	// oculus.setConfiguration(1);
	oculus.claimInterface(0,true); // Disconnect the kernel's generic HID driver
	std::cout<<"Connected to Oculus Rift with serial number "<<oculus.getSerialNumber()<<std::endl;
	
	/* Load calibration data for the connected Oculus Rift: */
	std::string calibFileName=VRDEVICEDAEMON_CONFIG_CONFIGDIR;
	calibFileName.append("/OculusRift-");
	calibFileName.append(oculus.getSerialNumber());
	calibFileName.append(".calib");
	tracker.loadCalibration(calibFileName);
	
	/* Determine the conversion factor from meters to Vrui's physical coordinate unit: */
	double unitScale=Vrui::getMeterFactor();
	
	#if 1
	
	/* Read the device's display information data structure: */
	DisplayInfo di(oculus);
	std::cout<<"Display information: "<<std::endl;
	std::cout<<"  Distortion type             : "<<(unsigned int)(di.distortionType)<<std::endl;
	std::cout<<"  Screen resolution           : "<<di.screenResolution[0]<<" x "<<di.screenResolution[1]<<std::endl;
	std::cout<<"  Physical screen size        : "<<di.screenSize[0]*unitScale<<" x "<<di.screenSize[1]*unitScale<<std::endl;
	std::cout<<"  Vertical screen center      : "<<di.screenCenterY*unitScale<<std::endl;
	std::cout<<"  Lens center distance        : "<<di.lensDistanceX*unitScale<<std::endl;
	std::cout<<"  Screen/eye distance         : "<<di.eyePos[0]*unitScale<<", "<<di.eyePos[1]*unitScale<<std::endl;
	std::cout<<"  Lens correction coefficients: "<<di.distortionCoeffs[0]<<", "<<di.distortionCoeffs[1]<<", "<<di.distortionCoeffs[2]<<", "<<di.distortionCoeffs[3]<<", "<<di.distortionCoeffs[4]<<", "<<di.distortionCoeffs[5]<<", "<<std::endl;
	
	/* Write a configuration file fragment: */
	std::cout<<std::endl<<"Configuration file settings to paste into etc/OculusRift.cfg:"<<std::endl;
	std::cout<<std::endl;
	std::cout<<"  section LeftScreen"<<std::endl;
	std::cout<<"    name LeftScreen"<<std::endl;
	std::cout<<"    deviceMounted true"<<std::endl;
	std::cout<<"    deviceName OculusRift"<<std::endl;
	std::cout<<"    origin ("<<-di.screenSize[0]*unitScale*0.5<<", "<<di.eyePos[0]*unitScale<<", "<<-di.screenCenterY*unitScale<<")"<<std::endl;
	std::cout<<"    horizontalAxis (1.0, 0.0, 0.0)"<<std::endl;
	std::cout<<"    width "<<di.screenSize[0]*unitScale*0.5<<std::endl;
	std::cout<<"    verticalAxis (0.0, 0.0, 1.0)"<<std::endl;
	std::cout<<"    height "<<di.screenSize[1]*unitScale<<std::endl;
	std::cout<<"  endsection"<<std::endl;
	std::cout<<std::endl;
	std::cout<<"  section RightScreen"<<std::endl;
	std::cout<<"    name RightScreen"<<std::endl;
	std::cout<<"    deviceMounted true"<<std::endl;
	std::cout<<"    deviceName OculusRift"<<std::endl;
	std::cout<<"    origin ("<<0.0<<", "<<di.eyePos[0]*unitScale<<", "<<-di.screenCenterY*unitScale<<")"<<std::endl;
	std::cout<<"    horizontalAxis (1.0, 0.0, 0.0)"<<std::endl;
	std::cout<<"    width "<<di.screenSize[0]*unitScale*0.5<<std::endl;
	std::cout<<"    verticalAxis (0.0, 0.0, 1.0)"<<std::endl;
	std::cout<<"    height "<<di.screenSize[1]*unitScale<<std::endl;
	std::cout<<"  endsection"<<std::endl;
	std::cout<<std::endl;
	std::cout<<"  section HMDWindow"<<std::endl;
	std::cout<<"    windowPos (0, 0), ("<<di.screenResolution[0]<<", "<<di.screenResolution[1]<<")"<<std::endl;
	std::cout<<"    decorate false"<<std::endl;
	std::cout<<"    windowType SplitViewportStereo"<<std::endl;
	std::cout<<"    leftViewportPos (0, 0), ("<<di.screenResolution[0]/2<<", "<<di.screenResolution[1]<<")"<<std::endl;
	std::cout<<"    rightViewportPos ("<<di.screenResolution[0]/2<<", 0), ("<<di.screenResolution[0]/2<<", "<<di.screenResolution[1]<<")"<<std::endl;
	std::cout<<"    leftScreenName LeftScreen"<<std::endl;
	std::cout<<"    rightScreenName RightScreen"<<std::endl;
	std::cout<<"    viewerName HMDViewer"<<std::endl;
	if(di.distortionType==1)
		{
		std::cout<<"    lensCorrection true"<<std::endl;
		std::cout<<"    lcPoly ("<<di.distortionCoeffs[0];
		int lcPolyDegree=5;
		while(di.distortionCoeffs[lcPolyDegree]==0.0)
			--lcPolyDegree;
		for(int i=1;i<=lcPolyDegree;++i)
			std::cout<<", "<<di.distortionCoeffs[i];
		std::cout<<")"<<std::endl;
		std::cout<<"    leftLcCenter ("<<1.0-di.lensDistanceX/di.screenSize[0]<<", "<<di.screenCenterY/di.screenSize[1]<<")"<<std::endl;
		std::cout<<"    rightLcCenter ("<<di.lensDistanceX/di.screenSize[0]<<", "<<di.screenCenterY/di.screenSize[1]<<")"<<std::endl;
		std::cout<<"    mouseScreenName MouseScreen"<<std::endl;
		std::cout<<"  endsection"<<std::endl;
		}
	std::cout<<std::endl;
	
	#endif
	
	#if 0
	/* Read the device's sensor range data structure: */
	SensorRange sr;
	sr.get(oculus);
	std::cout<<std::endl<<"Sensor ranges: "<<std::endl;
	std::cout<<sr.accelFactor<<std::endl;
	std::cout<<sr.gyroFactor<<std::endl;
	std::cout<<sr.magFactor<<std::endl;
	#endif
	
	/* Start the background tracking thread: */
	receiveSamples=true;
	trackingThread.start(this,&OculusCalibrator::trackingThreadMethod);
	
	#if FITRAWMEASUREMENTS
	addEventTool("Show Raw Measurements",0,0);
	addEventTool("Start/Stop Recording",0,1);
	addEventTool("Update Calibration",0,2);
	#endif
	addEventTool("Show Tracking",0,3);
	addEventTool("Lock Position",0,4);
	addEventTool("Reset Position",0,5);
	addEventTool("Print Yaw Angle",0,6);
	
	Vrui::setNavigationTransformation(Vrui::Point(0,0,0),Vrui::Scalar(15),Vrui::Vector(0,1,0));
	}

OculusCalibrator::~OculusCalibrator(void)
	{
	/* Shut down the tracking thread: */
	receiveSamples=false;
	trackingThread.join();
	
	#if FITRAWMEASUREMENTS
	if(tracker.isDirty())
		{
		std::string calibFileName=VRDEVICEDAEMON_CONFIG_CONFIGDIR;
		calibFileName.append("/OculusRift-");
		calibFileName.append(oculus.getSerialNumber());
		calibFileName.append(".calib");
		tracker.updateCalibration(calibFileName);
		}
	#endif
	
	/* Close the Oculus device: */
	oculus.close();
	}

void OculusCalibrator::frame(void)
	{
	/* Lock any new tracking results: */
	orientations.lockNewValue();
	
	#if 0
	Tracker::Vector& m=orientations.getLockedValue().magneticFlux;
	std::cout.precision(5);
	std::cout<<'\r'<<std::setw(10)<<m<<"; "<<std::setw(10)<<Geometry::mag(m)<<"        "<<std::flush;
	#endif
	
	#if 0
	/* Print acceleration magnitude: */
	std::cout<<std::fixed;
	std::cout.precision(8);
	// std::cout<<'\r'<<std::setw(12)<<orientations.getLockedValue().gAccelAv<<"          "<<std::flush;
	std::cout<<'\r'<<std::setw(12)<<orientations.getLockedValue().acceleration<<"        "<<std::flush;
	std::cout.unsetf(std::ios_base::floatfield);
	#endif
	
	#if 0
	/* Print magnetic flux density magnitude: */
	std::cout<<'\r'<<Geometry::mag(orientations.getLockedValue().magneticFlux)<<"        "<<std::flush;
	#endif
	
	#if 0
	/* Calculate drift to original position: */
	Tracker::Vector globalGravity=orientations.getLockedValue().orientation.transform(orientations.getLockedValue().acceleration);
	std::cout<<'\r'<<Math::deg(Math::acos((globalGravity*Tracker::Vector(0,1,0))/Geometry::mag(globalGravity)))<<"        "<<std::flush;
	#endif
	}

void OculusCalibrator::display(GLContextData& contextData) const
	{
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
	glMaterialSpecular(GLMaterialEnums::FRONT,GLColor<GLfloat,4>(1.0f,1.0f,1.0f));
	glMaterialShininess(GLMaterialEnums::FRONT,25.0f);
	
	if(showTracker)
		{
		/* Draw a global coordinate frame: */
		glPushMatrix();
		glColor3f(1.0f,0.5f,0.5f);
		glRotated(90.0,0.0,1.0,0.0);
		glTranslated(0.0,0.0,5.0);
		glDrawArrow(0.5f,1.0f,1.5f,10.0f,16);
		glPopMatrix();
		
		glPushMatrix();
		glColor3f(0.5f,1.0f,0.5f);
		glRotated(-90.0,1.0,0.0,0.0);
		glTranslated(0.0,0.0,5.0);
		glDrawArrow(0.5f,1.0f,1.5f,10.0f,16);
		glPopMatrix();
		
		glPushMatrix();
		glColor3f(0.5f,0.5f,1.0f);
		glTranslated(0.0,0.0,5.0);
		glDrawArrow(0.5f,1.0f,1.5f,10.0f,16);
		glPopMatrix();
		
		/* Draw a local coordinate frame: */
		glPushMatrix();
		if(lockPosition)
			glTranslated(5.0,5.0,5.0);
		else
			glTranslate((orientations.getLockedValue().position-Tracker::Point::origin)*Tracker::Scalar(10));
		glRotate(orientations.getLockedValue().orientation);
		
		glPushMatrix();
		glColor3f(1.0f,0.5f,0.5f);
		glRotated(90.0,0.0,1.0,0.0);
		glTranslated(0.0,0.0,2.5);
		glDrawArrow(0.5f,1.0f,1.5f,5.0f,16);
		glPopMatrix();
		
		glPushMatrix();
		glColor3f(0.5f,1.0f,0.5f);
		glRotated(-90.0,1.0,0.0,0.0);
		glTranslated(0.0,0.0,2.5);
		glDrawArrow(0.5f,1.0f,1.5f,5.0f,16);
		glPopMatrix();
		
		glPushMatrix();
		glColor3f(0.5f,0.5f,1.0f);
		glTranslated(0.0,0.0,2.5);
		glDrawArrow(0.5f,1.0f,1.5f,5.0f,16);
		glPopMatrix();
		
		#if 0
		glPopMatrix();
		glPushMatrix();
		glRotate(orientations.getLockedValue().orientation);
		#endif
		
		#if 0
		glPopMatrix();
		#endif
		
		/* Draw the current linear acceleration vector: */
		glPushMatrix();
		glColor3f(1.0f,1.0f,0.0f);
		const Tracker::Vector& accel=orientations.getLockedValue().acceleration;
		GLfloat len=GLfloat(Geometry::mag(accel));
		glRotate(Tracker::Rotation::rotateFromTo(Tracker::Vector(0,0,1),accel));
		glTranslatef(0.0f,0.0f,len*0.5f);
		glDrawArrow(0.5f,1.0f,1.5f,len,16);
		glPopMatrix();
		
		/* Draw the current magnetic flux density vector: */
		glPushMatrix();
		glColor3f(1.0f,0.0f,1.0f);
		const Tracker::Vector& mag=orientations.getLockedValue().magneticFlux;
		len=GLfloat(Geometry::mag(mag))*20.0f;
		glRotate(Tracker::Rotation::rotateFromTo(Tracker::Vector(0,0,1),mag));
		// glTranslatef(0.0f,0.0f,len*0.5f);
		glDrawArrow(0.5f,1.0f,1.5f,len*2.0f,16);
		glPopMatrix();
		
		#if 1
		glPopMatrix();
		#endif
		}
	
	glPopAttrib();
	
	#if FITRAWMEASUREMENTS
	if(showRawMeasurements)
		tracker.render();
	#endif
	}

void OculusCalibrator::eventCallback(EventID eventId,Vrui::InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState)
		{
		switch(eventId)
			{
			#if FITRAWMEASUREMENTS
			case 0:
				showRawMeasurements=!showRawMeasurements;
				break;
			
			case 1:
				tracker.setRecordRawMeasurements(true);
				break;
			
			case 2:
				{
				std::string calibFileName=VRDEVICEDAEMON_CONFIG_CONFIGDIR;
				calibFileName.append("/OculusRift-");
				calibFileName.append(oculus.getSerialNumber());
				calibFileName.append(".calib");
				tracker.updateCalibration(calibFileName);
				}
			#endif
			
			case 3:
				showTracker=!showTracker;
				break;
			
			case 4:
				lockPosition=!lockPosition;
				break;
			
			case 5:
				tracker.resetPosition();
				break;
			
			case 6:
				{
				Tracker::Vector xAxis=orientations.getLockedValue().orientation.getDirection(0);
				std::cout<<xAxis<<std::endl;
				xAxis[1]=Tracker::Scalar(0);
				Tracker::Rotation rot=Tracker::Rotation::rotateFromTo(xAxis,Tracker::Vector(1,0,0));
				std::cout<<rot<<std::endl;
				Tracker::Scalar yawAngle=Math::deg(rot.getAngle());
				if(rot.getAxis()[1]<Tracker::Scalar(0))
					yawAngle=-yawAngle;
				std::cout<<"Yaw correction transformation: rotate (0.0, 0.0, 1.0), "<<yawAngle<<std::endl;
				break;
				}
			}
		}
	else
		{
		switch(eventId)
			{
			#if FITRAWMEASUREMENTS
			case 1:
				tracker.setRecordRawMeasurements(false);
				break;
			#endif
			}
		}
	}

VRUI_APPLICATION_RUN(OculusCalibrator)
