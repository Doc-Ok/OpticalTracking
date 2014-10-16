/***********************************************************************
ClipPlaneTool - Class for tools that add a clipping plane into an
environment when activated.
Copyright (c) 2009-2010 Oliver Kreylos

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

#ifndef VRUI_CLIPPLANETOOL_INCLUDED
#define VRUI_CLIPPLANETOOL_INCLUDED

#include <Geometry/Vector.h>
#include <Geometry/Plane.h>
#include <Vrui/Geometry.h>
#include <Vrui/PointingTool.h>

/* Forward declarations: */
namespace Vrui {
class ClipPlane;
class ToolManager;
}

namespace Vrui {

class ClipPlaneTool;

class ClipPlaneToolFactory:public ToolFactory
	{
	friend class ClipPlaneTool;
	
	/* Elements: */
	private:
	Vector normal; // Normal vector for clipping planes in device coordinates
	
	/* Constructors and destructors: */
	public:
	ClipPlaneToolFactory(ToolManager& toolManager);
	virtual ~ClipPlaneToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class ClipPlaneTool:public PointingTool
	{
	friend class ClipPlaneToolFactory;
	
	/* Elements: */
	private:
	static ClipPlaneToolFactory* factory; // Pointer to the factory object for this class
	ClipPlane* clipPlane; // Clipping plane object allocated for this clipping plane tool
	
	/* Transient state: */
	bool active; // Flag if the tool is currently active
	
	/* Constructors and destructors: */
	public:
	ClipPlaneTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	virtual ~ClipPlaneTool(void);
	
	/* Methods from Tool: */
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	};

}

#endif
