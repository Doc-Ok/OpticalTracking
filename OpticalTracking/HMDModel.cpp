/***********************************************************************
HMDModel - Class to represent 3D models of head-mounted displays for
optical tracking purposes, including LED positions and emission vectors
and the position of the integrated inertial measurement unit.
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

#include "HMDModel.h"

#include <string.h>
#include <Misc/ThrowStdErr.h>
#include <RawHID/Device.h>

namespace {

/* Array of 10-bit patterns for zero-based marker indices: */
static const unsigned int markerPatterns[40]=
	{
	2,513,385,898,320,835,195,291,800,675,
	97,610,482,993,144,592,648,170,27,792,
	410,345,730,56,827,697,378,251,1016,196,
	165,21,534,407,916,853,727,308,182,119
	};

/****************
Helper functions:
****************/

inline unsigned int hammingDist(unsigned int p1,unsigned int p2) // Returns Hamming distance between two 10-bit patterns
	{
	unsigned int result=0;
	unsigned int diff=p1^p2;
	unsigned int mask=0x1;
	for(int i=0;i<10;++i,mask<<=1)
		if(diff&mask)
			++result;
	return result;
	}

}

/*************************
Methods of class HMDModel:
*************************/

HMDModel::HMDModel(void)
	:imu(Point::origin),
	 numMarkers(0),markers(0),
	 patternTable(new unsigned char[1024])
	{
	#if 0
	/* Calculate the minimal Hamming distance between any pair of bit patterns: */
	{
	unsigned int minDistance=11;
	for(int p1=0;p1<40-1;++p1)
		for(int p2=p1+1;p2<40;++p2)
			{
			/* Calculate the Hamming distance between the two bit patterns: */
			unsigned int distance=hammingDist(markerPatterns[p1],markerPatterns[p2]);
			if(minDistance>distance)
				minDistance=distance;
			}
	std::cout<<"Hamming separation of marker set: "<<minDistance<<std::endl;
	}
	
	/* Calculate the minimal Hamming distance between a left-shifted pattern and any other pattern: */
	{
	unsigned int minDistance=11;
	for(int p1=0;p1<40;++p1)
		{
		unsigned int shifted=markerPatterns[p1]<<1;
		if(markerPatterns[p1]&0x200U)
			shifted|=0x001U;
		for(int p2=0;p2<40;++p2)
			{
			/* Calculate the Hamming distance between the two bit patterns: */
			unsigned int distance=hammingDist(shifted,markerPatterns[p2]);
			if(minDistance>distance)
				minDistance=distance;
			}
		}
	std::cout<<"Hamming separation of left-shifted marker set: "<<minDistance<<std::endl;
	}
	
	/* Calculate the minimal Hamming distance between a right-shifted pattern and any other pattern: */
	{
	unsigned int minDistance=11;
	for(int p1=0;p1<40;++p1)
		{
		unsigned int shifted=markerPatterns[p1]>>1;
		if(markerPatterns[p1]&0x001U)
			shifted|=0x200U;
		for(int p2=0;p2<40;++p2)
			{
			/* Calculate the Hamming distance between the two bit patterns: */
			unsigned int distance=hammingDist(shifted,markerPatterns[p2]);
			if(minDistance>distance)
				minDistance=distance;
			}
		}
	std::cout<<"Hamming separation of right-shifted marker set: "<<minDistance<<std::endl;
	}
	#endif
	
	/* Create the pattern table: */
	for(unsigned int tp=0;tp<1023;++tp)
		{
		unsigned int minMarker=~0;
		unsigned int minDist=11;
		for(unsigned int markerIndex=0;markerIndex<40;++markerIndex)
			{
			/* Calculate the Hamming distance between the table pattern and the marker pattern: */
			unsigned int distance=hammingDist(tp,markerPatterns[markerIndex]);
			if(minDist>distance)
				{
				minMarker=markerIndex;
				minDist=distance;
				}
			}
		
		/* Assign the best marker unless its Hamming distance is too large: */
		patternTable[tp]=(unsigned char)(minDist<=1?minMarker:~0); // LED indices with more than one bit error are discarded
		}
	}

HMDModel::~HMDModel(void)
	{
	delete[] markers;
	delete[] patternTable;
	}

void HMDModel::readFromRiftDK2(RawHID::Device& rift)
	{
	/* Read a sequence of 0x0fU feature reports to get the position of all LEDs: */
	unsigned int firstReportIndex=0;
	unsigned int numReports=0;
	while(true)
		{
		/* Read the next LED feature report: */
		RawHID::Device::Byte buffer[30];
		memset(buffer,0,sizeof(buffer));
		buffer[0]=0x0fU;
		size_t reportSize=rift.readFeatureReport(buffer,sizeof(buffer));
		if(reportSize!=sizeof(buffer))
			Misc::throwStdErr("HMDModel::readFromRiftDK2: Received LED feature report of %u bytes instead of %u bytes",(unsigned int)reportSize,(unsigned int)sizeof(buffer));
		
		/* Extract the report index and total number of reports: */
		unsigned int ri=buffer[24];
		unsigned int nr=buffer[26];
		
		if(numReports==0) // Is this the first received report?
			{
			numReports=nr;
			firstReportIndex=ri;
			
			/* Allocate the marker array: */
			numMarkers=numReports-1; // One of the reports defines the IMU's position
			markers=new Marker[numMarkers];
			}
		else if(ri==firstReportIndex) // Is this the first received report again?
			break;
		
		/* Helper structure to convert two little-endian bytes to a 16-bit signed integer: */
		union
			{
			Misc::UInt8 b[2];
			Misc::SInt16 i;
			} convert16;
		
		/* Helper structure to convert four little-endian bytes to a 32-bit signed integer: */
		union
			{
			Misc::UInt8 b[4];
			Misc::SInt32 i;
			} convert32;
		
		/* Extract the LED position: */
		Point ledPos;
		RawHID::Device::Byte* bufPtr=buffer+4;
		for(int i=0;i<3;++i)
			{
			/* Convert four little-endian bytes to a 32-bit signed integer: */
			for(int j=0;j<4;++j,++bufPtr)
				convert32.b[j]=*bufPtr;
			ledPos[i]=Scalar(convert32.i)*1.0e-6; // Convert from micrometers to meters
			}
		
		/* Check if this is a LED or the IMU: */
		if(buffer[3]==0x02U)
			{
			/* It's an LED: */
			markers[ri].pattern=markerPatterns[ri];
			
			/* Assign the LED's position: */
			markers[ri].pos=ledPos;
			
			/* Extract the LED's direction vector: */
			for(int i=0;i<3;++i)
				{
				for(int j=0;j<2;++j,++bufPtr)
					convert16.b[j]=*bufPtr;
				markers[ri].dir[i]=Scalar(convert16.i);
				}
			
			/* Normalize the direction vector: */
			markers[ri].dir.normalize();
			}
		else
			{
			/* It's the IMU: */
			imu=ledPos;
			} 
		}
	}
