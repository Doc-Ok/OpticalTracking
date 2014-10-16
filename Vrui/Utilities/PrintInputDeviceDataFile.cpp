/***********************************************************************
PrintInputDeviceDataFile - Program to print the contents of a previously
saved input device data file in the format used by Vrui's
InputDeviceDataSaver and InputDeviceAdapterPlayback classes.
Copyright (c) 2008-2013 Oliver Kreylos

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

#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <Misc/StringMarshaller.h>
#include <IO/File.h>
#include <IO/SeekableFile.h>
#include <IO/OpenFile.h>
#include <Geometry/Vector.h>
#include <Vrui/Geometry.h>
#include <Vrui/InputDevice.h>
#include <Vrui/InputDeviceFeature.h>

std::string getDefaultFeatureName(const Vrui::InputDeviceFeature& feature)
	{
	char featureName[40];
	featureName[0]='\0';
	
	/* Check if the feature is a button or a valuator: */
	if(feature.isButton())
		{
		/* Return a default button name: */
		snprintf(featureName,sizeof(featureName),"Button%d",feature.getIndex());
		}
	if(feature.isValuator())
		{
		/* Return a default valuator name: */
		snprintf(featureName,sizeof(featureName),"Valuator%d",feature.getIndex());
		}
	
	return std::string(featureName);
	}

/**************
Helper classes:
**************/

struct DeviceFileHeader // Structure to store device name and layout in device data files
	{
	/* Elements: */
	public:
	char name[40];
	int trackType;
	int numButtons;
	int numValuators;
	Vrui::Vector deviceRayDirection;
	};

int main(int argc,char* argv[])
	{
	/* Open the input file: */
	IO::SeekableFilePtr inputDeviceDataFile(IO::openSeekableFile(argv[1]));
	inputDeviceDataFile->setEndianness(Misc::LittleEndian);
	
	/* Read the file header: */
	static const char* fileHeader="Vrui Input Device Data File v3.0\n";
	char header[34];
	inputDeviceDataFile->read<char>(header,34);
	header[33]='\0';
	
	int fileVersion;
	if(strncmp(header,fileHeader,29)!=0)
		{
		/* Pre-versioning file version: */
		fileVersion=1;
		
		/* Old file format doesn't have the header text: */
		inputDeviceDataFile->setReadPosAbs(0);
		}
	else if(strcmp(header+29,"2.0\n")==0)
		{
		/* File version without ray direction and velocities: */
		fileVersion=2;
		}
	else if(strcmp(header+29,"3.0\n")==0)
		{
		/* File version with ray direction and velocities: */
		fileVersion=3;
		}
	else
		{
		header[32]='\0';
		std::cerr<<"Unsupported input device data file version "<<header+29<<std::endl;
		return 1;
		}
	
	/* Skip random seed value: */
	inputDeviceDataFile->read<unsigned int>();
	
	/* Read file header: */
	int numInputDevices=inputDeviceDataFile->read<int>();
	Vrui::InputDevice** inputDevices=new Vrui::InputDevice*[numInputDevices];
	int* deviceFeatureBaseIndices=new int[numInputDevices];
	std::vector<std::string> deviceFeatureNames;
	
	/* Initialize devices: */
	for(int i=0;i<numInputDevices;++i)
		{
		/* Read device's name and layout from file: */
		std::string name;
		if(fileVersion>=2)
			name=Misc::readCppString(*inputDeviceDataFile);
		else
			{
			/* Read a fixed-size string: */
			char nameBuffer[40];
			inputDeviceDataFile->read(nameBuffer,sizeof(nameBuffer));
			name=nameBuffer;
			}
		int trackType=inputDeviceDataFile->read<int>();
		int numButtons=inputDeviceDataFile->read<int>();
		int numValuators=inputDeviceDataFile->read<int>();
		
		/* Create new input device: */
		Vrui::InputDevice* newDevice=new Vrui::InputDevice;
		newDevice->set(name.c_str(),trackType,numButtons,numValuators);
		
		if(fileVersion<3)
			{
			Vrui::Vector deviceRayDirection;
			inputDeviceDataFile->read(deviceRayDirection.getComponents(),3);
			newDevice->setDeviceRay(deviceRayDirection,Vrui::Scalar(0));
			}
		
		/* Store the input device: */
		inputDevices[i]=newDevice;
		
		/* Read or create the device's feature names: */
		deviceFeatureBaseIndices[i]=int(deviceFeatureNames.size());
		if(fileVersion>=2)
			{
			/* Read feature names from file: */
			for(int j=0;j<newDevice->getNumFeatures();++j)
				deviceFeatureNames.push_back(Misc::readCppString(*inputDeviceDataFile));
			}
		else
			{
			/* Create default feature names: */
			for(int j=0;j<newDevice->getNumFeatures();++j)
				deviceFeatureNames.push_back(getDefaultFeatureName(Vrui::InputDeviceFeature(newDevice,j)));
			}
		}
	
	/* Read all data frames from the input device data file: */
	while(true)
		{
		/* Read the next time stamp: */
		double timeStamp;
		try
			{
			timeStamp=inputDeviceDataFile->read<double>();
			}
		catch(IO::File::ReadError)
			{
			/* At end of file */
			break;
			}
		
		std::cout<<"Time stamp: "<<std::fixed<<std::setw(8)<<std::setprecision(3)<<timeStamp;
		
		/* Read data for all input devices: */
		for(int device=0;device<numInputDevices;++device)
			{
			/* Update tracker state: */
			if(inputDevices[device]->getTrackType()!=Vrui::InputDevice::TRACK_NONE)
				{
				if(fileVersion>=3)
					{
					Vrui::Vector deviceRayDir;
					inputDeviceDataFile->read(deviceRayDir.getComponents(),3);
					Vrui::Scalar deviceRayStart=inputDeviceDataFile->read<Vrui::Scalar>();
					inputDevices[device]->setDeviceRay(deviceRayDir,deviceRayStart);
					}
				Vrui::TrackerState::Vector translation;
				inputDeviceDataFile->read(translation.getComponents(),3);
				Vrui::Scalar quat[4];
				inputDeviceDataFile->read(quat,4);
				inputDevices[device]->setTransformation(Vrui::TrackerState(translation,Vrui::TrackerState::Rotation(quat)));
				if(fileVersion>=3)
					{
					Vrui::Vector linearVelocity,angularVelocity;
					inputDeviceDataFile->read(linearVelocity.getComponents(),3);
					inputDeviceDataFile->read(angularVelocity.getComponents(),3);
					inputDevices[device]->setLinearVelocity(linearVelocity);
					inputDevices[device]->setAngularVelocity(angularVelocity);
					}
				}
			
			/* Update button states: */
			if(fileVersion>=3)
				{
				unsigned char buttonBits=0x00U;
				int numBits=0;
				for(int i=0;i<inputDevices[device]->getNumButtons();++i)
					{
					if(numBits==0)
						{
						buttonBits=inputDeviceDataFile->read<unsigned char>();
						numBits=8;
						}
					inputDevices[device]->setButtonState(i,(buttonBits&0x80U)!=0x00U);
					buttonBits<<=1;
					--numBits;
					}
				}
			else
				{
				for(int i=0;i<inputDevices[device]->getNumButtons();++i)
					{
					int buttonState=inputDeviceDataFile->read<int>();
					inputDevices[device]->setButtonState(i,buttonState);
					}
				}
			
			/* Update valuator states: */
			for(int i=0;i<inputDevices[device]->getNumValuators();++i)
				{
				double valuatorState=inputDeviceDataFile->read<double>();
				inputDevices[device]->setValuator(i,valuatorState);
				}
			}
		
		std::cout<<std::endl;
		}
	
	return 0;
	}
