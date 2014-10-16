/***********************************************************************
GenericToolFactory - Class for factories for generic user interaction
tools.
Copyright (c) 2005-2014 Oliver Kreylos

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

#ifndef VRUI_GENERICTOOLFACTORY_INCLUDED
#define VRUI_GENERICTOOLFACTORY_INCLUDED

#include <string>
#include <vector>
#include <Vrui/Tool.h>

namespace Vrui {

/***********************************************************************
Base class containing functionality shared by all instantiated generic
tool factory classes:
***********************************************************************/

class GenericToolFactoryBase:public ToolFactory
	{
	/* Elements: */
	protected:
	std::string displayName; // Display name for tools of this class
	std::vector<std::string> buttonFunctions; // List of function descriptions for buttons of tools of this class
	std::vector<std::string> valuatorFunctions; // List of function descriptions for valuators of tools of this class
	
	/* Constructors and destructors: */
	GenericToolFactoryBase(const char* sClassName,const char* sDisplayName,ToolFactory* parentClass,ToolManager& toolManager); // Creates tool factory with basic settings
	
	/* Methods from ToolFactory: */
	public:
	virtual const char* getName(void) const
		{
		return displayName.c_str();
		}
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual const char* getValuatorFunction(int valuatorSlotIndex) const;
	
	/* New methods: */
	void setNumButtons(int newNumButtons,bool newOptionalButtons =false); // Allows clients to override the tool class' layout
	void setNumValuators(int newNumValuators,bool newOptionalValuators =false); // Allows clients to override the tool class' layout
	void setButtonFunction(int buttonSlot,const char* newButtonFunction); // Allows clients to set button descriptions
	void setValuatorFunction(int valuatorSlot,const char* newValuatorFunction); // Allows clients to set valuator descriptions
	};

/***********************************************************************
Generic tool factory class:
***********************************************************************/

template <class CreatedToolParam>
class GenericToolFactory:public GenericToolFactoryBase
	{
	/* Embedded classes: */
	public:
	typedef CreatedToolParam CreatedTool; // Class of tools created by this factory
	
	/* Constructors and destructors: */
	public:
	GenericToolFactory(const char* sClassName,const char* sDisplayName,ToolFactory* parentClass,ToolManager& toolManager) // Creates tool factory with basic settings
		:GenericToolFactoryBase(sClassName,sDisplayName,parentClass,toolManager)
		{
		/* Set the created tool class' factory pointer: */
		CreatedTool::factory=this;
		}
	private:
	GenericToolFactory(const GenericToolFactory& source); // Prohibit copy constructor
	GenericToolFactory& operator=(const GenericToolFactory& source); // Prohibit assignment operator
	public:
	virtual ~GenericToolFactory(void) // Destroys the tool factory
		{
		/* Reset the tool class' factory pointer: */
		CreatedTool::factory=0;
		}
	
	/* Methods from ToolFactory: */
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const
		{
		return new CreatedTool(this,inputAssignment);
		}
	virtual void destroyTool(Tool* tool) const
		{
		delete tool;
		}
	};

}

#endif
