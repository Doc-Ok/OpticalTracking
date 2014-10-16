/***********************************************************************
ScreenLocator - Simple locator class to use in simulated VR
environments.
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

#ifndef VRUI_SCREENLOCATOR_INCLUDED
#define VRUI_SCREENLOCATOR_INCLUDED

#include <Geometry/OrthogonalTransformation.h>
#include <Vrui/LocatorTool.h>

namespace Vrui {

/* Forward declarations: */
class ScreenLocatorTool;

class ScreenLocatorToolFactory:public ToolFactory
	{
	friend class ScreenLocatorTool;
	
	/* Elements: */
	private:
	Scalar crosshairSize; // Size of the screen-aligned crosshairs
	
	/* Constructors and destructors: */
	public:
	ScreenLocatorToolFactory(ToolManager& toolManager);
	virtual ~ScreenLocatorToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class ScreenLocatorTool:public LocatorTool
	{
	friend class ScreenLocatorToolFactory;
	
	/* Elements: */
	private:
	static ScreenLocatorToolFactory* factory; // Pointer to the factory object for this class
	NavTrackerState currentTransformation; // The current locator transformation in physical coordinates
	
	/* Private methods: */
	void calcTransformation(void); // Calculates the current screen transformation
	
	/* Constructors and destructors: */
	public:
	ScreenLocatorTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	};

}

#endif
