/***********************************************************************
TextureCoordinateNode - Class for arrays of vertex texture coordinates.
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

#ifndef TEXTURECOORDINATENODE_INCLUDED
#define TEXTURECOORDINATENODE_INCLUDED

#include <vector>
#include <Geometry/Point.h>

#include "VRMLNode.h"

class TextureCoordinateNode:public VRMLNode
	{
	/* Embedded classes: */
	public:
	typedef Geometry::Point<float,2> Point; // Type for points
	typedef std::vector<Point> PointList; // Type for lists of points
	
	/* Elements: */
	private:
	PointList points; // The point array
	
	/* Constructors and destructors: */
	public:
	TextureCoordinateNode(void); // Creates empty texture coordinate node, to be filled in later
	TextureCoordinateNode(VRMLParser& parser); // Creates texture coordinate node by parsing VRML file
	
	/* Methods: */
	virtual void glRenderAction(VRMLRenderState& renderState) const;
	PointList& getPoints(void) // Returns the array of points
		{
		return points;
		};
	size_t getNumPoints(void) const // Returns the number of points in the array
		{
		return points.size();
		};
	const Point& getPoint(int index) const // Returns the index-th point in the array
		{
		return points[index];
		};
	};

#endif
