/***********************************************************************
Factory - Abstract base class for all factory classes. Intended as a
wrapper for classes that can be loaded at runtime.
Copyright (c) 2003-2006 Oliver Kreylos

This file is part of the Plugin Handling Library (Plugins).

The Plugin Handling Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Plugin Handling Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Plugin Handling Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <string.h>

#include <Plugins/Factory.h>

namespace Plugins {

/************************
Methods of class Factory:
************************/

Factory::Factory(const char* sClassName)
	:className(new char[strlen(sClassName)+1])
	{
	strcpy(className,sClassName);
	}

Factory::~Factory(void)
	{
	/* Remove factory from class hierarchy: */
	for(ClassList::iterator pIt=parentClasses.begin();pIt!=parentClasses.end();++pIt)
		{
		/* Remove this factory from parent's child list: */
		for(ClassList::iterator chIt=(*pIt)->childClasses.begin();chIt!=(*pIt)->childClasses.end();++chIt)
			if(*chIt==this)
				{
				(*pIt)->childClasses.erase(chIt);
				break;
				}
		}
	for(ClassList::iterator chIt=childClasses.begin();chIt!=childClasses.end();++chIt)
		{
		/* Remove this factory from child's parent list: */
		ClassList::iterator pIt;
		for(ClassList::iterator pIt=(*chIt)->parentClasses.begin();pIt!=(*chIt)->parentClasses.end();++pIt)
			if(*pIt==this)
				{
				(*chIt)->parentClasses.erase(pIt);
				break;
				}
		}
	
	delete[] className;
	}

void Factory::addParentClass(Factory* newParentClass)
	{
	parentClasses.push_back(newParentClass);
	}

void Factory::addChildClass(Factory* newChildClass)
	{
	childClasses.push_back(newChildClass);
	}

bool Factory::isDerivedFrom(const char* baseClassName) const
	{
	/* Return true if this factory creates the base class: */
	if(strcmp(className,baseClassName)==0)
		return true;
	
	/* Find out if any of the parent classes are derived from the base class: */
	for(ClassList::const_iterator pIt=parentClasses.begin();pIt!=parentClasses.end();++pIt)
		if((*pIt)->isDerivedFrom(baseClassName))
			return true;
	
	return false;
	}

}
