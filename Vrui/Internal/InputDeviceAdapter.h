/***********************************************************************
InputDeviceAdapter - Base class to convert from diverse "raw" input
device representations to Vrui's internal input device representation.
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

#ifndef VRUI_INTERNAL_INPUTDEVICEADAPTER_INCLUDED
#define VRUI_INTERNAL_INPUTDEVICEADAPTER_INCLUDED

#include <string>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}
class GLContextData;
namespace Vrui {
class InputDevice;
class InputDeviceFeature;
class InputDeviceManager;
}

namespace Vrui {

class InputDeviceAdapter
	{
	/* Elements: */
	protected:
	InputDeviceManager* inputDeviceManager; // Pointer to input device manager
	int numInputDevices; // Number of Vrui input devices mapped to
	InputDevice** inputDevices; // Array of pointers to Vrui input devices owned by this adapter
	
	/* Protected methods: */
	virtual void createInputDevice(int deviceIndex,const Misc::ConfigurationFileSection& configFileSection); // Creates input device by reading current configuration file section
	void initializeAdapter(const Misc::ConfigurationFileSection& configFileSection); // Initializes adapter by reading configuration file section
	
	/* Constructors and destructors: */
	public:
	InputDeviceAdapter(InputDeviceManager* sInputDeviceManager) // Dummy constructor, creates uninitialized input device adapter
		:inputDeviceManager(sInputDeviceManager),
		 numInputDevices(0),inputDevices(0)
		{
		}
	virtual ~InputDeviceAdapter(void);
	
	/* Methods: */
	static std::string getDefaultFeatureName(const InputDeviceFeature& feature); // Returns a default name for the given feature
	static int getDefaultFeatureIndex(InputDevice* device,const char* featureName); // Parses a default feature name into a feature index for the given input device
	int getNumInputDevices(void) const // Returns number of input devices
		{
		return numInputDevices;
		}
	InputDevice* getInputDevice(int deviceIndex) const // Returns pointer to the given input device owned by this adapter
		{
		return inputDevices[deviceIndex];
		}
	virtual std::string getFeatureName(const InputDeviceFeature& feature) const; // Returns the name of a button or valuator on the given input device, which is owned by this adapter
	virtual int getFeatureIndex(InputDevice* device,const char* featureName) const; // Returns the index of a feature of the given name on the given input device, or -1 if feature does not exist
	virtual void updateInputDevices(void) =0; // Updates state of all Vrui input devices owned by this adapter
	virtual void glRenderAction(GLContextData& contextData) const; // Hook to allow an input device adapter to render something
	};

}

#endif
