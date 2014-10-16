/***********************************************************************
PointSetReaderNode - Base class for nodes that read point set data from
external files.
Copyright (c) 2008 Oliver Kreylos

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

#ifndef POINTSETREADERNODE_INCLUDED
#define POINTSETREADERNODE_INCLUDED

#include "VRMLNode.h"

/* Forward declarations: */
class CoordinateNode;
class ColorNode;

class PointSetReaderNode:public VRMLNode
	{
	/* New methods: */
	public:
	virtual bool hasColors(void) const =0; // Returns true if a point set reader can generate per-point colors
	virtual void readPoints(CoordinateNode* coordNode,ColorNode* colorNode) const =0; // Fills a coordinate and color nodes by reading an external point file
	};

#endif
