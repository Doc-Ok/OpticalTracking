/***********************************************************************
FontStyleNode - Class for nodes defining the appearance and layout of 3D
text.
Copyright (c) 2009 Oliver Kreylos

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

#include <SceneGraph/FontStyleNode.h>

#include <string.h>
#include <GL/GLFont.h>
#include <SceneGraph/VRMLFile.h>

namespace SceneGraph {

namespace {

/**************
Helper classes:
**************/

enum FontFamily // Enumerated type for font families
	{
	SERIF=0,SANS=1,TYPEWRITER=2
	};

enum FontStyle // Enumerated type for font styles
	{
	PLAIN=0,BOLD=1,ITALIC=2,BOLDITALIC=3
	};

/* Font file names for the possible combinations of families and styles: */
static const char* fontFileNames[3*4]=
	{
	"TimesMediumUpright12","TimesBoldUpright12","TimesMediumItalic12","TimesBoldItalic12",
	"HelveticaMediumUpright12","HelveticaBoldUpright12","HelveticaMediumOblique12","HelveticaBoldOblique12",
	"CourierMediumUpright12","CourierBoldUpright12","CourierMediumOblique12","CourierBoldOblique12"
	};

}

/******************************
Methods of class FontStyleNode:
******************************/

FontStyleNode::FontStyleNode(void)
	:family("SERIF"),
	 style("PLAIN"),
	 language(""),
	 size(Scalar(1)),
	 spacing(Scalar(1)),
	 horizontal(true),
	 leftToRight(true),
	 topToBottom(true)
	{
	}

FontStyleNode::~FontStyleNode(void)
	{
	delete font;
	}

const char* FontStyleNode::getStaticClassName(void)
	{
	return "FontStyle";
	}

const char* FontStyleNode::getClassName(void) const
	{
	return "FontStyle";
	}

void FontStyleNode::parseField(const char* fieldName,VRMLFile& vrmlFile)
	{
	if(strcmp(fieldName,"family")==0)
		{
		vrmlFile.parseField(family);
		}
	else if(strcmp(fieldName,"style")==0)
		{
		vrmlFile.parseField(style);
		}
	else if(strcmp(fieldName,"language")==0)
		{
		vrmlFile.parseField(language);
		}
	else if(strcmp(fieldName,"size")==0)
		{
		vrmlFile.parseField(size);
		}
	else if(strcmp(fieldName,"spacing")==0)
		{
		vrmlFile.parseField(spacing);
		}
	else if(strcmp(fieldName,"justify")==0)
		{
		vrmlFile.parseField(justify);
		}
	else if(strcmp(fieldName,"horizontal")==0)
		{
		vrmlFile.parseField(horizontal);
		}
	else if(strcmp(fieldName,"leftToRight")==0)
		{
		vrmlFile.parseField(leftToRight);
		}
	else if(strcmp(fieldName,"topToBottom")==0)
		{
		vrmlFile.parseField(topToBottom);
		}
	else
		Node::parseField(fieldName,vrmlFile);
	}

void FontStyleNode::update(void)
	{
	/* Extract the font family index: */
	FontFamily fontFamily=SERIF;
	if(family.getValue(0)=="SANS")
		fontFamily=SANS;
	else if(family.getValue(0)=="TYPEWRITER")
		fontFamily=TYPEWRITER;
	
	/* Extract the font style index: */
	FontStyle fontStyle=PLAIN;
	if(style.getValue()=="BOLD")
		fontStyle=BOLD;
	else if(style.getValue()=="ITALIC")
		fontStyle=ITALIC;
	else if(style.getValue()=="BOLDITALIC")
		fontStyle=BOLDITALIC;
	
	/* Load the requested GL font: */
	font=new GLFont(fontFileNames[int(fontFamily)*4+int(fontStyle)]);
	font->setAntialiasing(true);
	
	/* Set the font's size: */
	font->setTextHeight(size.getValue());
	
	/* Parse the justifications: */
	justifications[0]=BEGIN;
	if(justify.getNumValues()>=1)
		{
		if(justify.getValue(0)=="FIRST")
			justifications[0]=FIRST;
		else if(justify.getValue(0)=="MIDDLE")
			justifications[0]=MIDDLE;
		else if(justify.getValue(0)=="END")
			justifications[0]=END;
		}
	justifications[1]=FIRST;
	if(justify.getNumValues()>=2)
		{
		if(justify.getValue(1)=="BEGIN")
			justifications[1]=BEGIN;
		else if(justify.getValue(1)=="MIDDLE")
			justifications[1]=MIDDLE;
		else if(justify.getValue(1)=="END")
			justifications[1]=END;
		}
	}

}
