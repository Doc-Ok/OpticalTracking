/***********************************************************************
ClipPlaneManager - Class to manage clipping planes in virtual
environments. Maps created ClipPlane objects to OpenGL clipping planes.
Copyright (c) 2009-2012 Oliver Kreylos

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

#ifndef VRUI_CLIPPLANEMANAGER_INCLUDED
#define VRUI_CLIPPLANEMANAGER_INCLUDED

#include <Vrui/Geometry.h>
#include <Vrui/ClipPlane.h>

/* Forward declarations: */
class GLContextData;
namespace Vrui {
class DisplayState;
}

namespace Vrui {

class ClipPlaneManager
	{
	/* Embedded classes: */
	private:
	struct ClipPlaneListItem:public ClipPlane
		{
		/* Elements: */
		public:
		bool physical; // Flag if the clipping plane is defined in physical coordinates
		ClipPlaneListItem* succ; // Pointer to next element in clipping plane list
		
		/* Constructors and destructors: */
		ClipPlaneListItem(bool sPhysical)
			:physical(sPhysical),succ(0)
			{
			}
		ClipPlaneListItem(bool sPhysical,const Plane& sPlane)
			:ClipPlane(sPlane),
			 physical(sPhysical),succ(0)
			{
			}
		};
	
	/* Elements: */
	private:
	ClipPlaneListItem* firstClipPlane; // Pointer to first clipping plane
	ClipPlaneListItem* lastClipPlane; // Pointer to last clipping plane
	
	/* Constructors and destructors: */
	public:
	ClipPlaneManager(void); // Creates an empty clipping plane manager
	~ClipPlaneManager(void); // Destroys the clipping plane manager
	
	/* New methods: */
	ClipPlane* createClipPlane(bool physical); // Creates a disabled clipping plane
	ClipPlane* createClipPlane(bool physical,const Plane& sPlane); // Creates an enabled clipping plane with the given plane equation
	void destroyClipPlane(ClipPlane* clipPlane); // Destroys the given clipping plane
	void setClipPlanes(bool navigationEnabled,DisplayState* displayState,GLContextData& contextData) const; // Sets the clipping planes in the current OpenGL context
	void clipRay(bool physical,Ray& ray,Scalar& lambdaMax) const; // Clips the given ray (in physical or navigational coordinates) against all enabled clipping planes by adjusting the ray's origin and the maximum ray intercept
	};

}

#endif
