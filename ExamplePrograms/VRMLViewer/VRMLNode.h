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

#ifndef VRMLNODE_INCLUDED
#define VRMLNODE_INCLUDED

#include <Misc/RefCounted.h>
#include <Misc/Autopointer.h>
#include <Geometry/Box.h>

/* Forward declarations: */
class VRMLParser;
class VRMLRenderState;

class VRMLNode:public Misc::RefCounted
	{
	/* Embedded classes: */
	public:
	typedef Geometry::Box<float,3> Box; // Type for axis-aligned bounding boxes
	
	/* Constructors and destructors: */
	public:
	VRMLNode(void) // Does not initialize node
		{
		}
	virtual ~VRMLNode(void);
	
	/* Methods: */
	virtual Box calcBoundingBox(void) const; // Returns the bounding box of the node
	virtual void glRenderAction(VRMLRenderState& renderState) const; // Renders the node into the current OpenGL context
	};

typedef Misc::Autopointer<VRMLNode> VRMLNodePointer;

#endif
