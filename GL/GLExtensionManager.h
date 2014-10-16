/***********************************************************************
GLExtensionManager - Class to manage OpenGL extensions.
Copyright (c) 2005-2014 Oliver Kreylos
Mac OS X adaptation copyright (c) 2006 Braden Pellett

This file is part of the OpenGL Support Library (GLSupport).

The OpenGL Support Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The OpenGL Support Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the OpenGL Support Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef GLEXTENSIONMANAGER_INCLUDED
#define GLEXTENSIONMANAGER_INCLUDED

#include <string>
#include <Misc/HashTable.h>
#include <GL/TLSHelper.h>

/* Forward declarations: */
class GLExtension;

class GLExtensionManager
	{
	/* Embedded classes: */
	private:
	typedef void (*FunctionPointer)(void); // Generic function pointer type
	typedef Misc::HashTable<std::string,GLExtension*> ExtensionHash; // Hash table to map extension names to extension objects
	
	/* Elements: */
	private:
	static GL_THREAD_LOCAL(GLExtensionManager*) currentExtensionManager; // Pointer to extension manager for current OpenGL context
	ExtensionHash extensions; // Hash table mapping all extension strings to registered extension objects
	
	/* Private methods: */
	static FunctionPointer getFunctionPtr(const char* functionName); // Returns pointer to an OpenGL extension entry point
	
	/* Constructors and destructors: */
	public:
	GLExtensionManager(void); // Creates an extension manager; managed OpenGL context must be current when called
	~GLExtensionManager(void); // Destroys the extension manager
	
	/* Methods to manage the current extension manager: */
	static GLExtensionManager* getCurrent(void) // Returns the current extension manager
		{
		return currentExtensionManager;
		}
	static void makeCurrent(GLExtensionManager* newCurrentExtensionManager); // Sets the given extension manager as the current one
	
	/* Methods: */
	static bool isExtensionSupported(const char* queryExtensionName); // Returns true if the named extension is supported in the current OpenGL context
	template <class FunctionPointerTypeParam>
	static FunctionPointerTypeParam getFunction(const char* functionName) // Returns pointer to an OpenGL extension entry point
		{
		return FunctionPointerTypeParam(getFunctionPtr(functionName));
		}
	static bool isExtensionRegistered(const char* extensionName); // Returns true if an extension of the given OpenGL name is already registered
	static void registerExtension(GLExtension* newExtension); // Registers the given extension with the extension manager and activates it
	};

#endif
