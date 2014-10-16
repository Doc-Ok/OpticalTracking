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

#ifndef ALTHINGMANAGER_INCLUDED
#define ALTHINGMANAGER_INCLUDED

#include <Threads/Mutex.h>

/* Forward declarations: */
class ALObject;
class ALContextData;

class ALThingManager
	{
	friend class ALContextData;
	
	/* Embedded classes: */
	private:
	struct ThingAction // Structure to store actions related to a thing (an ALObject)
		{
		/* Embedded classes: */
		public:
		enum Action // Enumerated type for possible actions
			{
			INIT,DESTROY
			};
		
		/* Elements: */
		const ALObject* thing; // Thing this action relates to
		Action action; // The action
		ThingAction* succ; // Pointer to the next action in the chain
		};
	
	/* Elements: */
	private:
	static ALThingManager theThingManager; // Static thing manager
	bool active; // Flag if the thing manager accepts actions
	Threads::Mutex newActionMutex; // Mutex protecting the new action list
	ThingAction* firstNewAction; // List of actions added to by users
	ThingAction* lastNewAction; // Pointer to last element in new action list
	ThingAction* firstProcessAction; // List of actions initialized in the current sound cycle
	
	/* Constructors and destructors: */
	public:
	ALThingManager(void);
	~ALThingManager(void);
	
	/* Methods: */
	void shutdown(void); // Shuts down the thing manager
	void initThing(const ALObject* thing); // Marks the given thing for initialization
	void destroyThing(const ALObject* thing); // Marks the given thing for destruction
	void processActions(void); // Moves all new actions to the process list
	void updateThings(ALContextData& contextData) const; // Performs all actions for the current sound cycle
	};

#endif
