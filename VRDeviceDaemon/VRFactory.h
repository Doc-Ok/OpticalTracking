/***********************************************************************
VRFactory - Class to create objects of a specific class class derived
from a common base class.
Copyright (c) 2002-2014 Oliver Kreylos

This file is part of the Vrui VR Device Driver Daemon (VRDeviceDaemon).

The Vrui VR Device Driver Daemon is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Vrui VR Device Driver Daemon is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Vrui VR Device Driver Daemon; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef VRFACTORY_INCLUDED
#define VRFACTORY_INCLUDED

#include <string>

/* Forward declarations: */
namespace Misc {
class ConfigurationFile;
}
template <class BaseClassParam>
class VRFactoryManager;

template <class BaseClassParam>
class VRFactory
	{
	/* Embedded classes: */
	public:
	typedef BaseClassParam BaseClass; // Type of common base class
	private:
	typedef VRFactoryManager<BaseClass> FactoryManager;
	typedef void (*CreateClassFuncType)(VRFactory*,FactoryManager*);
	typedef BaseClass* (*CreateObjectFuncType)(VRFactory*,FactoryManager*,Misc::ConfigurationFile&);
	typedef void (*DestroyObjectFuncType)(BaseClass*,VRFactory*,FactoryManager*);
	typedef void (*DestroyClassFuncType)(VRFactory*,FactoryManager*);
	
	/* Elements: */
	FactoryManager* factoryManager; // Pointer to manager for this factory
	std::string className; // Name of specific class created by this factory
	void* dsoHandle; // Handle of dynamic shared object containing code for objects of specific class
	CreateClassFuncType createClassFunc; // Function to initialize specific class
	CreateObjectFuncType createObjectFunc; // Function to create objects of specific class
	DestroyObjectFuncType destroyObjectFunc; // Function to destroy objects of specific class
	DestroyClassFuncType destroyClassFunc; // Function to deinitialize specific class
	
	/* Constructors and destructors: */
	public:
	VRFactory(FactoryManager* sFactoryManager,const std::string& sClassName,const std::string& dsoPath); // Loads a DSO containing code for given specific class
	~VRFactory(void); // Unloads DSO
	
	/* Methods: */
	const std::string& getClassName(void) const // Returns name of specific class
		{
		return className;
		};
	BaseClass* createObject(Misc::ConfigurationFile& configFile) // Creates object of specific type from current section of configuration file
		{
		return createObjectFunc(this,factoryManager,configFile);
		};
	void destroyObject(BaseClass* object) // Destroys object of specific type
		{
		destroyObjectFunc(object,this,factoryManager);
		};
	};

#ifndef VRFACTORY_IMPLEMENTATION
#include <VRDeviceDaemon/VRFactory.icpp>
#endif

#endif
