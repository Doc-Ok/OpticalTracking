/***********************************************************************
LODNode - Class for group nodes that select between their children based
on distance from the viewpoint.
Copyright (c) 2011 Oliver Kreylos

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

#include <SceneGraph/LODNode.h>

#include <string.h>
#include <SceneGraph/EventTypes.h>
#include <SceneGraph/VRMLFile.h>
#include <SceneGraph/GLRenderState.h>

namespace SceneGraph {

/************************
Methods of class LODNode:
************************/

LODNode::LODNode(void)
	:center(Point::origin)
	{
	}

const char* LODNode::getStaticClassName(void)
	{
	return "LOD";
	}

const char* LODNode::getClassName(void) const
	{
	return "LOD";
	}

EventOut* LODNode::getEventOut(const char* fieldName) const
	{
	if(strcmp(fieldName,"level")==0)
		return makeEventOut(this,level);
	else if(strcmp(fieldName,"center")==0)
		return makeEventOut(this,center);
	else if(strcmp(fieldName,"range")==0)
		return makeEventOut(this,range);
	else
		return GraphNode::getEventOut(fieldName);
	}

EventIn* LODNode::getEventIn(const char* fieldName)
	{
	if(strcmp(fieldName,"level")==0)
		return makeEventIn(this,level);
	else if(strcmp(fieldName,"center")==0)
		return makeEventIn(this,center);
	else if(strcmp(fieldName,"range")==0)
		return makeEventIn(this,range);
	else
		return GraphNode::getEventIn(fieldName);
	}

void LODNode::parseField(const char* fieldName,VRMLFile& vrmlFile)
	{
	if(strcmp(fieldName,"level")==0)
		{
		vrmlFile.parseMFNode(level);
		}
	else if(strcmp(fieldName,"center")==0)
		{
		vrmlFile.parseField(center);
		}
	else if(strcmp(fieldName,"range")==0)
		{
		vrmlFile.parseField(range);
		}
	else
		GraphNode::parseField(fieldName,vrmlFile);
	}

Box LODNode::calcBoundingBox(void) const
	{
	/* Calculate the group's bounding box as the union of the children's boxes: */
	Box result=Box::empty;
	for(MFGraphNode::ValueList::const_iterator lIt=level.getValues().begin();lIt!=level.getValues().end();++lIt)
		result.addBox((*lIt)->calcBoundingBox());
	return result;
	}

void LODNode::glRenderAction(GLRenderState& renderState) const
	{
	/* Bail out if the level list is empty: */
	if(level.getValues().empty())
		return;
	
	/* Calculate the distance to the viewpoint: */
	Scalar viewDist2=Geometry::sqrDist(renderState.getViewerPos(),center.getValue());
	
	/* Find the appropriate level to display: */
	unsigned int l=0;
	unsigned int r=range.getNumValues()+1;
	while(r-l>1)
		{
		unsigned int m=(l+r)>>1;
		if(Math::sqr(range.getValue(m-1))<=viewDist2)
			l=m;
		else
			r=m;
		}
	
	/* Call the render action of the selected level: */
	if(l>level.getNumValues()-1)
		l=level.getNumValues()-1;
	level.getValue(l)->glRenderAction(renderState);
	}

}
