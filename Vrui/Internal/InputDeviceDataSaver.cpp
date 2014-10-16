/***********************************************************************
InputDeviceDataSaver - Class to save input device data to a file for
later playback.
Copyright (c) 2004-2014 Oliver Kreylos

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

#include <Vrui/Internal/InputDeviceDataSaver.h>

#include <iostream>
#include <Misc/StringMarshaller.h>
#include <Misc/CreateNumberedFileName.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <IO/File.h>
#include <IO/OpenFile.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Rotation.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Sound/SoundDataFormat.h>
#include <Sound/SoundRecorder.h>
#include <Vrui/Geometry.h>
#include <Vrui/InputDevice.h>
#include <Vrui/InputDeviceFeature.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/TextEventDispatcher.h>
#ifdef VRUI_INPUTDEVICEDATASAVER_USE_KINECT
#include <Vrui/Internal/KinectRecorder.h>
#endif

namespace Vrui {

/*************************************
Methods of class InputDeviceDataSaver:
*************************************/

InputDeviceDataSaver::InputDeviceDataSaver(const Misc::ConfigurationFileSection& configFileSection,InputDeviceManager& inputDeviceManager,TextEventDispatcher* sTextEventDispatcher,unsigned int randomSeed)
	:inputDeviceDataFile(IO::openFile(Misc::createNumberedFileName(configFileSection.retrieveString("./inputDeviceDataFileName"),4).c_str(),IO::File::WriteOnly)),
	 numInputDevices(inputDeviceManager.getNumInputDevices()),
	 inputDevices(new InputDevice*[numInputDevices]),
	 textEventDispatcher(sTextEventDispatcher),
	 soundRecorder(0),
	 #ifdef VRUI_INPUTDEVICEDATASAVER_USE_KINECT
	 kinectRecorder(0),
	 #endif
	 firstFrameCountdown(2)
	{
	/* Write a file identification header: */
	inputDeviceDataFile->setEndianness(Misc::LittleEndian);
	static const char* fileHeader="Vrui Input Device Data File v4.0\n";
	inputDeviceDataFile->write<char>(fileHeader,34);
	
	/* Save the random number seed: */
	inputDeviceDataFile->write<unsigned int>(randomSeed);
	
	/* Save number of input devices: */
	inputDeviceDataFile->write<int>(numInputDevices);
	
	/* Save layout and feature names of all input devices in the input device manager: */
	for(int i=0;i<numInputDevices;++i)
		{
		/* Get pointer to the input device: */
		inputDevices[i]=inputDeviceManager.getInputDevice(i);
		
		/* Save input device's name and layout: */
		Misc::writeCString(inputDevices[i]->getDeviceName(),*inputDeviceDataFile);
		inputDeviceDataFile->write<int>(inputDevices[i]->getTrackType());
		inputDeviceDataFile->write<int>(inputDevices[i]->getNumButtons());
		inputDeviceDataFile->write<int>(inputDevices[i]->getNumValuators());
		
		/* Save input device's feature names: */
		for(int j=0;j<inputDevices[i]->getNumFeatures();++j)
			{
			std::string featureName=inputDeviceManager.getFeatureName(InputDeviceFeature(inputDevices[i],j));
			Misc::writeCppString(featureName,*inputDeviceDataFile);
			}
		}
	
	/* Check if the user wants to record a commentary track: */
	std::string soundFileName=configFileSection.retrieveString("./soundFileName","");
	if(!soundFileName.empty())
		{
		try
			{
			/* Create a sound data format for recording: */
			Sound::SoundDataFormat soundFormat;
			soundFormat.bitsPerSample=configFileSection.retrieveValue<int>("./sampleResolution",soundFormat.bitsPerSample);
			soundFormat.samplesPerFrame=configFileSection.retrieveValue<int>("./numChannels",soundFormat.samplesPerFrame);
			soundFormat.framesPerSecond=configFileSection.retrieveValue<int>("./sampleRate",soundFormat.framesPerSecond);
			
			/* Create a sound recorder for the given sound file name: */
			std::string soundDeviceName=configFileSection.retrieveValue<std::string>("./soundDeviceName","default");
			soundRecorder=new Sound::SoundRecorder(soundDeviceName.c_str(),soundFormat,Misc::createNumberedFileName(soundFileName,4).c_str());
			}
		catch(std::runtime_error error)
			{
			/* Print a message, but carry on: */
			std::cerr<<"InputDeviceDataSaver: Disabling sound recording due to exception "<<error.what()<<std::endl;
			}
		}
	
	#ifdef VRUI_INPUTDEVICEDATASAVER_USE_KINECT
	/* Check if the user wants to record 3D video: */
	std::string kinectRecorderSectionName=configFileSection.retrieveString("./kinectRecorder","");
	if(!kinectRecorderSectionName.empty())
		{
		/* Go to the Kinect recorder's section: */
		Misc::ConfigurationFileSection kinectRecorderSection=configFileSection.getSection(kinectRecorderSectionName.c_str());
		kinectRecorder=new KinectRecorder(kinectRecorderSection);
		}
	#endif
	}

InputDeviceDataSaver::~InputDeviceDataSaver(void)
	{
	delete[] inputDevices;
	delete soundRecorder;
	#ifdef VRUI_INPUTDEVICEDATASAVER_USE_KINECT
	delete kinectRecorder;
	#endif
	}

void InputDeviceDataSaver::saveCurrentState(double currentTimeStamp)
	{
	/* Check if this is the first real Vrui frame: */
	if(firstFrameCountdown>0U)
		{
		--firstFrameCountdown;
		if(firstFrameCountdown==0U)
			{
			if(soundRecorder!=0)
				soundRecorder->start();
			#ifdef VRUI_INPUTDEVICEDATASAVER_USE_KINECT
			if(kinectRecorder!=0)
				kinectRecorder->start(currentTimeStamp);
			#endif
			}
		}
	
	/* Write current time stamp: */
	inputDeviceDataFile->write(currentTimeStamp);
	
	/* Write state of all input devices: */
	for(int i=0;i<numInputDevices;++i)
		{
		/* Write input device's tracker state: */
		if(inputDevices[i]->getTrackType()!=InputDevice::TRACK_NONE)
			{
			inputDeviceDataFile->write(inputDevices[i]->getDeviceRayDirection().getComponents(),3);
			inputDeviceDataFile->write(inputDevices[i]->getDeviceRayStart());
			const TrackerState& t=inputDevices[i]->getTransformation();
			inputDeviceDataFile->write(t.getTranslation().getComponents(),3);
			inputDeviceDataFile->write(t.getRotation().getQuaternion(),4);
			inputDeviceDataFile->write(inputDevices[i]->getLinearVelocity().getComponents(),3);
			inputDeviceDataFile->write(inputDevices[i]->getAngularVelocity().getComponents(),3);
			}
		
		/* Write input device's button states: */
		unsigned char buttonBits=0x00U;
		int numBits=0;
		for(int j=0;j<inputDevices[i]->getNumButtons();++j)
			{
			buttonBits<<=1;
			if(inputDevices[i]->getButtonState(j))
				buttonBits|=0x01U;
			if(++numBits==8)
				{
				inputDeviceDataFile->write(buttonBits);
				buttonBits=0x00U;
				numBits=0;
				}
			}
		if(numBits!=0)
			{
			buttonBits<<=8-numBits;
			inputDeviceDataFile->write(buttonBits);
			}
		
		/* Write input device's valuator states: */
		for(int j=0;j<inputDevices[i]->getNumValuators();++j)
			{
			double valuatorState=inputDevices[i]->getValuator(j);
			inputDeviceDataFile->write(valuatorState);
			}
		}
	
	/* Write all enqueued text and text control events: */
	textEventDispatcher->writeEventQueues(*inputDeviceDataFile);
	}

}
