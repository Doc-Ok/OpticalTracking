/***********************************************************************
DaisyWheelTool - Class for tools to enter text by pointing at characters
on a dynamic daisy wheel.
Copyright (c) 2008-2010 Oliver Kreylos

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

#ifndef VRUI_DAISYWHEELTOOL_INCLUDED
#define VRUI_DAISYWHEELTOOL_INCLUDED

#include <Geometry/Ray.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Vrui/Geometry.h>
#include <Vrui/UserInterfaceTool.h>

/* Forward declarations: */
class GLLabel;
namespace Vrui {
class ToolManager;
}

namespace Vrui {

class DaisyWheelTool;

class DaisyWheelToolFactory:public ToolFactory
	{
	friend class DaisyWheelTool;
	
	/* Elements: */
	private:
	Scalar petalSize; // Half side length of petal label square
	Scalar innerRadius,outerRadius; // Inner and outer radius of daisy wheel in physical coordinate units
	Scalar maxPetalAngle; // Angle assigned to selected petal at full zoom
	Scalar maxYOffset; // Maximum depth offset to stack "petals" in order of distance from selection
	
	/* Constructors and destructors: */
	public:
	DaisyWheelToolFactory(ToolManager& toolManager);
	virtual ~DaisyWheelToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class DaisyWheelTool:public UserInterfaceTool
	{
	friend class DaisyWheelToolFactory;
	
	/* Elements: */
	private:
	static DaisyWheelToolFactory* factory; // Pointer to the factory object for this class
	
	/* Transient state: */
	int numPetals; // Number of "petals" on the daisy wheel
	Scalar angleStep; // Angle per petal
	GLLabel* petals; // Characters associated with each petal
	bool active; // Flag if the tool is currently active
	bool buttonDown; // Flag whether the tool button is currently pressed
	bool hasEnteredWheel; // Flag if the selection ray has entered the daisy wheel while the button was down
	ONTransform wheelTransform; // Transformation from wheel coordinate to physical coordinates
	Ray selectionRay; // Current selection ray
	Scalar zoomStrength; // Angle zooming strength factor
	int selectedPetal; // Index of selected petal
	Scalar selectedAngle; // Angle at which selected petal appears
	bool rayInPetal; // Flag whether the selection ray is currently intersecting the selected petal
	
	/* Private methods: */
	Scalar calcPetalAngle(Scalar petal) const; // Calculates the display angle of the given petal
	
	/* Constructors and destructors: */
	public:
	DaisyWheelTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	virtual ~DaisyWheelTool(void);
	
	/* Methods from Tool: */
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	};

}

#endif
