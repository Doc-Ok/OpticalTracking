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

#include <GL/GLExtensionManager.h>

#include <string.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/StringHashFunctions.h>
#include <GL/gl.h>
#ifndef GLX_GLXEXT_PROTOTYPES
#define GLX_GLXEXT_PROTOTYPES 1
#endif
#include <GL/Config.h>
#if !GLSUPPORT_CONFIG_HAVE_GLXGETPROCADDRESS
#include <dlfcn.h>
#endif
#include <GL/glx.h>
#include <GL/Extensions/GLExtension.h>

#if 0
#if GLSUPPORT_CONFIG_HAVE_GLXGETPROCADDRESS
#ifndef GLX_ARB_get_proc_address
#define GLX_ARB_get_proc_address 1
typedef void (*__GLXextFuncPtr)(void);
extern __GLXextFuncPtr glXGetProcAddressARB(const GLubyte*);
#endif
#endif
#endif

/*******************************************
Static elements of class GLExtensionManager:
*******************************************/

GL_THREAD_LOCAL(GLExtensionManager*) GLExtensionManager::currentExtensionManager=0;

/***********************************
Methods of class GLExtensionManager:
***********************************/

GLExtensionManager::FunctionPointer GLExtensionManager::getFunctionPtr(const char* functionName)
	{
	#if GLSUPPORT_CONFIG_HAVE_GLXGETPROCADDRESS
	return glXGetProcAddressARB(reinterpret_cast<const GLubyte*>(functionName));
	#else
	/* Mac OS X's GLX does not support glXGetProcAddress, strangely enough: */
	FunctionPointer result;
	*reinterpret_cast<void**>(&result)=dlsym(RTLD_DEFAULT,functionName);
	return result;
	#endif
	}

GLExtensionManager::GLExtensionManager(void)
	:extensions(31)
	{
	/* Query the OpenGL extension string: */
	const char* extensionNames=(const char*)glGetString(GL_EXTENSIONS);
	
	/* Bail out if there is no current OpenGL context, or no extensions: */
	if(extensionNames==0)
		return;
	
	/* Enter all extension names into a hash table: */
	const char* start=extensionNames;
	while(*start!='\0')
		{
		/* Find the end of the current extension string: */
		const char* end;
		for(end=start;*end!='\0'&&*end!=' ';++end)
			;
		
		/* Store the extension string with a null extension pointer: */
		extensions.setEntry(ExtensionHash::Entry(std::string(start,end),0));
		
		/* Go to the next extension: */
		while(*end==' ')
			++end;
		start=end;
		}
	}

GLExtensionManager::~GLExtensionManager(void)
	{
	/* Destroy all registered extensions: */
	for(ExtensionHash::Iterator eIt=extensions.begin();!eIt.isFinished();++eIt)
		delete eIt->getDest();
	}

void GLExtensionManager::makeCurrent(GLExtensionManager* newCurrentExtensionManager)
	{
	if(newCurrentExtensionManager!=currentExtensionManager)
		{
		if(currentExtensionManager!=0)
			{
			/* Deactivate all extensions in the current extension manager: */
			for(ExtensionHash::Iterator eIt=currentExtensionManager->extensions.begin();!eIt.isFinished();++eIt)
				if(eIt->getDest()!=0)
					eIt->getDest()->deactivate();
			}
		
		/* Set the new current extension manager: */
		currentExtensionManager=newCurrentExtensionManager;
		
		if(currentExtensionManager!=0)
			{
			/* Activate all extensions in the current extension manager: */
			for(ExtensionHash::Iterator eIt=currentExtensionManager->extensions.begin();!eIt.isFinished();++eIt)
				if(eIt->getDest()!=0)
					eIt->getDest()->activate();
			}
		}
	}

bool GLExtensionManager::isExtensionSupported(const char* queryExtensionName)
	{
	/* Check if the extension name exists in the hash table: */
	return currentExtensionManager->extensions.isEntry(queryExtensionName);
	}

bool GLExtensionManager::isExtensionRegistered(const char* extensionName)
	{
	/* Search the extension name in the list of registered extensions: */
	ExtensionHash::Iterator eIt=currentExtensionManager->extensions.findEntry(extensionName);
	
	/* Throw an exception if the extension is not supported by the current OpenGL context: */
	if(eIt.isFinished())
		Misc::throwStdErr("GLExtensionManager: Extension %s not supported by local OpenGL",extensionName);
	
	/* Return true if the extension already has an associated extension object: */
	return eIt->getDest()!=0;
	}

void GLExtensionManager::registerExtension(GLExtension* newExtension)
	{
	/* Search the extension name in the list of registered extensions: */
	ExtensionHash::Iterator eIt=currentExtensionManager->extensions.findEntry(newExtension->getExtensionName());
	
	/* Check if the extension exists and is not yet registered: */
	if(!eIt.isFinished()&&eIt->getDest()==0)
		{
		/* Register and activate the extension: */
		eIt->getDest()=newExtension;
		newExtension->activate();
		}
	else
		{
		/* Delete the superfluous extension object: */
		delete newExtension;
		}
	}
