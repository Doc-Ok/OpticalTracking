/***********************************************************************
ScaleBar - Class to draw a scale bar in Vrui applications. Scale bar is
implemented as a special top-level GLMotif widget for simplicity.
Copyright (c) 2010-2012 Oliver Kreylos

This file is part of the Virtual Reality User Interface Library (Vrui).

The Virtual Reality User Interface Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Virtual Reality User Interface Library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality User Interface Library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#include <Vrui/Internal/ScaleBar.h>

#include <stdio.h>
#include <Math/Math.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Geometry/LinearUnit.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLFont.h>
#include <GL/GLLabel.h>
#include <GLMotif/Event.h>
#include <GLMotif/WidgetManager.h>
#include <Vrui/Vrui.h>
#include <Vrui/CoordinateManager.h>

namespace Vrui {

namespace {

/****************
Helper functions:
****************/

double getSmallerQuasiBinary(double value)
	{
	static const int nextMantissa[6]={0,2,5,0,0,1};
	static const int nextExponent[6]={0,0,0,0,0,1};
	if(value>1.0)
		{
		int mantissa=1;
		int exponent=0;
		while((double(nextMantissa[mantissa])+1.0e-10)*Math::pow(10.0,double(exponent+nextExponent[mantissa]))<value)
			{
			exponent=exponent+nextExponent[mantissa];
			mantissa=nextMantissa[mantissa];
			}
		return double(mantissa)*Math::pow(10.0,double(exponent));
		}
	else
		{
		value=1.0/value;
		int mantissa=1;
		int exponent=0;
		while((double(mantissa)-1.0e-10)*Math::pow(10.0,double(exponent))<=value)
			{
			exponent=exponent+nextExponent[mantissa];
			mantissa=nextMantissa[mantissa];
			}
		return 1.0/(double(mantissa)*Math::pow(10.0,double(exponent)));
		}
	}

double getBiggerQuasiBinary(double value)
	{
	static const int nextMantissa[6]={0,2,5,0,0,1};
	static const int nextExponent[6]={0,0,0,0,0,1};
	if(value>=1.0)
		{
		int mantissa=1;
		int exponent=0;
		while((double(mantissa)-1.0e-10)*Math::pow(10.0,double(exponent))<=value)
			{
			exponent=exponent+nextExponent[mantissa];
			mantissa=nextMantissa[mantissa];
			}
		return double(mantissa)*Math::pow(10.0,double(exponent));
		}
	else
		{
		value=1.0/value;
		int mantissa=1;
		int exponent=0;
		while((double(nextMantissa[mantissa])+1.0e-10)*Math::pow(10.0,double(exponent+nextExponent[mantissa]))<value)
			{
			exponent=exponent+nextExponent[mantissa];
			mantissa=nextMantissa[mantissa];
			}
		return 1.0/(double(mantissa)*Math::pow(10.0,double(exponent)));
		}
	}

}

/*************************
Methods of class ScaleBar:
*************************/

void ScaleBar::calcSize(const NavTransform& newNavigationTransformation)
	{
	const Geometry::LinearUnit& unit=getCoordinateManager()->getUnit();
	
	/* Get the application's unit conversion factor: */
	Scalar appUnitFactor=unit.factor;
	
	/* Get the navigation transformation's scale factor: */
	Scalar navScale=newNavigationTransformation.getScaling();
	
	/* Calculate the current physical-space scale bar length: */
	currentPhysLength=currentNavLength*navScale/appUnitFactor;
	
	/* Adapt the scale bar length to the display space: */
	bool navLengthChanged=false;
	while(currentPhysLength>targetLength*Math::sqrt(Scalar(2.5)))
		{
		/* Adjust scale bar length: */
		switch(currentMantissa)
			{
			case 1:
				currentMantissa=5;
				--currentExponent;
				break;
			
			case 2:
				currentMantissa=1;
				break;
			
			case 5:
				currentMantissa=2;
				break;
			}
		currentNavLength=Scalar(currentMantissa)*Math::pow(Scalar(10),Scalar(currentExponent));
		navLengthChanged=true;
		currentPhysLength=currentNavLength*navScale/appUnitFactor;
		}
	while(currentPhysLength<targetLength/Math::sqrt(Scalar(2.5)))
		{
		/* Adjust scale bar length: */
		switch(currentMantissa)
			{
			case 1:
				currentMantissa=2;
				break;
			
			case 2:
				currentMantissa=5;
				break;
			
			case 5:
				currentMantissa=1;
				++currentExponent;
				break;
			}
		currentNavLength=Scalar(currentMantissa)*Math::pow(Scalar(10),Scalar(currentExponent));
		navLengthChanged=true;
		currentPhysLength=currentNavLength*navScale/appUnitFactor;
		}
	
	if(navLengthChanged)
		{
		/* Update the length label: */
		char labelText[40];
		char* ltPtr=labelText;
		char* ltEnd=ltPtr+sizeof(labelText);
		if(currentExponent<-3||currentExponent>3)
			ltPtr+=snprintf(ltPtr,ltEnd-ltPtr,"%d.0e%+d",currentMantissa,currentExponent);
		else
			{
			if(currentExponent<0)
				{
				*(ltPtr++)='0';
				*(ltPtr++)='.';
				for(int i=currentExponent+1;i<0;++i)
					*(ltPtr++)='0';
				}
			ltPtr+=snprintf(ltPtr,ltEnd-ltPtr,"%d",currentMantissa);
			if(currentExponent>0)
				{
				for(int i=0;i<currentExponent;++i)
					*(ltPtr++)='0';
				}
			}
		*ltPtr='\0';
		if(unit.unit!=Geometry::LinearUnit::UNKNOWN)
			snprintf(ltPtr,ltEnd-ltPtr," %s",unit.getAbbreviation());
		lengthLabel->setString(labelText);
		GLLabel::Box::Vector labelSize=lengthLabel->getLabelSize();
		lengthLabel->setOrigin(GLLabel::Box::Vector(-labelSize[0]*0.5f,-labelSize[1]*1.5f,0.0f));
		}
	
	/* Calculate the scaling factor from navigational space to physical space: */
	if(unit.isImperial())
		{
		/* Calculate scale factor through imperial units: */
		currentScale=unit.getInchFactor()*navScale/getInchFactor();
		}
	else
		{
		/* Calculate scale factor through metric units: */
		currentScale=unit.getMeterFactor()*navScale/getMeterFactor();
		}
	
	/* Update the scale label: */
	char scaleLabelText[40];
	if(currentScale>=Scalar(1))
		snprintf(scaleLabelText,sizeof(scaleLabelText),"%g:1",double(currentScale));
	else
		snprintf(scaleLabelText,sizeof(scaleLabelText),"1:%g",1.0/double(currentScale));
	scaleLabel->setString(scaleLabelText);
	GLLabel::Box::Vector scaleLabelSize=scaleLabel->getLabelSize();
	scaleLabel->setOrigin(GLLabel::Box::Vector(-scaleLabelSize[0]*0.5f,-scaleLabelSize[1]*1.5f,0.0f));
	}

void ScaleBar::navigationChangedCallback(NavigationTransformationChangedCallbackData* cbData)
	{
	/* Check if the navigation scale changed: */
	if(cbData->oldTransform.getScaling()!=cbData->newTransform.getScaling())
		{
		/* Update the scale bar: */
		calcSize(cbData->newTransform);
		
		/* Resize the widget: */
		GLMotif::Vector newSize=calcNaturalSize();
		GLMotif::Vector newOrigin=GLMotif::Vector(0.0f,0.0f,0.0f);
		newOrigin[0]=-newSize[0]*0.5f;
		resize(GLMotif::Box(newOrigin,newSize));
		}
	}

ScaleBar::ScaleBar(const char* sName,GLMotif::WidgetManager* sManager)
	:GLMotif::Widget(sName,0,false),manager(sManager),
	 targetLength(getDisplaySize()*Scalar(0.2)),
	 currentMantissa(1),currentExponent(0),currentNavLength(1),currentScale(1),
	 lengthLabel(0),scaleLabel(0),
	 currentPhysLength(0)
	{
	/* Set widget parameters: */
	setBorderWidth(0.0f);
	setBorderType(GLMotif::Widget::PLAIN);
	
	/* Set default background and foreground colors: */
	Color bgColor=Vrui::getBackgroundColor();
	bgColor[3]=0.0f;
	Color fgColor;
	for(int i=0;i<3;++i)
		fgColor[i]=1.0f-bgColor[i];
	fgColor[3]=1.0f;
	setBorderColor(bgColor);
	setBackgroundColor(bgColor);
	setForegroundColor(fgColor);
	
	/* Create the initial scale bar length label: */
	if(getCoordinateManager()->getUnit().unit!=Geometry::LinearUnit::UNKNOWN)
		{
		char labelText[10];
		snprintf(labelText,sizeof(labelText),"1 %s",getCoordinateManager()->getUnit().getAbbreviation());
		lengthLabel=new GLLabel(labelText,*getUiFont());
		}
	else
		lengthLabel=new GLLabel("1",*getUiFont());
	lengthLabel->setBackground(bgColor);
	lengthLabel->setForeground(fgColor);
	scaleLabel=new GLLabel("1:1",*getUiFont());
	scaleLabel->setBackground(bgColor);
	scaleLabel->setForeground(fgColor);
	
	/* Calculate the initial navigation-space scale bar length: */
	calcSize(getNavigationTransformation());
	
	/* Resize the widget: */
	GLMotif::Vector newSize=calcNaturalSize();
	GLMotif::Vector newOrigin=GLMotif::Vector(0.0f,0.0f,0.0f);
	newOrigin[0]=-newSize[0]*0.5f;
	resize(GLMotif::Box(newOrigin,newSize));
	
	/* Register a navigation change callback with the Vrui kernel: */
	getNavigationTransformationChangedCallbacks().add(this,&ScaleBar::navigationChangedCallback);
	}

ScaleBar::~ScaleBar(void)
	{
	/* Pop down the widget: */
	manager->popdownWidget(this);
	
	/* Unregister the navigation change callback with the Vrui kernel: */
	getNavigationTransformationChangedCallbacks().remove(this,&ScaleBar::navigationChangedCallback);
	
	/* Delete the length and scale labels: */
	delete lengthLabel;
	delete scaleLabel;
	
	/* Unmanage the widget itself: */
	manager->unmanageWidget(this);
	}

GLMotif::Vector ScaleBar::calcNaturalSize(void) const
	{
	/* Get the scale bar's size: */
	GLMotif::Vector result(float(currentPhysLength),getUiFont()->getTextHeight()*3.0f,0.0f);
	
	if(lengthLabel!=0)
		{
		/* Adjust for the label size: */
		const GLLabel::Box::Vector& labelSize=lengthLabel->getLabelSize();
		if(result[0]<labelSize[0])
			result[0]=labelSize[0];
		}
	
	if(scaleLabel!=0)
		{
		/* Adjust for the label size: */
		const GLLabel::Box::Vector& labelSize=scaleLabel->getLabelSize();
		if(result[0]<labelSize[0])
			result[0]=labelSize[0];
		}
	
	/* Calculate the scale bar's current size: */
	return calcExteriorSize(result);
	}

void ScaleBar::resize(const GLMotif::Box& newExterior)
	{
	/* Resize the parent class widget: */
	GLMotif::Widget::resize(newExterior);
	
	if(lengthLabel!=0)
		{
		/* Reposition the length label: */
		const GLLabel::Box::Vector& labelSize=lengthLabel->getLabelSize();
		GLLabel::Box::Vector labelPos;
		labelPos[0]=getInterior().origin[0]+(getInterior().size[0]-labelSize[0])*0.5f;
		labelPos[1]=getInterior().origin[1]+getInterior().size[1]*0.5f-labelSize[1]*1.5f;
		labelPos[2]=0.0f;
		lengthLabel->setOrigin(labelPos);
		}
	
	if(scaleLabel!=0)
		{
		/* Reposition the scale label: */
		const GLLabel::Box::Vector& labelSize=scaleLabel->getLabelSize();
		GLLabel::Box::Vector labelPos;
		labelPos[0]=getInterior().origin[0]+(getInterior().size[0]-labelSize[0])*0.5f;
		labelPos[1]=getInterior().origin[1]+getInterior().size[1]*0.5f+labelSize[1]*0.5f;
		labelPos[2]=0.0f;
		scaleLabel->setOrigin(labelPos);
		}
	}

void ScaleBar::draw(GLContextData& contextData) const
	{
	/* Save and set OpenGL state: */
	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_ENABLE_BIT|GL_LINE_BIT);
	glDisable(GL_LIGHTING);
	
	/* Calculate the scale bar layout: */
	GLfloat x0=getInterior().origin[0]+(getInterior().size[0]-GLfloat(currentPhysLength))*0.5f;
	GLfloat x1=x0+GLfloat(currentPhysLength);
	const GLLabel::Box::Vector& labelSize=lengthLabel->getLabelSize();
	GLfloat y0=getInterior().origin[1]+(getInterior().size[1]-labelSize[1]*2.0f)*0.5f;
	GLfloat y1=y0+labelSize[1];
	GLfloat y2=y1+labelSize[1];
	
	/* Draw the scale bar: */
	glLineWidth(5.0f);
	glBegin(GL_LINES);
	glColor(getBackgroundColor());
	glVertex2f(x0,y1);
	glVertex2f(x1,y1);
	glEnd();
	
	glLineWidth(3.0f);
	glBegin(GL_LINES);
	glVertex2f(x0,y0);
	glVertex2f(x0,y2);
	glVertex2f(x1,y0);
	glVertex2f(x1,y2);
	
	glColor(getForegroundColor());
	glVertex2f(x0,y1);
	glVertex2f(x1,y1);
	glEnd();
	
	glLineWidth(1.0f);
	glBegin(GL_LINES);
	glVertex2f(x0,y0);
	glVertex2f(x0,y2);
	glVertex2f(x1,y0);
	glVertex2f(x1,y2);
	glEnd();
	
	/* Install a temporary deferred renderer: */
	{
	GLLabel::DeferredRenderer dr(contextData);
	
	/* Draw the length and scale labels: */
	// glEnable(GL_ALPHA_TEST);
	// glAlphaFunc(GL_GREATER,0.0f);
	lengthLabel->draw(contextData);
	scaleLabel->draw(contextData);
	}
	
	/* Restore OpenGL state: */
	glPopAttrib();
	}

void ScaleBar::pointerButtonDown(GLMotif::Event& event)
	{
	Scalar newScale=currentScale;
	
	/* Check if the event happened in the left or right corner: */
	float relEventPos=(event.getWidgetPoint().getPoint()[0]-getInterior().origin[0])/getInterior().size[0];
	if(relEventPos<=0.333f)
		{
		/* Calculate the next smaller quasi-binary scale factor: */
		newScale=Scalar(getSmallerQuasiBinary(currentScale));
		}
	else if(relEventPos>=0.667f)
		{
		/* Calculate the next bigger quasi-binary scale factor: */
		newScale=Scalar(getBiggerQuasiBinary(currentScale));
		}
	
	if(newScale!=currentScale&&activateNavigationTool(reinterpret_cast<const Tool*>(this)))
		{
		/* Adjust the navigation transformation: */
		Scalar newNavScale;
		const Geometry::LinearUnit& unit=getCoordinateManager()->getUnit();
		if(unit.isImperial())
			{
			/* Calculate scale factor through imperial units: */
			newNavScale=getInchFactor()*newScale/unit.getInchFactor();
			}
		else
			{
			/* Calculate scale factor through metric units: */
			newNavScale=getMeterFactor()*newScale/unit.getMeterFactor();
			}
		
		/* Get the current navigation transformation and calculate the display center position in navigation coordinates: */
		const NavTransform& nav=getNavigationTransformation();
		Point center=nav.inverseTransform(getDisplayCenter());
		
		/* Create the new navigation transformation: */
		NavTransform newNav=NavTransform(nav.getTranslation(),nav.getRotation(),newNavScale);
		newNav.leftMultiply(NavTransform::translate(getDisplayCenter()-newNav.transform(center)));
		setNavigationTransformation(newNav);
		
		deactivateNavigationTool(reinterpret_cast<const Tool*>(this));
		currentScale=newScale;
		
		/* Update the scale bar: */
		calcSize(newNav);
		
		/* Resize the widget so that the clicked point stays in the same place: */
		GLMotif::Vector newSize=calcNaturalSize();
		GLfloat newInteriorWidth=newSize[0]-2.0f*getBorderWidth();
		GLfloat newOrigin=event.getWidgetPoint().getPoint()[0]-newInteriorWidth*relEventPos-getBorderWidth();
		resize(GLMotif::Box(GLMotif::Vector(newOrigin,0.0f,0.0f),newSize));
		}
	}

void ScaleBar::pointerButtonUp(GLMotif::Event& event)
	{
	}

}
