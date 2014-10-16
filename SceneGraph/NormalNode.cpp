/***********************************************************************
NormalNode - Class for nodes defining normal vectors.
Copyright (c) 2009 Oliver Kreylos

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

#include <SceneGraph/NormalNode.h>

#include <string.h>
#include <SceneGraph/VRMLFile.h>

namespace SceneGraph {

/***************************
Methods of class NormalNode:
***************************/

NormalNode::NormalNode(void)
	{
	}

const char* NormalNode::getStaticClassName(void)
	{
	return "Normal";
	}

const char* NormalNode::getClassName(void) const
	{
	return "Normal";
	}

void NormalNode::parseField(const char* fieldName,VRMLFile& vrmlFile)
	{
	if(strcmp(fieldName,"vector")==0)
		{
		vrmlFile.parseField(vector);
		}
	else
		Node::parseField(fieldName,vrmlFile);
	}

void NormalNode::update(void)
	{
	}

}
