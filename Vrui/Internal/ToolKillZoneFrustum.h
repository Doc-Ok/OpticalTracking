/***********************************************************************
ToolKillZoneFrustum - Class for "kill zones" for tools and input devices
that use projection onto a plane to detect whether tools are inside the
zone.
Copyright (c) 2007-2014 Oliver Kreylos

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

#ifndef VRUI_INTERNAL_TOOLKILLZONEFRUSTUM_INCLUDED
#define VRUI_INTERNAL_TOOLKILLZONEFRUSTUM_INCLUDED

#include <Geometry/Box.h>
#include <Vrui/Geometry.h>
#include <Vrui/Internal/ToolKillZone.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}
namespace Vrui {
class Viewer;
class VRScreen;
}

namespace Vrui {

class ToolKillZoneFrustum:public ToolKillZone
	{
	/* Embedded classes: */
	public:
	typedef Geometry::Box<Scalar,3> Box;
	
	/* Elements: */
	private:
	Viewer* viewer; // Viewer whose head position defines the frustum
	VRScreen* screen; // Screen containing the frustum's base plane
	Box box; // Position and size of the kill zone's box in screen coordinates (z components both zero)
	
	/* Protected methods: */
	protected:
	virtual void renderModel(void) const;
	
	/* Constructors and destructors: */
	public:
	ToolKillZoneFrustum(const Misc::ConfigurationFileSection& configFileSection);
	
	/* Methods: */
	virtual Size getSize(void) const;
	virtual Point getCenter(void) const;
	virtual void setCenter(const Point& newCenter);
	bool isDeviceIn(const InputDevice* device) const;
	};

}

#endif
