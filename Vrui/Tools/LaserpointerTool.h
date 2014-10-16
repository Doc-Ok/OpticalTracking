/***********************************************************************
LaserpointerTool - Class for tools using rays to point out features in a
3D display.
Copyright (c) 2006-2011 Oliver Kreylos

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

#ifndef VRUI_LASERPOINTERTOOL_INCLUDED
#define VRUI_LASERPOINTERTOOL_INCLUDED

#include <Geometry/Ray.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/GLColor.h>
#include <Vrui/Geometry.h>
#include <Vrui/PointingTool.h>

namespace Vrui {

class LaserpointerTool;

class LaserpointerToolFactory:public ToolFactory
	{
	friend class LaserpointerTool;
	
	/* Embedded classes: */
	private:
	typedef GLColor<GLfloat,3> Color; // Type for ray colors
	
	/* Elements: */
	private:
	Scalar rayLength; // Length of laser ray
	float rayLineWidth; // Line width to render ray
	Color rayColor; // Color to render ray
	
	/* Constructors and destructors: */
	public:
	LaserpointerToolFactory(ToolManager& toolManager);
	virtual ~LaserpointerToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class LaserpointerTool:public PointingTool
	{
	friend class LaserpointerToolFactory;
	
	/* Elements: */
	private:
	static LaserpointerToolFactory* factory; // Pointer to the factory object for this class
	
	/* Transient state: */
	bool active; // Flag if the laser pointer is active
	Ray ray; // Current laser ray
	
	/* Constructors and destructors: */
	public:
	LaserpointerTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	};

}

#endif
