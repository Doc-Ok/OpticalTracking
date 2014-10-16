/***********************************************************************
GLARBTextureMultisample - OpenGL extension class for the
GL_ARB_texture_multisample extension.
Note: The functions and constants exported by this extension do *not*
use the ARB suffix.
Copyright (c) 2014 Oliver Kreylos

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

#include <GL/Extensions/GLARBTextureMultisample.h>

#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>

/************************************************
Static elements of class GLARBTextureMultisample:
************************************************/

GL_THREAD_LOCAL(GLARBTextureMultisample*) GLARBTextureMultisample::current=0;
const char* GLARBTextureMultisample::name="GL_ARB_texture_multisample";

/****************************************
Methods of class GLARBTextureMultisample:
****************************************/

GLARBTextureMultisample::GLARBTextureMultisample(void)
	:glTexImage2DMultisampleProc(GLExtensionManager::getFunction<PFNGLTEXIMAGE2DMULTISAMPLEPROC>("glTexImage2DMultisample")),
	 glTexImage3DMultisampleProc(GLExtensionManager::getFunction<PFNGLTEXIMAGE3DMULTISAMPLEPROC>("glTexImage3DMultisample")),
	 glGetMultisamplefvProc(GLExtensionManager::getFunction<PFNGLGETMULTISAMPLEFVPROC>("glGetMultisamplefv")),
	 glSampleMaskiProc(GLExtensionManager::getFunction<PFNGLSAMPLEMASKIPROC>("glSampleMaski"))
	{
	}

GLARBTextureMultisample::~GLARBTextureMultisample(void)
	{
	}

const char* GLARBTextureMultisample::getExtensionName(void) const
	{
	return name;
	}

void GLARBTextureMultisample::activate(void)
	{
	current=this;
	}

void GLARBTextureMultisample::deactivate(void)
	{
	current=0;
	}

bool GLARBTextureMultisample::isSupported(void)
	{
	/* Ask the current extension manager whether the extension is supported in the current OpenGL context: */
	return GLExtensionManager::isExtensionSupported(name);
	}

void GLARBTextureMultisample::initExtension(void)
	{
	/* Check if the extension is already initialized: */
	if(!GLExtensionManager::isExtensionRegistered(name))
		{
		/* Create a new extension object: */
		GLARBTextureMultisample* newExtension=new GLARBTextureMultisample;
		
		/* Register the extension with the current extension manager: */
		GLExtensionManager::registerExtension(newExtension);
		}
	}
