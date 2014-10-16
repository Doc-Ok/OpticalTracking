/***********************************************************************
Doom3DataContextNode - Class for nodes representing a collection of pak
files, textures, and materials to render Doom3 static and animated
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

#ifndef SCENEGRAPH_DOOM3DATACONTEXTNODE_INCLUDED
#define SCENEGRAPH_DOOM3DATACONTEXTNODE_INCLUDED

#include <Misc/Autopointer.h>
#include <SceneGraph/FieldTypes.h>
#include <SceneGraph/Node.h>

/* Forward declarations: */
namespace Cluster {
class Multiplexer;
}
namespace SceneGraph {
class Doom3FileManager;
class Doom3TextureManager;
class Doom3MaterialManager;
}

namespace SceneGraph {

class Doom3DataContextNode:public Node
	{
	/* Fields: */
	public:
	MFString baseUrl;
	SFString pakFilePrefix;
	
	/* Derived state: */
	protected:
	Cluster::Multiplexer* multiplexer; // Multiplexer to read resource files in a cluster environment
	Doom3FileManager* fileManager; // Manager to load pak and other resource files
	Doom3TextureManager* textureManager; // Manager to load and share textures
	Doom3MaterialManager* materialManager; // Manager to load and share material properties
	
	/* Constructors and destructors: */
	public:
	Doom3DataContextNode(void); // Creates an uninitialized data context node
	virtual ~Doom3DataContextNode(void); // Destroys the data context node
	
	/* Methods from Node: */
	static const char* getStaticClassName(void);
	virtual const char* getClassName(void) const;
	virtual void parseField(const char* fieldName,VRMLFile& vrmlFile);
	virtual void update(void);
	
	/* New methods: */
	Doom3FileManager* getFileManager(void) const // Returns the file manager
		{
		return fileManager;
		}
	Doom3TextureManager* getTextureManager(void) const // Returns the texture manager
		{
		return textureManager;
		}
	Doom3MaterialManager* getMaterialManager(void) const // Returns the material manager
		{
		return materialManager;
		}
	};

typedef Misc::Autopointer<Doom3DataContextNode> Doom3DataContextNodePointer;

}

#endif
