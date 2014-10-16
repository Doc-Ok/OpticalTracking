/***********************************************************************
ArtDTrack - Class for ART DTrack tracking devices.
Copyright (c) 2004-2013 Oliver Kreylos

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

#include <VRDeviceDaemon/VRDevices/ArtDTrack.h>

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <Misc/Time.h>
#include <Misc/Endianness.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/CompoundValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Geometry/Matrix.h>

#include <VRDeviceDaemon/VRDeviceManager.h>

namespace {

/****************
Helper functions:
****************/

ArtDTrack::DeviceReportFormat parseDeviceReportFormat(const char* start,const char* end,const char** parseEnd)
	{
	ArtDTrack::DeviceReportFormat result=ArtDTrack::DRF_NUMFORMATS;
	const char* cPtr=start;
	int state=0;
	while(cPtr!=end&&state>=0)
		{
		switch(state)
			{
			case 0: // Start state
				if(*cPtr=='6')
					state=1;
				else if(*cPtr=='3')
					state=5;
				else if(*cPtr=='g'||*cPtr=='G')
					state=6;
				else
					state=-1;
				break;
			
			case 1: // "6" read
				if(*cPtr=='d'||*cPtr=='D')
					{
					result=ArtDTrack::DRF_6D;
					state=2;
					}
				else
					state=-1;
				break;
			
			case 2: // "6d" read
				if(*cPtr=='f'||*cPtr=='F')
					{
					result=ArtDTrack::DRF_6DF;
					state=3;
					}
				else if(*cPtr=='m'||*cPtr=='M')
					state=4;
				else
					{
					if(isalnum(*cPtr)||*cPtr=='_')
						result=ArtDTrack::DRF_NUMFORMATS;
					state=-1;
					}
				break;
			
			case 3: // "6df" read
				if(*cPtr=='2')
					{
					result=ArtDTrack::DRF_6DF2;
					state=7;
					}
				else
					{
					if(isalnum(*cPtr)||*cPtr=='_')
						result=ArtDTrack::DRF_NUMFORMATS;
					state=-1;
					}
				break;
			
			case 4: // "6dm" read
				if(*cPtr=='t'||*cPtr=='T')
					{
					result=ArtDTrack::DRF_6DMT;
					state=7;
					}
				else
					state=-1;
				break;
			
			case 5: // "3" read
				if(*cPtr=='d'||*cPtr=='D')
					{
					result=ArtDTrack::DRF_3D;
					state=7;
					}
				else
					state=-1;
				break;
			
			case 6: // "g" read
				if(*cPtr=='l'||*cPtr=='L')
					{
					result=ArtDTrack::DRF_GL;
					state=7;
					}
				else
					state=-1;
				break;
			
			case 7: // Valid word read; check for end-of-word
				if(isalnum(*cPtr)||*cPtr=='_')
					result=ArtDTrack::DRF_NUMFORMATS;
				state=-1;
				break;
			}
		
		if(state>=0)
			{
			/* Go to next character: */
			++cPtr;
			}
		}
	
	/* Set the decode end and return the parsed word: */
	*parseEnd=cPtr;
	return result;
	}

}

namespace Misc {

/**********************************
Value coder class for data formats:
**********************************/

template <>
class ValueCoder<ArtDTrack::DataFormat>
	{
	/* Methods: */
	public:
	static std::string encode(const ArtDTrack::DataFormat& df)
		{
		std::string result;
		switch(df)
			{
			case ArtDTrack::ASCII:
				result="ASCII";
				break;
			
			case ArtDTrack::BINARY:
				result="Binary";
				break;
			}
		
		return result;
		}
	static ArtDTrack::DataFormat decode(const char* start,const char* end,const char** decodeEnd =0)
		{
		if(end-start>=5&&strncasecmp(start,"ASCII",5)==0)
			{
			if(end-start>5&&(isalnum(start[5])||start[5]=='_'))
				throw DecodingError(std::string("Unable to convert \"")+std::string(start,end)+std::string("\" to ArtDTrack::DataFormat"));
			
			if(decodeEnd!=0)
				*decodeEnd=start+5;
			return ArtDTrack::ASCII;
			}
		else if(end-start>=6&&strncasecmp(start,"Binary",6)==0)
			{
			if(end-start>6&&(isalnum(start[6])||start[6]=='_'))
				throw DecodingError(std::string("Unable to convert \"")+std::string(start,end)+std::string("\" to ArtDTrack::DataFormat"));
			
			if(decodeEnd!=0)
				*decodeEnd=start+6;
			return ArtDTrack::BINARY;
			}
		else
			throw DecodingError(std::string("Unable to convert \"")+std::string(start,end)+std::string("\" to ArtDTrack::DataFormat"));
		}
	};

template <>
class ValueCoder<ArtDTrack::DeviceReportFormat>
	{
	/* Methods: */
	public:
	static std::string encode(const ArtDTrack::DeviceReportFormat& drf)
		{
		std::string result;
		switch(drf)
			{
			case ArtDTrack::DRF_6D:
				result="6d";
				break;
			
			case ArtDTrack::DRF_6DF:
				result="6df";
				break;
			
			case ArtDTrack::DRF_6DF2:
				result="6df2";
				break;
			
			case ArtDTrack::DRF_6DMT:
				result="6dmt";
				break;
			
			case ArtDTrack::DRF_GL:
				result="gl";
				break;
			
			case ArtDTrack::DRF_3D:
				result="3d";
				break;
			
			default:
				result="";
			}
		
		return result;
		}
	static ArtDTrack::DeviceReportFormat decode(const char* start,const char* end,const char** decodeEnd =0)
		{
		const char* myDecodeEnd;
		ArtDTrack::DeviceReportFormat result=parseDeviceReportFormat(start,end,&myDecodeEnd);
		if(result==ArtDTrack::DRF_NUMFORMATS)
			throw DecodingError(std::string("Unable to convert \"")+std::string(start,end)+std::string("\" to ArtDTrack::DeviceReportFormat"));
		
		if(decodeEnd!=0)
			*decodeEnd=myDecodeEnd;
		return result;
		}
	};

}

namespace {

/***************************************************************
Helper functions to extract data from DTrack ASCII message body:
***************************************************************/

inline bool expectChar(char expect,const char*& mPtr)
	{
	/* Skip whitespace: */
	while(*mPtr!='\n'&&isspace(*mPtr))
		++mPtr;
	bool result=*mPtr==expect;
	if(result)
		++mPtr;
	return result;
	}

inline int readInt(const char*& mPtr)
	{
	/* Parse an integer: */
	char* intEnd;
	int result=int(strtol(mPtr,&intEnd,10));
	mPtr=intEnd;
	return result;
	}

inline unsigned int readUint(const char*& mPtr)
	{
	/* Parse an integer: */
	char* uintEnd;
	unsigned int result=(unsigned int)(strtoul(mPtr,&uintEnd,10));
	mPtr=uintEnd;
	return result;
	}

inline double readFloat(const char*& mPtr)
	{
	/* Parse a float: */
	char* floatEnd;
	double result=strtod(mPtr,&floatEnd);
	mPtr=floatEnd;
	return result;
	}

/****************************************************************
Helper functions to extract data from DTrack binary message body:
****************************************************************/

template <class DataParam>
inline
DataParam
extractData(
	const char*& dataPtr)
	{
	/* Extract data item: */
	DataParam result=*reinterpret_cast<const DataParam*>(dataPtr);
	
	#if __BYTE_ORDER==__BIG_ENDIAN
	/* Convert endianness of data item: */
	Misc::swapEndianness(result);
	#endif
	
	/* Advance data pointer: */
	dataPtr+=sizeof(DataParam);
	
	/* Return data item: */
	return result;
	}

template <class DataParam>
inline
void
skipData(
	const char*& dataPtr)
	{
	/* Advance data pointer: */
	dataPtr+=sizeof(DataParam);
	}

}

/**************************
Methods of class ArtDTrack:
**************************/

void ArtDTrack::processAsciiData(void)
	{
	Vrui::VRDeviceState::TrackerState ts;
	ts.linearVelocity=Vrui::VRDeviceState::TrackerState::LinearVelocity::zero;
	ts.angularVelocity=Vrui::VRDeviceState::TrackerState::AngularVelocity::zero;
	
	while(true)
		{
		/* Wait for the next data message from the DTrack daemon: */
		char messageBuffer[4096];
		size_t messageSize=dataSocket.receiveMessage(messageBuffer,sizeof(messageBuffer)-1);
		
		/* Newline-terminate the message as a sentinel: */
		messageBuffer[messageSize]='\n';
		
		/* Parse the received message: */
		const char* mPtr=messageBuffer;
		const char* mEnd=messageBuffer+(messageSize+1);
		while(mPtr!=mEnd)
			{
			/* Skip whitespace, but not the line terminator: */
			while(*mPtr!='\n'&&isspace(*mPtr))
				++mPtr;
			
			/* Get the line's device report format: */
			DeviceReportFormat drf=parseDeviceReportFormat(mPtr,0,&mPtr);
			
			/* Process the line: */
			if(drf!=DRF_NUMFORMATS)
				{
				if(drf==DRF_6DF2)
					{
					/* Skip the number of defined flysticks: */
					readInt(mPtr);
					}
				
				/* Read the number of bodies in this report: */
				int numBodies=readInt(mPtr);
				
				/* Parse all body reports: */
				for(int body=0;body<numBodies;++body)
					{
					/* Check for opening bracket: */
					if(!expectChar('[',mPtr))
						break;
					
					/* Read the body's ID and find the corresponding device structure: */
					int id=readInt(mPtr);
					int deviceIndex=deviceIdToIndex[drf][id];
					Device* device=deviceIndex>=0?&devices[deviceIndex]:0;
					
					/* Read the quality value: */
					float quality=float(readFloat(mPtr));
					
					/* Read button/valuator or finger data depending on report format: */
					int numButtons=0;
					int numValuators=0;
					int numFingers=0;
					
					if(drf==DRF_6DF)
						{
						/* Read the button bit mask: */
						unsigned int buttonBits=readUint(mPtr);
						
						if(device!=0)
							{
							/* Set the device's button states: */
							for(int i=0;i<32&&i<device->numButtons;++i,buttonBits>>=1)
								setButtonState(device->firstButtonIndex+i,(buttonBits&0x1)!=0x0);
							}
						}
					if(drf==DRF_6DF2||drf==DRF_6DMT)
						{
						/* Read the number of buttons: */
						numButtons=readInt(mPtr);
						if(drf==DRF_6DF2)
							{
							/* Read the number of valuators: */
							numValuators=readInt(mPtr);
							}
						}
					if(drf==DRF_GL)
						{
						/* Skip the glove's handedness: */
						readInt(mPtr);
						
						/* Read the number of fingers: */
						numFingers=readInt(mPtr);
						}
					
					/* Check for closing bracket followed by opening bracket: */
					if(!expectChar(']',mPtr)||!expectChar('[',mPtr))
						break;
					
					Vector pos;
					Rotation orient=Rotation::identity;
					
					/* Read the body's 3D position: */
					for(int i=0;i<3;++i)
						pos[i]=VScalar(readFloat(mPtr));
					
					if(drf!=DRF_3D)
						{
						/* Read the body's 3D orientation: */
						if(drf==DRF_6D||drf==DRF_6DF)
							{
							/* Read the body's orientation angles: */
							VScalar angles[3];
							for(int i=0;i<3;++i)
								angles[i]=VScalar(readFloat(mPtr));
							
							/* Convert the orientation angles to a 3D rotation: */
							orient*=Rotation::rotateX(Math::rad(angles[0]));
							orient*=Rotation::rotateY(Math::rad(angles[1]));
							orient*=Rotation::rotateZ(Math::rad(angles[2]));
							}
					
						/* Check for closing bracket followed by opening bracket: */
						if(!expectChar(']',mPtr)||!expectChar('[',mPtr))
							break;
						
						if(drf==DRF_6DF2||drf==DRF_6DMT||drf==DRF_GL)
							{
							/* Read the body's orientation matrix (yuck!): */
							Geometry::Matrix<VScalar,3,3> matrix;
							for(int j=0;j<3;++j)
								for(int i=0;i<3;++i)
									matrix(i,j)=VScalar(readFloat(mPtr));
							
							if(quality>0.0f)
								{
								/* Calculate the body's orientation quaternion (YUCK!): */
								orient=Rotation::fromMatrix(matrix);
								}
							}
						else
							{
							/* Skip the body's orientation matrix: */
							for(int i=0;i<9;++i)
								readFloat(mPtr);
							}
						}
					
					/* Check for closing bracket: */
					if(!expectChar(']',mPtr))
						break;
					
					if(drf==DRF_6DF2)
						{
						/* Check for opening bracket: */
						if(!expectChar('[',mPtr))
							break;
						
						/* Read button states: */
						for(int bitIndex=0;bitIndex<numButtons;bitIndex+=32)
							{
							/* Read the next button bit mask: */
							unsigned int buttonBits=readUint(mPtr);
							
							if(device!=0)
								{
								/* Set the device's button states: */
								for(int i=0;i<32&&bitIndex+i<device->numButtons;++i,buttonBits>>=1)
									setButtonState(device->firstButtonIndex+bitIndex+i,(buttonBits&0x1)!=0x0);
								}
							}
						
						/* Read valuator states: */
						for(int i=0;i<numValuators;++i)
							{
							/* Read the next valuator value: */
							float value=float(readFloat(mPtr));
							
							/* Set the valuator value if the valuator is valid: */
							if(device!=0&&i<device->numValuators)
								setValuatorState(device->firstValuatorIndex+i,value);
							}
						
						/* Check for closing bracket: */
						if(!expectChar(']',mPtr))
							break;
						}
					
					if(drf==DRF_GL)
						{
						/* Skip all finger data for now: */
						bool error=false;
						for(int finger=0;finger<numFingers;++finger)
							{
							/* Check for opening bracket: */
							if(!expectChar('[',mPtr))
								{
								error=true;
								break;
								}
							
							/* Skip finger position: */
							for(int i=0;i<3;++i)
								readFloat(mPtr);
							
							/* Check for closing followed by opening bracket: */
							if(!expectChar(']',mPtr)||!expectChar('[',mPtr))
								{
								error=true;
								break;
								}
							
							/* Skip finger orientation: */
							for(int i=0;i<9;++i)
								readFloat(mPtr);
							
							/* Check for closing followed by opening bracket: */
							if(!expectChar(']',mPtr)||!expectChar('[',mPtr))
								{
								error=true;
								break;
								}
							
							/* Skip finger bending parameters: */
							for(int i=0;i<6;++i)
								readFloat(mPtr);
							
							/* Check for closing bracket: */
							if(!expectChar(']',mPtr))
								{
								error=true;
								break;
								}
							}
						
						/* Stop parsing the packet on syntax error: */
						if(error)
							break;
						}
					
					/* Check if this body has a valid position/orientation and has been configured as a device: */
					if(quality>0.0f&&device!=0)
						{
						/* Set the device's tracker state: */
						ts.positionOrientation=PositionOrientation(pos,orient);
						setTrackerState(deviceIndex,ts);
						}
					}
				}
			
			/* Skip the rest of the line: */
			while(*mPtr!='\n')
				++mPtr;
			
			/* Go to the next line: */
			++mPtr;
			}
		
		/* Tell the VR device manager that the current state has updated completely: */
		updateState();
		}
	}

void ArtDTrack::processBinaryData(void)
	{
	Vrui::VRDeviceState::TrackerState ts;
	ts.linearVelocity=Vrui::VRDeviceState::TrackerState::LinearVelocity::zero;
	ts.angularVelocity=Vrui::VRDeviceState::TrackerState::AngularVelocity::zero;
	
	while(true)
		{
		/* Wait for the next data message from the DTrack daemon: */
		char messageBuffer[1024];
		dataSocket.receiveMessage(messageBuffer,sizeof(messageBuffer));
		
		/* Parse the received message: */
		const char* mPtr=messageBuffer;
		// unsigned int frameNr=extractData<unsigned int>(mPtr);
		skipData<unsigned int>(mPtr); // Skip frame number
		int numBodies=extractData<int>(mPtr);
		for(int i=0;i<numBodies;++i)
			{
			/* Read body's ID and measurement quality: */
			int trackerId=int(extractData<unsigned int>(mPtr));
			// float quality=extractData<float>(mPtr);
			skipData<float>(mPtr); // Skip measurement quality
			
			/* Read body's position: */
			Vector pos;
			for(int j=0;j<3;++j)
				pos[j]=VScalar(extractData<float>(mPtr));
			
			/* Read body's orientation as Euler angles: */
			RScalar angles[3];
			for(int j=0;j<3;++j)
				angles[j]=Math::rad(extractData<float>(mPtr));
			
			/* Convert Euler angles to rotation: */
			Rotation o=Rotation::identity;
			o*=Rotation::rotateX(angles[0]);
			o*=Rotation::rotateY(angles[1]);
			o*=Rotation::rotateZ(angles[2]);
			
			/* Skip body's orientation as rotation matrix: */
			for(int j=0;j<9;++j)
				skipData<float>(mPtr);
			
			/* Set tracker position and orientation: */
			if(trackerId<getNumTrackers())
				{
				ts.positionOrientation=PositionOrientation(pos,o);
				setTrackerState(trackerId,ts);
				}
			}
		
		/* Tell the VR device manager that the current state has updated completely: */
		updateState();
		}
	}

void ArtDTrack::deviceThreadMethod(void)
	{
	/* Select the appropriate processing method based on data format: */
	switch(dataFormat)
		{
		case ASCII:
			processAsciiData();
			break;
		
		case BINARY:
			processBinaryData();
			break;
		}
	}

ArtDTrack::ArtDTrack(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile)
	:VRDevice(sFactory,sDeviceManager,configFile),
	 useRemoteControl(configFile.retrieveValue<bool>("./useRemoteControl",false)),
	 controlSocket(useRemoteControl?new Comm::UDPSocket(-1,configFile.retrieveString("./serverName"),configFile.retrieveValue<int>("./serverControlPort")):0),
	 dataSocket(configFile.retrieveValue<int>("./serverDataPort"),0),
	 dataFormat(configFile.retrieveValue<DataFormat>("./dataFormat",ASCII)),
	 devices(0)
	{
	/* Initialize device ID mapper: */
	for(int reportFormat=0;reportFormat<DRF_NUMFORMATS;++reportFormat)
		{
		maxDeviceId[reportFormat]=0;
		deviceIdToIndex[reportFormat]=0;
		}
	
	/* Retrieve list of device names: */
	typedef std::vector<std::string> StringList;
	StringList deviceNames=configFile.retrieveValue<StringList>("./deviceNames");
	setNumTrackers(deviceNames.size(),configFile);
	devices=new Device[numTrackers];
	int totalNumButtons=0;
	int totalNumValuators=0;
	
	/* Initialize all configured devices: */
	#ifdef VERBOSE
	printf("ArtDTrack: Initializing tracked devices\n");
	fflush(stdout);
	#endif
	for(int i=0;i<numTrackers;++i)
		{
		/* Go to tracked device's section: */
		configFile.setCurrentSection(deviceNames[i].c_str());
		
		/* Read tracked device's configuration: */
		devices[i].reportFormat=configFile.retrieveValue<DeviceReportFormat>("./reportFormat",DRF_6D);
		devices[i].id=configFile.retrieveValue<int>("./id",maxDeviceId[devices[i].reportFormat]+1);
		if(maxDeviceId[devices[i].reportFormat]<devices[i].id)
			maxDeviceId[devices[i].reportFormat]=devices[i].id;
		devices[i].numButtons=configFile.retrieveValue<int>("./numButtons",0);
		devices[i].firstButtonIndex=totalNumButtons;
		totalNumButtons+=devices[i].numButtons;
		devices[i].numValuators=configFile.retrieveValue<int>("./numValuators",0);
		devices[i].firstValuatorIndex=totalNumValuators;
		totalNumValuators+=devices[i].numValuators;
		
		/* Go back to device's section: */
		configFile.setCurrentSection("..");
		}
	
	/* Create the device ID to index mapping: */
	for(int reportFormat=0;reportFormat<DRF_NUMFORMATS;++reportFormat)
		{
		deviceIdToIndex[reportFormat]=new int[maxDeviceId[reportFormat]+1];
		for(int i=0;i<=maxDeviceId[reportFormat];++i)
			deviceIdToIndex[reportFormat][i]=-1;
		}
	for(int i=0;i<numTrackers;++i)
		deviceIdToIndex[devices[i].reportFormat][devices[i].id]=i;
	
	/* Set total number of buttons and valuators: */
	setNumButtons(totalNumButtons,configFile);
	setNumValuators(totalNumValuators,configFile);
	}

ArtDTrack::~ArtDTrack(void)
	{
	if(isActive())
		stop();
	delete[] devices;
	for(int reportFormat=0;reportFormat<DRF_NUMFORMATS;++reportFormat)
		delete[] deviceIdToIndex[reportFormat];
	
	delete controlSocket;
	}

void ArtDTrack::start(void)
	{
	/* Start device communication thread: */
	startDeviceThread();
	
	if(useRemoteControl)
		{
		/* Activate DTrack: */
		#ifdef VERBOSE
		printf("ArtDTrack: Activating cameras and reconstruction\n");
		#endif
		const char* msg1="dtrack 10 3";
		controlSocket->sendMessage(msg1,strlen(msg1)+1);
		
		Misc::sleep(0.5);
		
		/* Start sending measurements: */
		#ifdef VERBOSE
		printf("ArtDTrack: Starting continuous update mode\n");
		#endif
		const char* msg2="dtrack 31";
		controlSocket->sendMessage(msg2,strlen(msg2)+1);
		}
	}

void ArtDTrack::stop(void)
	{
	if(useRemoteControl)
		{
		/* Stop sending measurements: */
		#ifdef VERBOSE
		printf("ArtDTrack: Stopping continuous update mode\n");
		#endif
		const char* msg1="dtrack 32";
		controlSocket->sendMessage(msg1,strlen(msg1)+1);
		
		Misc::sleep(0.5);
		
		/* Deactivate DTrack: */
		#ifdef VERBOSE
		printf("ArtDTrack: Deactivating cameras and reconstruction\n");
		#endif
		const char* msg2="dtrack 10 0";
		controlSocket->sendMessage(msg2,strlen(msg2)+1);
		}
	
	/* Stop device communication thread: */
	stopDeviceThread();
	}

/*************************************
Object creation/destruction functions:
*************************************/

extern "C" VRDevice* createObjectArtDTrack(VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager,Misc::ConfigurationFile& configFile)
	{
	VRDeviceManager* deviceManager=static_cast<VRDeviceManager::DeviceFactoryManager*>(factoryManager)->getDeviceManager();
	return new ArtDTrack(factory,deviceManager,configFile);
	}

extern "C" void destroyObjectArtDTrack(VRDevice* device,VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager)
	{
	delete device;
	}
