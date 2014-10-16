/***********************************************************************
ALThingManager - Class manage initialization and destruction of OpenAL-
related state in cooperation with ALContextData objects.
Copyright (c) 2008-2010 Oliver Kreylos

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

#include <AL/Internal/ALThingManager.h>

#include <AL/ALObject.h>
#include <AL/ALContextData.h>

/***************************************
Static elements of class ALThingManager:
***************************************/

ALThingManager ALThingManager::theThingManager;

/*******************************
Methods of class ALThingManager:
*******************************/

ALThingManager::ALThingManager(void)
	:active(true),
	 firstNewAction(0),lastNewAction(0),
	 firstProcessAction(0)
	{
	}

ALThingManager::~ALThingManager(void)
	{
	/* Delete all actions: */
	while(firstProcessAction!=0)
		{
		ThingAction* succ=firstProcessAction->succ;
		delete firstProcessAction;
		firstProcessAction=succ;
		}
	{
	Threads::Mutex::Lock newActionLock(newActionMutex);
	active=false;
	while(firstNewAction!=0)
		{
		ThingAction* succ=firstNewAction->succ;
		delete firstNewAction;
		firstNewAction=succ;
		}
	}
	}

void ALThingManager::shutdown(void)
	{
	/* Delete all pending actions: */
	while(firstProcessAction!=0)
		{
		ThingAction* succ=firstProcessAction->succ;
		delete firstProcessAction;
		firstProcessAction=succ;
		}
	
	/* Mark the thing manager as inactive: */
	{
	Threads::Mutex::Lock newActionLock(newActionMutex);
	active=false;
	while(firstNewAction!=0)
		{
		ThingAction* succ=firstNewAction->succ;
		delete firstNewAction;
		firstNewAction=succ;
		}
	}
	}

void ALThingManager::initThing(const ALObject* thing)
	{
	{
	Threads::Mutex::Lock newActionLock(newActionMutex);
	if(active)
		{
		/* Append the new thing action to the new action list: */
		ThingAction* newAction=new ThingAction;
		newAction->thing=thing;
		newAction->action=ThingAction::INIT;
		newAction->succ=0;
		if(lastNewAction!=0)
			lastNewAction->succ=newAction;
		else
			firstNewAction=newAction;
		lastNewAction=newAction;
		}
	}
	}

void ALThingManager::destroyThing(const ALObject* thing)
	{
	Threads::Mutex::Lock newActionLock(newActionMutex);
	if(active)
		{
		/* Search if the thing has a pending initialization action: */
		ThingAction* taPtr1=0;
		ThingAction* taPtr2;
		for(taPtr2=firstNewAction;taPtr2!=0;taPtr1=taPtr2,taPtr2=taPtr2->succ)
			if(taPtr2->thing==thing&&taPtr2->action==ThingAction::INIT)
				break;
		
		if(taPtr2!=0)
			{
			/* Thing has pending initialization; remove it: */
			if(taPtr1!=0)
				taPtr1->succ=taPtr2->succ;
			else
				firstNewAction=taPtr2->succ;
			if(taPtr2->succ==0)
				lastNewAction=taPtr1;
			delete taPtr2;
			}
		else
			{
			/* Append a destruction action to the list: */
			ThingAction* newAction=new ThingAction;
			newAction->thing=thing;
			newAction->action=ThingAction::DESTROY;
			newAction->succ=0;
			if(lastNewAction!=0)
				lastNewAction->succ=newAction;
			else
				firstNewAction=newAction;
			lastNewAction=newAction;
			}
		}
	}

void ALThingManager::processActions(void)
	{
	/* Delete the old process list: */
	while(firstProcessAction!=0)
		{
		ThingAction* succ=firstProcessAction->succ;
		delete firstProcessAction;
		firstProcessAction=succ;
		}
	
	/* Move the new action list to the process list: */
	{
	Threads::Mutex::Lock newActionLock(newActionMutex);
	firstProcessAction=firstNewAction;
	firstNewAction=0;
	lastNewAction=0;
	}
	}

void ALThingManager::updateThings(ALContextData& contextData) const
	{
	/* Perform all actions on the process list in order: */
	for(const ThingAction* taPtr=firstProcessAction;taPtr!=0;taPtr=taPtr->succ)
		{
		if(taPtr->action==ThingAction::INIT)
			{
			/* Call the thing's context initialization routine: */
			taPtr->thing->initContext(contextData);
			}
		else
			{
			/* Delete the context data item associated with the thing: */
			contextData.removeDataItem(taPtr->thing);
			}
		}
	}
