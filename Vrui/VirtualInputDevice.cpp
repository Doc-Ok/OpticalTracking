/***********************************************************************
VirtualInputDevice - Helper class to manage ungrabbed virtual input
devices.
Copyright (c) 2005 Oliver Kreylos

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

#include <Math/Constants.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Ray.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/gl.h>
#include <GL/GLMaterial.h>
#include <Vrui/InputDevice.h>
#include <Vrui/Vrui.h>

#include <Vrui/VirtualInputDevice.h>

namespace Vrui {

namespace {

/****************
Helper functions:
****************/

inline bool isPointInsideCube(const Point& pos,const Point& center,Scalar halfSize)
	{
	bool inside=true;
	for(int i=0;i<3&&inside;++i)
		inside=Math::abs(pos[i]-center[i])<=halfSize;
	
	return inside;
	}

inline Scalar doesRayIntersectCube(const Ray& ray,const Point& center,Scalar halfSize)
	{
	Scalar lMin=Scalar(0);
	Scalar lMax=Math::Constants<Scalar>::max;
	for(int i=0;i<3;++i)
		{
		Scalar l1,l2;
		if(ray.getDirection()[i]<Scalar(0))
			{
			l1=(center[i]+halfSize-ray.getOrigin()[i])/ray.getDirection()[i];
			l2=(center[i]-halfSize-ray.getOrigin()[i])/ray.getDirection()[i];
			}
		else if(ray.getDirection()[i]>Scalar(0))
			{
			l1=(center[i]-halfSize-ray.getOrigin()[i])/ray.getDirection()[i];
			l2=(center[i]+halfSize-ray.getOrigin()[i])/ray.getDirection()[i];
			}
		else if(Math::abs(center[i]-ray.getOrigin()[i])<=halfSize)
			{
			l1=Scalar(0);
			l2=Math::Constants<Scalar>::max;
			}
		else
			l1=l2=Scalar(-1);
		if(lMin<l1)
			lMin=l1;
		if(lMax>l2)
			lMax=l2;
		}
	
	if(lMin<lMax)
		return lMin;
	else
		return Math::Constants<Scalar>::max;
	}

}

/***********************************
Methods of class VirtualInputDevice:
***********************************/

VirtualInputDevice::VirtualInputDevice(GlyphRenderer* sGlyphRenderer,const Misc::ConfigurationFileSection&)
	:glyphRenderer(sGlyphRenderer)
	{
	buttonOffset=Vector(0,0,1.25)*Scalar(glyphRenderer->getGlyphSize());
	buttonPanelDirection=Vector(1,0,0);
	buttonSize=Scalar(0.4); // Size is relative to the input device glyph size
	buttonSpacing=Scalar(glyphRenderer->getGlyphSize())*Scalar(0.6);
	offButtonGlyph.enable(Glyph::CUBE,GLMaterial(GLMaterial::Color(0.5f,0.5f,0.5f),GLMaterial::Color(0.3f,0.3f,0.3f),5.0f));
	onButtonGlyph.enable(Glyph::CUBE,GLMaterial(GLMaterial::Color(0.25f,1.0f,0.25f),GLMaterial::Color(0.3f,0.3f,0.3f),5.0f));
	deviceGlyph.enable(Glyph::BOX,GLMaterial(GLMaterial::Color(0.5f,0.5f,0.5f),GLMaterial::Color(0.3f,0.3f,0.3f),5.0f));
	}

VirtualInputDevice::~VirtualInputDevice(void)
	{
	}

bool VirtualInputDevice::pick(const InputDevice* device,const Point& pos) const
	{
	/* Transform the position to device coordinates: */
	Point devicePos=device->getTransformation().inverseTransform(pos);
	
	/* Calculate the device glyph size: */
	Scalar halfSize=Scalar(glyphRenderer->getGlyphSize())*Scalar(0.575);
	
	/* Test the device position against the device first: */
	if(isPointInsideCube(devicePos,Point::origin,halfSize))
		return true;
	
	/* Test the position against the device's buttons: */
	Vector buttonStep=buttonPanelDirection*buttonSpacing;
	int numButtons=device->getNumButtons();
	Point buttonCenter=device->getTransformation().getOrigin()+buttonOffset-buttonStep*(Scalar(0.5)*Scalar(numButtons-1));
	halfSize*=buttonSize;
	for(int i=0;i<numButtons;++i)
		{
		if(isPointInsideCube(pos,buttonCenter,halfSize))
			return true;
		buttonCenter+=buttonStep;
		}
	
	/* Test the position against the device's navigation button: */
	if(isPointInsideCube(pos,device->getTransformation().getOrigin()-buttonOffset,halfSize))
		return true;
	
	return false;
	}

Scalar VirtualInputDevice::pick(const InputDevice* device,const Ray& ray) const
	{
	/* Transform the ray to device coordinates: */
	Ray deviceRay=ray;
	deviceRay.inverseTransform(device->getTransformation());
	
	/* Calculate the device glyph size: */
	Scalar halfSize=Scalar(glyphRenderer->getGlyphSize())*Scalar(0.575);
	
	/* Test the ray against the device first: */
	Scalar lambdaMin=doesRayIntersectCube(deviceRay,Point::origin,halfSize);
	
	/* Test the ray against the device's buttons: */
	Vector buttonStep=buttonPanelDirection*buttonSpacing;
	int numButtons=device->getNumButtons();
	Point buttonCenter=device->getTransformation().getOrigin()+buttonOffset-buttonStep*(Scalar(0.5)*Scalar(numButtons-1));
	halfSize*=buttonSize;
	for(int i=0;i<numButtons;++i)
		{
		Scalar lambda=doesRayIntersectCube(ray,buttonCenter,halfSize);
		if(lambdaMin>lambda)
			lambdaMin=lambda;
		buttonCenter+=buttonStep;
		}
	
	Scalar lambda=doesRayIntersectCube(ray,device->getTransformation().getOrigin()-buttonOffset,halfSize);
	if(lambdaMin>lambda)
		lambdaMin=lambda;
	
	return lambdaMin;
	}

int VirtualInputDevice::pickButton(const InputDevice* device,const Point& pos) const
	{
	/* Calculate the device button size: */
	Scalar halfSize=Scalar(glyphRenderer->getGlyphSize())*Scalar(0.575)*buttonSize;
	
	/* Test the position against the device's buttons: */
	Vector buttonStep=buttonPanelDirection*buttonSpacing;
	int numButtons=device->getNumButtons();
	Point buttonCenter=device->getTransformation().getOrigin()+buttonOffset-buttonStep*(Scalar(0.5)*Scalar(numButtons-1));
	for(int i=0;i<numButtons;++i)
		{
		if(isPointInsideCube(pos,buttonCenter,halfSize))
			return i;
		buttonCenter+=buttonStep;
		}
	
	/* Test the position against the device's navigation button: */
	if(isPointInsideCube(pos,device->getTransformation().getOrigin()-buttonOffset,halfSize))
		return numButtons;
	
	return -1;
	}

int VirtualInputDevice::pickButton(const InputDevice* device,const Ray& ray) const
	{
	/* Calculate the device button size: */
	Scalar halfSize=Scalar(glyphRenderer->getGlyphSize())*Scalar(0.575)*buttonSize;
	
	/* Test the ray against the device's buttons: */
	Vector buttonStep=buttonPanelDirection*buttonSpacing;
	int numButtons=device->getNumButtons();
	Point buttonCenter=device->getTransformation().getOrigin()+buttonOffset-buttonStep*(Scalar(0.5)*Scalar(numButtons-1));
	Scalar lambdaMin=Math::Constants<Scalar>::max;
	int result=-1;
	for(int i=0;i<numButtons;++i)
		{
		Scalar lambda=doesRayIntersectCube(ray,buttonCenter,halfSize);
		if(lambdaMin>lambda)
			{
			lambdaMin=lambda;
			result=i;
			}
		buttonCenter+=buttonStep;
		}
	
	/* Test the ray against the device's navigation button: */
	Scalar lambda=doesRayIntersectCube(ray,device->getTransformation().getOrigin()-buttonOffset,halfSize);
	if(lambdaMin>lambda)
		{
		lambdaMin=lambda;
		result=numButtons;
		}
	
	return result;
	}

void VirtualInputDevice::renderDevice(const InputDevice* device,bool navigational,const GlyphRenderer::DataItem* glyphRendererContextDataItem,GLContextData&) const
	{
	/* Get the device's current transformation: */
	OGTransform transform(device->getTransformation());
	
	/* Render glyphs for the device's buttons: */
	int numButtons=device->getNumButtons();
	OGTransform buttonTransform=OGTransform::translate(transform.getTranslation()+buttonOffset-buttonPanelDirection*(Scalar(0.5)*buttonSpacing*Scalar(numButtons-1)));
	buttonTransform*=OGTransform::scale(buttonSize);
	Vector step=buttonPanelDirection*(buttonSpacing/buttonSize);
	for(int i=0;i<numButtons;++i)
		{
		glyphRenderer->renderGlyph(device->getButtonState(i)?onButtonGlyph:offButtonGlyph,buttonTransform,glyphRendererContextDataItem);
		buttonTransform*=OGTransform::translate(step);
		}
	
	/* Render a glyph for the device's navigational coordinate mode button: */
	buttonTransform=OGTransform::translate(transform.getTranslation()-buttonOffset);
	buttonTransform*=OGTransform::scale(buttonSize);
	glyphRenderer->renderGlyph(navigational?onButtonGlyph:offButtonGlyph,buttonTransform,glyphRendererContextDataItem);
	
	/* Render a glyph for the device itself: */
	glyphRenderer->renderGlyph(deviceGlyph,transform,glyphRendererContextDataItem);
	}

}
