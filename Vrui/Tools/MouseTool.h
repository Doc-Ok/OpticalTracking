/***********************************************************************
MouseTool - Class to map regular 2D mice into VR environments by
representing them as virtual input devices sliding along the screen
planes.
Copyright (c) 2005-2010 Oliver Kreylos

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

#ifndef VRUI_MOUSETOOL_INCLUDED
#define VRUI_MOUSETOOL_INCLUDED

#include <Vrui/TransformTool.h>

namespace Vrui {

class MouseTool;

class MouseToolFactory:public ToolFactory
	{
	friend class MouseTool;
	
	/* Elements: */
	private:
	Scalar rayOffset; // Amount by which to set back the intersection ray's position
	Scalar crosshairSize; // Size of the screen-aligned crosshairs
	
	/* Constructors and destructors: */
	public:
	MouseToolFactory(ToolManager& toolManager);
	virtual ~MouseToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class MouseTool:public TransformTool
	{
	friend class MouseToolFactory;
	
	/* Elements: */
	private:
	static MouseToolFactory* factory; // Pointer to the factory object for this class
	
	/* Constructors and destructors: */
	public:
	MouseTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	virtual ~MouseTool(void);
	
	/* Methods from Tool: */
	virtual void initialize(void);
	virtual const ToolFactory* getFactory(void) const;
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	};

}

#endif
