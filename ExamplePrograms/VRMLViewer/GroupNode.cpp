/***********************************************************************
GroupNode - Base class for group nodes in VRML world files.
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
#include "Fields/SFVec3f.h"

#include "VRMLParser.h"

#include "GroupNode.h"

/**************************
Methods of class GroupNode:
**************************/

void GroupNode::parseChildren(VRMLParser& parser)
	{
	/* Check for the opening bracket: */
	parser.getNextToken();
	if(parser.isToken("["))
		{
		parser.getNextToken();
		
		/* Parse child nodes until closing bracket: */
		while(!parser.isToken("]"))
			{
			/* Parse the node and add it to the group: */
			addChild(parser.getNextNode());
			}
		
		/* Skip the closing bracket: */
		parser.getNextToken();
		}
	else
		{
		/* Parse the node and add it to the group: */
		addChild(parser.getNextNode());
		}
	}

void GroupNode::addChild(VRMLNodePointer newChild)
	{
	if(newChild!=0)
		children.push_back(newChild);
	}

void GroupNode::setBoundingBox(const Vec3f& bboxCenter,const Vec3f& bboxSize)
	{
	if(bboxSize[0]>=0.0f&&bboxSize[1]>=0.0f&&bboxSize[2]>=0.0f)
		{
		haveBoundingBox=true;
		Box::Point min(bboxCenter[0]-bboxSize[0],bboxCenter[1]-bboxSize[1],bboxCenter[2]-bboxSize[2]);
		Box::Point max(bboxCenter[0]+bboxSize[0],bboxCenter[1]+bboxSize[1],bboxCenter[2]+bboxSize[2]);
		boundingBox=Box(min,max);
		}
	}

GroupNode::GroupNode(void)
	:haveBoundingBox(false),
	 boundingBox(Box::empty)
	{
	}

GroupNode::GroupNode(VRMLParser& parser)
	:haveBoundingBox(false),
	 boundingBox(Box::empty)
	{
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("GroupNode::GroupNode: Missing opening brace in node definition, have %s instead",parser.getToken());
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
		else if(parser.isToken("children"))
			{
			/* Parse the node's children: */
			parseChildren(parser);
			}
		else
			Misc::throwStdErr("GroupNode::GroupNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	
	/* Construct the explicit bounding box: */
	setBoundingBox(bboxCenter,bboxSize);
	}

GroupNode::~GroupNode(void)
	{
	}

VRMLNode::Box GroupNode::calcBoundingBox(void) const
	{
	if(haveBoundingBox)
		return boundingBox;
	else
		{
		/* Return the union of bounding boxes of all children: */
		Box result=Box::empty;
		for(NodeList::const_iterator chIt=children.begin();chIt!=children.end();++chIt)
			result.addBox((*chIt)->calcBoundingBox());
		return result;
		}
	}

void GroupNode::glRenderAction(VRMLRenderState& renderState) const
	{
	/* Call all child nodes recursively: */
	for(NodeList::const_iterator chIt=children.begin();chIt!=children.end();++chIt)
		(*chIt)->glRenderAction(renderState);
	}
