/***********************************************************************
ShapeNode - Class for shapes in VRML files.
Copyright (c) 2006-2008 Oliver Kreylos

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

#ifndef SHAPENODE_INCLUDED
#define SHAPENODE_INCLUDED

#include "VRMLNode.h"
#include "AttributeNode.h"
#include "GeometryNode.h"

class ShapeNode:public VRMLNode
	{
	/* Elements: */
	private:
	AttributeNodePointer appearance; // The node defining the shape's appearance
	GeometryNodePointer geometry; // The node defining the shape's geometry
	
	/* Constructors and destructors: */
	public:
	ShapeNode(VRMLParser& parser);
	virtual ~ShapeNode(void);
	
	/* Methods: */
	virtual VRMLNode::Box calcBoundingBox(void) const;
	virtual void glRenderAction(VRMLRenderState& renderState) const;
	};

#endif
