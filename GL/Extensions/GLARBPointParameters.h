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

#ifndef GLEXTENSIONS_GLARBPOINTPARAMETERS_INCLUDED
#define GLEXTENSIONS_GLARBPOINTPARAMETERS_INCLUDED

#define GLEXTENSIONS_GLARBPOINTPARAMETERS_EXPORT_INTPROCS 0

#include <GL/gl.h>
#include <GL/TLSHelper.h>
#include <GL/Extensions/GLExtension.h>

/********************************
Extension-specific parts of gl.h:
********************************/

#ifndef GL_ARB_point_parameters
#define GL_ARB_point_parameters 1

/* Extension-specific functions: */
#if GLEXTENSIONS_GLARBPOINTPARAMETERS_EXPORT_INTPROCS
typedef void (APIENTRY * PFNGLPOINTPARAMETERIARBPROC)(GLenum pname,GLint param);
typedef void (APIENTRY * PFNGLPOINTPARAMETERIVARBPROC)(GLenum pname,const GLint* param);
#endif
typedef void (APIENTRY * PFNGLPOINTPARAMETERFARBPROC)(GLenum pname,GLfloat param);
typedef void (APIENTRY * PFNGLPOINTPARAMETERFVARBPROC)(GLenum pname,const GLfloat* param);

/* Extension-specific constants: */
#define GL_POINT_SIZE_MIN_ARB                           0x8126
#define GL_POINT_SIZE_MAX_ARB                           0x8127
#define GL_POINT_FADE_THRESHOLD_SIZE_ARB                0x8128
#define GL_POINT_DISTANCE_ATTENUATION_ARB               0x8129

#endif

/* Forward declarations of friend functions: */
#if GLEXTENSIONS_GLARBPOINTPARAMETERS_EXPORT_INTPROCS
void glPointParameteriARB(GLenum pname,GLint param);
void glPointParameterivARB(GLenum pname,const GLint* params);
#endif
void glPointParameterfARB(GLenum pname,GLfloat param);
void glPointParameterfvARB(GLenum pname,const GLfloat* params);

class GLARBPointParameters:public GLExtension
	{
	/* Elements: */
	private:
	static GL_THREAD_LOCAL(GLARBPointParameters*) current; // Pointer to extension object for current OpenGL context
	static const char* name; // Extension name
	#if GLEXTENSIONS_GLARBPOINTPARAMETERS_EXPORT_INTPROCS
	PFNGLPOINTPARAMETERIARBPROC glPointParameteriARBProc;
	PFNGLPOINTPARAMETERIVARBPROC glPointParameterivARBProc;
	#endif
	PFNGLPOINTPARAMETERFARBPROC glPointParameterfARBProc;
	PFNGLPOINTPARAMETERFVARBPROC glPointParameterfvARBProc;
	
	/* Constructors and destructors: */
	private:
	GLARBPointParameters(void);
	public:
	virtual ~GLARBPointParameters(void);
	
	/* Methods: */
	public:
	virtual const char* getExtensionName(void) const;
	virtual void activate(void);
	virtual void deactivate(void);
	static bool isSupported(void); // Returns true if the extension is supported in the current OpenGL context
	static void initExtension(void); // Initializes the extension in the current OpenGL context
	
	/* Extension entry points: */
	#if GLEXTENSIONS_GLARBPOINTPARAMETERS_EXPORT_INTPROCS
	inline friend void glPointParameteriARB(GLenum pname,GLint param)
		{
		GLARBPointParameters::current->glPointParameteriARBProc(pname,param);
		}
	inline friend void glPointParameterivARB(GLenum pname,const GLint* params)
		{
		GLARBPointParameters::current->glPointParameterivARBProc(pname,params);
		}
	#endif
	inline friend void glPointParameterfARB(GLenum pname,GLfloat param)
		{
		GLARBPointParameters::current->glPointParameterfARBProc(pname,param);
		}
	inline friend void glPointParameterfvARB(GLenum pname,const GLfloat* params)
		{
		GLARBPointParameters::current->glPointParameterfvARBProc(pname,params);
		}
	};

/*******************************
Extension-specific entry points:
*******************************/

#endif
