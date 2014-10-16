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

#include "VRMLParser.h"

#include "TextureCoordinateNode.h"

/**************************************
Methods of class TextureCoordinateNode:
**************************************/

TextureCoordinateNode::TextureCoordinateNode(void)
	{
	}

TextureCoordinateNode::TextureCoordinateNode(VRMLParser& parser)
	{
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("TextureCoordinateNode::TextureCoordinateNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	while(!parser.isToken("}"))
		{
		if(parser.isToken("point"))
			{
			/* Parse the point array: */
			
			/* Check for the opening bracket: */
			parser.getNextToken();
			if(!parser.isToken("["))
				Misc::throwStdErr("TextureCoordinateNode::TextureCoordinateNode: Missing opening bracket in point attribute");
			parser.getNextToken();
			
			/* Parse points until closing bracket: */
			while(!parser.isToken("]"))
				{
				/* Parse the next point: */
				Point p=Point::origin;
				for(int i=0;i<2&&!parser.isToken("]");++i)
					{
					p[i]=Point::Scalar(atof(parser.getToken()));
					parser.getNextToken();
					}
				points.push_back(p);
				}
			
			/* Skip the closing bracket: */
			parser.getNextToken();
			}
		else
			Misc::throwStdErr("TextureCoordinateNode::TextureCoordinateNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	
	}

void TextureCoordinateNode::glRenderAction(VRMLRenderState& renderState) const
	{
	}
