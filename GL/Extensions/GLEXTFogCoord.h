/***********************************************************************
GLEXTFogCoord - OpenGL extension class for the GL_EXT_fog_coord
extension.
Copyright (c) 2013-2014 Oliver Kreylos

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

#ifndef GLEXTENSIONS_GLEXTFOGCOORD_INCLUDED
#define GLEXTENSIONS_GLEXTFOGCOORD_INCLUDED

#include <GL/gl.h>
#include <GL/GLGetTemplates.h>
#include <GL/GLVector.h>
#include <GL/TLSHelper.h>
#include <GL/Extensions/GLExtension.h>

/********************************
Extension-specific parts of gl.h:
********************************/

#ifndef GL_EXT_fog_coord
#define GL_EXT_fog_coord 1

/* Extension-specific functions: */
typedef void (APIENTRYP PFNGLFOGCOORDFEXTPROC) (GLfloat coord);
typedef void (APIENTRYP PFNGLFOGCOORDFVEXTPROC) (const GLfloat *coord);
typedef void (APIENTRYP PFNGLFOGCOORDDEXTPROC) (GLdouble coord);
typedef void (APIENTRYP PFNGLFOGCOORDDVEXTPROC) (const GLdouble *coord);
typedef void (APIENTRYP PFNGLFOGCOORDPOINTEREXTPROC) (GLenum type, GLsizei stride, const GLvoid *pointer);

/* Extension-specific constants: */
#define GL_FOG_COORDINATE_SOURCE_EXT      0x8450
#define GL_FOG_COORDINATE_EXT             0x8451
#define GL_FRAGMENT_DEPTH_EXT             0x8452
#define GL_CURRENT_FOG_COORDINATE_EXT     0x8453
#define GL_FOG_COORDINATE_ARRAY_TYPE_EXT  0x8454
#define GL_FOG_COORDINATE_ARRAY_STRIDE_EXT 0x8455
#define GL_FOG_COORDINATE_ARRAY_POINTER_EXT 0x8456
#define GL_FOG_COORDINATE_ARRAY_EXT       0x8457

#endif

/* Forward declarations of friend functions: */
void glFogCoordfEXT(GLfloat coord);
void glFogCoorddEXT(GLdouble coord);
void glFogCoordfvEXT(const GLfloat* coord);
void glFogCoorddvEXT(const GLdouble* coord);
void glFogCoordPointerEXT(GLenum type,GLsizei stride,const GLvoid* pointer);

class GLEXTFogCoord:public GLExtension
	{
	/* Elements: */
	private:
	static GL_THREAD_LOCAL(GLEXTFogCoord*) current; // Pointer to extension object for current OpenGL context
	static const char* name; // Extension name
	PFNGLFOGCOORDFEXTPROC glFogCoordfEXTProc;
	PFNGLFOGCOORDDEXTPROC glFogCoorddEXTProc;
	PFNGLFOGCOORDFVEXTPROC glFogCoordfvEXTProc;
	PFNGLFOGCOORDDVEXTPROC glFogCoorddvEXTProc;
	PFNGLFOGCOORDPOINTEREXTPROC glFogCoordPointerEXTProc;
	
	/* Constructors and destructors: */
	private:
	GLEXTFogCoord(void);
	public:
	virtual ~GLEXTFogCoord(void);
	
	/* Methods: */
	public:
	virtual const char* getExtensionName(void) const;
	virtual void activate(void);
	virtual void deactivate(void);
	static bool isSupported(void); // Returns true if the extension is supported in the current OpenGL context
	static void initExtension(void); // Initializes the extension in the current OpenGL context
	
	/* Extension entry points: */
	inline friend void glFogCoordfEXT(GLfloat coord)
		{
		GLEXTFogCoord::current->glFogCoordfEXTProc(coord);
		}
	inline friend void glFogCoorddEXT(GLdouble coord)
		{
		GLEXTFogCoord::current->glFogCoorddEXTProc(coord);
		}
	inline friend void glFogCoordfvEXT(const GLfloat* coord)
		{
		GLEXTFogCoord::current->glFogCoordfvEXTProc(coord);
		}
	inline friend void glFogCoorddvEXT(const GLdouble* coord)
		{
		GLEXTFogCoord::current->glFogCoorddvEXTProc(coord);
		}
	inline friend void glFogCoordPointerEXT(GLenum type,GLsizei stride,const GLvoid* pointer)
		{
		GLEXTFogCoord::current->glFogCoordPointerEXTProc(type,stride,pointer);
		}
	};

/*******************************
Extension-specific entry points:
*******************************/

/**********************************************************
Overloaded versions of component-based glFogCoordEXT calls:
**********************************************************/

inline void glFogCoordEXT(GLfloat coord)
	{
	glFogCoordfEXT(coord);
	}

inline void glFogCoordEXT(GLdouble coord)
	{
	glFogCoorddEXT(coord);
	}

/*******************************************************
Dummy generic version of array-based glFogCoordEXT call:
*******************************************************/

template <GLsizei numComponents,class ScalarParam>
void glFogCoordEXT(const ScalarParam components[numComponents]);

/*******************************************************
Specialized versions of array-based glFogCoordEXT calls:
*******************************************************/

template <>
inline void glFogCoordEXT<1,GLfloat>(const GLfloat components[1])
	{
	glFogCoordfvEXT(components);
	}

template <>
inline void glFogCoordEXT<1,GLdouble>(const GLdouble components[1])
	{
	glFogCoorddvEXT(components);
	}

/*************************************************
Overloaded versions of glFogCoordPointerEXT calls:
*************************************************/

inline void glFogCoordPointerEXT(GLsizei stride,const GLfloat* pointer)
	{
	glFogCoordPointerEXT(GL_FLOAT,stride,pointer);
	}

inline void glFogCoordPointerEXT(GLsizei stride,const GLdouble* pointer)
	{
	glFogCoordPointerEXT(GL_DOUBLE,stride,pointer);
	}

template <class ScalarParam>
inline void glFogCoordPointerEXT(GLsizei stride,const GLVector<ScalarParam,1>* pointer)
	{
	glFogCoordPointerEXT(stride,pointer[0].getXyzw());
	}

/*************************************
Overloaded versions of glGet... calls:
*************************************/

template <class ScalarParam>
inline void glGetCurrentFogCoordEXT(ScalarParam params[1])
	{
	glGet(GL_CURRENT_FOG_COORDINATE_EXT,params);
	}

template <class ScalarParam>
inline GLVector<ScalarParam,1> glGetCurrentFogCoordEXT(void)
	{
	GLVector<ScalarParam,1> result;
	glGet(GL_CURRENT_FOG_COORDINATE_EXT,result.getXyzw());
	return result;
	}

template <class ScalarParam>
inline void glGetCurrentFogCoordEXT(GLVector<ScalarParam,1>& param)
	{
	glGet(GL_CURRENT_FOG_COORDINATE_EXT,param.getXyzw());
	}

#endif
