/***********************************************************************
ClutchTool - Class to offset the position and orientation of an input
device using a "clutch" button to disengage a virtual device from a
source device.
Copyright (c) 2007-2010 Oliver Kreylos

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

#ifndef VRUI_CLUTCHTOOL_INCLUDED
#define VRUI_CLUTCHTOOL_INCLUDED

#include <Geometry/OrthonormalTransformation.h>
#include <Vrui/TransformTool.h>

namespace Vrui {

class ClutchTool;

class ClutchToolFactory:public ToolFactory
	{
	friend class ClutchTool;
	
	/* Elements: */
	private:
	bool clutchButtonToggleFlag; // Flag whether the clutch button acts as a toggle
	
	/* Constructors and destructors: */
	public:
	ClutchToolFactory(ToolManager& toolManager);
	virtual ~ClutchToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class ClutchTool:public TransformTool
	{
	friend class ClutchToolFactory;
	
	/* Elements: */
	private:
	static ClutchToolFactory* factory; // Pointer to the factory object for this class
	TrackerState offset; // Offset from source input device to transformed input device
	bool clutchButtonState; // Current state of the simulated clutch toggle button
	
	/* Constructors and destructors: */
	public:
	ClutchTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	virtual ~ClutchTool(void);
	
	/* Methods from Tool: */
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	};

}

#endif
