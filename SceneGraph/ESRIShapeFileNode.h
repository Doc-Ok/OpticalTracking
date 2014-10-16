/***********************************************************************
ESRIShapeFileNode - Class to represent an ESRI shape file as a
collection of line sets, point sets, or face sets (each shape file can
only contain a single type of primitives).
Copyright (c) 2009-2011 Oliver Kreylos

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

#ifndef SCENEGRAPH_ESRISHAPEFILENODE_INCLUDED
#define SCENEGRAPH_ESRISHAPEFILENODE_INCLUDED

#include <SceneGraph/FieldTypes.h>
#include <SceneGraph/GroupNode.h>
#include <SceneGraph/AppearanceNode.h>
#include <SceneGraph/FontStyleNode.h>

/* Forward declarations: */
namespace Cluster {
class Multiplexer;
}

namespace SceneGraph {

class ESRIShapeFileNode:public GroupNode
	{
	/* Embedded classes: */
	public:
	typedef SF<AppearanceNodePointer> SFAppearanceNode;
	typedef SF<FontStyleNodePointer> SFFontStyleNode;
	
	/* Elements: */
	
	/* Fields: */
	public:
	MFString url; // URL for the ESRI shape file to load
	SFAppearanceNode appearance; // Node defining appearance of geometry loaded from shape file
	SFString labelField; // Name of attribute table field to be used to label shape file records
	SFFontStyleNode fontStyle; // Font style for shape file record labels
	SFBool transformToCartesian; // Flag whether to use the projection defined in the shape file to transform all geometry to Cartesian coordinates
	SFFloat pointSize;
	SFFloat lineWidth;
	
	/* Derived state: */
	protected:
	Cluster::Multiplexer* multiplexer; // Multiplexer to read shape files in a cluster environment
	
	/* Constructors and destructors: */
	public:
	ESRIShapeFileNode(void); // Creates an uninitialized ESRI shape file file node
	
	/* Methods from Node: */
	static const char* getStaticClassName(void);
	virtual const char* getClassName(void) const;
	virtual void parseField(const char* fieldName,VRMLFile& vrmlFile);
	virtual void update(void);
	};

}

#endif
