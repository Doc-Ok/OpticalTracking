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

#ifndef VRUI_DRAGGINGTOOLADAPTER_INCLUDED
#define VRUI_DRAGGINGTOOLADAPTER_INCLUDED

#include <Vrui/DraggingTool.h>

/* Forward declarations: */
namespace Misc {
class CallbackData;
}

namespace Vrui {

class DraggingToolAdapter
	{
	/* Elements: */
	private:
	DraggingTool* tool; // Pointer to dragging tool associated with this adapter
	
	/* Constructors and destructors: */
	public:
	DraggingToolAdapter(DraggingTool* sTool); // Creates an adapter connected to the given dragging tool
	virtual ~DraggingToolAdapter(void); // Destroys an adapter
	
	/* Methods: */
	DraggingTool* getTool(void) const // Returns pointer to dragging tool
		{
		return tool;
		}
	virtual void storeState(Misc::ConfigurationFileSection& configFileSection) const; // Stores the tool adapter's state to the given configuration file
	virtual void getName(std::string& name) const; // Returns a descriptive name for the tool adapter
	virtual void idleMotionCallback(DraggingTool::IdleMotionCallbackData* cbData); // Callback called when not dragging
	virtual void dragStartCallback(DraggingTool::DragStartCallbackData* cbData); // Callback called before dragging starts
	virtual void dragCallback(DraggingTool::DragCallbackData* cbData); // Callback called during dragging
	virtual void dragEndCallback(DraggingTool::DragEndCallbackData* cbData); // Callback called after dragging ends
	};

}

#endif
