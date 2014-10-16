/***********************************************************************
ComeHitherNavigationTool - Class to navigate by smoothly moving the
position of a 3D input device to the display center point.
Copyright (c) 2008-2013 Oliver Kreylos

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

#ifndef VRUI_COMEHITHERNAVIGATIONTOOL_INCLUDED
#define VRUI_COMEHITHERNAVIGATIONTOOL_INCLUDED

#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Vrui/NavigationTool.h>

namespace Vrui {

class ComeHitherNavigationTool;

class ComeHitherNavigationToolFactory:public ToolFactory
	{
	friend class ComeHitherNavigationTool;
	
	/* Elements: */
	private:
	Scalar linearSnapThreshold; // Maximum linear distance to snap to new position/orientation immediately
	Scalar angularSnapThreshold; // Maximum angular distance to snap to new position/orientation immediately
	Scalar maxLinearVelocity; // Maximum linear velocity during smooth motion
	Scalar maxAngularVelocity; // Maximum angular velocity during smooth motion
	
	/* Constructors and destructors: */
	public:
	ComeHitherNavigationToolFactory(ToolManager& toolManager);
	virtual ~ComeHitherNavigationToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class ComeHitherNavigationTool:public NavigationTool
	{
	friend class ComeHitherNavigationToolFactory;
	
	/* Elements: */
	private:
	static ComeHitherNavigationToolFactory* factory; // Pointer to the factory object for this class
	NavTransform startNav; // Navigation transformation at the beginning of move
	double startTime; // Application time at the beginning of move
	NavTransform targetNav; // Target navigation transformation at the end of move
	double endTime; // Projected application time at the end of the move
	Vector linearVelocity; // Linear velocity of move
	Vector angularVelocity; // Angular velocity of move
	
	/* Constructors and destructors: */
	public:
	ComeHitherNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	};

}

#endif
