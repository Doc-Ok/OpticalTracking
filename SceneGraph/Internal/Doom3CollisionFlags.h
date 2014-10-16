/***********************************************************************
Doom3CollisionFlags - Enumerated type for collision flags associated
with materials and collision brushes in Doom3 maps.
Copyright (c) 2007-2010 Oliver Kreylos

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

#ifndef SCENEGRAPH_INTERNAL_DOOM3COLLISIONFLAGS_INCLUDED
#define SCENEGRAPH_INTERNAL_DOOM3COLLISIONFLAGS_INCLUDED

namespace SceneGraph {

enum Doom3CollisionFlags
	{
	CF_SOLID=0x1,
	CF_OPAQUE=0x2,
	CF_PLAYERCLIP=0x4,
	CF_MONSTERCLIP=0x8,
	CF_MOVEABLECLIP=0x10,
	CF_IKCLIP=0x20
	};

}

#endif
