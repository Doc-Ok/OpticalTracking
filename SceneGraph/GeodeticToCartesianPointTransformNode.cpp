/***********************************************************************
GeodeticToCartesianPointTransformNode - Point transformation class to
convert geodetic coordinates (longitude/latitude/altitude on a reference
ellipsoid) to Cartesian coordinates.
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

#include <SceneGraph/GeodeticToCartesianPointTransformNode.h>

#include <utility>
#include <string.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Box.h>
#include <Geometry/Rotation.h>
#include <SceneGraph/Geometry.h>
#include <SceneGraph/VRMLFile.h>

namespace SceneGraph {

/******************************************************
Methods of class GeodeticToCartesianPointTransformNode:
******************************************************/

GeodeticToCartesianPointTransformNode::GeodeticToCartesianPointTransformNode(void)
	:longitude("X"),latitude("Y"),elevation("Z"),
	 degrees(false),
	 colatitude(false),
	 elevationScale(1),
	 moveToOrigin(false),
	 re(0)
	{
	}

const char* GeodeticToCartesianPointTransformNode::getStaticClassName(void)
	{
	return "GeodeticToCartesianPointTransform";
	}

const char* GeodeticToCartesianPointTransformNode::getClassName(void) const
	{
	return "GeodeticToCartesianPointTransform";
	}

void GeodeticToCartesianPointTransformNode::parseField(const char* fieldName,VRMLFile& vrmlFile)
	{
	if(strcmp(fieldName,"referenceEllipsoid")==0)
		{
		vrmlFile.parseSFNode(referenceEllipsoid);
		}
	else if(strcmp(fieldName,"longitude")==0)
		{
		vrmlFile.parseField(longitude);
		}
	else if(strcmp(fieldName,"latitude")==0)
		{
		vrmlFile.parseField(latitude);
		}
	else if(strcmp(fieldName,"elevation")==0)
		{
		vrmlFile.parseField(elevation);
		}
	else if(strcmp(fieldName,"degrees")==0)
		{
		vrmlFile.parseField(degrees);
		}
	else if(strcmp(fieldName,"colatitude")==0)
		{
		vrmlFile.parseField(colatitude);
		}
	else if(strcmp(fieldName,"elevationScale")==0)
		{
		vrmlFile.parseField(elevationScale);
		}
	else if(strcmp(fieldName,"moveToOrigin")==0)
		{
		vrmlFile.parseField(moveToOrigin);
		}
	else if(strcmp(fieldName,"originPoint")==0)
		{
		vrmlFile.parseField(originPoint);
		}
	else
		PointTransformNode::parseField(fieldName,vrmlFile);
	}

void GeodeticToCartesianPointTransformNode::update(void)
	{
	/* Create a default reference ellipsoid if none was given: */
	if(referenceEllipsoid.getValue()==0)
		{
		referenceEllipsoid.setValue(new ReferenceEllipsoidNode);
		referenceEllipsoid.getValue()->update();
		}
	
	/* Get a pointer to the low-level reference ellipsoid: */
	re=&referenceEllipsoid.getValue()->getRE();
	
	/* Get the point component indices: */
	if(longitude.getValue()=="X")
		componentIndices[0]=0;
	else if(longitude.getValue()=="Y")
		componentIndices[0]=1;
	else if(longitude.getValue()=="Z")
		componentIndices[0]=2;
	if(latitude.getValue()=="X")
		componentIndices[1]=0;
	else if(latitude.getValue()=="Y")
		componentIndices[1]=1;
	else if(latitude.getValue()=="Z")
		componentIndices[1]=2;
	if(elevation.getValue()=="X")
		componentIndices[2]=0;
	else if(elevation.getValue()=="Y")
		componentIndices[2]=1;
	else if(elevation.getValue()=="Z")
		componentIndices[2]=2;
	
	/* Calculate the geodetic point transformation: */
	for(int i=0;i<3;++i)
		{
		componentScales[i]=TScalar(1);
		componentOffsets[i]=TScalar(0);
		}
	if(degrees.getValue())
		{
		/* Scale longitude and latitude to radians: */
		componentScales[0]=componentScales[1]=Math::Constants<TScalar>::pi/TScalar(180);
		}
	if(colatitude.getValue())
		{
		/* Subtract latitude in radians from pi/2: */
		componentScales[1]=-componentScales[1];
		componentOffsets[1]=Math::div2(Math::Constants<TScalar>::pi);
		}
	componentScales[2]=elevationScale.getValue();
	
	/* Check if recentering was requested: */
	if(moveToOrigin.getValue())
		{
		/* Convert the origin point to Cartesian coordinates: */
		TPoint geodetic;
		for(int i=0;i<3;++i)
			geodetic[i]=TScalar(originPoint.getValue()[componentIndices[i]])*componentScales[i]+componentOffsets[i];
		TPoint cartesian=re->geodeticToCartesian(geodetic);
		
		/* Calculate the offset vector: */
		offset=TPoint::origin-cartesian;
		}
	else
		{
		/* Reset the offset vector: */
		offset=TVector::zero;
		}
	
	/* Check whether normal vectors need to be flipped: */
	int ci[3];
	for(int i=0;i<3;++i)
		ci[i]=componentIndices[i];
	int numSwaps=0;
	if(ci[0]>ci[1])
		{
		std::swap(ci[0],ci[1]);
		++numSwaps;
		}
	if(ci[1]>ci[2])
		{
		std::swap(ci[1],ci[2]);
		++numSwaps;
		}
	if(ci[0]>ci[1])
		{
		std::swap(ci[0],ci[1]);
		++numSwaps;
		}
	flipNormals=numSwaps%2==1;
	}

PointTransformNode::TPoint GeodeticToCartesianPointTransformNode::transformPoint(const PointTransformNode::TPoint& point) const
	{
	/* Convert the geodetic point to longitude and latitude in radians and elevation in meters: */
	TPoint geodetic;
	for(int i=0;i<3;++i)
		geodetic[i]=point[componentIndices[i]]*componentScales[i]+componentOffsets[i];
	
	/* Return the transformed point: */
	return re->geodeticToCartesian(geodetic)+offset;
	}

PointTransformNode::TPoint GeodeticToCartesianPointTransformNode::inverseTransformPoint(const PointTransformNode::TPoint& point) const
	{
	/* Transform the point from Cartesian to geodetic coordinates: */
	TPoint geodetic=re->cartesianToGeodetic(point-offset);
	
	/* Convert the geodetic point to geodetic coordinate order and scale: */
	TPoint result;
	for(int i=0;i<3;++i)
		result[componentIndices[i]]=(geodetic[i]-componentOffsets[i])/componentScales[i];
	return result;
	}

PointTransformNode::TBox GeodeticToCartesianPointTransformNode::calcBoundingBox(const std::vector<Point>& points) const
	{
	TBox result=TBox::empty;
	
	/* Transform each point individually: */
	for(std::vector<Point>::const_iterator pIt=points.begin();pIt!=points.end();++pIt)
		result.addPoint(transformPoint(TPoint(*pIt)));
	
	return result;
	}

PointTransformNode::TBox GeodeticToCartesianPointTransformNode::transformBox(const PointTransformNode::TBox& box) const
	{
	TBox result=TBox::empty;
	
	/* Transform the eight corners of the box (this is not entirely correct; to be fixed later): */
	for(int i=0;i<8;++i)
		result.addPoint(transformPoint(box.getVertex(i)));
	
	return result;
	}

PointTransformNode::TVector GeodeticToCartesianPointTransformNode::transformNormal(const PointTransformNode::TPoint& basePoint,const PointTransformNode::TVector& normal) const
	{
	/* Convert the geodetic base point and normal vector to longitude and latitude in radians: */
	TPoint geodetic;
	TVector geonormal;
	for(int i=0;i<3;++i)
		{
		geodetic[i]=basePoint[componentIndices[i]]*componentScales[i]+componentOffsets[i];
		geonormal[i]=normal[componentIndices[i]]/componentScales[i];
		}
	
	/* Calculate the geoid transformation's derivative at the base point: */
	ReferenceEllipsoidNode::Geoid::Derivative deriv=re->geodeticToCartesianDerivative(geodetic);
	
	/* Calculate the normal transformation matrix: */
	TScalar a=deriv(1,1)*deriv(2,2)-deriv(1,2)*deriv(2,1);
	TScalar b=deriv(1,2)*deriv(2,0)-deriv(1,0)*deriv(2,2);
	TScalar c=deriv(1,0)*deriv(2,1)-deriv(1,1)*deriv(2,0);
	TScalar d=deriv(0,2)*deriv(2,1)-deriv(0,1)*deriv(2,2);
	TScalar e=deriv(0,0)*deriv(2,2)-deriv(0,2)*deriv(2,0);
	TScalar f=deriv(0,1)*deriv(2,0)-deriv(0,0)*deriv(2,1);
	TScalar g=deriv(0,1)*deriv(1,2)-deriv(0,2)*deriv(1,1);
	TScalar h=deriv(0,2)*deriv(1,0)-deriv(0,0)*deriv(1,2);
	TScalar i=deriv(0,0)*deriv(1,1)-deriv(0,1)*deriv(1,0);
	
	/* Calculate the result normal: */
	TVector result;
	result[0]=a*geonormal[0]+b*geonormal[1]+c*geonormal[2];
	result[1]=d*geonormal[0]+e*geonormal[1]+f*geonormal[2];
	result[2]=g*geonormal[0]+h*geonormal[1]+i*geonormal[2];
	TScalar resultLen=Geometry::mag(result);
	if(flipNormals)
		resultLen=-resultLen;
	result/=resultLen;
	return result;
	}

}
