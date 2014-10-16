/***********************************************************************
GLNVPointSprite - OpenGL extension class for the GL_NV_point_sprite
extension.
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

#ifndef GLEXTENSIONS_GLNPOINTSPRITE_INCLUDED
#define GLEXTENSIONS_GLNPOINTSPRITE_INCLUDED

#include <GL/gl.h>
#include <GL/TLSHelper.h>
#include <GL/Extensions/GLExtension.h>

/********************************
Extension-specific parts of gl.h:
********************************/

#ifndef GL_NV_point_sprite
#define GL_NV_point_sprite 1

/* Extension-specific functions: */
typedef void (APIENTRY * PFNGLPOINTPARAMETERINVPROC) (GLenum pname, GLint param);
typedef void (APIENTRY * PFNGLPOINTPARAMETERIVNVPROC) (GLenum pname, const GLint *params);

/* Extension-specific constants: */
#define GL_POINT_SPRITE_NV                0x8861
#define GL_COORD_REPLACE_NV               0x8862
#define GL_POINT_SPRITE_R_MODE_NV         0x8863

#endif

/* Forward declarations of friend functions: */
void glPointParameteriNV(GLenum pname,GLint param);
void glPointParameterivNV(GLenum pname,const GLint* params);

class GLNVPointSprite:public GLExtension
	{
	/* Elements: */
	private:
	static GL_THREAD_LOCAL(GLNVPointSprite*) current; // Pointer to extension object for current OpenGL context
	static const char* name; // Extension name
	PFNGLPOINTPARAMETERINVPROC glPointParameteriNVProc;
	PFNGLPOINTPARAMETERIVNVPROC glPointParameterivNVProc;
	
	/* Constructors and destructors: */
	private:
	GLNVPointSprite(void);
	public:
	virtual ~GLNVPointSprite(void);
	
	/* Methods: */
	public:
	virtual const char* getExtensionName(void) const;
	virtual void activate(void);
	virtual void deactivate(void);
	static bool isSupported(void); // Returns true if the extension is supported in the current OpenGL context
	static void initExtension(void); // Initializes the extension in the current OpenGL context
	
	/* Extension entry points: */
	inline friend void glPointParameteriNV(GLenum pname,GLint param)
		{
		GLNVPointSprite::current->glPointParameteriNVProc(pname,param);
		}
	inline friend void glPointParameterivNV(GLenum pname,const GLint* params)
		{
		GLNVPointSprite::current->glPointParameterivNVProc(pname,params);
		}
	};

/*******************************
Extension-specific entry points:
*******************************/

#endif
