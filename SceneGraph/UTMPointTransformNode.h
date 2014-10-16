/***********************************************************************
UTMPointTransformNode - Point transformation class to
convert Universal Transverse Mercator coordinates on a reference
ellipsoid to geodetic (longitude/latitude) coordinates on the same
ellipsoid.
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

#ifndef SCENEGRAPH_UTMPOINTTRANSFORMNODE_INCLUDED
#define SCENEGRAPH_UTMPOINTTRANSFORMNODE_INCLUDED

#include <Geometry/UTMProjection.h>
#include <SceneGraph/PointTransformNode.h>
#include <SceneGraph/ReferenceEllipsoidNode.h>

namespace SceneGraph {

class UTMPointTransformNode:public PointTransformNode
	{
	/* Embedded classes: */
	public:
	typedef SF<ReferenceEllipsoidNodePointer> SFReferenceEllipsoidNode;
	
	/* Elements: */
	
	/* Fields: */
	public:
	SFReferenceEllipsoidNode referenceEllipsoid;
	SFInt zone; // UTM zone
	SFTScalar scaleFactor; // Scaling factor at central meridian
	SFTScalar falseNorthing;
	SFTScalar falseEasting;
	SFBool degrees;
	
	/* Derived state: */
	protected:
	Geometry::UTMProjection<TScalar> projection; // The low-level projection object
	
	/* Constructors and destructors: */
	public:
	UTMPointTransformNode(void); // Creates a default node
	
	/* Methods from Node: */
	static const char* getStaticClassName(void);
	virtual const char* getClassName(void) const;
	virtual void parseField(const char* fieldName,VRMLFile& vrmlFile);
	virtual void update(void);
	
	/* Methods from PointTransformNode: */
	virtual TPoint transformPoint(const TPoint& point) const;
	virtual TPoint inverseTransformPoint(const TPoint& point) const;
	virtual TBox calcBoundingBox(const std::vector<Point>& points) const;
	virtual TBox transformBox(const TBox& box) const;
	virtual TVector transformNormal(const TPoint& basePoint,const TVector& normal) const;
	};

}

#endif
