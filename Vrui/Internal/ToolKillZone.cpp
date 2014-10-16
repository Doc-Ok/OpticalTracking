/***********************************************************************
ToolKillZone - Base class for "kill zones" for tools and input devices.
Copyright (c) 2004-2014 Oliver Kreylos

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

#include <Vrui/Internal/ToolKillZone.h>

#include <string>
#include <Misc/ThrowStdErr.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <GL/gl.h>
#include <GL/GLValueCoders.h>
#include <GL/GLContextData.h>
#include <GL/GLTransformationWrappers.h>
#include <Vrui/InputDevice.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/Vrui.h>

namespace Vrui {

/***************************************
Methods of class ToolKillZone::DataItem:
***************************************/

ToolKillZone::DataItem::DataItem(void)
	:modelDisplayListId(glGenLists(1)),
	 modelVersion(0)
	{
	}

ToolKillZone::DataItem::~DataItem(void)
	{
	glDeleteLists(modelDisplayListId,1);
	}

/*****************************
Methods of class ToolKillZone:
*****************************/

void ToolKillZone::updateModel(void)
	{
	/* Bump up the version number: */
	++modelVersion;
	}

ToolKillZone::ToolKillZone(const Misc::ConfigurationFileSection& configFileSection)
	:baseDevice(0),
	 render(true),
	 material(getWidgetMaterial()),
	 modelVersion(1)
	{
	/* Retrieve the name of the base device (if any) from the configuration file: */
	std::string baseDeviceName=configFileSection.retrieveString("./killZoneBaseDevice","");
	if(baseDeviceName!="")
		{
		/* Attach the kill zone to the base device: */
		baseDevice=getInputDeviceManager()->findInputDevice(baseDeviceName.c_str());
		if(baseDevice==0)
			Misc::throwStdErr("ToolKillZone: Unknown base input device \"%s\"",baseDeviceName.c_str());
		}
	
	/* Retrieve the render flag: */
	render=configFileSection.retrieveValue<bool>("./killZoneRender",render);
	
	/* Retrieve the model material: */
	material=configFileSection.retrieveValue<GLMaterial>("./killZoneMaterial",material);
	}

ToolKillZone::~ToolKillZone(void)
	{
	}

void ToolKillZone::initContext(GLContextData& contextData) const
	{
	/* Create a new context data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	}

void ToolKillZone::glRenderAction(GLContextData& contextData) const
	{
	if(render)
		{
		/* Get pointer to context data item: */
		DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
		
		if(baseDevice!=0)
			{
			/* Go to base device's coordinate system: */
			glPushMatrix();
			glMultMatrix(baseDevice->getTransformation());
			}
		
		/* Check if the model display list is up-to-date: */
		if(modelVersion!=dataItem->modelVersion)
			{
			/* Update and render the kill zone model: */
			glNewList(dataItem->modelDisplayListId,GL_COMPILE_AND_EXECUTE);
			glMaterial(GLMaterialEnums::FRONT,material);
			renderModel();
			glEndList();
			dataItem->modelVersion=modelVersion;
			}
		else
			{
			/* Render the kill zone model from the display list: */
			glCallList(dataItem->modelDisplayListId);
			}
		
		if(baseDevice!=0)
			{
			/* Go back to physical coordinates: */
			glPopMatrix();
			}
		}
	}

}
