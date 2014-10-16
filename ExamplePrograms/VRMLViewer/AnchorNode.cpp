/***********************************************************************
AnchorNode - Node class for anchors linking to external VRML worlds or
other data.
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

#include "Types.h"
#include "Fields/SFVec3f.h"
#include "Fields/SFString.h"
#include "Fields/MFString.h"

#include "VRMLParser.h"

#include "AnchorNode.h"

/***************************
Methods of class AnchorNode:
***************************/

AnchorNode::AnchorNode(VRMLParser& parser)
	{
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("AnchorNode::AnchorNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	Vec3f bboxCenter(0.0f,0.0f,0.0f);
	Vec3f bboxSize(-1.0f,-1.0f,-1.0f);
	while(!parser.isToken("}"))
		{
		if(parser.isToken("description"))
			{
			parser.getNextToken();
			description=SFString::parse(parser);
			}
		else if(parser.isToken("parameter"))
			{
			parser.getNextToken();
			parameter=MFString::parse(parser);
			}
		else if(parser.isToken("url"))
			{
			parser.getNextToken();
			url=MFString::parse(parser);
			}
		else if(parser.isToken("bboxCenter"))
			{
			parser.getNextToken();
			bboxCenter=SFVec3f::parse(parser);
			}
		else if(parser.isToken("bboxSize"))
			{
			parser.getNextToken();
			bboxSize=SFVec3f::parse(parser);
			}
		else if(parser.isToken("children"))
			{
			/* Parse the node's children: */
			parseChildren(parser);
			}
		else
			Misc::throwStdErr("AnchorNode::AnchorNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	
	/* Construct the explicit bounding box: */
	setBoundingBox(bboxCenter,bboxSize);
	}
