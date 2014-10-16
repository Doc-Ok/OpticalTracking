/***********************************************************************
GLEXTPalettedTexture - OpenGL extension class for the
GL_EXT_paletted_texture extension.
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

#ifndef GLEXTENSIONS_GLEXTPALETTEDTEXTURE_INCLUDED
#define GLEXTENSIONS_GLEXTPALETTEDTEXTURE_INCLUDED

#include <GL/gl.h>
#include <GL/TLSHelper.h>
#include <GL/Extensions/GLExtension.h>

/********************************
Extension-specific parts of gl.h:
********************************/

#ifndef GL_EXT_paletted_texture
#define GL_EXT_paletted_texture 1

/* Extension-specific functions: */
typedef void (APIENTRY * PFNGLCOLORTABLEEXTPROC) (GLenum target, GLenum internalFormat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
typedef void (APIENTRY * PFNGLCOLORSUBTABLEEXTPROC) (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);
typedef void (APIENTRY * PFNGLGETCOLORTABLEEXTPROC) (GLenum target, GLenum format, GLenum type, GLvoid *data);
typedef void (APIENTRY * PFNGLGETCOLORTABLEPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETCOLORTABLEPARAMETERFVEXTPROC) (GLenum target, GLenum pname, GLfloat *params);

/* Extension-specific constants: */
#define GL_COLOR_INDEX1_EXT               0x80E2
#define GL_COLOR_INDEX2_EXT               0x80E3
#define GL_COLOR_INDEX4_EXT               0x80E4
#define GL_COLOR_INDEX8_EXT               0x80E5
#define GL_COLOR_INDEX12_EXT              0x80E6
#define GL_COLOR_INDEX16_EXT              0x80E7
#define GL_TEXTURE_INDEX_SIZE_EXT         0x80ED
#define GL_COLOR_TABLE_FORMAT_EXT         0x80D8
#define GL_COLOR_TABLE_WIDTH_EXT_EXT_EXT  0x80D9
#define GL_COLOR_TABLE_RED_SIZE_EXT_EXT   0x80DA
#define GL_COLOR_TABLE_GREEN_SIZE_EXT_EXT 0x80DB
#define GL_COLOR_TABLE_BLUE_SIZE_EXT_EXT  0x80DC
#define GL_COLOR_TABLE_ALPHA_SIZE_EXT_EXT 0x80DD
#define GL_COLOR_TABLE_LUMINANCE_SIZE_EXT 0x80DE
#define GL_COLOR_TABLE_INTENSITY_SIZE_EXT 0x80DF

#endif

/* Forward declarations of friend functions: */
void glColorTableEXT(GLenum target,GLenum internalFormat,GLsizei width,GLenum format,GLenum type,const GLvoid* table);
void glColorSubTableEXT(GLenum target,GLsizei start,GLsizei count,GLenum format,GLenum type,const GLvoid* table);
void glGetColorTableEXT(GLenum target,GLenum format,GLenum type,GLvoid* data);
void glGetColorTableParameterivEXT(GLenum target,GLenum pname,GLint* params);
void glGetColorTableParameterfvEXT(GLenum target,GLenum pname,GLfloat* params);

class GLEXTPalettedTexture:public GLExtension
	{
	/* Elements: */
	private:
	static GL_THREAD_LOCAL(GLEXTPalettedTexture*) current; // Pointer to extension object for current OpenGL context
	static const char* name; // Extension name
	PFNGLCOLORTABLEEXTPROC glColorTableEXTProc;
	PFNGLCOLORSUBTABLEEXTPROC glColorSubTableEXTProc;
	PFNGLGETCOLORTABLEEXTPROC glGetColorTableEXTProc;
	PFNGLGETCOLORTABLEPARAMETERIVEXTPROC glGetColorTableParameterivEXTProc;
	PFNGLGETCOLORTABLEPARAMETERFVEXTPROC glGetColorTableParameterfvEXTProc;
	
	/* Constructors and destructors: */
	private:
	GLEXTPalettedTexture(void);
	public:
	virtual ~GLEXTPalettedTexture(void);
	
	/* Methods: */
	public:
	virtual const char* getExtensionName(void) const;
	virtual void activate(void);
	virtual void deactivate(void);
	static bool isSupported(void); // Returns true if the extension is supported in the current OpenGL context
	static void initExtension(void); // Initializes the extension in the current OpenGL context
	
	/* Extension entry points: */
	inline friend void glColorTableEXT(GLenum target,GLenum internalFormat,GLsizei width,GLenum format,GLenum type,const GLvoid* table)
		{
		GLEXTPalettedTexture::current->glColorTableEXTProc(target,internalFormat,width,format,type,table);
		}
	inline friend void glColorSubTableEXT(GLenum target,GLsizei start,GLsizei count,GLenum format,GLenum type,const GLvoid* table)
		{
		GLEXTPalettedTexture::current->glColorSubTableEXTProc(target,start,count,format,type,table);
		}
	inline friend void glGetColorTableEXT(GLenum target,GLenum format,GLenum type,GLvoid* data)
		{
		GLEXTPalettedTexture::current->glGetColorTableEXTProc(target,format,type,data);
		}
	inline friend void glGetColorTableParameterivEXT(GLenum target,GLenum pname,GLint* params)
		{
		GLEXTPalettedTexture::current->glGetColorTableParameterivEXTProc(target,pname,params);
		}
	inline friend void glGetColorTableParameterfvEXT(GLenum target,GLenum pname,GLfloat* params)
		{
		GLEXTPalettedTexture::current->glGetColorTableParameterfvEXTProc(target,pname,params);
		}
	};

/*******************************
Extension-specific entry points:
*******************************/

#endif
