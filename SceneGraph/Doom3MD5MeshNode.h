/***********************************************************************
Doom3MD5MeshNode - Class for nodes to render Doom3 MD5Mesh animated
models.
Copyright (c) 2010 Oliver Kreylos

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

#ifndef SCENEGRAPH_DOOM3MD5MESHNODE_INCLUDED
#define SCENEGRAPH_DOOM3MD5MESHNODE_INCLUDED

#include <SceneGraph/FieldTypes.h>
#include <SceneGraph/GraphNode.h>
#include <SceneGraph/Doom3DataContextNode.h>

/* Forward declarations: */
namespace SceneGraph {
class Doom3MD5Mesh;
}

namespace SceneGraph {

class Doom3MD5MeshNode:public GraphNode
	{
	/* Embedded classes: */
	public:
	typedef SF<Doom3DataContextNodePointer> SFDoom3DataContextNode;
	
	/* Fields: */
	public:
	SFDoom3DataContextNode dataContext;
	SFString model;
	
	/* Derived state: */
	protected:
	Doom3MD5Mesh* mesh; // Pointer to the mesh object
	
	/* Constructors and destructors: */
	public:
	Doom3MD5MeshNode(void);
	virtual ~Doom3MD5MeshNode(void);
	
	/* Methods from Node: */
	static const char* getStaticClassName(void);
	virtual const char* getClassName(void) const;
	virtual void parseField(const char* fieldName,VRMLFile& vrmlFile);
	virtual void update(void);
	
	/* Methods from GraphNode: */
	virtual Box calcBoundingBox(void) const;
	virtual void glRenderAction(GLRenderState& renderState) const;
	};

}

#endif
