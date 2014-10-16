/***********************************************************************
GLColor - Class to represent color values in RGBA format.
Copyright (c) 2003-2012 Oliver Kreylos

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

#ifndef GLCOLOR_INCLUDED
#define GLCOLOR_INCLUDED

#include <GL/gl.h>
#include <GL/GLScalarLimits.h>

/********************************************************
GLColorBase: Generic base class for colors in RGBA format
********************************************************/

template <class ScalarParam,GLsizei numComponentsParam>
class GLColorBase
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Scalar type
	static const GLsizei numComponents=numComponentsParam; // Number of stored color components
	
	/* Elements: */
	protected:
	Scalar rgba[numComponentsParam]; // RGBA components
	
	/* Helper methods: */
	void copy(GLsizei sNumComponents,const Scalar* sRgba);
	template <class SourceScalarParam>
	void copy(GLsizei sNumComponents,const SourceScalarParam* sRgba);
	
	/* Constructors and destructors: */
	protected:
	inline GLColorBase(void)
		{
		}
	template <class SourceScalarParam>
	inline GLColorBase(GLsizei sNumComponents,const SourceScalarParam* sRgba)
		{
		copy(sNumComponents,sRgba);
		}
	
	/* Methods: */
	public:
	inline const Scalar* getRgba(void) const // Returns color components as array
		{
		return rgba;
		}
	inline Scalar* getRgba(void) // Ditto
		{
		return rgba;
		}
	inline Scalar operator[](GLsizei index) const // Returns a single color component
		{
		return rgba[index];
		}
	inline Scalar& operator[](GLsizei index) // Ditto
		{
		return rgba[index];
		}
	};

/**************************************
Dummy generic version of GLColor class:
**************************************/

template <class ScalarParam =GLubyte,GLsizei numComponentsParam =4>
class GLColor
	{
	};

/*************************************
Specialized versions of GLColor class:
*************************************/

template <class ScalarParam>
class GLColor<ScalarParam,3>:public GLColorBase<ScalarParam,3>
	{
	/* Embedded classes: */
	public:
	typedef GLColorBase<ScalarParam,3> BaseClass;
	
	/* Declarations of inherited types/elements: */
	using GLColorBase<ScalarParam,3>::rgba;
	
	/* Constructors and destructors: */
	inline GLColor(void)
		{
		}
	inline GLColor(typename BaseClass::Scalar sRed,typename BaseClass::Scalar sGreen,typename BaseClass::Scalar sBlue)
		{
		rgba[0]=sRed;
		rgba[1]=sGreen;
		rgba[2]=sBlue;
		}
	template <class SourceScalarParam>
	inline GLColor(const SourceScalarParam sRgba[3])
		:BaseClass(3,sRgba)
		{
		}
	template <class SourceScalarParam>
	inline GLColor(const GLColor<SourceScalarParam,3>& source)
		:BaseClass(3,source.getRgba())
		{
		}
	template <class SourceScalarParam>
	inline GLColor(const GLColor<SourceScalarParam,4>& source)
		:BaseClass(3,source.getRgba())
		{
		}
	template <class SourceScalarParam>
	GLColor& operator=(const GLColor<SourceScalarParam,3>& source)
		{
		BaseClass::copy(3,source.getRgba());
		return *this;
		}
	template <class SourceScalarParam>
	GLColor& operator=(const GLColor<SourceScalarParam,4>& source)
		{
		BaseClass::copy(3,source.getRgba());
		return *this;
		}
	};

template <class ScalarParam>
class GLColor<ScalarParam,4>:public GLColorBase<ScalarParam,4>
	{
	/* Embedded classes: */
	public:
	typedef GLColorBase<ScalarParam,4> BaseClass;
	
	/* Declarations of inherited types/elements: */
	using GLColorBase<ScalarParam,4>::rgba;
	
	/* Constructors and destructors: */
	inline GLColor(void)
		{
		}
	inline GLColor(typename BaseClass::Scalar sRed,typename BaseClass::Scalar sGreen,typename BaseClass::Scalar sBlue)
		{
		rgba[0]=sRed;
		rgba[1]=sGreen;
		rgba[2]=sBlue;
		rgba[3]=GLScalarLimits<typename BaseClass::Scalar>::max;
		}
	inline GLColor(typename BaseClass::Scalar sRed,typename BaseClass::Scalar sGreen,typename BaseClass::Scalar sBlue,typename BaseClass::Scalar sAlpha)
		{
		rgba[0]=sRed;
		rgba[1]=sGreen;
		rgba[2]=sBlue;
		rgba[3]=sAlpha;
		}
	template <class SourceScalarParam>
	inline GLColor(const SourceScalarParam sRgba[4])
		:BaseClass(4,sRgba)
		{
		}
	template <class SourceScalarParam>
	inline GLColor(const GLColor<SourceScalarParam,3>& source)
		:BaseClass(3,source.getRgba())
		{
		rgba[3]=GLScalarLimits<typename BaseClass::Scalar>::max;
		}
	template <class SourceScalarParam>
	inline GLColor(const GLColor<SourceScalarParam,4>& source)
		:BaseClass(4,source.getRgba())
		{
		}
	template <class SourceScalarParam>
	GLColor& operator=(const GLColor<SourceScalarParam,3>& source)
		{
		BaseClass::copy(3,source.getRgba());
		rgba[3]=GLScalarLimits<typename BaseClass::Scalar>::max;
		return *this;
		}
	template <class SourceScalarParam>
	GLColor& operator=(const GLColor<SourceScalarParam,4>& source)
		{
		BaseClass::copy(4,source.getRgba());
		return *this;
		}
	};

/* Comparison operators: */

template <class ScalarParam,GLsizei numComponentsParam>
inline bool operator==(const GLColor<ScalarParam,numComponentsParam>& c1,const GLColor<ScalarParam,numComponentsParam>& c2)
	{
	bool result=true;
	for(GLsizei i=0;i<numComponentsParam&&result;++i)
		result=c1[i]==c2[i];
	return result;
	}

template <class ScalarParam,GLsizei numComponentsParam>
inline bool operator!=(const GLColor<ScalarParam,numComponentsParam>& c1,const GLColor<ScalarParam,numComponentsParam>& c2)
	{
	bool result=false;
	for(GLsizei i=0;i<numComponentsParam&&!result;++i)
		result=c1[i]!=c2[i];
	return result;
	}

#endif
