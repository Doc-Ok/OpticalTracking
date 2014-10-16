/***********************************************************************
ToolKillZoneBox - Class for box-shaped "kill zones" for tools and input
devices.
Copyright (c) 2004-2014 Oliver Kreylos

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

#ifndef VRUI_INTERNAL_TOOLKILLZONEBOX_INCLUDED
#define VRUI_INTERNAL_TOOLKILLZONEBOX_INCLUDED

#include <Geometry/Box.h>
#include <Vrui/Geometry.h>
#include <Vrui/Internal/ToolKillZone.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}

namespace Vrui {

class ToolKillZoneBox:public ToolKillZone
	{
	/* Embedded classes: */
	public:
	typedef Geometry::Box<Scalar,3> Box;
	
	/* Elements: */
	private:
	Box box; // Position and size of the kill zone's box
	
	/* Protected methods: */
	protected:
	virtual void renderModel(void) const;
	
	/* Constructors and destructors: */
	public:
	ToolKillZoneBox(const Misc::ConfigurationFileSection& configFileSection);
	
	/* Methods: */
	virtual Size getSize(void) const
		{
		return box.getSize();
		}; 
	virtual Point getCenter(void) const;
	virtual void setCenter(const Point& newCenter);
	bool isDeviceIn(const InputDevice* device) const;
	};

}

#endif
