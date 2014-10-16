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

#ifndef SCENEGRAPH_FONTSTYLENODE_INCLUDED
#define SCENEGRAPH_FONTSTYLENODE_INCLUDED

#include <Misc/Autopointer.h>
#include <SceneGraph/FieldTypes.h>
#include <SceneGraph/Node.h>

/* Forward declarations: */
class GLFont;
namespace SceneGraph {
class TextNode;
class LabelSetNode;
}

namespace SceneGraph {

class FontStyleNode:public Node
	{
	friend class TextNode;
	friend class LabelSetNode;
	
	/* Embedded classes: */
	protected:
	enum Justification // Enumerated type for string justification
		{
		FIRST,BEGIN,MIDDLE,END
		};
	
	/* Elements: */
	
	/* Fields: */
	public:
	MFString family;
	SFString style;
	SFString language;
	SFFloat size;
	SFFloat spacing;
	MFString justify;
	SFBool horizontal;
	SFBool leftToRight;
	SFBool topToBottom;
	
	/* Derived state: */
	protected:
	GLFont* font; // The GL font object used to render text strings
	Justification justifications[2]; // Justification in major and minor directions
	
	/* Constructors and destructors: */
	public:
	FontStyleNode(void); // Creates a font style node with default properties
	virtual ~FontStyleNode(void); // Destroys the font style
	
	/* Methods from Node: */
	static const char* getStaticClassName(void);
	virtual const char* getClassName(void) const;
	virtual void parseField(const char* fieldName,VRMLFile& vrmlFile);
	virtual void update(void);
	
	/* New methods: */
	const GLFont* getFont(void) const // Returns the GL font object
		{
		return font;
		}
	};

typedef Misc::Autopointer<FontStyleNode> FontStyleNodePointer;

}

#endif
