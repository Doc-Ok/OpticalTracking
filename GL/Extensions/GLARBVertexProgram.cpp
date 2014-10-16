/***********************************************************************
GLARBVertexProgram - OpenGL extension class for the
GL_ARB_vertex_program extension.
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

#include <GL/Extensions/GLARBVertexProgram.h>

#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>

/*******************************************
Static elements of class GLARBVertexProgram:
*******************************************/

GL_THREAD_LOCAL(GLARBVertexProgram*) GLARBVertexProgram::current=0;
const char* GLARBVertexProgram::name="GL_ARB_vertex_program";

/***********************************
Methods of class GLARBVertexProgram:
***********************************/

GLARBVertexProgram::GLARBVertexProgram(void)
	:glVertexAttrib1sARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB1SARBPROC>("glVertexAttrib1sARB")),
	 glVertexAttrib1fARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB1FARBPROC>("glVertexAttrib1fARB")),
	 glVertexAttrib1dARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB1DARBPROC>("glVertexAttrib1dARB")),
	 glVertexAttrib2sARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB2SARBPROC>("glVertexAttrib2sARB")),
	 glVertexAttrib2fARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB2FARBPROC>("glVertexAttrib2fARB")),
	 glVertexAttrib2dARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB2DARBPROC>("glVertexAttrib2dARB")),
	 glVertexAttrib3sARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB3SARBPROC>("glVertexAttrib3sARB")),
	 glVertexAttrib3fARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB3FARBPROC>("glVertexAttrib3fARB")),
	 glVertexAttrib3dARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB3DARBPROC>("glVertexAttrib3dARB")),
	 glVertexAttrib4sARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB4SARBPROC>("glVertexAttrib4sARB")),
	 glVertexAttrib4fARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB4FARBPROC>("glVertexAttrib4fARB")),
	 glVertexAttrib4dARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB4DARBPROC>("glVertexAttrib4dARB")),
	 glVertexAttrib4NubARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB4NUBARBPROC>("glVertexAttrib4NubARB")),
	 glVertexAttrib1svARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB1SVARBPROC>("glVertexAttrib1svARB")),
	 glVertexAttrib1fvARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB1FVARBPROC>("glVertexAttrib1fvARB")),
	 glVertexAttrib1dvARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB1DVARBPROC>("glVertexAttrib1dvARB")),
	 glVertexAttrib2svARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB2SVARBPROC>("glVertexAttrib2svARB")),
	 glVertexAttrib2fvARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB2FVARBPROC>("glVertexAttrib2fvARB")),
	 glVertexAttrib2dvARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB2DVARBPROC>("glVertexAttrib2dvARB")),
	 glVertexAttrib3svARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB3SVARBPROC>("glVertexAttrib3svARB")),
	 glVertexAttrib3fvARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB3FVARBPROC>("glVertexAttrib3fvARB")),
	 glVertexAttrib3dvARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB3DVARBPROC>("glVertexAttrib3dvARB")),
	 glVertexAttrib4bvARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB4BVARBPROC>("glVertexAttrib4bvARB")),
	 glVertexAttrib4svARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB4SVARBPROC>("glVertexAttrib4svARB")),
	 glVertexAttrib4ivARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB4IVARBPROC>("glVertexAttrib4ivARB")),
	 glVertexAttrib4ubvARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB4UBVARBPROC>("glVertexAttrib4ubvARB")),
	 glVertexAttrib4usvARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB4USVARBPROC>("glVertexAttrib4usvARB")),
	 glVertexAttrib4uivARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB4UIVARBPROC>("glVertexAttrib4uivARB")),
	 glVertexAttrib4fvARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB4FVARBPROC>("glVertexAttrib4fvARB")),
	 glVertexAttrib4dvARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB4DVARBPROC>("glVertexAttrib4dvARB")),
	 glVertexAttrib4NbvARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB4NBVARBPROC>("glVertexAttrib4NbvARB")),
	 glVertexAttrib4NsvARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB4NSVARBPROC>("glVertexAttrib4NsvARB")),
	 glVertexAttrib4NivARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB4NIVARBPROC>("glVertexAttrib4NivARB")),
	 glVertexAttrib4NubvARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB4NUBVARBPROC>("glVertexAttrib4NubvARB")),
	 glVertexAttrib4NusvARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB4NUSVARBPROC>("glVertexAttrib4NusvARB")),
	 glVertexAttrib4NuivARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIB4NUIVARBPROC>("glVertexAttrib4NuivARB")),
	 glVertexAttribPointerARBProc(GLExtensionManager::getFunction<PFNGLVERTEXATTRIBPOINTERARBPROC>("glVertexAttribPointerARB")),
	 glEnableVertexAttribArrayARBProc(GLExtensionManager::getFunction<PFNGLENABLEVERTEXATTRIBARRAYARBPROC>("glEnableVertexAttribArrayARB")),
	 glDisableVertexAttribArrayARBProc(GLExtensionManager::getFunction<PFNGLDISABLEVERTEXATTRIBARRAYARBPROC>("glDisableVertexAttribArrayARB")),
	 glProgramStringARBProc(GLExtensionManager::getFunction<PFNGLPROGRAMSTRINGARBPROC>("glProgramStringARB")),
	 glBindProgramARBProc(GLExtensionManager::getFunction<PFNGLBINDPROGRAMARBPROC>("glBindProgramARB")),
	 glDeleteProgramsARBProc(GLExtensionManager::getFunction<PFNGLDELETEPROGRAMSARBPROC>("glDeleteProgramsARB")),
	 glGenProgramsARBProc(GLExtensionManager::getFunction<PFNGLGENPROGRAMSARBPROC>("glGenProgramsARB")),
	 glProgramEnvParameter4dARBProc(GLExtensionManager::getFunction<PFNGLPROGRAMENVPARAMETER4DARBPROC>("glProgramEnvParameter4dARB")),
	 glProgramEnvParameter4dvARBProc(GLExtensionManager::getFunction<PFNGLPROGRAMENVPARAMETER4DVARBPROC>("glProgramEnvParameter4dvARB")),
	 glProgramEnvParameter4fARBProc(GLExtensionManager::getFunction<PFNGLPROGRAMENVPARAMETER4FARBPROC>("glProgramEnvParameter4fARB")),
	 glProgramEnvParameter4fvARBProc(GLExtensionManager::getFunction<PFNGLPROGRAMENVPARAMETER4FVARBPROC>("glProgramEnvParameter4fvARB")),
	 glProgramLocalParameter4dARBProc(GLExtensionManager::getFunction<PFNGLPROGRAMLOCALPARAMETER4DARBPROC>("glProgramLocalParameter4dARB")),
	 glProgramLocalParameter4dvARBProc(GLExtensionManager::getFunction<PFNGLPROGRAMLOCALPARAMETER4DVARBPROC>("glProgramLocalParameter4dvARB")),
	 glProgramLocalParameter4fARBProc(GLExtensionManager::getFunction<PFNGLPROGRAMLOCALPARAMETER4FARBPROC>("glProgramLocalParameter4fARB")),
	 glProgramLocalParameter4fvARBProc(GLExtensionManager::getFunction<PFNGLPROGRAMLOCALPARAMETER4FVARBPROC>("glProgramLocalParameter4fvARB")),
	 glGetProgramEnvParameterdvARBProc(GLExtensionManager::getFunction<PFNGLGETPROGRAMENVPARAMETERDVARBPROC>("glGetProgramEnvParameterdvARB")),
	 glGetProgramEnvParameterfvARBProc(GLExtensionManager::getFunction<PFNGLGETPROGRAMENVPARAMETERFVARBPROC>("glGetProgramEnvParameterfvARB")),
	 glGetProgramLocalParameterdvARBProc(GLExtensionManager::getFunction<PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC>("glGetProgramLocalParameterdvARB")),
	 glGetProgramLocalParameterfvARBProc(GLExtensionManager::getFunction<PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC>("glGetProgramLocalParameterfvARB")),
	 glGetProgramivARBProc(GLExtensionManager::getFunction<PFNGLGETPROGRAMIVARBPROC>("glGetProgramivARB")),
	 glGetProgramStringARBProc(GLExtensionManager::getFunction<PFNGLGETPROGRAMSTRINGARBPROC>("glGetProgramStringARB")),
	 glGetVertexAttribdvARBProc(GLExtensionManager::getFunction<PFNGLGETVERTEXATTRIBDVARBPROC>("glGetVertexAttribdvARB")),
	 glGetVertexAttribfvARBProc(GLExtensionManager::getFunction<PFNGLGETVERTEXATTRIBFVARBPROC>("glGetVertexAttribfvARB")),
	 glGetVertexAttribivARBProc(GLExtensionManager::getFunction<PFNGLGETVERTEXATTRIBIVARBPROC>("glGetVertexAttribivARB")),
	 glGetVertexAttribPointervARBProc(GLExtensionManager::getFunction<PFNGLGETVERTEXATTRIBPOINTERVARBPROC>("glGetVertexAttribPointervARB")),
	 glIsProgramARBProc(GLExtensionManager::getFunction<PFNGLISPROGRAMARBPROC>("glIsProgramARB"))
	{
	}

GLARBVertexProgram::~GLARBVertexProgram(void)
	{
	}

const char* GLARBVertexProgram::getExtensionName(void) const
	{
	return name;
	}

void GLARBVertexProgram::activate(void)
	{
	current=this;
	}

void GLARBVertexProgram::deactivate(void)
	{
	current=0;
	}

bool GLARBVertexProgram::isSupported(void)
	{
	/* Ask the current extension manager whether the extension is supported in the current OpenGL context: */
	return GLExtensionManager::isExtensionSupported(name);
	}

void GLARBVertexProgram::initExtension(void)
	{
	/* Check if the extension is already initialized: */
	if(!GLExtensionManager::isExtensionRegistered(name))
		{
		/* Create a new extension object: */
		GLARBVertexProgram* newExtension=new GLARBVertexProgram;
		
		/* Register the extension with the current extension manager: */
		GLExtensionManager::registerExtension(newExtension);
		}
	}
