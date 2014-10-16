/***********************************************************************
InputDeviceAdapterTrackd - Class to connect a trackd tracking daemon to
a Vrui application.
Copyright (c) 2013 Oliver Kreylos

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

#include <Vrui/Internal/InputDeviceAdapterTrackd.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/CompoundValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Rotation.h>
#include <Geometry/GeometryValueCoders.h>
#include <Vrui/Vrui.h>
#include <Vrui/InputDevice.h>
#include <Vrui/InputDeviceFeature.h>

namespace Vrui {

/*****************************************
Methods of class InputDeviceAdapterTrackd:
*****************************************/

void InputDeviceAdapterTrackd::createInputDevice(int deviceIndex,const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Call base class method to initialize the input device: */
	InputDeviceAdapterIndexMap::createInputDevice(deviceIndex,configFileSection);
	
	/* Read the list of button names for this device: */
	/* Read the names of all button features: */
	typedef std::vector<std::string> StringList;
	StringList tempButtonNames=configFileSection.retrieveValue<StringList>("./buttonNames",StringList());
	int buttonIndex=0;
	for(StringList::iterator bnIt=tempButtonNames.begin();bnIt!=tempButtonNames.end()&&buttonIndex<inputDevices[deviceIndex]->getNumButtons();++bnIt,++buttonIndex)
		{
		/* Store the button name: */
		buttonNames.push_back(*bnIt);
		}
	for(;buttonIndex<inputDevices[deviceIndex]->getNumButtons();++buttonIndex)
		{
		char buttonName[40];
		snprintf(buttonName,sizeof(buttonName),"Button%d",buttonIndex);
		buttonNames.push_back(buttonName);
		}
	
	/* Read the names of all valuator features: */
	StringList tempValuatorNames=configFileSection.retrieveValue<StringList>("./valuatorNames",StringList());
	int valuatorIndex=0;
	for(StringList::iterator vnIt=tempValuatorNames.begin();vnIt!=tempValuatorNames.end()&&valuatorIndex<inputDevices[deviceIndex]->getNumValuators();++vnIt,++valuatorIndex)
		{
		/* Store the valuator name: */
		valuatorNames.push_back(*vnIt);
		}
	for(;valuatorIndex<inputDevices[deviceIndex]->getNumValuators();++valuatorIndex)
		{
		char valuatorName[40];
		snprintf(valuatorName,sizeof(valuatorName),"Valuator%d",valuatorIndex);
		valuatorNames.push_back(valuatorName);
		}
	}

void* InputDeviceAdapterTrackd::spinPollThreadMethod(void)
	{
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	Misc::UInt32 lastSensorTime[2]={0U,0U};
	Misc::UInt32 lastControllerTime[2]={0U,0U};
	while(runSpinPollThread)
		{
		/* Spin until the next time the sensor data or controller data time stamps change: */
		while(runSpinPollThread
		      &&sensorHeader->dataTimeStamp[0]==lastControllerTime[0]&&sensorHeader->dataTimeStamp[1]==lastSensorTime[1]
		      &&controllerHeader->dataTimeStamp[0]==lastControllerTime[0]&&controllerHeader->dataTimeStamp[1]==lastControllerTime[1])
			;
		
		/* Trigger a new Vrui frame: */
		requestUpdate();
		
		/* Update the data time stamps: */
		lastSensorTime[0]=sensorHeader->dataTimeStamp[0];
		lastSensorTime[1]=sensorHeader->dataTimeStamp[1];
		lastControllerTime[0]=controllerHeader->dataTimeStamp[0];
		lastControllerTime[1]=controllerHeader->dataTimeStamp[1];
		}
	
	return 0;
	}

InputDeviceAdapterTrackd::InputDeviceAdapterTrackd(InputDeviceManager* sInputDeviceManager,const Misc::ConfigurationFileSection& configFileSection)
	:InputDeviceAdapterIndexMap(sInputDeviceManager),
	 sensorHeader((SensorHeader*)-1),controllerHeader((ControllerHeader*)-1),
	 sensors(0),buttons(0),valuators(0),
	 calibrationTransformation(OGTransform::identity),
	 updateInterval(0.0),
	 runSpinPollThread(false)
	{
	/* Retrieve the shared memory keys for the sensor and controller segments from the configuration file: */
	key_t sensorMemoryKey=key_t(configFileSection.retrieveValue<int>("./sensorMemoryKey"));
	key_t controllerMemoryKey=key_t(configFileSection.retrieveValue<int>("./controllerMemoryKey"));
	
	/* Try attaching to the sensor shared memory segment: */
	int sensorMemoryID=shmget(sensorMemoryKey,sizeof(SensorHeader),0);
	if(sensorMemoryID<0)
		Misc::throwStdErr("InputDeviceAdapterTrackd::InputDeviceAdapterTrackd: Unable to access shared sensor memory segment using key %x",int(sensorMemoryKey));
	sensorHeader=static_cast<SensorHeader*>(shmat(sensorMemoryID,0,SHM_RDONLY));
	if(sensorHeader==(SensorHeader*)-1)
		Misc::throwStdErr("InputDeviceAdapterTrackd::InputDeviceAdapterTrackd: Unable to attach to shared sensor memory segment using key %x",int(sensorMemoryKey));
	
	/* Try attaching to the controller shared memory segment: */
	int controllerMemoryID=shmget(controllerMemoryKey,sizeof(ControllerHeader),0);
	if(controllerMemoryID<0)
		{
		shmdt(sensorHeader);
		sensorHeader=(SensorHeader*)-1;
		Misc::throwStdErr("InputDeviceAdapterTrackd::InputDeviceAdapterTrackd: Unable to access shared controller memory segment using key %x",int(controllerMemoryKey));
		}
	controllerHeader=static_cast<ControllerHeader*>(shmat(controllerMemoryID,0,SHM_RDONLY));
	if(controllerHeader==(ControllerHeader*)-1)
		{
		shmdt(sensorHeader);
		sensorHeader=(SensorHeader*)-1;
		Misc::throwStdErr("InputDeviceAdapterTrackd::InputDeviceAdapterTrackd: Unable to attach to shared controller memory segment using key %x",int(controllerMemoryKey));
		}
	
	/* Initialize the sensor data pointer array: */
	sensors=new SensorData*[sensorHeader->numSensors];
	for(unsigned int i=0;i<sensorHeader->numSensors;++i)
		{
		/* Set the i-th pointer to the start of the i-th sensor data, accounting for additional unknown entries at the ends of the sensor data structure: */
		sensors[i]=reinterpret_cast<SensorData*>(reinterpret_cast<char*>(sensorHeader)+sensorHeader->sensorsOffset+sensorHeader->sensorDataSize*i);
		}
	
	/* Initialize the button and valuator array pointers: */
	buttons=reinterpret_cast<Misc::SInt32*>(reinterpret_cast<char*>(controllerHeader)+controllerHeader->buttonsOffset);
	valuators=reinterpret_cast<Misc::Float32*>(reinterpret_cast<char*>(controllerHeader)+controllerHeader->valuatorsOffset);
	
	try
		{
		/* Initialize input device adapter: */
		InputDeviceAdapterIndexMap::initializeAdapter(sensorHeader->numSensors,controllerHeader->numButtons,controllerHeader->numValuators,configFileSection);
		}
	catch(...)
		{
		/* Clean up and re-throw the exception: */
		shmdt(sensorHeader);
		sensorHeader=(SensorHeader*)-1;
		shmdt(controllerHeader);
		controllerHeader=(ControllerHeader*)-1;
		delete[] sensors;
		
		throw;
		}
	
	/* Read the calibration transformation: */
	calibrationTransformation=configFileSection.retrieveValue<OGTransform>("./calibrationTransformation",calibrationTransformation);
	
	/* Read the update interval: */
	double updateRate=configFileSection.retrieveValue<double>("./updateRate",0.0);
	if(updateRate==0.0)
		updateInterval=0.0;
	else
		updateInterval=1.0/updateRate;
	
	/* Check if the configuration requested spin polling: */
	if(configFileSection.retrieveValue<bool>("./spinPoll",false))
		{
		/* Start the spin polling thread: */
		runSpinPollThread=true;
		spinPollThread.start(this,&InputDeviceAdapterTrackd::spinPollThreadMethod);
		}
	}

InputDeviceAdapterTrackd::~InputDeviceAdapterTrackd(void)
	{
	if(runSpinPollThread)
		{
		/* Shut down the spin polling thread: */
		runSpinPollThread=false;
		spinPollThread.join();
		}
	
	/* Detach from the shared memory segments: */
	if(sensorHeader!=(SensorHeader*)-1)
		shmdt(sensorHeader);
	if(controllerHeader!=(ControllerHeader*)-1)
		shmdt(controllerHeader);
	
	/* Delete the sensor data pointer array: */
	delete[] sensors;
	}

std::string InputDeviceAdapterTrackd::getFeatureName(const InputDeviceFeature& feature) const
	{
	/* Find the input device owning the given feature: */
	bool deviceFound=false;
	int buttonIndexBase=0;
	int valuatorIndexBase=0;
	for(int deviceIndex=0;deviceIndex<numInputDevices;++deviceIndex)
		{
		if(inputDevices[deviceIndex]==feature.getDevice())
			{
			deviceFound=true;
			break;
			}
		
		/* Go to the next device: */
		buttonIndexBase+=inputDevices[deviceIndex]->getNumButtons();
		valuatorIndexBase+=inputDevices[deviceIndex]->getNumValuators();
		}
	if(!deviceFound)
		Misc::throwStdErr("InputDeviceAdapterTrackd::getFeatureName: Unknown device %s",feature.getDevice()->getDeviceName());
	
	/* Check whether the feature is a button or a valuator: */
	std::string result;
	if(feature.isButton())
		{
		/* Return the button feature's name: */
		result=buttonNames[buttonIndexBase+feature.getIndex()];
		}
	if(feature.isValuator())
		{
		/* Return the valuator feature's name: */
		result=valuatorNames[valuatorIndexBase+feature.getIndex()];
		}
	
	return result;
	}

int InputDeviceAdapterTrackd::getFeatureIndex(InputDevice* device,const char* featureName) const
	{
	/* Find the input device owning the given feature: */
	bool deviceFound=false;
	int buttonIndexBase=0;
	int valuatorIndexBase=0;
	for(int deviceIndex=0;deviceIndex<numInputDevices;++deviceIndex)
		{
		if(inputDevices[deviceIndex]==device)
			{
			deviceFound=true;
			break;
			}
		
		/* Go to the next device: */
		buttonIndexBase+=inputDevices[deviceIndex]->getNumButtons();
		valuatorIndexBase+=inputDevices[deviceIndex]->getNumValuators();
		}
	if(!deviceFound)
		Misc::throwStdErr("InputDeviceAdapterTrackd::getFeatureIndex: Unknown device %s",device->getDeviceName());
	
	/* Check if the feature names a button or a valuator: */
	for(int buttonIndex=0;buttonIndex<device->getNumButtons();++buttonIndex)
		if(buttonNames[buttonIndexBase+buttonIndex]==featureName)
			return device->getButtonFeatureIndex(buttonIndex);
	for(int valuatorIndex=0;valuatorIndex<device->getNumValuators();++valuatorIndex)
		if(valuatorNames[valuatorIndexBase+valuatorIndex]==featureName)
			return device->getValuatorFeatureIndex(valuatorIndex);
	
	return -1;
	}

void InputDeviceAdapterTrackd::updateInputDevices(void)
	{
	for(int deviceIndex=0;deviceIndex<numInputDevices;++deviceIndex)
		{
		/* Get pointer to the input device: */
		InputDevice* device=inputDevices[deviceIndex];
		
		/* Don't update tracker-related state for devices that are not tracked: */
		if(trackerIndexMapping[deviceIndex]>=0)
			{
			/* Get device's tracker state from sensor shared memory segment: */
			SensorData& sd=*sensors[trackerIndexMapping[deviceIndex]];
			
			/*****************************************************************
			Construct device's transformation:
			*****************************************************************/
			
			/* Translation vector is straightforward: */
			Vector translation=Vector(Scalar(sd.position[0]),Scalar(sd.position[1]),Scalar(sd.position[2]));
			
			/* To assemble the orientation, we assume all angles are in degrees, and the order of rotations is as follows: */
			Rotation rotation=Rotation::rotateZ(Math::rad(Scalar(sd.angles[0])));
			rotation*=Rotation::rotateX(Math::rad(Scalar(sd.angles[1])));
			rotation*=Rotation::rotateY(Math::rad(Scalar(sd.angles[2])));
			
			/* Calibrate the device's position and orientation from the trackd daemon's space to Vrui's physical space: */
			OGTransform calibratedTransformation=calibrationTransformation;
			calibratedTransformation*=OGTransform(translation,rotation,Scalar(1));
			
			/* Calibrate and set the device's transformation: */
			device->setTransformation(TrackerState(calibratedTransformation.getTranslation(),calibratedTransformation.getRotation()));
			
			/* Set device's linear and angular velocities to zero because we don't know any better: */
			device->setLinearVelocity(Vector::zero);
			device->setAngularVelocity(Vector::zero);
			}
		
		/* Update button states: */
		for(int i=0;i<device->getNumButtons();++i)
			device->setButtonState(i,buttons[buttonIndexMapping[deviceIndex][i]]);
		
		/* Update valuator states: */
		for(int i=0;i<device->getNumValuators();++i)
			device->setValuator(i,valuators[valuatorIndexMapping[deviceIndex][i]]);
		}
	
	/* Schedule the next Vrui frame at the update interval if asked to do so: */
	if(updateInterval!=0.0)
		Vrui::scheduleUpdate(getApplicationTime()+updateInterval);
	}

}
