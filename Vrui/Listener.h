/***********************************************************************
Listener - Class for listeners/ sound observers in VR environments.
Copyright (c) 2008-2013 Oliver Kreylos

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

#ifndef VRUI_LISTENER_INCLUDED
#define VRUI_LISTENER_INCLUDED

#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Vrui/Geometry.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}
namespace Vrui {
class InputDevice;
}

namespace Vrui {

class Listener
	{
	/* Elements: */
	private:
	char* listenerName; // Listener name
	bool headTracked; // Flag if the listener is head-tracked
	InputDevice* headDevice; // Pointer to input device used for head tracking
	Point deviceHeadPosition; // Listening position in head device coordinates
	Vector deviceListenDirection; // Listening direction in head device coordinates
	Vector deviceUpDirection; // Up direction in head device coordinates
	Scalar gain; // Overall gain factor for this listener in all contexts
	
	/* Transient state data: */
	TrackerState headDeviceTransformation; // Head coordinate frame
	
	/* Constructors and destructors: */
	public:
	Listener(void); // Constructs uninitialized listener
	~Listener(void);
	
	/* Methods: */
	void initialize(const Misc::ConfigurationFileSection& configFileSection); // Initializes listener by reading current configuration file section
	const char* getName(void) const // Returns listener's name
		{
		return listenerName;
		}
	void update(void); // Updates listener state in frame callback
	const TrackerState& getHeadTransformation(void) const // Returns head transformation
		{
		return headDeviceTransformation;
		}
	Point getHeadPosition(void) const // Returns head position in physical coordinates
		{
		return headDeviceTransformation.transform(deviceHeadPosition);
		}
	Vector getListenDirection(void) const // Returns listening direction in physical coordinates
		{
		return headDeviceTransformation.transform(deviceListenDirection);
		}
	Vector getUpDirection(void) const // Returns up direction in physical coordinates
		{
		return headDeviceTransformation.transform(deviceUpDirection);
		}
	Scalar getGain(void) const // Returns the listener's gain factor
		{
		return gain;
		}
	};

}

#endif
