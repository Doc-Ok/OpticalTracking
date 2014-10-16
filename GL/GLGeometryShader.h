/***********************************************************************
GLGeometryShader - Class to represent GLSL shaders that contain at least
one geometry shader according to the GL_EXT_geometry_shader4 extension.
Copyright (c) 2009-2013 Oliver Kreylos
Based on code provided by Tony Bernardin

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

#ifndef GLGEOMETRYSHADER_INCLUDED
#define GLGEOMETRYSHADER_INCLUDED

#include <GL/GLShader.h>

class GLGeometryShader:public GLShader
	{
	/* Elements: */
	protected:
	HandleList geometryShaderObjects; // List of handles of compiled geometry shader objects
	
	/* Constructors and destructors: */
	public:
	GLGeometryShader(void); // Creates an "empty" shader
	~GLGeometryShader(void); // Destroys a shader
	
	/* Methods: */
	static bool isSupported(void); // Returns true if the current OpenGL context supports GLSL shaders
	void compileGeometryShader(const char* shaderSourceFileName); // Loads and compiles a geometry shader from a source file
	void compileGeometryShaderFromString(const char* shaderSource); // Compiles a geometry shader from a source code string
	void linkShader(GLint geometryInputType,GLint geometryOutputType,GLint maxNumOutputVertices); // Links all previously loaded vertex, geometry, and fragment shaders into a shader program
	};

#endif
