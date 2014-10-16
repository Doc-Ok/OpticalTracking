/***********************************************************************
BoxNode - Class for axis-aligned boxes as renderable geometry.
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

#include <SceneGraph/BoxNode.h>

#include <string.h>
#include <Math/Math.h>
#include <GL/gl.h>
#include <GL/GLVertexTemplates.h>
#include <SceneGraph/EventTypes.h>
#include <SceneGraph/VRMLFile.h>
#include <SceneGraph/GLRenderState.h>

namespace SceneGraph {

/************************
Methods of class BoxNode:
************************/

void BoxNode::createList(GLContextData& renderState) const
	{
	/* Draw the box faces as quads: */
	glBegin(GL_QUADS);
	
	/* Bottom face: */
	glNormal3f(0.0f,-1.0f,0.0f);
	glTexCoord2f(0.0f,0.0f);
	glVertex(box.min[0],box.min[1],box.min[2]);
	glTexCoord2f(1.0f,0.0f);
	glVertex(box.max[0],box.min[1],box.min[2]);
	glTexCoord2f(1.0f,1.0f);
	glVertex(box.max[0],box.min[1],box.max[2]);
	glTexCoord2f(0.0f,1.0f);
	glVertex(box.min[0],box.min[1],box.max[2]);
	
	/* Front face: */
	glNormal3f(0.0f,0.0f,1.0f);
	glTexCoord2f(0.0f,0.0f);
	glVertex(box.min[0],box.min[1],box.max[2]);
	glTexCoord2f(1.0f,0.0f);
	glVertex(box.max[0],box.min[1],box.max[2]);
	glTexCoord2f(1.0f,1.0f);
	glVertex(box.max[0],box.max[1],box.max[2]);
	glTexCoord2f(0.0f,1.0f);
	glVertex(box.min[0],box.max[1],box.max[2]);
	
	/* Right face: */
	glNormal3f(1.0f,0.0f,0.0f);
	glTexCoord2f(0.0f,0.0f);
	glVertex(box.max[0],box.min[1],box.max[2]);
	glTexCoord2f(1.0f,0.0f);
	glVertex(box.max[0],box.min[1],box.min[2]);
	glTexCoord2f(1.0f,1.0f);
	glVertex(box.max[0],box.max[1],box.min[2]);
	glTexCoord2f(0.0f,1.0f);
	glVertex(box.max[0],box.max[1],box.max[2]);
	
	/* Back face: */
	glNormal3f(0.0f,0.0f,-1.0f);
	glTexCoord2f(0.0f,0.0f);
	glVertex(box.max[0],box.min[1],box.min[2]);
	glTexCoord2f(1.0f,0.0f);
	glVertex(box.min[0],box.min[1],box.min[2]);
	glTexCoord2f(1.0f,1.0f);
	glVertex(box.min[0],box.max[1],box.min[2]);
	glTexCoord2f(0.0f,1.0f);
	glVertex(box.max[0],box.max[1],box.min[2]);
	
	/* Left face: */
	glNormal3f(-1.0f,0.0f,0.0f);
	glTexCoord2f(0.0f,0.0f);
	glVertex(box.min[0],box.min[1],box.min[2]);
	glTexCoord2f(1.0f,0.0f);
	glVertex(box.min[0],box.min[1],box.max[2]);
	glTexCoord2f(1.0f,1.0f);
	glVertex(box.min[0],box.max[1],box.max[2]);
	glTexCoord2f(0.0f,1.0f);
	glVertex(box.min[0],box.max[1],box.min[2]);
	
	/* Top face: */
	glNormal3f(0.0f, 1.0f,0.0f);
	glTexCoord2f(0.0f,0.0f);
	glVertex(box.min[0],box.max[1],box.max[2]);
	glTexCoord2f(1.0f,0.0f);
	glVertex(box.max[0],box.max[1],box.max[2]);
	glTexCoord2f(1.0f,1.0f);
	glVertex(box.max[0],box.max[1],box.min[2]);
	glTexCoord2f(0.0f,1.0f);
	glVertex(box.min[0],box.max[1],box.min[2]);
	
	glEnd();
	}

BoxNode::BoxNode(void)
	:center(Point::origin),
	 size(Size(2,2,2)),
	 box(Point(-1,-1,-1),Point(1,1,1))
	{
	}

const char* BoxNode::getStaticClassName(void)
	{
	return "Box";
	}

const char* BoxNode::getClassName(void) const
	{
	return "Box";
	}

EventOut* BoxNode::getEventOut(const char* fieldName) const
	{
	if(strcmp(fieldName,"center")==0)
		return makeEventOut(this,center);
	else if(strcmp(fieldName,"size")==0)
		return makeEventOut(this,size);
	else
		return GeometryNode::getEventOut(fieldName);
	}

EventIn* BoxNode::getEventIn(const char* fieldName)
	{
	if(strcmp(fieldName,"center")==0)
		return makeEventIn(this,center);
	else if(strcmp(fieldName,"size")==0)
		return makeEventIn(this,size);
	else
		return GeometryNode::getEventIn(fieldName);
	}

void BoxNode::parseField(const char* fieldName,VRMLFile& vrmlFile)
	{
	if(strcmp(fieldName,"center")==0)
		{
		vrmlFile.parseField(center);
		}
	else if(strcmp(fieldName,"size")==0)
		{
		vrmlFile.parseField(size);
		}
	else
		GeometryNode::parseField(fieldName,vrmlFile);
	}

void BoxNode::update(void)
	{
	Point pmin=center.getValue();
	Point pmax=center.getValue();
	for(int i=0;i<3;++i)
		{
		pmin[i]-=Math::div2(size.getValue()[i]);
		pmax[i]+=Math::div2(size.getValue()[i]);
		}
	box=Box(pmin,pmax);
	
	/* Invalidate the display list: */
	DisplayList::update();
	}

Box BoxNode::calcBoundingBox(void) const
	{
	/* Return the box itself: */
	return box;
	}

void BoxNode::glRenderAction(GLRenderState& renderState) const
	{
	/* Set up OpenGL state: */
	renderState.enableCulling(GL_BACK);
	
	/* Render the display list: */
	DisplayList::glRenderAction(renderState.contextData);
	}

}
