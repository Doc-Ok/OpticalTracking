/***********************************************************************
GLEXTTextureCompressionS3TC - OpenGL extension class for the
GL_EXT_texture_compression_s3tc extension.
Copyright (c) 2007-2014 Oliver Kreylos

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

#include <GL/Extensions/GLEXTTextureCompressionS3TC.h>

#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>

/****************************************************
Static elements of class GLEXTTextureCompressionS3TC:
****************************************************/

GL_THREAD_LOCAL(GLEXTTextureCompressionS3TC*) GLEXTTextureCompressionS3TC::current=0;
const char* GLEXTTextureCompressionS3TC::name="GL_EXT_texture_compression_s3tc";

/********************************************
Methods of class GLEXTTextureCompressionS3TC:
********************************************/

GLEXTTextureCompressionS3TC::GLEXTTextureCompressionS3TC(void)
	{
	}

GLEXTTextureCompressionS3TC::~GLEXTTextureCompressionS3TC(void)
	{
	}

const char* GLEXTTextureCompressionS3TC::getExtensionName(void) const
	{
	return name;
	}

void GLEXTTextureCompressionS3TC::activate(void)
	{
	current=this;
	}

void GLEXTTextureCompressionS3TC::deactivate(void)
	{
	current=0;
	}

bool GLEXTTextureCompressionS3TC::isSupported(void)
	{
	/* Ask the current extension manager whether the extension is supported in the current OpenGL context: */
	return GLExtensionManager::isExtensionSupported(name);
	}

void GLEXTTextureCompressionS3TC::initExtension(void)
	{
	/* Check if the extension is already initialized: */
	if(!GLExtensionManager::isExtensionRegistered(name))
		{
		/* Create a new extension object: */
		GLEXTTextureCompressionS3TC* newExtension=new GLEXTTextureCompressionS3TC;
		
		/* Register the extension with the current extension manager: */
		GLExtensionManager::registerExtension(newExtension);
		}
	}
