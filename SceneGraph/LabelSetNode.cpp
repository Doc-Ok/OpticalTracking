/***********************************************************************
LabelSetNode - Class for nodes to render sets of single-line labels at
individual positions.
Copyright (c) 2009-2013 Oliver Kreylos

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

#include <SceneGraph/LabelSetNode.h>

#include <string.h>
#include <Math/Math.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Rotation.h>
#include <GL/gl.h>
#include <GL/GLTexCoordTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GL/GLContextData.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <SceneGraph/VRMLFile.h>
#include <SceneGraph/GLRenderState.h>

namespace SceneGraph {

/***************************************
Methods of class LabelSetNode::DataItem:
***************************************/

LabelSetNode::DataItem::DataItem(int sNumTextures)
	:numTextures(sNumTextures),
	 textureObjectIds(new GLuint[numTextures])
	{
	glGenTextures(numTextures,textureObjectIds);
	}

LabelSetNode::DataItem::~DataItem(void)
	{
	glDeleteTextures(numTextures,textureObjectIds);
	delete[] textureObjectIds;
	}

/*****************************
Methods of class LabelSetNode:
*****************************/

LabelSetNode::LabelSetNode(void)
	:GLObject(false),
	 maxExtent(Scalar(0)),
	 inited(false)
	{
	}

const char* LabelSetNode::getStaticClassName(void)
	{
	return "LabelSet";
	}

const char* LabelSetNode::getClassName(void) const
	{
	return "LabelSet";
	}

void LabelSetNode::parseField(const char* fieldName,VRMLFile& vrmlFile)
	{
	if(strcmp(fieldName,"string")==0)
		{
		vrmlFile.parseField(string);
		}
	else if(strcmp(fieldName,"coord")==0)
		{
		vrmlFile.parseSFNode(coord);
		}
	else if(strcmp(fieldName,"fontStyle")==0)
		{
		vrmlFile.parseSFNode(fontStyle);
		}
	else if(strcmp(fieldName,"length")==0)
		{
		vrmlFile.parseField(length);
		}
	else if(strcmp(fieldName,"maxExtent")==0)
		{
		vrmlFile.parseField(maxExtent);
		}
	else
		GeometryNode::parseField(fieldName,vrmlFile);
	}

void LabelSetNode::update(void)
	{
	/* Create a default font style node if none was provided: */
	if(fontStyle.getValue()==0)
		{
		fontStyle.setValue(new FontStyleNode);
		fontStyle.getValue()->update();
		}
	FontStyleNode* fs=fontStyle.getValue().getPointer();
	
	/* Lay out the strings: */
	if(fs->horizontal.getValue())
		{
		/* Compute native text boxes for all strings: */
		Scalar maxWidth=Scalar(0);
		for(size_t i=0;i<string.getNumValues();++i)
			{
			/* Get the string's box and texture coordinate box: */
			GLFont::Box sBox=fs->font->calcStringBox(string.getValue(i).c_str());
			
			/* Adjust the width to the given value, if there is one: */
			if(i<length.getNumValues()&&length.getValue(i)>Scalar(0))
				sBox.size[0]=float(length.getValue(i));
			stringBox.push_back(sBox);
			stringTexBox.push_back(fs->font->calcStringTexCoords(string.getValue(i).c_str()));
			
			/* Update the maximum string width: */
			if(maxWidth<Scalar(sBox.size[0]))
				maxWidth=Scalar(sBox.size[0]);
			}
		
		/* Align the strings individually, horizontally and vertically: */
		Scalar horizontalScale=Scalar(1);
		if(maxExtent.getValue()>Scalar(0)&&maxWidth>maxExtent.getValue())
			{
			horizontalScale=maxExtent.getValue()/maxWidth;
			maxWidth=maxExtent.getValue();
			}
		for(size_t i=0;i<string.getNumValues();++i)
			{
			stringBox[i].size[0]*=float(horizontalScale);
			switch(fs->justifications[0])
				{
				case FontStyleNode::FIRST:
				case FontStyleNode::BEGIN:
					stringBox[i].origin[0]=0.0f;
					break;
				
				case FontStyleNode::MIDDLE:
					stringBox[i].origin[0]=-0.5f*stringBox[i].size[0];
					break;
				
				case FontStyleNode::END:
					stringBox[i].origin[0]=-stringBox[i].size[0];
					break;
				}
			
			switch(fs->justifications[1])
				{
				case FontStyleNode::BEGIN:
					stringBox[i].origin[1]=-stringBox[i].size[1];
					break;
				
				case FontStyleNode::MIDDLE:
					stringBox[i].origin[1]=-0.5f*stringBox[i].size[1];
					break;
				
				case FontStyleNode::FIRST:
				case FontStyleNode::END:
					stringBox[i].origin[1]=0.0f;
					break;
				}
			}
		}
	
	/* Register the object with all OpenGL contexts if not done already: */
	if(!inited)
		{
		GLObject::init();
		inited=true;
		}
	}

Box LabelSetNode::calcBoundingBox(void) const
	{
	/* Return the bounding box of the label positions: */
	if(coord.getValue()!=0)
		{
		/* Return the bounding box of the point coordinates: */
		if(pointTransform.getValue()!=0)
			{
			/* Return the bounding box of the transformed point coordinates: */
			return pointTransform.getValue()->calcBoundingBox(coord.getValue()->point.getValues());
			}
		else
			{
			/* Return the bounding box of the untransformed point coordinates: */
			return coord.getValue()->calcBoundingBox();
			}
		}
	else
		return Box::empty;
	}

void LabelSetNode::glRenderAction(GLRenderState& renderState) const
	{
	if(coord.getValue()!=0&&string.getNumValues()>0)
		{
		/* Retrieve the data item from the context: */
		DataItem* dataItem=renderState.contextData.retrieveDataItem<DataItem>(this);
		
		/* Set up OpenGL state: */
		renderState.disableCulling();
		renderState.enableTexture2D();
		
		/* Set up other OpenGL state: */
		glPushAttrib(GL_COLOR_BUFFER_BIT);
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GEQUAL,0.5f);
		
		/* Draw the label strings as texture-mapped quads billboarded at their point positions: */
		const std::vector<Point>& points=coord.getValue()->point.getValues();
		size_t numPoints=points.size();
		for(size_t i=0;i<string.getNumValues()&&i<numPoints;++i)
			{
			/* Calculate the label position: */
			Point labelPos=points[i];
			if(pointTransform.getValue()!=0)
				labelPos=pointTransform.getValue()->transformPoint(labelPos);
			
			/* Align the billboard's Z axis with the viewing direction: */
			Rotation transform=Rotation::rotateFromTo(Vector(0,0,1),renderState.getViewerPos()-labelPos);
			
			/* Rotate the billboard's Y axis into the plane formed by the viewing direction and the up direction: */
			Vector up=transform.inverseTransform(renderState.getUpVector());
			if(up[0]!=Scalar(0)||up[1]!=Scalar(0))
				{
				Scalar angle=Math::atan2(-up[0],up[1]);
				transform*=Rotation::rotateZ(angle);
				}
			
			glPushMatrix();
			glTranslate(labelPos-Point::origin);
			glRotate(transform);
			
			/* Draw the label: */
			glBindTexture(GL_TEXTURE_2D,dataItem->textureObjectIds[i]);
			glBegin(GL_QUADS);
			glNormal3f(0.0f,0.0f,1.0f);
			glTexCoord(stringTexBox[i].getCorner(0));
			glVertex(stringBox[i].getCorner(0));
			glTexCoord(stringTexBox[i].getCorner(1));
			glVertex(stringBox[i].getCorner(1));
			glTexCoord(stringTexBox[i].getCorner(3));
			glVertex(stringBox[i].getCorner(3));
			glTexCoord(stringTexBox[i].getCorner(2));
			glVertex(stringBox[i].getCorner(2));
			glEnd();
			
			glPopMatrix();
			}
		
		/* Protect the texture objects: */
		glBindTexture(GL_TEXTURE_2D,0);
		
		/* Reset OpenGL state: */
		glPopAttrib();
		}
	}

void LabelSetNode::initContext(GLContextData& contextData) const
	{
	/* Create a data item and store it in the context: */
	DataItem* dataItem=new DataItem(string.getNumValues());
	contextData.addDataItem(this,dataItem);
	
	/* Upload texture images for all strings: */
	GLFont::Color background(0.0f,0.0f,0.0f,0.0f);
	GLFont::Color foreground(1.0f,1.0f,1.0f,1.0f);
	for(size_t i=0;i<string.getNumValues();++i)
		{
		glBindTexture(GL_TEXTURE_2D,dataItem->textureObjectIds[i]);
		fontStyle.getValue()->font->uploadStringTexture(string.getValue(i).c_str(),background,foreground);
		}
	
	/* Protect the texture objects: */
	glBindTexture(GL_TEXTURE_2D,0);
	}

}
