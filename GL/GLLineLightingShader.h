/***********************************************************************
GLLineLightingShader - Automatic shader class for Phong illumination of
lines with tangent vectors.
Copyright (c) 2012-2013 Oliver Kreylos

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

#ifndef GLLINELIGHTINGSHADER_INCLUDED
#define GLLINELIGHTINGSHADER_INCLUDED

#include <GL/GLShader.h>
#include <GL/GLAutomaticShader.h>

class GLLineLightingShader:public GLAutomaticShader
	{
	/* Elements: */
	private:
	static const char accumulateLightTemplate[]; // GLSL function template to illuminate with a standard light source
	static const char fragmentShaderMain[]; // GLSL function for the fragment shader
	GLShader shader; // The GLSL shader object containing the linked line lighting shader program
	unsigned int lightTrackerVersion; // Version number of the OpenGL lighting state for which the shader program was built
	unsigned int clipPlaneTrackerVersion; // Version number of the OpenGL clipping plane state for which the shader program was built
	
	/* Private methods: */
	void buildShader(void); // Builds the shader program based on the OpenGL context's state
	
	/* Constructors and destructors: */
	public:
	GLLineLightingShader(GLContextData& sContextData);
	virtual ~GLLineLightingShader(void);
	
	/* Methods from GLAutomaticShader: */
	virtual void update(void);
	virtual void activate(void);
	virtual void deactivate(void);
	
	/* New methods: */
	static bool isSupported(GLContextData& contextData); // Returns true if line lighting is supported in the given OpenGL context
	};

#endif
