/***********************************************************************
InputDeviceAdapterHID - Mac OSX-specific version of HID input device
adapter.
Copyright (c) 2009-2010 Oliver Kreylos

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

#ifndef VRUI_INTERNAL_MACOSX_INPUTDEVICEADAPTERHID_INCLUDED
#define VRUI_INTERNAL_MACOSX_INPUTDEVICEADAPTERHID_INCLUDED

#include <string>
#include <vector>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDLib.h>
#include <Misc/HashTable.h>
#include <Threads/Mutex.h>
#include <Threads/Thread.h>
#include <Math/BrokenLine.h>
#include <MacOSX/AutoRef.h>
#include <Vrui/Internal/InputDeviceAdapter.h>

/* Forward declarations: */
namespace Vrui {
class InputDevice;
}

namespace Vrui {

class InputDeviceAdapterHID:public InputDeviceAdapter
	{
	/* Embedded classes: */
	private:
	struct Device // Structure describing a human interface device
		{
		/* Elements: */
		public:
		std::string name; // Name of device to be created
		long vendorId,productId; // Vendor and product IDs of the HID
		int deviceIndex; // Index of device among all devices with the same vendor/product ID
		int firstButtonIndex; // Index of HID's first button in device state array
		int numButtons; // Number of HID's buttons
		int firstValuatorIndex; // Index of HID's first axis in device state array
		int numValuators; // Number of HID's axes
		Vrui::InputDevice* device; // Pointer to Vrui input device associated with the HID
		std::vector<std::string> buttonNames; // Array of button feature names
		std::vector<std::string> valuatorNames; // Array of valuator feature names
		};
	
	struct ElementKey // Structure to map (device, element cookie) pairs to button or valuator indices
		{
		/* Elements: */
		public:
		void* device; // Pointer to HID device object
		IOHIDElementCookie cookie; // Cookie for the element
		
		/* Constructors and destructors: */
		ElementKey(void* sDevice,IOHIDElementCookie sCookie)
			:device(sDevice),cookie(sCookie)
			{
			}
		
		/* Methods: */
		friend bool operator!=(const ElementKey& key1,const ElementKey& key2)
			{
			return key1.device!=key2.device||key1.cookie!=key2.cookie;
			}
		static size_t hash(const ElementKey& source,size_t tableSize)
			{
			return (reinterpret_cast<size_t>(source.device)+size_t(source.cookie))%tableSize;
			}
		};
	
	struct ElementDescriptor // Structure describing how to convert a HIDValue into a button or a valuator
		{
		/* Embedded classes: */
		public:
		enum ElementType // Enumerated type for element types
			{
			BUTTON,VALUATOR,HATSWITCH
			};
		
		typedef Math::BrokenLine<double> AxisMapper; // Type for axis mappers
		
		/* Elements: */
		public:
		ElementType elementType; // Type of this element; hat switch is a special case generating two valuators
		int index; // Element's index in button or valuator state array
		int hsMin,hsMax; // Min and max values for hat switches
		AxisMapper axisMapper; // Axis mapper for a valuator element
		
		/* Constructors and destructors: */
		ElementDescriptor(void)
			:hsMin(0),hsMax(0),axisMapper(0.0,0.0,0.0,0.0)
			{
			}
		};
	
	typedef Misc::HashTable<ElementKey,ElementDescriptor,ElementKey> ElementMap; // Type for hash tables mapping elements to element descriptors
	
	/* Elements: */
	private:
	std::vector<Device> devices; // List of human interface devices
	Threads::Mutex deviceStateMutex; // Mutex protecting the device state array
	bool* buttonStates; // Button state array
	double* valuatorStates; // Valuator state array
	MacOSX::AutoRef<IOHIDManagerRef> hidManager; // HID manager object
	ElementMap elementMap; // Hash table for elements
	Threads::Thread devicePollingThread; // Thread polling the event files of all HIDs
	
	/* Protected methods from InputDeviceAdapter: */
	protected:
	virtual void createInputDevice(int deviceIndex,const Misc::ConfigurationFileSection& configFileSection);
	
	/* New private methods: */
	private:
	static void hidDeviceValueChangedCallbackWrapper(void* context,IOReturn result,void* device,IOHIDValueRef newValue)
		{
		static_cast<InputDeviceAdapterHID*>(context)->hidDeviceValueChangedCallback(result,device,newValue);
		}
	void hidDeviceValueChangedCallback(IOReturn result,void* device,IOHIDValueRef newValue); // Callback method when an element on a monitored HID changes value
	void* devicePollingThreadMethod(void); // Method polling the event files of all HIDs
	
	/* Constructors and destructors: */
	public:
	InputDeviceAdapterHID(InputDeviceManager* sInputDeviceManager,const Misc::ConfigurationFileSection& configFileSection); // Creates adapter connected to a set of human interface devices
	virtual ~InputDeviceAdapterHID(void);
	
	/* Methods from InputDeviceAdapter: */
	virtual std::string getFeatureName(const InputDeviceFeature& feature) const;
	virtual int getFeatureIndex(InputDevice* device,const char* featureName) const;
	virtual void updateInputDevices(void);
	};

}

#endif
