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

#ifndef PLUGINS_FACTORY_INCLUDED
#define PLUGINS_FACTORY_INCLUDED

#include <vector>

namespace Plugins {

class Factory
	{
	/* Embedded classes: */
	public:
	typedef std::vector<Factory*> ClassList;
	
	/* Elements: */
	private:
	char* className; // Name of the produced class
	ClassList parentClasses; // List of parent classes of the produced class
	ClassList childClasses; // List of child classes of the produced class
	
	/* Constructors and destructors: */
	protected:
	Factory(const char* sClassName);
	public:
	virtual ~Factory(void);
	
	/* Methods: */
	public:
	const char* getClassName(void) const // Returns class name
		{
		return className;
		}
	void addParentClass(Factory* newParentClass); // Adds another parent class to the dynamic class hierarchy
	void addChildClass(Factory* newChildClass); // Adds another child class to the dynamic class hierarchy
	bool isDerivedFrom(const char* baseClassName) const; // Returns true if the class created by this factory is derived from the class created by the factory of the given name
	const ClassList& getParents(void) const // Return list of parent classes
		{
		return parentClasses;
		}
	ClassList::const_iterator parentsBegin(void) const // Returns iterator to the beginning of the parent class list
		{
		return parentClasses.begin();
		}
	ClassList::const_iterator parentsEnd(void) const // Returns iterator to the end of the parent class list
		{
		return parentClasses.end();
		}
	const ClassList& getChildren(void) const // Return list of child classes
		{
		return childClasses;
		}
	ClassList::const_iterator childrenBegin(void) const // Returns iterator to the beginning of the child class list
		{
		return childClasses.begin();
		}
	ClassList::const_iterator childrenEnd(void) const // Returns iterator to the end of the child class list
		{
		return childClasses.end();
		}
	};

}

#endif
