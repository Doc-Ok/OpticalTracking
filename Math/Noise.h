/***********************************************************************
Noise - Class to implement multivariate band-limited noise following the
reference implementation described in Ken Perlin's 2002 SIGGRAPH paper.
User code can #define MATH_NOISE_NONSTANDARD_TEMPLATES to directly
inline the noise() method for a 10% speed boost.
Copyright (c) 2012 Oliver Kreylos

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

#ifndef GEOMETRY_NOISE_INCLUDED
#define GEOMETRY_NOISE_INCLUDED

#include <Math/Math.h>

namespace Math {

class Noise
	{
	/* Elements: */
	private:
	static const int permutation[512]; // Array tabulating a 256-element permutation, repeated twice to save check for wrap condition
	
	/* Private methods: */
	template <class ScalarParam>
	static ScalarParam basisFunction(ScalarParam t) // Polynomial interpolation basis function of degree 5 with vanishing first and second derivatives
		{
		return t*t*t*(t*(t*ScalarParam(6)-ScalarParam(15))+ScalarParam(10));
		}
	template <class ScalarParam>
	static ScalarParam interpolate(ScalarParam v0,ScalarParam v1,ScalarParam w1) // Linear interpolation between two values
		{
		return v0+(v1-v0)*w1;
		}
	template <class ScalarParam>
	static ScalarParam gradient(int hash,ScalarParam x,ScalarParam y,ScalarParam z) // Calculates scalar product of given vector with one of the 12 corner vectors of a recular icosahedron, chosen based on lowest 4 bits of hash value
		{
		/* Reduce hash to 4 lowest bits: */
		hash&=0x0fU;
		
		/* "Randomly" select two components from the provided vector: */
		ScalarParam u=hash<0x08?x:y;
		ScalarParam v=hash<0x04?y:(hash==0x0c||hash==0x0e?x:z);
		
		/* "Randomly" flip the signs of the two selected components and return their sum: */
		return ((hash&0x01)==0x00?u:-u)+((hash&0x02)==0x00?v:-v);
		}
	
	/* Methods: */
	public:
	template <class ScalarParam>
	static ScalarParam noise(ScalarParam x,ScalarParam y,ScalarParam z); // Calculates noise function value for given 3D point
	};

}

#if (defined(MATH_NONSTANDARD_TEMPLATES) || defined(MATH_NOISE_NONSTANDARD_TEMPLATES)) && !defined(MATH_NOISE_IMPLEMENTATION)
#include <Math/Noise.icpp>
#endif

#endif
