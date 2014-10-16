/***********************************************************************
AppearanceNode - Class for appearances of shapes in VRML files.
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

#include "VRMLParser.h"

#include "AppearanceNode.h"

/*******************************
Methods of class AppearanceNode:
*******************************/

AppearanceNode::AppearanceNode(VRMLParser& parser)
	{
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("AppearanceNode::AppearanceNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	while(!parser.isToken("}"))
		{
		if(parser.isToken("material"))
			{
			/* Parse the material node: */
			parser.getNextToken();
			material=parser.getNextNode();
			}
		else if(parser.isToken("texture"))
			{
			/* Parse the texture node: */
			parser.getNextToken();
			texture=parser.getNextNode();
			}
		else if(parser.isToken("textureTransform"))
			{
			/* Parse the texture transformation node: */
			parser.getNextToken();
			textureTransform=parser.getNextNode();
			}
		else
			Misc::throwStdErr("AppearanceNode::AppearanceNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	}

AppearanceNode::~AppearanceNode(void)
	{
	}

void AppearanceNode::setGLState(VRMLRenderState& renderState) const
	{
	if(material!=0)
		material->setGLState(renderState);
	else
		glDisable(GL_LIGHTING);
	if(texture!=0)
		{
		texture->setGLState(renderState);
		if(material!=0)
			{
			glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR);
			glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
			}
		else
			glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
		}
	if(textureTransform!=0)
		textureTransform->setGLState(renderState);
	}

void AppearanceNode::resetGLState(VRMLRenderState& renderState) const
	{
	if(textureTransform!=0)
		textureTransform->resetGLState(renderState);
	if(texture!=0)
		{
		if(material!=0)
			glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SINGLE_COLOR);
		texture->resetGLState(renderState);
		}
	if(material!=0)
		material->resetGLState(renderState);
	else
		glEnable(GL_LIGHTING);
	}
