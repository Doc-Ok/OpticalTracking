/***********************************************************************
ForceJumpNavigationTool - Class to navigate by jumping from the viewer's
current position to the position of an input device.
Copyright (c) 2010-2013 Oliver Kreylos

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

#ifndef VRUI_FORCEJUMPNAVIGATIONTOOL_INCLUDED
#define VRUI_FORCEJUMPNAVIGATIONTOOL_INCLUDED

#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Vrui/NavigationTool.h>

namespace Vrui {

class ForceJumpNavigationTool;

class ForceJumpNavigationToolFactory:public ToolFactory
	{
	friend class ForceJumpNavigationTool;
	
	/* Elements: */
	private:
	Scalar gravity; // Gravity acceleration in physical coordinate units/s
	Scalar jumpTime; // Time to complete a jump in seconds
	
	/* Constructors and destructors: */
	public:
	ForceJumpNavigationToolFactory(ToolManager& toolManager);
	virtual ~ForceJumpNavigationToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class ForceJumpNavigationTool:public NavigationTool
	{
	friend class ForceJumpNavigationToolFactory;
	
	/* Elements: */
	private:
	static ForceJumpNavigationToolFactory* factory; // Pointer to the factory object for this class
	NavTransform startNav; // Navigation transformation at the beginning of move
	double startTime; // Application time at the beginning of move
	Vector translation; // The translation vector from start to end position in physical space
	Vector initialVelocity; // Initial jumping velocity
	Vector flipAxis; // The flip rotation axis
	double endTime; // Projected application time at the end of the move
	
	/* Constructors and destructors: */
	public:
	ForceJumpNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	};

}

#endif
