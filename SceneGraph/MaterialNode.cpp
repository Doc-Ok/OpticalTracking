/***********************************************************************
MaterialNode - Class for attribute nodes defining Phong material
properties.
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

#include <SceneGraph/MaterialNode.h>

#include <string.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLColorOperations.h>
#include <GL/GLMaterialEnums.h>
#include <SceneGraph/VRMLFile.h>
#include <SceneGraph/GLRenderState.h>

namespace SceneGraph {

/*****************************
Methods of class MaterialNode:
*****************************/

MaterialNode::MaterialNode(void)
	:ambientIntensity(Scalar(0.2)),
	 diffuseColor(Color(0.8f,0.8f,0.8f)),
	 specularColor(Color(0.0f,0.0f,0.0f)),
	 shininess(Scalar(0.2)),
	 emissiveColor(Color(0.0f,0.0f,0.0f)),
	 transparency(Scalar(0))
	{
	}

const char* MaterialNode::getStaticClassName(void)
	{
	return "Material";
	}

const char* MaterialNode::getClassName(void) const
	{
	return "Material";
	}

void MaterialNode::parseField(const char* fieldName,VRMLFile& vrmlFile)
	{
	if(strcmp(fieldName,"ambientIntensity")==0)
		{
		vrmlFile.parseField(ambientIntensity);
		}
	else if(strcmp(fieldName,"diffuseColor")==0)
		{
		vrmlFile.parseField(diffuseColor);
		}
	else if(strcmp(fieldName,"specularColor")==0)
		{
		vrmlFile.parseField(specularColor);
		}
	else if(strcmp(fieldName,"shininess")==0)
		{
		vrmlFile.parseField(shininess);
		}
	else if(strcmp(fieldName,"emissiveColor")==0)
		{
		vrmlFile.parseField(emissiveColor);
		}
	else if(strcmp(fieldName,"transparency")==0)
		{
		vrmlFile.parseField(transparency);
		}
	else
		AttributeNode::parseField(fieldName,vrmlFile);
	}

void MaterialNode::update(void)
	{
	material.diffuse=diffuseColor.getValue();
	material.ambient=material.diffuse;
	material.ambient*=ambientIntensity.getValue();
	material.specular=specularColor.getValue();
	material.shininess=shininess.getValue()*128.0f;
	material.emission=emissiveColor.getValue();
	}

void MaterialNode::setGLState(GLRenderState& renderState) const
	{
	/* Enable material rendering: */
	renderState.enableMaterials();
	
	/* Set the material properties: */
	glMaterial(GLMaterialEnums::FRONT_AND_BACK,material);
	renderState.emissiveColor=material.emission;
	glColor(material.diffuse);
	}

void MaterialNode::resetGLState(GLRenderState& state) const
	{
	/* Don't do anything here; next guy cleans up */
	}

}
