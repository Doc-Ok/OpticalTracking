/***********************************************************************
ButtonToValuatorTool - Class to convert a single button or two buttons
into a two- or three-state valuator, respectively.
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

#ifndef VRUI_BUTTONTOVALUATORTOOL_INCLUDED
#define VRUI_BUTTONTOVALUATORTOOL_INCLUDED

#include <Vrui/TransformTool.h>

namespace Vrui {

class ButtonToValuatorTool;

class ButtonToValuatorToolFactory:public ToolFactory
	{
	friend class ButtonToValuatorTool;
	
	/* Constructors and destructors: */
	public:
	ButtonToValuatorToolFactory(ToolManager& toolManager);
	virtual ~ButtonToValuatorToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class ButtonToValuatorTool:public TransformTool
	{
	friend class ButtonToValuatorToolFactory;
	
	/* Elements: */
	private:
	static ButtonToValuatorToolFactory* factory; // Pointer to the factory object for this class
	
	/* Constructors and destructors: */
	public:
	ButtonToValuatorTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	virtual ~ButtonToValuatorTool(void);
	
	/* Methods from Tool: */
	virtual void initialize(void);
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	
	/* Methods from class DeviceForwarder: */
	virtual InputDeviceFeatureSet getSourceFeatures(const InputDeviceFeature& forwardedFeature);
	virtual InputDeviceFeatureSet getForwardedFeatures(const InputDeviceFeature& sourceFeature);
	};

}

#endif
