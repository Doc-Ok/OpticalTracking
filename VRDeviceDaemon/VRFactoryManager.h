/***********************************************************************
VRFactoryManager - Class to manage a set of DSOs containing code for a
set of classes derived from a common base class
Copyright (c) 2002-2010 Oliver Kreylos

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

#ifndef VRFACTORYMANAGER_INCLUDED
#define VRFACTORYMANAGER_INCLUDED

#include <vector>
#include <string>

/* Forward declarations: */
template <class BaseClassParam>
class VRFactory;

template <class BaseClassParam>
class VRFactoryManager
	{
	/* Embedded classes: */
	public:
	typedef BaseClassParam BaseClass; // Type of common base class
	typedef VRFactory<BaseClass> Factory; // Type of managed factory
	typedef std::vector<Factory*> FactoryList; // Type for lists of factories
	
	/* Elements: */
	private:
	const std::string dsoPath; // Common path for DSOs containing specific classes
	FactoryList factories; // List of instantiated factories
	
	/* Constructors and destructors: */
	public:
	VRFactoryManager(const std::string& sDsoPath); // Creates "empty" factory manager
	~VRFactoryManager(void); // Unloads all managed factories
	
	/* Methods: */
	Factory* getFactory(const std::string& className); // Returns pointer to factory for given specific class
	};

#ifndef VRFACTORYMANAGER_IMPLEMENTATION
#include <VRDeviceDaemon/VRFactoryManager.icpp>
#endif

#endif
