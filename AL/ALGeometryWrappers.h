/***********************************************************************
ALGeometryWrappers - Wrapper functions to use templatized geometry
library objects as parameters to OpenAL functions.
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

#ifndef ALGEOMETRYWRAPPERS_INCLUDED
#define ALGEOMETRYWRAPPERS_INCLUDED

#include <AL/Config.h>

#include <AL/ALTemplates.h>

/* Forward declarations: */
namespace Geometry {
template <class ScalarParam,int dimensionParam>
class Point;
template <class ScalarParam,int dimensionParam>
class Vector;
}

#if ALSUPPORT_CONFIG_HAVE_OPENAL

/***************************************
Generic versions of alListener... calls:
***************************************/

template <class ScalarParam>
inline void alListenerPosition(const Geometry::Point<ScalarParam,3>& value)
	{
	alListener<3>(ALListenerEnums::POSITION,value.getComponents());
	}

template <class ScalarParam>
inline void alListenerVelocity(const Geometry::Vector<ScalarParam,3>& value)
	{
	alListener<3>(ALListenerEnums::VELOCITY,value.getComponents());
	}

template <class ScalarParam>
inline void alListenerOrientation(const Geometry::Vector<ScalarParam,3>& forward,const Geometry::Vector<ScalarParam,3>& up)
	{
	ScalarParam tempValues[6];
	for(int i=0;i<3;++i)
		{
		tempValues[0+i]=forward[i];
		tempValues[3+i]=up[i];
		}
	alListener<6>(ALListenerEnums::ORIENTATION,tempValues);
	}

/******************************************************
Versions of alListener... calls using a transformation:
******************************************************/

template <class ScalarParam,class TransformParam>
inline void alListenerPosition(const Geometry::Point<ScalarParam,3>& value,const TransformParam& transform)
	{
	alListenerPosition(transform.transform(value));
	}

template <class ScalarParam,class TransformParam>
inline void alListenerVelocity(const Geometry::Vector<ScalarParam,3>& value,const TransformParam& transform)
	{
	alListenerVelocity(transform.transform(value));
	}

template <class ScalarParam,class TransformParam>
inline void alListenerOrientation(const Geometry::Vector<ScalarParam,3>& forward,const Geometry::Vector<ScalarParam,3>& up,const TransformParam& transform)
	{
	alListenerOrientation(transform.transform(forward),transform.transform(up));
	}

/*************************************
Generic versions of alSource... calls:
*************************************/

template <class ScalarParam>
inline void alSourcePosition(ALuint sid,const Geometry::Point<ScalarParam,3>& value)
	{
	alSource<3>(sid,ALSourceEnums::POSITION,value.getComponents());
	}

template <class ScalarParam>
inline void alSourceVelocity(ALuint sid,const Geometry::Vector<ScalarParam,3>& value)
	{
	alSource<3>(sid,ALSourceEnums::VELOCITY,value.getComponents());
	}

template <class ScalarParam>
inline void alSourceDirection(ALuint sid,const Geometry::Vector<ScalarParam,3>& value)
	{
	alSource<3>(sid,ALSourceEnums::DIRECTION,value.getComponents());
	}

/****************************************************
Versions of alSource... calls using a transformation:
****************************************************/

template <class ScalarParam,class TransformParam>
inline void alSourcePosition(ALuint sid,const Geometry::Point<ScalarParam,3>& value,const TransformParam& transform)
	{
	alSourcePosition(sid,transform.transform(value));
	}

template <class ScalarParam,class TransformParam>
inline void alSourceVelocity(ALuint sid,const Geometry::Vector<ScalarParam,3>& value,const TransformParam& transform)
	{
	alSourceVelocity(sid,transform.transform(value));
	}

template <class ScalarParam,class TransformParam>
inline void alSourceDirection(ALuint sid,const Geometry::Vector<ScalarParam,3>& value,const TransformParam& transform)
	{
	alSourceDirection(sid,transform.transform(value));
	}

#endif

#endif
