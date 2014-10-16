/***********************************************************************
MultipipeDispatcher - Class to distribute input device and ancillary
data between the nodes in a multipipe VR environment.
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

#ifndef VRUI_INTERNAL_MULTIPIPEDISPATCHER_INCLUDED
#define VRUI_INTERNAL_MULTIPIPEDISPATCHER_INCLUDED

#include <string>
#include <vector>
#include <Geometry/Vector.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Vrui/Geometry.h>
#include <Vrui/Internal/InputDeviceAdapter.h>

/* Forward declarations: */
namespace Cluster {
class MulticastPipe;
}
namespace Vrui {
class InputDevice;
}

namespace Vrui {

class MultipipeDispatcher:public InputDeviceAdapter
	{
	/* Embedded classes: */
	private:
	struct InputDeviceTrackingState // Structure for current input device tracking states
		{
		/* Elements: */
		public:
		Vector deviceRayDirection;
		Scalar deviceRayStart;
		TrackerState transformation;
		Vector linearVelocity;
		Vector angularVelocity;
		};
	
	/* Elements: */
	private:
	Cluster::MulticastPipe* pipe; // Multicast pipe connecting the master node to all slave nodes
	int totalNumButtons; // Total number of buttons on all dispatched input devices
	int totalNumValuators; // Total number of valuators on all dispatched input devices
	
	/* Slave state: */
	std::vector<std::string> buttonNames; // Array of button names for all dispatched input devices
	std::vector<std::string> valuatorNames; // Array of button names for all dispatched input devices
	
	/* Transient state to marshall input device states over a multicast pipe: */
	InputDeviceTrackingState* trackingStates; // Array of input device tracking states
	bool* buttonStates; // Array of input device button states
	double* valuatorStates; // Array of input device valuator states
	
	/* Constructors and destructors: */
	public:
	MultipipeDispatcher(InputDeviceManager* sInputDeviceManager,Cluster::MulticastPipe* sPipe);
	virtual ~MultipipeDispatcher(void);
	
	/* Methods from InputDeviceAdapter: */
	virtual std::string getFeatureName(const InputDeviceFeature& feature) const;
	virtual int getFeatureIndex(InputDevice* device,const char* featureName) const;
	virtual void updateInputDevices(void);
	};

}

#endif
