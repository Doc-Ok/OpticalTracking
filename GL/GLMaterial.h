/***********************************************************************
GLMaterial - Class to encapsulate OpenGL material properties.
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

#ifndef GLMATERIAL_INCLUDED
#define GLMATERIAL_INCLUDED

#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLMaterialEnums.h>

struct GLMaterial
	{
	/* Embedded classes: */
	public:
	typedef GLMaterialEnums::Face Face; // Enumerated type for material faces
	typedef GLfloat Scalar; // Type for scalar values
	typedef GLColor<Scalar,4> Color; // Type for colors used in materials
	
	/* Elements: */
	Color ambient; // Ambient color component
	Color diffuse; // Diffuse color component
	Color specular; // Specular color component
	Scalar shininess; // Specular lighting exponent
	Color emission; // Emissive color component
	
	/* Constructors and destructors: */
	GLMaterial(void); // Constructs default material
	GLMaterial(const Color& sAmbientDiffuse); // Constructs diffuse material
	GLMaterial(const Color& sAmbientDiffuse,
	           const Color& sSpecular,
	           Scalar sShininess); // Constructs specular material
	GLMaterial(const Color& sAmbient,
	           const Color& sDiffuse,
	           const Color& sSpecular,
	           Scalar sShininess); // Constructs specular material with separate ambient color
	GLMaterial(const Color& sAmbientDiffuse,
	           const Color& sSpecular,
	           Scalar sShininess,
	           const Color& sEmission); // Constructs specular and emissive material
	GLMaterial(const Color& sAmbient,
	           const Color& sDiffuse,
	           const Color& sSpecular,
	           Scalar sShininess,
	           const Color& sEmission); // Full initialization
	
	/* Methods: */
	friend void glMaterial(Face face,const GLMaterial& material); // Sets material properties for front- and/or backfaces
	friend void glGetMaterial(Face face,GLMaterial& material); // Stores current material properties for front- or backfaces in passed object
	friend GLMaterial glGetMaterial(Face face); // Returns current material properties for front- or backfaces
	};

#endif
