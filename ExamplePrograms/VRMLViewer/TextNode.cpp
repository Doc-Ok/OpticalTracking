/***********************************************************************
TextNode - Node class for text shapes, rendered as lines of texture-
mapped thin quads.
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

#include <GL/gl.h>
#include <GL/GLTexCoordTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GL/GLTexEnvTemplates.h>
#include <GL/GLContextData.h>

#include "Types.h"
#include "Fields/SFFloat.h"
#include "Fields/MFFloat.h"
#include "Fields/MFString.h"

#include "VRMLParser.h"
#include "VRMLRenderState.h"
#include "FontStyleNode.h"

#include "TextNode.h"

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

TextNode::TextNode(VRMLParser& parser)
	:maxExtent(0.0f)
	{
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("TextNode::TextNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	while(!parser.isToken("}"))
		{
		if(parser.isToken("string"))
			{
			/* Parse the list of strings: */
			parser.getNextToken();
			string=MFString::parse(parser);
			}
		else if(parser.isToken("fontStyle"))
			{
			/* Parse the font style node: */
			parser.getNextToken();
			fontStyle=parser.getNextNode();
			}
		else if(parser.isToken("length"))
			{
			/* Parse the list of string lengths: */
			parser.getNextToken();
			length=MFFloat::parse(parser);
			}
		else if(parser.isToken("maxExtent"))
			{
			/* Parse the maximum allowed string length: */
			parser.getNextToken();
			maxExtent=SFFloat::parse(parser);
			}
		else
			Misc::throwStdErr("TextNode::TextNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	
	/* Check if a correct font style node was supplied: */
	FontStyleNode* fs=dynamic_cast<FontStyleNode*>(fontStyle.getPointer());
	if(fs==0)
		{
		/* Create a default font style node: */
		fs=new FontStyleNode;
		fontStyle=fs;
		}
	
	/* Lay out the strings: */
	Box::Point bbOrigin;
	Box::Size bbSize;
	if(fs->horizontal)
		{
		/* Compute native text boxes for all strings: */
		float maxWidth=0.0f;
		for(size_t i=0;i<string.size();++i)
			{
			/* Get the string's box and texture coordinate box: */
			GLFont::Box sBox=fs->font->calcStringBox(string[i].c_str());
			
			/* Adjust the width to the given value, if there is one: */
			if(i<length.size()&&length[i]>0.0f)
				sBox.size[0]=length[i];
			stringBox.push_back(sBox);
			stringTexBox.push_back(fs->font->calcStringTexCoords(string[i].c_str()));
			
			/* Update the maximum string width: */
			if(maxWidth<sBox.size[0])
				maxWidth=sBox.size[0];
			}
		
		/* Align the strings horizontally: */
		float horizontalScale=1.0f;
		if(maxExtent>0.0f&&maxWidth>maxExtent)
			{
			horizontalScale=maxExtent/maxWidth;
			maxWidth=maxExtent;
			}
		bbSize[0]=maxWidth;
		switch(fs->justify[0])
			{
			case FontStyleNode::FIRST:
			case FontStyleNode::BEGIN:
				bbOrigin[0]=0.0f;
				break;

			case FontStyleNode::MIDDLE:
				bbOrigin[0]=-0.5*maxWidth;
				break;

			case FontStyleNode::END:
				bbOrigin[0]=-maxWidth;
				break;
			}
		for(size_t i=0;i<string.size();++i)
			{
			stringBox[i].size[0]*=horizontalScale;
			switch(fs->justify[0])
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
		float height=(string.size()-1)*fs->spacing+fs->font->getTextHeight();
		bbSize[1]=height;
		float sp=fs->topToBottom?-fs->spacing:fs->spacing;
		float base;
		switch(fs->justify[1])
			{
			case FontStyleNode::FIRST:
				base=0.0f;
				bbOrigin[1]=fs->topToBottom?-height+fs->font->getTextHeight():0.0f;
				break;
			
			case FontStyleNode::BEGIN:
				base=fs->topToBottom?-fs->font->getTextHeight():0.0f;
				bbOrigin[1]=fs->topToBottom?-height:0.0f;
				break;
			
			case FontStyleNode::MIDDLE:
				base=fs->topToBottom?0.5f*height-fs->font->getTextHeight():-0.5f*height;
				bbOrigin[1]=-0.5f*height;
				break;
			
			case FontStyleNode::END:
				base=fs->topToBottom?height-fs->font->getTextHeight():-height;
				bbOrigin[1]=fs->topToBottom?0.0f:-height;
				break;
			}
		for(size_t i=0;i<string.size();++i)
			stringBox[i].origin[1]=base+float(i)*sp;
		}
	
	/* Store the bounding box: */
	bbOrigin[2]=0.0f;
	bbSize[2]=0.0f;
	boundingBox=Box(bbOrigin,bbSize);
	}

void TextNode::initContext(GLContextData& contextData) const
	{
	/* Create a data item and store it in the context: */
	DataItem* dataItem=new DataItem(string.size());
	contextData.addDataItem(this,dataItem);
	
	/* Upload texture images for all strings: */
	const GLFont* font=dynamic_cast<const FontStyleNode*>(fontStyle.getPointer())->font;
	GLFont::Color background(0.0f,0.0f,0.0f,0.0f);
	GLFont::Color foreground(1.0f,1.0f,1.0f,1.0f);
	for(size_t i=0;i<string.size();++i)
		{
		glBindTexture(GL_TEXTURE_2D,dataItem->textureObjectIds[i]);
		font->uploadStringTexture(string[i].c_str(),background,foreground);
		}
	
	/* Protect the texture objects: */
	glBindTexture(GL_TEXTURE_2D,0);
	}

VRMLNode::Box TextNode::calcBoundingBox(void) const
	{
	return boundingBox;
	}

void TextNode::glRenderAction(VRMLRenderState& renderState) const
	{
	/* Retrieve the data item from the context: */
	DataItem* dataItem=renderState.contextData.retrieveDataItem<DataItem>(this);
	
	/* Set up OpenGL state: */
	glEnable(GL_TEXTURE_2D);
	glTexEnvMode(GLTexEnvEnums::TEXTURE_ENV,GLTexEnvEnums::MODULATE);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GEQUAL,0.5f);
	glDisable(GL_CULL_FACE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
	
	/* Draw the strings as texture-mapped quads: */
	for(size_t i=0;i<string.size();++i)
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
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
	glEnable(GL_CULL_FACE);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);
	}
