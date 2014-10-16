/***********************************************************************
UtilityTool - Base class for tools providing additional functions to VR
applications, without being tied directly into the application's user
interface.
Copyright (c) 2008-2010 Oliver Kreylos

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

#include <Vrui/UtilityTool.h>

#include <Vrui/ToolManager.h>

namespace Vrui {

/***********************************
Methods of class UtilityToolFactory:
***********************************/

UtilityToolFactory::UtilityToolFactory(ToolManager& toolManager)
	:ToolFactory("UtilityTool",toolManager)
	{
	#if 0
	/* Insert class into class hierarchy: */
	ToolFactory* toolFactory=toolManager.loadClass("Tool");
	toolFactory->addChildClass(this);
	addParentClass(toolFactory);
	#endif
	}

const char* UtilityToolFactory::getName(void) const
	{
	return "Utility";
	}

/****************************
Methods of class UtilityTool:
****************************/

UtilityTool::UtilityTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:Tool(factory,inputAssignment)
	{
	}

}
