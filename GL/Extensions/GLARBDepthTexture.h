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

#ifndef GLEXTENSIONS_GLARBDEPTHTEXTURE_INCLUDED
#define GLEXTENSIONS_GLARBDEPTHTEXTURE_INCLUDED

#include <GL/gl.h>
#include <GL/TLSHelper.h>
#include <GL/Extensions/GLExtension.h>

/********************************
Extension-specific parts of gl.h:
********************************/

#ifndef GL_ARB_depth_texture
#define GL_ARB_depth_texture 1

/* Extension-specific constants: */
#define GL_DEPTH_COMPONENT16_ARB          0x81A5
#define GL_DEPTH_COMPONENT24_ARB          0x81A6
#define GL_DEPTH_COMPONENT32_ARB          0x81A7
#define GL_TEXTURE_DEPTH_SIZE_ARB         0x884A
#define GL_DEPTH_TEXTURE_MODE_ARB         0x884B

#endif

class GLARBDepthTexture:public GLExtension
	{
	/* Elements: */
	private:
	static GL_THREAD_LOCAL(GLARBDepthTexture*) current; // Pointer to extension object for current OpenGL context
	static const char* name; // Extension name
	
	/* Constructors and destructors: */
	private:
	GLARBDepthTexture(void);
	public:
	virtual ~GLARBDepthTexture(void);
	
	/* Methods: */
	public:
	virtual const char* getExtensionName(void) const;
	virtual void activate(void);
	virtual void deactivate(void);
	static bool isSupported(void); // Returns true if the extension is supported in the current OpenGL context
	static void initExtension(void); // Initializes the extension in the current OpenGL context
	
	/* Extension entry points: */
	};

/*******************************
Extension-specific entry points:
*******************************/

#endif
