/***********************************************************************
ViewpointSaverTool - Class for tools to save environment-independent
viewing parameters.
Copyright (c) 2007-2010 Oliver Kreylos

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

#ifndef VRUI_VIEWPOINTSAVERTOOL_INCLUDED
#define VRUI_VIEWPOINTSAVERTOOL_INCLUDED

#include <string>
#include <Vrui/UtilityTool.h>

/* Forward declarations: */
namespace Misc {
class File;
}

namespace Vrui {

class ViewpointSaverTool;

class ViewpointSaverToolFactory:public ToolFactory
	{
	friend class ViewpointSaverTool;
	
	/* Elements: */
	private:
	std::string viewpointFileName; // Name of file into which viewpoint data is saved
	Scalar axisLength; // Length of displayed axis markers in physical units
	float axisLineWidth; // Cosmetic line width for axis markers
	Misc::File* viewpointFile; // File into which viewpoint data is saved; opened on demand and shared by all viewpoint saver tools during an application's lifetime
	
	/* Constructors and destructors: */
	public:
	ViewpointSaverToolFactory(ToolManager& toolManager);
	virtual ~ViewpointSaverToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class ViewpointSaverTool:public UtilityTool
	{
	friend class ViewpointSaverToolFactory;
	
	/* Elements: */
	private:
	static ViewpointSaverToolFactory* factory; // Pointer to the factory object for this class
	
	/* Constructors and destructors: */
	public:
	ViewpointSaverTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void display(GLContextData& contextData) const;
	};

}

#endif
