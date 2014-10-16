/***********************************************************************
SixDofDraggingTool - Class for simple 6-DOF dragging using a single
input device.
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

#ifndef VRUI_SIXDOFDRAGGINGTOOL_INCLUDED
#define VRUI_SIXDOFDRAGGINGTOOL_INCLUDED

#include <Geometry/OrthogonalTransformation.h>
#include <Vrui/DraggingTool.h>

namespace Vrui {

/* Forward declarations: */
class SixDofDraggingTool;

class SixDofDraggingToolFactory:public ToolFactory
	{
	friend class SixDofDraggingTool;
	
	/* Constructors and destructors: */
	public:
	SixDofDraggingToolFactory(ToolManager& toolManager);
	virtual ~SixDofDraggingToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class SixDofDraggingTool:public DraggingTool
	{
	friend class SixDofDraggingToolFactory;
	
	enum DraggingMode // Enumerated type for states the tool can be in
		{
		IDLE,MOVING
		};
	
	/* Elements: */
	private:
	static SixDofDraggingToolFactory* factory; // Pointer to the factory object for this class
	
	/* Transient dragging state: */
	DraggingMode draggingMode; // The tool's current dragging mode
	NavTrackerState preScale; // Transformation to be applied to the dragging transformation before scaling
	
	/* Constructors and destructors: */
	public:
	SixDofDraggingTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	};

}

#endif
