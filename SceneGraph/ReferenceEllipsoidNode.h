/***********************************************************************
ReferenceEllipsoidNode - Class for nodes defining reference ellipsoid
(geodesic data) for geodesic coordinate systems and transformations
between them.
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

#ifndef SCENEGRAPH_REFERENCEELLIPSOIDNODE_INCLUDED
#define SCENEGRAPH_REFERENCEELLIPSOIDNODE_INCLUDED

#include <Misc/Autopointer.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Rotation.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Geometry/Geoid.h>
#include <SceneGraph/Geometry.h>
#include <SceneGraph/FieldTypes.h>
#include <SceneGraph/Node.h>

namespace SceneGraph {

class ReferenceEllipsoidNode:public Node
	{
	/* Embedded classes: */
	public:
	typedef SF<double> SFDouble;
	typedef Geometry::Geoid<double> Geoid; // Low-level representation for reference ellipsoids
	
	/* Elements: */
	
	/* Fields: */
	public:
	SFDouble radius;
	SFDouble flattening;
	SFDouble scale;
	
	/* Derived state: */
	protected:
	Geoid re; // Low-level reference ellipsoid
	
	/* Constructors and destructors: */
	public:
	ReferenceEllipsoidNode(void); // Creates reference ellipsoid node with default settings (WGS84 in km)
	
	/* Methods from Node: */
	static const char* getStaticClassName(void);
	virtual const char* getClassName(void) const;
	virtual void parseField(const char* fieldName,VRMLFile& vrmlFile);
	virtual void update(void);
	
	/* New methods: */
	const Geoid& getRE(void) const // Returns the low-level reference ellipsoid
		{
		return re;
		}
	};

typedef Misc::Autopointer<ReferenceEllipsoidNode> ReferenceEllipsoidNodePointer;

}

#endif
