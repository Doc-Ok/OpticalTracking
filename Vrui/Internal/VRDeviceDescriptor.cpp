/***********************************************************************
VRDeviceDescriptor - Class describing the structure of an input device
represented by a VR device daemon.
Copyright (c) 2010-2013 Oliver Kreylos

This file is part of the Virtual Reality User Interface Library (Vrui).

The Virtual Reality User Interface Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Virtual Reality User Interface Library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality User Interface Library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#include <Vrui/Internal/VRDeviceDescriptor.h>

#include <Misc/PrintInteger.h>
#include <Misc/StandardMarshallers.h>
#include <Misc/ArrayMarshallers.h>
#include <Misc/ConfigurationFile.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ArrayValueCoders.h>
#include <IO/File.h>
#include <Geometry/GeometryMarshallers.h>
#include <Geometry/GeometryValueCoders.h>

namespace Vrui {

/***********************************
Methods of class VRDeviceDescriptor:
***********************************/

VRDeviceDescriptor::VRDeviceDescriptor(void)
	:trackType(TRACK_NONE),rayDirection(0,1,0),rayStart(0.0f),trackerIndex(-1),
	 numButtons(0),buttonNames(0),buttonIndices(0),
	 numValuators(0),valuatorNames(0),valuatorIndices(0)
	{
	}

VRDeviceDescriptor::VRDeviceDescriptor(int sNumButtons,int sNumValuators)
	:trackType(TRACK_NONE),rayDirection(0,1,0),rayStart(0.0f),trackerIndex(-1),
	 numButtons(sNumButtons),
	 buttonNames(numButtons>0?new std::string[numButtons]:0),buttonIndices(numButtons>0?new int[numButtons]:0),
	 numValuators(sNumValuators),
	 valuatorNames(numValuators>0?new std::string[numValuators]:0),valuatorIndices(numValuators>0?new int[numValuators]:0)
	{
	/* Initialize button and valuator indices: */
	for(int i=0;i<numButtons;++i)
		buttonIndices[i]=-1;
	for(int i=0;i<numValuators;++i)
		valuatorIndices[i]=-1;
	}

VRDeviceDescriptor::~VRDeviceDescriptor(void)
	{
	delete[] buttonNames;
	delete[] buttonIndices;
	delete[] valuatorNames;
	delete[] valuatorIndices;
	}

void VRDeviceDescriptor::write(IO::File& sink) const
	{
	Misc::Marshaller<std::string>::write(name,sink);
	sink.write<int>(trackType);
	Misc::Marshaller<Vector>::write(rayDirection,sink);
	sink.write<float>(rayStart);
	sink.write<int>(trackerIndex);
	sink.write<int>(numButtons);
	if(numButtons>0)
		{
		Misc::FixedArrayMarshaller<std::string>::write(buttonNames,numButtons,sink);
		Misc::FixedArrayMarshaller<int>::write(buttonIndices,numButtons,sink);
		}
	sink.write<int>(numValuators);
	if(numValuators>0)
		{
		Misc::FixedArrayMarshaller<std::string>::write(valuatorNames,numValuators,sink);
		Misc::FixedArrayMarshaller<int>::write(valuatorIndices,numValuators,sink);
		}
	}

void VRDeviceDescriptor::read(IO::File& source)
	{
	name=Misc::Marshaller<std::string>::read(source);
	trackType=source.read<int>();
	rayDirection=Misc::Marshaller<Vector>::read(source);
	rayStart=source.read<float>();
	trackerIndex=source.read<int>();
	numButtons=source.read<int>();
	delete[] buttonNames;
	delete[] buttonIndices;
	if(numButtons>0)
		{
		buttonNames=new std::string[numButtons];
		Misc::FixedArrayMarshaller<std::string>::read(buttonNames,numButtons,source);
		buttonIndices=new int[numButtons];
		Misc::FixedArrayMarshaller<int>::read(buttonIndices,numButtons,source);
		}
	else
		{
		buttonNames=0;
		buttonIndices=0;
		}
	
	numValuators=source.read<int>();
	delete[] valuatorNames;
	delete[] valuatorIndices;
	if(numValuators>0)
		{
		valuatorNames=new std::string[numValuators];
		Misc::FixedArrayMarshaller<std::string>::read(valuatorNames,numValuators,source);
		valuatorIndices=new int[numValuators];
		Misc::FixedArrayMarshaller<int>::read(valuatorIndices,numValuators,source);
		}
	else
		{
		valuatorNames=0;
		valuatorIndices=0;
		}
	}

void VRDeviceDescriptor::save(Misc::ConfigurationFileSection& configFileSection) const
	{
	configFileSection.storeValue<std::string>("./name",name);
	std::string trackTypeString;
	if(trackType==(TRACK_POS|TRACK_DIR|TRACK_ORIENT))
		trackTypeString="6D";
	else if(trackType==(TRACK_POS|TRACK_DIR))
		trackTypeString="Ray";
	else if(trackType==TRACK_POS)
		trackTypeString="3D";
	else
		trackTypeString="None";
	configFileSection.storeValue<std::string>("./trackType",trackTypeString);
	if(trackType&TRACK_DIR)
		{
		configFileSection.storeValue<Vector>("./rayDirection",rayDirection);
		configFileSection.storeValue<float>("./rayStart",rayStart);
		}
	if(trackType&TRACK_POS)
		configFileSection.storeValue<int>("./trackerIndex",trackerIndex);
	if(numButtons>0)
		{
		configFileSection.storeValue<int>("./numButtons",numButtons);
		configFileSection.storeValueWC<std::string*>("./buttonNames",buttonNames,Misc::FixedArrayValueCoder<std::string>(buttonNames,numButtons));
		configFileSection.storeValueWC<int*>("./buttonIndices",buttonIndices,Misc::FixedArrayValueCoder<int>(buttonIndices,numButtons));
		}
	if(numValuators>0)
		{
		configFileSection.storeValue<int>("./numValuators",numValuators);
		configFileSection.storeValueWC<std::string*>("./valuatorNames",valuatorNames,Misc::FixedArrayValueCoder<std::string>(valuatorNames,numValuators));
		configFileSection.storeValueWC<int*>("./valuatorIndices",valuatorIndices,Misc::FixedArrayValueCoder<int>(valuatorIndices,numValuators));
		}
	}

void VRDeviceDescriptor::load(const Misc::ConfigurationFileSection& configFileSection)
	{
	name=configFileSection.retrieveValue<std::string>("./name");
	std::string trackTypeString=configFileSection.retrieveValue<std::string>("./trackType","None");
	if(trackTypeString=="6D")
		trackType=TRACK_POS|TRACK_DIR|TRACK_ORIENT;
	else if(trackTypeString=="Ray")
		trackType=TRACK_POS|TRACK_DIR;
	else if(trackTypeString=="3D")
		trackType=TRACK_POS;
	else
		trackType=TRACK_NONE;
	rayDirection=configFileSection.retrieveValue<Vector>("./rayDirection",rayDirection);
	rayStart=configFileSection.retrieveValue<float>("./rayStart",rayStart);
	if(trackType&TRACK_POS)
		trackerIndex=configFileSection.retrieveValue<int>("./trackerIndex");
	
	numButtons=configFileSection.retrieveValue<int>("./numButtons",0);
	delete[] buttonNames;
	delete[] buttonIndices;
	if(numButtons>0)
		{
		buttonNames=new std::string[numButtons];
		for(int i=0;i<numButtons;++i)
			{
			buttonNames[i]="Button";
			char index[10];
			buttonNames[i].append(Misc::print(i,index+sizeof(index)-1));
			}
		Misc::DynamicArrayValueCoder<std::string> bnCoder(buttonNames,numButtons);
		configFileSection.retrieveValueWC<std::string*>("./buttonNames",buttonNames,bnCoder);
		
		buttonIndices=new int[numButtons];
		if(configFileSection.hasTag("./buttonIndexBase"))
			{
			int buttonIndexBase=configFileSection.retrieveValue<int>("./buttonIndexBase");
			for(int i=0;i<numButtons;++i)
				buttonIndices[i]=buttonIndexBase+i;
			}
		else
			{
			Misc::FixedArrayValueCoder<int> biCoder(buttonIndices,numButtons);
			configFileSection.retrieveValueWC<int*>("./buttonIndices",biCoder);
			}
		}
	else
		{
		buttonNames=0;
		buttonIndices=0;
		}
	
	numValuators=configFileSection.retrieveValue<int>("./numValuators",0);
	delete[] valuatorNames;
	delete[] valuatorIndices;
	if(numValuators>0)
		{
		valuatorNames=new std::string[numValuators];
		for(int i=0;i<numValuators;++i)
			{
			valuatorNames[i]="Valuator";
			char index[10];
			valuatorNames[i].append(Misc::print(i,index+sizeof(index)-1));
			}
		Misc::DynamicArrayValueCoder<std::string> vnCoder(valuatorNames,numValuators);
		configFileSection.retrieveValueWC<std::string*>("./valuatorNames",valuatorNames,vnCoder);
		
		valuatorIndices=new int[numValuators];
		if(configFileSection.hasTag("./valuatorIndexBase"))
			{
			int valuatorIndexBase=configFileSection.retrieveValue<int>("./valuatorIndexBase");
			for(int i=0;i<numValuators;++i)
				valuatorIndices[i]=valuatorIndexBase+i;
			}
		else
			{
			Misc::FixedArrayValueCoder<int> viCoder(valuatorIndices,numValuators);
			configFileSection.retrieveValueWC<int*>("./valuatorIndices",viCoder);
			}
		}
	else
		{
		valuatorNames=0;
		valuatorIndices=0;
		}
	}

}
