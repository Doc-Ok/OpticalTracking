/***********************************************************************
DraggingToolAdapter - Adapter class to connect a generic dragging tool
to application functionality.
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

#include <Vrui/DraggingToolAdapter.h>

#include <Misc/FunctionCalls.h>

namespace Vrui {

/************************************
Methods of class DraggingToolAdapter:
************************************/

DraggingToolAdapter::DraggingToolAdapter(DraggingTool* sTool)
	:tool(sTool)
	{
	/* Register functions with the dragging tool: */
	tool->setStoreStateFunction(Misc::createFunctionCall(this,&DraggingToolAdapter::storeState));
	tool->setGetNameFunction(Misc::createFunctionCall(this,&DraggingToolAdapter::getName));
	
	/* Register callbacks with the dragging tool: */
	tool->getIdleMotionCallbacks().add(this,&DraggingToolAdapter::idleMotionCallback);
	tool->getDragStartCallbacks().add(this,&DraggingToolAdapter::dragStartCallback);
	tool->getDragCallbacks().add(this,&DraggingToolAdapter::dragCallback);
	tool->getDragEndCallbacks().add(this,&DraggingToolAdapter::dragEndCallback);
	}

DraggingToolAdapter::~DraggingToolAdapter(void)
	{
	/* Unregister functions from the dragging tool: */
	tool->setStoreStateFunction(0);
	tool->setGetNameFunction(0);
	
	/* Unregister callbacks from the dragging tool: */
	tool->getIdleMotionCallbacks().remove(this,&DraggingToolAdapter::idleMotionCallback);
	tool->getDragStartCallbacks().remove(this,&DraggingToolAdapter::dragStartCallback);
	tool->getDragCallbacks().remove(this,&DraggingToolAdapter::dragCallback);
	tool->getDragEndCallbacks().remove(this,&DraggingToolAdapter::dragEndCallback);
	}

void DraggingToolAdapter::storeState(Misc::ConfigurationFileSection& configFileSection) const
	{
	/* No default behavior... */
	}

void DraggingToolAdapter::getName(std::string& name) const
	{
	/* No default behavior... */
	}

void DraggingToolAdapter::idleMotionCallback(DraggingTool::IdleMotionCallbackData*)
	{
	/* No default behaviour... */
	}

void DraggingToolAdapter::dragStartCallback(DraggingTool::DragStartCallbackData*)
	{
	/* No default behaviour... */
	}

void DraggingToolAdapter::dragCallback(DraggingTool::DragCallbackData*)
	{
	/* No default behaviour... */
	}

void DraggingToolAdapter::dragEndCallback(DraggingTool::DragEndCallbackData*)
	{
	/* No default behaviour... */
	}

}
