/***********************************************************************
ToggleButtonTool - Class to convert a set of regular buttons into toggle
buttons.
Copyright (c) 2013 Oliver Kreylos

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

#ifndef VRUI_TOGGLEBUTTONTOOL_INCLUDED
#define VRUI_TOGGLEBUTTONTOOL_INCLUDED

#include <Vrui/TransformTool.h>

namespace Vrui {

class ToggleButtonTool;

class ToggleButtonToolFactory:public ToolFactory
	{
	friend class ToggleButtonTool;
	
	/* Elements: */
	private:
	bool onRisingEdge; // Flag whether to change a toggle button state on the source button's rising edge or falling edge
	
	/* Constructors and destructors: */
	public:
	ToggleButtonToolFactory(ToolManager& toolManager);
	virtual ~ToggleButtonToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class ToggleButtonTool:public TransformTool
	{
	friend class ToggleButtonToolFactory;
	
	/* Elements: */
	private:
	static ToggleButtonToolFactory* factory; // Pointer to the factory object for this class
	
	/* Constructors and destructors: */
	public:
	ToggleButtonTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	virtual ~ToggleButtonTool(void);
	
	/* Methods from Tool: */
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	};

}

#endif
