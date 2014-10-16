/***********************************************************************
LinearInterpolator - Generic class to perform linear interpolation of
geometry data types.
Copyright (c) 2011 Oliver Kreylos

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

#ifndef GEOMETRY_LINEARINTERPOLATOR_INCLUDED
#define GEOMETRY_LINEARINTERPOLATOR_INCLUDED

#include <Geometry/ComponentArray.h>
#include <Geometry/Rotation.h>

namespace Geometry {

template <class InterpolantParam>
class LinearInterpolator
	{
	/* Embedded classes: */
	public:
	typedef InterpolantParam Interpolant; // Type of interpolated objects
	typedef typename Interpolant::Scalar Scalar; // Scalar type of interpolated objects
	
	/* Methods: */
	#if 0
	static void interpolate(const Interpolant& i0,const Interpolant& i1,Scalar w1,Interpolant& result); // Interpolates between the two interpolants
	#endif
	};

/********************************
Specialized interpolator classes:
********************************/

template <class ScalarParam,int dimensionParam>
class LinearInterpolator<ComponentArray<ScalarParam,dimensionParam> > // Interpolator for ComponentArray objects
	{
	/* Embedded classes: */
	public:
	typedef ComponentArray<ScalarParam,dimensionParam> Interpolant; // Type of interpolated objects
	typedef ScalarParam Scalar; // Scalar type of interpolated objects
	static const int dimension=dimensionParam; // Dimension of interpolated objects
	
	/* Methods: */
	static void interpolate(const Interpolant& i0,const Interpolant& i1,Scalar w1,Interpolant& result)
		{
		Scalar w0=Scalar(1)-w1;
		for(int i=0;i<dimensionParam;++i)
			result[i]=i0[i]*w0+i1[i]*w1;
		}
	};

template <class ScalarParam,int dimensionParam>
class LinearInterpolator<Point<ScalarParam,dimensionParam> > // Interpolator for affine points
	{
	/* Embedded classes: */
	public:
	typedef Point<ScalarParam,dimensionParam> Interpolant; // Type of interpolated objects
	typedef ScalarParam Scalar; // Scalar type of interpolated objects
	static const int dimension=dimensionParam; // Dimension of interpolated objects
	
	/* Methods: */
	static void interpolate(const Interpolant& i0,const Interpolant& i1,Scalar w1,Interpolant& result)
		{
		Scalar w0=Scalar(1)-w1;
		for(int i=0;i<dimensionParam;++i)
			result[i]=i0[i]*w0+i1[i]*w1;
		}
	};

template <class ScalarParam,int dimensionParam>
class LinearInterpolator<Vector<ScalarParam,dimensionParam> > // Interpolator for vectors
	{
	/* Embedded classes: */
	public:
	typedef Vector<ScalarParam,dimensionParam> Interpolant; // Type of interpolated objects
	typedef ScalarParam Scalar; // Scalar type of interpolated objects
	static const int dimension=dimensionParam; // Dimension of interpolated objects
	
	/* Methods: */
	static void interpolate(const Interpolant& i0,const Interpolant& i1,Scalar w1,Interpolant& result)
		{
		Scalar w0=Scalar(1)-w1;
		for(int i=0;i<dimensionParam;++i)
			result[i]=i0[i]*w0+i1[i]*w1;
		}
	};

template <class ScalarParam,int dimensionParam>
class LinearInterpolator<HVector<ScalarParam,dimensionParam> > // Interpolator for homogeneous vectors
	{
	/* Embedded classes: */
	public:
	typedef HVector<ScalarParam,dimensionParam> Interpolant; // Type of interpolated objects
	typedef ScalarParam Scalar; // Scalar type of interpolated objects
	static const int dimension=dimensionParam; // Dimension of interpolated objects
	
	/* Methods: */
	static void interpolate(const Interpolant& i0,const Interpolant& i1,Scalar w1,Interpolant& result)
		{
		Scalar w0=Scalar(1)-w1;
		for(int i=0;i<=dimensionParam;++i)
			result[i]=i0[i]*w0+i1[i]*w1;
		}
	};

template <class ScalarParam>
class LinearInterpolator<Rotation<ScalarParam,2> > // Interpolator for 2D rotations
	{
	/* Embedded classes: */
	public:
	typedef Rotation<ScalarParam,2> Interpolant; // Type of interpolated objects
	typedef ScalarParam Scalar; // Scalar type of interpolated objects
	static const int dimension=2; // Dimension of interpolated objects
	
	/* Methods: */
	static void interpolate(const Interpolant& i0,const Interpolant& i1,Scalar w1,Interpolant& result)
		{
		result.setAngle(i0.getAngle*(Scalar(1)-w1)+i1.getAngle*w1);
		}
	};

template <class ScalarParam>
class LinearInterpolator<Rotation<ScalarParam,3> > // Interpolator for 3D rotations
	{
	/* Embedded classes: */
	public:
	typedef Rotation<ScalarParam,3> Interpolant; // Type of interpolated objects
	typedef ScalarParam Scalar; // Scalar type of interpolated objects
	static const int dimension=3; // Dimension of interpolated objects
	
	/* Methods: */
	static void interpolate(const Interpolant& i0,const Interpolant& i1,Scalar w1,Interpolant& result)
		{
		Interpolant delta=invert(i0);
		delta.leftMultiply(i1);
		const Scalar* q=delta.getQuaternion();
		result=i0;
		if(Math::abs(q[3])<Scalar(1))
			{
			Scalar alpha=Math::acos(q[3]);
			Scalar f=Math::sin(w1*alpha)/Math::sin(alpha);
			double qn[4];
			for(int i=0;i<3;++i)
				qn[i]=q[i]*f;
			qn[3]=Math::cos(w1*alpha);
			result*=Interpolant::fromQuaternion(qn);
			}
		}
	};

}

#endif
