/***********************************************************************
GLObject - Base class for objects that store OpenGL context-specific
data.
Copyright (c) 2006-2013 Oliver Kreylos

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

#include <GL/GLContextData.h>

#include <GL/GLObject.h>

/*************************
Methods of class GLObject:
*************************/

void GLObject::dependsOn(const GLObject* thing) const
	{
	/* Ask the thing manager to initialize the other object before this one: */
	GLContextData::orderThings(thing,this);
	}

void GLObject::init(void)
	{
	/* Mark the object for context initialization: */
	GLContextData::initThing(this);
	}

GLObject::GLObject(bool autoInit)
	{
	if(autoInit)
		{
		/* Mark the object for context initialization: */
		GLContextData::initThing(this);
		}
	}

GLObject::~GLObject(void)
	{
	/* Mark the object's context data item for destruction: */
	GLContextData::destroyThing(this);
	}
