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

#ifndef GLCONTEXTDATA_INCLUDED
#define GLCONTEXTDATA_INCLUDED

#include <Misc/HashTable.h>
#include <Misc/CallbackData.h>
#include <Misc/CallbackList.h>
#include <GL/TLSHelper.h>
#include <GL/GLObject.h>

/* Forward declarations: */
class GLLightTracker;
class GLClipPlaneTracker;

class GLContextData
	{
	/* Embedded classes: */
	public:
	struct CurrentContextDataChangedCallbackData:public Misc::CallbackData
		{
		/* Elements: */
		public:
		GLContextData* oldContext; // Old context data object
		GLContextData* newContext; // New context data object
		
		/* Constructors and destructors: */
		CurrentContextDataChangedCallbackData(GLContextData* sOldContext,GLContextData* sNewContext)
			:oldContext(sOldContext),newContext(sNewContext)
			{
			}
		};
	
	private:
	typedef Misc::HashTable<const GLObject*,GLObject::DataItem*> ItemHash; // Class for hash table mapping pointers to data items
	
	/* Elements: */
	static Misc::CallbackList currentContextDataChangedCallbacks; // List of callbacks called whenever the current context data object changes
	static GL_THREAD_LOCAL(GLContextData*) currentContextData; // Pointer to the current context data object (associated with the current OpenGL context)
	ItemHash context; // A hash table for the context
	GLLightTracker* lightTracker; // An object to track the OpenGL context's lighting state
	GLClipPlaneTracker* clipPlaneTracker; // An object to track the OpenGL context's clipping plane state
	
	/* Constructors and destructors: */
	public:
	GLContextData(int sTableSize,float sWaterMark =0.9f,float sGrowRate =1.7312543); // Constructs an empty context
	~GLContextData(void);
	
	/* Methods to manage object initializations and clean-ups: */
	static void initThing(const GLObject* thing); // Marks a thing for context initialization
	static void destroyThing(const GLObject* thing); // Marks a thing for context data removal
	static void orderThings(const GLObject* thing1,const GLObject* thing2); // Asks thing manager to always initialize thing1 before thing2
	static void resetThingManager(void); // Resets the thing manager
	static void shutdownThingManager(void); // Shuts down the thing manager
	void updateThings(void); // Initializes or deletes all marked things
	
	/* Methods to manage the current context: */
	static Misc::CallbackList& getCurrentContextDataChangedCallbacks(void) // Returns the list of callbacks called whenever the current context data object changes
		{
		return currentContextDataChangedCallbacks;
		}
	static GLContextData* getCurrent(void) // Returns the current context data object
		{
		return currentContextData;
		}
	static void makeCurrent(GLContextData* newCurrentContextData); // Sets the given context data object as the current one
	
	/* Methods to store/retrieve context data items: */
	bool isRealized(const GLObject* thing) const
		{
		return context.isEntry(thing);
		}
	void addDataItem(const GLObject* thing,GLObject::DataItem* dataItem)
		{
		context.setEntry(ItemHash::Entry(thing,dataItem));
		}
	template <class DataItemParam>
	DataItemParam* retrieveDataItem(const GLObject* thing)
		{
		/* Find the data item associated with the given thing: */
		ItemHash::Iterator dataIt=context.findEntry(thing);
		if(dataIt.isFinished())
			return 0;
		else
			{
			/* Cast the data item's pointer to the requested type and return it: */
			return dynamic_cast<DataItemParam*>(dataIt->getDest());
			}
		}
	void removeDataItem(const GLObject* thing)
		{
		/* Find the data item associated with the given thing: */
		ItemHash::Iterator dataIt=context.findEntry(thing);
		if(!dataIt.isFinished())
			{
			/* Delete the data item (hopefully freeing all resources): */
			delete dataIt->getDest();
			
			/* Remove the data item from the hash table: */
			context.removeEntry(dataIt);
			}
		}
	
	/* Methods to retrieve other context-related state: */
	const GLLightTracker* getLightTracker(void) const // Returns the lighting state tracker
		{
		return lightTracker;
		}
	GLLightTracker* getLightTracker(void) // Ditto
		{
		return lightTracker;
		}
	const GLClipPlaneTracker* getClipPlaneTracker(void) const // Returns the clipping plane state tracker
		{
		return clipPlaneTracker;
		}
	GLClipPlaneTracker* getClipPlaneTracker(void) // Ditto
		{
		return clipPlaneTracker;
		}
	};

#endif
