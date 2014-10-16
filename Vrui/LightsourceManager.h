/***********************************************************************
LightsourceManager - Class to manage light sources in virtual
environments. Maps created Lightsource objects to OpenGL light sources.
Copyright (c) 2005-2012 Oliver Kreylos

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

#ifndef VRUI_LIGHTSOURCEMANAGER_INCLUDED
#define VRUI_LIGHTSOURCEMANAGER_INCLUDED

#include <Vrui/Geometry.h>
#include <Vrui/Lightsource.h>

/* Forward declarations: */
class GLContextData;
namespace Vrui {
class DisplayState;
}

namespace Vrui {

class LightsourceManager
	{
	/* Embedded classes: */
	private:
	struct LightsourceListItem:public Lightsource
		{
		/* Elements: */
		public:
		bool physical; // Flag if the light source is defined in physical coordinates
		LightsourceListItem* succ; // Pointer to next element in light source list
		
		/* Constructors and destructors: */
		LightsourceListItem(bool sPhysical)
			:physical(sPhysical),succ(0)
			{
			}
		LightsourceListItem(bool sPhysical,const GLLight& sLight)
			:Lightsource(sLight),
			 physical(sPhysical),succ(0)
			{
			}
		};
	
	/* Elements: */
	private:
	LightsourceListItem* firstLightsource; // Pointer to first light source
	LightsourceListItem* lastLightsource; // Pointer to last light source
	
	/* Constructors and destructors: */
	public:
	LightsourceManager(void); // Creates an empty light source manager
	~LightsourceManager(void); // Destroys the light source manager
	
	/* Methods: */
	Lightsource* createLightsource(bool physical); // Creates an enabled light source with standard OpenGL parameters
	Lightsource* createLightsource(bool physical,const GLLight& sLight); // Creates an enabled light source with the given OpenGL parameters
	void destroyLightsource(Lightsource* lightsource); // Destroys the given light source
	void setLightsources(bool navigationEnabled,DisplayState* displayState,GLContextData& contextData) const; // Sets the light sources in the current OpenGL context
	};

}

#endif
