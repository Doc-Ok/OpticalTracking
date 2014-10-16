/***********************************************************************
GLARBTextureMultisample - OpenGL extension class for the
GL_ARB_texture_multisample extension.
Note: The functions and constants exported by this extension do *not*
use the ARB suffix.
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

#ifndef GLEXTENSIONS_GLARBTEXTUREMULTISAMPLE_INCLUDED
#define GLEXTENSIONS_GLARBTEXTUREMULTISAMPLE_INCLUDED

#include <GL/gl.h>
#include <GL/TLSHelper.h>
#include <GL/Extensions/GLExtension.h>

/********************************
Extension-specific parts of gl.h:
********************************/

#ifndef GL_ARB_texture_multisample
#define GL_ARB_texture_multisample 1

/* Extension-specific functions: */
typedef void (APIENTRY * PFNGLTEXIMAGE2DMULTISAMPLEPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
typedef void (APIENTRY * PFNGLTEXIMAGE3DMULTISAMPLEPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
typedef void (APIENTRY * PFNGLGETMULTISAMPLEFVPROC) (GLenum pname, GLuint index, GLfloat *val);
typedef void (APIENTRY * PFNGLSAMPLEMASKIPROC) (GLuint index, GLbitfield mask);

/* Extension-specific constants: */
#define GL_SAMPLE_POSITION                             0x8E50
#define GL_SAMPLE_MASK                                 0x8E51
#define GL_SAMPLE_MASK_VALUE                           0x8E52
#define GL_TEXTURE_2D_MULTISAMPLE                      0x9100
#define GL_PROXY_TEXTURE_2D_MULTISAMPLE                0x9101
#define GL_TEXTURE_2D_MULTISAMPLE_ARRAY                0x9102
#define GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY          0x9103
#define GL_MAX_SAMPLE_MASK_WORDS                       0x8E59
#define GL_MAX_COLOR_TEXTURE_SAMPLES                   0x910E
#define GL_MAX_DEPTH_TEXTURE_SAMPLES                   0x910F
#define GL_MAX_INTEGER_SAMPLES                         0x9110
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE              0x9104
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY        0x9105
#define GL_TEXTURE_SAMPLES                             0x9106
#define GL_TEXTURE_FIXED_SAMPLE_LOCATIONS              0x9107
#define GL_SAMPLER_2D_MULTISAMPLE                      0x9108
#define GL_INT_SAMPLER_2D_MULTISAMPLE                  0x9109
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE         0x910A
#define GL_SAMPLER_2D_MULTISAMPLE_ARRAY                0x910B
#define GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY            0x910C
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY   0x910D

#endif

/* Forward declarations of friend functions: */
void glTexImage2DMultisample(GLenum target,GLsizei samples,GLenum internalformat,GLsizei width,GLsizei height,GLboolean fixedsamplelocations);
void glTexImage3DMultisample(GLenum target,GLsizei samples,GLenum internalformat,GLsizei width,GLsizei height,GLsizei depth,GLboolean fixedsamplelocations);
void glGetMultisamplefv(GLenum pname,GLuint index,GLfloat *val);
void glSampleMaski(GLuint index,GLbitfield mask);

class GLARBTextureMultisample:public GLExtension
	{
	/* Elements: */
	private:
	static GL_THREAD_LOCAL(GLARBTextureMultisample*) current; // Pointer to extension object for current OpenGL context
	static const char* name; // Extension name
	PFNGLTEXIMAGE2DMULTISAMPLEPROC glTexImage2DMultisampleProc;
	PFNGLTEXIMAGE3DMULTISAMPLEPROC glTexImage3DMultisampleProc;
	PFNGLGETMULTISAMPLEFVPROC glGetMultisamplefvProc;
	PFNGLSAMPLEMASKIPROC glSampleMaskiProc;
	
	/* Constructors and destructors: */
	private:
	GLARBTextureMultisample(void);
	public:
	virtual ~GLARBTextureMultisample(void);
	
	/* Methods: */
	public:
	virtual const char* getExtensionName(void) const;
	virtual void activate(void);
	virtual void deactivate(void);
	static bool isSupported(void); // Returns true if the extension is supported in the current OpenGL context
	static void initExtension(void); // Initializes the extension in the current OpenGL context
	
	/* Extension entry points: */
	inline friend void glTexImage2DMultisample(GLenum target,GLsizei samples,GLenum internalformat,GLsizei width,GLsizei height,GLboolean fixedsamplelocations)
		{
		GLARBTextureMultisample::current->glTexImage2DMultisampleProc(target,samples,internalformat,width,height,fixedsamplelocations);
		}
	inline friend void glTexImage3DMultisample(GLenum target,GLsizei samples,GLenum internalformat,GLsizei width,GLsizei height,GLsizei depth,GLboolean fixedsamplelocations)
		{
		GLARBTextureMultisample::current->glTexImage3DMultisampleProc(target,samples,internalformat,width,height,depth,fixedsamplelocations);
		}
	inline friend void glGetMultisamplefv(GLenum pname,GLuint index,GLfloat *val)
		{
		GLARBTextureMultisample::current->glGetMultisamplefvProc(pname,index,val);
		}
	inline friend void glSampleMaski(GLuint index,GLbitfield mask)
		{
		GLARBTextureMultisample::current->glSampleMaskiProc(index,mask);
		}
	};

/*******************************
Extension-specific entry points:
*******************************/

#endif
