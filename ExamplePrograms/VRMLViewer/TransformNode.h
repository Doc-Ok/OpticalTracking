/***********************************************************************
TransformNode - Node class for VRML transformations.
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

#ifndef TRANSFORMNODE_INCLUDED
#define TRANSFORMNODE_INCLUDED

#include "Types.h"
#include "GroupNode.h"

class TransformNode:public GroupNode
	{
	/* Elements: */
	private:
	Transformation transform; // This node's transformation
	
	/* Constructors and destructors: */
	public:
	TransformNode(VRMLParser& parser); // Initializes the node from the given VRML parser
	
	/* Methods: */
	virtual VRMLNode::Box calcBoundingBox(void) const;
	virtual void glRenderAction(VRMLRenderState& renderState) const;
	};

#endif
