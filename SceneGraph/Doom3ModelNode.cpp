/***********************************************************************
Doom3ModelNode - Class for nodes to render static models using Doom3's
lighting model.
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

#include <SceneGraph/Doom3ModelNode.h>

#include <string.h>
#include <Misc/FileNameExtensions.h>
#include <SceneGraph/VRMLFile.h>
#include <SceneGraph/GLRenderState.h>
#include <SceneGraph/Internal/Doom3MaterialManager.h>
#include <SceneGraph/Internal/Doom3Model.h>
#include <SceneGraph/Internal/LoadModelFromASEFile.h>
#include <SceneGraph/Internal/LoadModelFromLWOFile.h>

namespace SceneGraph {

/*******************************
Methods of class Doom3ModelNode:
*******************************/

Doom3ModelNode::Doom3ModelNode(void)
	:mesh(0)
	{
	}

Doom3ModelNode::~Doom3ModelNode(void)
	{
	delete mesh;
	}

const char* Doom3ModelNode::getStaticClassName(void)
	{
	return "Doom3Model";
	}

const char* Doom3ModelNode::getClassName(void) const
	{
	return "Doom3Model";
	}

void Doom3ModelNode::parseField(const char* fieldName,VRMLFile& vrmlFile)
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

void Doom3ModelNode::update(void)
	{
	/* Delete the current model: */
	delete mesh;
	mesh=0;
	
	/* Determine the model's format: */
	const char* fileExt=Misc::getExtension(model.getValue().c_str());
	
	try
		{
		/* Load the model using the appropriate format parser: */
		if(strcasecmp(fileExt,".lwo")==0)
			mesh=loadModelFromLWOFile(*dataContext.getValue()->getFileManager(),*dataContext.getValue()->getMaterialManager(),model.getValue().c_str());
		else if(strcasecmp(fileExt,".ase")==0)
			mesh=loadModelFromASEFile(*dataContext.getValue()->getFileManager(),*dataContext.getValue()->getMaterialManager(),model.getValue().c_str());
		
		/* Tell the material manager to load all requested materials: */
		dataContext.getValue()->getMaterialManager()->loadMaterials(*dataContext.getValue()->getFileManager());
		}
	catch(std::runtime_error err)
		{
		/* Just delete the model again: */
		delete mesh;
		mesh=0;
		}
	}

Box Doom3ModelNode::calcBoundingBox(void) const
	{
	/* Return the model's bounding box: */
	if(mesh!=0)
		return mesh->getBoundingBox();
	else
		return Box::empty;
	}

void Doom3ModelNode::glRenderAction(GLRenderState& renderState) const
	{
	if(mesh!=0)
		{
		/* Set polygon orientation to clockwise: */
		glPushAttrib(GL_POLYGON_BIT);
		glFrontFace(GL_CW);
		
		/* Initialize the material manager: */
		Doom3MaterialManager::RenderContext mmRc=dataContext.getValue()->getMaterialManager()->start(renderState.contextData,false);
		
		/* Draw the model: */
		mesh->glRenderAction(renderState.contextData,mmRc);
		
		/* Shut down the material manager and restore OpenGL state: */
		dataContext.getValue()->getMaterialManager()->finish(mmRc);
		glPopAttrib();
		}
	}

}
