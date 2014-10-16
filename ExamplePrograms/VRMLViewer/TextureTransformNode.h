/***********************************************************************
TextureTransformNode - Class for texture coordinate transformations.
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

#ifndef TEXTURETRANSFORMNODE_INCLUDED
#define TEXTURETRANSFORMNODE_INCLUDED

#include "Types.h"

#include "AttributeNode.h"

class TextureTransformNode:public AttributeNode
	{
	/* Elements: */
	private:
	Vec2f center; // Center point for scaling and rotation
	Float rotation; // Rotation angle
	Vec2f scale; // Scaling factors
	Vec2f translation; // Translation
	
	/* Constructors and destructors: */
	public:
	TextureTransformNode(VRMLParser& parser);
	
	/* Methods: */
	virtual void setGLState(VRMLRenderState& renderState) const;
	virtual void resetGLState(VRMLRenderState& renderState) const;
	};

#endif
