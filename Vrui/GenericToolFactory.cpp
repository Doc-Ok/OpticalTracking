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

#include <Vrui/GenericToolFactory.h>

namespace Vrui {

/***************************************
Methods of class GenericToolFactoryBase:
***************************************/

GenericToolFactoryBase::GenericToolFactoryBase(const char* sClassName,const char* sDisplayName,ToolFactory* parentClass,ToolManager& toolManager)
	:ToolFactory(sClassName,toolManager),
	 displayName(sDisplayName)
	{
	/* Add the tool factory to the class hierarchy: */
	if(parentClass!=0)
		{
		parentClass->addChildClass(this);
		addParentClass(parentClass);
		}
	}

const char* GenericToolFactoryBase::getButtonFunction(int buttonSlotIndex) const
	{
	if(buttonSlotIndex>=0&&buttonSlotIndex<int(buttonFunctions.size())&&!buttonFunctions[buttonSlotIndex].empty())
		return buttonFunctions[buttonSlotIndex].c_str();
	else
		return ToolFactory::getButtonFunction(buttonSlotIndex);
	}

const char* GenericToolFactoryBase::getValuatorFunction(int valuatorSlotIndex) const
	{
	if(valuatorSlotIndex>=0&&valuatorSlotIndex<int(valuatorFunctions.size())&&!valuatorFunctions[valuatorSlotIndex].empty())
		return valuatorFunctions[valuatorSlotIndex].c_str();
	else
		return ToolFactory::getValuatorFunction(valuatorSlotIndex);
	}

void GenericToolFactoryBase::setNumButtons(int newNumButtons,bool newOptionalButtons)
	{
	/* Forward the call to the layout structure: */
	layout.setNumButtons(newNumButtons,newOptionalButtons);
	
	/* Shrink the button description list if there are no optional buttons and it is too long: */
	if(!newOptionalButtons&&int(buttonFunctions.size())>newNumButtons)
		{
		std::vector<std::string> newButtonFunctions;
		newButtonFunctions.reserve(newNumButtons);
		for(int i=0;i<newNumButtons;++i)
			newButtonFunctions.push_back(buttonFunctions[i]);
		std::swap(buttonFunctions,newButtonFunctions);
		}
	}

void GenericToolFactoryBase::setNumValuators(int newNumValuators,bool newOptionalValuators)
	{
	/* Forward the call to the layout structure: */
	layout.setNumValuators(newNumValuators,newOptionalValuators);
	
	/* Shrink the valuator description list if there are no optional valuators and it is too long: */
	if(!newOptionalValuators&&int(valuatorFunctions.size())>newNumValuators)
		{
		std::vector<std::string> newValuatorFunctions;
		newValuatorFunctions.reserve(newNumValuators);
		for(int i=0;i<newNumValuators;++i)
			newValuatorFunctions.push_back(valuatorFunctions[i]);
		std::swap(valuatorFunctions,newValuatorFunctions);
		}
	}

void GenericToolFactoryBase::setButtonFunction(int buttonSlot,const char* newButtonFunction)
	{
	/* Ensure that the button description list is long enough: */
	if(int(buttonFunctions.size())<buttonSlot+1)
		{
		buttonFunctions.reserve(buttonSlot+1);
		for(int i=int(buttonFunctions.size());i<=buttonSlot;++i)
			buttonFunctions.push_back(std::string());
		}
	
	/* Override the previous button description: */
	buttonFunctions[buttonSlot]=newButtonFunction;
	}

void GenericToolFactoryBase::setValuatorFunction(int valuatorSlot,const char* newValuatorFunction)
	{
	/* Ensure that the valuator description list is long enough: */
	if(int(valuatorFunctions.size())<valuatorSlot+1)
		{
		valuatorFunctions.reserve(valuatorSlot+1);
		for(int i=int(valuatorFunctions.size());i<=valuatorSlot;++i)
			valuatorFunctions.push_back(std::string());
		}
	
	/* Override the previous valuator description: */
	valuatorFunctions[valuatorSlot]=newValuatorFunction;
	}

}
