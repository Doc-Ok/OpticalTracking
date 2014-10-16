/***********************************************************************
ColorNode - Class for arrays of vertex colors.
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
#include "Types.h"
#include "Fields/MFColor.h"

#include "VRMLParser.h"

#include "ColorNode.h"

/**************************
Methods of class ColorNode:
**************************/

ColorNode::ColorNode(void)
	{
	}

ColorNode::ColorNode(VRMLParser& parser)
	{
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("ColorNode::ColorNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	while(!parser.isToken("}"))
		{
		if(parser.isToken("color"))
			{
			/* Parse the color array: */
			parser.getNextToken();
			colors=MFColor::parse(parser);
			}
		else
			Misc::throwStdErr("ColorNode::ColorNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	
	}

void ColorNode::glRenderAction(VRMLRenderState& renderState) const
	{
	}
