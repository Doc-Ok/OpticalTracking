/***********************************************************************
MultiDeviceNavigationTool - Class to use multiple 3-DOF devices for full
navigation (translation, rotation, scaling).
Copyright (c) 2007-2013 Oliver Kreylos

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

#ifndef VRUI_MULTIDEVICENAVIGATIONTOOL_INCLUDED
#define VRUI_MULTIDEVICENAVIGATIONTOOL_INCLUDED

#include <Geometry/Point.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Vrui/NavigationTool.h>

namespace Vrui {

class MultiDeviceNavigationTool;

class MultiDeviceNavigationToolFactory:public ToolFactory
	{
	friend class MultiDeviceNavigationTool;
	
	/* Elements: */
	Scalar translationFactor; // Scale factor for translations
	Scalar minRotationScalingDistance; // Minimum distance from a device to the centroid for rotation and scaling to take effect
	Scalar rotationFactor; // Scale factor for rotations
	Scalar scalingFactor; // Scale factor for scalings
	
	/* Constructors and destructors: */
	public:
	MultiDeviceNavigationToolFactory(ToolManager& toolManager);
	virtual ~MultiDeviceNavigationToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class MultiDeviceNavigationTool:public NavigationTool
	{
	friend class MultiDeviceNavigationToolFactory;
	
	/* Elements: */
	private:
	static MultiDeviceNavigationToolFactory* factory; // Pointer to the factory object for this class
	
	/* Transient navigation state: */
	int numPressedButtons; // Number of currently pressed buttons
	bool* lastDeviceButtonStates; // Array of device button states from the last frame
	Point* lastDevicePositions; // Array of device positions from the last frame
	Point lastCentroid; // Centroid of all active devices from the last frame
	
	/* Constructors and destructors: */
	public:
	MultiDeviceNavigationTool(const ToolFactory* sFactory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	};

}

#endif
