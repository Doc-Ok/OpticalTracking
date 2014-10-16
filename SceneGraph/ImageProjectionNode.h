/***********************************************************************
ImageProjectionNode - Node to calculate texture coordinates for geometry
vertices based on potentially non-linear image and geometry
transformations.
Copyright (c) 2013 Oliver Kreylos

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

#ifndef SCENEGRAPH_IMAGEPROJECTIONNODE_INCLUDED
#define SCENEGRAPH_IMAGEPROJECTIONNODE_INCLUDED

#include <Misc/Autopointer.h>
#include <Geometry/AffineTransformation.h>
#include <SceneGraph/Geometry.h>
#include <SceneGraph/FieldTypes.h>
#include <SceneGraph/Node.h>
#include <SceneGraph/PointTransformNode.h>

namespace SceneGraph {

class ImageProjectionNode:public Node
	{
	/* Embedded classes: */
	private:
	typedef Geometry::AffineTransformation<PointTransformNode::TScalar,2> ATransform; // Type for affine transformations
	public:
	typedef PointTransformNode::TScalar TScalar; // Scalar type of point transformation classes
	typedef PointTransformNode::TPoint TPoint; // Point type of point transformation classes
	typedef PointTransformNode::MFTScalar MFTScalar; // Multi-value scalar field
	typedef SF<PointTransformNodePointer> SFPointTransformNodePointer;
	
	/* Elements: */
	
	/* Fields: */
	public:
	SFPointTransformNodePointer geometryTransform;
	SFPointTransformNodePointer imageTransform;
	MFTScalar imagePixelTransform;
	SFInt imageWidth;
	SFInt imageHeight;
	SFBool flipVertical;
	
	/* Derived state: */
	ATransform inverseImageTransform; // The inverse of the image pixel transformation created from the row-major image transformation matrix
	
	/* Constructors and destructors: */
	ImageProjectionNode(void); // Creates a default node
	
	/* Methods from Node: */
	static const char* getStaticClassName(void);
	virtual const char* getClassName(void) const;
	virtual void parseField(const char* fieldName,VRMLFile& vrmlFile);
	virtual void update(void);
	
	/* New methods: */
	TexCoord calcTexCoord(const Point& point) const // Calculates the texture coordinate for the given geometry vertex
		{
		TPoint p(point);
		if(geometryTransform.getValue()!=0)
			p=geometryTransform.getValue()->transformPoint(p);
		if(imageTransform.getValue()!=0)
			p=imageTransform.getValue()->inverseTransformPoint(p);
		ATransform::Point image=inverseImageTransform.transform(ATransform::Point(p[0],p[1]));
		return TexCoord(Scalar(image[0]),Scalar(image[1]));
		}
	};

typedef Misc::Autopointer<ImageProjectionNode> ImageProjectionNodePointer;

}

#endif
