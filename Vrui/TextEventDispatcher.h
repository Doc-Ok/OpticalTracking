/***********************************************************************
TextEventDispatcher - Class to centralize management and serialization
of GLMotif text and text control events.
Copyright (c) 2014 Oliver Kreylos

This file is part of the Virtual Reality User Interface Library (Vrui).

The Virtual Reality User Interface Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Virtual Reality User Interface Library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality User Interface Library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef VRUI_TEXTEVENTDISPATCHER_INCLUDED
#define VRUI_TEXTEVENTDISPATCHER_INCLUDED

#include <vector>
#include <GLMotif/TextEvent.h>
#include <GLMotif/TextControlEvent.h>

/* Forward declarations: */
namespace IO {
class File;
}
namespace GLMotif {
class WidgetManager;
}

namespace Vrui {

class TextEventDispatcher
	{
	/* Embedded classes: */
	private:
	struct TextEvent // Structure for ordered text events
		{
		/* Elements: */
		public:
		unsigned int ordinal; // Ordinal to order events between the text and text control event lists
		GLMotif::TextEvent event; // GLMotif event structure
		
		/* Constructors and destructors: */
		TextEvent(unsigned int sOrdinal,char sText)
			:ordinal(sOrdinal),event(sText)
			{
			}
		TextEvent(unsigned int sOrdinal,const char* sText)
			:ordinal(sOrdinal),event(sText)
			{
			}
		TextEvent(unsigned int sOrdinal,const GLMotif::TextEvent& sEvent)
			:ordinal(sOrdinal),event(sEvent)
			{
			}
		};
	
	struct TextControlEvent // Structure for ordered text control events
		{
		/* Elements: */
		public:
		unsigned int ordinal; // Ordinal to order events between the text and text control event lists
		GLMotif::TextControlEvent event; // GLMotif event structure
		
		/* Constructors and destructors: */
		TextControlEvent(unsigned int sOrdinal,GLMotif::TextControlEvent::EventType sEvent,bool sSelection)
			:ordinal(sOrdinal),event(sEvent,sSelection)
			{
			}
		TextControlEvent(unsigned int sOrdinal,const GLMotif::TextControlEvent& sEvent)
			:ordinal(sOrdinal),event(sEvent)
			{
			}
		};
	
	/* Elements: */
	bool enqueueLocalEvents; // Flag to disable enqueueing local events, for playback or slave instances
	unsigned int nextEventOrdinal; // Ordinal to assign to the next queued event
	std::vector<TextEvent> textEvents; // List of text events accumulated during frame processing
	std::vector<TextControlEvent> textControlEvents; // List of text control events accumulated during frame processing
	
	/* Constructors and destructors: */
	public:
	TextEventDispatcher(bool sEnqueueLocalEvents); // Creates a text event dispatcher
	
	/* Methods: */
	void text(char text) // Enqueues a single-character text event
		{
		if(enqueueLocalEvents)
			{
			textEvents.push_back(TextEvent(nextEventOrdinal,text));
			++nextEventOrdinal;
			}
		}
	void text(const char* text) // Enqueues a multi-character text event
		{
		if(enqueueLocalEvents)
			{
			textEvents.push_back(TextEvent(nextEventOrdinal,text));
			++nextEventOrdinal;
			}
		}
	void text(const GLMotif::TextEvent& event) // Enqueues a text event structure
		{
		if(enqueueLocalEvents)
			{
			textEvents.push_back(TextEvent(nextEventOrdinal,event));
			++nextEventOrdinal;
			}
		}
	void textControl(GLMotif::TextControlEvent::EventType event,bool selection) // Enqueues a text control event
		{
		if(enqueueLocalEvents)
			{
			textControlEvents.push_back(TextControlEvent(nextEventOrdinal,event,selection));
			++nextEventOrdinal;
			}
		}
	void textControl(const GLMotif::TextControlEvent& event) // Enqueues a text control event structure
		{
		if(enqueueLocalEvents)
			{
			textControlEvents.push_back(TextControlEvent(nextEventOrdinal,event));
			++nextEventOrdinal;
			}
		}
	void writeEventQueues(IO::File& file) const; // Writes the current event queues to the given file
	void readEventQueues(IO::File& file); // Enqueues all events previously written to the given file
	void dispatchEvents(GLMotif::WidgetManager& widgetManager); // Dispatches all enqueued events to the given GLMotif widget manager and re-initializes the queues
	};

}

#endif
