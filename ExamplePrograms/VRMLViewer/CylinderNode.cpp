/***********************************************************************
CylinderNode - Node class for cylindrical shapes.
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

#include <Math/Math.h>
#include <Math/Constants.h>
#include <GL/gl.h>
#include <GL/GLContextData.h>

#include "Types.h"
#include "Fields/SFBool.h"
#include "Fields/SFFloat.h"

#include "VRMLParser.h"
#include "VRMLRenderState.h"

#include "CylinderNode.h"

/***************************************
Methods of class CylinderNode::DataItem:
***************************************/

CylinderNode::DataItem::DataItem(void)
	:displayListId(glGenLists(1))
	{
	}

CylinderNode::DataItem::~DataItem(void)
	{
	/* Destroy the display list: */
	glDeleteLists(displayListId,1);
	}

/*****************************
Methods of class CylinderNode:
*****************************/

CylinderNode::CylinderNode(VRMLParser& parser)
	:bottom(true),side(true),top(true),
	 height(2.0f),radius(1.0f)
	{
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("CylinderNode::CylinderNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	while(!parser.isToken("}"))
		{
		if(parser.isToken("bottom"))
			{
			parser.getNextToken();
			bottom=SFBool::parse(parser);
			}
		else if(parser.isToken("side"))
			{
			parser.getNextToken();
			side=SFBool::parse(parser);
			}
		else if(parser.isToken("top"))
			{
			parser.getNextToken();
			top=SFBool::parse(parser);
			}
		else if(parser.isToken("height"))
			{
			parser.getNextToken();
			height=SFFloat::parse(parser);
			}
		else if(parser.isToken("radius"))
			{
			parser.getNextToken();
			radius=SFFloat::parse(parser);
			}
		else
			Misc::throwStdErr("CylinderNode::CylinderNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	}

CylinderNode::~CylinderNode(void)
	{
	}

void CylinderNode::initContext(GLContextData& contextData) const
	{
	/* Create a data item and store it in the context: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Create the cylinder's display list: */
	glNewList(dataItem->displayListId,GL_COMPILE);
	float h2=height*0.5f;
	int numQuads=32;
	if(bottom)
		{
		/* Draw the cylinder's bottom cap: */
		glBegin(GL_POLYGON);
		glNormal3f(0.0f,-1.0f,0.0f);
		for(int i=numQuads-1;i>=0;--i)
			{
			float angle=2.0f*Math::Constants<float>::pi*float(i)/float(numQuads);
			float c=Math::cos(angle);
			float s=Math::sin(angle);
			glTexCoord2f(-s*0.5f+0.5f,-c*0.5f+0.5f);
			glVertex3f(-s*radius,-h2,-c*radius);
			}
		glEnd();
		}
	if(side)
		{
		/* Draw the cylinder's side: */
		glBegin(GL_QUAD_STRIP);
		for(int i=0;i<numQuads;++i)
			{
			float angle=2.0f*Math::Constants<float>::pi*float(i)/float(numQuads);
			float texS=float(i)/float(numQuads);
			float c=Math::cos(angle);
			float s=Math::sin(angle);
			glNormal3f(-s,0.0f,-c);
			glTexCoord2f(texS,1.0f);
			glVertex3f(-s*radius, h2,-c*radius);
			glTexCoord2f(texS,0.0f);
			glVertex3f(-s*radius,-h2,-c*radius);
			}
		glNormal3f(0.0f,0.0f,-1.0f);
		glTexCoord2f(1.0f,1.0f);
		glVertex3f(0.0f,h2,-radius);
		glTexCoord2f(1.0f,0.0f);
		glVertex3f(0.0f,-h2,-radius);
		glEnd();
		}
	if(top)
		{
		/* Draw the cylinder's top cap: */
		glBegin(GL_POLYGON);
		glNormal3f(0.0f,1.0f,0.0f);
		for(int i=0;i<numQuads;++i)
			{
			float angle=2.0f*Math::Constants<float>::pi*float(i)/float(numQuads);
			float c=Math::cos(angle);
			float s=Math::sin(angle);
			glTexCoord2f(-s*0.5f+0.5f,c*0.5f+0.5f);
			glVertex3f(-s*radius,h2,-c*radius);
			}
		glEnd();
		}
	glEndList();
	}

VRMLNode::Box CylinderNode::calcBoundingBox(void) const
	{
	return Box(Box::Point(-radius,-height*0.5f,-radius),Box::Point(radius,height*0.5f,radius));
	}

void CylinderNode::glRenderAction(VRMLRenderState& renderState) const
	{
	/* Retrieve the data item from the context: */
	DataItem* dataItem=renderState.contextData.retrieveDataItem<DataItem>(this);
	
	/* Call the display list: */
	glCallList(dataItem->displayListId);
	}
