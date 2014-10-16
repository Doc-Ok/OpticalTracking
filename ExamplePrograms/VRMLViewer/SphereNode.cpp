/***********************************************************************
SphereNode - Node class for sphere shapes.
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

#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLModels.h>

#include "Types.h"
#include "Fields/SFFloat.h"

#include "VRMLParser.h"
#include "VRMLRenderState.h"

#include "SphereNode.h"

/*************************************
Methods of class SphereNode::DataItem:
*************************************/

SphereNode::DataItem::DataItem(void)
	:displayListId(glGenLists(1))
	{
	}

SphereNode::DataItem::~DataItem(void)
	{
	/* Destroy the display list: */
	glDeleteLists(displayListId,1);
	}

/***************************
Methods of class SphereNode:
***************************/

SphereNode::SphereNode(VRMLParser& parser)
	:radius(1.0f)
	{
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("SphereNode::SphereNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	while(!parser.isToken("}"))
		{
		if(parser.isToken("radius"))
			{
			parser.getNextToken();
			radius=SFFloat::parse(parser);
			}
		else
			Misc::throwStdErr("SphereNode::SphereNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	}

SphereNode::~SphereNode(void)
	{
	}

void SphereNode::initContext(GLContextData& contextData) const
	{
	/* Create a data item and store it in the context: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Create the sphere's display list: */
	glNewList(dataItem->displayListId,GL_COMPILE);
	glDrawSphereMercatorWithTexture(radius,45,90);
	glEndList();
	}

VRMLNode::Box SphereNode::calcBoundingBox(void) const
	{
	return Box(Box::Point(-radius,-radius,-radius),Box::Point(radius,radius,radius));
	}

void SphereNode::glRenderAction(VRMLRenderState& renderState) const
	{
	/* Retrieve the data item from the context: */
	DataItem* dataItem=renderState.contextData.retrieveDataItem<DataItem>(this);
	
	/* Call the display list: */
	glCallList(dataItem->displayListId);
	}
