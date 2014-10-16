/***********************************************************************
HSVColorSelector - Class for widgets to display and select colors based
on the HSV color model.
Copyright (c) 2012-2013 Oliver Kreylos

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

#include <GLMotif/HSVColorSelector.h>

#include <Math/Math.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GL/GLLightTracker.h>
#include <GL/GLContextData.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/Event.h>
#include <GLMotif/Container.h>

namespace GLMotif {

/*********************************
Methods of class HSVColorSelector:
*********************************/

void HSVColorSelector::sliderDraggingCallback(DragWidget::DraggingCallbackData* cbData)
	{
	/* Forward the callback to our own listeners: */
	DraggingCallbackData myCbData(this,cbData->reason);
	draggingCallbacks.call(&myCbData);
	}

void HSVColorSelector::sliderValueChangedCallback(Slider::ValueChangedCallbackData* cbData)
	{
	/* Get the new current value from the slider: */
	currentValue=cbData->value;
	
	/* Update the visual representation: */
	update();
	
	/* Call the value changed callbacks: */
	ValueChangedCallbackData myCbData(this,getCurrentColor());
	valueChangedCallbacks.call(&myCbData);
	}

HSVColorSelector::HSVColorSelector(const char* sName,Container* sParent,bool sManageChild)
	:Container(sName,sParent,false),
	 slider(new Slider("Slider",this,Slider::VERTICAL,0.0f,false)),
	 snapping(false)
	{
	/* Initialize the widget's layout: */
	const StyleSheet* ss=getStyleSheet();
	marginWidth=ss->size*0.5f;
	preferredSize=ss->size*6.0f;
	indicatorSize=ss->size*0.5f;
	
	/* Initialize the current color to white: */
	currentValue=1.0f;
	currentColorPos[0]=0.0f;
	currentColorPos[1]=0.0f;
	
	/* Initialize the slider: */
	slider->setBorderWidth(0.0f);
	slider->setBorderType(PLAIN);
	slider->setValueRange(0.0f,1.0f,0.0f);
	slider->setValue(currentValue);
	slider->getDraggingCallbacks().add(this,&HSVColorSelector::sliderDraggingCallback);
	slider->getValueChangedCallbacks().add(this,&HSVColorSelector::sliderValueChangedCallback);
	
	/* Manage the slider: */
	slider->manageChild();
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

HSVColorSelector::~HSVColorSelector(void)
	{
	/* Delete the slider: */
	deleteChild(slider);
	}

Vector HSVColorSelector::calcNaturalSize(void) const
	{
	/* Calculate the width and height of the color hexagon: */
	Vector result;
	result[0]=Math::cos(Math::rad(30.0f))*preferredSize;
	result[1]=preferredSize;
	result[2]=0.0f;
	
	/* Add the margin: */
	result[0]+=marginWidth*2.0f;
	result[1]+=marginWidth*2.0f;
	
	/* Add in the size of the slider: */
	Vector sliderSize=slider->calcNaturalSize();
	result[0]+=sliderSize[0];
	if(result[1]<sliderSize[1])
		result[1]=sliderSize[1];
	
	/* Return the exterior size: */
	return calcExteriorSize(result);
	}

ZRange HSVColorSelector::calcZRange(void) const
	{
	/* Return parent class' z range: */
	ZRange myZRange=Widget::calcZRange();
	
	/* Adjust for color indicator: */
	myZRange+=ZRange(getInterior().origin[2],getInterior().origin[2]+indicatorSize);
	
	/* Adjust for the slider: */
	myZRange+=slider->calcZRange();
	
	return myZRange;
	}

void HSVColorSelector::resize(const Box& newExterior)
	{
	/* Resize the parent class widget: */
	Container::resize(newExterior);
	
	/* Position the slider: */
	Box sliderBox=getInterior();
	Vector sliderSize=slider->calcNaturalSize();
	sliderBox.origin[0]+=sliderBox.size[0]-sliderSize[0];
	sliderBox.size[0]=sliderSize[0];
	slider->resize(sliderBox);
	
	/* Re-layout the color hexagon: */
	hexBox=getInterior();
	hexBox.size[0]-=sliderSize[0];
	hexCenter=hexBox.origin;
	for(int i=0;i<2;++i)
		hexCenter[i]+=hexBox.size[i]*0.5f;
	hexRadius=(hexBox.size[0]-marginWidth*2.0f)/Math::cos(Math::rad(30.0f));
	if(hexRadius>hexBox.size[1]-marginWidth*2.0f)
		hexRadius=hexBox.size[1]-marginWidth*2.0f;
	hexRadius*=0.5f;
	}

void HSVColorSelector::draw(GLContextData& contextData) const
	{
	/* Draw the parent class widget: */
	Container::draw(contextData);
	
	/* Draw the margin around the color hexagon and the slider: */
	glColor(backgroundColor);
	glBegin(GL_TRIANGLES);
	glNormal3f(0.0f,0.0f,1.0f);
	glVertex(getInterior().getCorner(0));
	glVertex(getInterior().getCorner(1));
	glVertex(hexBox.getCorner(1));
	glVertex(getInterior().getCorner(3));
	glVertex(getInterior().getCorner(2));
	glVertex(hexBox.getCorner(3));
	glEnd();
	
	/* Draw the margin area with the background color: */
	GLfloat xs=Math::cos(Math::rad(30.0f))*hexRadius;
	GLfloat ys=Math::sin(Math::rad(30.0f))*hexRadius;
	glBegin(GL_TRIANGLE_STRIP);
	glVertex3f(hexCenter[0]-xs,hexCenter[1]-ys,hexCenter[2]);
	glVertex(hexBox.getCorner(0));
	glVertex3f(hexCenter[0],hexCenter[1]-hexRadius,hexCenter[2]);
	glVertex(hexBox.getCorner(1));
	glVertex3f(hexCenter[0]+xs,hexCenter[1]-ys,hexCenter[2]);
	glVertex(hexBox.getCorner(3));
	glVertex3f(hexCenter[0]+xs,hexCenter[1]+ys,hexCenter[2]);
	glVertex(hexBox.getCorner(3));
	glVertex3f(hexCenter[0],hexCenter[1]+hexRadius,hexCenter[2]);
	glVertex(hexBox.getCorner(2));
	glVertex3f(hexCenter[0]-xs,hexCenter[1]+ys,hexCenter[2]);
	glVertex(hexBox.getCorner(0));
	glVertex3f(hexCenter[0]-xs,hexCenter[1]-ys,hexCenter[2]);
	glEnd();
	
	/* Draw the color hexagon with lighting disabled: */
	GLLightTracker* lt=contextData.getLightTracker();
	if(lt->isLightingEnabled())
		glDisable(GL_LIGHTING);
	glBegin(GL_TRIANGLE_FAN);
	
	/* Center point is white: */
	glColor3f(currentValue,currentValue,currentValue);
	glVertex(hexCenter);
	
	/* Top corner is red: */
	glColor3f(currentValue,0.0f,0.0f);
	glVertex3f(hexCenter[0],hexCenter[1]+hexRadius,hexCenter[2]);
	
	/* Top-left corner is yellow: */
	glColor3f(currentValue,currentValue,0.0f);
	glVertex3f(hexCenter[0]-xs,hexCenter[1]+ys,hexCenter[2]);
	
	/* Bottom-left corner is green: */
	glColor3f(0.0f,currentValue,0.0f);
	glVertex3f(hexCenter[0]-xs,hexCenter[1]-ys,hexCenter[2]);
	
	/* Bottom corner is cyan: */
	glColor3f(0.0f,currentValue,currentValue);
	glVertex3f(hexCenter[0],hexCenter[1]-hexRadius,hexCenter[2]);
	
	/* Bottom-right corner is blue: */
	glColor3f(0.0f,0.0f,currentValue);
	glVertex3f(hexCenter[0]+xs,hexCenter[1]-ys,hexCenter[2]);
	
	/* Top-right corner is magenta: */
	glColor3f(currentValue,0.0f,currentValue);
	glVertex3f(hexCenter[0]+xs,hexCenter[1]+ys,hexCenter[2]);
	
	/* Top corner is red: */
	glColor3f(currentValue,0.0f,0.0f);
	glVertex3f(hexCenter[0],hexCenter[1]+hexRadius,hexCenter[2]);
	
	glEnd();
	if(lt->isLightingEnabled())
		glEnable(GL_LIGHTING);
	
	/* Draw the current color indicator: */
	Vector indicatorPos=hexCenter;
	indicatorPos[0]+=currentColorPos[0]*hexRadius;
	indicatorPos[1]+=currentColorPos[1]*hexRadius;
	GLfloat nl=Math::sqrt(3.0f)/3.0f;
	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_TRIANGLES);
	glNormal3f(-nl,nl,nl);
	glVertex3f(indicatorPos[0],indicatorPos[1],indicatorPos[2]+indicatorSize);
	glVertex3f(indicatorPos[0],indicatorPos[1]+indicatorSize,indicatorPos[2]);
	glVertex3f(indicatorPos[0]-indicatorSize,indicatorPos[1],indicatorPos[2]);
	
	glNormal3f(-nl,-nl,nl);
	glVertex3f(indicatorPos[0],indicatorPos[1],indicatorPos[2]+indicatorSize);
	glVertex3f(indicatorPos[0]-indicatorSize,indicatorPos[1],indicatorPos[2]);
	glVertex3f(indicatorPos[0],indicatorPos[1]-indicatorSize,indicatorPos[2]);
	
	glNormal3f(nl,-nl,nl);
	glVertex3f(indicatorPos[0],indicatorPos[1],indicatorPos[2]+indicatorSize);
	glVertex3f(indicatorPos[0],indicatorPos[1]-indicatorSize,indicatorPos[2]);
	glVertex3f(indicatorPos[0]+indicatorSize,indicatorPos[1],indicatorPos[2]);
	
	glNormal3f(nl,nl,nl);
	glVertex3f(indicatorPos[0],indicatorPos[1],indicatorPos[2]+indicatorSize);
	glVertex3f(indicatorPos[0]+indicatorSize,indicatorPos[1],indicatorPos[2]);
	glVertex3f(indicatorPos[0],indicatorPos[1]+indicatorSize,indicatorPos[2]);
	glEnd();
	
	/* Draw the slider: */
	slider->draw(contextData);
	}

bool HSVColorSelector::findRecipient(Event& event)
	{
	/* Check the slider first: */
	if(slider->findRecipient(event))
		return true;
	
	/* Check the color hexagon: */
	if(isDragging())
		return overrideRecipient(this,event);
	else
		return Widget::findRecipient(event);
	}

void HSVColorSelector::pointerButtonDown(Event& event)
	{
	/* Convert the widget point to hexagon coordinates: */
	GLfloat colorPos[2];
	for(int i=0;i<2;++i)
		colorPos[i]=(event.getWidgetPoint().getPoint()[i]-hexCenter[i])/hexRadius;
	
	/* Calculate the (unnormalized) distance to the current color position: */
	GLfloat dist=0.0f;
	for(int i=0;i<2;++i)
		dist+=Math::sqr(currentColorPos[i]-colorPos[i]);
	dist=Math::sqrt(dist)*hexRadius;
	
	/* Enable snapping if the click is outside the current color indicator: */
	snapping=dist>indicatorSize;
	
	/* Start dragging: */
	startDragging(event);
	
	/* Generate the first value change event: */
	pointerMotion(event);
	}

void HSVColorSelector::pointerButtonUp(Event& event)
	{
	/* Stop dragging: */
	stopDragging(event);
	}

void HSVColorSelector::pointerMotion(Event& event)
	{
	if(isDragging())
		{
		/* Convert the widget point to hexagon coordinates: */
		for(int i=0;i<2;++i)
			currentColorPos[i]=(event.getWidgetPoint().getPoint()[i]-hexCenter[i])/hexRadius;
		
		/* Clip the new current color position against the hexagon's border: */
		GLfloat c=Math::cos(Math::rad(30.0f));
		GLfloat s=Math::sin(Math::rad(30.0f));
		GLfloat d0=-currentColorPos[0];
		if(d0>=0.0f)
			{
			/* Clip against left border: */
			if(d0>c)
				{
				currentColorPos[0]*=c/d0;
				currentColorPos[1]*=c/d0;
				}
			
			/* Clip against top-left border: */
			GLfloat d1=currentColorPos[1]*c-currentColorPos[0]*s;
			if(d1>c)
				{
				currentColorPos[0]*=c/d1;
				currentColorPos[1]*=c/d1;
				}
			
			/* Clip against the bottom-left border: */
			GLfloat d2=currentColorPos[1]*c+currentColorPos[0]*s;
			if(d2<-c)
				{
				currentColorPos[0]*=-c/d2;
				currentColorPos[1]*=-c/d2;
				}
			}
		else
			{
			/* Clip against right border: */
			if(d0<-c)
				{
				currentColorPos[0]*=-c/d0;
				currentColorPos[1]*=-c/d0;
				}
			
			/* Clip against bottom-right border: */
			GLfloat d1=currentColorPos[1]*c-currentColorPos[0]*s;
			if(d1<-c)
				{
				currentColorPos[0]*=-c/d1;
				currentColorPos[1]*=-c/d1;
				}
			
			/* Clip against the top-right border: */
			GLfloat d2=currentColorPos[1]*c+currentColorPos[0]*s;
			if(d2>c)
				{
				currentColorPos[0]*=c/d2;
				currentColorPos[1]*=c/d2;
				}
			}
		
		if(snapping)
			{
			/* Snap the current color against the 7 "pure" colors: */
			GLfloat closestDist=Math::sqr(currentColorPos[0])+Math::sqr(currentColorPos[1]);
			GLfloat closestColor[2];
			closestColor[0]=closestColor[1]=0.0f;
			for(int i=0;i<6;++i)
				{
				/* Calculate the pure color position: */
				GLfloat angle=2.0f*Math::Constants<GLfloat>::pi*GLfloat(i)/6.0f;
				GLfloat x=Math::sin(angle);
				GLfloat y=Math::cos(angle);
				GLfloat dist=Math::sqr(currentColorPos[0]-x)+Math::sqr(currentColorPos[1]-y);
				if(closestDist>dist)
					{
					closestDist=dist;
					closestColor[0]=x;
					closestColor[1]=y;
					}
				}
			for(int i=0;i<2;++i)
				currentColorPos[i]=closestColor[i];
			}
		
		/* Call the value changed callbacks: */
		ValueChangedCallbackData cbData(this,getCurrentColor());
		valueChangedCallbacks.call(&cbData);
		
		/* Update the visual representation: */
		update();
		}
	}

void HSVColorSelector::addChild(Widget* newChild)
	{
	/* Only the dedicated children can call this, and they get managed in the constructor, so ignore this */
	}

void HSVColorSelector::removeChild(Widget* removeChild)
	{
	/* This should never be called, so simply ignore it */
	}

void HSVColorSelector::requestResize(Widget* child,const Vector& newExteriorSize)
	{
	if(isManaged)
		{
		/* Adjust the widget's layout: */
		Vector size=newExteriorSize; // Size requested by slider, the only child
		size[0]+=hexBox.size[0];
		if(size[1]<hexBox.size[1])
			size[1]=hexBox.size[1];
		
		/* Resize the widget: */
		parent->requestResize(this,calcExteriorSize(size));
		}
	else
		{
		/* Just resize the child: */
		child->resize(Box(child->getExterior().origin,newExteriorSize));
		}
	}

Widget* HSVColorSelector::getFirstChild(void)
	{
	/* Return the slider: */
	return slider;
	}

Widget* HSVColorSelector::getNextChild(Widget*)
	{
	/* There is only one child: */
	return 0;
	}

void HSVColorSelector::setMarginWidth(GLfloat newMarginWidth)
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

void HSVColorSelector::setPreferredSize(GLfloat newPreferredSize)
	{
	/* Set the new preferred size: */
	preferredSize=newPreferredSize;
	
	if(isManaged)
		{
		/* Try adjusting the widget size to accomodate the new preferred size: */
		parent->requestResize(this,calcNaturalSize());
		}
	else
		resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

void HSVColorSelector::setIndicatorSize(GLfloat newIndicatorSize)
	{
	/* Set the new indicator size: */
	indicatorSize=newIndicatorSize;
	}

Color HSVColorSelector::getCurrentColor(void) const
	{
	/* Calculate distances of current color position from the hexagon's separating lines: */
	GLfloat c=Math::cos(Math::rad(30.0f));
	GLfloat t=Math::sin(Math::rad(30.0f))/c;
	GLfloat d0=-currentColorPos[0]/c;
	GLfloat d1=currentColorPos[1]-currentColorPos[0]*t;
	GLfloat d2=currentColorPos[1]+currentColorPos[0]*t;
	
	/*********************************************************************
	The following code uses barycentric interpolation in each of the color
	hexagon's six triangles to determine the selected RGB color. It's best
	to draw a diagram of the hexagon, and label the corners and the center
	point with their RGB color values to understand this.
	*********************************************************************/
	
	/* Find the hexagon triangle containing the current color position and interpolate colors using barycentric weights: */
	Color result;
	if(d0>=0.0f)
		{
		if(d1>=0.0f)
			{
			if(d2>=0.0f)
				{
				/* In top-left triangle: */
				result[0]=currentValue;
				result[1]=(1.0f-d2)*currentValue;
				result[2]=(1.0f-d0-d2)*currentValue;
				}
			else
				{
				/* In left triangle: */
				result[0]=(1.0f+d2)*currentValue;
				result[1]=currentValue;
				result[2]=(1.0f-d1+d2)*currentValue;
				}
			}
		else
			{
			/* In bottom-left triangle: */
			result[0]=(1.0f-d0+d1)*currentValue;
			result[1]=currentValue;
			result[2]=(1.0f-d0)*currentValue;
			}
		}
	else
		{
		if(d2>=0.0f)
			{
			if(d1>=0.0f)
				{
				/* In top-right triangle: */
				result[0]=currentValue;
				result[1]=(1.0f+d0-d1)*currentValue;
				result[2]=(1.0f-d1)*currentValue;
				}
			else
				{
				/* In right triangle: */
				result[0]=(1.0f+d1)*currentValue;
				result[1]=(1.0f+d1-d2)*currentValue;
				result[2]=currentValue;
				}
			}
		else
			{
			/* In bottom-right triangle: */
			result[0]=(1.0f+d0+d2)*currentValue;
			result[1]=(1.0f+d0)*currentValue;
			result[2]=currentValue;
			}
		}
	result[3]=1.0f;
	
	return result;
	}

void HSVColorSelector::setCurrentColor(const Color& newColor)
	{
	/* Calculate the new color's brightness value and smallest color component: */
	currentValue=newColor[0];
	int minComp=0;
	for(int i=1;i<3;++i)
		{
		if(currentValue<newColor[i])
			currentValue=newColor[i];
		if(newColor[minComp]>newColor[i])
			minComp=i;
		}
	
	if(currentValue>0.0f)
		{
		/* Scale the color to the full hue/saturation range: */
		GLfloat nc[3];
		for(int i=0;i<3;++i)
			nc[i]=newColor[i]/currentValue;
		
		/*******************************************************************
		The following code uses barycentric interpolation in each of the
		color hexagon's six triangles to determine the 2D position of the
		color indicator based on the new RGB color.
		It's best to draw a diagram of the hexagon, and label the corners
		and the center point with their RGB color values to understand this.
		*******************************************************************/
		
		/* Determine the color triangle containing the new color: */
		GLfloat s=Math::sin(Math::rad(30.0f));
		GLfloat c=Math::cos(Math::rad(30.0f));
		if(minComp==0) // Color is in green-cyan-blue triangles
			{
			if(nc[1]>=nc[2])
				{
				/* Color is in green-cyan triangle: */
				currentColorPos[0]=(1.0f-nc[2])*-c;
				currentColorPos[1]=(1.0f-nc[2])*-s-(nc[2]-nc[0]);
				}
			else
				{
				/* Color is in cyan-blue triangle: */
				currentColorPos[0]=(1.0f-nc[1])*c;
				currentColorPos[1]=(1.0f-nc[1])*-s-(nc[1]-nc[0]);
				}
			}
		else if(minComp==1) // Color is in blue-magenta-red triangles
			{
			if(nc[2]>=nc[0])
				{
				/* Color is in blue-magenta triangle: */
				currentColorPos[0]=(1.0f-nc[1])*c;
				currentColorPos[1]=(2.0f*nc[0]-nc[1]-1.0f)*s;
				}
			else
				{
				/* Color is in magenta-red triangle: */
				currentColorPos[0]=(nc[2]-nc[1])*c;
				currentColorPos[1]=(nc[2]-nc[1])*s+(1.0f-nc[2]);
				}
			}
		else // Color is in red-yellow-green triangles
			{
			if(nc[0]>=nc[1])
				{
				/* Color is in red-yellow triangle: */
				currentColorPos[0]=(nc[1]-nc[2])*-c;
				currentColorPos[1]=(nc[1]-nc[2])*s+(1.0f-nc[1]);
				}
			else
				{
				/* Color is in yellow-green triangle: */
				currentColorPos[0]=(1.0f-nc[2])*-c;
				currentColorPos[1]=(2.0f*nc[0]-nc[2]-1.0f)*s;
				}
			}
		}
	else
		{
		/* Set the color position for black to the center: */
		currentColorPos[0]=currentColorPos[1]=0.0f;
		}
	
	/* Set the slider's current value: */
	slider->setValue(currentValue);
	}

}
