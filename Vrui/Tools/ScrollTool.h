/***********************************************************************
ScrollTool - Class for tools that can scroll inside certain GLMotif GUI
widgets. ScrollTool objects are cascadable and prevent valuator events
if they would fall into the area of interest of scrollable widgets.
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

#ifndef VRUI_SCROLLTOOL_INCLUDED
#define VRUI_SCROLLTOOL_INCLUDED

#include <Vrui/GUIInteractor.h>
#include <Vrui/DeviceForwarder.h>
#include <Vrui/UserInterfaceTool.h>

/* Forward declarations: */
namespace Vrui {
class ToolManager;
}

namespace Vrui {

class ScrollTool;

class ScrollToolFactory:public ToolFactory
	{
	friend class ScrollTool;
	
	/* Constructors and destructors: */
	public:
	ScrollToolFactory(ToolManager& toolManager);
	virtual ~ScrollToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getValuatorFunction(int valuatorSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class ScrollTool:public UserInterfaceTool,public GUIInteractor,public DeviceForwarder
	{
	friend class ScrollToolFactory;
	
	/* Elements: */
	private:
	static ScrollToolFactory* factory; // Pointer to the factory object for this class
	InputDevice* valuatorDevice; // Pointer to the input device representing the forwarded valuator
	bool sendingEvents; // Flag if the tool is currently sending text control events
	
	/* Constructors and destructors: */
	public:
	ScrollTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual void initialize(void);
	virtual void deinitialize(void);
	virtual const ToolFactory* getFactory(void) const;
	virtual void valuatorCallback(int valuatorSlotIndex,InputDevice::ValuatorCallbackData* cbData);
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	
	/* Methods from DeviceForwarder: */
	virtual std::vector<InputDevice*> getForwardedDevices(void);
	virtual InputDeviceFeatureSet getSourceFeatures(const InputDeviceFeature& forwardedFeature);
	virtual InputDevice* getSourceDevice(const InputDevice* forwardedDevice);
	virtual InputDeviceFeatureSet getForwardedFeatures(const InputDeviceFeature& sourceFeature);
	};

}

#endif
