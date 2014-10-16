/***********************************************************************
Label - Class for (text) labels.
Copyright (c) 2001-2014 Oliver Kreylos

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

#include <string.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/Container.h>

#include <GLMotif/Label.h>

namespace GLMotif {

/**********************
Methods of class Label:
**********************/

void Label::positionLabel(void)
	{
	/* Reset the label box: */
	label.resetBox();
	
	/* Position the label box according to the alignment parameters: */
	Box labelSpace=getInterior();
	labelSpace.origin[0]+=marginWidth+leftInset;
	labelSpace.size[0]-=marginWidth+leftInset+rightInset+marginWidth;
	labelSpace.origin[1]+=marginWidth;
	labelSpace.size[1]-=marginWidth+marginWidth;
	Vector newOrigin=labelSpace.origin;
	switch(hAlignment)
		{
		case GLFont::Left:
			break;
		
		case GLFont::Center:
			newOrigin[0]+=0.5f*(labelSpace.size[0]-label.getLabelSize()[0]);
			break;
		
		case GLFont::Right:
			newOrigin[0]+=labelSpace.size[0]-label.getLabelSize()[0];
			break;
		}
	switch(vAlignment)
		{
		case GLFont::Bottom:
			break;
		
		case GLFont::Baseline:
			/* Don't know what to do about it. */
			break;
		
		case GLFont::VCenter:
			newOrigin[1]+=0.5f*(labelSpace.size[1]-label.getLabelSize()[1]);
			break;
		
		case GLFont::Top:
			newOrigin[1]+=labelSpace.size[1]-label.getLabelSize()[1];
			break;
		}
	label.setOrigin(newOrigin);
	
	/* Clip the label to the maximum label space: */
	label.clipBox(labelSpace);
	}

void Label::setInsets(GLfloat newLeftInset,GLfloat newRightInset)
	{
	/* Set the insets: */
	leftInset=newLeftInset;
	rightInset=newRightInset;
	
	if(isManaged)
		{
		/* Try adjusting the widget size to accomodate the new insets: */
		parent->requestResize(this,calcNaturalSize());
		}
	else
		resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

void Label::updateColors(void)
	{
	/* Calculate the foreground color based on the widget's enabled state: */
	Color fg=enabledForegroundColor;
	if(!isEnabled())
		{
		/* Blend the foreground color and the background color: */
		for(int i=0;i<4;++i)
			fg[i]=(backgroundColor[i]+enabledForegroundColor[i])*0.5f;
		}
	
	/* Set the widget's and label's foreground color: */
	Widget::setForegroundColor(fg);
	label.setForeground(fg);
	}

Label::Label(const char* sName,Container* sParent,const char* sLabel,const GLFont* sFont,bool sManageChild)
	:Widget(sName,sParent,false),
	 marginWidth(0.0f),
	 leftInset(0.0f),rightInset(0.0f),
	 enabledForegroundColor(foregroundColor),
	 label(sLabel,*sFont),
	 hAlignment(GLFont::Left),vAlignment(GLFont::VCenter)
	{
	/* Set the label's colors: */
	label.setBackground(backgroundColor);
	label.setForeground(foregroundColor);
	
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Label defaults to no border: */
	setBorderWidth(0.0f);
	
	/* Get the margin width: */
	marginWidth=ss->labelMarginWidth;
	
	/* Initialize the minimum size: */
	minSize[0]=minSize[1]=0.0f;
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

Label::Label(const char* sName,Container* sParent,const char* sLabel,bool sManageChild)
	:Widget(sName,sParent,false),
	 marginWidth(0.0f),
	 leftInset(0.0f),rightInset(0.0f),
	 enabledForegroundColor(foregroundColor),
	 hAlignment(GLFont::Left),vAlignment(GLFont::VCenter)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Set the label: */
	label.setString(sLabel,*ss->font);
	
	/* Set the label's colors: */
	label.setBackground(backgroundColor);
	label.setForeground(foregroundColor);
	
	/* Label defaults to no border: */
	setBorderWidth(0.0f);
	
	/* Get the margin width: */
	marginWidth=ss->labelMarginWidth;
	
	/* Initialize the minimum size: */
	minSize[0]=minSize[1]=0.0f;
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

Label::Label(const char* sName,Container* sParent,const char* sLabelBegin,const char* sLabelEnd,bool sManageChild)
	:Widget(sName,sParent,false),
	 marginWidth(0.0f),
	 leftInset(0.0f),rightInset(0.0f),
	 enabledForegroundColor(foregroundColor),
	 hAlignment(GLFont::Left),vAlignment(GLFont::VCenter)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Set the label: */
	label.setString(sLabelBegin,sLabelEnd,*ss->font);
	
	/* Set the label's colors: */
	label.setBackground(backgroundColor);
	label.setForeground(foregroundColor);
	
	/* Label defaults to no border: */
	setBorderWidth(0.0f);
	
	/* Get the margin width: */
	marginWidth=ss->labelMarginWidth;
	
	/* Initialize the minimum size: */
	minSize[0]=minSize[1]=0.0f;
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

Vector Label::calcNaturalSize(void) const
	{
	/* Return size of text plus margin: */
	Vector result=label.calcNaturalSize();
	result[0]+=2.0f*marginWidth+leftInset+rightInset;
	result[1]+=2.0f*marginWidth;
	
	/* Check against the minimum size: */
	for(int i=0;i<2;++i)
		if(result[i]<minSize[i])
			result[i]=minSize[i];
	
	return calcExteriorSize(result);
	}

void Label::resize(const Box& newExterior)
	{
	/* Resize the parent class widget: */
	Widget::resize(newExterior);
	
	/* Adjust the label position: */
	positionLabel();
	}

void Label::setBackgroundColor(const Color& newBackgroundColor)
	{
	/* Set the widget's and label's background color: */
	Widget::setBackgroundColor(newBackgroundColor);
	label.setBackground(newBackgroundColor);
	
	/* Update the foreground colors: */
	updateColors();
	}

void Label::setForegroundColor(const Color& newForegroundColor)
	{
	/* Set the enabled-state foreground color: */
	enabledForegroundColor=newForegroundColor;
	
	/* Update the foreground colors: */
	updateColors();
	}

void Label::draw(GLContextData& contextData) const
	{
	/* Draw parent class decorations: */
	Widget::draw(contextData);
	
	/* Draw the label margin: */
	glColor(backgroundColor);
	glBegin(GL_QUAD_STRIP);
	glNormal3f(0.0f,0.0f,1.0f);
	glVertex(label.getLabelBox().getCorner(0));
	glVertex(getInterior().getCorner(0));
	glVertex(label.getLabelBox().getCorner(1));
	glVertex(getInterior().getCorner(1));
	glVertex(label.getLabelBox().getCorner(3));
	glVertex(getInterior().getCorner(3));
	glVertex(label.getLabelBox().getCorner(2));
	glVertex(getInterior().getCorner(2));
	glVertex(label.getLabelBox().getCorner(0));
	glVertex(getInterior().getCorner(0));
	glEnd();
	
	/* Draw the label itself: */
	label.draw(contextData);
	}

void Label::setEnabled(bool newEnabled)
	{
	/* Call base class method: */
	Widget::setEnabled(newEnabled);
	
	/* Update the foreground colors: */
	updateColors();
	}

void Label::setMarginWidth(GLfloat newMarginWidth)
	{
	/* Set the new margin width: */
	marginWidth=newMarginWidth;
	
	if(isManaged)
		{
		/* Try adjusting the widget size to accomodate the new margin width: */
		parent->requestResize(this,calcNaturalSize());
		}
	else
		resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

void Label::setMinSize(GLfloat newMinWidth,GLfloat newMinHeight)
	{
	/* Set the new minimum sizes: */
	minSize[0]=newMinWidth;
	minSize[1]=newMinHeight;
	
	/* Check if the widget violates the new minimum sizes: */
	if(getInterior().size[0]<minSize[0]||getInterior().size[1]<minSize[1])
		{
		if(isManaged)
			{
			/* Try adjusting the widget size to accomodate the new margin width: */
			parent->requestResize(this,calcNaturalSize());
			}
		else
			resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
		}
	}

void Label::setHAlignment(GLFont::HAlignment newHAlignment)
	{
	/* Set the horizontal alignment: */
	hAlignment=newHAlignment;
	
	/* Update the label position: */
	positionLabel();
	
	/* Invalidate the visual representation: */
	update();
	}

void Label::setVAlignment(GLFont::VAlignment newVAlignment)
	{
	/* Set the vertical alignment: */
	vAlignment=newVAlignment;
	
	/* Update the label position: */
	positionLabel();
	
	/* Invalidate the visual representation: */
	update();
	}

void Label::setString(const char* newLabelBegin,const char* newLabelEnd)
	{
	/* Update the label text: */
	label.setString(newLabelBegin,newLabelEnd);
	
	if(isManaged)
		{
		/* Try adjusting the widget size to accomodate the new label: */
		parent->requestResize(this,calcNaturalSize());
		}
	else
		resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

void Label::setString(const char* newLabel)
	{
	setString(newLabel,newLabel+strlen(newLabel));
	}

}
