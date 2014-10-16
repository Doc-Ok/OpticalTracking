/***********************************************************************
GLAutomaticShader - Base class for self-contained shaders that adapt
themselves to the current state of the active OpenGL context.
Copyright (c) 2012 Oliver Kreylos

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

#ifndef GLAUTOMATICSHADER_INCLUDED
#define GLAUTOMATICSHADER_INCLUDED

/* Forward declarations: */
class GLContextData;

class GLAutomaticShader
	{
	/* Elements: */
	protected:
	GLContextData& contextData; // OpenGL context with which this shader is permanently associated
	
	/* Constructors and destructors: */
	public:
	GLAutomaticShader(GLContextData& sContextData); // Creates a shader for the given OpenGL context
	virtual ~GLAutomaticShader(void); // Destroys the shader
	
	/* Methods: */
	virtual void update(void) =0; // Updates the shader to the current state of its OpenGL context
	virtual void activate(void) =0; // Sets up the shader's OpenGL context such that the shader can be used immediately
	virtual void deactivate(void) =0; // Returns the shader's OpenGL context to its state before activate() was called
	};

#endif
