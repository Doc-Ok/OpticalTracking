/***********************************************************************
ShapeNode - Class for shapes in VRML files.
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

#include "ShapeNode.h"

/**************************
Methods of class ShapeNode:
**************************/

ShapeNode::ShapeNode(VRMLParser& parser)
	{
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("ShapeNode::ShapeNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	while(!parser.isToken("}"))
		{
		if(parser.isToken("appearance"))
			{
			/* Parse the appearance node: */
			parser.getNextToken();
			appearance=parser.getNextNode();
			}
		else if(parser.isToken("geometry"))
			{
			/* Parse the geometry node: */
			parser.getNextToken();
			geometry=parser.getNextNode();
			}
		else
			Misc::throwStdErr("ShapeNode::ShapeNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	}

ShapeNode::~ShapeNode(void)
	{
	}

VRMLNode::Box ShapeNode::calcBoundingBox(void) const
	{
	/* Return the bounding box of the geometry node: */
	if(geometry!=0)
		return geometry->calcBoundingBox();
	else
		return Box::empty;
	}

void ShapeNode::glRenderAction(VRMLRenderState& renderState) const
	{
	if(appearance!=0)
		appearance->setGLState(renderState);
	
	if(geometry!=0)
		{
		/* Render the geometry node: */
		geometry->glRenderAction(renderState);
		}
	
	if(appearance!=0)
		appearance->resetGLState(renderState);
	}
