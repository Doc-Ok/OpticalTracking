/***********************************************************************
ViconTarsus - Class for Vicon optical trackers using the real-time
streaming protocol.
Copyright (c) 2007-2011 Oliver Kreylos

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

#include <VRDeviceDaemon/VRDevices/ViconTarsus.h>

#include <stdio.h>
#include <Misc/StringMarshaller.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Misc/CompoundValueCoders.h>

#include <VRDeviceDaemon/VRDeviceManager.h>

/****************************
Methods of class ViconTarsus:
****************************/

void ViconTarsus::deviceThreadMethod(void)
	{
	while(true)
		{
		/* Wait for the next packet from the server: */
		int packetKind=pipe.read<int>();
		if(pipe.read<int>()==1) // Ignore request packets
			{
			switch(packetKind)
				{
				case 0:
				case 4:
					/* Shut down the thread method: */
					return;
				
				case 2:
					{
					/* Read the data packet: */
					int numPacketChannels=pipe.read<int>();
					if(numPacketChannels>numChannels)
						{
						/* Read the relevant channels: */
						pipe.read<double>(channelPacketBuffer,numChannels);
						
						/* Ignore all spurious channels: */
						for(int i=numChannels;i<numPacketChannels;++i)
							pipe.read<double>();
						numPacketChannels=numChannels;
						}
					else
						{
						/* Read all channels: */
						pipe.read<double>(channelPacketBuffer,numPacketChannels);
						}
					
					/* Process the data packet: */
					for(int trackerIndex=0;trackerIndex<getNumTrackers();++trackerIndex)
						{
						/* Get the tracker's position: */
						Vector translation=Vector::zero;
						bool valid=true;
						for(int i=0;i<3;++i)
							{
							if(trackerChannelIndices[trackerIndex*6+i]<numPacketChannels)
								translation[i]=VScalar(channelPacketBuffer[trackerChannelIndices[trackerIndex*6+i]]);
							else
								valid=false;
							}
						
						if(valid)
							{
							if(trackerSixDofs[trackerIndex])
								{
								/* Get the tracker's orientation: */
								PositionOrientation::Rotation::Vector rotation=PositionOrientation::Rotation::Vector::zero;
								bool sixDof=true;
								for(int i=0;i<3;++i)
									{
									if(trackerChannelIndices[trackerIndex*6+3+i]<numPacketChannels)
										rotation[i]=RScalar(channelPacketBuffer[trackerChannelIndices[trackerIndex*6+3+i]]);
									else
										sixDof=false;
									}
								
								if(sixDof)
									{
									/* Set the 6-DOF tracker state: */
									trackerStates[trackerIndex].positionOrientation=PositionOrientation(translation,Rotation::rotateScaledAxis(rotation));
									}
								else
									{
									/* Set the 6-DOF tracker state with the previous orientation: */
									trackerStates[trackerIndex].positionOrientation=PositionOrientation(translation,trackerStates[trackerIndex].positionOrientation.getRotation());
									}
								}
							else
								{
								/* Set the 3-DOF tracker state: */
								trackerStates[trackerIndex].positionOrientation=PositionOrientation(translation,Rotation::identity);
								}
							}
						}
					
					/* Update all tracker states (including those that were not updated): */
					for(int i=0;i<getNumTrackers();++i)
						setTrackerState(i,trackerStates[i]);
					break;
					}
				
				default:
					/* Ignore the packet: */
					;
				}
			}
		}
	}

ViconTarsus::ViconTarsus(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile)
	:VRDevice(sFactory,sDeviceManager,configFile),
	 pipe(configFile.retrieveString("./serverName").c_str(),configFile.retrieveValue<int>("./serverPort",800)),
	 trackerChannelIndices(0),
	 trackerSixDofs(0),
	 trackerMap(0),
	 channelPacketBuffer(0),
	 trackerStates(0)
	{
	/* Set the pipe's endianness: */
	pipe.setEndianness(Misc::LittleEndian);
	
	/* Read the list of tracked bodies: */
	std::vector<std::string> trackedBodies=configFile.retrieveValue<std::vector<std::string> >("./trackedBodies");
	
	/* Set tracker's layout: */
	setNumTrackers(trackedBodies.size(),configFile);
	
	/* Initialize the tracker channel index array: */
	trackerChannelIndices=new int[getNumTrackers()*6];
	for(int i=0;i<getNumTrackers()*6;++i)
		trackerChannelIndices[i]=-1;
	
	/* Send info request to server: */
	#ifdef VERBOSE
	printf("ViconTarsus: Requesting info packet\n");
	#endif
	pipe.write<int>(1); // Info
	pipe.write<int>(0); // Request
	pipe.flush();
	
	/* Wait for the server's reply: */
	if(pipe.read<int>()!=1)
		Misc::throwStdErr("ViconTarsus: Unable to connect to tracking server at %s",pipe.getPeerHostName().c_str());
	if(pipe.read<int>()!=1)
		Misc::throwStdErr("ViconTarsus: Unable to connect to tracking server at %s",pipe.getPeerHostName().c_str());
	
	/* Read the info packet's data: */
	numChannels=pipe.read<int>();
	#ifdef VERBOSE
	printf("ViconTarsus: Server reports %d channels\n",numChannels);
	#endif
	for(int channelIndex=0;channelIndex<numChannels;++channelIndex)
		{
		/* Read the channel descriptor (fortunately, Vicon's string protocol is compatible to the string marshaller's): */
		std::string channelName=Misc::readCppString(pipe);
		#ifdef VERBOSE
		printf("ViconTarsus: Server channel %2d: %s\n",channelIndex,channelName.c_str());
		#endif
		
		/* Parse the channel name: */
		const char* nameStart=channelName.c_str();
		const char* nameEnd;
		for(nameEnd=nameStart;*nameEnd!='\0'&&*nameEnd!=' '&&*nameEnd!='-'&&*nameEnd!='<'&&*nameEnd!='>';++nameEnd)
			;
		const char* codeStart;
		for(codeStart=nameEnd;*codeStart!='\0'&&*codeStart!='<';++codeStart)
			;
		const char* codeEnd;
		for(codeEnd=codeStart;*codeEnd!='\0'&&*codeEnd!='>';++codeEnd)
			;
		
		/* Check if the channel name is well-formed: */
		if(*codeStart=='<'&&*codeEnd=='>')
			{
			std::string code(codeStart+1,codeEnd);
			
			/* Check if the channel encodes a body parameter or a marker parameter: */
			if(code[0]=='P'||code[0]=='A'||code[0]=='T')
				{
				/* Check if the name corresponds to a tracked body: */
				std::string name(nameStart,nameEnd);
				int bodyIndex;
				for(bodyIndex=0;bodyIndex<getNumTrackers();++bodyIndex)
					if(trackedBodies[bodyIndex]==name)
						break;
				if(bodyIndex<getNumTrackers())
					{
					/* Parse the channel code: */
					if(code=="P-X"||code=="T-X")
						trackerChannelIndices[bodyIndex*6+0]=channelIndex;
					else if(code=="P-Y"||code=="T-Y")
						trackerChannelIndices[bodyIndex*6+1]=channelIndex;
					else if(code=="P-Z"||code=="T-Z")
						trackerChannelIndices[bodyIndex*6+2]=channelIndex;
					else if(code=="A-X")
						trackerChannelIndices[bodyIndex*6+3]=channelIndex;
					else if(code=="A-Y")
						trackerChannelIndices[bodyIndex*6+4]=channelIndex;
					else if(code=="A-Z")
						trackerChannelIndices[bodyIndex*6+5]=channelIndex;
					}
				}
			}
		}
	
	/* Check if all requested trackers have channels associated with them: */
	trackerSixDofs=new bool[getNumTrackers()];
	int numSixDofTrackers=0;
	for(int bodyIndex=0;bodyIndex<getNumTrackers();++bodyIndex)
		{
		/* Check if the tracker has 3-DOF data: */
		bool valid=true;
		for(int i=0;i<3;++i)
			if(trackerChannelIndices[bodyIndex*6+i]==-1)
				valid=false;
		if(!valid)
			Misc::throwStdErr("ViconTarsus: Insufficient data to track body %s",trackedBodies[bodyIndex].c_str());
		
		/* Check if the tracker has 6-DOF data (for later): */
		bool sixDof=true;
		for(int i=3;i<6;++i)
			if(trackerChannelIndices[bodyIndex*6+i]==-1)
				sixDof=false;
		trackerSixDofs[bodyIndex]=sixDof;
		if(sixDof)
			++numSixDofTrackers;
		}
	#ifdef VERBOSE
	printf("ViconTarsus: Tracking %d 3-DOF tracker(s) and %d 6-DOF tracker(s)\n",getNumTrackers()-numSixDofTrackers,numSixDofTrackers);
	#endif
	
	/* Construct the tracker map: */
	trackerMap=new std::pair<int,int>[numChannels];
	for(int i=0;i<numChannels;++i)
		trackerMap[i]=std::pair<int,int>(-1,-1);
	for(int bodyIndex=0;bodyIndex<getNumTrackers();++bodyIndex)
		for(int i=0;i<6;++i)
			{
			if(trackerChannelIndices[bodyIndex*6+i]!=-1)
				trackerMap[trackerChannelIndices[bodyIndex*6+i]]=std::pair<int,int>(bodyIndex,i);
			}
	
	/* Initialize the channel packet buffer: */
	channelPacketBuffer=new double[numChannels];
	
	/* Initialize tracker states: */
	trackerStates=new Vrui::VRDeviceState::TrackerState[getNumTrackers()];
	}

ViconTarsus::~ViconTarsus(void)
	{
	delete[] trackerChannelIndices;
	delete[] trackerSixDofs;
	delete[] trackerMap;
	delete[] channelPacketBuffer;
	delete[] trackerStates;
	}

void ViconTarsus::start(void)
	{
	/* Initialize tracker states: */
	for(int i=0;i<getNumTrackers();++i)
		{
		trackerStates[i].positionOrientation=PositionOrientation(Vector::zero,Rotation::identity);
		trackerStates[i].linearVelocity=Vrui::VRDeviceState::TrackerState::LinearVelocity::zero;
		trackerStates[i].angularVelocity=Vrui::VRDeviceState::TrackerState::AngularVelocity::zero;
		}
	
	/* Start device communication thread: */
	startDeviceThread();
	
	/* Activate streaming: */
	#ifdef VERBOSE
	printf("ViconTarsus: Starting continuous update mode\n");
	#endif
	pipe.write<int>(3);
	pipe.write<int>(0);
	pipe.flush();
	}

void ViconTarsus::stop(void)
	{
	/* Deactivate streaming: */
	#ifdef VERBOSE
	printf("ViconTarsus: Stopping continuous update mode\n");
	#endif
	pipe.write<int>(4);
	pipe.write<int>(0);
	pipe.flush();
	
	/* Stop device communication thread: */
	stopDeviceThread();
	}

/*************************************
Object creation/destruction functions:
*************************************/

extern "C" VRDevice* createObjectViconTarsus(VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager,Misc::ConfigurationFile& configFile)
	{
	VRDeviceManager* deviceManager=static_cast<VRDeviceManager::DeviceFactoryManager*>(factoryManager)->getDeviceManager();
	return new ViconTarsus(factory,deviceManager,configFile);
	}

extern "C" void destroyObjectViconTarsus(VRDevice* device,VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager)
	{
	delete device;
	}
