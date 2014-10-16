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

#ifndef VRUI_NOPTOOL_INCLUDED
#define VRUI_NOPTOOL_INCLUDED

#include <Vrui/UtilityTool.h>

namespace Vrui {

class NopTool;

class NopToolFactory:public ToolFactory
	{
	friend class NopTool;
	
	/* Constructors and destructors: */
	public:
	NopToolFactory(ToolManager& toolManager);
	virtual ~NopToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual const char* getValuatorFunction(int buttonSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class NopTool:public UtilityTool
	{
	friend class NopToolFactory;
	
	/* Elements: */
	private:
	static NopToolFactory* factory; // Pointer to the factory object for this class
	
	/* Constructors and destructors: */
	public:
	NopTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void valuatorCallback(int valuatorSlotIndex,InputDevice::ValuatorCallbackData* cbData);
	};

}

#endif
