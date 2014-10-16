/***********************************************************************
PlaneSnapInputDeviceTool - Class for tools that snap a virtual input
device to a plane defined by three points.
Copyright (c) 2009-2013 Oliver Kreylos

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

#ifndef VRUI_PLANESNAPINPUTDEVICETOOL_INCLUDED
#define VRUI_PLANESNAPINPUTDEVICETOOL_INCLUDED

#include <Geometry/Point.h>
#include <Vrui/InputDeviceTool.h>

/* Forward declarations: */
namespace Vrui {
class Viewer;
}

namespace Vrui {

class PlaneSnapInputDeviceTool;

class PlaneSnapInputDeviceToolFactory:public ToolFactory
	{
	friend class PlaneSnapInputDeviceTool;
	
	/* Elements: */
	private:
	Scalar markerSize; // Size of selected point markers in physical units
	
	/* Constructors and destructors: */
	public:
	PlaneSnapInputDeviceToolFactory(ToolManager& toolManager);
	virtual ~PlaneSnapInputDeviceToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class PlaneSnapInputDeviceTool:public InputDeviceTool
	{
	friend class PlaneSnapInputDeviceToolFactory;
	
	/* Elements: */
	private:
	static PlaneSnapInputDeviceToolFactory* factory; // Pointer to the factory object for this class
	
	/* Transient state: */
	int numSelectedPoints; // Number of currently selected points
	Point selectedPoints[3]; // Array of up to three selected points in navigational coordinates
	bool draggingPoint; // Flag if the tool is currently dragging another selection point
	
	/* Constructors and destructors: */
	public:
	PlaneSnapInputDeviceTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	};

}

#endif
