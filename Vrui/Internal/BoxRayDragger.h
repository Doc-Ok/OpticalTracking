/***********************************************************************
BoxRayDragger - A helper class to encapsulate dragging a wireframe
box with a ray-based input device.
Copyright (c) 2004-2013 Oliver Kreylos

This file is part of the Virtual Reality User Interface Library (Vrui).

The Virtual Reality User Interface Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Virtual Reality User Interface Library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality User Interface Library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef VRUI_INTERNAL_BOXRAYDRAGGER_INCLUDED
#define VRUI_INTERNAL_BOXRAYDRAGGER_INCLUDED

#include <utility>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Vrui/Geometry.h>

namespace Vrui {

class BoxRayDragger
	{
	/* Embedded classes: */
	public:
	enum DragMode // Enumerated type for dragging modes supported by a box dragger
		{
		DRAG_NONE,DRAG_VERTEX,DRAG_EDGE,DRAG_FACE
		};
	
	private:
	typedef std::pair<Scalar,Scalar> Interval; // Type to return intersection results
	
	/* Elements: */
	Scalar boxSize; // Overall size of the dragged box; determines size of edges and vertices as well
	Scalar rotateFactor; // Number of units to drag to rotate by one radians
	
	/* Transient state: */
	ONTransform initialTransformation; // Transformation of the box at the moment dragging started
	DragMode dragMode;
	Vector dragPlaneNormal;
	Scalar dragPlaneOffset;
	Point initialPoint;
	Point rotateCenter;
	Vector rotateAxis;
	Vector rotateDragDirection;
	ONTransform dragTransformation;
	ONTransform currentTransformation; // Current transformation of the box during dragging
	
	/* Private methods: */
	Interval intersectBox(const Ray& ray,const Point& center,const Scalar halfSize[3]);
	
	/* Constructors and destructors: */
	public:
	BoxRayDragger(Scalar sBoxSize,Scalar sRotateFactor);
	
	/* Methods: */
	bool pick(const ONTransform& newTransformation,const Ray& ray,const Vector& viewPlaneNormal);
	void drag(const Ray& ray);
	const ONTransform& getDragTransformation(void) const
		{
		return dragTransformation;
		}
	const ONTransform& getCurrentTransformation(void) const
		{
		return currentTransformation;
		}
	void release(void);
	};

}

#endif
