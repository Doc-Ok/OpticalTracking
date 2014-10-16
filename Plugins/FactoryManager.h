/***********************************************************************
FactoryManager - Generic base class for managers of factory classes
derived from a common base class. Intended to manage loading of dynamic
shared objects.
Copyright (c) 2003-2013 Oliver Kreylos

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

#ifndef PLUGINS_FACTORYMANAGER_INCLUDED
#define PLUGINS_FACTORYMANAGER_INCLUDED

#include <list>
#include <string>
#include <stdexcept>
#include <Misc/FileLocator.h>

namespace Plugins {

template <class ManagedFactoryParam>
class FactoryManager
	{
	/* Embedded classes: */
	public:
	typedef ManagedFactoryParam ManagedFactory; // Base class of factories actually managed

	typedef unsigned short int ClassIdType; // Type to store class IDs
	static const unsigned int maxClassId=65536; // One-past-maximum value for class IDs
	
	struct FactoryManagerError:public std::runtime_error
		{
		/* Constructors and destructors: */
		FactoryManagerError(std::string cause)
			:std::runtime_error(cause)
			{
			}
		};
	
	struct DsoError:public FactoryManagerError // Error type thrown if something goes wrong while processing a DSO
		{
		/* Constructors and destructors: */
		DsoError(std::string cause) // Creates DsoError object from error string returned by dl_* calls
			:FactoryManagerError(std::string("FactoryManager DSO error: ")+cause)
			{
			}
		};
	
	private:
	typedef void (*ResolveDependenciesFunction)(FactoryManager<ManagedFactory>&); // Type of dependency resolution function stored in DSOs
	typedef ManagedFactory* (*CreateFactoryFunction)(FactoryManager<ManagedFactory>&); // Type of class loader function stored in DSOs
	public:
	typedef void (*DestroyFactoryFunction)(ManagedFactory*); // Type of class unloader function stored in DSOs
	
	private:
	struct FactoryData // Structure containing information about a factory
		{
		/* Elements: */
		public:
		ClassIdType classId; // Unique number identifying the factory class
		void* dsoHandle; // Handle for dynamic shared object containing class
		ManagedFactory* factory; // Pointer to factory
		DestroyFactoryFunction destroyFactoryFunction; // Pointer to factory destruction function stored in DSO
		
		/* Constructors and destructors: */
		FactoryData(void* sDsoHandle,ManagedFactory* sFactory,DestroyFactoryFunction sDestroyFactoryFunction)
			:dsoHandle(sDsoHandle),factory(sFactory),destroyFactoryFunction(sDestroyFactoryFunction)
			{
			}
		};
	
	typedef std::list<FactoryData> FactoryList;
	
	public:
	class FactoryIterator
		{
		friend class FactoryManager<ManagedFactoryParam>;
		
		/* Elements: */
		typename FactoryList::iterator it; // Iterator to element of FactoryList
		
		/* Constructors and destructors: */
		private:
		FactoryIterator(const typename FactoryList::iterator& sIt) // Private constructor to convert from standard iterator
			:it(sIt)
			{
			}
		public:
		FactoryIterator(void) // Creates invalid constructor
			{
			}
		
		/* Methods: */
		ManagedFactory& operator*(void) const // Dereference operator
			{
			return *(it->factory);
			}
		ManagedFactory* operator->(void) const // Arrow operator
			{
			return it->factory;
			}
		bool operator==(const FactoryIterator& other) // Equality operator
			{
			return it==other.it;
			}
		bool operator!=(const FactoryIterator& other) // Inequality operator
			{
			return it!=other.it;
			}
		FactoryIterator& operator++(void) // Pre-increment
			{
			++it;
			return *this;
			}
		FactoryIterator operator++(int) // Post-increment
			{
			return FactoryIterator(it++);
			}
		};
	
	class ConstFactoryIterator
		{
		friend class FactoryManager<ManagedFactoryParam>;
		
		/* Elements: */
		typename FactoryList::const_iterator it; // Iterator to element of FactoryList
		
		/* Constructors and destructors: */
		private:
		ConstFactoryIterator(const typename FactoryList::const_iterator& sIt) // Private constructor to convert from standard iterator
			:it(sIt)
			{
			}
		public:
		ConstFactoryIterator(void) // Creates invalid constructor
			{
			}
		
		/* Methods: */
		const ManagedFactory& operator*(void) const // Dereference operator
			{
			return *(it->factory);
			}
		const ManagedFactory* operator->(void) const // Arrow operator
			{
			return it->factory;
			}
		bool operator==(const ConstFactoryIterator& other) // Equality operator
			{
			return it==other.it;
			}
		bool operator!=(const ConstFactoryIterator& other) // Inequality operator
			{
			return it!=other.it;
			}
		ConstFactoryIterator& operator++(void) // Pre-increment
			{
			++it;
			return *this;
			}
		ConstFactoryIterator operator++(int) // Post-increment
			{
			return ConstFactoryIterator(it++);
			}
		};
	
	/* Elements: */
	private:
	std::string dsoNameTemplate; // printf-style format string to create DSO names from class names
	Misc::FileLocator dsoLocator; // File locator to find DSO files
	FactoryList factories; // List of loaded factories
	
	/* Private methods: */
	FactoryData loadClassFromDSO(const char* className); // Loads class of given name from DSO and returns factory pointer
	
	/* Constructors and destructors: */
	public:
	FactoryManager(std::string sDsoNameTemplate); // Creates "empty" manager; initializes DSO locator search path to template's base directory
	~FactoryManager(void); // Releases all loaded object classes and DSOs
	
	/* Methods: */
	const Misc::FileLocator& getDsoLocator(void) const // Returns reference to the DSO file locator
		{
		return dsoLocator;
		}
	Misc::FileLocator& getDsoLocator(void) // Ditto
		{
		return dsoLocator;
		}
	ManagedFactory* loadClass(const char* className); // Loads an object class at runtime and returns class object pointer
	void addClass(ManagedFactory* newFactory,DestroyFactoryFunction newDestroyFactoryFunction =0); // Adds an existing factory to the manager
	void releaseClass(const char* className); // Destroys an object class at runtime; throws exception if class cannot be removed due to dependencies
	void releaseClasses(void); // Releases all loaded classes
	ClassIdType getClassId(const ManagedFactory* factory) const; // Returns class ID based on factory object
	ManagedFactory* getFactory(ClassIdType classId) const; // Returns factory object based on class ID
	ManagedFactory* getFactory(const char* className) const; // Returns factory object based on class name
	ConstFactoryIterator begin(void) const // Returns iterator to the beginning of the managed class list
		{
		return ConstFactoryIterator(factories.begin());
		}
	FactoryIterator begin(void) // Returns iterator to the beginning of the managed class list
		{
		return FactoryIterator(factories.begin());
		}
	ConstFactoryIterator end(void) const // Returns iterator past the end of the managed class list
		{
		return ConstFactoryIterator(factories.end());
		}
	FactoryIterator end(void) // Returns iterator past the end of the managed class list
		{
		return FactoryIterator(factories.end());
		}
	};

}

#ifndef PLUGINS_FACTORYMANAGER_IMPLEMENTATION
#include <Plugins/FactoryManager.icpp>
#endif

#endif
