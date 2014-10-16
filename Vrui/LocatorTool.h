/***********************************************************************
LocatorTool - Base class for tools encapsulating 6-DOF localization.
Copyright (c) 2004-2010 Oliver Kreylos

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

#ifndef VRUI_LOCATORTOOL_INCLUDED
#define VRUI_LOCATORTOOL_INCLUDED

#include <Misc/CallbackData.h>
#include <Misc/CallbackList.h>
#include <Misc/FunctionCalls.h>
#include <Vrui/Geometry.h>
#include <Vrui/Tool.h>

/* Forward declarations: */
namespace Vrui {
class ToolManager;
}

namespace Vrui {

class LocatorToolFactory:public ToolFactory
	{
	/* Constructors and destructors: */
	public:
	LocatorToolFactory(ToolManager& toolManager);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	};

class LocatorTool:public Tool
	{
	/* Embedded classes: */
	public:
	typedef Misc::FunctionCall<Misc::ConfigurationFileSection&> StoreStateFunction; // Type for functions to store state to a configuration file
	typedef Misc::FunctionCall<std::string&> GetNameFunction; // Type for functions to return a descriptive name for a locator tool associate
	
	class MotionCallbackData:public Misc::CallbackData
		{
		/* Elements: */
		public:
		LocatorTool* tool; // Pointer to the tool causing the event
		const NavTrackerState& currentTransformation; // Current motion transformation
		
		/* Constructors and destructors: */
		MotionCallbackData(LocatorTool* sTool,const NavTrackerState& sCurrentTransformation)
			:tool(sTool),
			 currentTransformation(sCurrentTransformation)
			{
			}
		};
	
	class ButtonPressCallbackData:public Misc::CallbackData
		{
		/* Elements: */
		public:
		LocatorTool* tool; // Pointer to the tool causing the event
		const NavTrackerState& currentTransformation; // Transformation at the moment the button was pressed
		
		/* Constructors and destructors: */
		ButtonPressCallbackData(LocatorTool* sTool,const NavTrackerState& sCurrentTransformation)
			:tool(sTool),
			 currentTransformation(sCurrentTransformation)
			{
			}
		};
	
	class ButtonReleaseCallbackData:public Misc::CallbackData
		{
		/* Elements: */
		public:
		LocatorTool* tool; // Pointer to the tool causing the event
		const NavTrackerState& currentTransformation; // Transformation at the moment the button was released
		
		/* Constructors and destructors: */
		ButtonReleaseCallbackData(LocatorTool* sTool,const NavTrackerState& sCurrentTransformation)
			:tool(sTool),
			 currentTransformation(sCurrentTransformation)
			{
			}
		};
	
	/* Elements: */
	protected:
	StoreStateFunction* storeStateFunction; // Function to be called when the tool stores its state to a configuration file
	GetNameFunction* getNameFunction; // Function to be called when the tool is asked for its name
	Misc::CallbackList motionCallbacks; // List of callbacks to be called when moving
	Misc::CallbackList buttonPressCallbacks; // List of callbacks to be called when button is pressed
	Misc::CallbackList buttonReleaseCallbacks; // List of callbacks to be called when button is released
	
	/* Constructors and destructors: */
	public:
	LocatorTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	virtual ~LocatorTool(void);
	
	/* Methods from Tool: */
	virtual void storeState(Misc::ConfigurationFileSection& configFileSection) const;
	virtual std::string getName(void) const;
	
	/* New methods: */
	void setStoreStateFunction(StoreStateFunction* newStoreStateFunction); // Adopts the given function, to be called when the tool stores its state to a configuration file
	void setGetNameFunction(GetNameFunction* newGetNameFunction); // Adopts the given function, to be called when the tool is asked for its name
	Misc::CallbackList& getMotionCallbacks(void) // Returns list of motion callbacks
		{
		return motionCallbacks;
		}
	Misc::CallbackList& getButtonPressCallbacks(void) // Returns list of button press callbacks
		{
		return buttonPressCallbacks;
		}
	Misc::CallbackList& getButtonReleaseCallbacks(void) // Returns list of button release callbacks
		{
		return buttonReleaseCallbacks;
		}
	};

}

#endif
