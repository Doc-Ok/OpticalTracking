/***********************************************************************
ObjectLoader - Light-weight class to load objects from dynamic shared
objects.
Copyright (c) 2009-2010 Oliver Kreylos

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

#ifndef PLUGINS_OBJECTLOADER_INCLUDED
#define PLUGINS_OBJECTLOADER_INCLUDED

#include <string>
#include <stdexcept>
#include <Misc/HashTable.h>
#include <Misc/FileLocator.h>

namespace Plugins {

template <class ManagedClassParam>
class ObjectLoader
	{
	/* Embedded classes: */
	public:
	typedef ManagedClassParam ManagedClass; // Base class of objects that can be loaded
	
	struct Error:public std::runtime_error
		{
		/* Constructors and destructors: */
		Error(std::string cause)
			:std::runtime_error(cause)
			{
			}
		};
	
	struct DsoError:public Error // Error type thrown if something goes wrong while processing a DSO
		{
		/* Constructors and destructors: */
		DsoError(std::string cause) // Creates DsoError object from error string returned by dl_* calls
			:Error(std::string("Object loader DSO error: ")+cause)
			{
			}
		};
	
	private:
	typedef ManagedClass* (*CreateObjectFunction)(ObjectLoader<ManagedClass>&); // Type of object creation function stored in DSOs
	typedef void (*DestroyObjectFunction)(ManagedClass*); // Type of object destruction function stored in DSOs
	
	struct DsoState // Structure retaining information about loaded DSOs
		{
		/* Elements: */
		public:
		void* dsoHandle; // Handle of DSO containing class code
		DestroyObjectFunction destroyObjectFunction; // Pointer to function to destroy objects of this class
		
		/* Constructors and destructors: */
		DsoState(void)
			:dsoHandle(0)
			{
			}
		};
	
	typedef Misc::HashTable<ManagedClass*,DsoState> DsoStateHasher; // Type for hash tables to find the DSO state of an object
	
	/* Elements: */
	std::string dsoNameTemplate; // printf-style format string to create DSO names from class names
	Misc::FileLocator dsoLocator; // File locator to find DSO files
	DsoStateHasher dsoStates; // Hash table of DSO states of created objects
	
	/* Constructors and destructors: */
	public:
	ObjectLoader(std::string sDsoNameTemplate); // Creates "empty" manager; initializes DSO locator search path to template's base directory
	~ObjectLoader(void); // Releases all loaded object classes and DSOs
	
	/* Methods: */
	const Misc::FileLocator& getDsoLocator(void) const // Returns reference to the DSO file locator
		{
		return dsoLocator;
		}
	Misc::FileLocator& getDsoLocator(void) // Ditto
		{
		return dsoLocator;
		}
	ManagedClass* createObject(const char* className); // Creates an object of the given class name by searching for a matching DSO
	bool isManaged(ManagedClass* object) const // Returns true if the given object is managed by the object loader
		{
		return dsoStates.isEntry(object);
		}
	void destroyObject(ManagedClass* object); // Destroys the object and releases the DSO from which it was loaded
	};

}

#ifndef PLUGINS_OBJECTLOADER_IMPLEMENTATION
#include <Plugins/ObjectLoader.icpp>
#endif

#endif
