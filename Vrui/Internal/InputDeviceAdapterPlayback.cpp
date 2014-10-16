/***********************************************************************
InputDeviceAdapterPlayback - Class to read input device states from a
pre-recorded file for playback and/or movie generation.
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

#include <Vrui/Internal/InputDeviceAdapterPlayback.h>

#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <Misc/Time.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/Endianness.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <IO/File.h>
#include <IO/OpenFile.h>
#include <Math/Constants.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Geometry/GeometryValueCoders.h>
#include <Sound/SoundPlayer.h>
#include <Vrui/Vrui.h>
#include <Vrui/InputDevice.h>
#include <Vrui/InputDeviceFeature.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/TextEventDispatcher.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/Internal/MouseCursorFaker.h>
#include <Vrui/VRWindow.h>
#include <Vrui/Internal/Vrui.h>
#include <Vrui/Internal/Config.h>
#ifdef VRUI_INPUTDEVICEADAPTERPLAYBACK_USE_KINECT
#include <Vrui/Internal/KinectPlayback.h>
#endif

namespace Vrui {

/*******************************************
Methods of class InputDeviceAdapterPlayback:
*******************************************/

InputDeviceAdapterPlayback::InputDeviceAdapterPlayback(InputDeviceManager* sInputDeviceManager,const Misc::ConfigurationFileSection& configFileSection)
	:InputDeviceAdapter(sInputDeviceManager),
	 inputDeviceDataFile(IO::openSeekableFile(configFileSection.retrieveString("./inputDeviceDataFileName").c_str())),
	 mouseCursorFaker(0),
	 synchronizePlayback(configFileSection.retrieveValue<bool>("./synchronizePlayback",false)),
	 quitWhenDone(configFileSection.retrieveValue<bool>("./quitWhenDone",false)),
	 soundPlayer(0),
	 #ifdef VRUI_INPUTDEVICEADAPTERPLAYBACK_USE_KINECT
	 kinectPlayer(0),
	 #endif
	 saveMovie(configFileSection.retrieveValue<bool>("./saveMovie",false)),
	 movieWindowIndex(0),movieWindow(0),movieFrameTimeInterval(1.0/30.0),
	 movieFrameStart(0),movieFrameOffset(0),
	 firstFrameCountdown(2),timeStamp(0.0),timeStampOffset(0.0),
	 nextTimeStamp(0.0),
	 nextMovieFrameTime(0.0),nextMovieFrameCounter(0),
	 done(false)
	{
	/* Read file header: */
	inputDeviceDataFile->setEndianness(Misc::LittleEndian);
	static const char* fileHeader="Vrui Input Device Data File v4.0\n";
	char header[34];
	inputDeviceDataFile->read<char>(header,34);
	header[33]='\0';
	
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
	else if(strcmp(header+29,"4.0\n")==0)
		{
		/* File version with text and text control events: */
		fileVersion=4;
		}
	else
		{
		header[32]='\0';
		Misc::throwStdErr("Vrui::InputDeviceAdapterPlayback: Unsupported input device data file version %s",header+29);
		}
	
	/* Read random seed value: */
	unsigned int randomSeed=inputDeviceDataFile->read<unsigned int>();
	setRandomSeed(randomSeed);
	
	/* Read number of saved input devices: */
	numInputDevices=inputDeviceDataFile->read<int>();
	inputDevices=new InputDevice*[numInputDevices];
	deviceFeatureBaseIndices=new int[numInputDevices];
	
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
		InputDevice* newDevice=inputDeviceManager->createInputDevice(name.c_str(),trackType,numButtons,numValuators,true);
		
		if(fileVersion<3)
			{
			/* Read the device ray direction: */
			Vector deviceRayDir;
			inputDeviceDataFile->read(deviceRayDir.getComponents(),3);
			newDevice->setDeviceRay(deviceRayDir,Scalar(0));
			}
		
		/* Initialize the new device's glyph from the current configuration file section: */
		Glyph& deviceGlyph=inputDeviceManager->getInputGraphManager()->getInputDeviceGlyph(newDevice);
		char deviceGlyphTypeTag[20];
		snprintf(deviceGlyphTypeTag,sizeof(deviceGlyphTypeTag),"./device%dGlyphType",i);
		char deviceGlyphMaterialTag[20];
		snprintf(deviceGlyphMaterialTag,sizeof(deviceGlyphMaterialTag),"./device%dGlyphMaterial",i);
		deviceGlyph.configure(configFileSection,deviceGlyphTypeTag,deviceGlyphMaterialTag);
		
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
				deviceFeatureNames.push_back(getDefaultFeatureName(InputDeviceFeature(newDevice,j)));
			}
		}
	
	/* Check if the user wants to use a fake mouse cursor: */
	int fakeMouseCursorDevice=configFileSection.retrieveValue<int>("./fakeMouseCursorDevice",-1);
	if(fakeMouseCursorDevice>=0)
		{
		/* Read the cursor file name and nominal size: */
		std::string mouseCursorImageFileName=VRUI_INTERNAL_CONFIG_SHAREDIR;
		mouseCursorImageFileName.append("/Textures/Cursor.Xcur");
		mouseCursorImageFileName=configFileSection.retrieveString("./mouseCursorImageFileName",mouseCursorImageFileName.c_str());
		unsigned int mouseCursorNominalSize=configFileSection.retrieveValue<unsigned int>("./mouseCursorNominalSize",24);
		
		/* Create the mouse cursor faker: */
		mouseCursorFaker=new MouseCursorFaker(inputDevices[fakeMouseCursorDevice],mouseCursorImageFileName.c_str(),mouseCursorNominalSize);
		mouseCursorFaker->setCursorSize(configFileSection.retrieveValue<Size>("./mouseCursorSize",mouseCursorFaker->getCursorSize()));
		mouseCursorFaker->setCursorHotspot(configFileSection.retrieveValue<Vector>("./mouseCursorHotspot",mouseCursorFaker->getCursorHotspot()));
		}
	
	/* Read time stamp of first data frame: */
	try
		{
		nextTimeStamp=inputDeviceDataFile->read<double>();
		
		/* Request an update for the next frame: */
		requestUpdate();
		}
	catch(IO::File::ReadError)
		{
		done=true;
		nextTimeStamp=Math::Constants<double>::max;
		
		if(quitWhenDone)
			{
			/* Request exiting the program: */
			shutdown();
			}
		}
	
	/* Check if the user wants to play back a commentary sound track: */
	std::string soundFileName=configFileSection.retrieveString("./soundFileName","");
	if(!soundFileName.empty())
		{
		try
			{
			/* Create a sound player for the given sound file name: */
			soundPlayer=new Sound::SoundPlayer(soundFileName.c_str());
			}
		catch(std::runtime_error error)
			{
			/* Print a message, but carry on: */
			std::cerr<<"InputDeviceAdapterPlayback: Disabling sound playback due to exception "<<error.what()<<std::endl;
			}
		}
	
	#ifdef VRUI_INPUTDEVICEADAPTERPLAYBACK_USE_KINECT
	/* Check if the user wants to play back 3D video: */
	std::string kinectPlayerSectionName=configFileSection.retrieveString("./kinectPlayer","");
	if(!kinectPlayerSectionName.empty())
		{
		/* Go to the Kinect player's section: */
		Misc::ConfigurationFileSection kinectPlayerSection=configFileSection.getSection(kinectPlayerSectionName.c_str());
		kinectPlayer=new KinectPlayback(nextTimeStamp,kinectPlayerSection);
		}
	#endif
	
	/* Check if the user wants to save a movie: */
	if(saveMovie)
		{
		/* Read the movie image file name template: */
		movieFileNameTemplate=configFileSection.retrieveString("./movieFileNameTemplate");
		
		/* Check if the name template has the correct format: */
		int numConversions=0;
		bool hasIntConversion=false;
		for(std::string::const_iterator mfntIt=movieFileNameTemplate.begin();mfntIt!=movieFileNameTemplate.end();++mfntIt)
			{
			if(*mfntIt=='%')
				{
				++mfntIt;
				if(*mfntIt!='%')
					{
					++numConversions;
					
					/* Skip width modifiers: */
					while(isdigit(*mfntIt))
						++mfntIt;
					
					/* Check for integer conversion: */
					if(*mfntIt=='d')
						hasIntConversion=true;
					}
				}
			else if(*mfntIt=='/') // Only accept conversions in the file name part
				hasIntConversion=false;
			}
		if(numConversions!=1||!hasIntConversion)
			Misc::throwStdErr("InputDeviceAdapterPlayback::InputDeviceAdapterPlayback: movie file name template \"%s\" does not have exactly one %%d conversion",movieFileNameTemplate.c_str());
		
		/* Get the index of the window from which to save the frames: */
		movieWindowIndex=configFileSection.retrieveValue<int>("./movieWindowIndex",movieWindowIndex);
		
		/* Get the intended frame rate for the movie: */
		movieFrameTimeInterval=1.0/configFileSection.retrieveValue<double>("./movieFrameRate",1.0/movieFrameTimeInterval);
		
		/* Get the number of movie frames to skip: */
		movieFrameStart=configFileSection.retrieveValue<int>("./movieSkipFrames",movieFrameStart);
		
		/* Get the index of the first movie frame: */
		movieFrameOffset=configFileSection.retrieveValue<int>("./movieFirstFrameIndex",movieFrameOffset);
		}
	}

InputDeviceAdapterPlayback::~InputDeviceAdapterPlayback(void)
	{
	delete mouseCursorFaker;
	delete soundPlayer;
	#ifdef VRUI_INPUTDEVICEADAPTERPLAYBACK_USE_KINECT
	delete kinectPlayer;
	#endif
	delete[] deviceFeatureBaseIndices;
	}

std::string InputDeviceAdapterPlayback::getFeatureName(const InputDeviceFeature& feature) const
	{
	/* Find the input device owning the given feature: */
	int featureBaseIndex=-1;
	for(int deviceIndex=0;deviceIndex<numInputDevices;++deviceIndex)
		{
		if(inputDevices[deviceIndex]==feature.getDevice())
			{
			featureBaseIndex=deviceFeatureBaseIndices[deviceIndex];
			break;
			}
		}
	if(featureBaseIndex<0)
		Misc::throwStdErr("InputDeviceAdapterPlayback::getFeatureName: Unknown device %s",feature.getDevice()->getDeviceName());
	
	/* Return the feature name: */
	return deviceFeatureNames[featureBaseIndex+feature.getFeatureIndex()];
	}

int InputDeviceAdapterPlayback::getFeatureIndex(InputDevice* device,const char* featureName) const
	{
	/* Find the input device owning the given feature: */
	int featureBaseIndex=-1;
	for(int deviceIndex=0;deviceIndex<numInputDevices;++deviceIndex)
		{
		if(inputDevices[deviceIndex]==device)
			{
			featureBaseIndex=deviceFeatureBaseIndices[deviceIndex];
			break;
			}
		}
	if(featureBaseIndex<0)
		Misc::throwStdErr("InputDeviceAdapterPlayback::getFeatureIndex: Unknown device %s",device->getDeviceName());
	
	/* Compare the given feature name against the device's feature names: */
	for(int i=0;i<device->getNumFeatures();++i)
		if(deviceFeatureNames[featureBaseIndex+i]==featureName)
			return i;
	
	return -1;
	}

void InputDeviceAdapterPlayback::updateInputDevices(void)
	{
	/* Do nothing if at end of file: */
	if(done)
		return;
	
	/* Check if this is the first real Vrui frame: */
	if(firstFrameCountdown>0U)
		{
		--firstFrameCountdown;
		if(firstFrameCountdown==0U)
			{
			if(synchronizePlayback)
				{
				/* Calculate the offset between the saved timestamps and the system's wall clock time: */
				Misc::Time rt=Misc::Time::now();
				double realTime=double(rt.tv_sec)+double(rt.tv_nsec)/1000000000.0;
				timeStampOffset=nextTimeStamp-realTime;
				}
			
			/* Start the sound player, if there is one: */
			if(soundPlayer!=0)
				soundPlayer->start();
			
			if(saveMovie)
				{
				/* Get a pointer to the window from which to save movie frames: */
				if(movieWindowIndex>=0&&movieWindowIndex<getNumWindows())
					movieWindow=getWindow(movieWindowIndex);
				else
					std::cerr<<"InputDeviceAdapterPlayback: Not saving movie due to invalid movie window index "<<movieWindowIndex<<std::endl;
				
				/* Calculate the first time at which to save a frame: */
				nextMovieFrameTime=nextTimeStamp+movieFrameTimeInterval*0.5;
				}
			}
		}
	
	if(synchronizePlayback)
		{
		/* Check if there is positive drift between the system's offset wall clock time and the next time stamp: */
		Misc::Time rt=Misc::Time::now();
		double realTime=double(rt.tv_sec)+double(rt.tv_nsec)/1000000000.0;
		double delta=nextTimeStamp-(realTime+timeStampOffset);
		if(delta>0.0)
			{
			/* Block to correct the drift: */
			vruiDelay(delta);
			}
		}
	
	/* Update all input devices: */
	for(int device=0;device<numInputDevices;++device)
		{
		/* Update tracker state: */
		if(inputDevices[device]->getTrackType()!=InputDevice::TRACK_NONE)
			{
			if(fileVersion>=3)
				{
				Vector deviceRayDir;
				inputDeviceDataFile->read(deviceRayDir.getComponents(),3);
				Scalar deviceRayStart=inputDeviceDataFile->read<Scalar>();
				inputDevices[device]->setDeviceRay(deviceRayDir,deviceRayStart);
				}
			TrackerState::Vector translation;
			inputDeviceDataFile->read(translation.getComponents(),3);
			Scalar quat[4];
			inputDeviceDataFile->read(quat,4);
			inputDevices[device]->setTransformation(TrackerState(translation,TrackerState::Rotation(quat)));
			if(fileVersion>=3)
				{
				Vector linearVelocity,angularVelocity;
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
	
	if(fileVersion>=4)
		{
		/* Read and enqueue all text and text control events: */
		inputDeviceManager->getTextEventDispatcher()->readEventQueues(*inputDeviceDataFile);
		}
	
	/* Read time stamp of next data frame: */
	try
		{
		nextTimeStamp=inputDeviceDataFile->read<double>();
		
		/* Request a synchronized update for the next frame: */
		synchronize(nextTimeStamp,false);
		requestUpdate();
		}
	catch(IO::File::ReadError)
		{
		done=true;
		nextTimeStamp=Math::Constants<double>::max;
		
		if(quitWhenDone)
			{
			/* Request exiting the program: */
			shutdown();
			}
		}
	
	#ifdef VRUI_INPUTDEVICEADAPTERPLAYBACK_USE_KINECT
	if(kinectPlayer!=0)
		{
		/* Update the 3D video player and prepare it for the next frame: */
		kinectPlayer->frame(timeStamp,nextTimeStamp);
		}
	#endif
	
	if(saveMovie&&movieWindow!=0)
		{
		/* Copy the last saved screenshot if multiple movie frames needed to be taken during the last Vrui frame: */
		while(nextMovieFrameTime<timeStamp&&nextMovieFrameCounter>movieFrameStart)
			{
			/* Copy the last saved screenshot: */
			pid_t childPid=fork();
			if(childPid==0)
				{
				/* Create the old and new file names: */
				char oldImageFileName[1024];
				snprintf(oldImageFileName,sizeof(oldImageFileName),movieFileNameTemplate.c_str(),nextMovieFrameCounter-movieFrameStart+movieFrameOffset-1);
				char imageFileName[1024];
				snprintf(imageFileName,sizeof(imageFileName),movieFileNameTemplate.c_str(),nextMovieFrameCounter-movieFrameStart+movieFrameOffset);
				
				/* Execute the cp system command: */
				char* cpArgv[10];
				int cpArgc=0;
				cpArgv[cpArgc++]=const_cast<char*>("/bin/cp");
				cpArgv[cpArgc++]=oldImageFileName;
				cpArgv[cpArgc++]=imageFileName;
				cpArgv[cpArgc++]=0;
				execvp(cpArgv[0],cpArgv);
				}
			else
				{
				/* Wait for the copy process to finish: */
				waitpid(childPid,0,0);
				}
			
			/* Advance the frame counters: */
			nextMovieFrameTime+=movieFrameTimeInterval;
			++nextMovieFrameCounter;
			}
		
		if(nextTimeStamp>nextMovieFrameTime)
			{
			if(nextMovieFrameCounter>=movieFrameStart)
				{
				/* Request a screenshot from the movie window: */
				char imageFileName[1024];
				snprintf(imageFileName,sizeof(imageFileName),movieFileNameTemplate.c_str(),nextMovieFrameCounter-movieFrameStart+movieFrameOffset);
				movieWindow->requestScreenshot(imageFileName);
				}
			
			/* Advance the movie frame counters: */
			nextMovieFrameTime+=movieFrameTimeInterval;
			++nextMovieFrameCounter;
			}
		}
	}

#ifdef VRUI_INPUTDEVICEADAPTERPLAYBACK_USE_KINECT

void InputDeviceAdapterPlayback::glRenderAction(GLContextData& contextData) const
	{
	if(kinectPlayer!=0)
		{
		/* Let the 3D video player render its current frames: */
		kinectPlayer->glRenderAction(contextData);
		}
	}

#endif

}
