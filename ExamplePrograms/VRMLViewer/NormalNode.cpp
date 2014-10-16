/***********************************************************************
NormalNode - Class for arrays of vertex normal vectors.
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

#include "VRMLParser.h"

#include "NormalNode.h"

/***************************
Methods of class NormalNode:
***************************/

NormalNode::NormalNode(void)
	{
	}

NormalNode::NormalNode(VRMLParser& parser)
	{
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("NormalNode::NormalNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	while(!parser.isToken("}"))
		{
		if(parser.isToken("vector"))
			{
			/* Parse the vector array: */
			
			/* Check for the opening bracket: */
			parser.getNextToken();
			if(!parser.isToken("["))
				Misc::throwStdErr("NormalNode::NormalNode: Missing opening bracket in vector attribute");
			parser.getNextToken();
			
			/* Parse vectors until closing bracket: */
			while(!parser.isToken("]"))
				{
				/* Parse the next vector: */
				Vector v=Vector::zero;
				for(int i=0;i<3&&!parser.isToken("]");++i)
					{
					v[i]=Vector::Scalar(atof(parser.getToken()));
					parser.getNextToken();
					}
				vectors.push_back(v);
				}
			
			/* Skip the closing bracket: */
			parser.getNextToken();
			}
		else
			Misc::throwStdErr("NormalNode::NormalNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	
	}

void NormalNode::glRenderAction(VRMLRenderState& renderState) const
	{
	}
