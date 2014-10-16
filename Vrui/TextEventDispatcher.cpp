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

#include <Vrui/TextEventDispatcher.h>

#include <Misc/SizedTypes.h>
#include <Misc/VarInt.h>
#include <IO/File.h>
#include <GLMotif/WidgetManager.h>

namespace Vrui {

/************************************
Methods of class TextEventDispatcher:
************************************/

TextEventDispatcher::TextEventDispatcher(bool sEnqueueLocalEvents)
	:enqueueLocalEvents(sEnqueueLocalEvents),
	 nextEventOrdinal(0)
	{
	}

void TextEventDispatcher::writeEventQueues(IO::File& file) const
	{
	/* Write the number of enqueued text events: */
	Misc::writeVarInt(Misc::UInt32(textEvents.size()),file);
	
	/* Write all enqueued text events: */
	for(std::vector<TextEvent>::const_iterator teIt=textEvents.begin();teIt!=textEvents.end();++teIt)
		{
		Misc::writeVarInt(Misc::UInt32(teIt->ordinal),file);
		Misc::writeVarInt(Misc::UInt32(teIt->event.getTextLength()),file);
		file.write(teIt->event.getText(),teIt->event.getTextLength());
		}
	
	/* Write the number of enqueued text control events: */
	Misc::writeVarInt(Misc::UInt32(textControlEvents.size()),file);
	
	/* Write all enqueued text control events: */
	for(std::vector<TextControlEvent>::const_iterator tceIt=textControlEvents.begin();tceIt!=textControlEvents.end();++tceIt)
		{
		Misc::writeVarInt(Misc::UInt32(tceIt->ordinal),file);
		file.write(Misc::UInt8(tceIt->event.event));
		file.write(Misc::UInt8(tceIt->event.selection?1U:0U));
		}
	}

void TextEventDispatcher::readEventQueues(IO::File& file)
	{
	unsigned int newNextEventOrdinal=nextEventOrdinal;
	
	/* Read the number of saved text events: */
	unsigned int numTextEvents=Misc::readVarInt(file);
	
	/* Read and enqueue all saved text events: */
	size_t stringBufferLen=31;
	char* stringBuffer=new char[stringBufferLen+1];
	for(unsigned int i=0;i<numTextEvents;++i)
		{
		unsigned int ordinal=Misc::readVarInt(file)+nextEventOrdinal;
		if(newNextEventOrdinal<ordinal+1)
			newNextEventOrdinal=ordinal+1;
		unsigned int stringLen=Misc::readVarInt(file);
		if(stringBufferLen<stringLen)
			{
			delete[] stringBuffer;
			stringBufferLen=stringLen;
			stringBuffer=new char[stringBufferLen+1];
			}
		file.read(stringBuffer,stringLen);
		stringBuffer[stringLen]='\0';
		
		textEvents.push_back(TextEvent(ordinal,stringBuffer));
		}
	delete[] stringBuffer;
	
	/* Read the number of saved text control events: */
	unsigned int numTextControlEvents=Misc::readVarInt(file);
	
	/* Read and enqueue all saved text control events: */
	for(unsigned int i=0;i<numTextControlEvents;++i)
		{
		unsigned int ordinal=Misc::readVarInt(file)+nextEventOrdinal;
		if(newNextEventOrdinal<ordinal+1)
			newNextEventOrdinal=ordinal+1;
		GLMotif::TextControlEvent::EventType event=GLMotif::TextControlEvent::EventType(file.read<Misc::UInt8>());
		bool selection=file.read<Misc::UInt8>()!=0U;
		
		textControlEvents.push_back(TextControlEvent(ordinal,event,selection));
		}
	
	nextEventOrdinal=newNextEventOrdinal;
	}

void TextEventDispatcher::dispatchEvents(GLMotif::WidgetManager& widgetManager)
	{
	/* Merge the queues of text and text control events by ordinal number: */
	std::vector<TextEvent>::iterator teIt=textEvents.begin();
	unsigned int teOrd=teIt!=textEvents.end()?teIt->ordinal:nextEventOrdinal;
	std::vector<TextControlEvent>::iterator tceIt=textControlEvents.begin();
	unsigned int tceOrd=tceIt!=textControlEvents.end()?tceIt->ordinal:nextEventOrdinal;
	while(teIt!=textEvents.end()||tceIt!=textControlEvents.end())
		{
		/* Dispatch the next event from either list: */
		if(teOrd<tceOrd)
			{
			/* Dispatch the next text event: */
			widgetManager.text(teIt->event);
			++teIt;
			teOrd=teIt!=textEvents.end()?teIt->ordinal:nextEventOrdinal;
			}
		else
			{
			/* Dispatch the next text control event: */
			widgetManager.textControl(tceIt->event);
			++tceIt;
			tceOrd=tceIt!=textControlEvents.end()?tceIt->ordinal:nextEventOrdinal;
			}
		}
	
	/* Clear the event queues: */
	nextEventOrdinal=0U;
	textEvents.clear();
	textControlEvents.clear();
	}

}
