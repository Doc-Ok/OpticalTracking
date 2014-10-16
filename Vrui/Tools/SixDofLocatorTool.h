/***********************************************************************
SixDofLocatorTool - Class for simple 6-DOF localization using a single
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

#ifndef VRUI_SIXDOFLOCATORTOOL_INCLUDED
#define VRUI_SIXDOFLOCATORTOOL_INCLUDED

#include <Vrui/LocatorTool.h>

namespace Vrui {

/* Forward declarations: */
class SixDofLocatorTool;

class SixDofLocatorToolFactory:public ToolFactory
	{
	friend class SixDofLocatorTool;
	
	/* Constructors and destructors: */
	public:
	SixDofLocatorToolFactory(ToolManager& toolManager);
	virtual ~SixDofLocatorToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class SixDofLocatorTool:public LocatorTool
	{
	friend class SixDofLocatorToolFactory;
	
	/* Elements: */
	private:
	static SixDofLocatorToolFactory* factory; // Pointer to the factory object for this class
	
	/* Constructors and destructors: */
	public:
	SixDofLocatorTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	};

}

#endif
