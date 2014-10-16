/***********************************************************************
TransformTool - Base class for tools used to transform the position or
orientation of input devices.
Copyright (c) 2007-2013 Oliver Kreylos

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

#ifndef VRUI_TRANSFORMTOOL_INCLUDED
#define VRUI_TRANSFORMTOOL_INCLUDED

#include <Vrui/DeviceForwarder.h>
#include <Vrui/Tool.h>

/* Forward declarations: */
namespace Vrui {
class ToolManager;
}

namespace Vrui {

class TransformTool;

class TransformToolFactory:public ToolFactory
	{
	friend class TransformTool;
	
	/* Constructors and destructors: */
	public:
	TransformToolFactory(ToolManager& toolManager);
	virtual ~TransformToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual const char* getValuatorFunction(int valuatorSlotIndex) const;
	};

class TransformTool:public Tool,public DeviceForwarder
	{
	friend class TransformToolFactory;
	
	/* Elements: */
	private:
	static TransformToolFactory* factory; // Pointer to the factory object for this class
	protected:
	InputDevice* sourceDevice; // Pointer to the source device used to control this tool
	InputDevice* transformedDevice; // Pointer to the transformed device controlled by this tool
	int numPrivateButtons; // Number of initial button slots that are not forwarded to the transformed device
	int numPrivateValuators; // Number of initial valuator slots that are not forwarded to the transformed device
	
	/* Protected methods: */
	protected:
	void resetDevice(void); // Resets the transformed device to the source device's position and orientation
	
	/* Constructors and destructors: */
	public:
	TransformTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	virtual ~TransformTool(void);
	
	/* Methods from class Tool: */
	virtual void initialize(void);
	virtual void deinitialize(void);
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void valuatorCallback(int valuatorSlotIndex,InputDevice::ValuatorCallbackData* cbData);
	virtual void frame(void);
	
	/* Methods from class DeviceForwarder: */
	virtual std::vector<InputDevice*> getForwardedDevices(void);
	virtual InputDeviceFeatureSet getSourceFeatures(const InputDeviceFeature& forwardedFeature);
	virtual InputDevice* getSourceDevice(const InputDevice* forwardedDevice);
	virtual InputDeviceFeatureSet getForwardedFeatures(const InputDeviceFeature& sourceFeature);
	};

}

#endif
