/***********************************************************************
TransformNode - Class for group nodes that apply an orthogonal
transformation to their children.
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

#include <SceneGraph/TransformNode.h>

#include <string.h>
#include <Math/Math.h>
#include <SceneGraph/EventTypes.h>
#include <SceneGraph/VRMLFile.h>
#include <SceneGraph/GLRenderState.h>

namespace SceneGraph {

/******************************
Methods of class TransformNode:
******************************/

TransformNode::TransformNode(void)
	:center(Point::origin),
	 rotation(Rotation::identity),
	 scale(Size(1,1,1)),
	 scaleOrientation(Rotation::identity),
	 translation(Vector::zero),
	 transform(OGTransform::identity)
	{
	}

const char* TransformNode::getStaticClassName(void)
	{
	return "Transform";
	}

const char* TransformNode::getClassName(void) const
	{
	return "Transform";
	}

EventOut* TransformNode::getEventOut(const char* fieldName) const
	{
	if(strcmp(fieldName,"center")==0)
		return makeEventOut(this,center);
	else if(strcmp(fieldName,"rotation")==0)
		return makeEventOut(this,rotation);
	else if(strcmp(fieldName,"scale")==0)
		return makeEventOut(this,scale);
	else if(strcmp(fieldName,"scaleOrientation")==0)
		return makeEventOut(this,scaleOrientation);
	else if(strcmp(fieldName,"translation")==0)
		return makeEventOut(this,translation);
	else
		return GroupNode::getEventOut(fieldName);
	}

EventIn* TransformNode::getEventIn(const char* fieldName)
	{
	if(strcmp(fieldName,"center")==0)
		return makeEventIn(this,center);
	else if(strcmp(fieldName,"rotation")==0)
		return makeEventIn(this,rotation);
	else if(strcmp(fieldName,"scale")==0)
		return makeEventIn(this,scale);
	else if(strcmp(fieldName,"scaleOrientation")==0)
		return makeEventIn(this,scaleOrientation);
	else if(strcmp(fieldName,"translation")==0)
		return makeEventIn(this,translation);
	else
		return GroupNode::getEventIn(fieldName);
	}

void TransformNode::parseField(const char* fieldName,VRMLFile& vrmlFile)
	{
	if(strcmp(fieldName,"center")==0)
		{
		vrmlFile.parseField(center);
		}
	else if(strcmp(fieldName,"rotation")==0)
		{
		vrmlFile.parseField(rotation);
		}
	else if(strcmp(fieldName,"scale")==0)
		{
		vrmlFile.parseField(scale);
		}
	else if(strcmp(fieldName,"scaleOrientation")==0)
		{
		vrmlFile.parseField(scaleOrientation);
		}
	else if(strcmp(fieldName,"translation")==0)
		{
		vrmlFile.parseField(translation);
		}
	else
		GroupNode::parseField(fieldName,vrmlFile);
	}

void TransformNode::update(void)
	{
	/* Calculate the transformation: */
	transform=OGTransform::identity;
	transform*=OGTransform::translate(translation.getValue());
	transform*=OGTransform::translateFromOriginTo(center.getValue());
	Scalar uniformScale=Math::pow(scale.getValue()[0]*scale.getValue()[1]*scale.getValue()[2],Scalar(1)/Scalar(3));
	transform*=OGTransform::scale(uniformScale);
	transform*=OGTransform::rotate(rotation.getValue());
	transform*=OGTransform::translateToOriginFrom(center.getValue());
	}

Box TransformNode::calcBoundingBox(void) const
	{
	/* Return the explicit bounding box if there is one: */
	if(haveExplicitBoundingBox)
		return explicitBoundingBox;
	else
		{
		/* Calculate the group's bounding box as the union of the transformed children's boxes: */
		Box result=Box::empty;
		for(MFGraphNode::ValueList::const_iterator chIt=children.getValues().begin();chIt!=children.getValues().end();++chIt)
			{
			Box childBox=(*chIt)->calcBoundingBox();
			childBox.transform(transform);
			result.addBox(childBox);
			}
		return result;
		}
	}

void TransformNode::glRenderAction(GLRenderState& renderState) const
	{
	/* Push the transformation onto the matrix stack: */
	GLRenderState::DOGTransform previousTransform=renderState.pushTransform(transform);
	
	/* Call the render actions of all children in order: */
	for(MFGraphNode::ValueList::const_iterator chIt=children.getValues().begin();chIt!=children.getValues().end();++chIt)
		(*chIt)->glRenderAction(renderState);
		
	/* Pop the transformation off the matrix stack: */
	renderState.popTransform(previousTransform);
	}

}
