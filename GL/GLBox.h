/***********************************************************************
GLBox - Class to represent axis-aligned rectangular boxes.
Copyright (c) 2005 Oliver Kreylos

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

#ifndef GLBOX_INCLUDED
#define GLBOX_INCLUDED

#include <GL/gl.h>
#include <GL/GLVector.h>

template <class ScalarParam,GLsizei numComponentsParam>
class GLBox
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Scalar type
	static const GLsizei numComponents=numComponentsParam; // Number of stored vector components
	typedef GLVector<ScalarParam,numComponentsParam> Vector; // Compatible vector type
	
	/* Elements: */
	Vector origin; // Origin of box
	Vector size; // Size of box
	
	/* Constructors and destructors: */
	GLBox(void) // Creates uninitialized box
		{
		}
	GLBox(const Vector& sOrigin,const Vector& sSize) // Creates box from given origin and size
		:origin(sOrigin),size(sSize)
		{
		}
	
	/* Methods: */
	GLBox offset(const Vector& offset) const // Returns a new box by moving the box by the given offset vector
		{
		GLBox result=*this;
		for(GLsizei i=0;i<numComponents;++i)
			result.origin[i]+=offset[i];
		return result;
		}
	GLBox& doOffset(const Vector& offset) // Moves the box by the given offset vector
		{
		for(GLsizei i=0;i<numComponents;++i)
			origin[i]+=offset[i];
		return *this;
		}
	GLBox outset(const Vector& sizeIncrement) const // Returns a new box by expanding it by the given increment in all directions
		{
		GLBox result=*this;
		for(GLsizei i=0;i<numComponents;++i)
			{
			result.origin[i]-=sizeIncrement[i];
			result.size[i]+=Scalar(2)*sizeIncrement[i];
			}
		return result;
		}
	GLBox& doOutset(const Vector& sizeIncrement) // Expands the box by the given increment in all directions
		{
		for(GLsizei i=0;i<numComponents;++i)
			{
			origin[i]-=sizeIncrement[i];
			size[i]+=Scalar(2)*sizeIncrement[i];
			}
		return *this;
		}
	GLBox inset(const Vector& sizeDecrement) const // Returns a new box by shrinking it by the given decrement in all directions
		{
		GLBox result=*this;
		for(GLsizei i=0;i<numComponents;++i)
			{
			result.origin[i]+=sizeDecrement[i];
			result.size[i]-=Scalar(2)*sizeDecrement[i];
			}
		return result;
		}
	GLBox& doInset(const Vector& sizeDecrement) // Shrinks the box by the given decrement in all directions
		{
		for(GLsizei i=0;i<numComponents;++i)
			{
			origin[i]+=sizeDecrement[i];
			size[i]-=Scalar(2)*sizeDecrement[i];
			}
		return *this;
		}
	Vector getCorner(GLsizei cornerIndex) const // Returns a corner of the box (in standard order)
		{
		Vector result=origin;
		for(GLsizei i=0;i<numComponents;++i,cornerIndex>>=1)
			if(cornerIndex&0x1)
				result[i]+=size[i];
		return result;
		}
	bool isInside(const Vector& p) const // Returns true if the given point is inside the box
		{
		for(GLsizei i=0;i<numComponents;++i)
			if(p[i]<origin[i]||p[i]>=origin[i]+size[i])
				return false;
		return true;
		}
	};

#endif
