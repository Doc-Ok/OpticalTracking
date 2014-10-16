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

#include <Vrui/CoordinateManager.h>

#include <string.h>
#include <Vrui/CoordinateTransform.h>

namespace Vrui {

/**********************************
Methods of class CoordinateManager:
**********************************/

CoordinateManager::CoordinateManager(void)
	:transform(0)
	{
	}

CoordinateManager::~CoordinateManager(void)
	{
	delete transform;
	}

void CoordinateManager::setUnit(const Geometry::LinearUnit& newUnit)
	{
	unit=newUnit;
	}

void CoordinateManager::setCoordinateTransform(CoordinateTransform* newTransform)
	{
	/* Delete the previous coordinate transformation: */
	delete transform;
	
	/* Adopt the new coordinate transformation: */
	transform=newTransform;
	}

}
