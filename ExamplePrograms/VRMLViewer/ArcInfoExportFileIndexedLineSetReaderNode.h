/***********************************************************************
ArcInfoExportFileIndexedLineSetReaderNode - Class for nodes that read
indexed line set data from external files in e00 Arc/Info export format.
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

#ifndef ARCINFOEXPORTFILEINDEXEDLINESETREADERNODE_INCLUDED
#define ARCINFOEXPORTFILEINDEXEDLINESETREADERNODE_INCLUDED

#include "Types.h"

#include "IndexedLineSetReaderNode.h"

class ArcInfoExportFileIndexedLineSetReaderNode:public IndexedLineSetReaderNode
	{
	/* Elements: */
	private:
	String url; // URL of the external Arc/Info export file
	VRMLNodePointer ellipsoid; // The ellipsoid used to convert spherical to Cartesian coordinates
	Bool radians; // Flag whether point set file contains latitude and longitude in radians
	Bool colatitude; // Flag whether point set file contains colatitude instead of latitude
	Bool depth; // Flag whether point set file contains negative elevation, i.e., depth
	double radialScale; // Scale factor from radial coordinate units to meters
	
	/* Constructors and destructors: */
	public:
	ArcInfoExportFileIndexedLineSetReaderNode(VRMLParser& parser); // Creates Arc/Info indexed line set reader by parsing VRML file
	
	/* Methods: */
	virtual bool hasColors(void) const;
	virtual void readIndexedLines(CoordinateNode* coordNode,std::vector<Int32>& coordIndices,ColorNode* colorNode,std::vector<Int32>& colorIndices) const;
	};

#endif
