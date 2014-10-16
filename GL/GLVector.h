/***********************************************************************
GLVector - Class to represent homogenuous three-dimensional vectors,
storing only the first n components of a vector.
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

#ifndef GLVECTOR_INCLUDED
#define GLVECTOR_INCLUDED

#include <GL/gl.h>

/*************************************************************************************
GLVectorCopier: Helper class to copy vectors with scalar type and dimension conversion
*************************************************************************************/

template <class DestScalarParam,class SourceScalarParam,GLsizei destDimensionParam,GLsizei sourceDimensionParam>
class GLVectorCopier
	{
	/* Methods: */
	public:
	inline static void copy(DestScalarParam* dest,const SourceScalarParam* source)
		{
		for(GLsizei i=0;i<sourceDimensionParam&&i<destDimensionParam;++i)
			dest[i]=DestScalarParam(source[i]);
		for(GLsizei i=sourceDimensionParam;i<destDimensionParam;++i)
			dest[i]=DestScalarParam(0);
		}
	};

template <class DestScalarParam,class SourceScalarParam,GLsizei dimensionParam>
class GLVectorCopier<DestScalarParam,SourceScalarParam,dimensionParam,dimensionParam>
	{
	/* Methods: */
	public:
	inline static void copy(DestScalarParam* dest,const SourceScalarParam* source)
		{
		for(GLsizei i=0;i<dimensionParam;++i)
			dest[i]=DestScalarParam(source[i]);
		}
	};

template <class ScalarParam,GLsizei destDimensionParam,GLsizei sourceDimensionParam>
class GLVectorCopier<ScalarParam,ScalarParam,destDimensionParam,sourceDimensionParam>
	{
	/* Methods: */
	public:
	inline static void copy(ScalarParam* dest,const ScalarParam* source)
		{
		for(GLsizei i=0;i<sourceDimensionParam&&i<destDimensionParam;++i)
			dest[i]=source[i];
		for(GLsizei i=sourceDimensionParam;i<destDimensionParam;++i)
			dest[i]=ScalarParam(0);
		}
	};

template <class ScalarParam,GLsizei dimensionParam>
class GLVectorCopier<ScalarParam,ScalarParam,dimensionParam,dimensionParam>
	{
	/* Methods: */
	public:
	inline static void copy(ScalarParam* dest,const ScalarParam* source)
		{
		for(GLsizei i=0;i<dimensionParam;++i)
			dest[i]=source[i];
		}
	};

/*******************************************
GLVectorBase: Generic base class for vectors
*******************************************/

template <class ScalarParam,GLsizei numComponentsParam>
class GLVectorBase
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Scalar type
	static const GLsizei numComponents=numComponentsParam; // Number of stored vector components
	
	/* Elements: */
	protected:
	Scalar xyzw[numComponents]; // Vector components
	
	/* Constructors and destructors: */
	protected:
	inline GLVectorBase(void)
		{
		}
	template <class SourceScalarParam>
	inline GLVectorBase(const SourceScalarParam sXyzw[numComponentsParam])
		{
		GLVectorCopier<Scalar,SourceScalarParam,numComponents,numComponents>::copy(xyzw,sXyzw);
		}
	template <class SourceScalarParam,GLsizei sourceNumComponentsParam>
	inline GLVectorBase(const GLVectorBase<SourceScalarParam,sourceNumComponentsParam>& source)
		{
		GLVectorCopier<Scalar,SourceScalarParam,numComponents,sourceNumComponentsParam>::copy(xyzw,source.getXyzw());
		}
	template <class SourceScalarParam>
	inline GLVectorBase& operator=(const SourceScalarParam sXyzw[numComponentsParam])
		{
		GLVectorCopier<Scalar,SourceScalarParam,numComponents,numComponents>::copy(xyzw,sXyzw);
		return *this;
		}
	template <class SourceScalarParam,GLsizei sourceNumComponentsParam>
	inline GLVectorBase& operator=(const GLVectorBase<SourceScalarParam,sourceNumComponentsParam>& source)
		{
		GLVectorCopier<Scalar,SourceScalarParam,numComponents,sourceNumComponentsParam>::copy(xyzw,source.getXyzw());
		return *this;
		}
	
	/* Methods: */
	public:
	inline const Scalar* getXyzw(void) const // Returns vector components as array
		{
		return xyzw;
		}
	inline Scalar* getXyzw(void) // Ditto
		{
		return xyzw;
		}
	inline Scalar operator[](GLsizei index) const // Returns a single vector component
		{
		return xyzw[index];
		}
	inline Scalar& operator[](GLsizei index) // Ditto
		{
		return xyzw[index];
		}
	};

/***************************************
Dummy generic version of GLVector class:
***************************************/

template <class ScalarParam =GLfloat,GLsizei numComponentsParam =4>
class GLVector
	{
	};

/**************************************
Specialized versions of GLVector class:
**************************************/

template <class ScalarParam>
class GLVector<ScalarParam,1>:public GLVectorBase<ScalarParam,1>
	{
	/* Embedded classes: */
	public:
	typedef GLVectorBase<ScalarParam,1> BaseClass;
	
	/* Declarations of inherited types/elements: */
	using GLVectorBase<ScalarParam,1>::xyzw;
	
	/* Constructors and destructors: */
	inline GLVector(void)
		{
		}
	inline GLVector(typename BaseClass::Scalar sX)
		{
		xyzw[0]=sX;
		}
	template <class SourceScalarParam>
	inline GLVector(const SourceScalarParam sXyzw[1])
		:BaseClass(sXyzw)
		{
		}
	template <class SourceScalarParam,GLsizei sourceNumComponentsParam>
	inline GLVector(const GLVector<SourceScalarParam,sourceNumComponentsParam>& source)
		:BaseClass(source)
		{
		}
	template <class SourceScalarParam>
	inline GLVector& operator=(const SourceScalarParam sXyzw[1])
		{
		BaseClass::operator=(sXyzw);
		return *this;
		}
	template <class SourceScalarParam,GLsizei sourceNumComponentsParam>
	inline GLVector& operator=(const GLVector<SourceScalarParam,sourceNumComponentsParam>& source)
		{
		BaseClass::operator=(source);
		return *this;
		}
	};

template <class ScalarParam>
class GLVector<ScalarParam,2>:public GLVectorBase<ScalarParam,2>
	{
	/* Embedded classes: */
	public:
	typedef GLVectorBase<ScalarParam,2> BaseClass;
	
	/* Declarations of inherited types/elements: */
	using GLVectorBase<ScalarParam,2>::xyzw;
	
	/* Constructors and destructors: */
	inline GLVector(void)
		{
		}
	inline GLVector(typename BaseClass::Scalar sX,typename BaseClass::Scalar sY)
		{
		xyzw[0]=sX;
		xyzw[1]=sY;
		}
	template <class SourceScalarParam>
	inline GLVector(const SourceScalarParam sXyzw[2])
		:BaseClass(sXyzw)
		{
		}
	template <class SourceScalarParam,GLsizei sourceNumComponentsParam>
	inline GLVector(const GLVector<SourceScalarParam,sourceNumComponentsParam>& source)
		:BaseClass(source)
		{
		}
	template <class SourceScalarParam>
	inline GLVector& operator=(const SourceScalarParam sXyzw[2])
		{
		BaseClass::operator=(sXyzw);
		return *this;
		}
	template <class SourceScalarParam,GLsizei sourceNumComponentsParam>
	inline GLVector& operator=(const GLVector<SourceScalarParam,sourceNumComponentsParam>& source)
		{
		BaseClass::operator=(source);
		return *this;
		}
	};

template <class ScalarParam>
class GLVector<ScalarParam,3>:public GLVectorBase<ScalarParam,3>
	{
	/* Embedded classes: */
	public:
	typedef GLVectorBase<ScalarParam,3> BaseClass;
	
	/* Declarations of inherited types/elements: */
	using GLVectorBase<ScalarParam,3>::xyzw;
	
	/* Constructors and destructors: */
	inline GLVector(void)
		{
		}
	inline GLVector(typename BaseClass::Scalar sX,typename BaseClass::Scalar sY,typename BaseClass::Scalar sZ)
		{
		xyzw[0]=sX;
		xyzw[1]=sY;
		xyzw[2]=sZ;
		}
	template <class SourceScalarParam>
	inline GLVector(const SourceScalarParam sXyzw[3])
		:BaseClass(sXyzw)
		{
		}
	template <class SourceScalarParam,GLsizei sourceNumComponentsParam>
	inline GLVector(const GLVector<SourceScalarParam,sourceNumComponentsParam>& source)
		:BaseClass(source)
		{
		}
	template <class SourceScalarParam>
	inline GLVector& operator=(const SourceScalarParam sXyzw[3])
		{
		BaseClass::operator=(sXyzw);
		return *this;
		}
	template <class SourceScalarParam,GLsizei sourceNumComponentsParam>
	inline GLVector& operator=(const GLVector<SourceScalarParam,sourceNumComponentsParam>& source)
		{
		BaseClass::operator=(source);
		return *this;
		}
	};

template <class ScalarParam>
class GLVector<ScalarParam,4>:public GLVectorBase<ScalarParam,4>
	{
	/* Embedded classes: */
	public:
	typedef GLVectorBase<ScalarParam,4> BaseClass;
	
	/* Declarations of inherited types/elements: */
	using GLVectorBase<ScalarParam,4>::xyzw;
	
	/* Constructors and destructors: */
	inline GLVector(void)
		{
		}
	inline GLVector(typename BaseClass::Scalar sX,typename BaseClass::Scalar sY,typename BaseClass::Scalar sZ,typename BaseClass::Scalar sW)
		{
		xyzw[0]=sX;
		xyzw[1]=sY;
		xyzw[2]=sZ;
		xyzw[3]=sW;
		}
	template <class SourceScalarParam>
	inline GLVector(const SourceScalarParam sXyzw[4])
		:BaseClass(sXyzw)
		{
		}
	template <class SourceScalarParam,GLsizei sourceNumComponentsParam>
	inline GLVector(const GLVector<SourceScalarParam,sourceNumComponentsParam>& source)
		:BaseClass(source)
		{
		}
	template <class SourceScalarParam>
	inline GLVector& operator=(const SourceScalarParam sXyzw[4])
		{
		BaseClass::operator=(sXyzw);
		return *this;
		}
	template <class SourceScalarParam,GLsizei sourceNumComponentsParam>
	inline GLVector& operator=(const GLVector<SourceScalarParam,sourceNumComponentsParam>& source)
		{
		BaseClass::operator=(source);
		return *this;
		}
	};

/* Comparison operators: */

template <class ScalarParam,GLsizei numComponentsParam>
inline bool operator==(const GLVector<ScalarParam,numComponentsParam>& c1,const GLVector<ScalarParam,numComponentsParam>& c2)
	{
	bool result=true;
	for(GLsizei i=0;i<numComponentsParam&&result;++i)
		result=c1[i]==c2[i];
	return result;
	}

template <class ScalarParam,GLsizei numComponentsParam>
inline bool operator!=(const GLVector<ScalarParam,numComponentsParam>& c1,const GLVector<ScalarParam,numComponentsParam>& c2)
	{
	bool result=false;
	for(GLsizei i=0;i<numComponentsParam&&!result;++i)
		result=c1[i]!=c2[i];
	return result;
	}

#endif
