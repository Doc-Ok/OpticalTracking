/***********************************************************************
SixDofNavigationTool - Class for 6-DOF navigation by grabbing space
using a single input device.
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

#ifndef VRUI_SIXDOFNAVIGATIONTOOL_INCLUDED
#define VRUI_SIXDOFNAVIGATIONTOOL_INCLUDED

#include <Geometry/OrthogonalTransformation.h>
#include <Vrui/NavigationTool.h>

namespace Vrui {

class SixDofNavigationTool;

class SixDofNavigationToolFactory:public ToolFactory
	{
	friend class SixDofNavigationTool;
	
	/* Constructors and destructors: */
	public:
	SixDofNavigationToolFactory(ToolManager& toolManager);
	virtual ~SixDofNavigationToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class SixDofNavigationTool:public NavigationTool
	{
	friend class SixDofNavigationToolFactory;
	
	/* Elements: */
	private:
	static SixDofNavigationToolFactory* factory; // Pointer to the factory object for this class
	
	/* Transient navigation state: */
	NavTrackerState preScale; // Transformation to be applied to the navigation transformation before scaling
	
	/* Constructors and destructors: */
	public:
	SixDofNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	};

}

#endif
