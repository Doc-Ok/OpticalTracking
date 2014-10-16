/***********************************************************************
GLEXTTextureInteger - OpenGL extension class for the
GL_EXT_texture_integer extension.
Copyright (c) 2014 Oliver Kreylos

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

#ifndef GLEXTENSIONS_GLEXTTEXTUREINTEGER_INCLUDED
#define GLEXTENSIONS_GLEXTTEXTUREINTEGER_INCLUDED

#include <GL/gl.h>
#include <GL/TLSHelper.h>
#include <GL/Extensions/GLExtension.h>

/********************************
Extension-specific parts of gl.h:
********************************/

#ifndef GL_EXT_texture_integer
#define GL_EXT_texture_integer 1

/* Extension-specific functions: */
typedef void (APIENTRYP PFNGLTEXPARAMETERIIVEXTPROC) (GLenum target, GLenum pname, const GLint *params);
typedef void (APIENTRYP PFNGLTEXPARAMETERIUIVEXTPROC) (GLenum target, GLenum pname, const GLuint *params);
typedef void (APIENTRYP PFNGLGETTEXPARAMETERIIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETTEXPARAMETERIUIVEXTPROC) (GLenum target, GLenum pname, GLuint *params);
typedef void (APIENTRYP PFNGLCLEARCOLORIIEXTPROC) (GLint red, GLint green, GLint blue, GLint alpha);
typedef void (APIENTRYP PFNGLCLEARCOLORIUIEXTPROC) (GLuint red, GLuint green, GLuint blue, GLuint alpha);

/* Extension-specific constants: */
#define GL_RGBA32UI_EXT                   0x8D70
#define GL_RGB32UI_EXT                    0x8D71
#define GL_ALPHA32UI_EXT                  0x8D72
#define GL_INTENSITY32UI_EXT              0x8D73
#define GL_LUMINANCE32UI_EXT              0x8D74
#define GL_LUMINANCE_ALPHA32UI_EXT        0x8D75
#define GL_RGBA16UI_EXT                   0x8D76
#define GL_RGB16UI_EXT                    0x8D77
#define GL_ALPHA16UI_EXT                  0x8D78
#define GL_INTENSITY16UI_EXT              0x8D79
#define GL_LUMINANCE16UI_EXT              0x8D7A
#define GL_LUMINANCE_ALPHA16UI_EXT        0x8D7B
#define GL_RGBA8UI_EXT                    0x8D7C
#define GL_RGB8UI_EXT                     0x8D7D
#define GL_ALPHA8UI_EXT                   0x8D7E
#define GL_INTENSITY8UI_EXT               0x8D7F
#define GL_LUMINANCE8UI_EXT               0x8D80
#define GL_LUMINANCE_ALPHA8UI_EXT         0x8D81
#define GL_RGBA32I_EXT                    0x8D82
#define GL_RGB32I_EXT                     0x8D83
#define GL_ALPHA32I_EXT                   0x8D84
#define GL_INTENSITY32I_EXT               0x8D85
#define GL_LUMINANCE32I_EXT               0x8D86
#define GL_LUMINANCE_ALPHA32I_EXT         0x8D87
#define GL_RGBA16I_EXT                    0x8D88
#define GL_RGB16I_EXT                     0x8D89
#define GL_ALPHA16I_EXT                   0x8D8A
#define GL_INTENSITY16I_EXT               0x8D8B
#define GL_LUMINANCE16I_EXT               0x8D8C
#define GL_LUMINANCE_ALPHA16I_EXT         0x8D8D
#define GL_RGBA8I_EXT                     0x8D8E
#define GL_RGB8I_EXT                      0x8D8F
#define GL_ALPHA8I_EXT                    0x8D90
#define GL_INTENSITY8I_EXT                0x8D91
#define GL_LUMINANCE8I_EXT                0x8D92
#define GL_LUMINANCE_ALPHA8I_EXT          0x8D93
#define GL_RED_INTEGER_EXT                0x8D94
#define GL_GREEN_INTEGER_EXT              0x8D95
#define GL_BLUE_INTEGER_EXT               0x8D96
#define GL_ALPHA_INTEGER_EXT              0x8D97
#define GL_RGB_INTEGER_EXT                0x8D98
#define GL_RGBA_INTEGER_EXT               0x8D99
#define GL_BGR_INTEGER_EXT                0x8D9A
#define GL_BGRA_INTEGER_EXT               0x8D9B
#define GL_LUMINANCE_INTEGER_EXT          0x8D9C
#define GL_LUMINANCE_ALPHA_INTEGER_EXT    0x8D9D
#define GL_RGBA_INTEGER_MODE_EXT          0x8D9E

#endif

/* Forward declarations of friend functions: */
void glTexParameterIivEXT(GLenum target,GLenum pname,const GLint* params);
void glTexParameterIuivEXT(GLenum target,GLenum pname,const GLuint* params);
void glGetTexParameterIivEXT(GLenum target,GLenum pname,GLint* params);
void glGetTexParameterIuivEXT(GLenum target,GLenum pname,GLuint* params);
void glClearColorIiEXT(GLint red,GLint green,GLint blue,GLint alpha);
void glClearColorIuiEXT(GLuint red,GLuint green,GLuint blue,GLuint alpha);

class GLEXTTextureInteger:public GLExtension
	{
	/* Elements: */
	private:
	static GL_THREAD_LOCAL(GLEXTTextureInteger*) current; // Pointer to extension object for current OpenGL context
	static const char* name; // Extension name
	PFNGLTEXPARAMETERIIVEXTPROC glTexParameterIivEXTProc;
	PFNGLTEXPARAMETERIUIVEXTPROC glTexParameterIuivEXTProc;
	PFNGLGETTEXPARAMETERIIVEXTPROC glGetTexParameterIivEXTProc;
	PFNGLGETTEXPARAMETERIUIVEXTPROC glGetTexParameterIuivEXTProc;
	PFNGLCLEARCOLORIIEXTPROC glClearColorIiEXTProc;
	PFNGLCLEARCOLORIUIEXTPROC glClearColorIuiEXTProc;
	
	/* Constructors and destructors: */
	private:
	GLEXTTextureInteger(void);
	public:
	virtual ~GLEXTTextureInteger(void);
	
	/* Methods: */
	public:
	virtual const char* getExtensionName(void) const;
	virtual void activate(void);
	virtual void deactivate(void);
	static bool isSupported(void); // Returns true if the extension is supported in the current OpenGL context
	static void initExtension(void); // Initializes the extension in the current OpenGL context
	
	/* Extension entry points: */
	inline friend void glTexParameterIivEXT(GLenum target,GLenum pname,const GLint* params)
		{
		GLEXTTextureInteger::current->glTexParameterIivEXTProc(target,pname,params);
		}
	inline friend void glTexParameterIuivEXT(GLenum target,GLenum pname,const GLuint* params)
		{
		GLEXTTextureInteger::current->glTexParameterIuivEXTProc(target,pname,params);
		}
	inline friend void glGetTexParameterIivEXT(GLenum target,GLenum pname,GLint* params)
		{
		GLEXTTextureInteger::current->glGetTexParameterIivEXTProc(target,pname,params);
		}
	inline friend void glGetTexParameterIuivEXT(GLenum target,GLenum pname,GLuint* params)
		{
		GLEXTTextureInteger::current->glGetTexParameterIuivEXTProc(target,pname,params);
		}
	inline friend void glClearColorIiEXT(GLint red,GLint green,GLint blue,GLint alpha)
		{
		GLEXTTextureInteger::current->glClearColorIiEXTProc(red,green,blue,alpha);
		}
	inline friend void glClearColorIuiEXT(GLuint red,GLuint green,GLuint blue,GLuint alpha)
		{
		GLEXTTextureInteger::current->glClearColorIuiEXTProc(red,green,blue,alpha);
		}
	};

/*******************************
Extension-specific entry points:
*******************************/

#endif
