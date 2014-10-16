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

#include <SceneGraph/UTMPointTransformNode.h>

#include <string.h>
#include <SceneGraph/VRMLFile.h>

namespace SceneGraph {

/**************************************
Methods of class UTMPointTransformNode:
**************************************/

UTMPointTransformNode::UTMPointTransformNode(void)
	:zone(0),
	 scaleFactor(0.9996),
	 falseNorthing(0.0),falseEasting(500000.0),
	 projection(0)
	{
	}

const char* UTMPointTransformNode::getStaticClassName(void)
	{
	return "UTMPointTransform";
	}

const char* UTMPointTransformNode::getClassName(void) const
	{
	return "UTMPointTransform";
	}

void UTMPointTransformNode::parseField(const char* fieldName,VRMLFile& vrmlFile)
	{
	if(strcmp(fieldName,"referenceEllipsoid")==0)
		{
		vrmlFile.parseSFNode(referenceEllipsoid);
		}
	else if(strcmp(fieldName,"zone")==0)
		{
		vrmlFile.parseField(zone);
		}
	else if(strcmp(fieldName,"scaleFactor")==0)
		{
		vrmlFile.parseField(scaleFactor);
		}
	else if(strcmp(fieldName,"falseNorthing")==0)
		{
		vrmlFile.parseField(falseNorthing);
		}
	else if(strcmp(fieldName,"falseEasting")==0)
		{
		vrmlFile.parseField(falseEasting);
		}
	else if(strcmp(fieldName,"degrees")==0)
		{
		vrmlFile.parseField(degrees);
		}
	else
		PointTransformNode::parseField(fieldName,vrmlFile);
	}

void UTMPointTransformNode::update(void)
	{
	/* Create a default reference ellipsoid if none was given: */
	if(referenceEllipsoid.getValue()==0)
		{
		referenceEllipsoid.setValue(new ReferenceEllipsoidNode);
		referenceEllipsoid.getValue()->update();
		}
	
	/* Get a reference to the low-level reference ellipsoid: */
	const ReferenceEllipsoidNode::Geoid& re=referenceEllipsoid.getValue()->getRE();
	
	/* Create the UTM projection object: */
	projection=Geometry::UTMProjection<TScalar>(zone.getValue(),re.getRadius(),re.getFlatteningFactor());
	projection.setStretching(scaleFactor.getValue());
	projection.setFalseNorthing(falseNorthing.getValue());
	projection.setFalseEasting(falseEasting.getValue());
	}

PointTransformNode::TPoint UTMPointTransformNode::transformPoint(const PointTransformNode::TPoint& point) const
	{
	/* Transform the point using the UTM projection object: */
	Geometry::UTMProjection<double>::PPoint geodetic=projection.mapToGeodetic(Geometry::UTMProjection<double>::PPoint(point[0],point[1]));
	if(degrees.getValue())
		{
		/* Convert geodetic coordinates from radians to degrees: */
		geodetic[0]*=TScalar(180)/Math::Constants<TScalar>::pi;
		geodetic[1]*=TScalar(180)/Math::Constants<TScalar>::pi;
		}
	return TPoint(geodetic[0],geodetic[1],point[2]);
	}

PointTransformNode::TPoint UTMPointTransformNode::inverseTransformPoint(const PointTransformNode::TPoint& point) const
	{
	/* Transform the point using the UTM projection object: */
	Geometry::UTMProjection<double>::PPoint geodetic(point[0],point[1]);
	if(degrees.getValue())
		{
		/* Convert geodetic coordinates from degrees to radians: */
		geodetic[0]*=Math::Constants<TScalar>::pi/TScalar(180);
		geodetic[1]*=Math::Constants<TScalar>::pi/TScalar(180);
		}
	Geometry::UTMProjection<double>::PPoint map=projection.geodeticToMap(geodetic);
	return TPoint(map[0],map[1],point[2]);
	}

PointTransformNode::TBox UTMPointTransformNode::calcBoundingBox(const std::vector<Point>& points) const
	{
	TBox result=TBox::empty;
	
	/* Transform each point individually: */
	for(std::vector<Point>::const_iterator pIt=points.begin();pIt!=points.end();++pIt)
		{
		Geometry::UTMProjection<double>::PPoint geodetic=projection.mapToGeodetic(Geometry::UTMProjection<double>::PPoint(TScalar((*pIt)[0]),TScalar((*pIt)[1])));
		if(degrees.getValue())
			{
			/* Convert geodetic coordinates from radians to degrees: */
			geodetic[0]*=TScalar(180)/Math::Constants<TScalar>::pi;
			geodetic[1]*=TScalar(180)/Math::Constants<TScalar>::pi;
			}
		result.addPoint(TPoint(geodetic[0],geodetic[1],TScalar((*pIt)[2])));
		}
	
	return result;
	}

PointTransformNode::TBox UTMPointTransformNode::transformBox(const PointTransformNode::TBox& box) const
	{
	TBox result=TBox::empty;
	
	/* Transform the eight corners of the box (this is not entirely correct; to be fixed later): */
	for(int i=0;i<8;++i)
		{
		TPoint vertex=box.getVertex(i);
		Geometry::UTMProjection<double>::PPoint geodetic=projection.mapToGeodetic(Geometry::UTMProjection<double>::PPoint(vertex[0],vertex[1]));
		if(degrees.getValue())
			{
			/* Convert geodetic coordinates from radians to degrees: */
			geodetic[0]*=TScalar(180)/Math::Constants<TScalar>::pi;
			geodetic[1]*=TScalar(180)/Math::Constants<TScalar>::pi;
			}
		result.addPoint(TPoint(geodetic[0],geodetic[1],vertex[2]));
		}
	
	return result;
	}

PointTransformNode::TVector UTMPointTransformNode::transformNormal(const PointTransformNode::TPoint& basePoint,const PointTransformNode::TVector& normal) const
	{
	/* This involves scaling factors and such. Implement later... */
	return normal;
	}

}
