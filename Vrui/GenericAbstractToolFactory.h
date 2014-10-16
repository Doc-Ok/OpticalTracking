/***********************************************************************
GenericToolFactory - Class for factories for generic user interaction
tools that are never actually created, i.e., only serve as base classes
for derived tools.
Copyright (c) 2009-2013 Oliver Kreylos

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

#ifndef VRUI_GENERICABSTRACTTOOLFACTORY_INCLUDED
#define VRUI_GENERICABSTRACTTOOLFACTORY_INCLUDED

#include <string>
#include <Vrui/Tool.h>

namespace Vrui {

template <class CreatedToolParam>
class GenericAbstractToolFactory:public ToolFactory
	{
	/* Embedded classes: */
	public:
	typedef CreatedToolParam CreatedTool; // Class of tools created by this factory (never actually instantiated)
	
	/* Elements: */
	private:
	std::string displayName; // Display name for tools of this class
	
	/* Constructors and destructors: */
	public:
	GenericAbstractToolFactory(const char* sClassName,const char* sDisplayName,ToolFactory* parentClass,ToolManager& toolManager)
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
	~GenericAbstractToolFactory(void)
		{
		}
	
	/* Methods from class ToolFactory: */
	virtual const char* getName(void) const
		{
		return displayName.c_str();
		}
	};

}

#endif
