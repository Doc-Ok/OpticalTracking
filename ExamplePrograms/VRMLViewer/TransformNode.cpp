/***********************************************************************
TransformNode - Node class for VRML transformations.
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

#include <Misc/ThrowStdErr.h>
#include <GL/gl.h>
#include <GL/GLTransformationWrappers.h>

#include "Types.h"
#include "Fields/SFVec3f.h"
#include "Fields/SFRotation.h"

#include "VRMLParser.h"
#include "VRMLRenderState.h"

#include "TransformNode.h"

/******************************
Methods of class TransformNode:
******************************/

TransformNode::TransformNode(VRMLParser& parser)
	:transform(Transformation::identity)
	{
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("TransformNode::TransformNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	Point center=Point::origin;
	Rotation rotation=Rotation::identity;
	float scale=1.0f;
	Rotation scaleOrientation=Rotation::identity;
	Vector translation=Vector::zero;
	Vec3f bboxCenter(0.0f,0.0f,0.0f);
	Vec3f bboxSize(-1.0f,-1.0f,-1.0f);
	while(!parser.isToken("}"))
		{
		if(parser.isToken("center"))
			{
			/* Parse the rotation and scaling center: */
			parser.getNextToken();
			Vec3f c=SFVec3f::parse(parser);
			center=Point(c.getXyzw());
			}
		else if(parser.isToken("rotation"))
			{
			/* Parse the rotation: */
			parser.getNextToken();
			rotation=SFRotation::parse(parser);
			}
		else if(parser.isToken("scale"))
			{
			/* Parse the scale factors: */
			parser.getNextToken();
			Vec3f s=SFVec3f::parse(parser);
			scale=Math::pow(s[0]*s[1]*s[2],1.0f/3.0f);
			}
		else if(parser.isToken("scaleOrientation"))
			{
			/* Parse the scale orientation: */
			parser.getNextToken();
			scaleOrientation=SFRotation::parse(parser);
			}
		else if(parser.isToken("translation"))
			{
			/* Parse the translation vector: */
			parser.getNextToken();
			Vec3f t=SFVec3f::parse(parser);
			translation=Vector(t.getXyzw());
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
			Misc::throwStdErr("TransformNode::TransformNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	
	/* Construct the explicit bounding box: */
	setBoundingBox(bboxCenter,bboxSize);
	
	/* Set the transformation: */
	transform=Transformation::identity;
	transform*=Transformation::translate(translation);
	transform*=Transformation::translateFromOriginTo(center);
	transform*=Transformation::scale(scale);
	transform*=Transformation::rotate(rotation);
	transform*=Transformation::translateToOriginFrom(center);
	}

VRMLNode::Box TransformNode::calcBoundingBox(void) const
	{
	if(haveBoundingBox)
		return boundingBox;
	else
		{
		/* Calculate the union of the transformed bounding boxes of all children: */
		Box result=Box::empty;
		for(NodeList::const_iterator chIt=children.begin();chIt!=children.end();++chIt)
			{
			Box childBox=(*chIt)->calcBoundingBox();
			childBox.transform(transform);
			result.addBox(childBox);
			}
		return result;
		}
	}

void TransformNode::glRenderAction(VRMLRenderState& renderState) const
	{
	/* Apply the transformation: */
	renderState.pushTransform(transform);
	
	/* Render all child nodes: */
	GroupNode::glRenderAction(renderState);
	
	/* Restore the modelview matrix: */
	renderState.popTransform();
	}
