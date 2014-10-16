/***********************************************************************
DeviceForwarderCreator - Helper class to create virtual input devices
and maintain associations from input slots to forwarded devices for
device-forwarding tools.
Copyright (c) 2014 Oliver Kreylos

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

#ifndef VRUI_DEVICEFORWARDERCREATOR_INCLUDED
#define VRUI_DEVICEFORWARDERCREATOR_INCLUDED

#include <Misc/HashTable.h>

/* Forward declarations: */
namespace Vrui {
class InputDevice;
}

namespace Vrui {

class DeviceForwarderCreator
	{
	/* Embedded classes: */
	public:
	struct ForwardingSlot // Structure associating a button or valuator slot with a forwarded device
		{
		/* Elements: */
		public:
		InputDevice* sourceDevice; // Pointer to source device
		int sourceDeviceFeatureIndex; // Index of forwarding button or valuator on source device
		InputDevice* virtualDevice; // Pointer to virtual device
		int virtualDeviceFeatureIndex; // Index of forwarded button or valuator on forwarded device
		};
	
	private:
	struct VirtualDeviceLayout // Structure describing the layout of a virtual input device
		{
		/* Elements: */
		public:
		InputDevice* device; // Pointer to virtual input device
		int numButtons; // Number of required buttons on virtual device
		int numValuators; // Number of required valuators on virtual device
		};
	
	typedef Misc::HashTable<InputDevice*,VirtualDeviceLayout> DeviceLayoutMap;
	
	/* Elements: */
	int numButtonSlots; // Total number of forwarded button slots
	ForwardingSlot* buttonSlots; // Array of button slot associations
	int numValuatorSlots; // Total number of forwarded valuator slots
	ForwardingSlot* valuatorSlots; // Array of valuator slot associations
	DeviceLayoutMap deviceLayoutMap; // Map from source input devices to layout of their forwarding virtual input devices
	
	/* Constructors and destructors: */
	public:
	DeviceForwarderCreator(int sNumButtonSlots,int sNumValuatorSlots); // Initializes creator for given number of forwarded buttons and valuators
	~DeviceForwarderCreator(void);
	
	/* Methods: */
	void forwardButton(int buttonSlotIndex,InputDevice* sourceDevice,int sourceDeviceButtonIndex); // Creates a forwarding device for the given button slot
	void forwardValuator(int valuatorSlotIndex,InputDevice* sourceDevice,int sourceDeviceValuatorIndex); // Creates a forwarding device for the given valuator slot
	void createDevices(void); // Creates all required virtual input devices after all slots have been forwarded
	size_t getNumDevices(void) const // Returns the number of distinct source devices, i.e., the number of created virtual input devices
		{
		return deviceLayoutMap.getNumEntries();
		}
	InputDevice* getSourceDevice(size_t index) const; // Returns the forwarded source device of the given index
	InputDevice* collectDevice(InputDevice* sourceDevice); // Returns one of the created virtual input devices and releases it from the creator
	const ForwardingSlot* getButtonSlots(void) const // Returns the array of button slot associations
		{
		return buttonSlots;
		}
	const ForwardingSlot* getValuatorSlots(void) const // Returns the array of valuator slot associations
		{
		return valuatorSlots;
		}
	};

}

#endif
