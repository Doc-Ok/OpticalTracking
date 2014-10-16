/***********************************************************************
Doom3MD5MeshNode - Class for nodes to render Doom3 MD5Mesh animated
models.
Copyright (c) 2010 Oliver Kreylos

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

#include <SceneGraph/Doom3MD5MeshNode.h>

#include <string.h>
#include <SceneGraph/VRMLFile.h>
#include <SceneGraph/GLRenderState.h>
#include <SceneGraph/Internal/Doom3MD5Mesh.h>

namespace SceneGraph {

/*********************************
Methods of class Doom3MD5MeshNode:
*********************************/

Doom3MD5MeshNode::Doom3MD5MeshNode(void)
	:mesh(0)
	{
	}

Doom3MD5MeshNode::~Doom3MD5MeshNode(void)
	{
	delete mesh;
	}

const char* Doom3MD5MeshNode::getStaticClassName(void)
	{
	return "Doom3MD5Mesh";
	}

const char* Doom3MD5MeshNode::getClassName(void) const
	{
	return "Doom3MD5Mesh";
	}

void Doom3MD5MeshNode::parseField(const char* fieldName,VRMLFile& vrmlFile)
	{
	if(strcmp(fieldName,"dataContext")==0)
		{
		vrmlFile.parseSFNode(dataContext);
		}
	else if(strcmp(fieldName,"model")==0)
		{
		vrmlFile.parseField(model);
		}
	else
		GraphNode::parseField(fieldName,vrmlFile);
	}

void Doom3MD5MeshNode::update(void)
	{
	/* Delete the current mesh: */
	delete mesh;
	mesh=0;
	
	try
		{
		/* Load an MD5 mesh of the given name: */
		mesh=new Doom3MD5Mesh(*dataContext.getValue()->getFileManager(),*dataContext.getValue()->getMaterialManager(),model.getValue().c_str());
		
		/* Tell the material manager to load all requested materials: */
		dataContext.getValue()->getMaterialManager()->loadMaterials(*dataContext.getValue()->getFileManager());
		}
	catch(std::runtime_error err)
		{
		/* Just delete the mesh again: */
		delete mesh;
		mesh=0;
		}
	}

Box Doom3MD5MeshNode::calcBoundingBox(void) const
	{
	/* Return the mesh's bounding box: */
	if(mesh!=0)
		return mesh->calcBoundingBox();
	else
		return Box::empty;
	}

void Doom3MD5MeshNode::glRenderAction(GLRenderState& renderState) const
	{
	if(mesh!=0)
		{
		/* Draw the mesh: */
		mesh->drawSurface(renderState.contextData,false);
		}
	}

}
