/***********************************************************************
ModelTracker - Class to calculate the position and orientation of rigid
3D models based on projected images of the models using the POSIT or
SoftPOSIT algorithms.
Copyright (c) 2014 Oliver Kreylos

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

#ifndef MODELTRACKER_INCLUDED
#define MODELTRACKER_INCLUDED

#include <Math/Matrix.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Geometry/AffineTransformation.h>
#include <Geometry/ProjectiveTransformation.h>

/* Forward declarations: */
namespace IO {
class Directory;
}

class ModelTracker
	{
	/* Embedded classes: */
	public:
	typedef double Scalar;
	typedef Geometry::Point<Scalar,3> Point; // Type for 3D model points
	typedef Geometry::Vector<Scalar,3> Vector; // Type for 3D vectors
	typedef Geometry::ProjectiveTransformation<Scalar,3> Projection; // Type for 3D projections
	typedef Geometry::Point<Scalar,2> ImgPoint; // Type for 2D image points
	typedef Geometry::AffineTransformation<Scalar,2> ImgTransform; // Type for 2D affine transformations
	typedef Geometry::OrthonormalTransformation<Scalar,3> Transform; // Type for 3D rigid body transformations
	
	/* Elements: */
	private:
	unsigned int numModelPoints; // Number of points in the rigid 3D model
	Point* modelPoints; // Array of model points
	Math::Matrix invModelMat; // Inverse of the model matrix describing the layout of the 3D model, for POSIT algorithm
	Projection projection; // The full projection matrix
	Scalar f; // Focal length of the projection
	ImgTransform imgTransform; // Transformation to apply to image points to rectify the camera projection
	Scalar maxMatchDist2; // Maximum squared distance between a model point projection and an image point to create a match
	
	/* Transient state: */
	Scalar* mpws; // Array of homogeneous weights of model points; updated during pose estimation
	
	/* Constructors and destructors: */
	public:
	ModelTracker(void); // Creates empty model tracker
	~ModelTracker(void); // Destroys the model tracker
	
	/* Methods: */
	ImgPoint project(const Point& modelPoint) const // Projects the given 3D point by the full projection
		{
		Point pmp=projection.transform(modelPoint);
		return ImgPoint(pmp[0],pmp[1]);
		}
	unsigned int getNumModelPoints(void) const // Returns the number of points in the 3D model
		{
		return numModelPoints;
		}
	const Point& getModelPoint(unsigned int index) const // Returns one of the 3D model's points
		{
		return modelPoints[index];
		}
	void setModel(unsigned int newNumModelPoints,const Point modelPoints[]); // Sets the rigid 3D model
	void loadCameraIntrinsics(const IO::Directory& directory,const char* intrinsicsFileName); // Loads camera intrinsic parameters from the given calibration file
	void setMaxMatchDist(Scalar newMaxMatchDist); // Sets the maximum matching distance between projected model points and image points for SoftPOSIT
	Transform position(const ImgPoint imagePoints[],const Transform::Rotation& orientation) const; // Returns the position and orientation of the 3D model based on the given known orientation and matched set of image points
	Transform posit(ImgPoint imagePoints[],unsigned int maxNumIterations); // Returns the position and orientation of the 3D model based on the given matched set of image points; modifies image point array
	#if 0
	Transform external_epnp(const ImgPoint imagePoints[]); // Returns the position and orientation of the 3D model based on the given matched set of image points
	#endif
	Transform epnp(const ImgPoint imagePoints[]); // Returns the position and orientation of the 3D model based on the given matched set of image points
	Transform levenbergMarquardt(const ImgPoint imagePoints[],const Transform& initialTransform,unsigned int maxNumIterations); // Optimizes the given initial transform via direct non-linear reprojection error minimization
	Transform softPosit(unsigned int numImagePoints,ImgPoint imagePoints[],const Transform& initialTransform); // Returns the position and orientation of the 3D model based on the given set of image points and initial guess; modifies image point array
	Scalar calcReprojectionError(const ImgPoint imagePoints[],const Transform& transform) const; // Calculates the total squared reprojection error
	};

#endif
