/***********************************************************************
Tool - Abstract base class for user interaction tools (navigation, menu
selection, selection, etc.).
Copyright (c) 2004-2013 Oliver Kreylos

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

#ifndef VRUI_TOOL_INCLUDED
#define VRUI_TOOL_INCLUDED

#include <string>
#include <Plugins/Factory.h>
#include <Geometry/Ray.h>
#include <Vrui/InputDevice.h>
#include <Vrui/ToolInputLayout.h>
#include <Vrui/ToolInputAssignment.h>

/* Forward declarations: */
namespace Misc {
class CallbackData;
class ConfigurationFileSection;
}
class GLContextData;
namespace Vrui {
class ToolManager;
}

namespace Vrui {

class Tool;

class ToolFactory:public Plugins::Factory
	{
	/* Elements: */
	protected:
	ToolInputLayout layout; // Input requirements of all tools created by this factory
	
	/* Constructors and destructors: */
	public:
	ToolFactory(const char* sClassName,ToolManager& toolManager); // Initializes tool factory settings
	
	/* Methods: */
	virtual const char* getName(void) const =0; // Returns a descriptive -- and unique -- name for tools created by this factory
	const ToolInputLayout& getLayout(void) const // Returns the input requirements of all tools created by this factory
		{
		return layout;
		}
	virtual const char* getButtonFunction(int buttonSlotIndex) const; // Returns a descriptive name for the function associated with the given button slot; buttonSlotIndex==layout.numButtons returns generic name for optional buttons
	virtual const char* getValuatorFunction(int valuatorSlotIndex) const; // Returns a descriptive name for the function associated with the given valuator slot; valuatorSlotIndex==layout.numValuators returns generic name for optional valuators
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const; // Creates a tool of the class represented by this factory and assigns it to the given input device(s)
	virtual void destroyTool(Tool* tool) const; // Destroys a tool of the class represented by this factory
	};

class Tool
	{
	/* Elements: */
	protected:
	const ToolInputLayout& layout; // Layout of the tool's input
	ToolInputAssignment input; // Assignment of input device buttons and valuators to this tool
	
	/* Private methods: */
	private:
	static void buttonCallbackWrapper(Misc::CallbackData* cbData,void* userData);
	static void valuatorCallbackWrapper(Misc::CallbackData* cbData,void* userData);
	
	/* Protected helper methods: */
	protected:
	const InputDevice* getButtonDevice(int buttonSlotIndex) const // Returns the input device associated with the given button slot
		{
		return input.getButtonSlot(buttonSlotIndex).device;
		}
	InputDevice* getButtonDevice(int buttonSlotIndex) // Ditto
		{
		return input.getButtonSlot(buttonSlotIndex).device;
		}
	const ONTransform& getButtonDeviceTransformation(int buttonSlotIndex) const // Returns the position and orientation of the input device associated with the given button slot in physical coordinates
		{
		return input.getButtonSlot(buttonSlotIndex).device->getTransformation();
		}
	Point getButtonDevicePosition(int buttonSlotIndex) const // Returns the position of the input device associated with the given button slot in physical coordinates
		{
		return input.getButtonSlot(buttonSlotIndex).device->getPosition();
		}
	Vector getButtonDeviceRayDirection(int buttonSlotIndex) const // Returns the default ray direction of the input device associated with the given button slot in physical coordinates
		{
		return input.getButtonSlot(buttonSlotIndex).device->getRayDirection();
		}
	Ray getButtonDeviceRay(int buttonSlotIndex) const // Returns the default ray of the input device associated with the given button slot in physical coordinates
		{
		const InputDevice* device=input.getButtonSlot(buttonSlotIndex).device;
		Vector rayDir=device->getRayDirection();
		return Ray(device->getPosition()+rayDir*device->getDeviceRayStart(),rayDir);
		}
	bool getButtonState(int buttonSlotIndex) const // Returns the state of the input device button associated with the given button slot
		{
		const ToolInputAssignment::Slot& slot=input.getButtonSlot(buttonSlotIndex);
		return slot.device->getButtonState(slot.index);
		}
	const InputDevice* getValuatorDevice(int valuatorSlotIndex) const // Returns the input device associated with the given valuator slot
		{
		return input.getValuatorSlot(valuatorSlotIndex).device;
		}
	InputDevice* getValuatorDevice(int valuatorSlotIndex) // Ditto
		{
		return input.getValuatorSlot(valuatorSlotIndex).device;
		}
	const ONTransform& getValuatorDeviceTransformation(int valuatorSlotIndex) const // Returns the position and orientation of the input device associated with the given valuator slot in physical coordinates
		{
		return input.getValuatorSlot(valuatorSlotIndex).device->getTransformation();
		}
	Point getValuatorDevicePosition(int valuatorSlotIndex) const // Returns the position of the input device associated with the given valuator slot in physical coordinates
		{
		return input.getValuatorSlot(valuatorSlotIndex).device->getPosition();
		}
	Vector getValuatorDeviceRayDirection(int valuatorSlotIndex) const // Returns the default ray direction of the input device associated with the given valuator slot in physical coordinates
		{
		return input.getValuatorSlot(valuatorSlotIndex).device->getRayDirection();
		}
	Ray getValuatorDeviceRay(int valuatorSlotIndex) const // Returns the default ray of the input device associated with the given valuator slot in physical coordinates
		{
		const InputDevice* device=input.getValuatorSlot(valuatorSlotIndex).device;
		Vector rayDir=device->getRayDirection();
		return Ray(device->getPosition()+rayDir*device->getDeviceRayStart(),rayDir);
		}
	double getValuatorState(int valuatorSlotIndex) const // Returns the value of the input device valuator associated with the given valuator slot
		{
		const ToolInputAssignment::Slot& slot=input.getValuatorSlot(valuatorSlotIndex);
		return slot.device->getValuator(slot.index);
		}
	
	/* Constructors and destructors: */
	public:
	Tool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment); // Initializes tool with layout defined by given factory and given input assignment
	virtual ~Tool(void);
	
	/* Methods: */
	virtual void configure(const Misc::ConfigurationFileSection& configFileSection); // Optionally called right after constructor
	private:
	virtual void configure(Misc::ConfigurationFileSection& configFileSection); // Previous method now forbidden
	public:
	virtual void storeState(Misc::ConfigurationFileSection& configFileSection) const; // Allows the tool to store its current state in the given configuration file section
	virtual void initialize(void); // Called right after a tool has been created and is fully installed
	virtual void deinitialize(void); // Called right before a tool is destroyed during runtime
	const ToolInputLayout& getLayout(void) const // Returns tool's input layout
		{
		return layout;
		}
	const ToolInputAssignment& getInputAssignment(void) const // Returns tool's input assignment
		{
		return input;
		}
	virtual const ToolFactory* getFactory(void) const; // Returns pointer to factory that created this tool
	virtual std::string getName(void) const; // Returns a descriptive name for the tool
	void assignButtonSlot(int buttonSlotIndex,InputDevice* newSlotDevice,int newSlotButtonIndex); // Re-assigns a button slot
	void assignValuatorSlot(int valuatorSlotIndex,InputDevice* newSlotDevice,int newSlotValuatorIndex); // Re-assigns a valuator slot
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData); // Method called when the state of a button changes
	virtual void valuatorCallback(int valuatorSlotIndex,InputDevice::ValuatorCallbackData* cbData); // Method called when value of a valuator changes
	virtual void frame(void); // Method called exactly once every frame
	virtual void display(GLContextData& contextData) const; // Method for rendering the tool's current state into the current OpenGL context
	};

}

#endif
