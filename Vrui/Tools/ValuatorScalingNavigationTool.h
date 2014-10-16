/***********************************************************************
ValuatorScalingNavigationTool - Class for tools that allow scaling the
navigation transformation using a valuator.
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

#ifndef VRUI_VALUATORSCALINGNAVIGATIONTOOL_INCLUDED
#define VRUI_VALUATORSCALINGNAVIGATIONTOOL_INCLUDED

#include <Geometry/Point.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Vrui/NavigationTool.h>

namespace Vrui {

class ValuatorScalingNavigationTool;

class ValuatorScalingNavigationToolFactory:public ToolFactory
	{
	friend class ValuatorScalingNavigationTool;
	
	/* Elements: */
	private:
	Scalar valuatorThreshold; // Threshold value beyond which a valuator is considered "pressed"
	Scalar scalingFactor; // Scaling multiplication factor
	
	/* Constructors and destructors: */
	public:
	ValuatorScalingNavigationToolFactory(ToolManager& toolManager);
	virtual ~ValuatorScalingNavigationToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getValuatorFunction(int valuatorSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class ValuatorScalingNavigationTool:public NavigationTool
	{
	friend class ValuatorScalingNavigationToolFactory;
	
	/* Elements: */
	private:
	static ValuatorScalingNavigationToolFactory* factory; // Pointer to the factory object for this class
	
	/* Transient navigation state: */
	Scalar currentValue; // Current value of the associated valuator
	NavTrackerState preScale; // Transformation to be applied to the navigation transformation before scaling
	Point scalingCenter; // Center position of scaling operation
	Scalar currentScale; // Current scaling factor
	NavTrackerState postScale; // Transformation to be applied to the navigation transformation after scaling
	
	/* Constructors and destructors: */
	public:
	ValuatorScalingNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual const ToolFactory* getFactory(void) const;
	virtual void valuatorCallback(int valuatorSlotIndex,InputDevice::ValuatorCallbackData* cbData);
	virtual void frame(void);
	};

}

#endif
