/***********************************************************************
Slider - Class for horizontal or vertical sliders.
Copyright (c) 2001-2012 Oliver Kreylos

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

#include <GLMotif/Slider.h>

#include <math.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLColorOperations.h>
#include <GL/GLVertexTemplates.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/Event.h>
#include <GLMotif/Container.h>

namespace GLMotif {

/***********************
Methods of class Slider:
***********************/

void Slider::positionShaft(void)
	{
	/* Position shaft according to widget size and slider orientation: */
	shaftBox.origin=getInterior().origin;
	shaftBox.doOffset(Vector(marginWidth,marginWidth,-shaftDepth));
	shaftBox.size[2]=shaftDepth;
	switch(orientation)
		{
		case HORIZONTAL:
			shaftBox.size[0]=getInterior().size[0]-marginWidth*2.0;
			shaftBox.origin[1]+=(getInterior().size[1]-marginWidth*2.0-shaftWidth)*0.5;
			shaftBox.size[1]=shaftWidth;
			break;
		
		case VERTICAL:
			shaftBox.origin[0]+=(getInterior().size[0]-marginWidth*2.0-shaftWidth)*0.5;
			shaftBox.size[0]=shaftWidth;
			shaftBox.size[1]=getInterior().size[1]-marginWidth*2.0;
			break;
		}
	}

void Slider::positionNotches(void)
	{
	/* Recalculate all notch positions: */
	notchPositions.clear();
	int dim=orientation==HORIZONTAL?0:1;
	for(std::vector<GLfloat>::iterator nvIt=notchValues.begin();nvIt!=notchValues.end();++nvIt)
		{
		GLfloat pos=shaftBox.origin[dim]+sliderLength*0.5f+(*nvIt-valueMin)*(shaftBox.size[dim]-sliderLength)/(valueMax-valueMin);
		notchPositions.push_back(pos);
		}
	}

void Slider::positionSlider(void)
	{
	/* Position slider handle according to widget size and slider orientation: */
	sliderBox.origin=shaftBox.origin;
	sliderBox.size[2]=sliderHeight+shaftDepth;
	GLfloat sliderPosition=(value-valueMin)/(valueMax-valueMin);
	switch(orientation)
		{
		case HORIZONTAL:
			sliderBox.origin[0]+=(shaftBox.size[0]-sliderLength)*sliderPosition;
			sliderBox.size[0]=sliderLength;
			sliderBox.origin[1]+=(shaftBox.size[1]-sliderWidth)*0.5;
			sliderBox.size[1]=sliderWidth;
			break;
		
		case VERTICAL:
			sliderBox.origin[0]+=(shaftBox.size[0]-sliderWidth)*0.5;
			sliderBox.size[0]=sliderWidth;
			sliderBox.origin[1]+=(shaftBox.size[1]-sliderLength)*sliderPosition;
			sliderBox.size[1]=sliderLength;
			break;
		}
	}

void Slider::decrement(void)
	{
	/* Calculate the new slider value: */
	GLfloat newValue;
	if(valueIncrement!=0.0f)
		{
		/* Decrement, range check, and quantize: */
		newValue=value-valueIncrement*1.001f;
		newValue=GLfloat(ceil(double(newValue)/double(valueIncrement))*double(valueIncrement));
		if(newValue<valueMin)
			newValue=valueMin;
		}
	else
		newValue=valueMin;
	
	/* Check if the old and new values straddle a notch: */
	std::vector<GLfloat>::reverse_iterator nvIt;
	for(nvIt=notchValues.rbegin();nvIt!=notchValues.rend()&&*nvIt>=value;++nvIt)
		;
	
	/* Set the new value to the notch's value: */
	if(nvIt!=notchValues.rend()&&*nvIt>newValue)
		newValue=*nvIt;
	
	if(value!=newValue)
		{
		/* Update the slider's state: */
		value=newValue;
		positionSlider();
		
		/* Call the value changed callbacks: */
		ValueChangedCallbackData cbData(this,ValueChangedCallbackData::CLICKED,value);
		valueChangedCallbacks.call(&cbData);
		
		/* Invalidate the visual representation: */
		update();
		}
	}

void Slider::increment(void)
	{
	/* Calculate the new slider value: */
	GLfloat newValue;
	if(valueIncrement!=0.0f)
		{
		/* Increment, range check, and quantize: */
		newValue=value+valueIncrement*1.001f;
		newValue=GLfloat(floor(double(newValue)/double(valueIncrement))*double(valueIncrement));
		if(newValue>valueMax)
			newValue=valueMax;
		}
	else
		newValue=valueMax;
	
	/* Check if the old and new values straddle a notch: */
	std::vector<GLfloat>::iterator nvIt;
	for(nvIt=notchValues.begin();nvIt!=notchValues.end()&&*nvIt<=value;++nvIt)
		;
	
	/* Set the new value to the notch's value: */
	if(nvIt!=notchValues.end()&&*nvIt<newValue)
		newValue=*nvIt;
	
	if(value!=newValue)
		{
		/* Update the slider's state: */
		value=newValue;
		positionSlider();
		
		/* Call the value changed callbacks: */
		ValueChangedCallbackData cbData(this,ValueChangedCallbackData::CLICKED,value);
		valueChangedCallbacks.call(&cbData);
		
		/* Invalidate the visual representation: */
		update();
		}
	}

void Slider::clickRepeatTimerEventCallback(Misc::TimerEventScheduler::CallbackData* cbData)
	{
	/* Only react to event if still in click-repeat mode: */
	if(isClicking!=0)
		{
		/* Adjust value and reposition slider: */
		if(isClicking<0)
			decrement();
		else
			increment();
		
		Misc::TimerEventScheduler* tes=getManager()->getTimerEventScheduler();
		if(tes!=0)
			{
			/* Schedule a timer event for click repeat: */
			nextClickEventTime+=0.1;
			tes->scheduleEvent(nextClickEventTime,this,&Slider::clickRepeatTimerEventCallback);
			}
		}
	}

Slider::Slider(const char* sName,Container* sParent,Slider::Orientation sOrientation,GLfloat sSliderWidth,GLfloat sShaftLength,bool sManageChild)
	:Widget(sName,sParent,false),
	 orientation(sOrientation),
	 valueMin(0.0),valueMax(1000.0),valueIncrement(1.0),value(500.0),
	 isClicking(0)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Slider defaults to no border: */
	setBorderWidth(0.0f);
	
	/* Set the slider margin: */
	marginWidth=sSliderWidth*0.25f;
	
	/* Set the slider handle dimensions: */
	sliderWidth=sSliderWidth;
	sliderLength=sliderWidth*0.5f;
	sliderHeight=sliderWidth*0.5f;
	sliderColor=ss->sliderHandleColor;
	
	/* Set the slider shaft dimensions: */
	shaftWidth=ss->sliderShaftWidth;
	shaftLength=sShaftLength;
	shaftDepth=ss->sliderShaftDepth;
	shaftColor=ss->sliderShaftColor;
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

Slider::Slider(const char* sName,Container* sParent,Slider::Orientation sOrientation,GLfloat sShaftLength,bool sManageChild)
	:Widget(sName,sParent,false),
	 orientation(sOrientation),
	 sliderHeight(0.0f),shaftDepth(0.0f),
	 valueMin(0.0),valueMax(1000.0),valueIncrement(1.0),value(500.0),
	 isClicking(0)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Slider defaults to no border: */
	setBorderWidth(0.0f);
	
	/* Set the slider margin: */
	marginWidth=ss->sliderMarginWidth;
	
	/* Set the slider handle dimensions: */
	sliderWidth=ss->sliderHandleWidth;
	sliderLength=ss->sliderHandleLength;
	sliderHeight=ss->sliderHandleHeight;
	sliderColor=ss->sliderHandleColor;
	
	/* Set the slider shaft dimensions: */
	shaftWidth=ss->sliderShaftWidth;
	shaftLength=sShaftLength;
	shaftDepth=ss->sliderShaftDepth;
	shaftColor=ss->sliderShaftColor;
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

Slider::~Slider(void)
	{
	/* Need to remove all click-repeat timer events from the event scheduler, just in case: */
	Misc::TimerEventScheduler* tes=getManager()->getTimerEventScheduler();
	if(tes!=0)
		tes->removeAllEvents(this,&Slider::clickRepeatTimerEventCallback);
	}

Vector Slider::calcNaturalSize(void) const
	{
	/* Determine width and length of the slider and shaft: */
	GLfloat width=shaftWidth;
	if(width<sliderWidth)
		width=sliderWidth;
	width+=marginWidth;
	GLfloat length=sliderLength;
	if(length<shaftLength)
		length=shaftLength;
	length+=marginWidth;
	
	/* Return size depending on slider orientation: */
	if(orientation==HORIZONTAL)
		return calcExteriorSize(Vector(length,width,0.0f));
	else
		return calcExteriorSize(Vector(width,length,0.0f));
	}

ZRange Slider::calcZRange(void) const
	{
	/* Return parent class' z range: */
	ZRange myZRange=Widget::calcZRange();
	
	/* Adjust for shaft depth and slider height: */
	myZRange+=ZRange(getInterior().origin[2]-shaftDepth,getInterior().origin[2]+sliderHeight);
	
	return myZRange;
	}

void Slider::resize(const Box& newExterior)
	{
	/* Resize the parent class widget: */
	Widget::resize(newExterior);
	
	/* Adjust the shaft, notches, and slider handle positions: */
	positionShaft();
	positionNotches();
	positionSlider();
	}

void Slider::draw(GLContextData& contextData) const
	{
	/* Draw parent class decorations: */
	Widget::draw(contextData);
	
	/* Draw the shaft margin: */
	if(notchValues.empty())
		{
		glColor(backgroundColor);
		glBegin(GL_QUAD_STRIP);
		glNormal3f(0.0f,0.0f,1.0f);
		glVertex(shaftBox.getCorner(4));
		glVertex(getInterior().getCorner(0));
		glVertex(shaftBox.getCorner(5));
		glVertex(getInterior().getCorner(1));
		glVertex(shaftBox.getCorner(7));
		glVertex(getInterior().getCorner(3));
		glVertex(shaftBox.getCorner(6));
		glVertex(getInterior().getCorner(2));
		glVertex(shaftBox.getCorner(4));
		glVertex(getInterior().getCorner(0));
		glEnd();
		}
	else
		{
		/* Calculate the notch size: */
		GLfloat ns=shaftWidth;
		if(ns<sliderWidth)
			ns=sliderWidth;
		ns+=marginWidth;
		ns=(ns-shaftWidth)*0.5f/6.0f;
		GLfloat nz=getInterior().origin[2];
		
		if(orientation==HORIZONTAL)
			{
			GLfloat ny;
			glColor(backgroundColor);
			glNormal3f(0.0f,0.0f,1.0f);
			
			/* Draw the bottom shaft margin part's bottom half: */
			ny=shaftBox.origin[1]-ns*5.0f;
			glBegin(GL_TRIANGLE_FAN);
			glVertex(getInterior().getCorner(0));
			glVertex(getInterior().getCorner(1));
			for(std::vector<GLfloat>::const_reverse_iterator npIt=notchPositions.rbegin();npIt!=notchPositions.rend();++npIt)
				{
				glVertex3f(*npIt+ns*0.5f,ny,nz);
				glVertex3f(*npIt-ns*0.5f,ny,nz);
				}
			glEnd();
			
			/* Draw the bottom shaft margin part's top half: */
			ny=shaftBox.origin[1]-ns;
			glBegin(GL_TRIANGLE_FAN);
			glVertex(shaftBox.getCorner(5));
			glVertex(shaftBox.getCorner(4));
			for(std::vector<GLfloat>::const_iterator npIt=notchPositions.begin();npIt!=notchPositions.end();++npIt)
				{
				glVertex3f(*npIt-ns*0.5f,ny,nz);
				glVertex3f(*npIt+ns*0.5f,ny,nz);
				}
			glEnd();
			
			/* Draw the top shaft margin part's bottom half: */
			ny=shaftBox.origin[1]+shaftBox.size[1]+ns;
			glBegin(GL_TRIANGLE_FAN);
			glVertex(shaftBox.getCorner(6));
			glVertex(shaftBox.getCorner(7));
			for(std::vector<GLfloat>::const_reverse_iterator npIt=notchPositions.rbegin();npIt!=notchPositions.rend();++npIt)
				{
				glVertex3f(*npIt+ns*0.5f,ny,nz);
				glVertex3f(*npIt-ns*0.5f,ny,nz);
				}
			glEnd();
			
			/* Draw the top shaft margin part's top half: */
			ny=shaftBox.origin[1]+shaftBox.size[1]+ns*5.0f;
			glBegin(GL_TRIANGLE_FAN);
			glVertex(getInterior().getCorner(3));
			glVertex(getInterior().getCorner(2));
			for(std::vector<GLfloat>::const_iterator npIt=notchPositions.begin();npIt!=notchPositions.end();++npIt)
				{
				glVertex3f(*npIt-ns*0.5f,ny,nz);
				glVertex3f(*npIt+ns*0.5f,ny,nz);
				}
			glEnd();
			
			/* Draw the shaft margin and the notches: */
			glBegin(GL_QUAD_STRIP);
			glVertex(shaftBox.getCorner(4));
			glVertex(getInterior().getCorner(0));
			ny=shaftBox.origin[1]-ns*3.0f;
			for(std::vector<GLfloat>::const_iterator npIt=notchPositions.begin();npIt!=notchPositions.end();++npIt)
				{
				glVertex3f(*npIt-ns*0.5f,ny+ns*2.0f,nz);
				glVertex3f(*npIt-ns*0.5f,ny-ns*2.0f,nz);
				glNormal3f(0.7071f,0.0f,0.7071f);
				glColor(shaftColor);
				glVertex3f(*npIt-ns*0.5f,ny+ns*2.0f,nz);
				glVertex3f(*npIt-ns*0.5f,ny-ns*2.0f,nz);
				glVertex3f(*npIt,ny+ns*1.5f,nz-ns*0.5f);
				glVertex3f(*npIt,ny-ns*1.5f,nz-ns*0.5f);
				glNormal3f(-0.7071f,0.0f,0.7071f);
				glVertex3f(*npIt,ny+ns*1.5f,nz-ns*0.5f);
				glVertex3f(*npIt,ny-ns*1.5f,nz-ns*0.5f);
				glVertex3f(*npIt+ns*0.5f,ny+ns*2.0f,nz);
				glVertex3f(*npIt+ns*0.5f,ny-ns*2.0f,nz);
				glNormal3f(0.0f,0.0f,1.0f);
				glColor(backgroundColor);
				glVertex3f(*npIt+ns*0.5f,ny+ns*2.0f,nz);
				glVertex3f(*npIt+ns*0.5f,ny-ns*2.0f,nz);
				}
			glVertex(shaftBox.getCorner(5));
			glVertex(getInterior().getCorner(1));
			glVertex(shaftBox.getCorner(7));
			glVertex(getInterior().getCorner(3));
			ny=shaftBox.origin[1]+shaftBox.size[1]+ns*3.0f;
			for(std::vector<GLfloat>::const_reverse_iterator npIt=notchPositions.rbegin();npIt!=notchPositions.rend();++npIt)
				{
				glVertex3f(*npIt+ns*0.5f,ny-ns*2.0f,nz);
				glVertex3f(*npIt+ns*0.5f,ny+ns*2.0f,nz);
				glNormal3f(-0.7071f,0.0f,0.7071f);
				glColor(shaftColor);
				glVertex3f(*npIt+ns*0.5f,ny-ns*2.0f,nz);
				glVertex3f(*npIt+ns*0.5f,ny+ns*2.0f,nz);
				glVertex3f(*npIt,ny-ns*1.5f,nz-ns*0.5f);
				glVertex3f(*npIt,ny+ns*1.5f,nz-ns*0.5f);
				glNormal3f(0.7071f,0.0f,0.7071f);
				glVertex3f(*npIt,ny-ns*1.5f,nz-ns*0.5f);
				glVertex3f(*npIt,ny+ns*1.5f,nz-ns*0.5f);
				glVertex3f(*npIt-ns*0.5f,ny-ns*2.0f,nz);
				glVertex3f(*npIt-ns*0.5f,ny+ns*2.0f,nz);
				glNormal3f(0.0f,0.0f,1.0f);
				glColor(backgroundColor);
				glVertex3f(*npIt-ns*0.5f,ny-ns*2.0f,nz);
				glVertex3f(*npIt-ns*0.5f,ny+ns*2.0f,nz);
				}
			glVertex(shaftBox.getCorner(6));
			glVertex(getInterior().getCorner(2));
			glVertex(shaftBox.getCorner(4));
			glVertex(getInterior().getCorner(0));
			glEnd();
			
			/* Draw the top and bottom triangles of all notches: */
			glColor(shaftColor);
			glBegin(GL_TRIANGLES);
			for(std::vector<GLfloat>::const_iterator npIt=notchPositions.begin();npIt!=notchPositions.end();++npIt)
				{
				GLfloat ny1=shaftBox.origin[1]-ns*3.0f;
				GLfloat ny2=shaftBox.origin[1]+shaftBox.size[1]+ns*3.0f;
				glNormal3f(0.0f,0.7071f,0.7071f);
				glVertex3f(*npIt-ns*0.5f,ny1-ns*2.0f,nz);
				glVertex3f(*npIt+ns*0.5f,ny1-ns*2.0f,nz);
				glVertex3f(*npIt,ny1-ns*1.5f,nz-ns*0.5f);
				glVertex3f(*npIt-ns*0.5f,ny2-ns*2.0f,nz);
				glVertex3f(*npIt+ns*0.5f,ny2-ns*2.0f,nz);
				glVertex3f(*npIt,ny2-ns*1.5f,nz-ns*0.5f);
				glNormal3f(0.0f,-0.7071f,0.7071f);
				glVertex3f(*npIt+ns*0.5f,ny1+ns*2.0f,nz);
				glVertex3f(*npIt-ns*0.5f,ny1+ns*2.0f,nz);
				glVertex3f(*npIt,ny1+ns*1.5f,nz-ns*0.5f);
				glVertex3f(*npIt+ns*0.5f,ny2+ns*2.0f,nz);
				glVertex3f(*npIt-ns*0.5f,ny2+ns*2.0f,nz);
				glVertex3f(*npIt,ny2+ns*1.5f,nz-ns*0.5f);
				}
			glEnd();
			}
		else
			{
			GLfloat nx;
			glColor(backgroundColor);
			glNormal3f(0.0f,0.0f,1.0f);
			
			/* Draw the left shaft margin part's left half: */
			nx=shaftBox.origin[0]-ns*5.0f;
			glBegin(GL_TRIANGLE_FAN);
			glVertex(getInterior().getCorner(2));
			glVertex(getInterior().getCorner(0));
			for(std::vector<GLfloat>::const_iterator npIt=notchPositions.begin();npIt!=notchPositions.end();++npIt)
				{
				glVertex3f(nx,*npIt-ns*0.5f,nz);
				glVertex3f(nx,*npIt+ns*0.5f,nz);
				}
			glEnd();
			
			/* Draw the left shaft margin part's right half: */
			nx=shaftBox.origin[0]-ns;
			glBegin(GL_TRIANGLE_FAN);
			glVertex(shaftBox.getCorner(4));
			glVertex(shaftBox.getCorner(6));
			for(std::vector<GLfloat>::const_reverse_iterator npIt=notchPositions.rbegin();npIt!=notchPositions.rend();++npIt)
				{
				glVertex3f(nx,*npIt+ns*0.5f,nz);
				glVertex3f(nx,*npIt-ns*0.5f,nz);
				}
			glEnd();
			
			/* Draw the right shaft margin part's left half: */
			nx=shaftBox.origin[0]+shaftBox.size[0]+ns;
			glBegin(GL_TRIANGLE_FAN);
			glVertex(shaftBox.getCorner(7));
			glVertex(shaftBox.getCorner(5));
			for(std::vector<GLfloat>::const_iterator npIt=notchPositions.begin();npIt!=notchPositions.end();++npIt)
				{
				glVertex3f(nx,*npIt-ns*0.5f,nz);
				glVertex3f(nx,*npIt+ns*0.5f,nz);
				}
			glEnd();
			
			/* Draw the right shaft margin part's right half: */
			nx=shaftBox.origin[0]+shaftBox.size[0]+ns*5.0f;
			glBegin(GL_TRIANGLE_FAN);
			glVertex(getInterior().getCorner(1));
			glVertex(getInterior().getCorner(3));
			for(std::vector<GLfloat>::const_reverse_iterator npIt=notchPositions.rbegin();npIt!=notchPositions.rend();++npIt)
				{
				glVertex3f(nx,*npIt+ns*0.5f,nz);
				glVertex3f(nx,*npIt-ns*0.5f,nz);
				}
			glEnd();
			
			/* Draw the shaft margin and the notches: */
			glBegin(GL_QUAD_STRIP);
			glVertex(shaftBox.getCorner(6));
			glVertex(getInterior().getCorner(2));
			nx=shaftBox.origin[0]-ns*3.0f;
			for(std::vector<GLfloat>::const_reverse_iterator npIt=notchPositions.rbegin();npIt!=notchPositions.rend();++npIt)
				{
				glVertex3f(nx+ns*2.0f,*npIt+ns*0.5f,nz);
				glVertex3f(nx-ns*2.0f,*npIt+ns*0.5f,nz);
				glNormal3f(0.0f,-0.7071f,0.7071f);
				glColor(shaftColor);
				glVertex3f(nx+ns*2.0f,*npIt+ns*0.5f,nz);
				glVertex3f(nx-ns*2.0f,*npIt+ns*0.5f,nz);
				glVertex3f(nx+ns*1.5f,*npIt,nz-ns*0.5f);
				glVertex3f(nx-ns*1.5f,*npIt,nz-ns*0.5f);
				glNormal3f(0.0f,0.7071f,0.7071f);
				glVertex3f(nx+ns*1.5f,*npIt,nz-ns*0.5f);
				glVertex3f(nx-ns*1.5f,*npIt,nz-ns*0.5f);
				glVertex3f(nx+ns*2.0f,*npIt-ns*0.5f,nz);
				glVertex3f(nx-ns*2.0f,*npIt-ns*0.5f,nz);
				glNormal3f(0.0f,0.0f,1.0f);
				glColor(backgroundColor);
				glVertex3f(nx+ns*2.0f,*npIt-ns*0.5f,nz);
				glVertex3f(nx-ns*2.0f,*npIt-ns*0.5f,nz);
				}
			glVertex(shaftBox.getCorner(4));
			glVertex(getInterior().getCorner(0));
			glVertex(shaftBox.getCorner(5));
			glVertex(getInterior().getCorner(1));
			nx=shaftBox.origin[0]+shaftBox.size[0]+ns*3.0f;
			for(std::vector<GLfloat>::const_iterator npIt=notchPositions.begin();npIt!=notchPositions.end();++npIt)
				{
				glVertex3f(nx-ns*2.0f,*npIt-ns*0.5f,nz);
				glVertex3f(nx+ns*2.0f,*npIt-ns*0.5f,nz);
				glNormal3f(0.0f,0.7071f,0.7071f);
				glColor(shaftColor);
				glVertex3f(nx-ns*2.0f,*npIt-ns*0.5f,nz);
				glVertex3f(nx+ns*2.0f,*npIt-ns*0.5f,nz);
				glVertex3f(nx-ns*1.5f,*npIt,nz-ns*0.5f);
				glVertex3f(nx+ns*1.5f,*npIt,nz-ns*0.5f);
				glNormal3f(0.0f,-0.7071f,0.7071f);
				glVertex3f(nx-ns*1.5f,*npIt,nz-ns*0.5f);
				glVertex3f(nx+ns*1.5f,*npIt,nz-ns*0.5f);
				glVertex3f(nx-ns*2.0f,*npIt+ns*0.5f,nz);
				glVertex3f(nx+ns*2.0f,*npIt+ns*0.5f,nz);
				glNormal3f(0.0f,0.0f,1.0f);
				glColor(backgroundColor);
				glVertex3f(nx-ns*2.0f,*npIt+ns*0.5f,nz);
				glVertex3f(nx+ns*2.0f,*npIt+ns*0.5f,nz);
				}
			glVertex(shaftBox.getCorner(7));
			glVertex(getInterior().getCorner(3));
			glVertex(shaftBox.getCorner(6));
			glVertex(getInterior().getCorner(2));
			glEnd();
			
			/* Draw the left and right triangles of all notches: */
			glColor(shaftColor);
			glBegin(GL_TRIANGLES);
			for(std::vector<GLfloat>::const_iterator npIt=notchPositions.begin();npIt!=notchPositions.end();++npIt)
				{
				GLfloat nx1=shaftBox.origin[0]-ns*3.0f;
				GLfloat nx2=shaftBox.origin[0]+shaftBox.size[0]+ns*3.0f;
				glNormal3f(0.7071f,0.0f,0.7071f);
				glVertex3f(nx1-ns*2.0f,*npIt+ns*0.5f,nz);
				glVertex3f(nx1-ns*2.0f,*npIt-ns*0.5f,nz);
				glVertex3f(nx1-ns*1.5f,*npIt,nz-ns*0.5f);
				glVertex3f(nx2-ns*2.0f,*npIt+ns*0.5f,nz);
				glVertex3f(nx2-ns*2.0f,*npIt-ns*0.5f,nz);
				glVertex3f(nx2-ns*1.5f,*npIt,nz-ns*0.5f);
				glNormal3f(-0.7071f,0.0f,0.7071f);
				glVertex3f(nx1+ns*2.0f,*npIt-ns*0.5f,nz);
				glVertex3f(nx1+ns*2.0f,*npIt+ns*0.5f,nz);
				glVertex3f(nx1+ns*1.5f,*npIt,nz-ns*0.5f);
				glVertex3f(nx2+ns*2.0f,*npIt-ns*0.5f,nz);
				glVertex3f(nx2+ns*2.0f,*npIt+ns*0.5f,nz);
				glVertex3f(nx2+ns*1.5f,*npIt,nz-ns*0.5f);
				}
			glEnd();
			}
		}
	
	/* Draw the shaft: */
	glColor(shaftColor);
	glBegin(GL_QUADS);
	glNormal3f(0.0f,1.0f,0.0f);
	glVertex(shaftBox.getCorner(4));
	glVertex(shaftBox.getCorner(5));
	glVertex(shaftBox.getCorner(1));
	glVertex(shaftBox.getCorner(0));
	glNormal3f(0.0f,-1.0f,0.0f);
	glVertex(shaftBox.getCorner(2));
	glVertex(shaftBox.getCorner(3));
	glVertex(shaftBox.getCorner(7));
	glVertex(shaftBox.getCorner(6));
	glNormal3f(1.0f,0.0f,0.0f);
	glVertex(shaftBox.getCorner(0));
	glVertex(shaftBox.getCorner(2));
	glVertex(shaftBox.getCorner(6));
	glVertex(shaftBox.getCorner(4));
	glNormal3f(-1.0f,0.0f,0.0f);
	glVertex(shaftBox.getCorner(1));
	glVertex(shaftBox.getCorner(5));
	glVertex(shaftBox.getCorner(7));
	glVertex(shaftBox.getCorner(3));
	glNormal3f(0.0f,0.0f,1.0f);
	glVertex(shaftBox.getCorner(0));
	glVertex(shaftBox.getCorner(1));
	glVertex(shaftBox.getCorner(3));
	glVertex(shaftBox.getCorner(2));
	glEnd();
	
	/* Draw the slider handle: */
	glColor(sliderColor);
	switch(orientation)
		{
		case HORIZONTAL:
			{
			GLfloat x1=sliderBox.origin[0];
			glBegin(GL_QUAD_STRIP);
			glNormal3f(-1.0f,0.0f,0.0f);
			glVertex3f(x1,shaftBox.origin[1]+shaftBox.size[1],sliderBox.origin[2]);
			glVertex3f(x1,shaftBox.origin[1],sliderBox.origin[2]);
			glVertex3f(x1,shaftBox.origin[1]+shaftBox.size[1],sliderBox.origin[2]+shaftDepth);
			glVertex3f(x1,shaftBox.origin[1],sliderBox.origin[2]+shaftDepth);
			glVertex3f(x1,sliderBox.origin[1]+sliderBox.size[1],sliderBox.origin[2]+shaftDepth);
			glVertex3f(x1,sliderBox.origin[1],sliderBox.origin[2]+shaftDepth);
			glVertex3f(x1,sliderBox.origin[1]+sliderBox.size[1]*0.75,sliderBox.origin[2]+sliderBox.size[2]);
			glVertex3f(x1,sliderBox.origin[1]+sliderBox.size[1]*0.25,sliderBox.origin[2]+sliderBox.size[2]);
			glEnd();
			GLfloat x2=sliderBox.origin[0]+sliderBox.size[0];
			glBegin(GL_QUAD_STRIP);
			glNormal3f(1.0f,0.0f,0.0f);
			glVertex3f(x2,shaftBox.origin[1],sliderBox.origin[2]);
			glVertex3f(x2,shaftBox.origin[1]+shaftBox.size[1],sliderBox.origin[2]);
			glVertex3f(x2,shaftBox.origin[1],sliderBox.origin[2]+shaftDepth);
			glVertex3f(x2,shaftBox.origin[1]+shaftBox.size[1],sliderBox.origin[2]+shaftDepth);
			glVertex3f(x2,sliderBox.origin[1],sliderBox.origin[2]+shaftDepth);
			glVertex3f(x2,sliderBox.origin[1]+sliderBox.size[1],sliderBox.origin[2]+shaftDepth);
			glVertex3f(x2,sliderBox.origin[1]+sliderBox.size[1]*0.25,sliderBox.origin[2]+sliderBox.size[2]);
			glVertex3f(x2,sliderBox.origin[1]+sliderBox.size[1]*0.75,sliderBox.origin[2]+sliderBox.size[2]);
			glEnd();
			glBegin(GL_QUADS);
			glNormal3f(0.0f,0.0f,-1.0f);
			glVertex3f(x1,shaftBox.origin[1],sliderBox.origin[2]);
			glVertex3f(x1,shaftBox.origin[1]+shaftBox.size[1],sliderBox.origin[2]);
			glVertex3f(x2,shaftBox.origin[1]+shaftBox.size[1],sliderBox.origin[2]);
			glVertex3f(x2,shaftBox.origin[1],sliderBox.origin[2]);
			glNormal3f(0.0f,1.0f,0.0f);
			glVertex3f(x1,shaftBox.origin[1]+shaftBox.size[1],sliderBox.origin[2]);
			glVertex3f(x1,shaftBox.origin[1]+shaftBox.size[1],sliderBox.origin[2]+shaftDepth);
			glVertex3f(x2,shaftBox.origin[1]+shaftBox.size[1],sliderBox.origin[2]+shaftDepth);
			glVertex3f(x2,shaftBox.origin[1]+shaftBox.size[1],sliderBox.origin[2]);
			glNormal3f(0.0f,0.0f,-1.0f);
			glVertex3f(x1,shaftBox.origin[1]+shaftBox.size[1],sliderBox.origin[2]+shaftDepth);
			glVertex3f(x1,sliderBox.origin[1]+sliderBox.size[1],sliderBox.origin[2]+shaftDepth);
			glVertex3f(x2,sliderBox.origin[1]+sliderBox.size[1],sliderBox.origin[2]+shaftDepth);
			glVertex3f(x2,shaftBox.origin[1]+shaftBox.size[1],sliderBox.origin[2]+shaftDepth);
			glNormal3f(0.0f,1.0f,0.25f);
			glVertex3f(x1,sliderBox.origin[1]+sliderBox.size[1],sliderBox.origin[2]+shaftDepth);
			glVertex3f(x1,sliderBox.origin[1]+sliderBox.size[1]*0.75,sliderBox.origin[2]+sliderBox.size[2]);
			glVertex3f(x2,sliderBox.origin[1]+sliderBox.size[1]*0.75,sliderBox.origin[2]+sliderBox.size[2]);
			glVertex3f(x2,sliderBox.origin[1]+sliderBox.size[1],sliderBox.origin[2]+shaftDepth);
			glNormal3f(0.0f,0.0f,1.0f);
			glVertex3f(x1,sliderBox.origin[1]+sliderBox.size[1]*0.75,sliderBox.origin[2]+sliderBox.size[2]);
			glVertex3f(x1,sliderBox.origin[1]+sliderBox.size[1]*0.25,sliderBox.origin[2]+sliderBox.size[2]);
			glVertex3f(x2,sliderBox.origin[1]+sliderBox.size[1]*0.25,sliderBox.origin[2]+sliderBox.size[2]);
			glVertex3f(x2,sliderBox.origin[1]+sliderBox.size[1]*0.75,sliderBox.origin[2]+sliderBox.size[2]);
			glNormal3f(0.0f,-1.0f,0.25f);
			glVertex3f(x1,sliderBox.origin[1]+sliderBox.size[1]*0.25,sliderBox.origin[2]+sliderBox.size[2]);
			glVertex3f(x1,sliderBox.origin[1],sliderBox.origin[2]+shaftDepth);
			glVertex3f(x2,sliderBox.origin[1],sliderBox.origin[2]+shaftDepth);
			glVertex3f(x2,sliderBox.origin[1]+sliderBox.size[1]*0.25,sliderBox.origin[2]+sliderBox.size[2]);
			glNormal3f(0.0f,0.0f,-1.0f);
			glVertex3f(x1,sliderBox.origin[1],sliderBox.origin[2]+shaftDepth);
			glVertex3f(x1,shaftBox.origin[1],sliderBox.origin[2]+shaftDepth);
			glVertex3f(x2,shaftBox.origin[1],sliderBox.origin[2]+shaftDepth);
			glVertex3f(x2,sliderBox.origin[1],sliderBox.origin[2]+shaftDepth);
			glNormal3f(0.0f,-1.0f,0.0f);
			glVertex3f(x1,shaftBox.origin[1],sliderBox.origin[2]+shaftDepth);
			glVertex3f(x1,shaftBox.origin[1],sliderBox.origin[2]);
			glVertex3f(x2,shaftBox.origin[1],sliderBox.origin[2]);
			glVertex3f(x2,shaftBox.origin[1],sliderBox.origin[2]+shaftDepth);
			glEnd();
			break;
			}
		
		case VERTICAL:
			{
			GLfloat y1=sliderBox.origin[1];
			glBegin(GL_QUAD_STRIP);
			glNormal3f(0.0f,-1.0f,0.0f);
			glVertex3f(shaftBox.origin[0],y1,sliderBox.origin[2]);
			glVertex3f(shaftBox.origin[0]+shaftBox.size[0],y1,sliderBox.origin[2]);
			glVertex3f(shaftBox.origin[0],y1,sliderBox.origin[2]+shaftDepth);
			glVertex3f(shaftBox.origin[0]+shaftBox.size[0],y1,sliderBox.origin[2]+shaftDepth);
			glVertex3f(sliderBox.origin[0],y1,sliderBox.origin[2]+shaftDepth);
			glVertex3f(sliderBox.origin[0]+sliderBox.size[0],y1,sliderBox.origin[2]+shaftDepth);
			glVertex3f(sliderBox.origin[0]+sliderBox.size[0]*0.25,y1,sliderBox.origin[2]+sliderBox.size[2]);
			glVertex3f(sliderBox.origin[0]+sliderBox.size[0]*0.75,y1,sliderBox.origin[2]+sliderBox.size[2]);
			glEnd();
			GLfloat y2=sliderBox.origin[1]+sliderBox.size[1];
			glBegin(GL_QUAD_STRIP);
			glNormal3f(0.0f,1.0f,0.0f);
			glVertex3f(shaftBox.origin[0]+shaftBox.size[0],y2,sliderBox.origin[2]);
			glVertex3f(shaftBox.origin[0],y2,sliderBox.origin[2]);
			glVertex3f(shaftBox.origin[0]+shaftBox.size[0],y2,sliderBox.origin[2]+shaftDepth);
			glVertex3f(shaftBox.origin[0],y2,sliderBox.origin[2]+shaftDepth);
			glVertex3f(sliderBox.origin[0]+sliderBox.size[0],y2,sliderBox.origin[2]+shaftDepth);
			glVertex3f(sliderBox.origin[0],y2,sliderBox.origin[2]+shaftDepth);
			glVertex3f(sliderBox.origin[0]+sliderBox.size[0]*0.75,y2,sliderBox.origin[2]+sliderBox.size[2]);
			glVertex3f(sliderBox.origin[0]+sliderBox.size[0]*0.25,y2,sliderBox.origin[2]+sliderBox.size[2]);
			glEnd();
			glBegin(GL_QUADS);
			glNormal3f(0.0f,0.0f,-1.0f);
			glVertex3f(shaftBox.origin[0],y1,sliderBox.origin[2]);
			glVertex3f(shaftBox.origin[0],y2,sliderBox.origin[2]);
			glVertex3f(shaftBox.origin[0]+shaftBox.size[0],y2,sliderBox.origin[2]);
			glVertex3f(shaftBox.origin[0]+shaftBox.size[0],y1,sliderBox.origin[2]);
			glNormal3f(1.0f,0.0f,0.0f);
			glVertex3f(shaftBox.origin[0]+shaftBox.size[0],y1,sliderBox.origin[2]);
			glVertex3f(shaftBox.origin[0]+shaftBox.size[0],y2,sliderBox.origin[2]);
			glVertex3f(shaftBox.origin[0]+shaftBox.size[0],y2,sliderBox.origin[2]+shaftDepth);
			glVertex3f(shaftBox.origin[0]+shaftBox.size[0],y1,sliderBox.origin[2]+shaftDepth);
			glNormal3f(0.0f,0.0f,-1.0f);
			glVertex3f(shaftBox.origin[0]+shaftBox.size[0],y1,sliderBox.origin[2]+shaftDepth);
			glVertex3f(shaftBox.origin[0]+shaftBox.size[0],y2,sliderBox.origin[2]+shaftDepth);
			glVertex3f(sliderBox.origin[0]+sliderBox.size[0],y2,sliderBox.origin[2]+shaftDepth);
			glVertex3f(sliderBox.origin[0]+sliderBox.size[0],y1,sliderBox.origin[2]+shaftDepth);
			glNormal3f(1.0f,0.0f,0.25f);
			glVertex3f(sliderBox.origin[0]+sliderBox.size[0],y1,sliderBox.origin[2]+shaftDepth);
			glVertex3f(sliderBox.origin[0]+sliderBox.size[0],y2,sliderBox.origin[2]+shaftDepth);
			glVertex3f(sliderBox.origin[0]+sliderBox.size[0]*0.75,y2,sliderBox.origin[2]+sliderBox.size[2]);
			glVertex3f(sliderBox.origin[0]+sliderBox.size[0]*0.75,y1,sliderBox.origin[2]+sliderBox.size[2]);
			glNormal3f(0.0f,0.0f,1.0f);
			glVertex3f(sliderBox.origin[0]+sliderBox.size[0]*0.75,y1,sliderBox.origin[2]+sliderBox.size[2]);
			glVertex3f(sliderBox.origin[0]+sliderBox.size[0]*0.75,y2,sliderBox.origin[2]+sliderBox.size[2]);
			glVertex3f(sliderBox.origin[0]+sliderBox.size[0]*0.25,y2,sliderBox.origin[2]+sliderBox.size[2]);
			glVertex3f(sliderBox.origin[0]+sliderBox.size[0]*0.25,y1,sliderBox.origin[2]+sliderBox.size[2]);
			glNormal3f(-1.0f,0.0f,0.25f);
			glVertex3f(sliderBox.origin[0]+sliderBox.size[0]*0.25,y1,sliderBox.origin[2]+sliderBox.size[2]);
			glVertex3f(sliderBox.origin[0]+sliderBox.size[0]*0.25,y2,sliderBox.origin[2]+sliderBox.size[2]);
			glVertex3f(sliderBox.origin[0],y2,sliderBox.origin[2]+shaftDepth);
			glVertex3f(sliderBox.origin[0],y1,sliderBox.origin[2]+shaftDepth);
			glNormal3f(0.0f,0.0f,-1.0f);
			glVertex3f(sliderBox.origin[0],y1,sliderBox.origin[2]+shaftDepth);
			glVertex3f(sliderBox.origin[0],y2,sliderBox.origin[2]+shaftDepth);
			glVertex3f(sliderBox.origin[0],y2,sliderBox.origin[2]);
			glVertex3f(sliderBox.origin[0],y1,sliderBox.origin[2]);
			glEnd();
			break;
			}
		}
	}

bool Slider::findRecipient(Event& event)
	{
	if(isDragging())
		return overrideRecipient(this,event);
	else
		return Widget::findRecipient(event);
	}

void Slider::pointerButtonDown(Event& event)
	{
	/* Where inside the widget did the event hit? */
	int dimension=orientation==HORIZONTAL?0:1;
	GLfloat picked=event.getWidgetPoint().getPoint()[dimension];
	if(picked>=sliderBox.origin[dimension]&&picked<=sliderBox.origin[dimension]+sliderBox.size[dimension])
		{
		/* Picked slider handle, start dragging: */
		dragOffset=sliderBox.origin[dimension]-picked;
		lastDragPos=sliderBox.origin[dimension];
		startDragging(event);
		}
	else
		{
		/* Check if a notch was hit: */
		unsigned int i;
		for(i=0;i<notchPositions.size();++i)
			if(picked>=notchPositions[i]-shaftWidth*0.75f&&picked<=notchPositions[i]+shaftWidth*0.75f)
				break;
		if(i<notchPositions.size())
			{
			/* Update the slider's state: */
			value=notchValues[i];
			positionSlider();
			
			/* Call the value changed callbacks: */
			ValueChangedCallbackData cbData(this,ValueChangedCallbackData::CLICKED,value);
			valueChangedCallbacks.call(&cbData);
			
			/* Invalidate the visual representation: */
			update();
			}
		else
			{
			/* Decrement or increment the slider value to the next tick or the minimum/maximum: */
			if(picked<sliderBox.origin[dimension])
				{
				decrement();
				isClicking=-1;
				}
			else
				{
				increment();
				isClicking=1;
				}
			
			/* Schedule a timer event for click repeat: */
			Misc::TimerEventScheduler* tes=getManager()->getTimerEventScheduler();
			if(tes!=0)
				{
				nextClickEventTime=tes->getCurrentTime()+0.5;
				tes->scheduleEvent(nextClickEventTime,this,&Slider::clickRepeatTimerEventCallback);
				}
			}
		}
	}

void Slider::pointerButtonUp(Event& event)
	{
	stopDragging(event);
	
	/* Cancel any pending click-repeat events: */
	Misc::TimerEventScheduler* tes=getManager()->getTimerEventScheduler();
	if(tes!=0)
		tes->removeEvent(nextClickEventTime,this,&Slider::clickRepeatTimerEventCallback);
	isClicking=0;
	}

void Slider::pointerMotion(Event& event)
	{
	if(isDragging())
		{
		/* Update the slider value and position: */
		int dimension=orientation==HORIZONTAL?0:1;
		GLfloat newSliderPosition=event.getWidgetPoint().getPoint()[dimension]+dragOffset;
		
		/* Check if the slider handle is currently hanging at a notch: */
		GLfloat newValue=0.0f;
		bool hanging=false;
		if(newSliderPosition<lastDragPos)
			{
			unsigned int i;
			for(i=0;i<notchPositions.size()&&notchPositions[i]-sliderLength*0.5f<lastDragPos;++i)
				;
			if(i<notchPositions.size()&&newSliderPosition>=notchPositions[i]-sliderLength*0.5f-shaftWidth*1.5f)
				{
				lastDragPos=notchPositions[i]-sliderLength*0.5f;
				newValue=notchValues[i];
				hanging=true;
				}
			}
		else if(newSliderPosition>lastDragPos)
			{
			unsigned int i;
			for(i=notchPositions.size();i>0&&notchPositions[i-1]-sliderLength*0.5f>lastDragPos;--i)
				;
			if(i>0&&newSliderPosition<=notchPositions[i-1]-sliderLength*0.5f+shaftWidth*1.5f)
				{
				lastDragPos=notchPositions[i-1]-sliderLength*0.5f;
				newValue=notchValues[i-1];
				hanging=true;
				}
			}
		
		if(!hanging)
			{
			/* Calculate the new slider value and reposition the slider: */
			newValue=(newSliderPosition-shaftBox.origin[dimension])*(valueMax-valueMin)/(shaftBox.size[dimension]-sliderLength)+valueMin;
			if(newValue<valueMin)
				newValue=valueMin;
			else if(newValue>valueMax)
				newValue=valueMax;
			if(valueIncrement>0.0f)
				newValue=GLfloat(floor(double(newValue)/double(valueIncrement)+0.5)*double(valueIncrement));
			lastDragPos=newSliderPosition;
			}
		
		if(newValue!=value)
			{
			/* Update the slider: */
			value=newValue;
			positionSlider();
			
			/* Call the value changed callbacks: */
			ValueChangedCallbackData cbData(this,ValueChangedCallbackData::DRAGGED,value);
			valueChangedCallbacks.call(&cbData);
			
			/* Update the visual representation: */
			update();
			}
		}
	}

void Slider::setMarginWidth(GLfloat newMarginWidth)
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

void Slider::addNotch(GLfloat newNotchValue)
	{
	/* Find the appropriate place for the new notch value to keep the vector sorted: */
	std::vector<GLfloat>::iterator nvIt;
	for(nvIt=notchValues.begin();nvIt!=notchValues.end()&&*nvIt<newNotchValue;++nvIt)
		;
	
	/* Insert the new notch value if it is not there already: */
	if(nvIt==notchValues.end()||*nvIt>newNotchValue)
		notchValues.insert(nvIt,newNotchValue);
	
	/* Update the notch positions: */
	positionNotches();
	}

void Slider::removeNotch(GLfloat notchValue)
	{
	/* Find the notch value in the vector: */
	std::vector<GLfloat>::iterator nvIt;
	for(nvIt=notchValues.begin();nvIt!=notchValues.end()&&*nvIt<notchValue;++nvIt)
		;
	
	/* Remove the notch value if it is there: */
	if(nvIt!=notchValues.end()&&*nvIt==notchValue)
		notchValues.erase(nvIt);
	
	/* Update the notch positions: */
	positionNotches();
	}

void Slider::setValue(GLfloat newValue)
	{
	/* Update the value and reposition the slider: */
	value=newValue;
	positionSlider();
	
	/* Update the visual representation: */
	update();
	}

void Slider::setValueRange(GLfloat newValueMin,GLfloat newValueMax,GLfloat newValueIncrement)
	{
	/* Update the value range: */
	valueMin=newValueMin;
	valueMax=newValueMax;
	valueIncrement=newValueIncrement;
	
	/* Adjust the current value and reposition the slider: */
	if(value<valueMin)
		value=valueMin;
	else if(value>valueMax)
		value=valueMax;
	if(valueIncrement>0.0f)
		value=GLfloat(floor(double(value)/double(valueIncrement)+0.5)*double(valueIncrement));
	positionSlider();
	
	/* Update the visual representation: */
	update();
	}

}
