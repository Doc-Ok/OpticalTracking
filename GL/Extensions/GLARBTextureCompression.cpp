/***********************************************************************
GLARBTextureCompression - OpenGL extension class for the
GL_ARB_texture_compression extension.
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

#include <GL/Extensions/GLARBTextureCompression.h>

#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>

/************************************************
Static elements of class GLARBTextureCompression:
************************************************/

GL_THREAD_LOCAL(GLARBTextureCompression*) GLARBTextureCompression::current=0;
const char* GLARBTextureCompression::name="GL_ARB_texture_compression";

/****************************************
Methods of class GLARBTextureCompression:
****************************************/

GLARBTextureCompression::GLARBTextureCompression(void)
	{
	/* Get all function pointers: */
	glCompressedTexImage3DARBProc=GLExtensionManager::getFunction<PFNGLCOMPRESSEDTEXIMAGE3DARBPROC>("glCompressedTexImage3DARB");
	glCompressedTexImage2DARBProc=GLExtensionManager::getFunction<PFNGLCOMPRESSEDTEXIMAGE2DARBPROC>("glCompressedTexImage2DARB");
	glCompressedTexImage1DARBProc=GLExtensionManager::getFunction<PFNGLCOMPRESSEDTEXIMAGE1DARBPROC>("glCompressedTexImage1DARB");
	glCompressedTexSubImage3DARBProc=GLExtensionManager::getFunction<PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC>("glCompressedTexSubImage3DARB");
	glCompressedTexSubImage2DARBProc=GLExtensionManager::getFunction<PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC>("glCompressedTexSubImage2DARB");
	glCompressedTexSubImage1DARBProc=GLExtensionManager::getFunction<PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC>("glCompressedTexSubImage1DARB");
	glGetCompressedTexImageARBProc=GLExtensionManager::getFunction<PFNGLGETCOMPRESSEDTEXIMAGEARBPROC>("glGetCompressedTexImageARB");
	}

GLARBTextureCompression::~GLARBTextureCompression(void)
	{
	}

const char* GLARBTextureCompression::getExtensionName(void) const
	{
	return name;
	}

void GLARBTextureCompression::activate(void)
	{
	current=this;
	}

void GLARBTextureCompression::deactivate(void)
	{
	current=0;
	}

bool GLARBTextureCompression::isSupported(void)
	{
	/* Ask the current extension manager whether the extension is supported in the current OpenGL context: */
	return GLExtensionManager::isExtensionSupported(name);
	}

void GLARBTextureCompression::initExtension(void)
	{
	/* Check if the extension is already initialized: */
	if(!GLExtensionManager::isExtensionRegistered(name))
		{
		/* Create a new extension object: */
		GLARBTextureCompression* newExtension=new GLARBTextureCompression;
		
		/* Register the extension with the current extension manager: */
		GLExtensionManager::registerExtension(newExtension);
		}
	}
