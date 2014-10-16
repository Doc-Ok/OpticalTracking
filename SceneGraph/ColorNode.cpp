/***********************************************************************
ColorNode - Class for nodes defining colors.
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

#include <SceneGraph/ColorNode.h>

#include <string.h>
#include <SceneGraph/EventTypes.h>
#include <SceneGraph/VRMLFile.h>

namespace SceneGraph {

/**************************
Methods of class ColorNode:
**************************/

ColorNode::ColorNode(void)
	{
	}

const char* ColorNode::getStaticClassName(void)
	{
	return "Color";
	}

const char* ColorNode::getClassName(void) const
	{
	return "Color";
	}

EventOut* ColorNode::getEventOut(const char* fieldName) const
	{
	if(strcmp(fieldName,"color")==0)
		return makeEventOut(this,color);
	else
		return Node::getEventOut(fieldName);
	}

EventIn* ColorNode::getEventIn(const char* fieldName)
	{
	if(strcmp(fieldName,"color")==0)
		return makeEventIn(this,color);
	else
		return Node::getEventIn(fieldName);
	}

void ColorNode::parseField(const char* fieldName,VRMLFile& vrmlFile)
	{
	if(strcmp(fieldName,"color")==0)
		{
		vrmlFile.parseField(color);
		}
	else
		Node::parseField(fieldName,vrmlFile);
	}

void ColorNode::update(void)
	{
	}

}
