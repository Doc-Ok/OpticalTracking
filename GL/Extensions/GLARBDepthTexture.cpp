/***********************************************************************
GLARBDepthTexture - OpenGL extension class for the GL_ARB_depth_texture
extension.
Copyright (c) 2008-2014 Oliver Kreylos

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

#include <GL/Extensions/GLARBDepthTexture.h>

#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>

/******************************************
Static elements of class GLARBDepthTexture:
******************************************/

GL_THREAD_LOCAL(GLARBDepthTexture*) GLARBDepthTexture::current=0;
const char* GLARBDepthTexture::name="GL_ARB_depth_texture";

/**********************************
Methods of class GLARBDepthTexture:
**********************************/

GLARBDepthTexture::GLARBDepthTexture(void)
	{
	}

GLARBDepthTexture::~GLARBDepthTexture(void)
	{
	}

const char* GLARBDepthTexture::getExtensionName(void) const
	{
	return name;
	}

void GLARBDepthTexture::activate(void)
	{
	current=this;
	}

void GLARBDepthTexture::deactivate(void)
	{
	current=0;
	}

bool GLARBDepthTexture::isSupported(void)
	{
	/* Ask the current extension manager whether the extension is supported in the current OpenGL context: */
	return GLExtensionManager::isExtensionSupported(name);
	}

void GLARBDepthTexture::initExtension(void)
	{
	/* Check if the extension is already initialized: */
	if(!GLExtensionManager::isExtensionRegistered(name))
		{
		/* Create a new extension object: */
		GLARBDepthTexture* newExtension=new GLARBDepthTexture;
		
		/* Register the extension with the current extension manager: */
		GLExtensionManager::registerExtension(newExtension);
		}
	}
