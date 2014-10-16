/***********************************************************************
GLScalarConverter - Helper class to convert color, normal and depth
scalar values between different scalar data types.
Copyright (c) 2004-2005 Oliver Kreylos

This file is part of the OpenGL C++ Wrapper Library (GLWrappers).

The OpenGL C++ Wrapper Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The OpenGL C++ Wrapper Library is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the OpenGL C++ Wrapper Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef GLSCALARCONVERTER_INCLUDED
#define GLSCALARCONVERTER_INCLUDED

#include <GL/gl.h>
#include <GL/GLScalarLimits.h>

/****************************************************************************************************
Generic class to convert range-limited scalar values between integral types and floating-point types:
****************************************************************************************************/

/* Dummy generic class: */

template <class FloatParam,class IntegralParam,class TraitParam>
class GLIntegralFloatConverter
	{
	};

/* Specialization for unsigned integral types: */

template <class FloatParam,class IntegralParam>
class GLIntegralFloatConverter<FloatParam,IntegralParam,GLScalarUnsignedTrait>
	{
	/* Methods: */
	public:
	static inline FloatParam toFloat(IntegralParam value)
		{
		return FloatParam(value)/FloatParam(GLScalarLimits<IntegralParam>::scale);
		}
	static inline IntegralParam toIntegral(FloatParam value)
		{
		return IntegralParam(value*FloatParam(GLScalarLimits<IntegralParam>::scale));
		}
	};

/* Specialization for signed integral types: */

template <class FloatParam,class IntegralParam>
class GLIntegralFloatConverter<FloatParam,IntegralParam,GLScalarSignedTrait>
	{
	/* Methods: */
	public:
	static inline FloatParam toFloat(IntegralParam value)
		{
		return (FloatParam(value)*FloatParam(2)+FloatParam(1))/FloatParam(GLScalarLimits<IntegralParam>::scale);
		}
	static inline IntegralParam toIntegral(FloatParam value)
		{
		return IntegralParam((value*FloatParam(GLScalarLimits<IntegralParam>::scale)-FloatParam(1))/FloatParam(2));
		}
	};

/* Specialization for float types: */

template <class FloatParam,class IntegralParam>
class GLIntegralFloatConverter<FloatParam,IntegralParam,GLScalarFloatTrait>
	{
	/* Methods: */
	public:
	static inline FloatParam toFloat(IntegralParam value)
		{
		return FloatParam(value);
		}
	static inline IntegralParam toIntegral(FloatParam value)
		{
		return IntegralParam(value);
		}
	};

/******************************************************************************************
Generic class to convert range-limited scalar values between arbitrary OpenGL scalar types:
******************************************************************************************/

/* Generic class to convert between any OpenGL scalar types: */

template <class DestScalarParam,class DestTraitParam,class SourceScalarParam,class SourceTraitParam>
class GLScalarConverter
	{
	/* Embedded classes: */
	public:
	typedef GLIntegralFloatConverter<GLdouble,DestScalarParam,DestTraitParam> DestConverter;
	typedef GLIntegralFloatConverter<GLdouble,SourceScalarParam,SourceTraitParam> SourceConverter;
	
	/* Methods: */
	inline static DestScalarParam convert(SourceScalarParam value)
		{
		/* Convert through GLdouble: */
		GLdouble temp=SourceConverter::toFloat(value);
		return DestConverter::toIntegral(temp);
		}
	};

/* Specialized version to convert to float scalar types: */

template <class DestScalarParam,class SourceScalarParam,class SourceTraitParam>
class GLScalarConverter<DestScalarParam,GLScalarFloatTrait,SourceScalarParam,SourceTraitParam>
	{
	/* Embedded classes: */
	public:
	typedef GLIntegralFloatConverter<DestScalarParam,SourceScalarParam,SourceTraitParam> Converter;
	
	/* Methods: */
	inline static DestScalarParam convert(SourceScalarParam value)
		{
		/* Convert directly: */
		return Converter::toFloat(value);
		}
	};

/* Specialized version to convert from float scalar types: */

template <class DestScalarParam,class DestTraitParam,class SourceScalarParam>
class GLScalarConverter<DestScalarParam,DestTraitParam,SourceScalarParam,GLScalarFloatTrait>
	{
	/* Embedded classes: */
	public:
	typedef GLIntegralFloatConverter<SourceScalarParam,DestScalarParam,DestTraitParam> Converter;
	
	/* Methods: */
	inline static DestScalarParam convert(SourceScalarParam value)
		{
		/* Convert directly: */
		return Converter::toIntegral(value);
		}
	};

/* Specialized version to convert between float scalar types: */

template <class DestScalarParam,class SourceScalarParam>
class GLScalarConverter<DestScalarParam,GLScalarFloatTrait,SourceScalarParam,GLScalarFloatTrait>
	{
	/* Methods: */
	public:
	inline static DestScalarParam convert(SourceScalarParam value)
		{
		/* Cast directly: */
		return DestScalarParam(value);
		}
	};

/* Specialized version for identity conversions: */

template <class ScalarParam,class TraitParam>
class GLScalarConverter<ScalarParam,TraitParam,ScalarParam,TraitParam>
	{
	/* Methods: */
	public:
	inline static ScalarParam convert(ScalarParam value)
		{
		return value;
		}
	};

/* Specialized versions for common conversions between integral types: */

template <>
class GLScalarConverter<GLubyte,GLScalarUnsignedTrait,GLushort,GLScalarUnsignedTrait>
	{
	/* Methods: */
	public:
	inline static GLubyte convert(GLushort value)
		{
		return GLubyte(value>>8);
		}
	};

template <>
class GLScalarConverter<GLushort,GLScalarUnsignedTrait,GLubyte,GLScalarUnsignedTrait>
	{
	/* Methods: */
	public:
	inline static GLushort convert(GLubyte value)
		{
		return (GLushort(value)<<8)|GLushort(value);
		}
	};

/***********************************************************
Helper function with automatic template parameter detection:
***********************************************************/

template <class DestScalarParam,class SourceScalarParam>
inline DestScalarParam glConvertScalar(SourceScalarParam source)
	{
	return GLScalarConverter<DestScalarParam,typename GLScalarLimits<DestScalarParam>::Trait,SourceScalarParam,typename GLScalarLimits<SourceScalarParam>::Trait>::convert(source);
	}

#endif
