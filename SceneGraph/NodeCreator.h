/***********************************************************************
NodeCreator - Class to create node objects based on a node type name.
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

#ifndef SCENEGRAPH_NODECREATOR_INCLUDED
#define SCENEGRAPH_NODECREATOR_INCLUDED

#include <string>
#include <Misc/StringHashFunctions.h>
#include <Misc/HashTable.h>

/* Forward declarations: */
namespace SceneGraph {
class Node;
class NodeFactory;
}

namespace SceneGraph {

class NodeCreator
	{
	/* Embedded classes: */
	private:
	typedef Misc::HashTable<std::string,NodeFactory*> NodeFactoryMap; // Hash table type for node factory maps
	
	/* Elements: */
	NodeFactoryMap nodeFactoryMap; // Hash table mapping node type names to node factories
	
	/* Constructors and destructors: */
	public:
	NodeCreator(void); // Creates a node creator for all standard node types
	~NodeCreator(void); // Destroys the node creator
	
	/* Methods: */
	void registerNodeType(NodeFactory* nodeFactory); // Registers a node factory for nodes of the given type; node creator inherits factory object
	Node* createNode(const char* nodeTypeName); // Creates a new node of the given type
	};

}

#endif

