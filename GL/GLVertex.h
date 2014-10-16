/***********************************************************************
GLVertex - Class to encapsulate OpenGL vertex properties.
Copyright (c) 2004-2010 Oliver Kreylos

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

#ifndef GLVERTEX_INCLUDED
#define GLVERTEX_INCLUDED

#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLVector.h>
#include <GL/GLVertexArrayParts.h>

template <class TexCoordScalarParam,GLsizei numTexCoordComponentsParam,
          class ColorScalarParam,GLsizei numColorComponentsParam,
          class NormalScalarParam,
          class PositionScalarParam,GLsizei numPositionComponentsParam>
struct GLVertex
	{
	/* Embedded classes: */
	public:
	typedef GLVector<TexCoordScalarParam,numTexCoordComponentsParam> TexCoord;
	typedef GLColor<ColorScalarParam,numColorComponentsParam> Color;
	typedef GLVector<NormalScalarParam,3> Normal;
	typedef GLVector<PositionScalarParam,numPositionComponentsParam> Position;
	
	/* Elements: */
	TexCoord texCoord; // Vertex texture coordinates
	Color color; // Vertex color
	Normal normal; // Vertex normal
	Position position; // Vertex position
	
	/* Constructors and destructors: */
	GLVertex(void)
		{
		}
	GLVertex(const TexCoord& sTexCoord,
	         const Color& sColor,
	         const Normal& sNormal,
	         const Position& sPosition)
		:texCoord(sTexCoord),
		 color(sColor),
		 normal(sNormal),
		 position(sPosition)
		{
		}
	
	/* Methods: */
	static int getPartsMask(void)
		{
		return GLVertexArrayParts::Position|GLVertexArrayParts::Normal|GLVertexArrayParts::Color|GLVertexArrayParts::TexCoord;
		}
	};

template <class TexCoordScalarParam,GLsizei numTexCoordComponentsParam,
          class NormalScalarParam,
          class PositionScalarParam,GLsizei numPositionComponentsParam>
struct GLVertex<TexCoordScalarParam,numTexCoordComponentsParam,
                void,0,
                NormalScalarParam,
                PositionScalarParam,numPositionComponentsParam>
	{
	/* Embedded classes: */
	public:
	typedef GLVector<TexCoordScalarParam,numTexCoordComponentsParam> TexCoord;
	typedef GLVector<NormalScalarParam,3> Normal;
	typedef GLVector<PositionScalarParam,numPositionComponentsParam> Position;
	
	/* Elements: */
	TexCoord texCoord; // Vertex texture coordinates
	Normal normal; // Vertex normal
	Position position; // Vertex position
	
	/* Constructors and destructors: */
	GLVertex(void)
		{
		}
	GLVertex(const TexCoord& sTexCoord,
	         const Normal& sNormal,
	         const Position& sPosition)
		:texCoord(sTexCoord),
		 normal(sNormal),
		 position(sPosition)
		{
		}
	
	/* Methods: */
	static int getPartsMask(void)
		{
		return GLVertexArrayParts::Position|GLVertexArrayParts::Normal|GLVertexArrayParts::TexCoord;
		}
	};

template <class TexCoordScalarParam,GLsizei numTexCoordComponentsParam,
          class ColorScalarParam,GLsizei numColorComponentsParam,
          class PositionScalarParam,GLsizei numPositionComponentsParam>
struct GLVertex<TexCoordScalarParam,numTexCoordComponentsParam,
                ColorScalarParam,numColorComponentsParam,
                void,
                PositionScalarParam,numPositionComponentsParam>
	{
	/* Embedded classes: */
	public:
	typedef GLVector<TexCoordScalarParam,numTexCoordComponentsParam> TexCoord;
	typedef GLColor<ColorScalarParam,numColorComponentsParam> Color;
	typedef GLVector<PositionScalarParam,numPositionComponentsParam> Position;
	
	/* Elements: */
	TexCoord texCoord; // Vertex texture coordinates
	Color color; // Vertex color
	Position position; // Vertex position
	
	/* Constructors and destructors: */
	GLVertex(void)
		{
		}
	GLVertex(const TexCoord& sTexCoord,
	         const Color& sColor,
	         const Position& sPosition)
		:texCoord(sTexCoord),
		 color(sColor),
		 position(sPosition)
		{
		}
	
	/* Methods: */
	static int getPartsMask(void)
		{
		return GLVertexArrayParts::Position|GLVertexArrayParts::Color|GLVertexArrayParts::TexCoord;
		}
	};

template <class ColorScalarParam,GLsizei numColorComponentsParam,
          class NormalScalarParam,
          class PositionScalarParam,GLsizei numPositionComponentsParam>
struct GLVertex<void,0,
                ColorScalarParam,numColorComponentsParam,
                NormalScalarParam,
                PositionScalarParam,numPositionComponentsParam>
	{
	/* Embedded classes: */
	public:
	typedef GLColor<ColorScalarParam,numColorComponentsParam> Color;
	typedef GLVector<NormalScalarParam,3> Normal;
	typedef GLVector<PositionScalarParam,numPositionComponentsParam> Position;
	
	/* Elements: */
	Color color; // Vertex color
	Normal normal; // Vertex normal
	Position position; // Vertex position
	
	/* Constructors and destructors: */
	GLVertex(void)
		{
		}
	GLVertex(const Color& sColor,
	         const Normal& sNormal,
	         const Position& sPosition)
		:color(sColor),
		 normal(sNormal),
		 position(sPosition)
		{
		}
	
	/* Methods: */
	static int getPartsMask(void)
		{
		return GLVertexArrayParts::Position|GLVertexArrayParts::Normal|GLVertexArrayParts::Color;
		}
	};

template <class TexCoordScalarParam,GLsizei numTexCoordComponentsParam,
          class PositionScalarParam,GLsizei numPositionComponentsParam>
struct GLVertex<TexCoordScalarParam,numTexCoordComponentsParam,
                void,0,
                void,
                PositionScalarParam,numPositionComponentsParam>
	{
	/* Embedded classes: */
	public:
	typedef GLVector<TexCoordScalarParam,numTexCoordComponentsParam> TexCoord;
	typedef GLVector<PositionScalarParam,numPositionComponentsParam> Position;
	
	/* Elements: */
	TexCoord texCoord; // Vertex texture coordinates
	Position position; // Vertex position
	
	/* Constructors and destructors: */
	GLVertex(void)
		{
		}
	GLVertex(const TexCoord& sTexCoord,
	         const Position& sPosition)
		:texCoord(sTexCoord),
		 position(sPosition)
		{
		}
	
	/* Methods: */
	static int getPartsMask(void)
		{
		return GLVertexArrayParts::Position|GLVertexArrayParts::TexCoord;
		}
	};

template <class NormalScalarParam,
          class PositionScalarParam,GLsizei numPositionComponentsParam>
struct GLVertex<void,0,
                void,0,
                NormalScalarParam,
                PositionScalarParam,numPositionComponentsParam>
	{
	/* Embedded classes: */
	public:
	typedef GLVector<NormalScalarParam,3> Normal;
	typedef GLVector<PositionScalarParam,numPositionComponentsParam> Position;
	
	/* Elements: */
	Normal normal; // Vertex normal
	Position position; // Vertex position
	
	/* Constructors and destructors: */
	GLVertex(void)
		{
		}
	GLVertex(const Normal& sNormal,
	         const Position& sPosition)
		:normal(sNormal),
		 position(sPosition)
		{
		}
	
	/* Methods: */
	static int getPartsMask(void)
		{
		return GLVertexArrayParts::Position|GLVertexArrayParts::Normal;
		}
	};

template <class ColorScalarParam,GLsizei numColorComponentsParam,
          class PositionScalarParam,GLsizei numPositionComponentsParam>
struct GLVertex<void,0,
                ColorScalarParam,numColorComponentsParam,
                void,
                PositionScalarParam,numPositionComponentsParam>
	{
	/* Embedded classes: */
	public:
	typedef GLColor<ColorScalarParam,numColorComponentsParam> Color;
	typedef GLVector<PositionScalarParam,numPositionComponentsParam> Position;
	
	/* Elements: */
	Color color; // Vertex color
	Position position; // Vertex position
	
	/* Constructors and destructors: */
	GLVertex(void)
		{
		}
	GLVertex(const Color& sColor,
	         const Position& sPosition)
		:color(sColor),
		 position(sPosition)
		{
		}
	
	/* Methods: */
	static int getPartsMask(void)
		{
		return GLVertexArrayParts::Position|GLVertexArrayParts::Color;
		}
	};

template <class PositionScalarParam,GLsizei numPositionComponentsParam>
struct GLVertex<void,0,
                void,0,
                void,
                PositionScalarParam,numPositionComponentsParam>
	{
	/* Embedded classes: */
	public:
	typedef GLVector<PositionScalarParam,numPositionComponentsParam> Position;
	
	/* Elements: */
	Position position; // Vertex position
	
	/* Constructors and destructors: */
	GLVertex(void)
		{
		}
	GLVertex(const Position& sPosition)
		:position(sPosition)
		{
		}
	
	/* Methods: */
	static int getPartsMask(void)
		{
		return GLVertexArrayParts::Position;
		}
	};

/***********************************
Overloaded version of glVertex call:
***********************************/

template <class TexCoordScalarParam,GLsizei numTexCoordComponentsParam,
          class ColorScalarParam,GLsizei numColorComponentsParam,
          class NormalScalarParam,
          class PositionScalarParam,GLsizei numPositionComponentsParam>
void glVertex(const GLVertex<TexCoordScalarParam,numTexCoordComponentsParam,
                             ColorScalarParam,numColorComponentsParam,
                             NormalScalarParam,
                             PositionScalarParam,numPositionComponentsParam>& vertex);

/******************************************
Overloaded version of glVertexPointer call:
******************************************/

template <class TexCoordScalarParam,GLsizei numTexCoordComponentsParam,
          class ColorScalarParam,GLsizei numColorComponentsParam,
          class NormalScalarParam,
          class PositionScalarParam,GLsizei numPositionComponentsParam>
void glVertexPointer(const GLVertex<TexCoordScalarParam,numTexCoordComponentsParam,
                                    ColorScalarParam,numColorComponentsParam,
                                    NormalScalarParam,
                                    PositionScalarParam,numPositionComponentsParam>* vertexPointer);

template <class TexCoordScalarParam,GLsizei numTexCoordComponentsParam,
          class ColorScalarParam,GLsizei numColorComponentsParam,
          class NormalScalarParam,
          class PositionScalarParam,GLsizei numPositionComponentsParam>
void glVertexPointer(int vertexPartsMask,
                     const GLVertex<TexCoordScalarParam,numTexCoordComponentsParam,
                                    ColorScalarParam,numColorComponentsParam,
                                    NormalScalarParam,
                                    PositionScalarParam,numPositionComponentsParam>* vertexPointer);

#if defined(GLVERTEX_NONSTANDARD_TEMPLATES) && !defined(GLVERTEX_IMPLEMENTATION)
#include <GL/GLVertex.icpp>
#endif

#endif
