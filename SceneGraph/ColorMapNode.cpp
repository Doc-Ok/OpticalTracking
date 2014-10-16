/***********************************************************************
ColorMapNode - Class for nodes that define mappings from scalar values
to RGB colors.
Copyright (c) 2012 Oliver Kreylos

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

#include <SceneGraph/ColorMapNode.h>

#include <string.h>
#include <SceneGraph/EventTypes.h>
#include <SceneGraph/VRMLFile.h>

namespace SceneGraph {

/*****************************
Methods of class ColorMapNode:
*****************************/

ColorMapNode::ColorMapNode(void)
	{
	}

const char* ColorMapNode::getStaticClassName(void)
	{
	return "ColorMap";
	}

const char* ColorMapNode::getClassName(void) const
	{
	return "ColorMap";
	}

EventOut* ColorMapNode::getEventOut(const char* fieldName) const
	{
	if(strcmp(fieldName,"scalar")==0)
		return makeEventOut(this,scalar);
	else if(strcmp(fieldName,"color")==0)
		return makeEventOut(this,color);
	else
		return Node::getEventOut(fieldName);
	}

EventIn* ColorMapNode::getEventIn(const char* fieldName)
	{
	if(strcmp(fieldName,"scalar")==0)
		return makeEventIn(this,scalar);
	else if(strcmp(fieldName,"color")==0)
		return makeEventIn(this,color);
	else
		return Node::getEventIn(fieldName);
	}

void ColorMapNode::parseField(const char* fieldName,VRMLFile& vrmlFile)
	{
	if(strcmp(fieldName,"scalar")==0)
		vrmlFile.parseField(scalar);
	else if(strcmp(fieldName,"color")==0)
		vrmlFile.parseField(color);
	else
		Node::parseField(fieldName,vrmlFile);
	}

void ColorMapNode::update(void)
	{
	/* Calculate the number of color map entries: */
	numMapEntries=scalar.getNumValues();
	if(numMapEntries>color.getNumValues())
		numMapEntries=color.getNumValues();
	}

Color ColorMapNode::mapColor(Scalar mapScalar) const
	{
	/* Find the scalar interval containing the given scalar value: */
	if(mapScalar<=scalar.getValue(0))
		return color.getValue(0);
	else if(mapScalar>=scalar.getValue(numMapEntries-1))
		return color.getValue(numMapEntries-1);
	else
		{
		size_t l=0;
		size_t r=numMapEntries;
		while(r-l>1)
			{
			size_t m=(l+r)>>1;
			if(scalar.getValue(m)<=mapScalar)
				l=m;
			else
				r=m;
			}
		
		/* Interpolate between the found interval's two color values: */
		GLfloat w0=GLfloat((scalar.getValue(r)-mapScalar)/(scalar.getValue(r)-scalar.getValue(l)));
		GLfloat w1=1.0f-w0;
		Color result;
		for(int i=0;i<3;++i)
			result[i]=color.getValue(l)[i]*w0+color.getValue(r)[i]*w1;
		return result;
		}
	}

}
