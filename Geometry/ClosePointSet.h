/***********************************************************************
ClosePointSet - Class to store results of nearest-neighbour-queries in
spatial data structures.
Copyright (c) 2003-2013 Oliver Kreylos

This file is part of the Templatized Geometry Library (TGL).

The Templatized Geometry Library is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Templatized Geometry Library is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Templatized Geometry Library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef GEOMETRY_CLOSEPOINTSET_INCLUDED
#define GEOMETRY_CLOSEPOINTSET_INCLUDED

#include <Math/Math.h>
#include <Math/Constants.h>

namespace Geometry {

template <class StoredPointParam>
class ClosePointSet
	{
	/* Embedded classes: */
	public:
	typedef typename StoredPointParam::Scalar Scalar; // Scalar type used by points
	static const int dimension=StoredPointParam::dimension; // Dimension of points
	typedef StoredPointParam StoredPoint; // Type of reported points (typically with some associated value)
	typedef Geometry::Point<Scalar,dimension> Point; // Type for positions
	
	private:
	struct ClosePoint // Structure to store a single close point
		{
		/* Elements: */
		public:
		const StoredPoint* point; // Pointer to stored point
		Scalar dist2; // Squared distance between stored point and original query point
		};
	
	/* Elements: */
	private:
	int maxNumPoints; // Maximum number of points in the set
	int numPoints; // Number of points in the set
	ClosePoint* points; // Array of close points
	Scalar maxDist2; // Maximum squared distance of points allowed into the set
	Scalar dist2; // Current maximum distance of points in set
	
	/* Constructors and destructors: */
	public:
	ClosePointSet(int sMaxNumPoints) // Creates empty point set holding up to given number of points
		:maxNumPoints(sMaxNumPoints),numPoints(0),points(new ClosePoint[maxNumPoints]),
		 maxDist2(Math::Constants<Scalar>::max),dist2(maxDist2)
		{
		}
	ClosePointSet(int sMaxNumPoints,Scalar sMaxSqrDist) // Creates empty point set holding up to given number of points up to given distance
		:maxNumPoints(sMaxNumPoints),numPoints(0),points(new ClosePoint[maxNumPoints]),
		 maxDist2(sMaxSqrDist),dist2(maxDist2)
		{
		}
	ClosePointSet(const ClosePointSet& source) // Copy constructor
		:maxNumPoints(source.maxNumPoints),numPoints(source.numPoints),points(new ClosePoint[maxNumPoints]),
		 maxDist2(source.maxDist2),dist2(source.dist2)
		{
		/* Copy close points: */
		for(int i=0;i<numPoints;++i)
			points[i]=source.points[i];
		}
	ClosePointSet& operator=(const ClosePointSet& source) // Assignment operator
		{
		if(this!=&source)
			{
			/* Delete existing point set: */
			delete[] points;
		
			/* Copy new point set: */
			maxNumPoints=source.maxNumPoints;
			numPoints=source.numPoints;
			points=new ClosePoint[maxNumPoints];
			for(int i=0;i<numPoints;++i)
				points[i]=source.points[i];
			maxDist2=source.maxDist2;
			dist2=source.dist2;
			}
		
		return *this;
		}
	~ClosePointSet(void)
		{
		delete[] points;
		}
	
	/* Methods: */
	int getMaxNumPoints(void) const // Returns maximum number of points in set
		{
		return maxNumPoints;
		}
	int getNumPoints(void) const // Returns number of points in set
		{
		return numPoints;
		}
	Scalar getMaxSqrDist(void) const // Returns maximum squared distance that can still be inserted into set
		{
		return dist2;
		}
	Scalar getSqrDist(int index) const // Returns squared distance from original query point to close point
		{
		return points[index].dist2;
		}
	Scalar getDist(int index) const // Returns squared distance from original query point to close point
		{
		return Math::sqrt(points[index].dist2);
		}
	const StoredPoint& getPoint(int index) const // Returns close point
		{
		return *points[index].point;
		}
	void insertPoint(const StoredPoint& newPoint,Scalar newSqrDist) // Inserts new point (with given squared distance from query point) into set
		{
		if(newSqrDist<dist2)
			{
			if(numPoints<maxNumPoints)
				{
				/* Insert the new point into the list: */
				int insertIndex;
				for(insertIndex=numPoints;insertIndex>0&&points[insertIndex-1].dist2>newSqrDist;--insertIndex)
					points[insertIndex]=points[insertIndex-1];
				points[insertIndex].point=&newPoint;
				points[insertIndex].dist2=newSqrDist;
				++numPoints;
				if(numPoints==maxNumPoints)
					dist2=points[numPoints-1].dist2;
				}
			else
				{
				/* Remove the last point from the list and insert the new point: */
				int insertIndex;
				for(insertIndex=numPoints-1;insertIndex>0&&points[insertIndex-1].dist2>newSqrDist;--insertIndex)
					points[insertIndex]=points[insertIndex-1];
				points[insertIndex].point=&newPoint;
				points[insertIndex].dist2=newSqrDist;
				dist2=points[numPoints-1].dist2;
				}
			}
		}
	void clear(void) // Clears the point set
		{
		/* Reset number of points in the set: */
		numPoints=0;
		
		/* Reset maximum point distance: */
		dist2=maxDist2;
		}
	};

}

#endif
