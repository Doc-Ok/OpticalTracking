/***********************************************************************
Viewer - Class for viewers/observers in VR environments.
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

#ifndef VRUI_VIEWER_INCLUDED
#define VRUI_VIEWER_INCLUDED

#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthonormalTransformation.h>
#include <GL/gl.h>
#include <GL/GLLight.h>
#include <Vrui/Geometry.h>
#include <Vrui/InputDevice.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}
namespace Vrui {
class Lightsource;
}

namespace Vrui {

class Viewer
	{
	/* Embedded classes: */
	public:
	enum Eye // Enumerated type for projection eye selection
		{
		MONO,LEFT,RIGHT
		};
	
	/* Elements: */
	private:
	char* viewerName; // Viewer name
	bool headTracked; // Flag if the viewer is head-tracked
	const InputDevice* headDevice; // Pointer to input device used for head tracking
	TrackerState headDeviceTransformation; // Fixed head coordinate frame if head tracking is disabled
	Vector deviceViewDirection; // Viewing direction in head device coordinates
	Point deviceMonoEyePosition; // Eye position for monoscopic viewing in head device coordinates
	Point deviceLeftEyePosition; // Left eye position in head device coordinates
	Point deviceRightEyePosition; // Right eye position in head device coordinates
	Lightsource* lightsource; // Pointer to the viewer's head light source
	Point headLightDevicePosition; // Position of head light source in head device coordinates
	Vector headLightDeviceDirection; // Direction of head light source in head device coordinates
	
	/* Transient state data: */
	
	/* Constructors and destructors: */
	public:
	Viewer(void); // Constructs uninitialized viewer
	~Viewer(void);
	
	/* Methods: */
	void initialize(const Misc::ConfigurationFileSection& configFileSection); // Initializes viewer by reading current configuration file section
	void attachToDevice(const InputDevice* newHeadDevice); // Attaches the viewer to a head device
	void detachFromDevice(const TrackerState& newHeadDeviceTransformation); // Turns viewer into a static viewer
	void setEyes(const Vector& newViewDirection,const Point& newMonoEyePosition,const Vector& newEyeOffset); // Sets view direction and eye positions in head device coordinates
	const char* getName(void) const // Returns viewer's name
		{
		return viewerName;
		}
	const Lightsource& getHeadlight(void) const // Returns the viewer's headlight
		{
		return *lightsource;
		}
	void setHeadlightState(bool newHeadlightState); // Enables or disables the viewer's headlight
	void update(void); // Updates viewer state in frame callback
	const TrackerState& getHeadTransformation(void) const // Returns head transformation
		{
		return headTracked?headDevice->getTransformation():headDeviceTransformation;
		}
	Point getHeadPosition(void) const // Returns head position in physical coordinates
		{
		return getHeadTransformation().transform(deviceMonoEyePosition);
		}
	Vector getViewDirection(void) const // Returns view direction in physical coordinates
		{
		return getHeadTransformation().transform(deviceViewDirection);
		}
	const Point& getDeviceEyePosition(Eye eye) const // Returns eye position in head device coordinates
		{
		switch(eye)
			{
			case MONO:
				return deviceMonoEyePosition;
			
			case LEFT:
				return deviceLeftEyePosition;
			
			case RIGHT:
				return deviceRightEyePosition;
			}
		
		/* Dummy statement to make the compiler happy: */
		return deviceMonoEyePosition;
		}
	Point getEyePosition(Eye eye) const // Returns eye position in physical coordinates
		{
		return getHeadTransformation().transform(getDeviceEyePosition(eye));
		}
	};

}

#endif
