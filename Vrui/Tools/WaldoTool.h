/***********************************************************************
WaldoTool - Class to scale translations and rotations on 6-DOF input
devices to improve interaction accuracy in tracked environments.
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

#ifndef VRUI_WALDOTOOL_INCLUDED
#define VRUI_WALDOTOOL_INCLUDED

#include <Geometry/OrthonormalTransformation.h>
#include <Vrui/TransformTool.h>

/* Forward declarations: */
namespace Vrui {
class Glyph;
}

namespace Vrui {

/* Forward declarations: */
class WaldoTool;

class WaldoToolFactory:public ToolFactory
	{
	friend class WaldoTool;
	
	/* Elements: */
	private:
	Scalar linearScale; // Scale factor for linear motions
	Scalar angularScale; // Scale factor for angular motions
	
	/* Constructors and destructors: */
	public:
	WaldoToolFactory(ToolManager& toolManager);
	virtual ~WaldoToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class WaldoTool:public TransformTool
	{
	friend class WaldoToolFactory;
	
	/* Elements: */
	private:
	static WaldoToolFactory* factory; // Pointer to the factory object for this class
	Glyph* waldoGlyph; // Pointer to visual representation of virtual waldo input device
	
	/* Transient motion scaling state: */
	int numPressedButtons; // Number of currently pressed buttons on transformed device
	bool transformActive; // Flag if the waldo transformation is active
	TrackerState last; // Last used source input device transformation
	
	/* Constructors and destructors: */
	public:
	WaldoTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual void initialize(void);
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	};

}

#endif
