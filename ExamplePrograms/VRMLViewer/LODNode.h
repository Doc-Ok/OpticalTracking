/***********************************************************************
LODNode - Node class to render models at different levels of detail.
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

#ifndef LODNODE_INCLUDED
#define LODNODE_INCLUDED

#include <vector>
#include <Geometry/Point.h>

#include "Types.h"

#include "GroupNode.h"

class LODNode:public GroupNode
	{
	/* Embedded classes: */
	public:
	typedef Geometry::Point<float,3> Point; // Type for points
	typedef std::vector<Float> DistanceList; // Type for lists of distance values
	
	/* Elements: */
	private:
	Point center; // Center for distance calculation
	DistanceList distance2s; // List of squared level change distances
	
	/* Constructors and destructors: */
	public:
	LODNode(VRMLParser& parser); // Initializes the node from the given VRML parser
	
	/* Methods: */
	virtual VRMLNode::Box calcBoundingBox(void) const;
	virtual void glRenderAction(VRMLRenderState& renderState) const;
	};

#endif
