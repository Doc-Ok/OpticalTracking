/***********************************************************************
EllipsoidNode - Class to represent different ellipsoid shapes. These are
not meant for rendering, but to convert spherical coordinates into
geocentric Cartesian coordinates, also known as GPS coordinates.
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

#include <stdlib.h>

#include"VRMLParser.h"

#include "EllipsoidNode.h"

/******************************
Methods of class EllipsoidNode:
******************************/

EllipsoidNode::EllipsoidNode(VRMLParser& parser)
	:radius(6378137.0),
	 flatteningFactor(1.0/298.257223563),
	 scaleFactor(1.0e-3)
	{
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("EllipsoidNode::EllipsoidNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	while(!parser.isToken("}"))
		{
		if(parser.isToken("radius"))
			{
			/* Parse the ellipsoid radius: */
			parser.getNextToken();
			radius=atof(parser.getToken());
			parser.getNextToken();
			}
		else if(parser.isToken("flatteningFactor"))
			{
			/* Parse the flattening factor: */
			parser.getNextToken();
			flatteningFactor=atof(parser.getToken());
			parser.getNextToken();
			}
		else if(parser.isToken("inverseFlatteningFactor"))
			{
			/* Parse the inverse flattening factor: */
			parser.getNextToken();
			flatteningFactor=1.0/atof(parser.getToken());
			parser.getNextToken();
			}
		else if(parser.isToken("scaleFactor"))
			{
			/* Parse the scale factor: */
			parser.getNextToken();
			scaleFactor=atof(parser.getToken());
			parser.getNextToken();
			}
		else
			Misc::throwStdErr("EllipsoidNode::EllipsoidNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	
	/* Calculate derived values: */
	modelRadius=radius*scaleFactor;
	e2=(2.0-flatteningFactor)*flatteningFactor;
	}
