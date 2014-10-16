/***********************************************************************
ButtonInputDeviceTool - Class for tools using buttons (such as keyboard
keys) to interact with virtual input devices.
Copyright (c) 2007-2010 Oliver Kreylos

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

#ifndef VRUI_BUTTONINPUTDEVICETOOL_INCLUDED
#define VRUI_BUTTONINPUTDEVICETOOL_INCLUDED

#include <Geometry/Vector.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Vrui/InputDeviceTool.h>

namespace Vrui {

class ButtonInputDeviceTool;

class ButtonInputDeviceToolFactory:public ToolFactory
	{
	friend class ButtonInputDeviceTool;
	
	/* Elements: */
	private:
	Vector translations[6]; // Translation vectors associated with the six navigational buttons
	Vector rotations[6]; // Scaled rotation axes associated with the six navigational buttons
	
	/* Constructors and destructors: */
	public:
	ButtonInputDeviceToolFactory(ToolManager& toolManager);
	virtual ~ButtonInputDeviceToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class ButtonInputDeviceTool:public InputDeviceTool
	{
	friend class ButtonInputDeviceToolFactory;
	
	/* Embedded classes: */
	enum TransformationMode // Enumerated type for transformation modes
		{
		TRANSLATING,ROTATING
		};
	
	/* Elements: */
	private:
	static ButtonInputDeviceToolFactory* factory; // Pointer to the factory object for this class
	
	/* Transient dragging state: */
	TransformationMode transformationMode; // Current transformation mode
	bool navButtonStates[6]; // Current status of navigation buttons
	int numPressedNavButtons; // Number of currently pressed navigation buttons
	double lastFrameTime; // Time of last frame
	
	/* Constructors and destructors: */
	public:
	ButtonInputDeviceTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	};

}

#endif
