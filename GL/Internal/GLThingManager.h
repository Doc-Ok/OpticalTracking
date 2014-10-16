/***********************************************************************
GLThingManager - Class manage initialization and destruction of OpenGL-
related state in cooperation with GLContextData objects.
Copyright (c) 2006-2010 Oliver Kreylos

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

#ifndef GLTHINGMANAGER_INCLUDED
#define GLTHINGMANAGER_INCLUDED

#include <Threads/Mutex.h>

/* Forward declarations: */
class GLObject;
class GLContextData;

class GLThingManager
	{
	friend class GLContextData;
	
	/* Embedded classes: */
	private:
	struct ThingAction // Structure to store actions related to a thing (a GLObject)
		{
		/* Embedded classes: */
		public:
		enum Action // Enumerated type for possible actions
			{
			INIT,DESTROY
			};
		
		/* Elements: */
		const GLObject* thing; // Thing this action relates to
		Action action; // The action
		ThingAction* succ; // Pointer to the next action in the chain
		};
	
	/* Elements: */
	private:
	static GLThingManager theThingManager; // Static thing manager
	bool active; // Flag if the thing manager accepts actions
	Threads::Mutex newActionMutex; // Mutex protecting the new action list
	ThingAction* firstNewAction; // List of actions added to by users
	ThingAction* lastNewAction; // Pointer to last element in new action list
	ThingAction* firstProcessAction; // List of actions initialized in the current render cycle
	
	/* Constructors and destructors: */
	public:
	GLThingManager(void);
	~GLThingManager(void);
	
	/* Methods: */
	void shutdown(void); // Shuts down the thing manager
	void initThing(const GLObject* thing); // Marks the given thing for initialization
	void destroyThing(const GLObject* thing); // Marks the given thing for destruction
	void orderThings(const GLObject* thing1,const GLObject* thing2); // Orders process list such that thing1 is initialized before thing2; assumes both things exist and have not been initialized yet
	void processActions(void); // Moves all new actions to the process list
	void updateThings(GLContextData& contextData) const; // Performs all actions for the current render cycle
	};

#endif
