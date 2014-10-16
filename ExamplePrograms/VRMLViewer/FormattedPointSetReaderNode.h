/***********************************************************************
FormattedPointSetReaderNode - Point set reader class for formatted
(fixed-width) ASCII files.
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

#ifndef FORMATTEDPOINTSETREADERNODE_INCLUDED
#define FORMATTEDPOINTSETREADERNODE_INCLUDED

#include <vector>

#include "Types.h"

#include "PointSetReaderNode.h"

class FormattedPointSetReaderNode:public PointSetReaderNode
	{
	/* Elements: */
	private:
	String url; // URL of the external point file
	std::vector<Int32> columnStarts; // Array of (zero-based) column starting positions; automatically computed if not specified
	std::vector<Int32> columnWidths; // Array of column widths
	VRMLNodePointer ellipsoid; // The ellipsoid used to convert spherical to Cartesian coordinates
	VRMLNodePointer colorMap; // The color map to convert point values into colors
	Int32 columnIndices[4]; // Array of column indices containing point coordinates, in order lat, long, radius, and color mapping value
	Int32 numHeaderLines; // Number of header lines to skip
	
	/* Constructors and destructors: */
	public:
	FormattedPointSetReaderNode(VRMLParser& parser); // Creates formatted point set reader by parsing VRML file
	
	/* Methods: */
	virtual bool hasColors(void) const;
	virtual void readPoints(CoordinateNode* coordNode,ColorNode* colorNode) const;
	};

#endif
