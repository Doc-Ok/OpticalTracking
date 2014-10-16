/***********************************************************************
LensDistortionParameters - Class to represent functions to correct
common lens distortions; specifically, the Brown-Conrady radial/
tangential distortion formula with quartic radial and tangential terms.
Copyright (c) 2009-2014 Oliver Kreylos

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

#ifndef LENSDISTORTIONPARAMETERS_INCLUDED
#define LENSDISTORTIONPARAMETERS_INCLUDED

#include <Math/Math.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>

/* Forward declarations: */
namespace IO {
class File;
}
namespace Geometry {
template <class ScalarParam,int numRowsParam,int numColumnsParam>
class Matrix;
}

class LensDistortionParameters
	{
	/* Embedded classes: */
	public:
	static const int numParameters=7; // Total number of coefficients for lens distortion formula
	typedef Geometry::ComponentArray<double,numParameters> Parameters; // Type to hold distortion formula parameters
	typedef Geometry::Point<double,2> Point; // Type for image points
	typedef Geometry::Vector<double,2> Vector; // Type for image vectors
	typedef Geometry::Matrix<double,2,numParameters> Derivative; // Type for distortion function derivative
	
	/* Elements: */
	Point center; // Center point of radial distortion
	double radiusScale2; // Normalization factor for squared point radii
	double kappa[2]; // Radial coefficients of distortion formula
	double rho[3]; // Tangential coefficients of distortion formula
	
	/* Constructors and destructors: */
	LensDistortionParameters(void) // Dummy constructor
		{
		}
	LensDistortionParameters(const int imageSize[2]); // Creates identity transformation for given image size in pixels
	
	/* Methods: */
	void read(const char* fileName); // Reads distortion parameters from a file
	void read(IO::File& file); // Ditto, from already-opened file
	void write(const char* fileName) const; // Writes distortion parameters to a file
	void write(IO::File& file) const; // Ditto, to already-opened file
	Parameters getParameters(void) const // Returns the current distortion parameter values
		{
		Parameters result;
		for(int i=0;i<2;++i)
			result[i]=center[i];
		for(int i=0;i<2;++i)
			result[2+i]=kappa[i];
		for(int i=0;i<3;++i)
			result[4+i]=rho[i];
		return result;
		}
	void setParameters(const Parameters& parameters) // Sets the current distortion parameter values
		{
		for(int i=0;i<2;++i)
			center[i]=parameters[i];
		for(int i=0;i<2;++i)
			kappa[i]=parameters[2+i];
		for(int i=0;i<3;++i)
			rho[i]=parameters[4+i];
		}
	int getNumVars(void) const
		{
		return numParameters;
		}
	static const char* getVarName(int index);
	double getVar(int index) const
		{
		switch(index)
			{
			case 0:
			case 1:
				return center[index];
			
			case 2:
			case 3:
				return kappa[index-2];
				
			default:
				return rho[index-4];
			}
		}
	void setVar(int index,double newValue)
		{
		switch(index)
			{
			case 0:
			case 1:
				center[index]=newValue;
				break;
			
			case 2:
			case 3:
				kappa[index-2]=newValue;
				
			default:
				rho[index-4]=newValue;
			}
		}
	void normalize(void)
		{
		/* Not necessary for Brown-Conrady formula */
		}
	Point transformNonNormalized(const Point& source) const // Transforms the given point without assuming that the parameters are normalized
		{
		Vector d=source-center;
		double r2=d.sqr()*radiusScale2;
		double rScale=(1.0+(kappa[0]+kappa[1]*r2)*r2);
		double tScale=(1.0+kappa[4]*r2);
		return Point(center[0]+d[0]*rScale+(kappa[2]*(r2+2.0*(d[0]*d[0]))+2.0*kappa[3]*d[0]*d[1])*tScale,
		             center[1]+d[1]*rScale+(kappa[3]*(r2+2.0*(d[1]*d[1]))+2.0*kappa[2]*d[0]*d[1])*tScale);
		}
	Derivative transformp(const Point& source) const; // Calculates the transformation derivative for the given point
	Point transform(const Point& source) const // Transforms the given point assuming that the parameters are normalized
		{
		Vector d=source-center;
		double r2=d.sqr()*radiusScale2;
		double rScale=(1.0+(kappa[0]+kappa[1]*r2)*r2);
		double tScale=(1.0+kappa[4]*r2);
		return Point(center[0]+d[0]*rScale+(kappa[2]*(r2+2.0*(d[0]*d[0]))+2.0*kappa[3]*d[0]*d[1])*tScale,
		             center[1]+d[1]*rScale+(kappa[3]*(r2+2.0*(d[1]*d[1]))+2.0*kappa[2]*d[0]*d[1])*tScale);
		}
	};

#endif
