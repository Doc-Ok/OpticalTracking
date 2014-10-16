/***********************************************************************
GLMarshallers - Marshaller classes for OpenGL abstraction classes.
Copyright (c) 2010 Oliver Kreylos

This file is part of the OpenGL Support Library (GLSupport).

The OpenGL Support Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The OpenGL Support Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the OpenGL Support Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef GLMARSHALLERS_INCLUDED
#define GLMARSHALLERS_INCLUDED

#include <Misc/Marshaller.h>
#include <GL/gl.h>
#include <GL/GLMaterial.h>

/* Forward declarations: */
template <class ScalarParam,GLsizei numComponentsParam>
class GLColor;
template <class ScalarParam,GLsizei numComponentsParam>
class GLVector;
template <class ScalarParam,GLsizei numComponentsParam>
class GLBox;

namespace Misc {

template <class ScalarParam,int numComponentsParam>
class Marshaller<GLColor<ScalarParam,numComponentsParam> >
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar;
	static const int numComponents=numComponentsParam;
	typedef GLColor<ScalarParam,numComponentsParam> Value;
	
	/* Methods: */
	static size_t getSize(const Value& value);
	template <class DataSinkParam>
	static void write(const Value& value,DataSinkParam& sink);
	template <class DataSourceParam>
	static Value read(DataSourceParam& source);
	};

template <class ScalarParam,int numComponentsParam>
class Marshaller<GLVector<ScalarParam,numComponentsParam> >
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar;
	static const int numComponents=numComponentsParam;
	typedef GLVector<ScalarParam,numComponentsParam> Value;
	
	/* Methods: */
	static size_t getSize(const Value& value);
	template <class DataSinkParam>
	static void write(const Value& value,DataSinkParam& sink);
	template <class DataSourceParam>
	static Value read(DataSourceParam& source);
	};

template <class ScalarParam,int numComponentsParam>
class Marshaller<GLBox<ScalarParam,numComponentsParam> >
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar;
	static const int numComponents=numComponentsParam;
	typedef GLBox<ScalarParam,numComponentsParam> Value;
	typedef typename Value::Vector Vector;
	
	/* Methods: */
	static size_t getSize(const Value& value);
	template <class DataSinkParam>
	static void write(const Value& value,DataSinkParam& sink);
	template <class DataSourceParam>
	static Value read(DataSourceParam& source);
	};

template <>
class Marshaller<GLMaterial>
	{
	/* Embedded classes: */
	public:
	typedef GLMaterial Value;
	typedef Value::Scalar Scalar;
	typedef Value::Color Color;
	
	/* Methods: */
	static size_t getSize(const Value& value);
	template <class DataSinkParam>
	static void write(const Value& value,DataSinkParam& sink);
	template <class DataSourceParam>
	static Value read(DataSourceParam& source);
	};

}

#if !defined(GLMARSHALLERS_IMPLEMENTATION)
#include <GL/GLMarshallers.icpp>
#endif

#endif
