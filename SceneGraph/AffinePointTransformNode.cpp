/***********************************************************************
AffinePointTransformNode - Point transformation class to transform
points by arbitrary affine transformations.
Copyright (c) 2011-2013 Oliver Kreylos

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

#include <SceneGraph/AffinePointTransformNode.h>

#include <string.h>
#include <utility>
#include <SceneGraph/VRMLFile.h>

namespace SceneGraph {

/*****************************************
Methods of class AffinePointTransformNode:
*****************************************/

AffinePointTransformNode::AffinePointTransformNode(void)
	{
	/* Initialize the matrix to the identity transformation: */
	for(int i=0;i<3;++i)
		for(int j=0;j<4;++j)
			matrix.appendValue(i==j?TScalar(1):TScalar(0));
	}

const char* AffinePointTransformNode::getStaticClassName(void)
	{
	return "AffinePointTransform";
	}

const char* AffinePointTransformNode::getClassName(void) const
	{
	return "AffinePointTransform";
	}

void AffinePointTransformNode::parseField(const char* fieldName,VRMLFile& vrmlFile)
	{
	if(strcmp(fieldName,"matrix")==0)
		{
		vrmlFile.parseField(matrix);
		}
	else
		PointTransformNode::parseField(fieldName,vrmlFile);
	}

void AffinePointTransformNode::update(void)
	{
	/* Convert the matrix to an affine transformation: */
	transform=ATransform::identity;
	int index=0;
	for(MFTScalar::ValueList::const_iterator mIt=matrix.getValues().begin();mIt!=matrix.getValues().end()&&index<12;++mIt,++index)
		transform.getMatrix()(index/4,index%4)=*mIt;
	
	/* Calculate the inverse and normal transformation: */
	inverseTransform=Geometry::invert(transform);
	for(int i=0;i<3;++i)
		{
		for(int j=0;j<3;++j)
			normalTransform.getMatrix()(i,j)=inverseTransform.getMatrix()(j,i);
		normalTransform.getMatrix()(i,3)=TScalar(0);
		}
	}

PointTransformNode::TPoint AffinePointTransformNode::transformPoint(const PointTransformNode::TPoint& point) const
	{
	return transform.transform(point);
	}

PointTransformNode::TPoint AffinePointTransformNode::inverseTransformPoint(const PointTransformNode::TPoint& point) const
	{
	return inverseTransform.transform(point);
	}

PointTransformNode::TBox AffinePointTransformNode::calcBoundingBox(const std::vector<Point>& points) const
	{
	TBox result=Box::empty;
	
	/* Transform all points individually: */
	for(std::vector<Point>::const_iterator pIt=points.begin();pIt!=points.end();++pIt)
		result.addPoint(transform.transform(TPoint(*pIt)));
	
	return result;
	}

PointTransformNode::TBox AffinePointTransformNode::transformBox(const PointTransformNode::TBox& box) const
	{
	TBox result=Box::empty;
	
	/* Transform the eight corners of the bounding box: */
	for(int i=0;i<8;++i)
		result.addPoint(transform.transform(box.getVertex(i)));
	
	return result;
	}

PointTransformNode::TVector AffinePointTransformNode::transformNormal(const PointTransformNode::TPoint& basePoint,const PointTransformNode::TVector& normal) const
	{
	TVector result=normalTransform.transform(normal);
	result.normalize();
	return result;
	}

}
