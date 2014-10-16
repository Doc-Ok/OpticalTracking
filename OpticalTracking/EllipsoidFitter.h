/***********************************************************************
EllipsoidFitter - Helper class to fit an ellipsoid to a set of 3D points
for sensor calibration and rectification.
Copyright (c) 2013 Oliver Kreylos

This file is part of the optical/inertial sensor fusion tracking
package.

The optical/inertial sensor fusion tracking package is free software;
you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

The optical/inertial sensor fusion tracking package is distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the optical/inertial sensor fusion tracking package; if not, write
to the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef ELLIPSOIDFITTER_INCLUDED
#define ELLIPSOIDFITTER_INCLUDED

#include <utility>
#include <Misc/ChunkedArray.h>
#include <Geometry/Point.h>
#include <Geometry/Matrix.h>
#include <GL/gl.h>

/* Forward declarations: */
class GLContextData;

class EllipsoidFitter
	{
	/* Embedded classes: */
	public:
	typedef double Scalar; // Scalar type for points
	typedef Geometry::Point<Scalar,3> Point; // Type for fitting points
	typedef Geometry::Matrix<Scalar,3,4> Matrix; // Type for ellipsoid fitting matrices
	typedef std::pair<Matrix,Scalar> Calibration; // Result of ellipsoid calibration; a matrix converting the measurement ellipsoid into a sphere, and that sphere's radius
	
	private:
	class PointRenderFunctor // Helper class to render arrays of points using vertex arrays
		{
		/* Methods: */
		public:
		void operator()(const Point* points,size_t numPoints)
			{
			glVertexPointer(3,GL_DOUBLE,0,points);
			glDrawArrays(GL_POINTS,0,numPoints);
			}
		};
	
	/* Elements: */
	private:
	Misc::ChunkedArray<Point> points; // List of points to be fitted
	
	/* Constructors and destructors: */
	public:
	EllipsoidFitter(void); // Creates an empty ellipsoid fitter
	
	/* Methods: */
	void addPoint(const Point& point) // Adds another point to be fitted
		{
		points.push_back(point);
		}
	Calibration calculateFit(void) const; // Returns a fitting matrix and a sphere radius for the current set of points
	void clear(void) // Clears the point set
		{
		points.clear();
		}
	void glRenderAction(GLContextData& contextData) const; // Draws the set of fitted points
	};

#endif
