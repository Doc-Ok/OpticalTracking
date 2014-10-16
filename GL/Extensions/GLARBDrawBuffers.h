/***********************************************************************
GLARBDrawBuffers - OpenGL extension class for the GL_ARB_draw_buffers
extension.
Copyright (c) 2012-2014 Oliver Kreylos

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

#ifndef GLEXTENSIONS_GLARBDRAWBUFFERS_INCLUDED
#define GLEXTENSIONS_GLARBDRAWBUFFERS_INCLUDED

#include <GL/gl.h>
#include <GL/TLSHelper.h>
#include <GL/Extensions/GLExtension.h>

/********************************
Extension-specific parts of gl.h:
********************************/

#ifndef GL_ARB_draw_buffers
#define GL_ARB_draw_buffers 1

/* Extension-specific functions: */
typedef void (APIENTRY * PFNGLDRAWBUFFERSARBPROC) (GLsizei n, const GLenum *bufs);

/* Extension-specific constants: */
#define GL_MAX_DRAW_BUFFERS_ARB           0x8824
#define GL_DRAW_BUFFER0_ARB               0x8825
#define GL_DRAW_BUFFER1_ARB               0x8826
#define GL_DRAW_BUFFER2_ARB               0x8827
#define GL_DRAW_BUFFER3_ARB               0x8828
#define GL_DRAW_BUFFER4_ARB               0x8829
#define GL_DRAW_BUFFER5_ARB               0x882A
#define GL_DRAW_BUFFER6_ARB               0x882B
#define GL_DRAW_BUFFER7_ARB               0x882C
#define GL_DRAW_BUFFER8_ARB               0x882D
#define GL_DRAW_BUFFER9_ARB               0x882E
#define GL_DRAW_BUFFER10_ARB              0x882F
#define GL_DRAW_BUFFER11_ARB              0x8830
#define GL_DRAW_BUFFER12_ARB              0x8831
#define GL_DRAW_BUFFER13_ARB              0x8832
#define GL_DRAW_BUFFER14_ARB              0x8833
#define GL_DRAW_BUFFER15_ARB              0x8834

#endif

/* Forward declarations of friend functions: */
void glDrawBuffersARB(GLsizei n,const GLenum* bufs);

class GLARBDrawBuffers:public GLExtension
	{
	/* Elements: */
	private:
	static GL_THREAD_LOCAL(GLARBDrawBuffers*) current; // Pointer to extension object for current OpenGL context
	static const char* name; // Extension name
	PFNGLDRAWBUFFERSARBPROC glDrawBuffersARBProc;
	
	/* Constructors and destructors: */
	private:
	GLARBDrawBuffers(void);
	public:
	virtual ~GLARBDrawBuffers(void);
	
	/* Methods: */
	public:
	virtual const char* getExtensionName(void) const;
	virtual void activate(void);
	virtual void deactivate(void);
	static bool isSupported(void); // Returns true if the extension is supported in the current OpenGL context
	static void initExtension(void); // Initializes the extension in the current OpenGL context
	
	/* Extension entry points: */
	inline friend void glDrawBuffersARB(GLsizei n,const GLenum* bufs)
		{
		GLARBDrawBuffers::current->glDrawBuffersARBProc(n,bufs);
		}
	};

/*******************************
Extension-specific entry points:
*******************************/

#endif
