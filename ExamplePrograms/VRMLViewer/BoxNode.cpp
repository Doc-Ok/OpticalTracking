/***********************************************************************
BoxNode - Node class for box shapes.
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

#include "Types.h"
#include "Fields/SFVec3f.h"

#include "VRMLParser.h"
#include "VRMLRenderState.h"

#include "BoxNode.h"

/**********************************
Methods of class BoxNode::DataItem:
**********************************/

BoxNode::DataItem::DataItem(void)
	:displayListId(glGenLists(1))
	{
	}

BoxNode::DataItem::~DataItem(void)
	{
	/* Destroy the display list: */
	glDeleteLists(displayListId,1);
	}

/************************
Methods of class BoxNode:
************************/

BoxNode::BoxNode(VRMLParser& parser)
	:size(2.0f,2.0f,2.0f)
	{
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("BoxNode::BoxNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	while(!parser.isToken("}"))
		{
		if(parser.isToken("size"))
			{
			parser.getNextToken();
			size=SFVec3f::parse(parser);
			}
		else
			Misc::throwStdErr("BoxNode::BoxNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	}

BoxNode::~BoxNode(void)
	{
	}

void BoxNode::initContext(GLContextData& contextData) const
	{
	/* Create a data item and store it in the context: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Create the box's display list: */
	glNewList(dataItem->displayListId,GL_COMPILE);
	Vec3f s2(size[0]*0.5f,size[1]*0.5f,size[2]*0.5f);
	glBegin(GL_QUADS);
	
	/* Bottom face: */
	glNormal3f(0.0f,-1.0f,0.0f);
	glTexCoord2f(0.0f,0.0f);
	glVertex3f(-s2[0],-s2[1],-s2[2]);
	glTexCoord2f(1.0f,0.0f);
	glVertex3f( s2[0],-s2[1],-s2[2]);
	glTexCoord2f(1.0f,1.0f);
	glVertex3f( s2[0],-s2[1], s2[2]);
	glTexCoord2f(0.0f,1.0f);
	glVertex3f(-s2[0],-s2[1], s2[2]);
	
	/* Front face: */
	glNormal3f(0.0f,0.0f,1.0f);
	glTexCoord2f(0.0f,0.0f);
	glVertex3f(-s2[0],-s2[1], s2[2]);
	glTexCoord2f(1.0f,0.0f);
	glVertex3f( s2[0],-s2[1], s2[2]);
	glTexCoord2f(1.0f,1.0f);
	glVertex3f( s2[0], s2[1], s2[2]);
	glTexCoord2f(0.0f,1.0f);
	glVertex3f(-s2[0], s2[1], s2[2]);
	
	/* Right face: */
	glNormal3f(1.0f,0.0f,0.0f);
	glTexCoord2f(0.0f,0.0f);
	glVertex3f( s2[0],-s2[1], s2[2]);
	glTexCoord2f(1.0f,0.0f);
	glVertex3f( s2[0],-s2[1],-s2[2]);
	glTexCoord2f(1.0f,1.0f);
	glVertex3f( s2[0], s2[1],-s2[2]);
	glTexCoord2f(0.0f,1.0f);
	glVertex3f( s2[0], s2[1], s2[2]);
	
	/* Back face: */
	glNormal3f(0.0f,0.0f,-1.0f);
	glTexCoord2f(0.0f,0.0f);
	glVertex3f( s2[0],-s2[1],-s2[2]);
	glTexCoord2f(1.0f,0.0f);
	glVertex3f(-s2[0],-s2[1],-s2[2]);
	glTexCoord2f(1.0f,1.0f);
	glVertex3f(-s2[0], s2[1],-s2[2]);
	glTexCoord2f(0.0f,1.0f);
	glVertex3f( s2[0], s2[1],-s2[2]);
	
	/* Left face: */
	glNormal3f(-1.0f,0.0f,0.0f);
	glTexCoord2f(0.0f,0.0f);
	glVertex3f(-s2[0],-s2[1],-s2[2]);
	glTexCoord2f(1.0f,0.0f);
	glVertex3f(-s2[0],-s2[1], s2[2]);
	glTexCoord2f(1.0f,1.0f);
	glVertex3f(-s2[0], s2[1], s2[2]);
	glTexCoord2f(0.0f,1.0f);
	glVertex3f(-s2[0], s2[1],-s2[2]);
	
	/* Top face: */
	glNormal3f(0.0f, 1.0f,0.0f);
	glTexCoord2f(0.0f,0.0f);
	glVertex3f(-s2[0], s2[1], s2[2]);
	glTexCoord2f(1.0f,0.0f);
	glVertex3f( s2[0], s2[1], s2[2]);
	glTexCoord2f(1.0f,1.0f);
	glVertex3f( s2[0], s2[1],-s2[2]);
	glTexCoord2f(0.0f,1.0f);
	glVertex3f(-s2[0], s2[1],-s2[2]);
	
	glEnd();
	glEndList();
	}

VRMLNode::Box BoxNode::calcBoundingBox(void) const
	{
	return Box(Box::Point(-size[0]*0.5f,-size[1]*0.5f,-size[2]*0.5f),Box::Point(size[0]*0.5f,size[1]*0.5f,size[2]*0.5f));
	}

void BoxNode::glRenderAction(VRMLRenderState& renderState) const
	{
	/* Retrieve the data item from the context: */
	DataItem* dataItem=renderState.contextData.retrieveDataItem<DataItem>(this);
	
	/* Call the display list: */
	glCallList(dataItem->displayListId);
	}
