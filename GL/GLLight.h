/***********************************************************************
GLLight - Class to encapsulate OpenGL light source properties.
Copyright (c) 2003-2005 Oliver Kreylos

This file is part of the OpenGL C++ Wrapper Library (GLWrappers).

The OpenGL C++ Wrapper Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The OpenGL C++ Wrapper Library is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the OpenGL C++ Wrapper Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef GLLIGHT_INCLUDED
#define GLLIGHT_INCLUDED

#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLVector.h>

struct GLLight
	{
	/* Embedded classes: */
	public:
	typedef GLfloat Scalar; // Type for scalar values
	typedef GLColor<Scalar,4> Color; // Type for colors used in light sources
	typedef GLVector<Scalar,4> Position; // Type for light source positions
	typedef GLVector<Scalar,3> SpotDirection; // Type for light source spotlight directions
	
	/* Elements: */
	Color ambient; // Ambient color component
	Color diffuse; // Diffuse color component
	Color specular; // Specular color component
	Position position; // Light source position
	SpotDirection spotDirection; // Spot light direction
	Scalar spotCutoff; // Spot light cutoff angle
	Scalar spotExponent; // Spot light attenuation exponent
	Scalar constantAttenuation; // Point light constant attenuation coefficient
	Scalar linearAttenuation; // Point light linear attenuation coefficient
	Scalar quadraticAttenuation; // Point light quadratic attenuation coefficient
	
	/* Constructors and destructors: */
	GLLight(void); // Constructs default light source
	GLLight(const Color& sColor,
	        const Position& sPosition,
	        Scalar sConstantAttenuation =Scalar(1),
	        Scalar sLinearAttenuation =Scalar(0),
	        Scalar sQuadraticAttenuation =Scalar(0)); // Sets a monochromatic point or directional light source
	GLLight(const Color& sColor,
	        const Position& sPosition,
	        const SpotDirection& sSpotDirection,
	        Scalar sSpotCutoff,
	        Scalar sSpotExponent =Scalar(0),
	        Scalar sConstantAttenuation =Scalar(1),
	        Scalar sLinearAttenuation =Scalar(0),
	        Scalar sQuadraticAttenuation =Scalar(0)); // Sets a monochromatic spot light source
	GLLight(const Color& sAmbient,
	        const Color& sDiffuse,
	        const Color& sSpecular,
	        const Position& sPosition,
	        const SpotDirection& sSpotDirection,
	        Scalar sSpotCutoff,
	        Scalar sSpotExponent =Scalar(0),
	        Scalar sConstantAttenuation =Scalar(1),
	        Scalar sLinearAttenuation =Scalar(0),
	        Scalar sQuadraticAttenuation =Scalar(0)); // Full initialization
	
	/* Methods: */
	friend void glLight(GLsizei lightIndex,const GLLight& light); // Sets properties for the given light source index
	friend void glGetLight(GLsizei lightIndex,GLLight& light); // Stores current properties of the given light source index in the passed object
	friend GLLight glGetLight(GLsizei lightIndex); // Returns current properties of the given light source index
	};

#endif
