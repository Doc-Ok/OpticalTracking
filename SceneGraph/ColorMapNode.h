/***********************************************************************
ColorMapNode - Class for nodes that define mappings from scalar values
to RGB colors.
Copyright (c) 2012 Oliver Kreylos

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

#ifndef SCENEGRAPH_COLORMAPNODE_INCLUDED
#define SCENEGRAPH_COLORMAPNODE_INCLUDED

#include <Misc/Autopointer.h>
#include <SceneGraph/FieldTypes.h>
#include <SceneGraph/Node.h>

namespace SceneGraph {

class ColorMapNode:public Node
	{
	/* Elements: */
	
	/* Fields: */
	public:
	MFFloat scalar;
	MFColor color;
	
	/* Derived state: */
	size_t numMapEntries; // Number of color map entries (smaller of number of scalars and number of colors)
	
	/* Constructors and destructors: */
	public:
	ColorMapNode(void); // Creates color map node with empty color map
	
	/* Methods from Node: */
	static const char* getStaticClassName(void);
	virtual const char* getClassName(void) const;
	virtual EventOut* getEventOut(const char* fieldName) const;
	virtual EventIn* getEventIn(const char* fieldName);
	virtual void parseField(const char* fieldName,VRMLFile& vrmlFile);
	virtual void update(void);
	
	/* New methods: */
	public:
	Color mapColor(Scalar mapScalar) const; // Returns the color value corresponding to the given scalar value
	};

typedef Misc::Autopointer<ColorMapNode> ColorMapNodePointer;

}

#endif
