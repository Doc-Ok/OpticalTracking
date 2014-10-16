/***********************************************************************
TextNode - Class for nodes to render 3D text.
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

#include <SceneGraph/TextNode.h>

#include <string.h>
#include <GL/gl.h>
#include <GL/GLTexCoordTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GL/GLContextData.h>
#include <SceneGraph/VRMLFile.h>
#include <SceneGraph/GLRenderState.h>

namespace SceneGraph {

/***********************************
Methods of class TextNode::DataItem:
***********************************/

TextNode::DataItem::DataItem(int sNumTextures)
	:numTextures(sNumTextures),
	 textureObjectIds(new GLuint[numTextures])
	{
	glGenTextures(numTextures,textureObjectIds);
	}

TextNode::DataItem::~DataItem(void)
	{
	glDeleteTextures(numTextures,textureObjectIds);
	delete[] textureObjectIds;
	}

/*************************
Methods of class TextNode:
*************************/

TextNode::TextNode(void)
	:GLObject(false),
	 maxExtent(Scalar(0)),
	 inited(false)
	{
	}

const char* TextNode::getStaticClassName(void)
	{
	return "Text";
	}

const char* TextNode::getClassName(void) const
	{
	return "Text";
	}

void TextNode::parseField(const char* fieldName,VRMLFile& vrmlFile)
	{
	if(strcmp(fieldName,"string")==0)
		{
		vrmlFile.parseField(string);
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

void TextNode::update(void)
	{
	/* Create a default font style node if none was provided: */
	if(fontStyle.getValue()==0)
		{
		fontStyle.setValue(new FontStyleNode);
		fontStyle.getValue()->update();
		}
	FontStyleNode* fs=fontStyle.getValue().getPointer();
	
	/* Lay out the strings: */
	Point bbOrigin;
	Size bbSize;
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
		
		/* Align the strings horizontally: */
		Scalar horizontalScale=Scalar(1);
		if(maxExtent.getValue()>Scalar(0)&&maxWidth>maxExtent.getValue())
			{
			horizontalScale=maxExtent.getValue()/maxWidth;
			maxWidth=maxExtent.getValue();
			}
		bbSize[0]=maxWidth;
		switch(fs->justifications[0])
			{
			case FontStyleNode::FIRST:
			case FontStyleNode::BEGIN:
				bbOrigin[0]=Scalar(0);
				break;

			case FontStyleNode::MIDDLE:
				bbOrigin[0]=-Scalar(0.5)*maxWidth;
				break;

			case FontStyleNode::END:
				bbOrigin[0]=-maxWidth;
				break;
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
			}
		
		/* Align the strings vertically: */
		Scalar size=fs->size.getValue();
		Scalar spacing=size*fs->spacing.getValue();
		Scalar height=Scalar(string.getNumValues()-1)*spacing+size;
		bbSize[1]=height;
		Scalar sp=fs->topToBottom.getValue()?-spacing:spacing;
		Scalar base=Scalar(0);
		switch(fs->justifications[1])
			{
			case FontStyleNode::FIRST:
				bbOrigin[1]=fs->topToBottom.getValue()?-height+size:Scalar(0);
				break;
			
			case FontStyleNode::BEGIN:
				base=fs->topToBottom.getValue()?-size:Scalar(0);
				bbOrigin[1]=fs->topToBottom.getValue()?-height:Scalar(0);
				break;
			
			case FontStyleNode::MIDDLE:
				base=fs->topToBottom.getValue()?Scalar(0.5)*height-size:-Scalar(0.5)*height;
				bbOrigin[1]=-Scalar(0.5)*height;
				break;
			
			case FontStyleNode::END:
				base=fs->topToBottom.getValue()?height-size:-height;
				bbOrigin[1]=fs->topToBottom.getValue()?Scalar(0):-height;
				break;
			}
		for(size_t i=0;i<string.getNumValues();++i)
			stringBox[i].origin[1]=float(base+Scalar(i)*sp);
		}
	
	/* Store the bounding box: */
	bbOrigin[2]=Scalar(0);
	bbSize[2]=Scalar(0);
	boundingBox=Box(bbOrigin,bbSize);
	
	/* Register the object with all OpenGL contexts if not done already: */
	if(!inited)
		{
		GLObject::init();
		inited=true;
		}
	}

Box TextNode::calcBoundingBox(void) const
	{
	return boundingBox;
	}

void TextNode::glRenderAction(GLRenderState& renderState) const
	{
	if(string.getNumValues()>0)
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
		
		/* Draw the strings as texture-mapped quads: */
		for(size_t i=0;i<string.getNumValues();++i)
			{
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
			}
		
		/* Protect the texture objects: */
		glBindTexture(GL_TEXTURE_2D,0);
		
		/* Reset OpenGL state: */
		glPopAttrib();
		}
	}

void TextNode::initContext(GLContextData& contextData) const
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
