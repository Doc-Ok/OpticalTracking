/***********************************************************************
ALContextData - Class to store per-AL-context data for application
objects.
Copyright (c) 2006-2009 Oliver Kreylos

This file is part of the OpenAL Support Library (ALSupport).

The OpenAL Support Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The OpenAL Support Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the OpenAL Support Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <AL/ALContextData.h>

#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Rotation.h>
#include <Geometry/OrthogonalTransformation.h>
#include <AL/Internal/ALThingManager.h>

/**************************************
Static elements of class ALContextData:
**************************************/

Misc::CallbackList ALContextData::currentContextDataChangedCallbacks;
ALContextData* ALContextData::currentContextData=0;

/******************************
Methods of class ALContextData:
******************************/

ALContextData::ALContextData(int sTableSize,float sWaterMark,float sGrowRate)
	:context(sTableSize,sWaterMark,sGrowRate),
	 modelviewStackSize(16),modelviewStack(new Transform[modelviewStackSize]),
	 modelview(modelviewStack),
	 lastError(NO_ERROR)
	{
	/* Initialize the modelview matrix stack: */
	*modelview=Transform::identity;
	}

ALContextData::~ALContextData(void)
	{
	/* Delete all data items in this context: */
	for(ItemHash::Iterator it=context.begin();!it.isFinished();++it)
		delete it->getDest();
	
	/* Delete the modelview matrix stack: */
	delete[] modelviewStack;
	}

void ALContextData::initThing(const ALObject* thing)
	{
	ALThingManager::theThingManager.initThing(thing);
	}

void ALContextData::destroyThing(const ALObject* thing)
	{
	ALThingManager::theThingManager.destroyThing(thing);
	}

void ALContextData::resetThingManager(void)
	{
	ALThingManager::theThingManager.processActions();
	}

void ALContextData::shutdownThingManager(void)
	{
	ALThingManager::theThingManager.shutdown();
	}

void ALContextData::updateThings(void)
	{
	ALThingManager::theThingManager.updateThings(*this);
	}

void ALContextData::makeCurrent(ALContextData* newCurrentContextData)
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

void ALContextData::resetMatrixStack(void)
	{
	/* Clear the modelview stack: */
	modelview=modelviewStack;
	
	/* Initialize the modelview matrix: */
	*modelview=Transform::identity;
	}

void ALContextData::pushMatrix(void)
	{
	/* Check if there is room in the stack: */
	if(modelview!=modelviewStack+(modelviewStackSize-1))
		{
		/* Copy the top matrix: */
		modelview[1]=modelview[0];
		
		/* Move up one stack frame: */
		++modelview;
		}
	else
		lastError=STACK_OVERFLOW;
	}

void ALContextData::popMatrix(void)
	{
	/* Check if there is a matrix on the stack: */
	if(modelview!=modelviewStack)
		{
		/* Move down one stack frame: */
		--modelview;
		}
	else
		lastError=STACK_UNDERFLOW;
	}

void ALContextData::loadIdentity(void)
	{
	/* Replace the modelview matrix: */
	*modelview=Transform::identity;
	}

void ALContextData::translate(const ALContextData::Vector& t)
	{
	(*modelview)*=Transform::translate(t);
	}

void ALContextData::rotate(const ALContextData::Rotation& r)
	{
	(*modelview)*=Transform::rotate(r);
	}

void ALContextData::scale(ALContextData::Scalar s)
	{
	(*modelview)*=Transform::scale(s);
	}

void ALContextData::loadMatrix(const ALContextData::Transform& t)
	{
	/* Replace the modelview matrix: */
	*modelview=t;
	}

void ALContextData::multMatrix(const ALContextData::Transform& t)
	{
	/* Modify the modelview matrix: */
	(*modelview)*=t;
	}

ALContextData::Error ALContextData::getError(void)
	{
	Error result=lastError;
	lastError=NO_ERROR;
	return result;
	}
