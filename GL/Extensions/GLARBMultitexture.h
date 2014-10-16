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

#ifndef GLEXTENSIONS_GLARBMULTITEXTURE_INCLUDED
#define GLEXTENSIONS_GLARBMULTITEXTURE_INCLUDED

#include <GL/gl.h>
#include <GL/TLSHelper.h>
#include <GL/Extensions/GLExtension.h>

/********************************
Extension-specific parts of gl.h:
********************************/

#ifndef GL_ARB_multitexture
#define GL_ARB_multitexture 1

/* Extension-specific functions: */
typedef void (APIENTRY * PFNGLACTIVETEXTUREARBPROC) (GLenum texture);
typedef void (APIENTRY * PFNGLCLIENTACTIVETEXTUREARBPROC) (GLenum texture);
typedef void (APIENTRY * PFNGLMULTITEXCOORD1DARBPROC) (GLenum target, GLdouble s);
typedef void (APIENTRY * PFNGLMULTITEXCOORD1DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD1FARBPROC) (GLenum target, GLfloat s);
typedef void (APIENTRY * PFNGLMULTITEXCOORD1FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD1IARBPROC) (GLenum target, GLint s);
typedef void (APIENTRY * PFNGLMULTITEXCOORD1IVARBPROC) (GLenum target, const GLint *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD1SARBPROC) (GLenum target, GLshort s);
typedef void (APIENTRY * PFNGLMULTITEXCOORD1SVARBPROC) (GLenum target, const GLshort *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD2DARBPROC) (GLenum target, GLdouble s, GLdouble t);
typedef void (APIENTRY * PFNGLMULTITEXCOORD2DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD2FARBPROC) (GLenum target, GLfloat s, GLfloat t);
typedef void (APIENTRY * PFNGLMULTITEXCOORD2FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD2IARBPROC) (GLenum target, GLint s, GLint t);
typedef void (APIENTRY * PFNGLMULTITEXCOORD2IVARBPROC) (GLenum target, const GLint *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD2SARBPROC) (GLenum target, GLshort s, GLshort t);
typedef void (APIENTRY * PFNGLMULTITEXCOORD2SVARBPROC) (GLenum target, const GLshort *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD3DARBPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r);
typedef void (APIENTRY * PFNGLMULTITEXCOORD3DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD3FARBPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r);
typedef void (APIENTRY * PFNGLMULTITEXCOORD3FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD3IARBPROC) (GLenum target, GLint s, GLint t, GLint r);
typedef void (APIENTRY * PFNGLMULTITEXCOORD3IVARBPROC) (GLenum target, const GLint *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD3SARBPROC) (GLenum target, GLshort s, GLshort t, GLshort r);
typedef void (APIENTRY * PFNGLMULTITEXCOORD3SVARBPROC) (GLenum target, const GLshort *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD4DARBPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
typedef void (APIENTRY * PFNGLMULTITEXCOORD4DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD4FARBPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef void (APIENTRY * PFNGLMULTITEXCOORD4FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD4IARBPROC) (GLenum target, GLint s, GLint t, GLint r, GLint q);
typedef void (APIENTRY * PFNGLMULTITEXCOORD4IVARBPROC) (GLenum target, const GLint *v);
typedef void (APIENTRY * PFNGLMULTITEXCOORD4SARBPROC) (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
typedef void (APIENTRY * PFNGLMULTITEXCOORD4SVARBPROC) (GLenum target, const GLshort *v);

/* Extension-specific constants: */
#define GL_TEXTURE0_ARB                   0x84C0
#define GL_TEXTURE1_ARB                   0x84C1
#define GL_TEXTURE2_ARB                   0x84C2
#define GL_TEXTURE3_ARB                   0x84C3
#define GL_TEXTURE4_ARB                   0x84C4
#define GL_TEXTURE5_ARB                   0x84C5
#define GL_TEXTURE6_ARB                   0x84C6
#define GL_TEXTURE7_ARB                   0x84C7
#define GL_TEXTURE8_ARB                   0x84C8
#define GL_TEXTURE9_ARB                   0x84C9
#define GL_TEXTURE10_ARB                  0x84CA
#define GL_TEXTURE11_ARB                  0x84CB
#define GL_TEXTURE12_ARB                  0x84CC
#define GL_TEXTURE13_ARB                  0x84CD
#define GL_TEXTURE14_ARB                  0x84CE
#define GL_TEXTURE15_ARB                  0x84CF
#define GL_TEXTURE16_ARB                  0x84D0
#define GL_TEXTURE17_ARB                  0x84D1
#define GL_TEXTURE18_ARB                  0x84D2
#define GL_TEXTURE19_ARB                  0x84D3
#define GL_TEXTURE20_ARB                  0x84D4
#define GL_TEXTURE21_ARB                  0x84D5
#define GL_TEXTURE22_ARB                  0x84D6
#define GL_TEXTURE23_ARB                  0x84D7
#define GL_TEXTURE24_ARB                  0x84D8
#define GL_TEXTURE25_ARB                  0x84D9
#define GL_TEXTURE26_ARB                  0x84DA
#define GL_TEXTURE27_ARB                  0x84DB
#define GL_TEXTURE28_ARB                  0x84DC
#define GL_TEXTURE29_ARB                  0x84DD
#define GL_TEXTURE30_ARB                  0x84DE
#define GL_TEXTURE31_ARB                  0x84DF
#define GL_ACTIVE_TEXTURE_ARB             0x84E0
#define GL_CLIENT_ACTIVE_TEXTURE_ARB      0x84E1
#define GL_MAX_TEXTURE_UNITS_ARB          0x84E2

#endif

/* Forward declarations of friend functions: */
void glActiveTextureARB(GLenum texture);
void glClientActiveTextureARB(GLenum texture);
void glMultiTexCoord1dARB(GLenum target,GLdouble s);
void glMultiTexCoord1dvARB(GLenum target,const GLdouble* v);
void glMultiTexCoord1fARB(GLenum target,GLfloat s);
void glMultiTexCoord1fvARB(GLenum target,const GLfloat* v);
void glMultiTexCoord1iARB(GLenum target,GLint s);
void glMultiTexCoord1ivARB(GLenum target,const GLint* v);
void glMultiTexCoord1sARB(GLenum target,GLshort s);
void glMultiTexCoord1svARB(GLenum target,const GLshort* v);
void glMultiTexCoord2dARB(GLenum target,GLdouble s,GLdouble t);
void glMultiTexCoord2dvARB(GLenum target,const GLdouble* v);
void glMultiTexCoord2fARB(GLenum target,GLfloat s,GLfloat t);
void glMultiTexCoord2fvARB(GLenum target,const GLfloat* v);
void glMultiTexCoord2iARB(GLenum target,GLint s,GLint t);
void glMultiTexCoord2ivARB(GLenum target,const GLint* v);
void glMultiTexCoord2sARB(GLenum target,GLshort s,GLshort t);
void glMultiTexCoord2svARB(GLenum target,const GLshort* v);
void glMultiTexCoord3dARB(GLenum target,GLdouble s,GLdouble t,GLdouble r);
void glMultiTexCoord3dvARB(GLenum target,const GLdouble* v);
void glMultiTexCoord3fARB(GLenum target,GLfloat s,GLfloat t,GLfloat r);
void glMultiTexCoord3fvARB(GLenum target,const GLfloat* v);
void glMultiTexCoord3iARB(GLenum target,GLint s,GLint t,GLint r);
void glMultiTexCoord3ivARB(GLenum target,const GLint* v);
void glMultiTexCoord3sARB(GLenum target,GLshort s,GLshort t,GLshort r);
void glMultiTexCoord3svARB(GLenum target,const GLshort* v);
void glMultiTexCoord4dARB(GLenum target,GLdouble s,GLdouble t,GLdouble r,GLdouble q);
void glMultiTexCoord4dvARB(GLenum target,const GLdouble* v);
void glMultiTexCoord4fARB(GLenum target,GLfloat s,GLfloat t,GLfloat r,GLfloat q);
void glMultiTexCoord4fvARB(GLenum target,const GLfloat* v);
void glMultiTexCoord4iARB(GLenum target,GLint s,GLint t,GLint r,GLint q);
void glMultiTexCoord4ivARB(GLenum target,const GLint* v);
void glMultiTexCoord4sARB(GLenum target,GLshort s,GLshort t,GLshort r,GLshort q);
void glMultiTexCoord4svARB(GLenum target,const GLshort* v);

class GLARBMultitexture:public GLExtension
	{
	/* Elements: */
	private:
	static GL_THREAD_LOCAL(GLARBMultitexture*) current; // Pointer to extension object for current OpenGL context
	static const char* name; // Extension name
	PFNGLACTIVETEXTUREARBPROC glActiveTextureARBProc;
	PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARBProc;
	PFNGLMULTITEXCOORD1DARBPROC glMultiTexCoord1dARBProc;
	PFNGLMULTITEXCOORD1DVARBPROC glMultiTexCoord1dvARBProc;
	PFNGLMULTITEXCOORD1FARBPROC glMultiTexCoord1fARBProc;
	PFNGLMULTITEXCOORD1FVARBPROC glMultiTexCoord1fvARBProc;
	PFNGLMULTITEXCOORD1IARBPROC glMultiTexCoord1iARBProc;
	PFNGLMULTITEXCOORD1IVARBPROC glMultiTexCoord1ivARBProc;
	PFNGLMULTITEXCOORD1SARBPROC glMultiTexCoord1sARBProc;
	PFNGLMULTITEXCOORD1SVARBPROC glMultiTexCoord1svARBProc;
	PFNGLMULTITEXCOORD2DARBPROC glMultiTexCoord2dARBProc;
	PFNGLMULTITEXCOORD2DVARBPROC glMultiTexCoord2dvARBProc;
	PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARBProc;
	PFNGLMULTITEXCOORD2FVARBPROC glMultiTexCoord2fvARBProc;
	PFNGLMULTITEXCOORD2IARBPROC glMultiTexCoord2iARBProc;
	PFNGLMULTITEXCOORD2IVARBPROC glMultiTexCoord2ivARBProc;
	PFNGLMULTITEXCOORD2SARBPROC glMultiTexCoord2sARBProc;
	PFNGLMULTITEXCOORD2SVARBPROC glMultiTexCoord2svARBProc;
	PFNGLMULTITEXCOORD3DARBPROC glMultiTexCoord3dARBProc;
	PFNGLMULTITEXCOORD3DVARBPROC glMultiTexCoord3dvARBProc;
	PFNGLMULTITEXCOORD3FARBPROC glMultiTexCoord3fARBProc;
	PFNGLMULTITEXCOORD3FVARBPROC glMultiTexCoord3fvARBProc;
	PFNGLMULTITEXCOORD3IARBPROC glMultiTexCoord3iARBProc;
	PFNGLMULTITEXCOORD3IVARBPROC glMultiTexCoord3ivARBProc;
	PFNGLMULTITEXCOORD3SARBPROC glMultiTexCoord3sARBProc;
	PFNGLMULTITEXCOORD3SVARBPROC glMultiTexCoord3svARBProc;
	PFNGLMULTITEXCOORD4DARBPROC glMultiTexCoord4dARBProc;
	PFNGLMULTITEXCOORD4DVARBPROC glMultiTexCoord4dvARBProc;
	PFNGLMULTITEXCOORD4FARBPROC glMultiTexCoord4fARBProc;
	PFNGLMULTITEXCOORD4FVARBPROC glMultiTexCoord4fvARBProc;
	PFNGLMULTITEXCOORD4IARBPROC glMultiTexCoord4iARBProc;
	PFNGLMULTITEXCOORD4IVARBPROC glMultiTexCoord4ivARBProc;
	PFNGLMULTITEXCOORD4SARBPROC glMultiTexCoord4sARBProc;
	PFNGLMULTITEXCOORD4SVARBPROC glMultiTexCoord4svARBProc;
	
	/* Constructors and destructors: */
	private:
	GLARBMultitexture(void);
	public:
	virtual ~GLARBMultitexture(void);
	
	/* Methods: */
	public:
	virtual const char* getExtensionName(void) const;
	virtual void activate(void);
	virtual void deactivate(void);
	static bool isSupported(void); // Returns true if the extension is supported in the current OpenGL context
	static void initExtension(void); // Initializes the extension in the current OpenGL context
	
	/* Extension entry points: */
	inline friend void glActiveTextureARB(GLenum texture)
		{
		GLARBMultitexture::current->glActiveTextureARBProc(texture);
		}
	inline friend void glClientActiveTextureARB(GLenum texture)
		{
		GLARBMultitexture::current->glClientActiveTextureARBProc(texture);
		}
	inline friend void glMultiTexCoord1dARB(GLenum target,GLdouble s)
		{
		GLARBMultitexture::current->glMultiTexCoord1dARBProc(target,s);
		}
	inline friend void glMultiTexCoord1dvARB(GLenum target,const GLdouble* v)
		{
		GLARBMultitexture::current->glMultiTexCoord1dvARBProc(target,v);
		}
	inline friend void glMultiTexCoord1fARB(GLenum target,GLfloat s)
		{
		GLARBMultitexture::current->glMultiTexCoord1fARBProc(target,s);
		}
	inline friend void glMultiTexCoord1fvARB(GLenum target,const GLfloat* v)
		{
		GLARBMultitexture::current->glMultiTexCoord1fvARBProc(target,v);
		}
	inline friend void glMultiTexCoord1iARB(GLenum target,GLint s)
		{
		GLARBMultitexture::current->glMultiTexCoord1iARBProc(target,s);
		}
	inline friend void glMultiTexCoord1ivARB(GLenum target,const GLint* v)
		{
		GLARBMultitexture::current->glMultiTexCoord1ivARBProc(target,v);
		}
	inline friend void glMultiTexCoord1sARB(GLenum target,GLshort s)
		{
		GLARBMultitexture::current->glMultiTexCoord1sARBProc(target,s);
		}
	inline friend void glMultiTexCoord1svARB(GLenum target,const GLshort* v)
		{
		GLARBMultitexture::current->glMultiTexCoord1svARBProc(target,v);
		}
	inline friend void glMultiTexCoord2dARB(GLenum target,GLdouble s,GLdouble t)
		{
		GLARBMultitexture::current->glMultiTexCoord2dARBProc(target,s,t);
		}
	inline friend void glMultiTexCoord2dvARB(GLenum target,const GLdouble* v)
		{
		GLARBMultitexture::current->glMultiTexCoord2dvARBProc(target,v);
		}
	inline friend void glMultiTexCoord2fARB(GLenum target,GLfloat s,GLfloat t)
		{
		GLARBMultitexture::current->glMultiTexCoord2fARBProc(target,s,t);
		}
	inline friend void glMultiTexCoord2fvARB(GLenum target,const GLfloat* v)
		{
		GLARBMultitexture::current->glMultiTexCoord2fvARBProc(target,v);
		}
	inline friend void glMultiTexCoord2iARB(GLenum target,GLint s,GLint t)
		{
		GLARBMultitexture::current->glMultiTexCoord2iARBProc(target,s,t);
		}
	inline friend void glMultiTexCoord2ivARB(GLenum target,const GLint* v)
		{
		GLARBMultitexture::current->glMultiTexCoord2ivARBProc(target,v);
		}
	inline friend void glMultiTexCoord2sARB(GLenum target,GLshort s,GLshort t)
		{
		GLARBMultitexture::current->glMultiTexCoord2sARBProc(target,s,t);
		}
	inline friend void glMultiTexCoord2svARB(GLenum target,const GLshort* v)
		{
		GLARBMultitexture::current->glMultiTexCoord2svARBProc(target,v);
		}
	inline friend void glMultiTexCoord3dARB(GLenum target,GLdouble s,GLdouble t,GLdouble r)
		{
		GLARBMultitexture::current->glMultiTexCoord3dARBProc(target,s,t,r);
		}
	inline friend void glMultiTexCoord3dvARB(GLenum target,const GLdouble* v)
		{
		GLARBMultitexture::current->glMultiTexCoord3dvARBProc(target,v);
		}
	inline friend void glMultiTexCoord3fARB(GLenum target,GLfloat s,GLfloat t,GLfloat r)
		{
		GLARBMultitexture::current->glMultiTexCoord3fARBProc(target,s,t,r);
		}
	inline friend void glMultiTexCoord3fvARB(GLenum target,const GLfloat* v)
		{
		GLARBMultitexture::current->glMultiTexCoord3fvARBProc(target,v);
		}
	inline friend void glMultiTexCoord3iARB(GLenum target,GLint s,GLint t,GLint r)
		{
		GLARBMultitexture::current->glMultiTexCoord3iARBProc(target,s,t,r);
		}
	inline friend void glMultiTexCoord3ivARB(GLenum target,const GLint* v)
		{
		GLARBMultitexture::current->glMultiTexCoord3ivARBProc(target,v);
		}
	inline friend void glMultiTexCoord3sARB(GLenum target,GLshort s,GLshort t,GLshort r)
		{
		GLARBMultitexture::current->glMultiTexCoord3sARBProc(target,s,t,r);
		}
	inline friend void glMultiTexCoord3svARB(GLenum target,const GLshort* v)
		{
		GLARBMultitexture::current->glMultiTexCoord3svARBProc(target,v);
		}
	inline friend void glMultiTexCoord4dARB(GLenum target,GLdouble s,GLdouble t,GLdouble r,GLdouble q)
		{
		GLARBMultitexture::current->glMultiTexCoord4dARBProc(target,s,t,r,q);
		}
	inline friend void glMultiTexCoord4dvARB(GLenum target,const GLdouble* v)
		{
		GLARBMultitexture::current->glMultiTexCoord4dvARBProc(target,v);
		}
	inline friend void glMultiTexCoord4fARB(GLenum target,GLfloat s,GLfloat t,GLfloat r,GLfloat q)
		{
		GLARBMultitexture::current->glMultiTexCoord4fARBProc(target,s,t,r,q);
		}
	inline friend void glMultiTexCoord4fvARB(GLenum target,const GLfloat* v)
		{
		GLARBMultitexture::current->glMultiTexCoord4fvARBProc(target,v);
		}
	inline friend void glMultiTexCoord4iARB(GLenum target,GLint s,GLint t,GLint r,GLint q)
		{
		GLARBMultitexture::current->glMultiTexCoord4iARBProc(target,s,t,r,q);
		}
	inline friend void glMultiTexCoord4ivARB(GLenum target,const GLint* v)
		{
		GLARBMultitexture::current->glMultiTexCoord4ivARBProc(target,v);
		}
	inline friend void glMultiTexCoord4sARB(GLenum target,GLshort s,GLshort t,GLshort r,GLshort q)
		{
		GLARBMultitexture::current->glMultiTexCoord4sARBProc(target,s,t,r,q);
		}
	inline friend void glMultiTexCoord4svARB(GLenum target,const GLshort* v)
		{
		GLARBMultitexture::current->glMultiTexCoord4svARBProc(target,v);
		}
	};

/*******************************
Extension-specific entry points:
*******************************/

/***************************************************************
Overloaded versions of component-based glMultiTexCoordARB calls:
***************************************************************/

inline void glMultiTexCoordARB(GLenum target,GLshort s)
	{
	glMultiTexCoord1sARB(target,s);
	}

inline void glMultiTexCoordARB(GLenum target,GLint s)
	{
	glMultiTexCoord1iARB(target,s);
	}

inline void glMultiTexCoordARB(GLenum target,GLfloat s)
	{
	glMultiTexCoord1fARB(target,s);
	}

inline void glMultiTexCoordARB(GLenum target,GLdouble s)
	{
	glMultiTexCoord1dARB(target,s);
	}

inline void glMultiTexCoordARB(GLenum target,GLshort s,GLshort t)
	{
	glMultiTexCoord2sARB(target,s,t);
	}

inline void glMultiTexCoordARB(GLenum target,GLint s,GLint t)
	{
	glMultiTexCoord2iARB(target,s,t);
	}

inline void glMultiTexCoordARB(GLenum target,GLfloat s,GLfloat t)
	{
	glMultiTexCoord2fARB(target,s,t);
	}

inline void glMultiTexCoordARB(GLenum target,GLdouble s,GLdouble t)
	{
	glMultiTexCoord2dARB(target,s,t);
	}

inline void glMultiTexCoordARB(GLenum target,GLshort s,GLshort t,GLshort r)
	{
	glMultiTexCoord3sARB(target,s,t,r);
	}

inline void glMultiTexCoordARB(GLenum target,GLint s,GLint t,GLint r)
	{
	glMultiTexCoord3iARB(target,s,t,r);
	}

inline void glMultiTexCoordARB(GLenum target,GLfloat s,GLfloat t,GLfloat r)
	{
	glMultiTexCoord3fARB(target,s,t,r);
	}

inline void glMultiTexCoordARB(GLenum target,GLdouble s,GLdouble t,GLdouble r)
	{
	glMultiTexCoord3dARB(target,s,t,r);
	}

inline void glMultiTexCoordARB(GLenum target,GLshort s,GLshort t,GLshort r,GLshort q)
	{
	glMultiTexCoord4sARB(target,s,t,r,q);
	}

inline void glMultiTexCoordARB(GLenum target,GLint s,GLint t,GLint r,GLint q)
	{
	glMultiTexCoord4iARB(target,s,t,r,q);
	}

inline void glMultiTexCoordARB(GLenum target,GLfloat s,GLfloat t,GLfloat r,GLfloat q)
	{
	glMultiTexCoord4fARB(target,s,t,r,q);
	}

inline void glMultiTexCoordARB(GLenum target,GLdouble s,GLdouble t,GLdouble r,GLdouble q)
	{
	glMultiTexCoord4dARB(target,s,t,r,q);
	}

/************************************************************
Dummy generic version of array-based glMultiTexCoordARB call:
************************************************************/

template <GLsizei numComponents,class ScalarParam>
void glMultiTexCoordARB(GLenum target,const ScalarParam components[numComponents]);

/************************************************************
Specialized versions of array-based glMultiTexCoordARB calls:
************************************************************/

template <>
inline void glMultiTexCoordARB<1,GLshort>(GLenum target,const GLshort components[1])
	{
	glMultiTexCoord1svARB(target,components);
	}

template <>
inline void glMultiTexCoordARB<1,GLint>(GLenum target,const GLint components[1])
	{
	glMultiTexCoord1ivARB(target,components);
	}

template <>
inline void glMultiTexCoordARB<1,GLfloat>(GLenum target,const GLfloat components[1])
	{
	glMultiTexCoord1fvARB(target,components);
	}

template <>
inline void glMultiTexCoordARB<1,GLdouble>(GLenum target,const GLdouble components[1])
	{
	glMultiTexCoord1dvARB(target,components);
	}

template <>
inline void glMultiTexCoordARB<2,GLshort>(GLenum target,const GLshort components[2])
	{
	glMultiTexCoord2svARB(target,components);
	}

template <>
inline void glMultiTexCoordARB<2,GLint>(GLenum target,const GLint components[2])
	{
	glMultiTexCoord2ivARB(target,components);
	}

template <>
inline void glMultiTexCoordARB<2,GLfloat>(GLenum target,const GLfloat components[2])
	{
	glMultiTexCoord2fvARB(target,components);
	}

template <>
inline void glMultiTexCoordARB<2,GLdouble>(GLenum target,const GLdouble components[2])
	{
	glMultiTexCoord2dvARB(target,components);
	}

template <>
inline void glMultiTexCoordARB<3,GLshort>(GLenum target,const GLshort components[3])
	{
	glMultiTexCoord3svARB(target,components);
	}

template <>
inline void glMultiTexCoordARB<3,GLint>(GLenum target,const GLint components[3])
	{
	glMultiTexCoord3ivARB(target,components);
	}

template <>
inline void glMultiTexCoordARB<3,GLfloat>(GLenum target,const GLfloat components[3])
	{
	glMultiTexCoord3fvARB(target,components);
	}

template <>
inline void glMultiTexCoordARB<3,GLdouble>(GLenum target,const GLdouble components[3])
	{
	glMultiTexCoord3dvARB(target,components);
	}

template <>
inline void glMultiTexCoordARB<4,GLshort>(GLenum target,const GLshort components[4])
	{
	glMultiTexCoord4svARB(target,components);
	}

template <>
inline void glMultiTexCoordARB<4,GLint>(GLenum target,const GLint components[4])
	{
	glMultiTexCoord4ivARB(target,components);
	}

template <>
inline void glMultiTexCoordARB<4,GLfloat>(GLenum target,const GLfloat components[4])
	{
	glMultiTexCoord4fvARB(target,components);
	}

template <>
inline void glMultiTexCoordARB<4,GLdouble>(GLenum target,const GLdouble components[4])
	{
	glMultiTexCoord4dvARB(target,components);
	}

#endif
