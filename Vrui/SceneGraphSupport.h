/***********************************************************************
SceneGraphSupport - Helper functions to simplify adding scene graphs to
Vrui applications.
Copyright (c) 2013 Oliver Kreylos

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

#ifndef VRUI_SCENEGRAPHSUPPORT_INCLUDED
#define VRUI_SCENEGRAPHSUPPORT_INCLUDED

#include <Vrui/Geometry.h>

/* Forward declarations: */
class GLContextData;
namespace SceneGraph {
class GLRenderState;
class GraphNode;
}

namespace Vrui {

/* These functions return new-allocated SceneGraph::GLRenderState objects: */

SceneGraph::GLRenderState* createRenderState(bool navigational,GLContextData& contextData); // Creates a scene graph render state starting in physical or navigational coordinates
SceneGraph::GLRenderState* createRenderState(const NavTransform& transform,bool navigational,GLContextData& contextData); // Creates a scene graph render state starting with the given transformation relative to physical or navigational coordinates

/* These functions render the given scene graph: */

void renderSceneGraph(const SceneGraph::GraphNode* root,bool navigational,GLContextData& contextData); // Renders the given scene graph in physical or navigational coordinates
void renderSceneGraph(const SceneGraph::GraphNode* root,const NavTransform& transform,bool navigational,GLContextData& contextData); // Renders the given scene graph with the given transformation relative to physical or navigational coordinates

}

#endif
