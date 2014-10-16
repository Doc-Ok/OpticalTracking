/***********************************************************************
ToggleButton - Class for buttons displaying a binary (on/off) status.
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

#include <GLMotif/ToggleButton.h>

namespace GLMotif {

/*****************************
Methods of class ToggleButton:
*****************************/

void ToggleButton::setArmed(bool newArmed)
	{
	/* Call the parent class widget's setArmed method: */
	DecoratedButton::setArmed(newArmed);
	
	/* Update the toggle button display: */
	setSet(set);
	}

void ToggleButton::select(void)
	{
	/* Toggle the toggle status: */
	setSet(!set);
	
	/* Call the parent class widget's select method: */
	DecoratedButton::select();
	
	/* Call the value changed callbacks: */
	ValueChangedCallbackData cbData(this,set);
	valueChangedCallbacks.call(&cbData);
	}

void ToggleButton::drawDecoration(GLContextData&) const
	{
	/* Draw the margin around the toggle: */
	glColor(backgroundColor);
	glNormal3f(0.0f,0.0f,1.0f);
	glBegin(GL_TRIANGLE_STRIP);
	glVertex(toggleOuter[0]);
	glVertex(decorationBox.getCorner(0));
	glVertex(toggleOuter[1]);
	glVertex(decorationBox.getCorner(1));
	glVertex(toggleOuter[2]);
	glVertex(decorationBox.getCorner(3));
	glVertex(toggleOuter[3]);
	glVertex(decorationBox.getCorner(2));
	glVertex(toggleOuter[0]);
	glVertex(decorationBox.getCorner(0));
	glEnd();
	
	/* Draw the toggle border: */
	Vector normal;
	if(toggleType==TOGGLE_BUTTON)
		normal=Vector(0.0f,-0.707f,0.707f);
	else
		normal=Vector(-0.577f,-0.577f,0.577f);
	if(set!=isArmed)
		{
		normal[0]=-normal[0];
		normal[1]=-normal[1];
		}
	glBegin(GL_QUADS);
	glNormal3f(normal[0],normal[1],normal[2]);
	glVertex(toggleOuter[0]);
	glVertex(toggleOuter[1]);
	glVertex(toggleInner[1]);
	glVertex(toggleInner[0]);
	glNormal3f(-normal[1],normal[0],normal[2]);
	glVertex(toggleOuter[1]);
	glVertex(toggleOuter[2]);
	glVertex(toggleInner[2]);
	glVertex(toggleInner[1]);
	glNormal3f(-normal[0],-normal[1],normal[2]);
	glVertex(toggleOuter[2]);
	glVertex(toggleOuter[3]);
	glVertex(toggleInner[3]);
	glVertex(toggleInner[2]);
	glNormal3f(normal[1],-normal[0],normal[2]);
	glVertex(toggleOuter[3]);
	glVertex(toggleOuter[0]);
	glVertex(toggleInner[0]);
	glVertex(toggleInner[3]);
	glEnd();
	
	/* Draw the toggle face: */
	if(set)
		glColor(toggleColor);
	else
		glColor(backgroundColor);
	glBegin(GL_QUADS);
	glNormal3f(0.0f,0.0f,1.0f);
	glVertex(toggleInner[0]);
	glVertex(toggleInner[1]);
	glVertex(toggleInner[2]);
	glVertex(toggleInner[3]);
	glEnd();
	}

void ToggleButton::positionToggle(void)
	{
	/* Calculate the toggle corners: */
	Vector center=decorationBox.origin;
	center[0]+=0.5f*decorationBox.size[0];
	center[1]+=0.5f*decorationBox.size[1];
	Vector innerOffset,outerOffset;
	if(toggleType==TOGGLE_BUTTON)
		{
		innerOffset[0]=0.5f*toggleWidth;
		innerOffset[1]=0.5f*toggleWidth;
		outerOffset[0]=innerOffset[0]+toggleBorderWidth;
		outerOffset[1]=innerOffset[1]+toggleBorderWidth;
		}
	else
		{
		innerOffset[0]=0.5f*toggleWidth;
		innerOffset[1]=0.0f;
		outerOffset[0]=innerOffset[0]+toggleBorderWidth;
		outerOffset[1]=0.0f;
		}
	
	/* Calculate the four outer toggle points: */
	toggleInner[0]=center;
	toggleInner[0][0]-=innerOffset[0];
	toggleInner[0][1]-=innerOffset[1];
	toggleInner[1]=center;
	toggleInner[1][0]+=innerOffset[1];
	toggleInner[1][1]-=innerOffset[0];
	toggleInner[2]=center;
	toggleInner[2][0]+=innerOffset[0];
	toggleInner[2][1]+=innerOffset[1];
	toggleInner[3]=center;
	toggleInner[3][0]-=innerOffset[1];
	toggleInner[3][1]+=innerOffset[0];
	
	/* Calculate the four outer toggle points: */
	toggleOuter[0]=center;
	toggleOuter[0][0]-=outerOffset[0];
	toggleOuter[0][1]-=outerOffset[1];
	toggleOuter[1]=center;
	toggleOuter[1][0]+=outerOffset[1];
	toggleOuter[1][1]-=outerOffset[0];
	toggleOuter[2]=center;
	toggleOuter[2][0]+=outerOffset[0];
	toggleOuter[2][1]+=outerOffset[1];
	toggleOuter[3]=center;
	toggleOuter[3][0]-=outerOffset[1];
	toggleOuter[3][1]+=outerOffset[0];
	
	/* Set the toggle raised/lowered state: */
	setSet(set);
	}

void ToggleButton::setSet(bool newSet)
	{
	set=newSet;

	/* Raise or lower the inner toggle points: */
	if(set!=isArmed)
		{
		for(int i=0;i<4;++i)
			toggleInner[i][2]=decorationBox.origin[2]-toggleBorderWidth;
		}
	else
		{
		for(int i=0;i<4;++i)
			toggleInner[i][2]=decorationBox.origin[2]+toggleBorderWidth;
		}
	}

ToggleButton::ToggleButton(const char* sName,Container* sParent,const char* sLabel,const GLFont* sFont,bool sManageChild)
	:DecoratedButton(sName,sParent,sLabel,sFont,false),
	 toggleType(TOGGLE_BUTTON),
	 set(false)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Set the toggle sizes: */
	toggleBorderWidth=ss->size*0.25f;
	toggleWidth=ss->size;
	
	/* Set the decoration width: */
	GLfloat width=2.0f*toggleBorderWidth+toggleWidth;
	setDecorationSize(Vector(width,width,0.0f));
	
	/* Set the toggle color: */
	toggleColor=ss->toggleButtonToggleColor;
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

ToggleButton::ToggleButton(const char* sName,Container* sParent,const char* sLabel,bool sManageChild)
	:DecoratedButton(sName,sParent,sLabel,false),
	 toggleType(TOGGLE_BUTTON),
	 set(false)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Set the toggle sizes: */
	toggleBorderWidth=ss->size*0.25f;
	toggleWidth=ss->size;
	
	/* Set the decoration width: */
	GLfloat width=2.0f*toggleBorderWidth+toggleWidth;
	setDecorationSize(Vector(width,width,0.0f));
	
	/* Set the toggle color: */
	toggleColor=ss->toggleButtonToggleColor;
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

ZRange ToggleButton::calcZRange(void) const
	{
	/* Calculate the parent class widget's z range: */
	ZRange zRange=DecoratedButton::calcZRange();
	
	/* Adjust for the popping in/out of the toggle: */
	zRange+=ZRange(-getBorderWidth()-toggleBorderWidth,getBorderWidth()+toggleBorderWidth);
	
	return zRange;
	}
	
void ToggleButton::resize(const Box& newExterior)
	{
	/* Resize the parent class widget: */
	DecoratedButton::resize(newExterior);
	
	/* Position the toggle button: */
	positionToggle();
	}

void ToggleButton::setToggleType(ToggleButton::ToggleType newToggleType)
	{
	toggleType=newToggleType;
	
	/* Position the toggle button: */
	positionToggle();
	
	/* Invalidate the visual representation: */
	update();
	}

void ToggleButton::setToggleBorderWidth(GLfloat newToggleBorderWidth)
	{
	toggleBorderWidth=newToggleBorderWidth;
	
	/* Set the decoration width: */
	GLfloat width=2.0f*toggleBorderWidth+toggleWidth;
	setDecorationSize(Vector(width,width,0.0f));
	}

void ToggleButton::setToggleWidth(GLfloat newToggleWidth)
	{
	toggleWidth=newToggleWidth;
	
	/* Set the decoration width: */
	GLfloat width=2.0f*toggleBorderWidth+toggleWidth;
	setDecorationSize(Vector(width,width,0.0f));
	}

}
