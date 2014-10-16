/***********************************************************************
ValuatorFlyTurnNavigationTool - Class providing a fly navigation tool
with turning using two valuators.
Copyright (c) 2005-2010 Oliver Kreylos

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

#ifndef VRUI_VALUATORFLYTURNNAVIGATIONTOOL_INCLUDED
#define VRUI_VALUATORFLYTURNNAVIGATIONTOOL_INCLUDED

#include <Vrui/NavigationTool.h>

/* Forward declarations: */
namespace Vrui {
class Viewer;
}

namespace Vrui {

class ValuatorFlyTurnNavigationTool;

class ValuatorFlyTurnNavigationToolFactory:public ToolFactory
	{
	friend class ValuatorFlyTurnNavigationTool;
	
	/* Elements: */
	private:
	Scalar valuatorThreshold; // Threshold value beyond which a valuator is considered "pressed"
	Scalar valuatorExponent; // Exponent for valuator values applied after threshold
	Scalar superAccelerationFactor; // Factor for super acceleration if valuator is pressed to limit
	bool flyDirectionDeviceCoordinates; // Flag whether the flying direction is specified in device coordinates
	Vector flyDirection; // Flying direction of tool in device coordinates or physical coordinates
	Scalar flyFactor; // Velocity multiplication factor
	bool rotationAxisDeviceCoordinates; // Flag whether the rotation axis is specified in device coordinates
	Vector rotationAxis; // Rotation axis of tool in device coordinates or physical coordinates
	bool rotationCenterDeviceCoordinates; // Flag whether the rotation center is specified in device coordinates
	Point rotationCenter; // Center point of rotation in device coordinates or physical coordinates
	Scalar rotationFactor; // Angular velocity multiplication factor
	
	/* Constructors and destructors: */
	public:
	ValuatorFlyTurnNavigationToolFactory(ToolManager& toolManager);
	virtual ~ValuatorFlyTurnNavigationToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getValuatorFunction(int valuatorSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class ValuatorFlyTurnNavigationTool:public NavigationTool
	{
	friend class ValuatorFlyTurnNavigationToolFactory;
	
	/* Elements: */
	private:
	static ValuatorFlyTurnNavigationToolFactory* factory; // Pointer to the factory object for this class
	
	/* Transient navigation state: */
	Scalar currentValues[2]; // Current value of the associated valuators
	Scalar superAcceleration; // Current super acceleration
	
	/* Constructors and destructors: */
	public:
	ValuatorFlyTurnNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual const ToolFactory* getFactory(void) const;
	virtual void valuatorCallback(int valuatorSlotIndex,InputDevice::ValuatorCallbackData* cbData);
	virtual void frame(void);
	};

}

#endif
