/***********************************************************************
BillboardNode - Class for group nodes that transform their children to
always face the viewer.
Copyright (c) 2009-2013 Oliver Kreylos

This file is part of the Simple Scene Graph Renderer (SceneGraph).

The Simple Scene Graph Renderer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Simple Scene Graph Renderer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Simple Scene Graph Renderer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <SceneGraph/BillboardNode.h>

#include <string.h>
#include <Math/Math.h>
#include <Geometry/Point.h>
#include <Geometry/OrthogonalTransformation.h>
#include <SceneGraph/EventTypes.h>
#include <SceneGraph/VRMLFile.h>
#include <SceneGraph/GLRenderState.h>

namespace SceneGraph {

/******************************
Methods of class BillboardNode:
******************************/

BillboardNode::BillboardNode(void)
	:axisOfRotation(Vector(0,1,0)),
	 aor2(1),
	 orthoZAxis(0,0,1),
	 rotationNormal(1,0,0)
	{
	}

const char* BillboardNode::getStaticClassName(void)
	{
	return "Billboard";
	}

const char* BillboardNode::getClassName(void) const
	{
	return "Billboard";
	}

EventOut* BillboardNode::getEventOut(const char* fieldName) const
	{
	if(strcmp(fieldName,"axisOfRotation")==0)
		return makeEventOut(this,axisOfRotation);
	else
		return GroupNode::getEventOut(fieldName);
	}

EventIn* BillboardNode::getEventIn(const char* fieldName)
	{
	if(strcmp(fieldName,"axisOfRotation")==0)
		return makeEventIn(this,axisOfRotation);
	else
		return GroupNode::getEventIn(fieldName);
	}

void BillboardNode::parseField(const char* fieldName,VRMLFile& vrmlFile)
	{
	if(strcmp(fieldName,"axisOfRotation")==0)
		{
		vrmlFile.parseField(axisOfRotation);
		}
	else
		GroupNode::parseField(fieldName,vrmlFile);
	}

void BillboardNode::update(void)
	{
	/* Compute the orthonormalized Z axis: */
	aor2=Geometry::sqr(axisOfRotation.getValue());
	if(aor2!=Scalar(0))
		{
		orthoZAxis=Vector(0,0,1);
		orthoZAxis-=axisOfRotation.getValue()*((orthoZAxis*axisOfRotation.getValue())/aor2);
		orthoZAxis.normalize();
		rotationNormal=axisOfRotation.getValue()^orthoZAxis;
		}
	}

void BillboardNode::glRenderAction(GLRenderState& renderState) const
	{
	/* Calculate the billboard transformation: */
	OGTransform previousTransform;
	Vector viewDirection=renderState.getViewerPos()-Point::origin;
	if(aor2!=Scalar(0))
		{
		/* Rotate the billboard around its axis: */
		viewDirection-=axisOfRotation.getValue()*((viewDirection*axisOfRotation.getValue())/aor2);
		Scalar vdLen=Geometry::mag(viewDirection);
		if(vdLen!=Scalar(0))
			{
			Scalar angle=Math::acos((viewDirection*orthoZAxis)/vdLen);
			if(rotationNormal*viewDirection<Scalar(0))
				angle=-angle;
			
			/* Apply the transformation: */
			previousTransform=renderState.pushTransform(OGTransform::rotate(OGTransform::Rotation::rotateAxis(axisOfRotation.getValue(),angle)));
			}
		else
			previousTransform=renderState.pushTransform(OGTransform::identity);
		}
	else
		{
		/* Align the billboard's Z axis with the viewing direction: */
		OGTransform transform=OGTransform::rotate(OGTransform::Rotation::rotateFromTo(Vector(0,0,1),viewDirection));
		
		/* Rotate the billboard's Y axis into the plane formed by the viewing direction and the up direction: */
		Vector up=transform.inverseTransform(renderState.getUpVector());
		if(up[0]!=Scalar(0)||up[1]!=Scalar(0))
			{
			Scalar angle=Math::atan2(-up[0],up[1]);
			transform*=OGTransform::rotate(OGTransform::Rotation::rotateZ(angle));
			}
		
		previousTransform=renderState.pushTransform(transform);
		}
	
	/* Call the render actions of all children in order: */
	for(MFGraphNode::ValueList::const_iterator chIt=children.getValues().begin();chIt!=children.getValues().end();++chIt)
		(*chIt)->glRenderAction(renderState);
		
	/* Pop the transformation off the matrix stack: */
	renderState.popTransform(previousTransform);
	}

}
