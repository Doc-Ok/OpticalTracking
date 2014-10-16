/***********************************************************************
TrackballNavigationTool - Class for trackball navigation in 3D space.
Copyright (c) 2004-2013 Oliver Kreylos

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

#ifndef VRUI_TRACKBALLNAVIGATIONTOOL_INCLUDED
#define VRUI_TRACKBALLNAVIGATIONTOOL_INCLUDED

#include <Geometry/Point.h>
#include <Vrui/NavigationTool.h>

namespace Vrui {

class TrackballNavigationTool;

class TrackballNavigationToolFactory:public ToolFactory
	{
	friend class TrackballNavigationTool;
	
	/* Elements: */
	private:
	Scalar rotateFactor; // Scaling factor for rotation angles
	
	/* Constructors and destructors: */
	public:
	TrackballNavigationToolFactory(ToolManager& toolManager);
	virtual ~TrackballNavigationToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class TrackballNavigationTool:public NavigationTool
	{
	friend class TrackballNavigationToolFactory;
	
	/* Elements: */
	private:
	static TrackballNavigationToolFactory* factory; // Pointer to the factory object for this class
	
	/* Transient navigation state: */
	Point lastDragPosition; // Last drag position on trackball
	
	/* Private methods: */
	Point calcTrackballPosition(void) const; // Calculates the intersection of the interaction ray and the trackball sphere
	
	/* Constructors and destructors: */
	public:
	TrackballNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	};

}

#endif
