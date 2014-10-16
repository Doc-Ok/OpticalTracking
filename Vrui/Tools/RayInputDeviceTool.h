/***********************************************************************
RayInputDeviceTool - Class for tools using a ray to interact with
virtual input devices.
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

#ifndef VRUI_RAYINPUTDEVICETOOL_INCLUDED
#define VRUI_RAYINPUTDEVICETOOL_INCLUDED

#include <Geometry/Ray.h>
#include <Vrui/Internal/BoxRayDragger.h>
#include <Vrui/InputDeviceTool.h>

namespace Vrui {

class RayInputDeviceTool;

class RayInputDeviceToolFactory:public ToolFactory
	{
	friend class RayInputDeviceTool;
	
	/* Elements: */
	private:
	Scalar rotateFactor; // Distance the input device has to be moved to rotate the input device by one radians
	
	/* Constructors and destructors: */
	public:
	RayInputDeviceToolFactory(ToolManager& toolManager);
	virtual ~RayInputDeviceToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class RayInputDeviceTool:public InputDeviceTool
	{
	friend class RayInputDeviceToolFactory;
	
	/* Elements: */
	private:
	static RayInputDeviceToolFactory* factory; // Pointer to the factory object for this class
	BoxRayDragger dragger; // A box dragger to calculate dragging transformations
	
	/* Transient state: */
	Ray interactionRay; // Current interaction ray
	
	/* Constructors and destructors: */
	public:
	RayInputDeviceTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	};

}

#endif
