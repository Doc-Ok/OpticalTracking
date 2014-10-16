/***********************************************************************
CoordinateManager - Class to manage the (navigation) coordinate system
of a Vrui application to support system-wide navigation manipulation
interfaces.
Copyright (c) 2007-2010 Oliver Kreylos

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

#ifndef VRUI_COORDINATEMANAGER_INCLUDED
#define VRUI_COORDINATEMANAGER_INCLUDED

#include <Misc/CallbackData.h>
#include <Misc/CallbackList.h>
#include <Geometry/LinearUnit.h>
#include <Vrui/Geometry.h>

/* Forward declarations: */
namespace Vrui {
class CoordinateTransform;
}

namespace Vrui {

class CoordinateManager
	{
	/* Embedded classes: */
	public:
	class CallbackData:public Misc::CallbackData // Base class for coordinate manager events
		{
		/* Constructors and destructors: */
		public:
		CallbackData(void)
			{
			}
		};
	
	class CoordinateTransformChangedCallbackData:public CallbackData // Class for callback data sent when the user coordinate transformation changes
		{
		/* Elements: */
		public:
		CoordinateTransform* oldTransform; // Previous coordinate transformation
		CoordinateTransform* newTransform; // New coordinate transformation (already installed at the time callback is called)
		
		/* Constructors and destructors: */
		CoordinateTransformChangedCallbackData(CoordinateTransform* sOldTransform,CoordinateTransform* sNewTransform)
			:oldTransform(sOldTransform),newTransform(sNewTransform)
			{
			}
		};
	
	/* Elements: */
	private:
	
	/* Current coordinate unit: */
	Geometry::LinearUnit unit; // Unit of length measurements
	
	/* Current coordinate transformation: */
	CoordinateTransform* transform; // Coordinate transformation from navigation space to "user interest space," used by measurement tools
	Misc::CallbackList coordinateTransformChangedCallbacks; // List of callbacks to be called when the user coordinate transformation changes
	
	/* Constructors and destructors: */
	public:
	CoordinateManager(void); // Creates coordinate manager with default settings (unknown unit with factor 1)
	~CoordinateManager(void); // Destroys coordinate manager
	
	/* Methods: */
	void setUnit(const Geometry::LinearUnit& newUnit); // Sets the application's coordinate unit and scale factor
	const Geometry::LinearUnit& getUnit(void) const // Returns the current application coordinate unit
		{
		return unit;
		}
	void setCoordinateTransform(CoordinateTransform* newTransform); // Sets a new coordinate transformation; coordinate manager adopts object
	const CoordinateTransform* getCoordinateTransform(void) const // Returns current coordinate transformation
		{
		return transform;
		}
	CoordinateTransform* getCoordinateTransform(void) // Ditto
		{
		return transform;
		}
	Misc::CallbackList& getCoordinateTransformChangedCallbacks(void) // Returns the list of coordinate transformation change callbacks
		{
		return coordinateTransformChangedCallbacks;
		}
	};

}

#endif
