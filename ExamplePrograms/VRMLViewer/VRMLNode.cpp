/***********************************************************************
VRMLNode - Base class for nodes in VRML world files.
Copyright (c) 2000-2008 Oliver Kreylos

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

#include "VRMLNode.h"

/*************************
Methods of class VRMLNode:
*************************/

VRMLNode::~VRMLNode(void)
	{
	}

VRMLNode::Box VRMLNode::calcBoundingBox(void) const
	{
	return Box::empty;
	}

void VRMLNode::glRenderAction(VRMLRenderState& renderState) const
	{
	}
