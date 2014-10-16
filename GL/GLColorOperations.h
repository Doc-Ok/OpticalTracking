/***********************************************************************
GLColorOperations - Operations on objects of type GLColor.
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

#ifndef GLCOLOROPERATIONS_INCLUDED
#define GLCOLOROPERATIONS_INCLUDED

#include <GL/gl.h>
#include <GL/GLColor.h>

template <class ScalarParam,GLsizei numComponentsParam>
GLColor<ScalarParam,numComponentsParam>& operator+=(GLColor<ScalarParam,numComponentsParam>& col1,const GLColor<ScalarParam,numComponentsParam>& col2)
	{
	for(GLsizei i=0;i<numComponentsParam;++i)
		col1[i]+=col2[i];
	return col1;
	}

template <class ScalarParam,GLsizei numComponentsParam>
GLColor<ScalarParam,numComponentsParam> operator+(const GLColor<ScalarParam,numComponentsParam>& col1,const GLColor<ScalarParam,numComponentsParam>& col2)
	{
	GLColor<ScalarParam,numComponentsParam> result;
	for(GLsizei i=0;i<numComponentsParam;++i)
		result[i]=col1[i]+col2[i];
	return result;
	}

template <class ScalarParam,GLsizei numComponentsParam>
GLColor<ScalarParam,numComponentsParam>& operator-=(GLColor<ScalarParam,numComponentsParam>& col1,const GLColor<ScalarParam,numComponentsParam>& col2)
	{
	for(GLsizei i=0;i<numComponentsParam;++i)
		col1[i]-=col2[i];
	return col1;
	}

template <class ScalarParam,GLsizei numComponentsParam>
GLColor<ScalarParam,numComponentsParam> operator-(const GLColor<ScalarParam,numComponentsParam>& col1,const GLColor<ScalarParam,numComponentsParam>& col2)
	{
	GLColor<ScalarParam,numComponentsParam> result;
	for(GLsizei i=0;i<numComponentsParam;++i)
		result[i]=col1[i]-col2[i];
	return result;
	}

template <class ScalarParam,GLsizei numComponentsParam>
GLColor<ScalarParam,numComponentsParam>& operator*=(GLColor<ScalarParam,numComponentsParam>& col,ScalarParam factor)
	{
	for(GLsizei i=0;i<numComponentsParam;++i)
		col[i]*=factor;
	return col;
	}

template <class ScalarParam,GLsizei numComponentsParam>
GLColor<ScalarParam,numComponentsParam> operator*(const GLColor<ScalarParam,numComponentsParam>& col,ScalarParam factor)
	{
	GLColor<ScalarParam,numComponentsParam> result;
	for(GLsizei i=0;i<numComponentsParam;++i)
		result[i]=col[i]*factor;
	return result;
	}

template <class ScalarParam,GLsizei numComponentsParam>
GLColor<ScalarParam,numComponentsParam> operator*(ScalarParam factor,const GLColor<ScalarParam,numComponentsParam>& col)
	{
	GLColor<ScalarParam,numComponentsParam> result;
	for(GLsizei i=0;i<numComponentsParam;++i)
		result[i]=factor*col[i];
	return result;
	}

template <class ScalarParam,GLsizei numComponentsParam>
GLColor<ScalarParam,numComponentsParam>& operator*=(GLColor<ScalarParam,numComponentsParam>& col1,const GLColor<ScalarParam,numComponentsParam>& col2)
	{
	for(GLsizei i=0;i<numComponentsParam;++i)
		col1[i]*=col2[i];
	return col1;
	}

template <class ScalarParam,GLsizei numComponentsParam>
GLColor<ScalarParam,numComponentsParam> operator*(const GLColor<ScalarParam,numComponentsParam>& col1,const GLColor<ScalarParam,numComponentsParam>& col2)
	{
	GLColor<ScalarParam,numComponentsParam> result;
	for(GLsizei i=0;i<numComponentsParam;++i)
		result[i]=col1[i]*col2[i];
	return result;
	}

template <GLsizei numComponentsParam>
GLColor<GLfloat,numComponentsParam>& clamp(GLColor<GLfloat,numComponentsParam>& col)
	{
	for(GLsizei i=0;i<numComponentsParam;++i)
		{
		if(col[i]<0.0f)
			col[i]=0.0f;
		else if(col[i]>1.0f)
			col[i]=1.0f;
		}
	return col;
	}

template <GLsizei numComponentsParam>
GLColor<GLdouble,numComponentsParam>& clamp(GLColor<GLdouble,numComponentsParam>& col)
	{
	for(GLsizei i=0;i<numComponentsParam;++i)
		{
		if(col[i]<0.0)
			col[i]=0.0;
		else if(col[i]>1.0)
			col[i]=1.0;
		}
	return col;
	}

#endif
