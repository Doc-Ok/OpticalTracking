/***********************************************************************
Route - Class for routes connecting an event source to an event sink of
the same field type.
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

#ifndef SCENEGRAPH_ROUTE_INCLUDED
#define SCENEGRAPH_ROUTE_INCLUDED

#include <stdexcept>

/* Forward declarations: */
namespace SceneGraph {
class EventOut;
class EventIn;
}

namespace SceneGraph {

class Route
	{
	/* Embedded classes: */
	public:
	class TypeMismatchError:public std::runtime_error
		{
		/* Constructors and destructors: */
		public:
		TypeMismatchError(void)
			:std::runtime_error("Route: Event source type does not match event sink type")
			{
			}
		};
	
	/* Constructors and destructors: */
	virtual ~Route(void)
		{
		}
	};

}

#endif
