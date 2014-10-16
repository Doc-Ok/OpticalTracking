/***********************************************************************
TextureTransformNode - Class for texture coordinate transformations.
Copyright (c) 2008 Oliver Kreylos

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

#include <Math/Math.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>

#include "Types.h"
#include "Fields/SFFloat.h"
#include "Fields/SFVec2f.h"

#include "VRMLParser.h"

#include "TextureTransformNode.h"

/*************************************
Methods of class TextureTransformNode:
*************************************/

TextureTransformNode::TextureTransformNode(VRMLParser& parser)
	:center(0.0f,0.0f),
	 rotation(0.0f),
	 scale(1.0f,1.0f),
	 translation(0.0f,0.0f)
	{
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("TextureTransformNode::TextureTransformNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	while(!parser.isToken("}"))
		{
		if(parser.isToken("center"))
			{
			parser.getNextToken();
			center=SFVec2f::parse(parser);
			}
		else if(parser.isToken("rotation"))
			{
			parser.getNextToken();
			rotation=SFFloat::parse(parser);
			}
		else if(parser.isToken("scale"))
			{
			parser.getNextToken();
			scale=SFVec2f::parse(parser);
			}
		else if(parser.isToken("translation"))
			{
			parser.getNextToken();
			translation=SFVec2f::parse(parser);
			}
		else
			Misc::throwStdErr("TextureTransformNode::TextureTransformNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	}

void TextureTransformNode::setGLState(VRMLRenderState& renderState) const
	{
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(translation[0],translation[1],0.0f);
	glTranslatef(center[0],center[1],0.0f);
	glRotatef(Math::deg(rotation),0.0f,0.0f,1.0f);
	glScalef(scale[0],scale[1],1.0f);
	glTranslatef(-center[0],-center[1],0.0f);
	glMatrixMode(GL_MODELVIEW);
	}

void TextureTransformNode::resetGLState(VRMLRenderState& renderState) const
	{
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	}
