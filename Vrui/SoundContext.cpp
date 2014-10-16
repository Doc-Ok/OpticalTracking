/***********************************************************************
SoundContext - Class for OpenAL contexts that are used to map a listener
to an OpenAL sound device.
Copyright (c) 2008-2014 Oliver Kreylos

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

#include <Vrui/SoundContext.h>

#include <AL/Config.h>

#include <stdio.h>
#include <string>
#include <Misc/ThrowStdErr.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <AL/ALTemplates.h>
#include <AL/ALGeometryWrappers.h>
#include <AL/ALContextData.h>
#include <Vrui/Vrui.h>
#include <Vrui/Listener.h>
#include <Vrui/Internal/Vrui.h>

namespace Misc {

/**************************************************
Helper class to decode distance attenuation models:
**************************************************/

template <>
class ValueCoder<Vrui::SoundContext::DistanceAttenuationModel>
	{
	/* Methods: */
	public:
	static std::string encode(const Vrui::SoundContext::DistanceAttenuationModel& value)
		{
		switch(value)
			{
			case Vrui::SoundContext::CONSTANT:
				return "Constant";
			
			case Vrui::SoundContext::INVERSE:
				return "Inverse";
			
			case Vrui::SoundContext::INVERSE_CLAMPED:
				return "InverseClamped";
			
			case Vrui::SoundContext::LINEAR:
				return "Linear";
			
			case Vrui::SoundContext::LINEAR_CLAMPED:
				return "LinearClamped";
			
			case Vrui::SoundContext::EXPONENTIAL:
				return "Exponential";
			
			case Vrui::SoundContext::EXPONENTIAL_CLAMPED:
				return "ExponentialClamped";
			}
		
		/* Never reached; just to make compiler happy: */
		return "";
		}
	static Vrui::SoundContext::DistanceAttenuationModel decode(const char* start,const char* end,const char** decodeEnd =0)
		{
		if(end-start>=8&&strncasecmp(start,"Constant",8)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+8;
			return Vrui::SoundContext::CONSTANT;
			}
		else if(end-start>=14&&strncasecmp(start,"InverseClamped",14)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+14;
			return Vrui::SoundContext::INVERSE_CLAMPED;
			}
		else if(end-start>=7&&strncasecmp(start,"Inverse",7)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+7;
			return Vrui::SoundContext::INVERSE;
			}
		else if(end-start>=13&&strncasecmp(start,"LinearClamped",13)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+13;
			return Vrui::SoundContext::LINEAR_CLAMPED;
			}
		else if(end-start>=6&&strncasecmp(start,"Linear",6)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+6;
			return Vrui::SoundContext::LINEAR;
			}
		else if(end-start>=18&&strncasecmp(start,"ExponentialClamped",18)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+18;
			return Vrui::SoundContext::EXPONENTIAL_CLAMPED;
			}
		else if(end-start>=11&&strncasecmp(start,"Exponential",11)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+11;
			return Vrui::SoundContext::EXPONENTIAL;
			}
		else
			throw DecodingError(std::string("Unable to convert \"")+std::string(start,end)+std::string("\" to SoundContext::DistanceAttenuationModel"));
		}
	};

}

namespace Vrui {

/*****************************
Methods of class SoundContext:
*****************************/

SoundContext::SoundContext(const Misc::ConfigurationFileSection& configFileSection,VruiState* sVruiState)
	:vruiState(sVruiState),
	 #if ALSUPPORT_CONFIG_HAVE_OPENAL
	 alDevice(0),alContext(0),
	 #endif
	 contextData(0),
	 listener(findListener(configFileSection.retrieveString("./listenerName").c_str())),
	 speedOfSound(float(getMeterFactor())*343.0f),
	 dopplerFactor(1.0f),
	 distanceAttenuationModel(CONSTANT)
	{
	/* Set sound context parameters from configuration file: */
	speedOfSound=configFileSection.retrieveValue<float>("./speedOfSound",speedOfSound);
	dopplerFactor=configFileSection.retrieveValue<float>("./dopplerFactor",dopplerFactor);
	distanceAttenuationModel=configFileSection.retrieveValue<DistanceAttenuationModel>("./distanceAttenuationModel",distanceAttenuationModel);
	
	#if ALSUPPORT_CONFIG_HAVE_OPENAL
	/* Open the OpenAL device: */
	std::string alDeviceName=configFileSection.retrieveValue<std::string>("./deviceName","Default");
	alDevice=alcOpenDevice(alDeviceName!="Default"?alDeviceName.c_str():0);
	if(alDevice==0)
		Misc::throwStdErr("SoundContext::SoundContext: Could not open OpenAL sound device \"%s\"",alDeviceName.c_str());
	
	/* Create a list of context attributes: */
	ALCint alContextAttributes[9];
	ALCint* attPtr=alContextAttributes;
	if(configFileSection.hasTag("./mixerFrequency"))
		{
		*(attPtr++)=ALC_FREQUENCY;
		*(attPtr++)=configFileSection.retrieveValue<ALCint>("./mixerFrequency");
		}
	if(configFileSection.hasTag("./refreshFrequency"))
		{
		*(attPtr++)=ALC_REFRESH;
		*(attPtr++)=configFileSection.retrieveValue<ALCint>("./refreshFrequency");
		}
	if(configFileSection.hasTag("./numMonoSources"))
		{
		*(attPtr++)=ALC_MONO_SOURCES;
		*(attPtr++)=configFileSection.retrieveValue<ALCint>("./numMonoSources");
		}
	if(configFileSection.hasTag("./numStereoSources"))
		{
		*(attPtr++)=ALC_STEREO_SOURCES;
		*(attPtr++)=configFileSection.retrieveValue<ALCint>("./numStereoSources");
		}
	*(attPtr++)=ALC_INVALID;
	
	/* Create an OpenAL context: */
	alContext=alcCreateContext(alDevice,alContextAttributes);
	if(alContext==0)
		{
		alcCloseDevice(alDevice);
		Misc::throwStdErr("SoundContext::SoundContext: Could not create OpenAL context for sound device %s",alDeviceName.c_str());
		}
	#endif
	
	/* Create an AL context data object: */
	contextData=new ALContextData(101);
	
	/* Initialize the sound context's OpenAL context: */
	makeCurrent();
	
	#if ALSUPPORT_CONFIG_HAVE_OPENAL
	/* Set global OpenAL parameters: */
	alSpeedOfSound(speedOfSound);
	alDopplerFactor(dopplerFactor);
	switch(distanceAttenuationModel)
		{
		case CONSTANT:
			alDistanceModel(AL_NONE);
			break;
		
		case INVERSE:
			alDistanceModel(AL_INVERSE_DISTANCE);
			break;
		
		case INVERSE_CLAMPED:
			alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
			break;
		
		case LINEAR:
			alDistanceModel(AL_LINEAR_DISTANCE);
			break;
		
		case LINEAR_CLAMPED:
			alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
			break;
		
		case EXPONENTIAL:
			alDistanceModel(AL_EXPONENT_DISTANCE);
			break;
		
		case EXPONENTIAL_CLAMPED:
			alDistanceModel(AL_EXPONENT_DISTANCE_CLAMPED);
			break;
		}
	#endif
	}

SoundContext::~SoundContext(void)
	{
	ALContextData::makeCurrent(0);
	delete contextData;
	
	#if ALSUPPORT_CONFIG_HAVE_OPENAL
	if(alcGetCurrentContext()==alContext)
		alcMakeContextCurrent(0);
	alcDestroyContext(alContext);
	alcCloseDevice(alDevice);
	#endif
	}

void SoundContext::makeCurrent(void)
	{
	#if ALSUPPORT_CONFIG_HAVE_OPENAL
	/* Activate the sound context's OpenAL context: */
	alcMakeContextCurrent(alContext);
	#endif
	
	/* Install the sound context's AL context data manager: */
	ALContextData::makeCurrent(contextData);
	}

void SoundContext::draw(void)
	{
	makeCurrent();
	
	/* Update things in the sound context's AL context data: */
	contextData->updateThings();
	
	#if ALSUPPORT_CONFIG_HAVE_OPENAL
	/* Set the listener in physical coordinates: */
	contextData->resetMatrixStack();
	alListenerPosition(listener->getHeadPosition());
	alListenerVelocity(Vector::zero);
	alListenerOrientation(listener->getListenDirection(),listener->getUpDirection());
	alListenerGain(listener->getGain());
	
	/* Render Vrui state: */
	vruiState->sound(*contextData);
	
	/* Check for OpenAL errors: */
	ALenum error;
	ALContextData::Error alcdError=ALContextData::NO_ERROR;
	while((error=alGetError())!=AL_NO_ERROR||(alcdError=contextData->getError())!=ALContextData::NO_ERROR)
		{
		printf("AL error: ");
		switch(error)
			{
			case AL_INVALID_ENUM:
				printf("Invalid enum");
				break;
			
			case AL_INVALID_NAME:
				printf("Invalid name");
				break;
			
			case AL_INVALID_OPERATION:
				printf("Invalid operation");
				break;
			
			case AL_INVALID_VALUE:
				printf("Invalid value");
				break;
			
			case AL_OUT_OF_MEMORY:
				printf("Out of memory");
				break;
			
			default:
				;
			}
		switch(alcdError)
			{
			case ALContextData::STACK_OVERFLOW:
				printf("Stack overflow");
				break;
			
			case ALContextData::STACK_UNDERFLOW:
				printf("Stack underflow");
				break;
			
			default:
				;
			}
		printf("\n");
		}
	#endif
	}

}
