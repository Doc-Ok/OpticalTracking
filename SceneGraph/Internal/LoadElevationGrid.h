/***********************************************************************
LoadElevationGrid - Function to load an elevation grid's height values
from an external file.
Copyright (c) 2010-2012 Oliver Kreylos

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

#ifndef SCENEGRAPH_INTERNAL_LOADELEVATIONGRID_INCLUDED
#define SCENEGRAPH_INTERNAL_LOADELEVATIONGRID_INCLUDED

#include <SceneGraph/FieldTypes.h>

/* Forward declarations: */
namespace Cluster {
class Multiplexer;
}
namespace SceneGraph {
class ElevationGridNode;
}

namespace SceneGraph {

void loadElevationGrid(ElevationGridNode& node,Cluster::Multiplexer* multiplexer);

}

#endif
