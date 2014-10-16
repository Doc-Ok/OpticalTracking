/***********************************************************************
HIDDevice - VR device driver class for generic input devices supported
by the Linux HID event interface. Reports buttons and absolute axes.
Copyright (c) 2004-2014 Oliver Kreylos

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

#ifndef __USE_GNU
#define __USE_GNU
#include <dirent.h>
#undef __USE_GNU
#else
#include <dirent.h>
#endif
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <VRDeviceDaemon/Config.h>

#if !VRDEVICEDAEMON_CONFIG_INPUT_H_HAS_STRUCTS

/*******************************************************************
Classes to deal with HID devices (should all be defined in input.h):
*******************************************************************/

struct input_id
	{
	/* Elements: */
	public:
	unsigned short bustype;
	unsigned short vendor;
	unsigned short product;
	unsigned short version;
	};

struct input_absinfo
	{
	/* Elements: */
	public:
	int value;
	int minimum;
	int maximum;
	int fuzz;
	int flat;
	};

#endif

/****************
Helper functions:
****************/

int isEventFile(const struct dirent* directoryEntry)
	{
	return strncmp(directoryEntry->d_name,"event",5)==0;
	}

/**************************
Methods of class HIDDevice:
**************************/

int HIDDevice::findDevice(int vendorId,int productId)
	{
	/* Create list of all available /dev/input/eventX devices, in numerical order: */
	struct dirent** eventFiles=0;
	int numEventFiles=scandir("/dev/input",&eventFiles,isEventFile,versionsort);
	
	/* Check all event files for the wanted device: */
	int deviceFd=-1;
	for(int eventFileIndex=0;eventFileIndex<numEventFiles;++eventFileIndex)
		{
		/* Open the event file: */
		char eventFileName[256];
		snprintf(eventFileName,sizeof(eventFileName),"/dev/input/%s",eventFiles[eventFileIndex]->d_name);
		int eventFd=open(eventFileName,O_RDONLY);
		if(eventFd>=0)
			{
			/* Get device information: */
			input_id deviceInformation;
			if(ioctl(eventFd,EVIOCGID,&deviceInformation)>=0)
				{
				if(deviceInformation.vendor==vendorId&&deviceInformation.product==productId)
					{
					/* We have a winner! */
					deviceFd=eventFd;
					break;
					}
				}
			
			/* This is not the device you are looking for, go to the next: */
			close(eventFd);
			}
		}
	
	/* Destroy list of event files: */
	for(int i=0;i<numEventFiles;++i)
		free(eventFiles[i]);
	free(eventFiles);
	
	/* Return found device file descriptor, or -1: */
	return deviceFd;
	}

int HIDDevice::findDevice(const char* deviceName)
	{
	/* Create list of all available /dev/input/eventX devices, in numerical order: */
	struct dirent** eventFiles=0;
	int numEventFiles=scandir("/dev/input",&eventFiles,isEventFile,versionsort);
	
	/* Check all event files for the wanted device: */
	int deviceFd=-1;
	for(int eventFileIndex=0;eventFileIndex<numEventFiles;++eventFileIndex)
		{
		/* Open the event file: */
		char eventFileName[256];
		snprintf(eventFileName,sizeof(eventFileName),"/dev/input/%s",eventFiles[eventFileIndex]->d_name);
		int eventFd=open(eventFileName,O_RDONLY);
		if(eventFd>=0)
			{
			/* Get device name: */
			char eventFileDeviceName[256];
			if(ioctl(eventFd,EVIOCGNAME(sizeof(eventFileDeviceName)),eventFileDeviceName)>=0)
				{
				if(strcmp(deviceName,eventFileDeviceName)==0)
					{
					/* We have a winner! */
					deviceFd=eventFd;
					break;
					}
				}
			
			/* This is not the device you are looking for, go to the next: */
			close(eventFd);
			}
		}
	
	/* Destroy list of event files: */
	for(int i=0;i<numEventFiles;++i)
		free(eventFiles[i]);
	free(eventFiles);
	
	/* Return found device file descriptor, or -1: */
	return deviceFd;
	}

void HIDDevice::deviceThreadMethod(void)
	{
	while(true)
		{
		/* Read a bunch of events: */
		input_event events[32];
		int numEvents=read(deviceFd,events,sizeof(events));
		if(numEvents>0)
			{
			/* Process all received events: */
			numEvents/=sizeof(input_event);
			for(int i=0;i<numEvents;++i)
				{
				switch(events[i].type)
					{
					case EV_KEY:
						{
						int buttonIndex=keyMap[events[i].code];
						if(buttonIndex>=0)
							{
							bool newButtonState=events[i].value!=0;
							if(newButtonState!=buttonStates[buttonIndex]&&reportEvents)
								setButtonState(buttonIndex,newButtonState);
							buttonStates[buttonIndex]=newButtonState;
							}
						break;
						}
					
					case EV_ABS:
						{
						int valuatorIndex=absAxisMap[events[i].code];
						if(valuatorIndex>=0)
							{
							float newValuatorState=axisConverters[valuatorIndex].map(events[i].value);
							if(newValuatorState!=valuatorStates[valuatorIndex]&&reportEvents)
								setValuatorState(valuatorIndex,newValuatorState);
							valuatorStates[valuatorIndex]=newValuatorState;
							}
						break;
						}
					
					case EV_REL:
						{
						int valuatorIndex=relAxisMap[events[i].code];
						if(valuatorIndex>=0)
							{
							float newValuatorState=axisConverters[valuatorIndex].map(events[i].value);
							if(newValuatorState!=valuatorStates[valuatorIndex]&&reportEvents)
								setValuatorState(valuatorIndex,newValuatorState);
							valuatorStates[valuatorIndex]=newValuatorState;
							}
						break;
						}
					}
				}
			
			/* Mark manager state as complete: */
			updateState();
			}
		}
	}

HIDDevice::HIDDevice(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile)
	:VRDevice(sFactory,sDeviceManager,configFile),
	 deviceFd(-1),
	 keyMap(0),
	 absAxisMap(0),relAxisMap(0),axisConverters(0),
	 reportEvents(false),
	 buttonStates(0),valuatorStates(0)
	{
	/* First option: Open device by explicit event device file name: */
	if(deviceFd<0)
		{
		std::string deviceFileName=configFile.retrieveString("./deviceFileName","");
		if(deviceFileName!="")
			{
			#ifdef VERBOSE
			printf("HIDDevice: Opening device %s\n",deviceFileName.c_str());
			fflush(stdout);
			#endif
			deviceFd=open(deviceFileName.c_str(),O_RDONLY);
			if(deviceFd<0)
				Misc::throwStdErr("HIDDevice: Unable to open device file \"%s\"",deviceFileName.c_str());
			}
		}
	
	/* Second option: Open device by vendor ID / product ID: */
	if(deviceFd<0)
		{
		std::string deviceVendorProductId=configFile.retrieveString("./deviceVendorProductId","");
		if(deviceVendorProductId!="")
			{
			/* Split ID string into vendor ID / product ID: */
			char* colonPtr;
			int vendorId=strtol(deviceVendorProductId.c_str(),&colonPtr,16);
			char* endPtr;
			int productId=strtol(colonPtr+1,&endPtr,16);
			if(*colonPtr!=':'||*endPtr!='\0'||vendorId<0||productId<0)
				Misc::throwStdErr("HIDDevice: Malformed vendorId:productId string \"%s\"",deviceVendorProductId.c_str());
			#ifdef VERBOSE
			printf("HIDDevice: Searching device %04x:%04x\n",vendorId,productId);
			fflush(stdout);
			#endif
			deviceFd=findDevice(vendorId,productId);
			if(deviceFd<0)
				Misc::throwStdErr("HIDDevice: No device with vendorId:productId %04x:%04x found",vendorId,productId);
			}
		}
	
	/* Third option: Open device by device name: */
	if(deviceFd<0)
		{
		std::string deviceName=configFile.retrieveString("./deviceName","");
		if(deviceName!="")
			{
			#ifdef VERBOSE
			printf("HIDDevice: Searching device %s\n",deviceName.c_str());
			fflush(stdout);
			#endif
			deviceFd=findDevice(deviceName.c_str());
			if(deviceFd<0)
				Misc::throwStdErr("HIDDevice: No device with name \"%s\" found",deviceName.c_str());
			}
		}
	
	/* Bail out if no device was found: */
	if(deviceFd<0)
		Misc::throwStdErr("HIDDevice: No device specified");
	
	/* Set number of trackers on device: */
	setNumTrackers(0,configFile);
	
	/* Query all feature types of the device: */
	unsigned char featureTypeBits[EV_MAX/8+1];
	memset(featureTypeBits,0,EV_MAX/8+1);
	if(ioctl(deviceFd,EVIOCGBIT(0,sizeof(featureTypeBits)),featureTypeBits)<0)
		Misc::throwStdErr("HIDDevice: Unable to query device feature types");
	
	/* Query the number of keys on the device: */
	if(featureTypeBits[EV_KEY/8]&(1<<(EV_KEY%8)))
		{
		#ifdef VERBOSE
		printf("HIDDevice: Initializing buttons...\n");
		fflush(stdout);
		#endif
		
		/* Query key features: */
		unsigned char keyBits[KEY_MAX/8+1];
		memset(keyBits,0,KEY_MAX/8+1);
		if(ioctl(deviceFd,EVIOCGBIT(EV_KEY,sizeof(keyBits)),keyBits)<0)
			Misc::throwStdErr("HIDDevice: Unable to query device key features");
		
		/* Initialize the key translation array: */
		keyMap=new int[KEY_MAX+1];
		int numKeys=0;
		for(int i=0;i<=KEY_MAX;++i)
			{
			if(keyBits[i/8]&(1<<(i%8)))
				{
				keyMap[i]=numKeys;
				++numKeys;
				}
			else
				keyMap[i]=-1;
			}
		
		/* Set number of buttons on device: */
		#ifdef VERBOSE
		printf("HIDDevice: %d buttons found\n",numKeys);
		fflush(stdout);
		#endif
		setNumButtons(numKeys,configFile);
		}
	else
		setNumButtons(0,configFile);
	
	/* Count the number of absolute and relative axes: */
	int numAxes=0;
	
	/* Query the number of absolute axes on the device: */
	if(featureTypeBits[EV_ABS/8]&(1<<(EV_ABS%8)))
		{
		#ifdef VERBOSE
		printf("HIDDevice: Initializing absolute axes...\n");
		fflush(stdout);
		#endif
		
		/* Query absolute axis features: */
		unsigned char absAxisBits[ABS_MAX/8+1];
		memset(absAxisBits,0,ABS_MAX/8+1);
		if(ioctl(deviceFd,EVIOCGBIT(EV_ABS,sizeof(absAxisBits)),absAxisBits)<0)
			Misc::throwStdErr("HIDDevice: Unable to query device absolute axis features");
		
		/* Initialize the axis translation array: */
		absAxisMap=new int[ABS_MAX+1];
		int numAbsAxes=0;
		for(int i=0;i<=ABS_MAX;++i)
			{
			if(absAxisBits[i/8]&(1<<(i%8)))
				{
				absAxisMap[i]=numAxes;
				++numAxes;
				++numAbsAxes;
				}
			else
				absAxisMap[i]=-1;
			}
		
		#ifdef VERBOSE
		printf("HIDDevice: %d absolute axes found\n",numAbsAxes);
		fflush(stdout);
		#endif
		}
	
	/* Query the number of relative axes on the device: */
	if(featureTypeBits[EV_REL/8]&(1<<(EV_REL%8)))
		{
		#ifdef VERBOSE
		printf("HIDDevice: Initializing relative axes...\n");
		fflush(stdout);
		#endif
		
		/* Query relative axis features: */
		unsigned char relAxisBits[REL_MAX/8+1];
		memset(relAxisBits,0,REL_MAX/8+1);
		if(ioctl(deviceFd,EVIOCGBIT(EV_REL,sizeof(relAxisBits)),relAxisBits)<0)
			Misc::throwStdErr("HIDDevice: Unable to query device relative axis features");
		
		/* Initialize the axis translation array: */
		relAxisMap=new int[REL_MAX+1];
		int numRelAxes=0;
		for(int i=0;i<=REL_MAX;++i)
			{
			if(relAxisBits[i/8]&(1<<(i%8)))
				{
				relAxisMap[i]=numAxes;
				++numAxes;
				++numRelAxes;
				}
			else
				relAxisMap[i]=-1;
			}
		
		#ifdef VERBOSE
		printf("HIDDevice: %d relative axes found\n",numRelAxes);
		fflush(stdout);
		#endif
		}
	
	/* Set number of valuators on device: */
	setNumValuators(numAxes,configFile);
		
	/* Initialize axis converters: */
	axisConverters=new AxisConverter[numAxes];
	
	if(absAxisMap!=0)
		{
		/* Initialize absolute axis converters: */
		#ifdef VERBOSE
		printf("HIDDevice: Initializing absolute axis converters\n");
		fflush(stdout);
		#endif
		for(int i=0;i<=ABS_MAX;++i)
			if(absAxisMap[i]>=0)
				{
				/* Query configuration of this axis: */
				input_absinfo absAxisConf;
				if(ioctl(deviceFd,EVIOCGABS(i),&absAxisConf)<0)
					Misc::throwStdErr("HIDDevice: Unable to query device absolute axis configuration");
				
				#ifdef VERBOSE
				printf("Axis %2d: min %d, max %d, fuzz %d, flat %d\n",absAxisMap[i],absAxisConf.minimum,absAxisConf.maximum,absAxisConf.fuzz,absAxisConf.flat);
				fflush(stdout);
				#endif
				
				/* Initialize converter with queried values: */
				AxisConverter& converter=axisConverters[absAxisMap[i]];
				float mid=Math::mid(absAxisConf.minimum,absAxisConf.maximum);
				converter=AxisConverter(absAxisConf.minimum,mid-absAxisConf.flat,mid+absAxisConf.flat,absAxisConf.maximum);
				
				/* Override axis settings from configuration file: */
				char axisSettingsTag[20];
				snprintf(axisSettingsTag,sizeof(axisSettingsTag),"axis%dSettings",absAxisMap[i]);
				converter=configFile.retrieveValue<AxisConverter>(axisSettingsTag,converter);
				
				#ifdef VERBOSE
				printf("Axis %2d: %s\n",absAxisMap[i],Misc::ValueCoder<AxisConverter>::encode(converter).c_str());
				fflush(stdout);
				#endif
				}
		}
	
	if(relAxisMap!=0)
		{
		/* Initialize relative axis converters: */
		#ifdef VERBOSE
		printf("HIDDevice: Initializing relative axis converters\n");
		fflush(stdout);
		#endif
		for(int i=0;i<=REL_MAX;++i)
			if(relAxisMap[i]>=0)
				{
				/* Initialize converter with default values: */
				AxisConverter& converter=axisConverters[relAxisMap[i]];
				converter=AxisConverter(-1.0f,1.0f);
				
				/* Override axis settings from configuration file: */
				char axisSettingsTag[20];
				snprintf(axisSettingsTag,sizeof(axisSettingsTag),"axis%dSettings",relAxisMap[i]);
				converter=configFile.retrieveValue<AxisConverter>(axisSettingsTag,converter);
				
				#ifdef VERBOSE
				printf("Axis %2d: %s\n",relAxisMap[i],Misc::ValueCoder<AxisConverter>::encode(converter).c_str());
				fflush(stdout);
				#endif
				}
		}
	
	#if 0
	/* Initialize gain arrays: */
	valuatorGains=new float[getNumValuators()];
	for(int i=0;i<getNumValuators();++i)
		{
		char valuatorGainTag[40];
		snprintf(valuatorGainTag,sizeof(valuatorGainTag),"./valuatorGain%d",i);
		valuatorGains[i]=configFile.retrieveValue<float>(valuatorGainTag,1.0f);
		}
	#endif
	
	/* Initialize state arrays: */
	buttonStates=new bool[getNumButtons()];
	for(int i=0;i<getNumButtons();++i)
		buttonStates[i]=false;
	valuatorStates=new float[getNumValuators()];
	for(int i=0;i<getNumValuators();++i)
		valuatorStates[i]=0.0f;
	
	/* Start device thread (HID device cannot be disabled): */
	startDeviceThread();
	}

HIDDevice::~HIDDevice(void)
	{
	/* Stop device thread (HID device cannot be disabled): */
	{
	Threads::Mutex::Lock stateLock(stateMutex);
	stopDeviceThread();
	}
	delete[] buttonStates;
	delete[] valuatorStates;
	delete[] keyMap;
	delete[] absAxisMap;
	delete[] relAxisMap;
	delete[] axisConverters;
	if(deviceFd>=0)
		close(deviceFd);
	}

void HIDDevice::start(void)
	{
	/* Set device manager's button and valuator states to current states: */
	{
	Threads::Mutex::Lock stateLock(stateMutex);
	for(int i=0;i<getNumButtons();++i)
		setButtonState(i,buttonStates[i]);
	for(int i=0;i<getNumValuators();++i)
		setValuatorState(i,valuatorStates[i]);
	
	/* Start reporting events to the device manager: */
	reportEvents=true;
	}
	}

void HIDDevice::stop(void)
	{
	/* Stop reporting events to the device manager: */
	{
	Threads::Mutex::Lock stateLock(stateMutex);
	reportEvents=false;
	}
	}
