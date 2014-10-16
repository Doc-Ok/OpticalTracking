/***********************************************************************
CoordinateNode - Class for nodes defining point coordinates.
Copyright (c) 2009-2010 Oliver Kreylos

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

#include <SceneGraph/CoordinateNode.h>

#include <string.h>
#include <Geometry/Box.h>
#include <SceneGraph/EventTypes.h>
#include <SceneGraph/VRMLFile.h>

namespace SceneGraph {

/*******************************
Methods of class CoordinateNode:
*******************************/

CoordinateNode::CoordinateNode(void)
	{
	}

const char* CoordinateNode::getStaticClassName(void)
	{
	return "Coordinate";
	}

const char* CoordinateNode::getClassName(void) const
	{
	return "Coordinate";
	}

EventOut* CoordinateNode::getEventOut(const char* fieldName) const
	{
	if(strcmp(fieldName,"point")==0)
		return makeEventOut(this,point);
	else
		return Node::getEventOut(fieldName);
	}

EventIn* CoordinateNode::getEventIn(const char* fieldName)
	{
	if(strcmp(fieldName,"point")==0)
		return makeEventIn(this,point);
	else
		return Node::getEventIn(fieldName);
	}

void CoordinateNode::parseField(const char* fieldName,VRMLFile& vrmlFile)
	{
	if(strcmp(fieldName,"point")==0)
		{
		vrmlFile.parseField(point);
		}
	else
		Node::parseField(fieldName,vrmlFile);
	}

void CoordinateNode::update(void)
	{
	}

Box CoordinateNode::calcBoundingBox(void) const
	{
	Box result=Box::empty;
	for(MFPoint::ValueList::const_iterator pIt=point.getValues().begin();pIt!=point.getValues().end();++pIt)
		result.addPoint(*pIt);
	return result;
	}

}
