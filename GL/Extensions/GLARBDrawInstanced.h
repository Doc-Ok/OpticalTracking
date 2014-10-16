/***********************************************************************
GLARBDrawInstanced - OpenGL extension class for the
GL_ARB_draw_instanced extension.
Copyright (c) 2010 Rolf Westerteiger
Fixes copyright (c) 2010-2014 Oliver Kreylos

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

#ifndef GLEXTENSIONS_GLARBDRAWINSTANCED_INCLUDED
#define GLEXTENSIONS_GLARBDRAWINSTANCED_INCLUDED

#include <GL/gl.h>
#include <GL/TLSHelper.h>
#include <GL/Extensions/GLExtension.h>

/********************************
Extension-specific parts of gl.h:
********************************/

#ifndef GL_ARB_draw_instanced
#define GL_ARB_draw_instanced 1

/* Extension-specific functions: */
typedef void (APIENTRY * PFNGLDRAWARRAYSINSTANCEDARBPROC) (GLenum, GLint, GLsizei, GLsizei);
typedef void (APIENTRY * PFNGLDRAWELEMENTSINSTANCEDARBPROC) (GLenum, GLsizei, GLenum, const GLvoid*, GLsizei);

#endif

/* Forward declarations of friend functions: */
void glDrawArraysInstancedARB(GLenum mode,GLint first,GLsizei count,GLsizei primcount);
void glDrawElementsInstancedARB(GLenum mode,GLsizei count,GLenum type,const GLvoid* indices,GLsizei primcount);

class GLARBDrawInstanced:public GLExtension
	{
	/* Elements: */
	private:
	static GL_THREAD_LOCAL(GLARBDrawInstanced*) current; // Pointer to extension object for current OpenGL context
	static const char* name; // Extension name
	PFNGLDRAWARRAYSINSTANCEDARBPROC glDrawArraysInstancedARBProc;
	PFNGLDRAWELEMENTSINSTANCEDARBPROC glDrawElementsInstancedARBProc;
	
	/* Constructors and destructors: */
	private:
	GLARBDrawInstanced(void);
	public:
	virtual ~GLARBDrawInstanced(void);
	
	/* Methods: */
	public:
	virtual const char* getExtensionName(void) const;
	virtual void activate(void);
	virtual void deactivate(void);
	static bool isSupported(void); // Returns true if the extension is supported in the current OpenGL context
	static void initExtension(void); // Initializes the extension in the current OpenGL context
	
	/* Extension entry points: */
	inline friend void glDrawArraysInstancedARB(GLenum mode,GLint first,GLsizei count,GLsizei primcount)
		{
		GLARBDrawInstanced::current->glDrawArraysInstancedARBProc(mode,first,count,primcount);
		}
	inline friend void glDrawElementsInstancedARB(GLenum mode,GLsizei count,GLenum type,const GLvoid* indices,GLsizei primcount)
		{
		GLARBDrawInstanced::current->glDrawElementsInstancedARBProc(mode,count,type,indices,primcount);
		}
	};

/*******************************
Extension-specific entry points:
*******************************/

#endif
