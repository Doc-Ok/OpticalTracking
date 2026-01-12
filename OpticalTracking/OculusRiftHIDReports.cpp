/***********************************************************************
OculusRiftHIDReports - Classes defining the feature reports and raw
reports used by the Oculus Rift DK1 and DK2's raw HID protocol.
Copyright (c) 2014-2022 Oliver Kreylos

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

#include "OculusRiftHIDReports.h"

#include <IO/FixedMemoryFile.h>
#include <RawHID/Device.h>
#include <Math/Math.h>
#include <Math/Constants.h>

#include "TimeStampSource.h"

// DEBUGGING
#include <iostream>
#include <iomanip>

namespace {

/****************
Helper functions:
****************/

/* Convert unsigned fixed-point number to floating-point number: */
inline float fixToFloat(unsigned int fix,float bias,int numFractionBits)
	{
	return (float(fix)-bias)/float(1<<numFractionBits);
	}

/* Unpack an unsigned integer from 2 unsigned bytes: */
inline int unpackUInt16(const Misc::UInt8 raw[2])
	{
	union // Helper union to assemble 2 bytes into an unsigned 16-bit integer
		{
		Misc::UInt8 b[2];
		Misc::UInt16 u;
		} p;
	
	/* Assemble the integer's components: */
	p.b[0]=raw[0];
	p.b[1]=raw[1];
	
	/* Return the unsigned integer: */
	return p.u;
	}

/* Unpack a signed integer from 2 unsigned bytes: */
inline int unpackSInt16(const Misc::UInt8 raw[2])
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

/* Convert 8 bytes into a vector of 3 21-bit signed integers: */
inline void unpackVector(const Misc::UInt8 raw[8],int vector[3])
	{
	#if 0 // Unpack via bit shifting and masking
	
	vector[0]=(Misc::UInt32(raw[0])<<13)|(Misc::UInt32(raw[1])<<5)|(Misc::UInt32(raw[2])>>3);
	if(vector[0]&0x00100000)
		vector[0]|=~0x001fffff;
	vector[1]=(Misc::UInt32(raw[2]&0x07U)<<18)|(Misc::UInt32(raw[3])<<10)|(Misc::UInt32(raw[4])<<2)|(Misc::UInt32(raw[5])>>6);
	if(vector[1]&0x00100000)
		vector[1]|=~0x001fffff;
	vector[2]=(Misc::UInt32(raw[5]&0x3fU)<<15)|(Misc::UInt32(raw[6])<<7)|(Misc::UInt32(raw[7])>>1);
	if(vector[2]&0x00100000)
		vector[2]|=~0x001fffff;
	
	#else // Unpack via unions and bitfield structures
	
	union // Helper union to convert four little-endian bytes to a 32-bit integer
		{
		Misc::UInt8 b[4];
		Misc::SInt32 i;
		} p;
	
	struct // Helper bitfield structure to sign-extend a 21-bit integer
		{
		signed int si:21;
		} s;
	
	p.b[0]=raw[2];
	p.b[1]=raw[1];
	p.b[2]=raw[0];
	// p.b[3]=0U; // Not needed because it's masked out below anyway
	vector[0]=s.si=p.i>>3;
	p.b[0]=raw[5];
	p.b[1]=raw[4];
	p.b[2]=raw[3];
	p.b[3]=raw[2];
	vector[1]=s.si=(p.i>>6)&0x001fffff;
	p.b[0]=raw[7];
	p.b[1]=raw[6];
	p.b[2]=raw[5];
	// p.b[3]=0U; // Not needed because it's masked out below anyway
	vector[2]=s.si=(p.i>>1)&0x001fffff;
	
	#endif
	}

}

/*****************************
Methods of class SensorConfig:
*****************************/

SensorConfig::SensorConfig(void)
	:flags(0x00),packetInterval(19),sampleRate(1000)
	{
	}

unsigned int SensorConfig::get(RawHID::Device& device)
	{
	/* Set up the feature report packet buffer: */
	IO::FixedMemoryFile pktBuffer(7);
	pktBuffer.setEndianness(Misc::LittleEndian);
	Misc::UInt8* buf=static_cast<Misc::UInt8*>(pktBuffer.getMemory());
	memset(buf,0U,pktBuffer.getSize());
	buf[0]=0x02U;
	
	/* Read the sensor configuration feature report: */
	device.readSizedFeatureReport(buf,pktBuffer.getSize());
	
	/* Unpack the packet buffer: */
	pktBuffer.skip<Misc::UInt8>(1); // Skip report ID
	unsigned int commandId=pktBuffer.read<Misc::UInt16>();
	flags=pktBuffer.read<Misc::UInt8>(); // Read flags
	packetInterval=pktBuffer.read<Misc::UInt8>(); // Read sensor packet interval
	sampleRate=pktBuffer.read<Misc::UInt16>(); // Read sensor sample rate
	
	return commandId;
	}

void SensorConfig::set(RawHID::Device& device,unsigned int commandId) const
	{
	/* Set up the feature report packet buffer: */
	IO::FixedMemoryFile pktBuffer(7);
	pktBuffer.setEndianness(Misc::LittleEndian);
	
	/* Pack the packet buffer: */
	pktBuffer.write<Misc::UInt8>(0x02U); // Report ID
	pktBuffer.write<Misc::UInt16>(commandId);
	pktBuffer.write<Misc::UInt8>(flags);
	pktBuffer.write<Misc::UInt8>(packetInterval);
	pktBuffer.write<Misc::UInt16>(sampleRate);
	
	/* Write the sensor configuration feature report: */
	device.writeFeatureReport(static_cast<const RawHID::Device::Byte*>(pktBuffer.getMemory()),pktBuffer.getSize());
	}

void SensorConfig::print(std::ostream& os) const
	{
	os<<"Sensor configuration:"<<std::endl;
	os<<"  Raw mode          : "<<((flags&RawFlags)!=0x0U?"on":"off")<<std::endl;
	os<<"  Calibration test  : "<<((flags&TestCalibFlags)!=0x0U?"on":"off")<<std::endl;
	os<<"  Use calibration   : "<<((flags&UseCalibFlags)!=0x0U?"on":"off")<<std::endl;
	os<<"  Auto calibration  : "<<((flags&AutoCalibFlags)!=0x0U?"on":"off")<<std::endl;
	os<<"  Motion keep-alive : "<<((flags&MotionKeepAliveFlags)!=0x0U?"on":"off")<<std::endl;
	os<<"  Command keep-alive: "<<((flags&CommandKeepAliveFlags)!=0x0U?"on":"off")<<std::endl;
	os<<"  Sensor coordinates: "<<((flags&SensorCoordinatesFlags)!=0x0U?"on":"off")<<std::endl;
	os<<"  Packet Interval   : "<<packetInterval<<", "<<double(sampleRate)/double(packetInterval+1)<<" Hz"<<std::endl;
	os<<"  Sensor sample rate: "<<sampleRate<<" Hz"<<std::endl;
	}

/*******************************
Methods of class IMUCalibration:
*******************************/

IMUCalibration::IMUCalibration(void)
	{
	for(int i=0;i<3;++i)
		for(int j=0;j<4;++j)
			{
			accelMatrix[i][j]=i==j?1.0f:0.0f;
			gyroMatrix[i][j]=i==j?1.0f:0.0f;
			}
	temperature=0.0f;
	}

unsigned int IMUCalibration::get(RawHID::Device& device)
	{
	/* Set up the feature report packet buffer: */
	Misc::UInt8 pktBuffer[69];
	memset(pktBuffer,0U,sizeof(pktBuffer));
	pktBuffer[0]=0x03U;
	
	/* Read the IMU calibration data feature report: */
	device.readSizedFeatureReport(pktBuffer,sizeof(pktBuffer));
	
	/* Unpack the packet buffer: */
	int vector[3];
	unsigned int commandId=unpackUInt16(pktBuffer+1);
	unpackVector(pktBuffer+3,vector);
	for(int i=0;i<3;++i)
		accelMatrix[i][3]=float(vector[i])*1.0e-4f;
	unpackVector(pktBuffer+11,vector);
	for(int i=0;i<3;++i)
		gyroMatrix[i][3]=float(vector[i])*1.0e-4f;
	for(int j=0;j<3;++j)
		{
		unpackVector(pktBuffer+19+j*8,vector);
		for(int i=0;i<3;++i)
			accelMatrix[i][j]=float(vector[i])/float((1<<20)-1);
		accelMatrix[j][j]+=1.0f;
		}
	for(int j=0;j<3;++j)
		{
		unpackVector(pktBuffer+43+j*8,vector);
		for(int i=0;i<3;++i)
			gyroMatrix[i][j]=float(vector[i])/float((1<<20)-1);
		gyroMatrix[j][j]+=1.0f;
		}
	temperature=float(unpackUInt16(pktBuffer+67))*0.01f;
	
	return commandId;
	}

void printMatrix(std::ostream& os,const char* label,const float matrix[3][4])
	{
	for(int i=0;i<3;++i)
		{
		if(i==0)
			os<<label;
		else
			{
			for(const char* lPtr=label;*lPtr!='\0';++lPtr)
				os<<' ';
			}
		
		for(int j=0;j<4;++j)
			os<<' '<<std::setw(12)<<matrix[i][j];
		os<<std::endl;
		}
	}

void IMUCalibration::print(std::ostream& os) const
	{
	os<<"IMU calibration data:"<<std::endl;
	printMatrix(os,"  Accelerometer matrix:",accelMatrix);
	printMatrix(os,"  Gyroscope matrix    :",accelMatrix);
	os<<"  Temperature         : "<<temperature<<std::endl;
	}

/************************************
Static elements of class SensorRange:
************************************/

const unsigned int SensorRange::accelRanges[4]={2,4,8,16};
const unsigned int SensorRange::gyroRanges[4]={250,500,1000,2000};
const unsigned int SensorRange::magRanges[4]={880,1300,1900,2500};

/****************************
Methods of class SensorRange:
****************************/

unsigned int SensorRange::findRangeMatch(float requested,float scale,const unsigned int* range,unsigned int rangeSize)
	{
	unsigned int value=(unsigned int)(Math::floor(requested/scale+0.5f));
	unsigned int i;
	for(i=0;i<rangeSize-1&&range[i]<value;++i)
		;
	return range[i];
	}
SensorRange::SensorRange(void)
	:accelFactor(float(accelRanges[3])*9.81f),
	 gyroFactor(float(gyroRanges[3])*Math::Constants<float>::pi/250.0f),
	 magFactor(float(magRanges[3])*0.001f)
	{
	}

unsigned int SensorRange::get(RawHID::Device& device)
	{
	/* Set up the feature report packet buffer: */
	IO::FixedMemoryFile pktBuffer(8);
	pktBuffer.setEndianness(Misc::LittleEndian);
	Misc::UInt8* buf=static_cast<Misc::UInt8*>(pktBuffer.getMemory());
	memset(buf,0U,pktBuffer.getSize());
	buf[0]=0x04U;
	
	/* Read the sensor range feature report: */
	device.readSizedFeatureReport(buf,pktBuffer.getSize());
	
	/* Unpack the packet buffer: */
	pktBuffer.skip<Misc::UInt8>(1); // Skip report ID
	unsigned int commandId=pktBuffer.read<Misc::UInt16>();
	accelFactor=float(pktBuffer.read<Misc::UInt8>())*9.81f;
	gyroFactor=float(pktBuffer.read<Misc::UInt16>())*Math::Constants<float>::pi/250.0f;
	magFactor=float(pktBuffer.read<Misc::UInt16>())*0.001f;
	
	return commandId;
	}

void SensorRange::set(RawHID::Device& device,unsigned int commandId) const
	{
	/* Set up the feature report packet buffer: */
	IO::FixedMemoryFile pktBuffer(8);
	pktBuffer.setEndianness(Misc::LittleEndian);
	
	/* Pack the packet buffer: */
	pktBuffer.write<Misc::UInt8>(0x04U); // Report ID
	pktBuffer.write<Misc::UInt16>(commandId);
	pktBuffer.write<Misc::UInt8>(findRangeMatch(accelFactor,9.81f,accelRanges,4));
	pktBuffer.write<Misc::UInt16>(findRangeMatch(gyroFactor,Math::Constants<float>::pi/250.0f,gyroRanges,4));
	pktBuffer.write<Misc::UInt16>(findRangeMatch(magFactor,0.001f,magRanges,4));
	
	/* Write the sensor range feature report: */
	device.writeFeatureReport(static_cast<const RawHID::Device::Byte*>(pktBuffer.getMemory()),pktBuffer.getSize());
	}

/*****************************
Methods of class KeepAliveDK1:
*****************************/

KeepAliveDK1::KeepAliveDK1(unsigned int sInterval)
	:interval(sInterval)
	{
	}

unsigned int KeepAliveDK1::get(RawHID::Device& device)
	{
	/* Set up the feature report packet buffer: */
	IO::FixedMemoryFile pktBuffer(5);
	pktBuffer.setEndianness(Misc::LittleEndian);
	RawHID::Device::Byte* buf=static_cast<RawHID::Device::Byte*>(pktBuffer.getMemory());
	memset(buf,0U,pktBuffer.getSize());
	buf[0]=0x08U;
	
	/* Read the keep-alive interval feature report: */
	device.readSizedFeatureReport(buf,pktBuffer.getSize());
	
	/* Unpack the packet buffer: */
	pktBuffer.skip<Misc::UInt8>(1); // Skip report ID
	unsigned int commandId=pktBuffer.read<Misc::UInt16>();
	interval=pktBuffer.read<Misc::UInt16>();
	
	return commandId;
	}

void KeepAliveDK1::set(RawHID::Device& device,unsigned int commandId) const
	{
	/* Set up the feature report packet buffer: */
	IO::FixedMemoryFile pktBuffer(5);
	pktBuffer.setEndianness(Misc::LittleEndian);
	
	/* Pack the packet buffer: */
	pktBuffer.write<Misc::UInt8>(0x08U); // Report ID
	pktBuffer.write<Misc::UInt16>(commandId);
	pktBuffer.write<Misc::UInt16>(interval);
	
	/* Write the keep-alive interval feature report: */
	device.writeFeatureReport(static_cast<const RawHID::Device::Byte*>(pktBuffer.getMemory()),pktBuffer.getSize());
	}

/****************************
Methods of class DisplayInfo:
****************************/

DisplayInfo::DisplayInfo(void)
	{
	/* Initialize to default values for DK1 screen: */
	distortionType=2;
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
	}

unsigned int DisplayInfo::get(RawHID::Device& device)
	{
	/* Set up the feature report packet buffer: */
	IO::FixedMemoryFile pktBuffer(56);
	pktBuffer.setEndianness(Misc::LittleEndian);
	RawHID::Device::Byte* buf=static_cast<RawHID::Device::Byte*>(pktBuffer.getMemory());
	memset(buf,0U,pktBuffer.getSize());
	buf[0]=0x09U;
	
	/* Read the display info feature report: */
	device.readSizedFeatureReport(buf,pktBuffer.getSize());
	
	/* Unpack the packet buffer: */
	pktBuffer.skip<Misc::UInt8>(1); // Skip report number
	unsigned int commandId=pktBuffer.read<Misc::UInt16>();
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
	
	return commandId;
	}

void DisplayInfo::print(std::ostream& os) const
	{
	std::cout<<"Display information:"<<std::endl;
	std::cout<<"  Distortion type         : "<<distortionType<<std::endl;
	if((distortionType&0x0fU)>=0x01U)
		{
		std::cout<<"  Screen resolution       : "<<screenResolution[0]<<" x "<<screenResolution[1]<<std::endl;
		std::cout<<"  Screen size             : "<<screenSize[0]<<" x "<<screenSize[1]<<std::endl;
		std::cout<<"  Vertical screen center  : "<<screenCenterY<<std::endl;
		std::cout<<"  Horizontal lens distance: "<<lensDistanceX<<std::endl;
		std::cout<<"  Eye position            : "<<eyePos[0]<<", "<<eyePos[1]<<std::endl;
		if((distortionType&0x0fU)>=0x02U)
			{
			std::cout<<"  Distortion coefficients :";
			for(int i=0;i<6;++i)
				std::cout<<' '<<distortionCoeffs[i];
			std::cout<<std::endl;
			}
		}
	}

/****************************
Methods of class Unknown0x0a:
****************************/

Unknown0x0a::Unknown0x0a()
	{
	}

unsigned int Unknown0x0a::get(RawHID::Device& device)
	{
	/* Set up the feature report packet buffer: */
	IO::FixedMemoryFile pktBuffer(15);
	pktBuffer.setEndianness(Misc::LittleEndian);
	Misc::UInt8* buf=static_cast<Misc::UInt8*>(pktBuffer.getMemory());
	memset(buf,0U,pktBuffer.getSize());
	buf[0]=0x0aU;
	
	/* Read the unknown feature report: */
	device.readSizedFeatureReport(buf,pktBuffer.getSize());
	
	/* Unpack the packet buffer: */
	pktBuffer.skip<Misc::UInt8>(1); // Skip report ID
	unsigned int commandId=pktBuffer.read<Misc::UInt16>();
	pktBuffer.skip<Misc::UInt8>(12); // Skip rest of report
	
	return commandId;
	}

/***************************
Methods of class LEDControl:
***************************/

LEDControl::LEDControl(void)
	:pattern(0),
	 enable(false),autoIncrement(false),useCarrier(false),syncInput(false),vsyncLock(false),customPattern(false),
	 exposureLength(350),
	 frameInterval(16666),
	 vsyncOffset(0),
	 dutyCycle(127)
	{
	}

unsigned int LEDControl::get(RawHID::Device& device)
	{
	/* Set up the feature report packet buffer: */
	IO::FixedMemoryFile pktBuffer(13);
	pktBuffer.setEndianness(Misc::LittleEndian);
	RawHID::Device::Byte* buf=static_cast<RawHID::Device::Byte*>(pktBuffer.getMemory());
	memset(buf,0U,pktBuffer.getSize());
	buf[0]=0x0cU;
	
	/* Read the sensor range feature report: */
	device.readSizedFeatureReport(buf,pktBuffer.getSize());
	
	/* Unpack the packet buffer: */
	pktBuffer.skip<Misc::UInt8>(1); // Skip report ID
	unsigned int commandId=pktBuffer.read<Misc::UInt16>();
	pattern=pktBuffer.read<Misc::UInt8>();
	Misc::UInt8 flags=pktBuffer.read<Misc::UInt8>();
	enable=(flags&0x01U)!=0x0U;
	autoIncrement=(flags&0x02U)!=0x0U;
	useCarrier=(flags&0x04U)!=0x0U;
	syncInput=(flags&0x08U)!=0x0U;
	vsyncLock=(flags&0x10U)!=0x0U;
	customPattern=(flags&0x20U)!=0x0U;
	pktBuffer.skip<Misc::UInt8>(1); // Skip reserved byte
	exposureLength=pktBuffer.read<Misc::UInt16>();
	frameInterval=pktBuffer.read<Misc::UInt16>();
	vsyncOffset=pktBuffer.read<Misc::UInt16>();
	dutyCycle=pktBuffer.read<Misc::UInt8>();
	
	return commandId;
	}

void LEDControl::set(RawHID::Device& device,unsigned int commandId) const
	{
	/* Set up the feature report packet buffer: */
	IO::FixedMemoryFile pktBuffer(13);
	pktBuffer.setEndianness(Misc::LittleEndian);
	
	/* Pack the packet buffer: */
	pktBuffer.write<Misc::UInt8>(0x0cU); // Report ID
	pktBuffer.write<Misc::UInt16>(commandId);
	pktBuffer.write<Misc::UInt8>(pattern);
	Misc::UInt8 flags=0x00U;
	if(enable)
		flags|=0x01U;
	if(autoIncrement)
		flags|=0x02U;
	if(useCarrier)
		flags|=0x04U;
	if(syncInput)
		flags|=0x08U;
	if(vsyncLock)
		flags|=0x10U;
	if(customPattern)
		flags|=0x20U;
	pktBuffer.write<Misc::UInt8>(flags);
	pktBuffer.write<Misc::UInt8>(0x00U); // Reserved byte
	pktBuffer.write<Misc::UInt16>(exposureLength);
	pktBuffer.write<Misc::UInt16>(frameInterval);
	pktBuffer.write<Misc::UInt16>(vsyncOffset);
	pktBuffer.write<Misc::UInt8>(dutyCycle);

	/* Write the sensor range feature report: */
	device.writeFeatureReport(static_cast<const RawHID::Device::Byte*>(pktBuffer.getMemory()),pktBuffer.getSize());
	}

void LEDControl::print(std::ostream& os) const
	{
	os<<"LED control:"<<std::endl;
	os<<"  Pattern        : "<<pattern<<std::endl;
	os<<"  Enabled        : "<<(enable?"true":"false")<<std::endl;
	os<<"  Auto-increment : "<<(autoIncrement?"on":"off")<<std::endl;
	os<<"  Use carrier    : "<<(useCarrier?"on":"off")<<std::endl;
	os<<"  Sync input     : "<<(syncInput?"on":"off")<<std::endl;
	os<<"  Vsync lock     : "<<(vsyncLock?"on":"off")<<std::endl;
	os<<"  Custom pattern : "<<(customPattern?"on":"off")<<std::endl;
	os<<"  Exposure length: "<<exposureLength<<std::endl;
	os<<"  Frame interval : "<<frameInterval<<" ("<<1.0e6/double(frameInterval)<<" Hz)"<<std::endl;
	os<<"  Vsync offset   : "<<vsyncOffset<<std::endl;
	os<<"  Duty cycle     : "<<dutyCycle<<" ("<<double(dutyCycle)/255.0<<"%)"<<std::endl;
	}

/****************************
Methods of class Unknown0x0d:
****************************/

Unknown0x0d::Unknown0x0d()
	{
	}

unsigned int Unknown0x0d::get(RawHID::Device& device)
	{
	/* Set up the feature report packet buffer: */
	IO::FixedMemoryFile pktBuffer(16);
	pktBuffer.setEndianness(Misc::LittleEndian);
	Misc::UInt8* buf=static_cast<Misc::UInt8*>(pktBuffer.getMemory());
	memset(buf,0U,pktBuffer.getSize());
	buf[0]=0x0dU;
	
	/* Read the unknown feature report: */
	device.readSizedFeatureReport(buf,pktBuffer.getSize());
	
	/* Unpack the packet buffer: */
	pktBuffer.skip<Misc::UInt8>(1); // Skip report ID
	unsigned int commandId=pktBuffer.read<Misc::UInt16>();
	pktBuffer.skip<Misc::UInt8>(13); // Skip rest of report
	
	return commandId;
	}

/******************************************
Methods of class CameraIntrinsicParameters:
******************************************/

CameraIntrinsicParameters::CameraIntrinsicParameters(void)
	{
	/* Initialize the matrix: */
	for(int i=0;i<3;++i)
		for(int j=0;j<4;++j)
			matrix[i][j]=i==j?1.0f:0.0f;
	}

unsigned int CameraIntrinsicParameters::get(RawHID::Device& device)
	{
	/* Set up the feature report packet buffer: */
	IO::FixedMemoryFile pktBuffer(52);
	pktBuffer.setEndianness(Misc::LittleEndian);
	RawHID::Device::Byte* buf=static_cast<RawHID::Device::Byte*>(pktBuffer.getMemory());
	memset(buf,0U,pktBuffer.getSize());
	buf[0]=0x0eU;
	
	/* Read the camera intrinsic parameters feature report: */
	device.readSizedFeatureReport(buf,pktBuffer.getSize());
	
	/* Unpack the packet buffer: */
	pktBuffer.skip<Misc::UInt8>(1); // Skip report ID
	unsigned int commandId=pktBuffer.read<Misc::UInt16>();
	pktBuffer.skip<Misc::UInt8>(1); // Skip unknown byte
	for(int i=0;i<3;++i)
		for(int j=0;j<4;++j)
			matrix[i][j]=float(pktBuffer.read<Misc::SInt32>());
	
	return commandId;
	}

/****************************
Methods of class LEDPosition:
****************************/

LEDPosition::LEDPosition(void)
	:numReports(1),reportIndex(0),
	 isLed(false)
	{
	for(int i=0;i<3;++i)
		pos[i]=0;
	for(int i=0;i<3;++i)
		dir[i]=0;
	}

unsigned int LEDPosition::get(RawHID::Device& device)
	{
	/* Set up the feature report packet buffer: */
	IO::FixedMemoryFile pktBuffer(30);
	pktBuffer.setEndianness(Misc::LittleEndian);
	RawHID::Device::Byte* buf=static_cast<RawHID::Device::Byte*>(pktBuffer.getMemory());
	memset(buf,0U,pktBuffer.getSize());
	buf[0]=0x0fU;
	
	/* Read the keep-alive interval feature report: */
	device.readSizedFeatureReport(buf,pktBuffer.getSize());
	
	/* Unpack the packet buffer: */
	pktBuffer.skip<Misc::UInt8>(1); // Skip report ID
	unsigned int commandId=pktBuffer.read<Misc::UInt16>();
	isLed=pktBuffer.read<Misc::UInt8>()==0x02U;
	for(int i=0;i<3;++i)
		pos[i]=float(pktBuffer.read<Misc::SInt32>())*1.0e-6f;
	for(int i=0;i<3;++i)
		dir[i]=float(pktBuffer.read<Misc::SInt16>());
	pktBuffer.skip<Misc::UInt8>(2); // Skip unknown bytes
	reportIndex=pktBuffer.read<Misc::UInt8>();
	pktBuffer.skip<Misc::UInt8>(1); // Skip unknown byte
	numReports=pktBuffer.read<Misc::UInt8>();
	
	return commandId;
	}

/*****************************
Methods of class KeepAliveDK2:
*****************************/

KeepAliveDK2::KeepAliveDK2(bool sKeepLeds,unsigned int sInterval)
	:keepLeds(sKeepLeds),interval(sInterval)
	{
	}

unsigned int KeepAliveDK2::get(RawHID::Device& device)
	{
	/* Set up the feature report packet buffer: */
	IO::FixedMemoryFile pktBuffer(6);
	pktBuffer.setEndianness(Misc::LittleEndian);
	RawHID::Device::Byte* buf=static_cast<RawHID::Device::Byte*>(pktBuffer.getMemory());
	memset(buf,0U,pktBuffer.getSize());
	buf[0]=0x11U;
	
	/* Read the keep-alive interval feature report: */
	device.readSizedFeatureReport(buf,pktBuffer.getSize());
	
	/* Unpack the packet buffer: */
	pktBuffer.skip<Misc::UInt8>(1); // Skip report ID
	unsigned int commandId=pktBuffer.read<Misc::UInt16>();
	unsigned int flags=pktBuffer.read<Misc::UInt8>();
	keepLeds=flags==0x0bU;
	interval=pktBuffer.read<Misc::UInt16>();
	
	return commandId;
	}

void KeepAliveDK2::set(RawHID::Device& device,unsigned int commandId) const
	{
	/* Set up the feature report packet buffer: */
	IO::FixedMemoryFile pktBuffer(6);
	pktBuffer.setEndianness(Misc::LittleEndian);
	
	/* Pack the packet buffer: */
	pktBuffer.write<Misc::UInt8>(0x11U); // Report ID
	pktBuffer.write<Misc::UInt16>(commandId);
	pktBuffer.write<Misc::UInt8>(keepLeds?0x0bU:0x01U);
	pktBuffer.write<Misc::UInt16>(interval);
	
	/* Write the keep-alive interval feature report: */
	device.writeFeatureReport(static_cast<const RawHID::Device::Byte*>(pktBuffer.getMemory()),pktBuffer.getSize());
	}

/*****************************
Methods of class SerialNumber:
*****************************/

SerialNumber::SerialNumber()
	{
	/* Initialize serial number: */
	serialNumber[0]='\0';
	}

unsigned int SerialNumber::get(RawHID::Device& device)
	{
	/* Set up the feature report packet buffer: */
	IO::FixedMemoryFile pktBuffer(23);
	pktBuffer.setEndianness(Misc::LittleEndian);
	Misc::UInt8* buf=static_cast<Misc::UInt8*>(pktBuffer.getMemory());
	memset(buf,0U,pktBuffer.getSize());
	buf[0]=0x13U;
	
	/* Read the unknown feature report: */
	device.readSizedFeatureReport(buf,pktBuffer.getSize());
	
	/* Unpack the packet buffer: */
	pktBuffer.skip<Misc::UInt8>(1); // Skip report ID
	unsigned int commandId=pktBuffer.read<Misc::UInt16>();
	pktBuffer.read(reinterpret_cast<Misc::UInt8*>(serialNumber),20);
	serialNumber[20]='\0';
	
	return commandId;
	}

/**********************************
Methods of class LensConfiguration:
**********************************/

LensConfiguration::LensConfiguration(void)
	{
	/* Set lens configuration to default values for Rift DK2: */
	numReports=1;
	reportIndex=0;
	version=1;
	r2Max=1.0376f;
	catmullRom[0]=1.003f;
	catmullRom[1]=1.02f;
	catmullRom[2]=1.042f;
	catmullRom[3]=1.066f;
	catmullRom[4]=1.094f;
	catmullRom[5]=1.126f;
	catmullRom[6]=1.162f;
	catmullRom[7]=1.203f;
	catmullRom[8]=1.25f;
	catmullRom[9]=1.31f;
	catmullRom[10]=1.38f;
	pixelSize=0.03125f;
	eyeRelief=0.036f;
	redPolynomial[0]=-0.015f;
	redPolynomial[1]=-0.02f;
	bluePolynomial[0]=0.025f;
	bluePolynomial[1]=0.02f;
	}

unsigned int LensConfiguration::get(RawHID::Device& device)
	{
	/* Set up the feature report packet buffer: */
	IO::FixedMemoryFile pktBuffer(64);
	pktBuffer.setEndianness(Misc::LittleEndian);
	RawHID::Device::Byte* buf=static_cast<RawHID::Device::Byte*>(pktBuffer.getMemory());
	memset(buf,0U,pktBuffer.getSize());
	buf[0]=0x16U;
	
	/* Read the lens configuration feature report: */
	device.readSizedFeatureReport(buf,pktBuffer.getSize());
	
	/* Unpack the packet buffer: */
	pktBuffer.skip<Misc::UInt8>(1); // Skip report number
	unsigned int commandId=pktBuffer.read<Misc::UInt16>();
	numReports=pktBuffer.read<Misc::UInt8>();
	reportIndex=pktBuffer.read<Misc::UInt8>();
	pktBuffer.skip<Misc::UInt8>(3); // Skip 3 unknown bytes
	version=pktBuffer.read<Misc::UInt16>();
	r2Max=fixToFloat(pktBuffer.read<Misc::UInt16>(),0.0f,14);
	for(int i=0;i<11;++i)
		catmullRom[i]=fixToFloat(pktBuffer.read<Misc::UInt16>(),0.0f,14);
	pixelSize=fixToFloat(pktBuffer.read<Misc::UInt16>(),0.0f,19);
	eyeRelief=fixToFloat(pktBuffer.read<Misc::UInt16>(),0.0f,19);
	for(int i=0;i<2;++i)
		redPolynomial[i]=fixToFloat(pktBuffer.read<Misc::UInt16>(),32768.0f,19);
	for(int i=0;i<2;++i)
		bluePolynomial[i]=fixToFloat(pktBuffer.read<Misc::UInt16>(),32768.0f,19);
	
	return commandId;
	}

void LensConfiguration::print(std::ostream& os) const
	{
	std::cout<<"Lens configuration "<<reportIndex<<" of "<<numReports<<":"<<std::endl;
	std::cout<<"  Version        : "<<version<<std::endl;
	std::cout<<"  R2Max          : "<<r2Max<<std::endl;
	std::cout<<"  Catmull-Rom    :";
	for(int i=0;i<11;++i)
		std::cout<<' '<<catmullRom[i];
	std::cout<<std::endl;
	std::cout<<"  Pixel size     : "<<pixelSize<<std::endl;
	std::cout<<"  Eye relief     : "<<eyeRelief<<std::endl;
	std::cout<<"  Red polynomial : "<<redPolynomial[0]<<", "<<redPolynomial[1]<<std::endl;
	std::cout<<"  Blue polynomial: "<<bluePolynomial[0]<<", "<<bluePolynomial[1]<<std::endl;
	}

/*****************************
Methods of class RadioControl:
*****************************/

RadioControl::RadioControl(unsigned int c0,unsigned int c1,unsigned int c2)
	{
	/* Copy the command sequence: */
	command[0]=Misc::UInt8(c0);
	command[1]=Misc::UInt8(c1);
	command[2]=Misc::UInt8(c2);
	}

unsigned int RadioControl::get(RawHID::Device& device)
	{
	/* Set up the feature report packet buffer: */
	IO::FixedMemoryFile pktBuffer(6);
	pktBuffer.setEndianness(Misc::LittleEndian);
	RawHID::Device::Byte* buf=static_cast<RawHID::Device::Byte*>(pktBuffer.getMemory());
	memset(buf,0U,pktBuffer.getSize());
	buf[0]=0x1aU; // Report ID
	
	/* Read the radio control feature report: */
	device.readSizedFeatureReport(buf,pktBuffer.getSize());
	
	/* Unpack the packet buffer: */
	pktBuffer.skip<Misc::UInt8>(1); // Skip report number
	unsigned int commandId=pktBuffer.read<Misc::UInt16>();
	pktBuffer.read(command,3);
	
	return commandId;
	}

void RadioControl::set(RawHID::Device& device,unsigned int commandId) const
	{
	/* Set up the feature report packet buffer: */
	IO::FixedMemoryFile pktBuffer(6);
	pktBuffer.setEndianness(Misc::LittleEndian);
	
	/* Pack the packet buffer: */
	pktBuffer.write<Misc::UInt8>(0x1aU); // Report ID
	pktBuffer.write<Misc::UInt16>(commandId);
	pktBuffer.write(command,3);
	
	/* Write the radio control feature report: */
	device.writeFeatureReport(static_cast<const RawHID::Device::Byte*>(pktBuffer.getMemory()),pktBuffer.getSize());
	}

/**************************
Methods of class RadioData:
**************************/

RadioData::RadioData(RadioData::ReportType sReportType)
	:reportType(sReportType)
	{
	switch(reportType)
		{
		case MemoryReport:
			memory.start=0;
			memory.length=0;
			memset(memory.data,0,sizeof(memory.data));
			break;
		
		case FirmwareVersionReport:
			memset(firmwareVersion.date,0,sizeof(firmwareVersion.date));
			memset(firmwareVersion.version,0,sizeof(firmwareVersion.version));
			break;
		
		case SerialNumberReport:
			serialNumber.address=0;
			serialNumber.deviceType=0;
			memset(serialNumber.serialNumber,0,sizeof(serialNumber.serialNumber));
			break;
		}
	}

unsigned int RadioData::get(RawHID::Device& device)
	{
	/* Set up the feature report packet buffer: */
	IO::FixedMemoryFile pktBuffer(31);
	pktBuffer.setEndianness(Misc::LittleEndian);
	RawHID::Device::Byte* buf=static_cast<RawHID::Device::Byte*>(pktBuffer.getMemory());
	memset(buf,0U,pktBuffer.getSize());
	buf[0]=0x1bU; // Report ID
	
	/* Read the radio data feature report: */
	device.readSizedFeatureReport(buf,pktBuffer.getSize());
	
	/* Unpack the packet buffer: */
	pktBuffer.skip<Misc::UInt8>(1); // Skip report number
	unsigned int commandId=pktBuffer.read<Misc::UInt16>();
	
	switch(reportType)
		{
		case MemoryReport:
			memory.start=pktBuffer.read<Misc::UInt16>();
			memory.length=pktBuffer.read<Misc::UInt16>();
			pktBuffer.read(memory.data,sizeof(memory.data));
			break;
		
		case FirmwareVersionReport:
			pktBuffer.skip<Misc::UInt8>(3);
			pktBuffer.read(firmwareVersion.date,sizeof(firmwareVersion.date)-1); // Exclude NUL terminator
			pktBuffer.read(firmwareVersion.version,sizeof(firmwareVersion.version)-1); // Exclude NUL terminator
			break;
		
		case SerialNumberReport:
			serialNumber.address=pktBuffer.read<Misc::UInt32>();
			serialNumber.deviceType=pktBuffer.read<Misc::UInt8>();
			pktBuffer.skip<Misc::UInt8>(4);
			pktBuffer.read(serialNumber.serialNumber,sizeof(serialNumber.serialNumber)-1); // Exclude NUL terminator
			break;
		}
	
	return commandId;
	}

/********************************
Methods of class ComponentStatus:
********************************/

ComponentStatus::ComponentStatus(bool sDisplayEnabled,bool sAudioEnabled,bool sLedsEnabled)
	:displayEnabled(sDisplayEnabled),audioEnabled(sAudioEnabled),ledsEnabled(sLedsEnabled)
	{
	}

unsigned int ComponentStatus::get(RawHID::Device& device)
	{
	/* Set up the feature report packet buffer: */
	IO::FixedMemoryFile pktBuffer(4);
	pktBuffer.setEndianness(Misc::LittleEndian);
	RawHID::Device::Byte* buf=static_cast<RawHID::Device::Byte*>(pktBuffer.getMemory());
	memset(buf,0U,pktBuffer.getSize());
	buf[0]=0x1dU; // Report ID
	
	/* Read the component status feature report: */
	device.readSizedFeatureReport(buf,pktBuffer.getSize());
	
	/* Unpack the packet buffer: */
	pktBuffer.skip<Misc::UInt8>(1); // Skip report number
	unsigned int commandId=pktBuffer.read<Misc::UInt16>();
	unsigned int flags=pktBuffer.read<Misc::UInt8>();
	displayEnabled=(flags&DisplayFlag)!=0x00U;
	audioEnabled=(flags&AudioFlag)!=0x00U;
	ledsEnabled=(flags&LedsFlag)!=0x00U;
	
	return commandId;
	}

void ComponentStatus::set(RawHID::Device& device,unsigned int commandId) const
	{
	/* Set up the feature report packet buffer: */
	IO::FixedMemoryFile pktBuffer(4);
	pktBuffer.setEndianness(Misc::LittleEndian);
	
	/* Pack the packet buffer: */
	pktBuffer.write<Misc::UInt8>(0x1dU); // Report ID
	pktBuffer.write<Misc::UInt16>(commandId);
	Misc::UInt8 flags=0x0;
	if(displayEnabled)
		flags|=DisplayFlag;
	if(audioEnabled)
		flags|=AudioFlag;
	if(ledsEnabled)
		flags|=LedsFlag;
	pktBuffer.write<Misc::UInt8>(flags);
	
	/* Write the component status interval feature report: */
	device.writeFeatureReport(static_cast<const RawHID::Device::Byte*>(pktBuffer.getMemory()),pktBuffer.getSize());
	}

/***************************
Methods of class SensorData:
***************************/

SensorData::SensorData(void)
	:numSamples(0U),
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

namespace {

/****************
Helper functions:
****************/

}

void SensorData::get(RawHID::Device& device)
	{
	/* Read next raw HID report: */
	device.readSizedReport(pktBuffer,sizeof(pktBuffer));
	
	/* Unpack the message: */
	if(pktBuffer[0]==0x01U)
		{
		/* Unpack packet header: */
		numSamples=pktBuffer[1];
		timeStamp=unpackUInt16(pktBuffer+2)+(numSamples-1); // Move time stamp from first sample to last sample
		temperature=unpackUInt16(pktBuffer+6);
		
		/* Calculate number of accel/gyro samples contained in the packet: */
		unsigned int numContainedSamples=numSamples;
		if(numContainedSamples>3)
			numContainedSamples=3;
		
		/* Unpack all contained accel/gyro samples: */
		Misc::UInt8* sPtr=pktBuffer+8;
		for(unsigned int sample=0;sample<numContainedSamples;++sample,sPtr+=16)
			{
			unpackVector(sPtr,samples[sample].accel);
			unpackVector(sPtr+8,samples[sample].gyro);
			}
		
		/* Unpack the mag sample: */
		Misc::UInt8* mPtr=pktBuffer+56;
		for(int i=0;i<3;++i,mPtr+=2)
			mag[i]=unpackSInt16(mPtr);
		}
	}

unsigned int SensorData::get(RawHID::Device& device,IMU::RawSample rawSamples[3],TimeStampSource& timeStampSource)
	{
	/* Read next raw HID report: */
	device.readSizedReport(pktBuffer,sizeof(pktBuffer));
	
	/* Unpack the message: */
	if(pktBuffer[0]==0x01U)
		{
		/* Unpack packet header: */
		numSamples=pktBuffer[1];
		Misc::UInt16 newTimeStamp=unpackUInt16(pktBuffer+2);
		newTimeStamp+=numSamples-1; // Move time stamp from first sample to last sample
		Misc::UInt16 timeStampInterval=newTimeStamp-timeStamp;
		timeStamp=newTimeStamp;
		
		/* Update the given time stamp source: */
		timeStampSource.advance(TimeStamp(timeStampInterval)*sampleInterval);
		
		/* Unpack temperature reading: */
		temperature=unpackUInt16(pktBuffer+6);
		
		/* Calculate number of accel/gyro samples contained in the packet: */
		unsigned int numContainedSamples=numSamples;
		if(numContainedSamples>3)
			numContainedSamples=3;
		
		/* Unpack all contained accel/gyro samples: */
		Misc::UInt8* sPtr=pktBuffer+8;
		for(unsigned int sample=0;sample<numContainedSamples;++sample,sPtr+=16)
			{
			unpackVector(sPtr,rawSamples[sample].accelerometer);
			unpackVector(sPtr+8,rawSamples[sample].gyroscope);
			}
		
		/* Unpack the mag sample: */
		Misc::UInt8* mPtr=pktBuffer+56;
		for(int i=0;i<3;++i,mPtr+=2)
			rawSamples[0].magnetometer[i]=unpackSInt16(mPtr);
		
		/* Copy the mag sample into the other raw samples: */
		for(unsigned int sample=1;sample<numContainedSamples;++sample)
			for(int i=0;i<3;++i)
				rawSamples[sample].magnetometer[i]=rawSamples[0].magnetometer[i];
		
		return numContainedSamples;
		}
	else
		return 0;
	}
