/***********************************************************************
GLARBPointParameters - OpenGL extension class for the
GL_ARB_point_parameters extension.
Copyright (c) 2005-2014 Oliver Kreylos

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

#include <GL/Extensions/GLARBPointParameters.h>

#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>

/*********************************************
Static elements of class GLARBPointParameters:
*********************************************/

GL_THREAD_LOCAL(GLARBPointParameters*) GLARBPointParameters::current=0;
const char* GLARBPointParameters::name="GL_ARB_point_parameters";

/*************************************
Methods of class GLARBPointParameters:
*************************************/

GLARBPointParameters::GLARBPointParameters(void)
	{
	/* Get all function pointers: */
	#if GLEXTENSIONS_GLARBPOINTPARAMETERS_EXPORT_INTPROCS
	glPointParameteriARBProc=GLExtensionManager::getFunction<PFNGLPOINTPARAMETERIARBPROC>("glPointParameteriARB");
	glPointParameterivARBProc=GLExtensionManager::getFunction<PFNGLPOINTPARAMETERIVARBPROC>("glPointParameterivARB");
	#endif
	glPointParameterfARBProc=GLExtensionManager::getFunction<PFNGLPOINTPARAMETERFARBPROC>("glPointParameterfARB");
	glPointParameterfvARBProc=GLExtensionManager::getFunction<PFNGLPOINTPARAMETERFVARBPROC>("glPointParameterfvARB");
	}

GLARBPointParameters::~GLARBPointParameters(void)
	{
	}

const char* GLARBPointParameters::getExtensionName(void) const
	{
	return name;
	}

void GLARBPointParameters::activate(void)
	{
	current=this;
	}

void GLARBPointParameters::deactivate(void)
	{
	current=0;
	}

bool GLARBPointParameters::isSupported(void)
	{
	/* Ask the current extension manager whether the extension is supported in the current OpenGL context: */
	return GLExtensionManager::isExtensionSupported(name);
	}

void GLARBPointParameters::initExtension(void)
	{
	/* Check if the extension is already initialized: */
	if(!GLExtensionManager::isExtensionRegistered(name))
		{
		/* Create a new extension object: */
		GLARBPointParameters* newExtension=new GLARBPointParameters;
		
		/* Register the extension with the current extension manager: */
		GLExtensionManager::registerExtension(newExtension);
		}
	}
