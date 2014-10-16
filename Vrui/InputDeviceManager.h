/***********************************************************************
InputDeviceManager - Class to manage physical and virtual input devices,
tools associated to input devices, and the input device update graph.
Copyright (c) 2004-2014 Oliver Kreylos

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

#ifndef VRUI_INPUTDEVICEMANAGER_INCLUDED
#define VRUI_INPUTDEVICEMANAGER_INCLUDED

#include <string>
#include <list>
#include <vector>
#include <Misc/CallbackData.h>
#include <Misc/CallbackList.h>
#include <Vrui/InputDevice.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}
class GLContextData;
namespace Vrui {
class InputDeviceFeature;
class InputGraphManager;
class TextEventDispatcher;
class InputDeviceAdapter;
}

namespace Vrui {

class InputDeviceManager
	{
	/* Embedded classes: */
	public:
	class CallbackData:public Misc::CallbackData // Base class for input device manager callbacks
		{
		/* Elements: */
		public:
		InputDeviceManager* manager; // Pointer to the input device manager that initiated the callback
		
		/* Constructors and destructors: */
		CallbackData(InputDeviceManager* sManager)
			:manager(sManager)
			{
			}
		};
	
	class InputDeviceCreationCallbackData:public CallbackData // Callback data sent when an input device is created
		{
		/* Elements: */
		public:
		InputDevice* inputDevice; // Pointer to newly created input device
		
		/* Constructors and destructors: */
		InputDeviceCreationCallbackData(InputDeviceManager* sManager,InputDevice* sInputDevice)
			:CallbackData(sManager),
			 inputDevice(sInputDevice)
			{
			}
		};
	
	class InputDeviceDestructionCallbackData:public CallbackData // Callback data sent when an input device is destroyed
		{
		/* Elements: */
		public:
		InputDevice* inputDevice; // Pointer to input device to be destroyed
		
		/* Constructors and destructors: */
		InputDeviceDestructionCallbackData(InputDeviceManager* sManager,InputDevice* sInputDevice)
			:CallbackData(sManager),
			 inputDevice(sInputDevice)
			{
			}
		};
	
	class InputDeviceUpdateCallbackData:public CallbackData // Callback data sent after the manager updated all physical input devices
		{
		/* Constructors and destructors: */
		public:
		InputDeviceUpdateCallbackData(InputDeviceManager* sManager)
			:CallbackData(sManager)
			{
			}
		};
	
	private:
	typedef std::list<InputDevice> InputDevices;
	
	/* Elements: */
	private:
	InputGraphManager* inputGraphManager; // Pointer to the input graph manager
	TextEventDispatcher* textEventDispatcher; // Pointer to object dispatching GLMotif text and text control events
	int numInputDeviceAdapters; // Number of input device adapters managed by the input device manager
	InputDeviceAdapter** inputDeviceAdapters; // Array of pointers to managed input device adapters
	InputDevices inputDevices; // List of all created input devices
	Misc::CallbackList inputDeviceCreationCallbacks; // List of callbacks to be called after a new input device has been created
	Misc::CallbackList inputDeviceDestructionCallbacks; // List of callbacks to be called before an input device will be destroyed
	Misc::CallbackList inputDeviceUpdateCallbacks; // List of callbacks to be called immediately after the input device manager updated all physical input devices
	
	/* Constructors and destructors: */
	public:
	InputDeviceManager(InputGraphManager* sInputGraphManager,TextEventDispatcher* sTextEventDispatcher);
	private:
	InputDeviceManager(const InputDeviceManager& source); // Prohibit copy constructor
	InputDeviceManager& operator=(const InputDeviceManager& source); // Prohibit assignment operator
	public:
	~InputDeviceManager(void);
	
	/* Methods: */
	void initialize(const Misc::ConfigurationFileSection& configFileSection); // Creates all input device adapters listed in the configuration file section
	void addAdapter(InputDeviceAdapter* newAdapter); // Adds an input device adapter to the input device manager
	int getNumInputDeviceAdapters(void) const // Returns number of input device adapters
		{
		return numInputDeviceAdapters;
		}
	InputDeviceAdapter* getInputDeviceAdapter(int inputDeviceAdapterIndex) // Returns pointer to an input device adapter
		{
		return inputDeviceAdapters[inputDeviceAdapterIndex];
		}
	InputDeviceAdapter* findInputDeviceAdapter(InputDevice* device) const; // Returns pointer to the input device adapter owning the given device (or 0)
	InputGraphManager* getInputGraphManager(void) const
		{
		return inputGraphManager;
		}
	TextEventDispatcher* getTextEventDispatcher(void) const
		{
		return textEventDispatcher;
		}
	InputDevice* createInputDevice(const char* deviceName,int trackType,int numButtons,int numValuators,bool physicalDevice =false);
	int getNumInputDevices(void) const
		{
		return inputDevices.size();
		}
	InputDevice* getInputDevice(int deviceIndex);
	InputDevice* findInputDevice(const char* deviceName);
	void destroyInputDevice(InputDevice* device);
	std::string getFeatureName(const InputDeviceFeature& feature) const; // Returns the name of the given input device feature
	int getFeatureIndex(InputDevice* device,const char* featureName) const; // Returns the index of the feature of the given name on the given input device, or -1 if feature does not exist
	void updateInputDevices(void);
	Misc::CallbackList& getInputDeviceCreationCallbacks(void) // Returns list of input device creation callbacks
		{
		return inputDeviceCreationCallbacks;
		}
	Misc::CallbackList& getInputDeviceDestructionCallbacks(void) // Returns list of input device creation callbacks
		{
		return inputDeviceDestructionCallbacks;
		}
	Misc::CallbackList& getInputDeviceUpdateCallbacks(void) // Returns list of input device update callbacks
		{
		return inputDeviceUpdateCallbacks;
		}
	void glRenderAction(GLContextData& contextData) const; // Renders the input device manager's state
	};

}

#endif
