/***********************************************************************
LoadModelFromLWOFile - Function to load static (non-animated) models
from files in LightWave Object (LWO) format.
Copyright (c) 2007-2013 Oliver Kreylos

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

#ifndef SCENEGRAPH_INTERNAL_LOADMODELFROMLWOFILE_INCLUDED
#define SCENEGRAPH_INTERNAL_LOADMODELFROMLWOFILE_INCLUDED

/* Forward declarations: */
namespace SceneGraph {
class Doom3FileManager;
class Doom3MaterialManager;
class Doom3Model;
}

namespace SceneGraph {

Doom3Model* loadModelFromLWOFile(Doom3FileManager& fileManager,Doom3MaterialManager& materialManager,const char* aseFileName);

}

#endif
