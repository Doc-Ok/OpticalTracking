/***********************************************************************
LocatorTool - Base class for tools encapsulating 6-DOF localization.
Copyright (c) 2004-2010 Oliver Kreylos

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

#include <Vrui/LocatorTool.h>

#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/***********************************
Methods of class LocatorToolFactory:
***********************************/

LocatorToolFactory::LocatorToolFactory(ToolManager& toolManager)
	:ToolFactory("LocatorTool",toolManager)
	{
	#if 0
	/* Insert class into class hierarchy: */
	ToolFactory* toolFactory=toolManager.loadClass("Tool");
	toolFactory->addChildClass(this);
	addParentClass(toolFactory);
	#endif
	}

const char* LocatorToolFactory::getName(void) const
	{
	return "Locator";
	}

const char* LocatorToolFactory::getButtonFunction(int) const
	{
	/* By default, locator tools only use a single button: */
	return "Locate";
	}

/****************************
Methods of class LocatorTool:
****************************/

LocatorTool::LocatorTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:Tool(factory,inputAssignment),
	 storeStateFunction(0),getNameFunction(0)
	{
	}

LocatorTool::~LocatorTool(void)
	{
	delete storeStateFunction;
	delete getNameFunction;
	}

void LocatorTool::storeState(Misc::ConfigurationFileSection& configFileSection) const
	{
	/* Call the function pointer if it is valid: */
	if(storeStateFunction!=0)
		(*storeStateFunction)(configFileSection);
	}

std::string LocatorTool::getName(void) const
	{
	/* Get the name of the tool class itself: */
	std::string result=getFactory()->getName();
	
	/* Append the name returned by the function pointer if it is valid and returns a non-empty string: */
	if(getNameFunction!=0)
		{
		/* Get the dependent name: */
		std::string dependentName;
		(*getNameFunction)(dependentName);
		
		if(!dependentName.empty())
			{
			/* Append the dependent name in parentheses: */
			result.append(" (");
			result.append(dependentName);
			result.push_back(')');
			}
		}
	
	return result;
	}

void LocatorTool::setStoreStateFunction(LocatorTool::StoreStateFunction* newStoreStateFunction)
	{
	/* Delete the previous function pointer: */
	delete storeStateFunction;
	
	/* Adopt the given function pointer: */
	storeStateFunction=newStoreStateFunction;
	}

void LocatorTool::setGetNameFunction(LocatorTool::GetNameFunction* newGetNameFunction)
	{
	/* Delete the previous function pointer: */
	delete getNameFunction;
	
	/* Adopt the given function pointer: */
	getNameFunction=newGetNameFunction;
	}

}
