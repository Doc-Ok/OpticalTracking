/***********************************************************************
NopTool - Class for tools that do nothing at all. Used to disable
unwanted buttons or valuators that might otherwise interfere with use.
Copyright (c) 2011 Oliver Kreylos

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

#include <Vrui/Tools/NopTool.h>

#include <Vrui/ToolManager.h>

namespace Vrui {

/*******************************
Methods of class NopToolFactory:
*******************************/

NopToolFactory::NopToolFactory(ToolManager& toolManager)
	:ToolFactory("NopTool",toolManager)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(0,true);
	layout.setNumValuators(0,true);
	
	/* Insert class into class hierarchy: */
	ToolFactory* toolFactory=toolManager.loadClass("UtilityTool");
	toolFactory->addChildClass(this);
	addParentClass(toolFactory);
	
	/* Set tool class' factory pointer: */
	NopTool::factory=this;
	}

NopToolFactory::~NopToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	NopTool::factory=0;
	}

const char* NopToolFactory::getName(void) const
	{
	return "Do Nothing";
	}

const char* NopToolFactory::getButtonFunction(int) const
	{
	return "Nothing";
	}

const char* NopToolFactory::getValuatorFunction(int) const
	{
	return "Nothing";
	}

Tool* NopToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new NopTool(this,inputAssignment);
	}

void NopToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveNopToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("UtilityTool");
	}

extern "C" ToolFactory* createNopToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	NopToolFactory* nopToolFactory=new NopToolFactory(*toolManager);
	
	/* Return factory object: */
	return nopToolFactory;
	}

extern "C" void destroyNopToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/********************************
Static elements of class NopTool:
********************************/

NopToolFactory* NopTool::factory=0;

/************************
Methods of class NopTool:
************************/

NopTool::NopTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:UtilityTool(factory,inputAssignment)
	{
	}

const ToolFactory* NopTool::getFactory(void) const
	{
	return factory;
	}

void NopTool::buttonCallback(int,InputDevice::ButtonCallbackData*)
	{
	}

void NopTool::valuatorCallback(int,InputDevice::ValuatorCallbackData*)
	{
	}

}
