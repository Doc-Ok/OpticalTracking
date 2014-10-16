/***********************************************************************
ALTemplates - Overloaded versions of OpenAL functions.
Copyright (c) 2009-2010 Oliver Kreylos

This file is part of the OpenAL Support Library (ALSupport).

The OpenAL Support Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The OpenAL Support Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the OpenAL Support Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef ALTEMPLATES_INCLUDED
#define ALTEMPLATES_INCLUDED

#include <AL/Config.h>

#if ALSUPPORT_CONFIG_HAVE_OPENAL

/***********************************************************
Structure to define enumeration values for alListener calls:
***********************************************************/

struct ALListenerEnums
	{
	/* Embedded classes: */
	public:
	enum Param
		{
		GAIN=AL_GAIN,
		POSITION=AL_POSITION,
		VELOCITY=AL_VELOCITY,
		ORIENTATION=AL_ORIENTATION
		};
	};

/****************************************************
Overloaded versions of direct-value alListener calls:
****************************************************/

inline void alListener(ALListenerEnums::Param param,ALint value)
	{
	alListeneri(param,value);
	}

inline void alListener(ALListenerEnums::Param param,ALfloat value)
	{
	alListenerf(param,value);
	}

inline void alListener(ALListenerEnums::Param param,ALdouble value)
	{
	alListenerf(param,ALfloat(value));
	}

inline void alListener(ALListenerEnums::Param param,ALint value1,ALint value2,ALint value3)
	{
	alListener3i(param,value1,value2,value3);
	}

inline void alListener(ALListenerEnums::Param param,ALfloat value1,ALfloat value2,ALfloat value3)
	{
	alListener3f(param,value1,value2,value3);
	}

inline void alListener(ALListenerEnums::Param param,ALdouble value1,ALdouble value2,ALdouble value3)
	{
	alListener3f(param,ALfloat(value1),ALfloat(value2),ALfloat(value3));
	}

/****************************************************
Dummy generic version of array-based alListener call:
****************************************************/

template <int numValuesParam,class ScalarParam>
void alListener(ALListenerEnums::Param param,const ScalarParam values[numValuesParam]);

/****************************************************
Specialized versions of array-based alListener calls:
****************************************************/

template <>
inline void alListener<1,ALint>(ALListenerEnums::Param param,const ALint values[1])
	{
	alListeneriv(param,values);
	}

template <>
inline void alListener<1,ALfloat>(ALListenerEnums::Param param,const ALfloat values[1])
	{
	alListenerfv(param,values);
	}

template <>
inline void alListener<1,ALdouble>(ALListenerEnums::Param param,const ALdouble values[1])
	{
	ALfloat tempValues[1];
	for(int i=0;i<1;++i)
		tempValues[i]=ALfloat(values[i]);
	alListenerfv(param,tempValues);
	}

template <>
inline void alListener<3,ALint>(ALListenerEnums::Param param,const ALint values[3])
	{
	alListeneriv(param,values);
	}

template <>
inline void alListener<3,ALfloat>(ALListenerEnums::Param param,const ALfloat values[3])
	{
	alListenerfv(param,values);
	}

template <>
inline void alListener<3,ALdouble>(ALListenerEnums::Param param,const ALdouble values[3])
	{
	ALfloat tempValues[3];
	for(int i=0;i<3;++i)
		tempValues[i]=ALfloat(values[i]);
	alListenerfv(param,tempValues);
	}

template <>
inline void alListener<6,ALint>(ALListenerEnums::Param param,const ALint values[6])
	{
	alListeneriv(param,values);
	}

template <>
inline void alListener<6,ALfloat>(ALListenerEnums::Param param,const ALfloat values[6])
	{
	alListenerfv(param,values);
	}

template <>
inline void alListener<6,ALdouble>(ALListenerEnums::Param param,const ALdouble values[6])
	{
	ALfloat tempValues[6];
	for(int i=0;i<6;++i)
		tempValues[i]=ALfloat(values[i]);
	alListenerfv(param,tempValues);
	}

/***************************************
Generic versions of alListener... calls:
***************************************/

inline void alListenerGain(ALfloat value)
	{
	alListener(ALListenerEnums::GAIN,value);
	}

template <class ScalarParam>
inline void alListenerPosition(const ScalarParam values[3])
	{
	alListener<3>(ALListenerEnums::POSITION,values);
	}

template <class ScalarParam>
inline void alListenerVelocity(const ScalarParam values[3])
	{
	alListener<3>(ALListenerEnums::VELOCITY,values);
	}

template <class ScalarParam>
inline void alListenerOrientation(const ScalarParam values[6])
	{
	alListener<6>(ALListenerEnums::ORIENTATION,values);
	}

/*********************************************************
Structure to define enumeration values for alSource calls:
*********************************************************/

struct ALSourceEnums
	{
	/* Embedded classes: */
	public:
	enum Param
		{
		GAIN=AL_GAIN,
		MIN_GAIN=AL_MIN_GAIN,
		MAX_GAIN=AL_MAX_GAIN,
		POSITION=AL_POSITION,
		VELOCITY=AL_VELOCITY,
		DIRECTION=AL_DIRECTION,
		SOURCE_RELATIVE=AL_SOURCE_RELATIVE,
		REFERENCE_DISTANCE=AL_REFERENCE_DISTANCE,
		MAX_DISTANCE=AL_MAX_DISTANCE,
		ROLLOFF_FACTOR=AL_ROLLOFF_FACTOR,
		CONE_INNER_ANGLE=AL_CONE_INNER_ANGLE,
		CONE_OUTER_ANGLE=AL_CONE_OUTER_ANGLE,
		CONE_OUTER_GAIN=AL_CONE_OUTER_GAIN,
		PITCH=AL_PITCH,
		LOOPING=AL_LOOPING,
		BYTE_OFFSET=AL_BYTE_OFFSET,
		SAMPLE_OFFSET=AL_SAMPLE_OFFSET,
		BUFFER=AL_BUFFER,
		SOURCE_STATE=AL_SOURCE_STATE,
		BUFFERS_QUEUED=AL_BUFFERS_QUEUED,
		BUFFERS_PROCESSED=AL_BUFFERS_PROCESSED
		};
	};

/**************************************************
Overloaded versions of direct-value alSource calls:
**************************************************/

inline void alSource(ALuint sid,ALSourceEnums::Param param,ALint value)
	{
	alSourcei(sid,param,value);
	}

inline void alSource(ALuint sid,ALSourceEnums::Param param,ALfloat value)
	{
	alSourcef(sid,param,value);
	}

inline void alSource(ALuint sid,ALSourceEnums::Param param,ALdouble value)
	{
	alSourcef(sid,param,ALfloat(value));
	}

inline void alSource(ALuint sid,ALSourceEnums::Param param,ALint value1,ALint value2,ALint value3)
	{
	alSource3i(sid,param,value1,value2,value3);
	}

inline void alSource(ALuint sid,ALSourceEnums::Param param,ALfloat value1,ALfloat value2,ALfloat value3)
	{
	alSource3f(sid,param,value1,value2,value3);
	}

inline void alSource(ALuint sid,ALSourceEnums::Param param,ALdouble value1,ALdouble value2,ALdouble value3)
	{
	alSource3f(sid,param,ALfloat(value1),ALfloat(value2),ALfloat(value3));
	}

/**************************************************
Dummy generic version of array-based alSource call:
**************************************************/

template <int numValuesParam,class ScalarParam>
void alSource(ALuint sid,ALSourceEnums::Param param,const ScalarParam values[numValuesParam]);

/**************************************************
Specialized versions of array-based alSource calls:
**************************************************/

template <>
inline void alSource<1,ALint>(ALuint sid,ALSourceEnums::Param param,const ALint values[1])
	{
	alSourceiv(sid,param,values);
	}

template <>
inline void alSource<1,ALfloat>(ALuint sid,ALSourceEnums::Param param,const ALfloat values[1])
	{
	alSourcefv(sid,param,values);
	}

template <>
inline void alSource<1,ALdouble>(ALuint sid,ALSourceEnums::Param param,const ALdouble values[1])
	{
	ALfloat tempValues[1];
	for(int i=0;i<1;++i)
		tempValues[i]=ALfloat(values[i]);
	alSourcefv(sid,param,tempValues);
	}

template <>
inline void alSource<3,ALint>(ALuint sid,ALSourceEnums::Param param,const ALint values[3])
	{
	alSourceiv(sid,param,values);
	}

template <>
inline void alSource<3,ALfloat>(ALuint sid,ALSourceEnums::Param param,const ALfloat values[3])
	{
	alSourcefv(sid,param,values);
	}

template <>
inline void alSource<3,ALdouble>(ALuint sid,ALSourceEnums::Param param,const ALdouble values[3])
	{
	ALfloat tempValues[3];
	for(int i=0;i<3;++i)
		tempValues[i]=ALfloat(values[i]);
	alSourcefv(sid,param,tempValues);
	}

/*************************************
Generic versions of alSource... calls:
*************************************/

inline void alSourceGain(ALuint sid,ALfloat value)
	{
	alSource(sid,ALSourceEnums::GAIN,value);
	}

inline void alSourceMinGain(ALuint sid,ALfloat value)
	{
	alSource(sid,ALSourceEnums::MIN_GAIN,value);
	}

inline void alSourceMaxGain(ALuint sid,ALfloat value)
	{
	alSource(sid,ALSourceEnums::MAX_GAIN,value);
	}

template <class ScalarParam>
inline void alSourcePosition(ALuint sid,const ScalarParam values[3])
	{
	alSource<3>(sid,ALSourceEnums::POSITION,values);
	}

template <class ScalarParam>
inline void alSourceVelocity(ALuint sid,const ScalarParam values[3])
	{
	alSource<3>(sid,ALSourceEnums::VELOCITY,values);
	}

template <class ScalarParam>
inline void alSourceDirection(ALuint sid,const ScalarParam values[3])
	{
	alSource<3>(sid,ALSourceEnums::DIRECTION,values);
	}

inline void alSourceSourceRelative(ALuint sid,bool value)
	{
	alSource(sid,ALSourceEnums::SOURCE_RELATIVE,value?AL_TRUE:AL_FALSE);
	}

inline void alSourceReferenceDistance(ALuint sid,ALfloat value)
	{
	alSource(sid,ALSourceEnums::REFERENCE_DISTANCE,value);
	}

inline void alSourceMaxDistance(ALuint sid,ALfloat value)
	{
	alSource(sid,ALSourceEnums::MAX_DISTANCE,value);
	}

inline void alSourceRolloffFactor(ALuint sid,ALfloat value)
	{
	alSource(sid,ALSourceEnums::ROLLOFF_FACTOR,value);
	}

template <class ScalarParam>
inline void alSourceConeInnerAngle(ALuint sid,ScalarParam value)
	{
	alSource(sid,ALSourceEnums::CONE_INNER_ANGLE,value);
	}

template <class ScalarParam>
inline void alSourceConeOuterAngle(ALuint sid,ScalarParam value)
	{
	alSource(sid,ALSourceEnums::CONE_OUTER_ANGLE,value);
	}

template <class ScalarParam>
inline void alSourceConeOuterGain(ALuint sid,ScalarParam value)
	{
	alSource(sid,ALSourceEnums::CONE_OUTER_GAIN,value);
	}

inline void alSourcePitch(ALuint sid,ALfloat value)
	{
	alSource(sid,ALSourceEnums::PITCH,value);
	}

inline void alSourceLooping(ALuint sid,bool value)
	{
	alSource(sid,ALSourceEnums::LOOPING,value?AL_TRUE:AL_FALSE);
	}

template <class ScalarParam>
inline void alSourceByteOffset(ALuint sid,ScalarParam value)
	{
	alSource(sid,ALSourceEnums::BYTE_OFFSET,value);
	}

template <class ScalarParam>
inline void alSourceSampleOffset(ALuint sid,ScalarParam value)
	{
	alSource(sid,ALSourceEnums::SAMPLE_OFFSET,value);
	}

inline void alSourceBuffer(ALuint sid,ALint value)
	{
	alSource(sid,ALSourceEnums::BUFFER,value);
	}

/*********************************************************
Structure to define enumeration values for alBuffer calls:
*********************************************************/

struct ALBufferEnums
	{
	/* Embedded classes: */
	public:
	enum Param
		{
		FREQUENCY=AL_FREQUENCY,
		SIZE=AL_SIZE,
		BITS=AL_BITS,
		CHANNELS=AL_CHANNELS
		};
	};

/**************************************************
Overloaded versions of direct-value alBuffer calls:
**************************************************/

inline void alBuffer(ALuint bid,ALBufferEnums::Param param,ALint value)
	{
	alBufferi(bid,param,value);
	}

inline void alBuffer(ALuint bid,ALBufferEnums::Param param,ALfloat value)
	{
	alBufferf(bid,param,value);
	}

inline void alBuffer(ALuint bid,ALBufferEnums::Param param,ALdouble value)
	{
	alBufferf(bid,param,ALfloat(value));
	}

inline void alBuffer(ALuint bid,ALBufferEnums::Param param,ALint value1,ALint value2,ALint value3)
	{
	alBuffer3i(bid,param,value1,value2,value3);
	}

inline void alBuffer(ALuint bid,ALBufferEnums::Param param,ALfloat value1,ALfloat value2,ALfloat value3)
	{
	alBuffer3f(bid,param,value1,value2,value3);
	}

inline void alBuffer(ALuint bid,ALBufferEnums::Param param,ALdouble value1,ALdouble value2,ALdouble value3)
	{
	alBuffer3f(bid,param,ALfloat(value1),ALfloat(value2),ALfloat(value3));
	}

/**************************************************
Dummy generic version of array-based alBuffer call:
**************************************************/

template <int numValuesParam,class ScalarParam>
void alBuffer(ALuint bid,ALBufferEnums::Param param,const ScalarParam values[numValuesParam]);

/**************************************************
Specialized versions of array-based alBuffer calls:
**************************************************/

template <>
inline void alBuffer<1,ALint>(ALuint bid,ALBufferEnums::Param param,const ALint values[1])
	{
	alBufferiv(bid,param,values);
	}

template <>
inline void alBuffer<1,ALfloat>(ALuint bid,ALBufferEnums::Param param,const ALfloat values[1])
	{
	alBufferfv(bid,param,values);
	}

template <>
inline void alBuffer<1,ALdouble>(ALuint bid,ALBufferEnums::Param param,const ALdouble values[1])
	{
	ALfloat tempValues[1];
	for(int i=0;i<1;++i)
		tempValues[i]=ALfloat(values[i]);
	alBufferfv(bid,param,tempValues);
	}

template <>
inline void alBuffer<3,ALint>(ALuint bid,ALBufferEnums::Param param,const ALint values[3])
	{
	alBufferiv(bid,param,values);
	}

template <>
inline void alBuffer<3,ALfloat>(ALuint bid,ALBufferEnums::Param param,const ALfloat values[3])
	{
	alBufferfv(bid,param,values);
	}

template <>
inline void alBuffer<3,ALdouble>(ALuint bid,ALBufferEnums::Param param,const ALdouble values[3])
	{
	ALfloat tempValues[3];
	for(int i=0;i<3;++i)
		tempValues[i]=ALfloat(values[i]);
	alBufferfv(bid,param,tempValues);
	}

#endif

#endif
