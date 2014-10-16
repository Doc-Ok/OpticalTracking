/***********************************************************************
NodeFactory - Generic class for factories that create scene graph nodes
of particular types.
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

#ifndef SCENEGRAPH_NODEFACTORY_INCLUDED
#define SCENEGRAPH_NODEFACTORY_INCLUDED

/* Forward declarations: */
namespace SceneGraph {
class Node;
}

namespace SceneGraph {

class NodeFactory // Factory base class
	{
	/* Constructors and destructors: */
	public:
	virtual ~NodeFactory(void)
		{
		}
	
	/* Methods: */
	virtual const char* getClassName(void) const =0; // Returns the name of the created node class
	virtual Node* createNode(void) =0; // Creates a new node
	};

template <class NodeParam>
class GenericNodeFactory:public NodeFactory // Generic factory class
	{
	/* Embedded classes: */
	public:
	typedef NodeParam Node; // Type of created nodes
	
	/* Methods from NodeFactory: */
	virtual const char* getClassName(void) const
		{
		return Node::getStaticClassName();
		}
	virtual Node* createNode(void)
		{
		return new Node;
		}
	};

}

#endif
