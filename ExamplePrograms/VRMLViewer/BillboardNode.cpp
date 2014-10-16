/***********************************************************************
BillboardNode - Node class to orient a group of nodes towards the
viewer.
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

#include <Misc/ThrowStdErr.h>

#include "Types.h"
#include "Fields/SFVec3f.h"

#include "VRMLParser.h"
#include "VRMLRenderState.h"

#include "BillboardNode.h"

/******************************
Methods of class BillboardNode:
******************************/

BillboardNode::BillboardNode(VRMLParser& parser)
	:axisOfRotation(0,1,0),
	 orthoZAxis(0,0,1),
	 rotationNormal(0,0,0)
	{
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("BillboardNode::BillboardNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	Vec3f bboxCenter(0.0f,0.0f,0.0f);
	Vec3f bboxSize(-1.0f,-1.0f,-1.0f);
	while(!parser.isToken("}"))
		{
		if(parser.isToken("axisOfRotation"))
			{
			/* Parse the translation vector: */
			parser.getNextToken();
			Vec3f aor=SFVec3f::parse(parser);
			axisOfRotation=Vector(aor.getXyzw());
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
			Misc::throwStdErr("BillboardNode::BillboardNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	
	/* Construct the explicit bounding box: */
	setBoundingBox(bboxCenter,bboxSize);
	
	/* Compute the orthonormalized Z axis: */
	aor2=Geometry::sqr(axisOfRotation);
	if(aor2>0.0f)
		{
		orthoZAxis-=axisOfRotation*((orthoZAxis*axisOfRotation)/aor2);
		orthoZAxis.normalize();
		rotationNormal=Geometry::cross(axisOfRotation,orthoZAxis);
		}
	}

void BillboardNode::glRenderAction(VRMLRenderState& renderState) const
	{
	Vector viewDirection=renderState.viewerPos-Point::origin;
	if(aor2>0.0f)
		{
		/* Rotate the billboard around its axis: */
		viewDirection-=axisOfRotation*((viewDirection*axisOfRotation)/aor2);
		float vdLen=Geometry::mag(viewDirection);
		if(vdLen>0.0f)
			{
			float angle=Math::acos((viewDirection*orthoZAxis)/vdLen);
			if(rotationNormal*viewDirection<0.0f)
				angle=-angle;
			
			/* Apply the transformation: */
			renderState.pushTransform(Transformation::rotate(Transformation::Rotation::rotateAxis(axisOfRotation,angle)));
			
			/* Render all child nodes: */
			GroupNode::glRenderAction(renderState);
			
			/* Restore the modelview matrix: */
			renderState.popTransform();
			}
		else
			{
			/* Render all child nodes: */
			GroupNode::glRenderAction(renderState);
			}
		}
	else
		{
		/* Rotate the billboard into the screen plane: */
		/* Not implemented yet... */
		GroupNode::glRenderAction(renderState);
		}
	}
