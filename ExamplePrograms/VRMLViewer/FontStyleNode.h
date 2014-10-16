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

#ifndef FONTSTYLENODE_INCLUDED
#define FONTSTYLENODE_INCLUDED

#include "Types.h"

#include "AttributeNode.h"

/* Forward declarations: */
class GLFont;
class TextNode;

class FontStyleNode:public AttributeNode
	{
	friend class TextNode;
	
	/* Embedded classes: */
	public:
	enum Justification // Enumerated type for string justification
		{
		FIRST,BEGIN,MIDDLE,END
		};
	
	/* Elements: */
	private:
	GLFont* font; // Font object defining the font family, style, and size in model coordinate units
	Float spacing; // Spacing between lines of text in model coordinate units
	Bool horizontal; // Flag to choose between horizontal and vertical font alignment
	Bool leftToRight; // Flag whether to render string left-to-right or right-to-left
	Bool topToBottom; // Flag whether to render string top-to-bottom or bottom-to-top
	Justification justify[2]; // String justification in major and minor directions
	String language; // Language for text strings
	
	/* Constructors and destructors: */
	public:
	FontStyleNode(void); // Creates a default font style node
	FontStyleNode(VRMLParser& parser); // Creates font style node by parsing VRML file
	virtual ~FontStyleNode(void);
	};

#endif
