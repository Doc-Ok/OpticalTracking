/***********************************************************************
GLTextureObject - Base class for objects containing a single texture
image.
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

#ifndef GLTEXTUREIMAGE_INCLUDED
#define GLTEXTUREIMAGE_INCLUDED

#include <GL/gl.h>
#include <GL/GLObject.h>

class GLTextureObject:public GLObject
	{
	/* Embedded classes: */
	protected:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint textureObjectId; // ID of texture object
		unsigned int textureObjectVersion; // Version number of texture image in texture object
		
		/* Constructors and destructors: */
		DataItem(void) // Creates the texture object
			:textureObjectId(0),
			 textureObjectVersion(0)
			{
			/* Create the texture object: */
			glGenTextures(1,&textureObjectId);
			}
		virtual ~DataItem(void); // Destroys the texture object
		};
	
	/* Elements: */
	unsigned int textureVersion; // Version number of texture image
	
	/* Constructors and destructors: */
	public:
	GLTextureObject(void) // Creates an uninitialized texture object
		:textureVersion(0)
		{
		}
	};

#endif
