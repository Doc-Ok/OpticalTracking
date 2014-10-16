/***********************************************************************
EventOut - Base class for event sources.
Copyright (c) 2009 Oliver Kreylos

This file is part of the Simple Scene Graph Renderer (SceneGraph).

The Simple Scene Graph Renderer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Simple Scene Graph Renderer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Simple Scene Graph Renderer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef SCENEGRAPH_EVENTOUT_INCLUDED
#define SCENEGRAPH_EVENTOUT_INCLUDED

/* Forward declarations: */
namespace SceneGraph {
class EventIn;
class Route;
class Node;
}

namespace SceneGraph {

class EventOut
	{
	/* Elements: */
	protected:
	const Node* node; // The node containing the field generating events
	
	/* Constructors and destructors: */
	protected:
	EventOut(const Node* sNode) // Creates an event source for the given node
		:node(sNode)
		{
		}
	public:
	virtual ~EventOut(void) // Destroys the event source
		{
		}
	
	/* Methods: */
	const Node* getNode(void) const // Returns the node
		{
		return node;
		}
	virtual Route* connectTo(const EventIn* sink) const =0; // Returns a route from the event source to the given event sink
	};

}

#endif
