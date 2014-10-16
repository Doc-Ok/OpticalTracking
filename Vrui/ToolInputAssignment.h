/***********************************************************************
ToolInputAssignment - Class defining the input assignments of a tool.
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

#ifndef VRUI_TOOLINPUTASSIGNMENT_INCLUDED
#define VRUI_TOOLINPUTASSIGNMENT_INCLUDED

/* Forward declarations: */
namespace Vrui {
class InputDevice;
class InputDeviceFeature;
class ToolInputLayout;
}

namespace Vrui {

class ToolInputAssignment
	{
	/* Embedded classes: */
	public:
	struct Slot // Structure for button or valuator assignment slots
		{
		/* Elements: */
		public:
		InputDevice* device; // Pointer to input device containing slot
		int index; // Index of slot's button or valuator on input device
		
		/* Constructors and destructors: */
		Slot(void) // Creates unassigned slot
			:device(0),index(-1)
			{
			}
		Slot(InputDevice* sDevice,int sIndex) // Elementwise constructor
			:device(sDevice),index(sIndex)
			{
			}
		};
	
	/* Elements: */
	private:
	int numButtonSlots; // Current number of button slots in the assignment
	Slot* buttonSlots; // Array of button slots
	int numValuatorSlots; // Current number of valuator slots in the assignment
	Slot* valuatorSlots; // Array of valuator slots
	
	/* Constructors and destructors: */
	public:
	ToolInputAssignment(const ToolInputLayout& layout); // Creates "empty" assignment for given layout
	ToolInputAssignment(const ToolInputAssignment& source); // Copies assignment from source
	private:
	ToolInputAssignment& operator=(const ToolInputAssignment& source); // Disallow assignment operator
	public:
	~ToolInputAssignment(void); // Destroys assignment
	
	/* Methods: */
	void setButtonSlot(int buttonSlotIndex,InputDevice* slotDevice,int slotButtonIndex); // Sets the button assignment of the given index
	void addButtonSlot(InputDevice* slotDevice,int slotButtonIndex); // Adds an optional button slot to the end of the input assignment
	void setValuatorSlot(int valuatorSlotIndex,InputDevice* slotDevice,int slotValuatorIndex); // Sets the valuator assignment of the given index
	void addValuatorSlot(InputDevice* slotDevice,int slotValuatorIndex); // Adds an optional valuator slot to the end of the input assignment
	int getNumButtonSlots(void) const // Returns the current total number of required and optional button slots in the assignment
		{
		return numButtonSlots;
		}
	const Slot& getButtonSlot(int buttonSlotIndex) const // Returns the button slot of the given index
		{
		return buttonSlots[buttonSlotIndex];
		}
	InputDeviceFeature getButtonSlotFeature(int buttonSlotIndex) const; // Returns the input device feature in the given button assignment slot
	int getNumValuatorSlots(void) const // Returns the current total number of required and optional valuator slots in the assignment
		{
		return numValuatorSlots;
		}
	const Slot& getValuatorSlot(int valuatorSlotIndex) const // Returns the valuator slot of the given index
		{
		return valuatorSlots[valuatorSlotIndex];
		}
	InputDeviceFeature getValuatorSlotFeature(int valuatorSlotIndex) const; // Returns the input device feature in the given valuator assignment slot
	int getNumSlots(void) const // Returns number of button and valuator slots
		{
		return numButtonSlots+numValuatorSlots;
		}
	bool isSlotButton(int slotIndex) const // Returns true if the given assignment slot is a button slot
		{
		return slotIndex<numButtonSlots;
		}
	bool isSlotValuator(int slotIndex) const // Returns true if the given assignment slot is a valuator slot
		{
		return slotIndex>=numButtonSlots;
		}
	InputDevice* getSlotDevice(int slotIndex) const // Returns the input device in the given assignment slot
		{
		if(slotIndex<numButtonSlots)
			return buttonSlots[slotIndex].device;
		else
			return valuatorSlots[slotIndex-numButtonSlots].device;
		}
	int getSlotFeatureIndex(int slotIndex) const; // Returns the index of the feature in the given assignment slot
	InputDeviceFeature getSlotFeature(int slotIndex) const; // Returns the input device feature in the given assignment slot
	bool isAssigned(const InputDeviceFeature& feature) const; // Returns true if the given input device feature is already part of the input assignment
	int findFeature(const InputDeviceFeature& feature) const; // Returns the slot index of the given feature, or -1 if the feature is not part of the assignment
	int getButtonSlotIndex(int slotIndex) const // Returns the button slot index of a button slot
		{
		return slotIndex;
		}
	int getValuatorSlotIndex(int slotIndex) const // Returns the valuator slot index of a valuator slot
		{
		return slotIndex-numButtonSlots;
		}
	};

}

#endif
