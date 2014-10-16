/***********************************************************************
AttributeNode - Base class for nodes that define attributes used to
render geometry.
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

#ifndef SCENEGRAPH_ATTRIBUTENODE_INCLUDED
#define SCENEGRAPH_ATTRIBUTENODE_INCLUDED

#include <Misc/Autopointer.h>
#include <SceneGraph/Node.h>

/* Forward declarations: */
namespace SceneGraph {
class GLRenderState;
}

namespace SceneGraph {

class AttributeNode:public Node
	{
	/* New methods: */
	public:
	virtual void setGLState(GLRenderState& renderState) const =0; // Sets OpenGL state for rendering
	virtual void resetGLState(GLRenderState& renderState) const =0; // Resets OpenGL state after rendering
	};

typedef Misc::Autopointer<AttributeNode> AttributeNodePointer;

}

#endif
