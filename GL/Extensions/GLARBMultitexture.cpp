/***********************************************************************
GLARBMultitexture - OpenGL extension class for the
GL_ARB_multitexture extension.
Copyright (c) 2006-2014 Oliver Kreylos

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

#include <GL/Extensions/GLARBMultitexture.h>

#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>

/******************************************
Static elements of class GLARBMultitexture:
******************************************/

GL_THREAD_LOCAL(GLARBMultitexture*) GLARBMultitexture::current=0;
const char* GLARBMultitexture::name="GL_ARB_multitexture";

/**********************************
Methods of class GLARBMultitexture:
**********************************/

GLARBMultitexture::GLARBMultitexture(void)
	{
	/* Get all function pointers: */
	glActiveTextureARBProc=GLExtensionManager::getFunction<PFNGLACTIVETEXTUREARBPROC>("glActiveTextureARB");
	glClientActiveTextureARBProc=GLExtensionManager::getFunction<PFNGLCLIENTACTIVETEXTUREARBPROC>("glClientActiveTextureARB");
	glMultiTexCoord1dARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD1DARBPROC>("glMultiTexCoord1dARB");
	glMultiTexCoord1dvARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD1DVARBPROC>("glMultiTexCoord1dvARB");
	glMultiTexCoord1fARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD1FARBPROC>("glMultiTexCoord1fARB");
	glMultiTexCoord1fvARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD1FVARBPROC>("glMultiTexCoord1fvARB");
	glMultiTexCoord1iARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD1IARBPROC>("glMultiTexCoord1iARB");
	glMultiTexCoord1ivARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD1IVARBPROC>("glMultiTexCoord1ivARB");
	glMultiTexCoord1sARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD1SARBPROC>("glMultiTexCoord1sARB");
	glMultiTexCoord1svARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD1SVARBPROC>("glMultiTexCoord1svARB");
	glMultiTexCoord2dARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD2DARBPROC>("glMultiTexCoord2dARB");
	glMultiTexCoord2dvARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD2DVARBPROC>("glMultiTexCoord2dvARB");
	glMultiTexCoord2fARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD2FARBPROC>("glMultiTexCoord2fARB");
	glMultiTexCoord2fvARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD2FVARBPROC>("glMultiTexCoord2fvARB");
	glMultiTexCoord2iARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD2IARBPROC>("glMultiTexCoord2iARB");
	glMultiTexCoord2ivARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD2IVARBPROC>("glMultiTexCoord2ivARB");
	glMultiTexCoord2sARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD2SARBPROC>("glMultiTexCoord2sARB");
	glMultiTexCoord2svARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD2SVARBPROC>("glMultiTexCoord2svARB");
	glMultiTexCoord3dARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD3DARBPROC>("glMultiTexCoord3dARB");
	glMultiTexCoord3dvARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD3DVARBPROC>("glMultiTexCoord3dvARB");
	glMultiTexCoord3fARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD3FARBPROC>("glMultiTexCoord3fARB");
	glMultiTexCoord3fvARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD3FVARBPROC>("glMultiTexCoord3fvARB");
	glMultiTexCoord3iARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD3IARBPROC>("glMultiTexCoord3iARB");
	glMultiTexCoord3ivARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD3IVARBPROC>("glMultiTexCoord3ivARB");
	glMultiTexCoord3sARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD3SARBPROC>("glMultiTexCoord3sARB");
	glMultiTexCoord3svARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD3SVARBPROC>("glMultiTexCoord3svARB");
	glMultiTexCoord4dARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD4DARBPROC>("glMultiTexCoord4dARB");
	glMultiTexCoord4dvARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD4DVARBPROC>("glMultiTexCoord4dvARB");
	glMultiTexCoord4fARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD4FARBPROC>("glMultiTexCoord4fARB");
	glMultiTexCoord4fvARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD4FVARBPROC>("glMultiTexCoord4fvARB");
	glMultiTexCoord4iARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD4IARBPROC>("glMultiTexCoord4iARB");
	glMultiTexCoord4ivARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD4IVARBPROC>("glMultiTexCoord4ivARB");
	glMultiTexCoord4sARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD4SARBPROC>("glMultiTexCoord4sARB");
	glMultiTexCoord4svARBProc=GLExtensionManager::getFunction<PFNGLMULTITEXCOORD4SVARBPROC>("glMultiTexCoord4svARB");
	}

GLARBMultitexture::~GLARBMultitexture(void)
	{
	}

const char* GLARBMultitexture::getExtensionName(void) const
	{
	return name;
	}

void GLARBMultitexture::activate(void)
	{
	current=this;
	}

void GLARBMultitexture::deactivate(void)
	{
	current=0;
	}

bool GLARBMultitexture::isSupported(void)
	{
	/* Ask the current extension manager whether the extension is supported in the current OpenGL context: */
	return GLExtensionManager::isExtensionSupported(name);
	}

void GLARBMultitexture::initExtension(void)
	{
	/* Check if the extension is already initialized: */
	if(!GLExtensionManager::isExtensionRegistered(name))
		{
		/* Create a new extension object: */
		GLARBMultitexture* newExtension=new GLARBMultitexture;
		
		/* Register the extension with the current extension manager: */
		GLExtensionManager::registerExtension(newExtension);
		}
	}
