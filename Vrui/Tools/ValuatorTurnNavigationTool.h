/***********************************************************************
ValuatorTurnNavigationTool - Class providing a rotation navigation tool
using two valuators.
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

#ifndef VRUI_VALUATORTURNNAVIGATIONTOOL_INCLUDED
#define VRUI_VALUATORTURNNAVIGATIONTOOL_INCLUDED

#include <Vrui/NavigationTool.h>

namespace Vrui {

class ValuatorTurnNavigationTool;

class ValuatorTurnNavigationToolFactory:public ToolFactory
	{
	friend class ValuatorTurnNavigationTool;
	
	/* Elements: */
	private:
	Scalar valuatorThreshold; // Threshold value beyond which a valuator is considered "pressed"
	Vector flyDirection; // Flying direction of tool in device coordinates
	Scalar flyFactor; // Velocity multiplication factor
	Vector rotationAxis0; // First rotation axis of tool in physical coordinates
	Vector rotationAxis1; // Second rotation axis of tool in physical coordinates
	Point rotationCenter; // Center point of rotation in device coordinates
	Scalar rotationFactor; // Angular velocity multiplication factor
	
	/* Constructors and destructors: */
	public:
	ValuatorTurnNavigationToolFactory(ToolManager& toolManager);
	virtual ~ValuatorTurnNavigationToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual const char* getValuatorFunction(int valuatorSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class ValuatorTurnNavigationTool:public NavigationTool
	{
	friend class ValuatorTurnNavigationToolFactory;
	
	/* Elements: */
	private:
	static ValuatorTurnNavigationToolFactory* factory; // Pointer to the factory object for this class
	
	/* Transient navigation state: */
	bool buttonState; // Current value of the associated button
	Scalar currentValues[2]; // Current value of the associated valuators
	
	/* Constructors and destructors: */
	public:
	ValuatorTurnNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void valuatorCallback(int valuatorSlotIndex,InputDevice::ValuatorCallbackData* cbData);
	virtual void frame(void);
	};

}

#endif
