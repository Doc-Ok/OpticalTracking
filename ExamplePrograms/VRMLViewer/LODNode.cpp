/***********************************************************************
LODNode - Node class to render models at different levels of detail.
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

#include <Misc/ThrowStdErr.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/HVector.h>
#include <Geometry/ProjectiveTransformation.h>
#include <GL/gl.h>
#include <GL/GLTransformationWrappers.h>

#include "Types.h"
#include "Fields/MFFloat.h"

#include "VRMLParser.h"
#include "VRMLRenderState.h"

#include "LODNode.h"

/************************
Methods of class LODNode:
************************/

LODNode::LODNode(VRMLParser& parser)
	:center(Point::origin)
	{
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("LODNode::LODNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	while(!parser.isToken("}"))
		{
		if(parser.isToken("center"))
			{
			/* Parse the center point: */
			parser.getNextToken();
			for(int i=0;i<3;++i)
				{
				center[i]=Point::Scalar(atof(parser.getToken()));
				parser.getNextToken();
				}
			}
		else if(parser.isToken("range"))
			{
			/* Parse the LOD distance ranges: */
			parser.getNextToken();
			distance2s=MFFloat::parse(parser);
			
			/* Compute squared distances, and check for strict increase: */
			float lastDistance=0.0f;
			for(DistanceList::iterator dlIt=distance2s.begin();dlIt!=distance2s.end();++dlIt)
				{
				if(*dlIt<=lastDistance)
					Misc::throwStdErr("LODNode::LODNode: Distances in range attribute not strictly increasing");
				lastDistance=*dlIt;
				*dlIt=Math::sqr(*dlIt);
				}
			}
		else if(parser.isToken("level"))
			{
			/* Parse the node's children: */
			parseChildren(parser);
			}
		else
			Misc::throwStdErr("LODNode::LODNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	
	/* Check if the number of children matches the number of range values: */
	if(children.size()!=distance2s.size()+1)
		Misc::throwStdErr("LODNode::LODNode: mismatching numbers of children and range values");
	}

VRMLNode::Box LODNode::calcBoundingBox(void) const
	{
	/* Calculate the union of the bounding boxes of all children: */
	Box result=Box::empty;
	for(NodeList::const_iterator chIt=children.begin();chIt!=children.end();++chIt)
		result.addBox((*chIt)->calcBoundingBox());
	return result;
	}

void LODNode::glRenderAction(VRMLRenderState& renderState) const
	{
	/* Calculate the distance from the center point to the view point: */
	float distance2=Geometry::sqrDist(center,renderState.viewerPos);
	
	/* Select the correct level-of-detail model: */
	size_t l=0;
	size_t r=distance2s.size();
	while(l<r)
		{
		size_t m=(l+r)>>1;
		if(distance2s[m]<distance2)
			l=m+1;
		else
			r=m;
		}
	
	/* Render the appropriate child nodes: */
	children[l]->glRenderAction(renderState);
	}
