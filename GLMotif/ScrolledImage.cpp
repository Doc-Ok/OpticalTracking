/***********************************************************************
ScrolledImage - Compound widget containing an image, and a vertical and
horizontal scroll bar.
Copyright (c) 2011-2014 Oliver Kreylos

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

#include <GLMotif/ScrolledImage.h>

#include <iostream>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/Event.h>

namespace GLMotif {

/******************************
Methods of class ScrolledImage:
******************************/

void ScrolledImage::init(bool sManageChild)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Set the compound widget's borders: */
	setBorderWidth(ss->textfieldBorderWidth);
	setBorderType(Widget::LOWERED);
	
	/* Disable the image's borders: */
	//image->setBorderWidth(image->getBorderWidth());
	//image->setBorderType(Widget::PLAIN);
	
	/* Initialize the horizontal scroll bar: */
	horizontalScrollBar->setPositionRange(0,image->getImage().getSize(0),image->getImage().getSize(0));
	horizontalScrollBar->getValueChangedCallbacks().add(this,&ScrolledImage::scrollBarCallback);
	
	/* Initialize the vertical scroll bar: */
	verticalScrollBar->setPositionRange(0,image->getImage().getSize(1),image->getImage().getSize(1));
	verticalScrollBar->getValueChangedCallbacks().add(this,&ScrolledImage::scrollBarCallback);
	
	/* Manage the children: */
	image->manageChild();
	horizontalScrollBar->manageChild();
	verticalScrollBar->manageChild();
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

void ScrolledImage::scrollBarCallback(ScrollBar::ValueChangedCallbackData* cbData)
	{
	GLfloat region[4];
	for(int i=0;i<4;++i)
		region[i]=image->getRegion()[i];
	
	if(cbData->scrollBar==horizontalScrollBar)
		{
		GLfloat size=region[2]-region[0];
		region[0]=GLfloat(cbData->position);
		region[2]=region[0]+size;
		}
	if(cbData->scrollBar==verticalScrollBar)
		{
		GLfloat size=region[3]-region[1];
		region[1]=GLfloat(cbData->position);
		region[3]=region[1]+size;
		}
	
	image->setRegion(region);
	}

ScrolledImage::ScrolledImage(const char* sName,Container* sParent,const Images::RGBImage& sImage,const GLfloat sResolution[2],bool sManageChild)
	:Container(sName,sParent,false),
	 preferredSize(0.0f,0.0f,0.0f),
	 image(new Image("Image",this,sImage,sResolution,false)),
	 horizontalScrollBar(new ScrollBar("HorizontalScrollBar",this,ScrollBar::HORIZONTAL,false,false)),
	 verticalScrollBar(new ScrollBar("VerticalScrollBar",this,ScrollBar::VERTICAL,false,false)),
	 zoomFactor(1.0f)
	{
	init(sManageChild);
	}

ScrolledImage::ScrolledImage(const char* sName,Container* sParent,const char* sImageFileName,const GLfloat sResolution[2],bool sManageChild)
	:Container(sName,sParent,false),
	 preferredSize(0.0f,0.0f,0.0f),
	 image(new Image("Image",this,sImageFileName,sResolution,false)),
	 horizontalScrollBar(new ScrollBar("HorizontalScrollBar",this,ScrollBar::HORIZONTAL,false,false)),
	 verticalScrollBar(new ScrollBar("VerticalScrollBar",this,ScrollBar::VERTICAL,false,false)),
	 zoomFactor(1.0f)
	{
	init(sManageChild);
	}

ScrolledImage::ScrolledImage(const char* sName,Container* sParent,Image* sImage,bool sManageChild)
	:Container(sName,sParent,false),
	 preferredSize(0.0f,0.0f,0.0f),
	 image(sImage),
	 horizontalScrollBar(new ScrollBar("HorizontalScrollBar",this,ScrollBar::HORIZONTAL,false,false)),
	 verticalScrollBar(new ScrollBar("VerticalScrollBar",this,ScrollBar::VERTICAL,false,false)),
	 zoomFactor(1.0f)
	{
	/* Reparent the image widget: */
	image->reparent(this,false);
	
	init(sManageChild);
	}

ScrolledImage::~ScrolledImage(void)
	{
	/* Delete the child widgets: */
	deleteChild(image);
	deleteChild(horizontalScrollBar);
	deleteChild(verticalScrollBar);
	}

Vector ScrolledImage::calcNaturalSize(void) const
	{
	/* Start with the widget's preferred interior size: */
	Vector result=preferredSize;
	
	/* Adjust for the horizontal and vertical scroll bars: */
	Vector hBarSize=horizontalScrollBar->calcNaturalSize();
	Vector vBarSize=verticalScrollBar->calcNaturalSize();
	if(result[0]<hBarSize[0])
		result[0]=hBarSize[0];
	if(result[1]<vBarSize[1])
		result[1]=vBarSize[1];
	result[0]+=vBarSize[0];
	result[1]+=hBarSize[1];
	
	return calcExteriorSize(result);
	}

ZRange ScrolledImage::calcZRange(void) const
	{
	/* Calculate the parent class widget's z range: */
	ZRange myZRange=Container::calcZRange();
	
	/* Adjust for the image: */
	myZRange+=image->calcZRange();
	
	/* Adjust for the horizontal and vertical scroll bars: */
	myZRange+=horizontalScrollBar->calcZRange();
	myZRange+=verticalScrollBar->calcZRange();
	
	return myZRange;
	}

void ScrolledImage::resize(const Box& newExterior)
	{
	/* Resize the parent class widget: */
	Container::resize(newExterior);
	
	/* Get the new interior size: */
	Box iBox=getInterior();
	
	/* Make room for and position the horizontal and vertical scroll bars: */
	Vector hbSize=horizontalScrollBar->calcNaturalSize();
	Vector vbSize=verticalScrollBar->calcNaturalSize();
	Box hbBox=iBox;
	hbBox.size[0]-=vbSize[0];
	hbBox.size[1]=hbSize[1];
	horizontalScrollBar->resize(hbBox);
	Box vbBox=iBox;
	vbBox.size[0]=vbSize[0];
	vbBox.origin[0]+=iBox.size[0]-vbBox.size[0];
	vbBox.size[1]-=hbSize[1];
	vbBox.origin[1]+=hbSize[1];
	verticalScrollBar->resize(vbBox);
	
	/* Resize the image: */
	iBox.size[0]-=vbSize[0];
	iBox.size[1]-=hbSize[1];
	iBox.origin[1]+=hbSize[1];
	image->resize(iBox);
	
	/* Adjust the image's display region: */
	setZoomFactor(zoomFactor);
	}

void ScrolledImage::draw(GLContextData& contextData) const
	{
	/* Draw the parent class widget: */
	Container::draw(contextData);
	
	/* Draw the margin around the child widgets: */
	const Box& iBox=image->getExterior();
	const Box& hbBox=horizontalScrollBar->getExterior();
	const Box& vbBox=verticalScrollBar->getExterior();
	
	glColor(backgroundColor);
	
	/* Draw the top left margin part: */
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0f,0.0f,1.0f);
	glVertex(getInterior().getCorner(2));
	glVertex(getInterior().getCorner(0));
	glVertex(hbBox.getCorner(0));
	glVertex(hbBox.getCorner(2));
	glVertex(iBox.getCorner(0));
	glVertex(iBox.getCorner(2));
	glVertex(iBox.getCorner(3));
	glVertex(vbBox.getCorner(3));
	glVertex(getInterior().getCorner(3));
	glEnd();
	
	/* Draw the bottom right margin part: */
	glBegin(GL_TRIANGLE_FAN);
	glVertex(getInterior().getCorner(1));
	glVertex(getInterior().getCorner(3));
	glVertex(vbBox.getCorner(3));
	glVertex(vbBox.getCorner(1));
	glVertex(vbBox.getCorner(0));
	glVertex(hbBox.getCorner(3));
	glVertex(hbBox.getCorner(1));
	glVertex(hbBox.getCorner(0));
	glVertex(getInterior().getCorner(0));
	glEnd();
	
	/* Draw the image and scroll bars: */
	image->draw(contextData);
	horizontalScrollBar->draw(contextData);
	verticalScrollBar->draw(contextData);
	}

bool ScrolledImage::findRecipient(Event& event)
	{
	/* Distribute the question to the child widgets: */
	bool childFound=false;
	if(!childFound)
		childFound=image->findRecipient(event);
	if(!childFound)
		childFound=horizontalScrollBar->findRecipient(event);
	if(!childFound)
		childFound=verticalScrollBar->findRecipient(event);
	
	/* If no child was found, return ourselves (and ignore any incoming events): */
	if(childFound)
		return true;
	else
		{
		/* Check ourselves: */
		Event::WidgetPoint wp=event.calcWidgetPoint(this);
		if(isInside(wp.getPoint()))
			return event.setTargetWidget(this,wp);
		else
			return false;
		}
	}

void ScrolledImage::addChild(Widget* newChild)
	{
	/* Only the dedicated children can call this, and they get managed in the constructor, so ignore this */
	}

void ScrolledImage::removeChild(Widget* removeChild)
	{
	/* This should never be called, so simply ignore it */
	}

void ScrolledImage::requestResize(Widget* child,const Vector& newExteriorSize)
	{
	if(isManaged)
		{
		/* Gather the old and new sizes of the child widgets: */
		Vector iSize=child==image?newExteriorSize:image->getExterior().size;
		Vector hbSize=child==horizontalScrollBar?newExteriorSize:horizontalScrollBar->getExterior().size;
		Vector vbSize=child==verticalScrollBar?newExteriorSize:verticalScrollBar->getExterior().size;
		
		/* Assemble the new interior size: */
		if(iSize[0]<hbSize[0])
			iSize[0]=hbSize[0];
		if(iSize[1]<vbSize[1])
			iSize[1]=vbSize[1];
		iSize[0]+=vbSize[0];
		iSize[1]+=hbSize[1];
		
		/* Resize the widget: */
		parent->requestResize(this,calcExteriorSize(iSize));
		}
	else
		{
		/* Just resize the child: */
		child->resize(Box(child->getExterior().origin,newExteriorSize));
		}
	}

Widget* ScrolledImage::getFirstChild(void)
	{
	/* Return the image: */
	return image;
	}

Widget* ScrolledImage::getNextChild(Widget* child)
	{
	/* Return the next child: */
	if(child==image)
		return horizontalScrollBar;
	else if(child==horizontalScrollBar)
		return verticalScrollBar;
	else
		return 0;
	}

void ScrolledImage::setPreferredSize(const Vector& newPreferredSize)
	{
	/* Set the preferred size: */
	preferredSize=newPreferredSize;
	}

void ScrolledImage::setZoomFactor(GLfloat newZoomFactor)
	{
	/* Zoom the image around its center point: */
	Vector imageCenter;
	imageCenter=image->getInterior().origin;
	for(int i=0;i<3;++i)
		imageCenter[i]=image->getInterior().origin[i]+image->getInterior().size[i]*0.5f;
	
	setZoomFactor(newZoomFactor,imageCenter);
	}

void ScrolledImage::setZoomFactor(GLfloat newZoomFactor,const Vector& fixedPoint)
	{
	/* Set the zoom factor: */
	zoomFactor=newZoomFactor;
	
	/* Calculate the fixed point's position relative to the current image region: */
	GLfloat fpw[2]; // Relative fixed point position
	GLfloat fpi[2]; // Fixed point position in image space
	for(int i=0;i<2;++i)
		{
		fpw[i]=(fixedPoint[i]-image->getInterior().origin[i])/image->getInterior().size[i];
		fpi[i]=image->getRegionMin(i)+fpw[i]*(image->getRegionMax(i)-image->getRegionMin(i));
		}
	
	/* Set the displayed image region based on the image's current resolution and widget size: */
	GLfloat newSize[2];
	GLfloat newRegion[4];
	int newPageSize[2];
	int newPageOrigin[2];
	for(int i=0;i<2;++i)
		{
		newSize[i]=image->getInterior().size[i]*image->getResolution(i)/zoomFactor;
		GLfloat imageSize=GLfloat(image->getImage().getSize(i));
		if(newSize[i]>=imageSize)
			{
			newRegion[0+i]=-(newSize[i]-imageSize)*0.5;
			newRegion[2+i]=imageSize+(newSize[i]-imageSize)*0.5;
			}
		else
			{
			newRegion[0+i]=fpi[i]-newSize[i]*fpw[i];
			newRegion[2+i]=fpi[i]+newSize[i]*(1.0f-fpw[i]);
			if(newRegion[0+i]<0.0f)
				{
				newRegion[2+i]+=0.0f-newRegion[0+i];
				newRegion[0+i]=0.0f;
				}
			else if(newRegion[2+i]>imageSize)
				{
				newRegion[0+i]+=imageSize-newRegion[2+i];
				newRegion[2+i]=imageSize;
				}
			}
		newPageSize[i]=int(Math::floor(newSize[i]+0.5f));
		if(newPageSize[i]>int(image->getImage().getSize(i)))
			newPageSize[i]=int(image->getImage().getSize(i));
		newPageOrigin[i]=int(Math::floor(newRegion[i]+0.5));
		if(newPageOrigin[i]<0)
			newPageOrigin[i]=0;
		if(newPageOrigin[i]>int(image->getImage().getSize(i))-newPageSize[i])
			newPageOrigin[i]=int(image->getImage().getSize(i))-newPageSize[i];
		}
	image->setRegion(newRegion);
	
	/* Adjust the scroll bars: */
	horizontalScrollBar->setPositionRange(0,int(image->getImage().getSize(0)),newPageSize[0]);
	horizontalScrollBar->setPosition(newPageOrigin[0]);
	verticalScrollBar->setPositionRange(0,int(image->getImage().getSize(1)),newPageSize[1]);
	verticalScrollBar->setPosition(newPageOrigin[1]);
	}

}
