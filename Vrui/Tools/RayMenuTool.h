/***********************************************************************
RayMenuTool - Class for menu selection tools using ray selection.
Copyright (c) 2004-2014 Oliver Kreylos

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

#ifndef VRUI_RAYMENUTOOL_INCLUDED
#define VRUI_RAYMENUTOOL_INCLUDED

#include <Vrui/GUIInteractor.h>
#include <Vrui/MenuTool.h>

namespace Vrui {

class RayMenuTool;

class RayMenuToolFactory:public ToolFactory
	{
	friend class RayMenuTool;
	
	/* Elements: */
	private:
	Scalar initialMenuOffset; // Offset of initial menu position along selection ray
	bool interactWithWidgets; // Flag if the menu tool doubles as a widget interaction tool
	
	/* Constructors and destructors: */
	public:
	RayMenuToolFactory(ToolManager& toolManager);
	virtual ~RayMenuToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class RayMenuTool:public MenuTool,public GUIInteractor
	{
	friend class RayMenuToolFactory;
	
	/* Elements: */
	private:
	static RayMenuToolFactory* factory; // Pointer to the factory object for this class
	
	/* Constructors and destructors: */
	public:
	RayMenuTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	
	/* Methods from GUIInteractor: */
	virtual Point calcHotSpot(void) const;
	};

}

#endif
