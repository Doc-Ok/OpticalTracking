/***********************************************************************
CylinderNode - Class for upright circular cylinders as renderable
geometry.
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

#include <SceneGraph/CylinderNode.h>

#include <string.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <GL/gl.h>
#include <GL/GLNormalTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <SceneGraph/VRMLFile.h>
#include <SceneGraph/GLRenderState.h>

namespace SceneGraph {

/*****************************
Methods of class CylinderNode:
*****************************/

void CylinderNode::createList(GLContextData& renderState) const
	{
	Scalar h=height.getValue();
	Scalar h2=Math::div2(h);
	Scalar r=radius.getValue();
	int ns=numSegments.getValue();
	
	if(side.getValue())
		{
		/* Draw the cylinder side: */
		glBegin(GL_QUAD_STRIP);
		glNormal(Scalar(0),Scalar(0),Scalar(-1));
		glTexCoord2f(0.0f,1.0f);
		glVertex(Scalar(0),h2,-r);
		glTexCoord2f(0.0f,0.0f);
		glVertex(Scalar(0),-h2,-r);
		for(int i=1;i<ns;++i)
			{
			Scalar angle=Scalar(2)*Math::Constants<Scalar>::pi*Scalar(i)/Scalar(ns);
			float texS=float(i)/float(ns);
			Scalar c=Math::cos(angle);
			Scalar s=Math::sin(angle);
			glNormal(-s,Scalar(0),-c);
			glTexCoord2f(texS,1.0f);
			glVertex(-s*r, h2,-c*r);
			glTexCoord2f(texS,0.0f);
			glVertex(-s*r,-h2,-c*r);
			}
		glNormal(Scalar(0),Scalar(0),Scalar(-1));
		glTexCoord2f(1.0f,1.0f);
		glVertex(Scalar(0),h2,-r);
		glTexCoord2f(1.0f,0.0f);
		glVertex(Scalar(0),-h2,-r);
		glEnd();
		}
	
	if(bottom.getValue())
		{
		/* Draw the cylinder bottom: */
		glBegin(GL_TRIANGLE_FAN);
		glNormal(Scalar(0),Scalar(-1),Scalar(0));
		glTexCoord2f(0.5f,0.5f);
		glVertex(Scalar(0),-h2,Scalar(0));
		glTexCoord2f(0.5f,0.0f);
		glVertex(Scalar(0),-h2,-r);
		for(int i=ns-1;i>0;--i)
			{
			Scalar angle=Scalar(2)*Math::Constants<Scalar>::pi*Scalar(i)/Scalar(ns);
			Scalar c=Math::cos(angle);
			Scalar s=Math::sin(angle);
			glTexCoord2f(-float(s)*0.5f+0.5f,-float(c)*0.5f+0.5f);
			glVertex(-s*r,-h2,-c*r);
			}
		glTexCoord2f(0.5f,0.0f);
		glVertex(Scalar(0),-h2,-r);
		glEnd();
		}
	
	if(top.getValue())
		{
		/* Draw the cylinder top: */
		glBegin(GL_TRIANGLE_FAN);
		glNormal(Scalar(0),Scalar(1),Scalar(0));
		glTexCoord2f(0.5f,0.5f);
		glVertex(Scalar(0),h2,Scalar(0));
		glTexCoord2f(0.5f,1.0f);
		glVertex(Scalar(0),h2,-r);
		for(int i=1;i<ns;++i)
			{
			Scalar angle=Scalar(2)*Math::Constants<Scalar>::pi*Scalar(i)/Scalar(ns);
			Scalar c=Math::cos(angle);
			Scalar s=Math::sin(angle);
			glTexCoord2f(-float(s)*0.5f+0.5f,float(c)*0.5f+0.5f);
			glVertex(-s*r,h2,-c*r);
			}
		glTexCoord2f(0.5f,1.0f);
		glVertex(Scalar(0),h2,-r);
		glEnd();
		}
	}

CylinderNode::CylinderNode(void)
	:height(Scalar(2)),
	 radius(Scalar(1)),
	 numSegments(12),
	 side(true),
	 bottom(true),
	 top(true)
	{
	}

const char* CylinderNode::getStaticClassName(void)
	{
	return "Cylinder";
	}

const char* CylinderNode::getClassName(void) const
	{
	return "Cylinder";
	}

void CylinderNode::parseField(const char* fieldName,VRMLFile& vrmlFile)
	{
	if(strcmp(fieldName,"height")==0)
		{
		vrmlFile.parseField(height);
		}
	else if(strcmp(fieldName,"radius")==0)
		{
		vrmlFile.parseField(radius);
		}
	else if(strcmp(fieldName,"numSegments")==0)
		{
		vrmlFile.parseField(numSegments);
		}
	else if(strcmp(fieldName,"side")==0)
		{
		vrmlFile.parseField(side);
		}
	else if(strcmp(fieldName,"bottom")==0)
		{
		vrmlFile.parseField(bottom);
		}
	else if(strcmp(fieldName,"top")==0)
		{
		vrmlFile.parseField(top);
		}
	else
		GeometryNode::parseField(fieldName,vrmlFile);
	}

void CylinderNode::update(void)
	{
	/* Invalidate the display list: */
	DisplayList::update();
	}

Box CylinderNode::calcBoundingBox(void) const
	{
	/* Calculate the bounding box: */
	Scalar r=radius.getValue();
	Scalar h2=Math::div2(height.getValue());
	return Box(Point(-r,-h2,-r),Point(r,h2,r));
	}

void CylinderNode::glRenderAction(GLRenderState& renderState) const
	{
	/* Set up OpenGL state: */
	renderState.enableCulling(GL_BACK);
	
	/* Render the display list: */
	DisplayList::glRenderAction(renderState.contextData);
	}

}
