/***********************************************************************
ShiftButtonTool - Class to switch between planes of buttons and/or
valuators by pressing a "shift" button.
Copyright (c) 2010-2013 Oliver Kreylos

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

#ifndef VRUI_SHIFTBUTTONTOOL_INCLUDED
#define VRUI_SHIFTBUTTONTOOL_INCLUDED

#include <Vrui/TransformTool.h>

namespace Vrui {

class ShiftButtonTool;

class ShiftButtonToolFactory:public ToolFactory
	{
	friend class ShiftButtonTool;
	
	/* Elements: */
	private:
	bool toggle; // Flag whether the shift button acts as a toggle
	bool forwardShiftButton; // Flag whether the shift button itself is forwarded to both button/valuator planes
	bool resetFeatures; // Flag whether to reset buttons and valuators when shifting away from their plane
	
	/* Constructors and destructors: */
	public:
	ShiftButtonToolFactory(ToolManager& toolManager);
	virtual ~ShiftButtonToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class ShiftButtonTool:public TransformTool
	{
	friend class ShiftButtonToolFactory;
	
	/* Elements: */
	private:
	static ShiftButtonToolFactory* factory; // Pointer to the factory object for this class
	bool toggle; // Flag whether the shift button acts as a toggle
	bool forwardShiftButton; // Flag whether the shift button itself is forwarded to both button/valuator planes
	bool resetFeatures; // Flag whether to reset buttons and valuators when shifting away from their plane
	bool shifted; // Flag whether the shift button is currently pressed
	
	/* Constructors and destructors: */
	public:
	ShiftButtonTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	virtual ~ShiftButtonTool(void);
	
	/* Methods from Tool: */
	virtual void configure(const Misc::ConfigurationFileSection& configFileSection);
	virtual void storeState(Misc::ConfigurationFileSection& configFileSection) const;
	virtual void initialize(void);
	virtual void deinitialize(void);
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void valuatorCallback(int valuatorSlotIndex,InputDevice::ValuatorCallbackData* cbData);
	
	/* Methods from class DeviceForwarder: */
	virtual InputDeviceFeatureSet getSourceFeatures(const InputDeviceFeature& forwardedFeature);
	virtual InputDeviceFeatureSet getForwardedFeatures(const InputDeviceFeature& sourceFeature);
	};

}

#endif
