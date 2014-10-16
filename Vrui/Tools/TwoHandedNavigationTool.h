/***********************************************************************
TwoHandedNavigationTool - Class encapsulating the behaviour of the old
famous Vrui two-handed navigation tool.
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

#ifndef VRUI_TWOHANDEDNAVIGATIONTOOL_INCLUDED
#define VRUI_TWOHANDEDNAVIGATIONTOOL_INCLUDED

#include <Geometry/Point.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Vrui/NavigationTool.h>

namespace Vrui {

class TwoHandedNavigationTool;

class TwoHandedNavigationToolFactory:public ToolFactory
	{
	friend class TwoHandedNavigationTool;
	
	/* Constructors and destructors: */
	public:
	TwoHandedNavigationToolFactory(ToolManager& toolManager);
	virtual ~TwoHandedNavigationToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class TwoHandedNavigationTool:public NavigationTool
	{
	friend class TwoHandedNavigationToolFactory;
	
	/* Embedded classes: */
	public:
	enum NavigationMode // Enumerated type for states the tool can be in
		{
		IDLE,MOVING,SCALING
		};
	
	/* Elements: */
	private:
	static TwoHandedNavigationToolFactory* factory; // Pointer to the factory object for this class
	
	/* Transient navigation state: */
	NavigationMode navigationMode; // The tool's current navigation mode
	int movingButtonSlotIndex; // Index of the button slot whose device is responsible for moving
	NavTrackerState preScale; // Transformation to be applied to the navigation transformation before scaling
	Point scalingCenter; // Center position of scaling operation
	Scalar initialScale; // Initial distance between input devices
	NavTrackerState postScale; // Transformation to be applied to the navigation transformation after scaling
	
	/* Constructors and destructors: */
	public:
	TwoHandedNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	};

}

#endif
