/***********************************************************************
GLContextData - Class to store per-GL-context data for application
objects.
Copyright (c) 2000-2012 Oliver Kreylos

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

#include <GL/GLContextData.h>

#include <GL/GLLightTracker.h>
#include <GL/GLClipPlaneTracker.h>
#include <GL/Internal/GLThingManager.h>

/**************************************
Static elements of class GLContextData:
**************************************/

Misc::CallbackList GLContextData::currentContextDataChangedCallbacks;
GL_THREAD_LOCAL(GLContextData*) GLContextData::currentContextData=0;

/******************************
Methods of class GLContextData:
******************************/

GLContextData::GLContextData(int sTableSize,float sWaterMark,float sGrowRate)
	:context(sTableSize,sWaterMark,sGrowRate),
	 lightTracker(new GLLightTracker),
	 clipPlaneTracker(new GLClipPlaneTracker)
	{
	}

GLContextData::~GLContextData(void)
	{
	/* Delete all data items in this context: */
	for(ItemHash::Iterator it=context.begin();!it.isFinished();++it)
		delete it->getDest();
	
	/* Delete the state trackers: */
	delete lightTracker;
	delete clipPlaneTracker;
	}

void GLContextData::initThing(const GLObject* thing)
	{
	GLThingManager::theThingManager.initThing(thing);
	}

void GLContextData::destroyThing(const GLObject* thing)
	{
	GLThingManager::theThingManager.destroyThing(thing);
	}

void GLContextData::orderThings(const GLObject* thing1,const GLObject* thing2)
	{
	GLThingManager::theThingManager.orderThings(thing1,thing2);
	}

void GLContextData::resetThingManager(void)
	{
	GLThingManager::theThingManager.processActions();
	}

void GLContextData::shutdownThingManager(void)
	{
	GLThingManager::theThingManager.shutdown();
	}

void GLContextData::updateThings(void)
	{
	GLThingManager::theThingManager.updateThings(*this);
	}

void GLContextData::makeCurrent(GLContextData* newCurrentContextData)
	{
	if(newCurrentContextData!=currentContextData)
		{
		/* Create the callback data object: */
		CurrentContextDataChangedCallbackData cbData(currentContextData,newCurrentContextData);
		
		/* Set the current context data object: */
		currentContextData=newCurrentContextData;
		
		/* Call all callbacks: */
		currentContextDataChangedCallbacks.call(&cbData);
		}
	}
