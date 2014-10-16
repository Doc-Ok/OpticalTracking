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

#include <Vrui/Internal/BoxRayDragger.h>

#include <Math/Constants.h>
#include <Geometry/Ray.h>

namespace Vrui {

/******************************
Methods of class BoxRayDragger:
******************************/

BoxRayDragger::Interval BoxRayDragger::intersectBox(const Ray& ray,const Point& center,const Scalar halfSize[3])
	{
	/* Initialize the intersection interval to the full ray range: */
	Scalar l1=Scalar(0);
	Scalar l2=Math::Constants<Scalar>::max;
	
	/* Calculate the intersection interval of the ray with each box face and intersect the intervals: */
	for(int i=0;i<3;++i)
		{
		/* Calculate top and bottom faces: */
		Scalar top=center[i]+halfSize[i];
		Scalar bot=center[i]-halfSize[i];
		
		/* Calculate the ray intersections along this axis: */
		Scalar lf1,lf2;
		
		/* Check the direction of the ray relatively to the box: */
		if(ray.getDirection()[i]<Scalar(0))
			{
			/* Ray intersects top face first: */
			lf1=(top-ray.getOrigin()[i])/ray.getDirection()[i];
			lf2=(bot-ray.getOrigin()[i])/ray.getDirection()[i];
			}
		else if(ray.getDirection()[i]>Scalar(0))
			{
			/* Ray intersects bottom face first: */
			lf1=(bot-ray.getOrigin()[i])/ray.getDirection()[i];
			lf2=(top-ray.getOrigin()[i])/ray.getDirection()[i];
			}
		else if(center[i]-halfSize[i]<=ray.getOrigin()[i]&&ray.getOrigin()[i]<=center[i]+halfSize[i])
			{
			/* Ray is completely between the faces: */
			lf1=Scalar(0);
			lf2=Math::Constants<Scalar>::max;
			}
		else
			{
			/* Ray is completely outside the box: */
			lf1=Scalar(-1);
			lf2=Scalar(-1);
			}
		
		/* Intersect the calculated ray intersection with the complete interval: */
		if(l1<lf1)
			l1=lf1;
		if(l2>lf2)
			l2=lf2;
		}
	
	/* Return the result interval: */
	return Interval(l1,l2);
	}

BoxRayDragger::BoxRayDragger(Scalar sBoxSize,Scalar sRotateFactor)
	:boxSize(sBoxSize),
	 rotateFactor(sRotateFactor),
	 dragMode(DRAG_NONE)
	{
	}

bool BoxRayDragger::pick(const ONTransform& newTransformation,const Ray& ray,const Vector& viewPlaneNormal)
	{
	/* Set the initial transformation: */
	initialTransformation=newTransformation;
	
	/* Transform the ray to box coordinates: */
	Ray boxRay(initialTransformation.inverseTransform(ray.getOrigin()),initialTransformation.inverseTransform(ray.getDirection()));
	
	/* Calculate the half-sizes of the box, the edges and the vertices: */
	Scalar bs=boxSize*Scalar(0.5);
	Scalar es=boxSize*Scalar(0.075*0.5);
	Scalar vs=boxSize*Scalar(0.15*0.5);
	
	/* Initialize the hit result: */
	Scalar minLambda=Math::Constants<Scalar>::max;
	
	Point center;
	Scalar s[3];
	
	/* Intersect the ray with all box vertices: */
	s[0]=s[1]=s[2]=vs;
	for(int vertex=0;vertex<8;++vertex)
		{
		/* Use some bit masking magic to calculate the coordinates of the vertex: */
		for(int i=0;i<3;++i)
			center[i]=vertex&(1<<i)?bs:-bs;
		
		/* Intersect the ray with the vertex box: */
		Interval vi=intersectBox(boxRay,center,s);
		
		/* Check if this intersection is valid and closer than the current one: */
		if(vi.first<=vi.second&&vi.first<minLambda)
			{
			minLambda=vi.first;
			dragMode=DRAG_VERTEX;
			
			/* Initialize vertex dragging: */
			dragPlaneNormal=dragPlaneNormal;
			initialPoint=ray(minLambda);
			dragPlaneOffset=initialPoint*viewPlaneNormal;
			rotateCenter=initialTransformation.getOrigin();
			}
		}
	
	/* Intersect the ray with all box edges: */
	for(int edgeDirection=0;edgeDirection<3;++edgeDirection)
		{
		/* Calculate the size of the edge boxes: */
		s[edgeDirection]=bs;
		for(int i=1;i<3;++i)
			s[(edgeDirection+i)%3]=es;
		
		for(int edge=0;edge<4;++edge)
			{
			/* Use some more bit masking magic to calculate the center point of the edge: */
			center[edgeDirection]=0.0;
			for(int i=0;i<2;++i)
				center[(edgeDirection+i+1)%3]=edge&(1<<i)?bs:-bs;
			
			/* Intersect the ray with the edge box: */
			Interval ei=intersectBox(boxRay,center,s);
			
			/* Check if this intersection is valid and closer than the current one: */
			if(ei.first<=ei.second&&ei.first<minLambda)
				{
				minLambda=ei.first;
				dragMode=DRAG_EDGE;
				
				/* Initialize edge dragging: */
				dragPlaneNormal=viewPlaneNormal;
				initialPoint=ray(minLambda);
				dragPlaneOffset=initialPoint*dragPlaneNormal;
				for(int i=0;i<3;++i)
					rotateAxis[i]=i==edgeDirection?Scalar(1):Scalar(0);
				rotateCenter=initialTransformation.getOrigin();
				rotateAxis=initialTransformation.transform(rotateAxis);
				rotateDragDirection=rotateAxis^viewPlaneNormal;
				rotateDragDirection.normalize();
				}
			}
		}
	
	/* Intersect the ray with all box faces: */
	for(int faceDirection=0;faceDirection<3;++faceDirection)
		{
		/* Calculate the size of the face boxes: */
		s[faceDirection]=0.0;
		for(int i=1;i<3;++i)
			s[(faceDirection+i)%3]=bs;
		
		for(int face=0;face<2;++face)
			{
			/* Use even more bit masking magic to calculate the center point of the face: */
			center[faceDirection]=face&0x1?bs:-bs;
			for(int i=0;i<2;++i)
				center[(faceDirection+i+1)%3]=Scalar(0);
			
			/* Intersect the ray with the face box: */
			Interval fi=intersectBox(boxRay,center,s);
			
			/* Check if this intersection is valid and closer than the current one: */
			if(fi.first<=fi.second&&fi.first<minLambda)
				{
				minLambda=fi.first;
				dragMode=DRAG_FACE;
				
				/* Initialize face dragging: */
				for(int i=0;i<3;++i)
					dragPlaneNormal[i]=i==faceDirection?Scalar(1):Scalar(0);
				dragPlaneNormal=initialTransformation.transform(dragPlaneNormal);
				initialPoint=ray(minLambda);
				dragPlaneOffset=initialPoint*dragPlaneNormal;
				}
			}
		}
	
	/* Initialize the transient transformations: */
	dragTransformation=ONTransform::identity;
	currentTransformation=initialTransformation;
	
	return dragMode!=DRAG_NONE;
	}

void BoxRayDragger::drag(const Ray& ray)
	{
	/* Calculate the intersection point of the ray and the drag plane: */
	Point point;
	bool pointValid=false;
	Scalar denominator=ray.getDirection()*dragPlaneNormal;
	if(denominator!=Scalar(0))
		{
		Scalar lambda=(dragPlaneOffset-ray.getOrigin()*dragPlaneNormal)/denominator;
		if(lambda>=Scalar(0))
			{
			point=ray(lambda);
			pointValid=true;
			}
		}
	
	/* Bail out if the intersection point is invalid: */
	if(!pointValid)
		return;
	
	switch(dragMode)
		{
		case DRAG_VERTEX:
			{
			/* Calculate the axis and amount of rotation using a virtual trackball: */
			Vector a=initialPoint-rotateCenter;
			Vector delta=point-initialPoint;
			Vector axis=a^delta;
			Scalar axisLen=Geometry::mag(axis);
			if(axisLen>Scalar(0))
				{
				axis/=axisLen;
				Scalar angle=Geometry::mag(delta)/rotateFactor;
				dragTransformation*=ONTransform::translateFromOriginTo(rotateCenter);
				dragTransformation*=ONTransform::rotate(ONTransform::Rotation::rotateAxis(axis,angle));
				dragTransformation*=ONTransform::translateToOriginFrom(rotateCenter);
				}
			
			initialPoint=point;
			break;
			}
		
		case DRAG_EDGE:
			/* Calculate the amount of rotation based on the distance between the current and initial points and the rotateDragDirection: */
			dragTransformation=ONTransform::translateFromOriginTo(rotateCenter);
			dragTransformation*=ONTransform::rotate(ONTransform::Rotation::rotateAxis(rotateAxis,(point-initialPoint)*(rotateDragDirection/rotateFactor)));
			dragTransformation*=ONTransform::translateToOriginFrom(rotateCenter);
			break;
		
		case DRAG_FACE:
			/* Translate the initial transformation by the distance vector between the current and initial points: */
			dragTransformation=ONTransform::translate(point-initialPoint);
			break;
		
		default:
			/* Do nothing */
			;
		}
	
	/* Calculate the new current transformation: */
	currentTransformation=dragTransformation;
	currentTransformation*=initialTransformation;
	}

void BoxRayDragger::release(void)
	{
	/* Just reset the dragging mode: */
	dragMode=DRAG_NONE;
	}

}
