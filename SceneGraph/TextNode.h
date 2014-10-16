/***********************************************************************
TextNode - Class for nodes to render 3D text.
Copyright (c) 2009-2013 Oliver Kreylos

This file is part of the Simple Scene Graph Renderer (SceneGraph).

The Simple Scene Graph Renderer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Simple Scene Graph Renderer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Simple Scene Graph Renderer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef SCENEGRAPH_TEXTNODE_INCLUDED
#define SCENEGRAPH_TEXTNODE_INCLUDED

#include <vector>
#include <Geometry/Box.h>
#include <GL/gl.h>
#include <GL/GLFont.h>
#include <GL/GLObject.h>
#include <SceneGraph/FieldTypes.h>
#include <SceneGraph/GeometryNode.h>
#include <SceneGraph/FontStyleNode.h>

namespace SceneGraph {

class TextNode:public GeometryNode,public GLObject
	{
	/* Embedded classes: */
	public:
	typedef SF<FontStyleNodePointer> SFFontStyleNode;
	
	protected:
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
	
	/* Fields: */
	public:
	MFString string;
	SFFontStyleNode fontStyle;
	MFFloat length;
	SFFloat maxExtent;
	
	/* Derived elements: */
	protected:
	bool inited; // Flag whether GLObject::init() has already been called
	std::vector<GLFont::Box> stringBox; // Array of model-space positions and sizes of the quads used to render the strings
	std::vector<GLFont::TBox> stringTexBox; // Array of texture-space boxes to map the strings to the quads
	Box boundingBox; // Bounding box around all strings
	
	/* Constructors and destructors: */
	public:
	TextNode(void); // Creates a default text node
	
	/* Methods from Node: */
	static const char* getStaticClassName(void);
	virtual const char* getClassName(void) const;
	virtual void parseField(const char* fieldName,VRMLFile& vrmlFile);
	virtual void update(void);
	
	/* Methods from GeometryNode: */
	virtual Box calcBoundingBox(void) const;
	virtual void glRenderAction(GLRenderState& renderState) const;
	
	/* Methods from GLObject: */
	virtual void initContext(GLContextData& contextData) const;
	};

typedef Misc::Autopointer<TextNode> TextNodePointer;

}

#endif
