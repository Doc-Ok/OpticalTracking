/***********************************************************************
OculusRiftHIDReports - Classes defining the feature reports and raw
reports used by the Oculus Rift DK1 and DK2's raw HID protocol.
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

#include "OculusRiftHIDReports.h"

#include <RawHID/Device.h>
#include <Math/Math.h>
#include <Math/Constants.h>

namespace {

/****************
Helper functions:
****************/

/* Convert unsigned fixed-point number to floating-point number: */
inline float fixToFloat(unsigned int fix,float bias,int numFractionBits)
	{
	return (float(fix)-bias)/float(1<<numFractionBits);
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
	p.b[3]=0U;
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
	
	#endif
	}

}

/****************************
Methods of class Unknown0x02:
****************************/

Unknown0x02::Unknown0x02(unsigned int sValue)
	:value(sValue)
	{
	}

unsigned int Unknown0x02::get(RawHID::Device& device)
	{
	/* Set up the feature report packet buffer: */
	IO::FixedMemoryFile pktBuffer(7);
	pktBuffer.setEndianness(Misc::LittleEndian);
	Misc::UInt8* buf=static_cast<Misc::UInt8*>(pktBuffer.getMemory());
	memset(buf,0U,pktBuffer.getSize());
	buf[0]=0x02U;
	
	/* Read the unknown feature report: */
	device.readSizedFeatureReport(buf,pktBuffer.getSize());
	
	/* Unpack the packet buffer: */
	pktBuffer.skip<Misc::UInt8>(1); // Skip report ID
	unsigned int commandId=pktBuffer.read<Misc::UInt16>();
	pktBuffer.skip<Misc::UInt8>(1); // Skip unknown fixed value (0x20)
	value=pktBuffer.read<Misc::UInt8>(); // Read value of unknown meaning
	pktBuffer.skip<Misc::UInt16>(1); // Skip unknown fixed value (0x3e8 or 1000) Timeout in ms?
	
	return commandId;
	}

void Unknown0x02::set(RawHID::Device& device,unsigned int commandId) const
	{
	/* Set up the feature report packet buffer: */
	IO::FixedMemoryFile pktBuffer(7);
	pktBuffer.setEndianness(Misc::LittleEndian);
	
	/* Pack the packet buffer: */
	pktBuffer.write<Misc::UInt8>(0x02U); // Report ID
	pktBuffer.write<Misc::UInt16>(commandId);
	pktBuffer.write<Misc::UInt8>(0x20U); // Unknown fixed value
	pktBuffer.write<Misc::UInt8>(value);
	pktBuffer.write<Misc::UInt16>(1000); // Timeout in ms?
	
	/* Write the unknown feature report: */
	device.writeFeatureReport(static_cast<const RawHID::Device::Byte*>(pktBuffer.getMemory()),pktBuffer.getSize());
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

KeepAliveDK2::KeepAliveDK2(unsigned int sInterval)
	:interval(sInterval)
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
	pktBuffer.skip<Misc::UInt8>(1); // Skip unknown byte
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
	pktBuffer.write<Misc::UInt8>(0x0bU); // Unknown byte
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
	pktBuffer.read<Misc::UInt8>(reinterpret_cast<Misc::UInt8*>(serialNumber),20);
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

/***************************
Methods of class SensorData:
***************************/

SensorData::SensorData(void)
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

void SensorData::get(RawHID::Device& device)
	{
	/* Read next raw HID report: */
	device.readSizedReport(static_cast<RawHID::Device::Byte*>(pktBuffer.getMemory()),pktBuffer.getSize());
	
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
