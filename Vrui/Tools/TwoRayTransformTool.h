/***********************************************************************
TwoRayTransformTool - Class to select 3D positions using ray-based input
devices by intersecting two rays from two different starting points.
Copyright (c) 2010-2014 Oliver Kreylos

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

#ifndef VRUI_TWORAYTRANSFORMTOOL_INCLUDED
#define VRUI_TWORAYTRANSFORMTOOL_INCLUDED

#include <Geometry/Point.h>
#include <Geometry/Ray.h>
#include <Vrui/Geometry.h>
#include <Vrui/TransformTool.h>

namespace Vrui {

class TwoRayTransformTool;

class TwoRayTransformToolFactory:public ToolFactory
	{
	friend class TwoRayTransformTool;
	
	/* Constructors and destructors: */
	public:
	TwoRayTransformToolFactory(ToolManager& toolManager);
	virtual ~TwoRayTransformToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class TwoRayTransformTool:public TransformTool
	{
	friend class TwoRayTransformToolFactory;
	
	/* Elements: */
	private:
	static TwoRayTransformToolFactory* factory; // Pointer to the factory object for this class
	Ray rays[2]; // The two selection rays in navigational coordinates
	int numRays; // Number of finalized rays
	bool active; // Flag whether the tool is currently dragging a ray
	Point intersection; // Intersection point between two rays in navigational coordinates
	
	/* Constructors and destructors: */
	public:
	TwoRayTransformTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	virtual ~TwoRayTransformTool(void);
	
	/* Methods from Tool: */
	virtual void initialize(void);
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	};

}

#endif
