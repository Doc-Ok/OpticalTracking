/***********************************************************************
FontStyleNode - Class for fonts and text styles in VRML files.
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

#include <vector>
#include <GL/gl.h>
#include <GL/GLFont.h>

#include "Types.h"
#include "Fields/SFBool.h"
#include "Fields/SFFloat.h"
#include "Fields/SFString.h"
#include "Fields/MFString.h"

#include "VRMLParser.h"

#include "FontStyleNode.h"

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
	:horizontal(true),leftToRight(true),topToBottom(true),
	 language("")
	{
	justify[0]=BEGIN;
	justify[1]=FIRST;
	
	/* Load the requested GL font: */
	font=new GLFont(fontFileNames[int(SERIF)*4+int(PLAIN)]);
	font->setAntialiasing(true);
	
	/* Set the font's size and compute the model-coordinate spacing: */
	font->setTextHeight(1.0f);
	spacing=font->getTextHeight();
	}

FontStyleNode::FontStyleNode(VRMLParser& parser)
	:horizontal(true),leftToRight(true),topToBottom(true),
	 language("")
	{
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("FontStyleNode::FontStyleNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Initialize default settings: */
	FontFamily fontFamily=SERIF;
	FontStyle fontStyle=PLAIN;
	Float size=1.0f;
	Float relSpacing=1.0f;
	justify[0]=BEGIN;
	justify[1]=FIRST;
	
	/* Process attributes until closing brace: */
	while(!parser.isToken("}"))
		{
		if(parser.isToken("family"))
			{
			/* Parse the font family: */
			parser.getNextToken();
			if(parser.isToken("SERIF")||parser.isToken(""))
				fontFamily=SERIF;
			else if(parser.isToken("SANS"))
				fontFamily=SANS;
			else if(parser.isToken("TYPEWRITER"))
				fontFamily=TYPEWRITER;
			else
				Misc::throwStdErr("FontStyleNode::FontStyleNode: unknown font family \"%s\" in node definition",parser.getToken());
			parser.getNextToken();
			}
		else if(parser.isToken("style"))
			{
			/* Parse the font style: */
			parser.getNextToken();
			if(parser.isToken("PLAIN")||parser.isToken(""))
				fontStyle=PLAIN;
			else if(parser.isToken("BOLD"))
				fontStyle=BOLD;
			else if(parser.isToken("ITALIC"))
				fontStyle=ITALIC;
			else if(parser.isToken("BOLDITALIC"))
				fontStyle=BOLDITALIC;
			else
				Misc::throwStdErr("FontStyleNode::FontStyleNode: unknown font style \"%s\" in node definition",parser.getToken());
			parser.getNextToken();
			}
		else if(parser.isToken("size"))
			{
			/* Parse the font size: */
			parser.getNextToken();
			size=SFFloat::parse(parser);
			}
		else if(parser.isToken("spacing"))
			{
			/* Parse the (relative) line spacing: */
			parser.getNextToken();
			relSpacing=SFFloat::parse(parser);
			}
		else if(parser.isToken("horizontal"))
			{
			/* Parse the horizontal flag: */
			parser.getNextToken();
			horizontal=SFBool::parse(parser);
			}
		else if(parser.isToken("leftToRight"))
			{
			/* Parse the major layout direction flag: */
			parser.getNextToken();
			leftToRight=SFBool::parse(parser);
			}
		else if(parser.isToken("topToBottom"))
			{
			/* Parse the minor layout direction flag: */
			parser.getNextToken();
			topToBottom=SFBool::parse(parser);
			}
		else if(parser.isToken("justify"))
			{
			/* Parse the major and minor justifications: */
			parser.getNextToken();
			std::vector<String> justification=MFString::parse(parser);
			for(int i=0;i<2&&i<justification.size();++i)
				{
				if(justification[i]=="")
					justify[i]=i==0?FIRST:BEGIN;
				else if(justification[i]=="FIRST")
					justify[i]=FIRST;
				else if(justification[i]=="BEGIN")
					justify[i]=BEGIN;
				else if(justification[i]=="MIDDLE")
					justify[i]=MIDDLE;
				else if(justification[i]=="END")
					justify[i]=END;
				else
					Misc::throwStdErr("FontStyleNode::FontStyleNode: unknown %s text justification \"%s\" in node definition",i==0?"major":"minor",justification[i].c_str());
				}
			}
		else if(parser.isToken("language"))
			{
			/* Parse the text language: */
			parser.getNextToken();
			language=SFString::parse(parser);
			}
		else
			Misc::throwStdErr("FontStyleNode::FontStyleNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	
	/* Load the requested GL font: */
	font=new GLFont(fontFileNames[int(fontFamily)*4+int(fontStyle)]);
	font->setAntialiasing(true);
	
	/* Set the font's size and compute the model-coordinate spacing: */
	font->setTextHeight(size);
	spacing=relSpacing*font->getTextHeight();
	}

FontStyleNode::~FontStyleNode(void)
	{
	delete font;
	}
