/***********************************************************************
GLARBFragmentProgram - OpenGL extension class for the
GL_ARB_fragment_program extension.
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

#ifndef GLEXTENSIONS_GLARBFRAGMENTPROGRAM_INCLUDED
#define GLEXTENSIONS_GLARBFRAGMENTPROGRAM_INCLUDED

#include <GL/gl.h>
#include <GL/TLSHelper.h>
#include <GL/Extensions/GLExtension.h>
#include <GL/Extensions/GLARBVertexProgram.h>

/********************************
Extension-specific parts of gl.h:
********************************/

#ifndef GL_ARB_fragment_program
#define GL_ARB_fragment_program 1

/* Extension-specific constants: */
#define GL_FRAGMENT_PROGRAM_ARB                    0x8804
#define GL_PROGRAM_ALU_INSTRUCTIONS_ARB            0x8805
#define GL_PROGRAM_TEX_INSTRUCTIONS_ARB            0x8806
#define GL_PROGRAM_TEX_INDIRECTIONS_ARB            0x8807
#define GL_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB     0x8808
#define GL_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB     0x8809
#define GL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB     0x880A
#define GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB        0x880B
#define GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB        0x880C
#define GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB        0x880D
#define GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB 0x880E
#define GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB 0x880F
#define GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB 0x8810
#define GL_MAX_TEXTURE_COORDS_ARB                  0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS_ARB             0x8872

#endif

class GLARBFragmentProgram:public GLExtension
	{
	/* Elements: */
	private:
	static GL_THREAD_LOCAL(GLARBFragmentProgram*) current; // Pointer to extension object for current OpenGL context
	static const char* name; // Extension name
	
	/* Constructors and destructors: */
	private:
	GLARBFragmentProgram(void);
	public:
	virtual ~GLARBFragmentProgram(void);
	
	/* Methods: */
	public:
	virtual const char* getExtensionName(void) const;
	virtual void activate(void);
	virtual void deactivate(void);
	static bool isSupported(void); // Returns true if the extension is supported in the current OpenGL context
	static void initExtension(void); // Initializes the extension in the current OpenGL context
	};

/*******************************
Extension-specific entry points:
*******************************/

#endif
