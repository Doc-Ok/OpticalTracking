/***********************************************************************
LocatorToolAdapter - Adapter class to connect a generic locator tool to
application functionality.
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

#include <Vrui/LocatorToolAdapter.h>

#include <Misc/FunctionCalls.h>

namespace Vrui {

/***********************************
Methods of class LocatorToolAdapter:
***********************************/

LocatorToolAdapter::LocatorToolAdapter(LocatorTool* sTool)
	:tool(sTool)
	{
	/* Register functions with the locator tool: */
	tool->setStoreStateFunction(Misc::createFunctionCall(this,&LocatorToolAdapter::storeState));
	tool->setGetNameFunction(Misc::createFunctionCall(this,&LocatorToolAdapter::getName));
	
	/* Register callbacks with the locator tool: */
	tool->getMotionCallbacks().add(this,&LocatorToolAdapter::motionCallback);
	tool->getButtonPressCallbacks().add(this,&LocatorToolAdapter::buttonPressCallback);
	tool->getButtonReleaseCallbacks().add(this,&LocatorToolAdapter::buttonReleaseCallback);
	}

LocatorToolAdapter::~LocatorToolAdapter(void)
	{
	/* Unregister functions from the locator tool: */
	tool->setStoreStateFunction(0);
	tool->setGetNameFunction(0);
	
	/* Unregister callbacks from the locator tool: */
	tool->getMotionCallbacks().remove(this,&LocatorToolAdapter::motionCallback);
	tool->getButtonPressCallbacks().remove(this,&LocatorToolAdapter::buttonPressCallback);
	tool->getButtonReleaseCallbacks().remove(this,&LocatorToolAdapter::buttonReleaseCallback);
	}

void LocatorToolAdapter::storeState(Misc::ConfigurationFileSection& configFileSection) const
	{
	/* No default behavior... */
	}

void LocatorToolAdapter::getName(std::string& name) const
	{
	/* No default behavior... */
	}

void LocatorToolAdapter::motionCallback(LocatorTool::MotionCallbackData*)
	{
	/* No default behaviour... */
	}

void LocatorToolAdapter::buttonPressCallback(LocatorTool::ButtonPressCallbackData*)
	{
	/* No default behaviour... */
	}

void LocatorToolAdapter::buttonReleaseCallback(LocatorTool::ButtonReleaseCallbackData*)
	{
	/* No default behaviour... */
	}

}
