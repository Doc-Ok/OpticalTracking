/***********************************************************************
AppearanceNode - Class defining the appearance (material properties,
textures) of a shape node.
Copyright (c) 2009-2010 Oliver Kreylos

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

#ifndef SCENEGRAPH_APPEARANCENODE_INCLUDED
#define SCENEGRAPH_APPEARANCENODE_INCLUDED

#include <Misc/Autopointer.h>
#include <SceneGraph/FieldTypes.h>
#include <SceneGraph/AttributeNode.h>
#include <SceneGraph/MaterialNode.h>
#include <SceneGraph/TextureNode.h>
#include <SceneGraph/TransformNode.h>

namespace SceneGraph {

class AppearanceNode:public AttributeNode
	{
	/* Embedded classes: */
	public:
	typedef SF<MaterialNodePointer> SFMaterialNode;
	typedef SF<TextureNodePointer> SFTextureNode;
	typedef SF<TransformNodePointer> SFTransformNode;
	
	/* Elements: */
	
	/* Fields: */
	public:
	SFMaterialNode material;
	SFTextureNode texture;
	SFTransformNode textureTransform;
	
	/* Constructors and destructors: */
	public:
	AppearanceNode(void); // Creates a default appearance node
	
	/* Methods from Node: */
	static const char* getStaticClassName(void);
	virtual const char* getClassName(void) const;
	virtual EventOut* getEventOut(const char* fieldName) const;
	virtual EventIn* getEventIn(const char* fieldName);
	virtual void parseField(const char* fieldName,VRMLFile& vrmlFile);
	virtual void update(void);
	
	/* Methods from AttributeNode: */
	virtual void setGLState(GLRenderState& renderState) const;
	virtual void resetGLState(GLRenderState& renderState) const;
	};

typedef Misc::Autopointer<AppearanceNode> AppearanceNodePointer;

}

#endif
