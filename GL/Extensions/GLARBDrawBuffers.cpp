/***********************************************************************
GLARBDrawBuffers - OpenGL extension class for the GL_ARB_draw_buffers
extension.
Copyright (c) 2012-2014 Oliver Kreylos

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

#include <GL/Extensions/GLARBDrawBuffers.h>

#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>

/***************************************
Static elements of class GLARBDrawBuffers:
***************************************/

GL_THREAD_LOCAL(GLARBDrawBuffers*) GLARBDrawBuffers::current=0;
const char* GLARBDrawBuffers::name="GL_ARB_draw_buffers";

/*******************************
Methods of class GLARBDrawBuffers:
*******************************/

GLARBDrawBuffers::GLARBDrawBuffers(void)
	{
	/* Get all function pointers: */
	glDrawBuffersARBProc=GLExtensionManager::getFunction<PFNGLDRAWBUFFERSARBPROC>("glDrawBuffersARB");
	}

GLARBDrawBuffers::~GLARBDrawBuffers(void)
	{
	}

const char* GLARBDrawBuffers::getExtensionName(void) const
	{
	return name;
	}

void GLARBDrawBuffers::activate(void)
	{
	current=this;
	}

void GLARBDrawBuffers::deactivate(void)
	{
	current=0;
	}

bool GLARBDrawBuffers::isSupported(void)
	{
	/* Ask the current extension manager whether the extension is supported in the current OpenGL context: */
	return GLExtensionManager::isExtensionSupported(name);
	}

void GLARBDrawBuffers::initExtension(void)
	{
	/* Check if the extension is already initialized: */
	if(!GLExtensionManager::isExtensionRegistered(name))
		{
		/* Create a new extension object: */
		GLARBDrawBuffers* newExtension=new GLARBDrawBuffers;
		
		/* Register the extension with the current extension manager: */
		GLExtensionManager::registerExtension(newExtension);
		}
	}
