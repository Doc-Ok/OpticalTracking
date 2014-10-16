/***********************************************************************
Separator - Class for widgets to visually separate adjacent components
in widget layouts.
Copyright (c) 2008 Oliver Kreylos

This file is part of the GLMotif Widget Library (GLMotif).

The GLMotif Widget Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GLMotif Widget Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the GLMotif Widget Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/Container.h>

#include <GLMotif/Separator.h>

namespace GLMotif {

/**************************
Methods of class Separator:
**************************/

Separator::Separator(const char* sName,Container* sParent,Orientation sOrientation,GLfloat sPreferredHeight,Style sStyle,bool sManageChild)
	:Widget(sName,sParent,false),
	 orientation(sOrientation),
	 preferredHeight(sPreferredHeight),
	 style(sStyle)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Set the default margin and separator width: */
	marginWidth=ss->size*0.5f;
	separatorWidth=ss->size*0.5f;
	
	/* Separator defaults to no border: */
	setBorderWidth(0.0f);
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

Vector Separator::calcNaturalSize(void) const
	{
	/* Calculate the preferred height: */
	GLfloat height=2.0f*marginWidth+3.0f*separatorWidth;
	if(height<preferredHeight)
		height=preferredHeight;
	
	/* Return the preferred size: */
	if(orientation==VERTICAL)
		return calcExteriorSize(Vector(separatorWidth+2.0f*marginWidth,height,0.0f));
	else
		return calcExteriorSize(Vector(height,separatorWidth+2.0f*marginWidth,0.0f));
	}

ZRange Separator::calcZRange(void) const
	{
	/* Calculate the parent class widget's z range: */
	ZRange myZRange=Widget::calcZRange();
	
	/* Adjust for the separator: */
	if(style==RAISED)
		myZRange.second+=separatorWidth*0.5f;
	else
		myZRange.first-=separatorWidth*0.5f;
	
	return myZRange;
	}

void Separator::resize(const Box& newExterior)
	{
	/* Resize the parent class widget: */
	Widget::resize(newExterior);
	
	/* Calculate the separator box: */
	separator=getInterior();
	if(orientation==VERTICAL)
		{
		separator.origin[0]+=(separator.size[0]-separatorWidth)*0.5f;
		separator.size[0]=separatorWidth;
		separator.origin[1]+=marginWidth;
		separator.size[1]-=2.0f*marginWidth;
		}
	else
		{
		separator.origin[0]+=marginWidth;
		separator.size[0]-=2.0f*marginWidth;
		separator.origin[1]+=(separator.size[1]-separatorWidth)*0.5f;
		separator.size[1]=separatorWidth;
		}
	
	/* Calculate the separator ridge line: */
	p1=separator.getCorner(0);
	p2=separator.getCorner(3);
	for(int i=0;i<2;++i)
		{
		p1[i]+=separatorWidth*0.5f;
		p2[i]-=separatorWidth*0.5f;
		}
	if(style==RAISED)
		{
		p1[2]+=separatorWidth*0.5f;
		p2[2]+=separatorWidth*0.5f;
		}
	else
		{
		p1[2]-=separatorWidth*0.5f;
		p2[2]-=separatorWidth*0.5f;
		}
	}

void Separator::draw(GLContextData& contextData) const
	{
	/* Draw the parent class widget: */
	Widget::draw(contextData);
	
	/* Draw the margin around the separator: */
	glColor(backgroundColor);
	glBegin(GL_QUAD_STRIP);
	glNormal3f(0.0f,0.0f,1.0f);
	glVertex(separator.getCorner(0));
	glVertex(getInterior().getCorner(0));
	glVertex(separator.getCorner(1));
	glVertex(getInterior().getCorner(1));
	glVertex(separator.getCorner(3));
	glVertex(getInterior().getCorner(3));
	glVertex(separator.getCorner(2));
	glVertex(getInterior().getCorner(2));
	glVertex(separator.getCorner(0));
	glVertex(getInterior().getCorner(0));
	glEnd();
	
	/* Draw the separator sides: */
	GLfloat normalC=style==RAISED?-0.7071f:0.7071f;
	if(orientation==VERTICAL)
		{
		glBegin(GL_QUADS);
		glNormal3f(normalC,0.0f,0.7071f);
		glVertex(separator.getCorner(2));
		glVertex(separator.getCorner(0));
		glVertex(p1);
		glVertex(p2);
		glNormal3f(-normalC,0.0f,0.7071f);
		glVertex(separator.getCorner(1));
		glVertex(separator.getCorner(3));
		glVertex(p2);
		glVertex(p1);
		glEnd();
		glBegin(GL_TRIANGLES);
		glNormal3f(0.0f,normalC,0.7071f);
		glVertex(separator.getCorner(0));
		glVertex(separator.getCorner(1));
		glVertex(p1);
		glNormal3f(0.0f,-normalC,0.7071f);
		glVertex(separator.getCorner(3));
		glVertex(separator.getCorner(2));
		glVertex(p2);
		glEnd();
		}
	else
		{
		glBegin(GL_QUADS);
		glNormal3f(0.0f,normalC,0.7071f);
		glVertex(separator.getCorner(0));
		glVertex(separator.getCorner(1));
		glVertex(p2);
		glVertex(p1);
		glNormal3f(0.0f,-normalC,0.7071f);
		glVertex(separator.getCorner(3));
		glVertex(separator.getCorner(2));
		glVertex(p1);
		glVertex(p2);
		glEnd();
		glBegin(GL_TRIANGLES);
		glNormal3f(normalC,0.0f,0.7071f);
		glVertex(separator.getCorner(2));
		glVertex(separator.getCorner(0));
		glVertex(p1);
		glNormal3f(-normalC,0.0f,0.7071f);
		glVertex(separator.getCorner(1));
		glVertex(separator.getCorner(3));
		glVertex(p2);
		glEnd();
		}
	}

void Separator::setPreferredHeight(GLfloat newPreferredHeight)
	{
	/* Set the preferred height: */
	preferredHeight=newPreferredHeight;
	
	if(isManaged)
		{
		/* Try adjusting the widget size to accomodate the new preferred size: */
		parent->requestResize(this,calcNaturalSize());
		}
	else
		resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

void Separator::setMarginWidth(GLfloat newMarginWidth)
	{
	/* Set the margin width: */
	marginWidth=newMarginWidth;
	
	if(isManaged)
		{
		/* Try adjusting the widget size to accomodate the new margin width: */
		parent->requestResize(this,calcNaturalSize());
		}
	else
		resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

void Separator::setStyle(Separator::Style newStyle)
	{
	/* Set the style: */
	style=newStyle;
	
	if(isManaged)
		{
		/* Try adjusting the widget size to accomodate the new style: */
		parent->requestResize(this,calcNaturalSize());
		}
	else
		resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

void Separator::setSeparatorWidth(GLfloat newSeparatorWidth)
	{
	/* Set the separator width: */
	separatorWidth=newSeparatorWidth;
	
	if(isManaged)
		{
		/* Try adjusting the widget size to accomodate the new separator width: */
		parent->requestResize(this,calcNaturalSize());
		}
	else
		resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

}
