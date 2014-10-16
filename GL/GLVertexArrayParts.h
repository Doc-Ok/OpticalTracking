/***********************************************************************
GLVertexArrayParts - Helper class to enable/disable individual parts of
the OpenGL vertex array client state.
Copyright (c) 2005 Oliver Kreylos

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

#ifndef GLVERTEXARRAYPARTS_INCLUDED
#define GLVERTEXARRAYPARTS_INCLUDED

#include <GL/gl.h>

class GLVertexArrayParts
	{
	/* Embedded classes: */
	public:
	enum Parts // Enumerated type for vertex array parts
		{
		No=0x0,
		Position=0x1,
		Normal=0x2,
		Color=0x4,
		TexCoord=0x8,
		EdgeFlag=0x10,
		Index=0x20,
		All=0x3f
		};
	
	/* Methods: */
	static void enable(int vertexArrayPartsMask)
		{
		if(vertexArrayPartsMask&Position)
			glEnableClientState(GL_VERTEX_ARRAY);
		if(vertexArrayPartsMask&Normal)
			glEnableClientState(GL_NORMAL_ARRAY);
		if(vertexArrayPartsMask&Color)
			glEnableClientState(GL_COLOR_ARRAY);
		if(vertexArrayPartsMask&TexCoord)
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		if(vertexArrayPartsMask&EdgeFlag)
			glEnableClientState(GL_EDGE_FLAG_ARRAY);
		if(vertexArrayPartsMask&Index)
			glEnableClientState(GL_INDEX_ARRAY);
		}
	static void disable(int vertexArrayPartsMask)
		{
		if(vertexArrayPartsMask&Position)
			glDisableClientState(GL_VERTEX_ARRAY);
		if(vertexArrayPartsMask&Normal)
			glDisableClientState(GL_NORMAL_ARRAY);
		if(vertexArrayPartsMask&Color)
			glDisableClientState(GL_COLOR_ARRAY);
		if(vertexArrayPartsMask&TexCoord)
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		if(vertexArrayPartsMask&EdgeFlag)
			glDisableClientState(GL_EDGE_FLAG_ARRAY);
		if(vertexArrayPartsMask&Index)
			glDisableClientState(GL_INDEX_ARRAY);
		}
	};

#endif
