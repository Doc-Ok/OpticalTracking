/***********************************************************************
DecoratedButton - Class for buttons with decoration around their labels.
Copyright (c) 2001-2010 Oliver Kreylos

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

#include <GLMotif/DecoratedButton.h>

namespace GLMotif {

/********************************
Methods of class DecoratedButton:
********************************/

DecoratedButton::DecoratedButton(const char* sName,Container* sParent,const char* sLabel,const GLFont* sFont,bool sManageChild)
	:Button(sName,sParent,sLabel,sFont,false),
	decorationSize(Vector(0.0f,0.0f,0.0f)),decorationPosition(DECORATION_LEFT)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Set the decoration spacing: */
	spacing=ss->decoratedButtonSpacing;
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

DecoratedButton::DecoratedButton(const char* sName,Container* sParent,const char* sLabel,bool sManageChild)
	:Button(sName,sParent,sLabel,false),
	decorationSize(Vector(0.0f,0.0f,0.0f)),decorationPosition(DECORATION_LEFT)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Set the decoration spacing: */
	spacing=ss->decoratedButtonSpacing;
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

Vector DecoratedButton::calcNaturalSize(void) const
	{
	/* Return size of decoration and text plus margin: */
	Vector result=label.calcNaturalSize();
	if(result[1]<decorationSize[1])
		result[1]=decorationSize[1];
	result[0]+=2.0f*marginWidth+leftInset+rightInset;
	result[1]+=2.0f*marginWidth;
	
	return calcExteriorSize(result);
	}

void DecoratedButton::resize(const Box& newExterior)
	{
	/* Resize the parent class widget: */
	Button::resize(newExterior);
	
	/* Position the decoration: */
	decorationBox=getInterior().inset(Vector(marginWidth,marginWidth,0.0f));
	if(decorationPosition==DECORATION_RIGHT)
		decorationBox.origin[0]+=decorationBox.size[0]-decorationSize[0];
	decorationBox.size[0]=decorationSize[0];
	}

void DecoratedButton::draw(GLContextData& contextData) const
	{
	/* Draw the base class widget: */
	Widget::draw(contextData);
	
	/* Draw the margin and label separator: */
	glColor(backgroundColor);
	
	if(decorationPosition==DECORATION_LEFT)
		{
		/* Draw the top left margin part: */
		glBegin(GL_TRIANGLE_FAN);
		glNormal3f(0.0f,0.0f,1.0f);
		glVertex(getInterior().getCorner(2));
		glVertex(getInterior().getCorner(0));
		glVertex(decorationBox.getCorner(0));
		glVertex(decorationBox.getCorner(2));
		glVertex(decorationBox.getCorner(3));
		glVertex(label.getLabelBox().getCorner(2));
		glVertex(label.getLabelBox().getCorner(3));
		glVertex(getInterior().getCorner(3));
		glEnd();
		
		/* Draw the bottom left margin part: */
		glBegin(GL_TRIANGLE_FAN);
		glVertex(getInterior().getCorner(1));
		glVertex(getInterior().getCorner(3));
		glVertex(label.getLabelBox().getCorner(3));
		glVertex(label.getLabelBox().getCorner(1));
		glVertex(label.getLabelBox().getCorner(0));
		glVertex(decorationBox.getCorner(1));
		glVertex(decorationBox.getCorner(0));
		glVertex(getInterior().getCorner(0));
		glEnd();
		
		/* Draw the label separator: */
		glBegin(GL_QUADS);
		glVertex(decorationBox.getCorner(3));
		glVertex(decorationBox.getCorner(1));
		glVertex(label.getLabelBox().getCorner(0));
		glVertex(label.getLabelBox().getCorner(2));
		glEnd();
		}
	else
		{
		/* Draw the top left margin part: */
		glBegin(GL_TRIANGLE_FAN);
		glNormal3f(0.0f,0.0f,1.0f);
		glVertex(getInterior().getCorner(2));
		glVertex(getInterior().getCorner(0));
		glVertex(label.getLabelBox().getCorner(0));
		glVertex(label.getLabelBox().getCorner(2));
		glVertex(label.getLabelBox().getCorner(3));
		glVertex(decorationBox.getCorner(2));
		glVertex(decorationBox.getCorner(3));
		glVertex(getInterior().getCorner(3));
		glEnd();
		
		/* Draw the bottom right margin part: */
		glBegin(GL_TRIANGLE_FAN);
		glVertex(getInterior().getCorner(1));
		glVertex(getInterior().getCorner(3));
		glVertex(decorationBox.getCorner(3));
		glVertex(decorationBox.getCorner(1));
		glVertex(decorationBox.getCorner(0));
		glVertex(label.getLabelBox().getCorner(1));
		glVertex(label.getLabelBox().getCorner(0));
		glVertex(getInterior().getCorner(0));
		glEnd();
		
		/* Draw the label separator: */
		glBegin(GL_QUADS);
		glVertex(label.getLabelBox().getCorner(3));
		glVertex(label.getLabelBox().getCorner(1));
		glVertex(decorationBox.getCorner(0));
		glVertex(decorationBox.getCorner(2));
		glEnd();
		}
	
	/* Draw the decoration and label: */
	drawDecoration(contextData);
	label.draw(contextData);
	}

void DecoratedButton::setDecorationSize(const Vector& newDecorationSize)
	{
	decorationSize=newDecorationSize;
	if(decorationPosition==DECORATION_LEFT)
		setInsets(decorationSize[0]+spacing,0.0f);
	else
		setInsets(0.0f,decorationSize[0]+spacing);
	
	/* Try adjusting the widget size to accomodate the new setting: */
	if(isManaged)
		parent->requestResize(this,calcNaturalSize());
	else
		resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

void DecoratedButton::setDecorationPosition(DecorationPosition newDecorationPosition)
	{
	decorationPosition=newDecorationPosition;
	if(decorationPosition==DECORATION_LEFT)
		setInsets(decorationSize[0]+spacing,0.0f);
	else
		setInsets(0.0f,decorationSize[0]+spacing);
	
	/* Update the label position, no resize necessary: */
	positionLabel();
	
	/* Invalidate the visual representation: */
	update();
	}

void DecoratedButton::setSpacing(GLfloat newSpacing)
	{
	spacing=newSpacing;
	if(decorationPosition==DECORATION_LEFT)
		setInsets(decorationSize[0]+spacing,0.0f);
	else
		setInsets(0.0f,decorationSize[0]+spacing);
	
	/* Try adjusting the widget size to accomodate the new setting: */
	if(isManaged)
		parent->requestResize(this,calcNaturalSize());
	else
		resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

}
