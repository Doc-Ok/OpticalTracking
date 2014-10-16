/***********************************************************************
JediTool - Class for tools using light sabers to point out features in a
3D display.
Copyright (c) 2007-2014 Oliver Kreylos

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

#include <Vrui/Tools/JediTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Geometry/OrthonormalTransformation.h>
#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLGeometryWrappers.h>
#include <Images/ReadImageFile.h>
#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>
#include <Vrui/DisplayState.h>
#include <Vrui/Internal/Config.h>

namespace Vrui {

/********************************
Methods of class JediToolFactory:
********************************/

JediToolFactory::JediToolFactory(ToolManager& toolManager)
	:ToolFactory("JediTool",toolManager),
	 lightsaberLength(Scalar(48)*getInchFactor()),
	 lightsaberWidth(Scalar(6)*getInchFactor()),
	 baseOffset(Scalar(3)*getInchFactor()),
	 lightsaberImageFileName(std::string(VRUI_INTERNAL_CONFIG_SHAREDIR)+"/Textures/Lightsaber.png")
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* toolFactory=toolManager.loadClass("PointingTool");
	toolFactory->addChildClass(this);
	addParentClass(toolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	lightsaberLength=cfs.retrieveValue<Scalar>("./lightsaberLength",lightsaberLength);
	lightsaberWidth=cfs.retrieveValue<Scalar>("./lightsaberWidth",lightsaberWidth);
	baseOffset=cfs.retrieveValue<Scalar>("./baseOffset",baseOffset);
	lightsaberImageFileName=cfs.retrieveString("./lightsaberImageFileName",lightsaberImageFileName);
	
	/* Set tool class' factory pointer: */
	JediTool::factory=this;
	}

JediToolFactory::~JediToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	JediTool::factory=0;
	}

const char* JediToolFactory::getName(void) const
	{
	return "Jedi Tool";
	}

const char* JediToolFactory::getButtonFunction(int) const
	{
	return "Toggle on / off";
	}

Tool* JediToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new JediTool(this,inputAssignment);
	}

void JediToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveJediToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("UtilityTool");
	}

extern "C" ToolFactory* createJediToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	JediToolFactory* jediToolFactory=new JediToolFactory(*toolManager);
	
	/* Return factory object: */
	return jediToolFactory;
	}

extern "C" void destroyJediToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/*********************************
Static elements of class JediTool:
*********************************/

JediToolFactory* JediTool::factory=0;

/*************************
Methods of class JediTool:
*************************/

JediTool::JediTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:PointingTool(factory,inputAssignment),
	 GLObject(false),
	 lightsaberImage(Images::readImageFile(JediTool::factory->lightsaberImageFileName.c_str())),
	 active(false)
	{
	GLObject::init();
	}

const ToolFactory* JediTool::getFactory(void) const
	{
	return factory;
	}

void JediTool::buttonCallback(int,InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState) // Activation button has just been pressed
		{
		if(!active)
			{
			/* Activate the light saber: */
			active=true;
			activationTime=getApplicationTime();
			}
		else
			{
			/* Deactivate the light saber: */
			active=false;
			}
		}
	}

void JediTool::frame(void)
	{
	if(active)
		{
		/* Update the light saber billboard: */
		origin=getButtonDevicePosition(0);
		axis=getButtonDeviceRayDirection(0);
		
		/* Scale the lightsaber during activation: */
		length=factory->lightsaberLength;
		double activeTime=getApplicationTime()-activationTime;
		if(activeTime<1.5)
			{
			length*=activeTime/1.5;
			
			/* Request another frame: */
			scheduleUpdate(getApplicationTime()+1.0/125.0);
			}
		}
	}

void JediTool::initContext(GLContextData& contextData) const
	{
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Upload the light saber image as a 2D texture: */
	glBindTexture(GL_TEXTURE_2D,dataItem->textureObjectId);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_BASE_LEVEL,0);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,0);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	lightsaberImage.glTexImage2D(GL_TEXTURE_2D,0,GL_RGB);
	glBindTexture(GL_TEXTURE_2D,0);
	}

void JediTool::glRenderActionTransparent(GLContextData& contextData) const
	{
	if(active)
		{
		/* Get the data item: */
		DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
		
		/* Get the eye position for the current rendering pass from Vrui's display state: */
		const Point& eyePosition=Vrui::getDisplayState(contextData).eyePosition;
		
		/* Calculate the billboard size and orientation: */
		Vector y=axis;
		Vector x=axis^(eyePosition-origin);
		x.normalize();
		y*=length*scaleFactor;
		x*=Math::div2(factory->lightsaberWidth*scaleFactor);
		Point basePoint=origin;
		basePoint-=axis*(factory->baseOffset*scaleFactor);
		
		/* Draw the light saber: */
		glPushAttrib(GL_COLOR_BUFFER_BIT|GL_ENABLE_BIT|GL_POLYGON_BIT|GL_TEXTURE_BIT);
		glDisable(GL_LIGHTING);
		glBlendFunc(GL_ONE,GL_ONE);
		glDisable(GL_CULL_FACE);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,dataItem->textureObjectId);
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f,0.0f);
		glVertex(basePoint-x);
		glTexCoord2f(1.0f,0.0f);
		glVertex(basePoint+x);
		glTexCoord2f(1.0f,1.0f);
		glVertex(basePoint+x+y);
		glTexCoord2f(0.0f,1.0f);
		glVertex(basePoint-x+y);
		glEnd();
		glBindTexture(GL_TEXTURE_2D,0);
		glPopAttrib();
		}
	}

}
