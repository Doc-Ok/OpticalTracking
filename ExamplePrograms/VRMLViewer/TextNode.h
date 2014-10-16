/***********************************************************************
TextNode - Node class for text shapes, rendered as lines of texture-
mapped thin quads.
Copyright (c) 2008 Oliver Kreylos

This file is part of the Virtual Reality VRML viewer (VRMLViewer).

The Virtual Reality VRML viewer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Virtual Reality VRML viewer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality VRML viewer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef TEXTNODE_INCLUDED
#define TEXTNODE_INCLUDED

#include <vector>
#include <GL/gl.h>
#include <GL/GLFont.h>

#include "Types.h"

#include "GeometryNode.h"

class TextNode:public GeometryNode
	{
	/* Embedded classes: */
	private:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLsizei numTextures; // Number of texture objects representing the strings
		GLuint* textureObjectIds; // Array of IDs of texture objects representing the strings
		
		/* Constructors and destructors: */
		DataItem(int sNumTextures);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	std::vector<String> string; // Array of text strings
	VRMLNodePointer fontStyle; // Pointer to the font style to use for this text node
	std::vector<Float> length; // Array of lengths to which to scale the text strings
	Float maxExtent; // Maximum allowed width for any text string; font is scaled if limit is exceeded
	std::vector<GLFont::Box> stringBox; // Array of model-space positions and sizes of the quads used to render the strings
	std::vector<GLFont::TBox> stringTexBox; // Array of texture-space boxes to map the strings to the quads
	Box boundingBox; // Bounding box around all strings
	
	/* Constructors and destructors: */
	public:
	TextNode(VRMLParser& parser);
	
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const;
	virtual VRMLNode::Box calcBoundingBox(void) const;
	virtual void glRenderAction(VRMLRenderState& renderState) const;
	};

#endif
