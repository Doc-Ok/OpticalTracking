/***********************************************************************
MaterialNode - Class for surface materials in VRML files.
Copyright (c) 2006-2008 Oliver Kreylos

This file is part of the Virtual Reality VRML viewer (VRMLViewer).

The Virtual Reality VRML viewer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Virtual Reality VRML viewer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality VRML viewer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <GL/gl.h>
#include <GL/GLColorTemplates.h>

#include "VRMLParser.h"

#include "MaterialNode.h"

/*****************************
Methods of class MaterialNode:
*****************************/

MaterialNode::MaterialNode(VRMLParser& parser)
	{
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("MaterialNode::MaterialNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	while(!parser.isToken("}"))
		{
		if(parser.isToken("diffuseColor"))
			{
			/* Parse the diffuse color: */
			parser.getNextToken();
			for(int i=0;i<3;++i)
				{
				material.diffuse[i]=GLMaterial::Scalar(atof(parser.getToken()));
				parser.getNextToken();
				}
			}
		else if(parser.isToken("ambientIntensity"))
			{
			/* Parse the ambient intensity: */
			parser.getNextToken();
			GLMaterial::Scalar ambientIntensity=GLMaterial::Scalar(atof(parser.getToken()));
			parser.getNextToken();
			for(int i=0;i<3;++i)
				material.ambient[i]=material.diffuse[i]*ambientIntensity;
			}
		else if(parser.isToken("specularColor"))
			{
			/* Parse the specular color: */
			parser.getNextToken();
			for(int i=0;i<3;++i)
				{
				material.specular[i]=GLMaterial::Scalar(atof(parser.getToken()));
				parser.getNextToken();
				}
			}
		else if(parser.isToken("shininess"))
			{
			/* Parse the shininess: */
			parser.getNextToken();
			material.shininess=GLMaterial::Scalar(atof(parser.getToken()))*GLMaterial::Scalar(128);
			parser.getNextToken();
			}
		else if(parser.isToken("transparency"))
			{
			/* Parse the transparency: */
			parser.getNextToken();
			material.diffuse[3]=GLMaterial::Scalar(1)-GLMaterial::Scalar(atof(parser.getToken()));
			parser.getNextToken();
			}
		else if(parser.isToken("emissiveColor"))
			{
			/* Parse the emissive color: */
			parser.getNextToken();
			for(int i=0;i<3;++i)
				{
				material.emission[i]=GLMaterial::Scalar(atof(parser.getToken()));
				parser.getNextToken();
				}
			}
		else
			Misc::throwStdErr("MaterialNode::MaterialNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	}

void MaterialNode::setGLState(VRMLRenderState& renderState) const
	{
	/* Set the OpenGL material properties: */
	glEnable(GL_LIGHTING);
	glMaterial(GLMaterialEnums::FRONT_AND_BACK,material);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
	glColor(material.diffuse);
	}

void MaterialNode::resetGLState(VRMLRenderState& renderState) const
	{
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
	}
