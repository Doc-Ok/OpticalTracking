/***********************************************************************
ReferenceEllipsoidNode - Class for nodes defining reference ellipsoid
(geodesic data) for geodesic coordinate systems and transformations
between them.
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

#include <SceneGraph/ReferenceEllipsoidNode.h>

#include <string.h>
#include <SceneGraph/VRMLFile.h>

namespace SceneGraph {

/***************************************
Methods of class ReferenceEllipsoidNode:
***************************************/

ReferenceEllipsoidNode::ReferenceEllipsoidNode(void)
	:radius(6378137.0),
	 flattening(1.0/298.257223563),
	 scale(1.0e-3)
	{
	}

const char* ReferenceEllipsoidNode::getStaticClassName(void)
	{
	return "ReferenceEllipsoid";
	}

const char* ReferenceEllipsoidNode::getClassName(void) const
	{
	return "ReferenceEllipsoid";
	}

void ReferenceEllipsoidNode::parseField(const char* fieldName,VRMLFile& vrmlFile)
	{
	if(strcmp(fieldName,"radius")==0)
		{
		vrmlFile.parseField(radius);
		}
	else if(strcmp(fieldName,"flattening")==0)
		{
		vrmlFile.parseField(flattening);
		}
	else if(strcmp(fieldName,"inverseFlattening")==0)
		{
		/* Parse the field as if it were normal flattening: */
		vrmlFile.parseField(flattening);
		
		/* Invert the field value: */
		flattening.setValue(1.0/flattening.getValue());
		}
	else if(strcmp(fieldName,"scale")==0)
		{
		vrmlFile.parseField(scale);
		}
	else
		Node::parseField(fieldName,vrmlFile);
	}

void ReferenceEllipsoidNode::update(void)
	{
	/* Update the low-level reference ellipsoid: */
	re=Geoid(radius.getValue()*scale.getValue(),flattening.getValue());
	}

}
