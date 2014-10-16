/***********************************************************************
InputDeviceFeature - Helper class to identify buttons or valuators on
input devices.
Copyright (c) 2010 Oliver Kreylos

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

#ifndef VRUI_INPUTDEVICEFEATURE_INCLUDED
#define VRUI_INPUTDEVICEFEATURE_INCLUDED

#include <vector>
#include <Vrui/InputDevice.h>

namespace Vrui {

class InputDeviceFeature
	{
	/* Elements: */
	private:
	InputDevice* device; // Pointer to the device owning the feature
	int featureIndex; // Feature's index

	/* Constructors and destructors: */
	public:
	InputDeviceFeature(void)
		:device(0),featureIndex(-1)
		{
		}
	InputDeviceFeature(InputDevice* sDevice,int sFeatureIndex)
		:device(sDevice),featureIndex(sFeatureIndex)
		{
		}
	InputDeviceFeature(InputDevice* sDevice,InputDevice::FeatureType sFeatureType,int sIndex)
		:device(sDevice),featureIndex(device->getFeatureIndex(sFeatureType,sIndex))
		{
		}
	
	/* Methods: */
	bool isValid(void) const // Returns true if the feature is valid
		{
		return device!=0&&featureIndex>=0;
		}
	bool operator==(const InputDeviceFeature& other) const // Equality operator
		{
		return featureIndex==other.featureIndex&&device==other.device;
		}
	bool operator!=(const InputDeviceFeature& other) const // Inequality operator
		{
		return featureIndex!=other.featureIndex||device!=other.device;
		}
	InputDevice* getDevice(void) const // Returns the input device
		{
		return device;
		}
	int getFeatureIndex(void) const // Returns the feature's index on the input device
		{
		return featureIndex;
		}
	InputDevice::FeatureType getType(void) const // Returns the feature's type
		{
		return device->getFeatureType(featureIndex);
		}
	bool isButton(void) const // Returns true if the feature is a button
		{
		return device->isFeatureButton(featureIndex);
		}
	bool isValuator(void) const // Returns true if the feature is a valuator
		{
		return device->isFeatureValuator(featureIndex);
		}
	int getIndex(void) const // Returns the button or valuator index of a button or valuator feature, respectively
		{
		return device->getFeatureTypeIndex(featureIndex);
		}
	Misc::CallbackList& getCallbacks(void) const // Returns the feature's callback list
		{
		return device->getFeatureCallbacks(featureIndex);
		}
	};

typedef std::vector<InputDeviceFeature> InputDeviceFeatureSet; // Type for lists of input device features in no particular order

}

#endif
