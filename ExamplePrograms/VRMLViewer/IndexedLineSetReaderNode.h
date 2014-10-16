/***********************************************************************
IndexedLineSetReaderNode - Base class for nodes that read indexed line
set data from external files.
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

#ifndef INDEXEDLINESETREADERNODE_INCLUDED
#define INDEXEDLINESETREADERNODE_INCLUDED

#include <vector>

#include "Types.h"

#include "VRMLNode.h"

/* Forward declarations: */
class CoordinateNode;
class ColorNode;

class IndexedLineSetReaderNode:public VRMLNode
	{
	/* New methods: */
	public:
	virtual bool hasColors(void) const =0; // Returns true if an indexed line set reader can generate colors
	virtual void readIndexedLines(CoordinateNode* coordNode,std::vector<Int32>& coordIndices,ColorNode* colorNode,std::vector<Int32>& colorIndices) const =0; // Fills a coordinate and color nodes by reading an external indexed line set file
	};

#endif
