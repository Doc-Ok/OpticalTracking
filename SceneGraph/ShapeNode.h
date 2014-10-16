/***********************************************************************
ShapeNode - Class for shapes represented as a combination of a geometry
node and an appearance node defining the geometry's appearance.
Copyright (c) 2009 Oliver Kreylos

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

#ifndef SCENEGRAPH_SHAPENODE_INCLUDED
#define SCENEGRAPH_SHAPENODE_INCLUDED

#include <SceneGraph/FieldTypes.h>
#include <SceneGraph/GraphNode.h>
#include <SceneGraph/AppearanceNode.h>
#include <SceneGraph/GeometryNode.h>

namespace SceneGraph {

class ShapeNode:public GraphNode
	{
	/* Embedded classes: */
	public:
	typedef SF<AppearanceNodePointer> SFAppearanceNode;
	typedef SF<GeometryNodePointer> SFGeometryNode;
	
	/* Elements: */
	
	/* Fields: */
	public:
	SFAppearanceNode appearance; // The shape's appearance
	SFGeometryNode geometry; // The shape's geometry
	
	/* Constructors and destructors: */
	public:
	ShapeNode(void); // Creates a shape node with default appearance and no geometry
	
	/* Methods from Node: */
	static const char* getStaticClassName(void);
	virtual const char* getClassName(void) const;
	virtual void parseField(const char* fieldName,VRMLFile& vrmlFile);
	virtual void update(void);
	
	/* Methods from GraphNode: */
	virtual Box calcBoundingBox(void) const;
	virtual void glRenderAction(GLRenderState& renderState) const;
	};

typedef Misc::Autopointer<ShapeNode> ShapeNodePointer;

}

#endif
