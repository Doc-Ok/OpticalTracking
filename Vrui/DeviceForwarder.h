/***********************************************************************
DeviceForwarder - Abstract mixin base class for tools that permanently
forward input device buttons or valuators to virtual input devices.
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

#ifndef VRUI_DEVICEFORWARDER_INCLUDED
#define VRUI_DEVICEFORWARDER_INCLUDED

#include <vector>
#include <Vrui/InputDevice.h>
#include <Vrui/InputDeviceFeature.h>

/* Forward declarations: */
namespace Vrui {
class ToolInputAssignment;
}

namespace Vrui {

class DeviceForwarder
	{
	/* Constructors and destructors: */
	public:
	virtual ~DeviceForwarder(void);
	
	/* Methods: */
	virtual std::vector<InputDevice*> getForwardedDevices(void) =0; // Returns the list of all virtual input devices controlled by the device forwarder
	virtual InputDeviceFeatureSet getSourceFeatures(const InputDeviceFeature& forwardedFeature) =0; // Returns the set of source features routed to the given forwarded feature
	virtual InputDevice* getSourceDevice(const InputDevice* forwardedDevice) =0; // Returns the input device from which the given forwarded device's position and orientation are derived
	virtual InputDeviceFeatureSet getForwardedFeatures(const InputDeviceFeature& sourceFeature) =0; // Returns the set of forwarded features routed from the given source feature
	};

}

#endif
