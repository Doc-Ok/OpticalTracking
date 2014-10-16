/***********************************************************************
ConeNode - Class for upright circular cones as renderable geometry.
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

#include <SceneGraph/ConeNode.h>

#include <string.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <GL/gl.h>
#include <GL/GLNormalTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <SceneGraph/EventTypes.h>
#include <SceneGraph/VRMLFile.h>
#include <SceneGraph/GLRenderState.h>

namespace SceneGraph {

/*************************
Methods of class ConeNode:
*************************/

void ConeNode::createList(GLContextData& renderState) const
	{
	Scalar h=height.getValue();
	Scalar h2=Math::div2(h);
	Scalar br=bottomRadius.getValue();
	int ns=numSegments.getValue();
	
	if(side.getValue())
		{
		/* Draw the cone side: */
		glBegin(GL_QUAD_STRIP);
		Scalar nScale=Scalar(1)/Math::sqrt(h*h+br*br);
		glNormal(Scalar(0),br*nScale,-h*nScale);
		glTexCoord2f(0.0f,1.0f);
		glVertex(Scalar(0),h2,Scalar(0));
		glTexCoord2f(0.0f,0.0f);
		glVertex(Scalar(0),-h2,-br);
		for(int i=1;i<ns;++i)
			{
			Scalar angle=Scalar(2)*Math::Constants<Scalar>::pi*Scalar(i)/Scalar(ns);
			float texS=float(i)/float(ns);
			Scalar c=Math::cos(angle);
			Scalar s=Math::sin(angle);
			glNormal(-s*h*nScale,br*nScale,-c*h*nScale);
			glTexCoord2f(texS,1.0f);
			glVertex(Scalar(0),h2,Scalar(0));
			glTexCoord2f(texS,0.0f);
			glVertex(-s*br,-h2,-c*br);
			}
		glNormal(Scalar(0),br*nScale,-h*nScale);
		glTexCoord2f(1.0f,1.0f);
		glVertex(Scalar(0),h2,Scalar(0));
		glTexCoord2f(1.0f,0.0f);
		glVertex(Scalar(0),-h2,-br);
		glEnd();
		}
	
	if(bottom.getValue())
		{
		/* Draw the cone bottom: */
		glBegin(GL_TRIANGLE_FAN);
		glNormal(Scalar(0),Scalar(-1),Scalar(0));
		glTexCoord2f(0.5f,0.5f);
		glVertex(Scalar(0),-h2,Scalar(0));
		glTexCoord2f(0.5f,0.0f);
		glVertex(Scalar(0),-h2,-br);
		for(int i=ns-1;i>0;--i)
			{
			Scalar angle=Scalar(2)*Math::Constants<Scalar>::pi*Scalar(i)/Scalar(ns);
			Scalar c=Math::cos(angle);
			Scalar s=Math::sin(angle);
			glTexCoord2f(-float(s)*0.5f+0.5f,-float(c)*0.5f+0.5f);
			glVertex(-s*br,-h2,-c*br);
			}
		glTexCoord2f(0.5f,0.0f);
		glVertex(Scalar(0),-h2,-br);
		glEnd();
		}
	}

ConeNode::ConeNode(void)
	:height(Scalar(2)),
	 bottomRadius(Scalar(1)),
	 numSegments(12),
	 side(true),
	 bottom(true)
	{
	}

const char* ConeNode::getStaticClassName(void)
	{
	return "Cone";
	}

const char* ConeNode::getClassName(void) const
	{
	return "Cone";
	}

EventOut* ConeNode::getEventOut(const char* fieldName) const
	{
	if(strcmp(fieldName,"height")==0)
		return makeEventOut(this,height);
	else if(strcmp(fieldName,"bottomRadius")==0)
		return makeEventOut(this,bottomRadius);
	else if(strcmp(fieldName,"numSegments")==0)
		return makeEventOut(this,numSegments);
	else if(strcmp(fieldName,"side")==0)
		return makeEventOut(this,side);
	else if(strcmp(fieldName,"bottom")==0)
		return makeEventOut(this,bottom);
	else
		return GeometryNode::getEventOut(fieldName);
	}

EventIn* ConeNode::getEventIn(const char* fieldName)
	{
	if(strcmp(fieldName,"height")==0)
		return makeEventIn(this,height);
	else if(strcmp(fieldName,"bottomRadius")==0)
		return makeEventIn(this,bottomRadius);
	else if(strcmp(fieldName,"numSegments")==0)
		return makeEventIn(this,numSegments);
	else if(strcmp(fieldName,"side")==0)
		return makeEventIn(this,side);
	else if(strcmp(fieldName,"bottom")==0)
		return makeEventIn(this,bottom);
	else
		return GeometryNode::getEventIn(fieldName);
	}

void ConeNode::parseField(const char* fieldName,VRMLFile& vrmlFile)
	{
	if(strcmp(fieldName,"height")==0)
		{
		vrmlFile.parseField(height);
		}
	else if(strcmp(fieldName,"bottomRadius")==0)
		{
		vrmlFile.parseField(bottomRadius);
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
	else
		GeometryNode::parseField(fieldName,vrmlFile);
	}

void ConeNode::update(void)
	{
	/* Invalidate the display list: */
	DisplayList::update();
	}

Box ConeNode::calcBoundingBox(void) const
	{
	/* Calculate the bounding box: */
	Scalar br=bottomRadius.getValue();
	Scalar h2=Math::div2(height.getValue());
	return Box(Point(-br,-h2,-br),Point(br,h2,br));
	}

void ConeNode::glRenderAction(GLRenderState& renderState) const
	{
	/* Set up OpenGL state: */
	renderState.enableCulling(GL_BACK);
	
	/* Render the display list: */
	DisplayList::glRenderAction(renderState.contextData);
	}

}
