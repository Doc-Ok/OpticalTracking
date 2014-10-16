/***********************************************************************
InlineNode - Class for VRML nodes referring to an external VRML file.
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

#include <stdlib.h>
#include <string>
#include <Misc/ThrowStdErr.h>

#include "Types.h"
#include "Fields/SFVec3f.h"

#include "VRMLParser.h"

#include "InlineNode.h"

/***************************
Methods of class InlineNode:
***************************/

InlineNode::InlineNode(VRMLParser& parser)
	{
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("InlineNode::InlineNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	Vec3f bboxCenter(0.0f,0.0f,0.0f);
	Vec3f bboxSize(-1.0f,-1.0f,-1.0f);
	while(!parser.isToken("}"))
		{
		if(parser.isToken("bboxCenter"))
			{
			parser.getNextToken();
			bboxCenter=SFVec3f::parse(parser);
			}
		else if(parser.isToken("bboxSize"))
			{
			parser.getNextToken();
			bboxSize=SFVec3f::parse(parser);
			}
		else if(parser.isToken("url"))
			{
			parser.getNextToken();
			
			/* Load the external VRML file: */
			VRMLParser externalParser(parser.getFullUrl(parser.getToken()).c_str());
			
			/* Read nodes from the external VRML file until end-of-file: */
			while(!externalParser.eof())
				{
				/* Read the next node and add it to the group: */
				addChild(externalParser.getNextNode());
				}
			
			parser.getNextToken();
			}
		else
			Misc::throwStdErr("InlineNode::InlineNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	
	/* Construct the explicit bounding box: */
	setBoundingBox(bboxCenter,bboxSize);
	}
