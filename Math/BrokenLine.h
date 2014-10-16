/***********************************************************************
BrokenLine - Class to represent constant piecewise linear functions to
map from a source value range with a dead zone in the center to the
interval [-1, +1].
Copyright (c) 2009 Oliver Kreylos

This file is part of the Templatized Math Library (Math).

The Templatized Math Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Templatized Math Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Templatized Math Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef MATH_BROKENLINE_INCLUDED
#define MATH_BROKENLINE_INCLUDED

#include <Math/Math.h>

namespace Math {

template <class ScalarParam>
class BrokenLine
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Broken line's scalar type
	
	/* Elements: */
	Scalar min,max; // Lower and upper limits of source interval
	Scalar deadMin,deadMax; // Lower and upper limits of source interval's flat "dead zone"
	
	/* Constructors and destructors: */
	BrokenLine(void) // Constructs uninitialized broken line
		{
		}
	BrokenLine(Scalar sMin,Scalar sMax,Scalar deadRatio =Scalar(0.05)) // Creates a default broken line
		:min(sMin),max(sMax)
		{
		Scalar mid=Math::mid(min,max);
		Scalar dead=(max-min)*deadRatio;
		deadMin=mid-dead;
		deadMax=mid+dead;
		}
	BrokenLine(Scalar sMin,Scalar sDeadMin,Scalar sDeadMax,Scalar sMax) // Elementwise constructor
		:min(sMin),max(sMax),deadMin(sDeadMin),deadMax(sDeadMax)
		{
		}
	template <class SourceScalarParam>
	BrokenLine(const BrokenLine<SourceScalarParam>& source) // Copy constructor with type conversion
		:min(Scalar(source.min)),max(Scalar(source.max)),
		 deadMin(Scalar(source.deadMin)),deadMax(Scalar(source.deadMax))
		{
		}
	template <class SourceScalarParam>
	BrokenLine& operator=(const BrokenLine<SourceScalarParam>& source) // Assignment operator with type conversion
		{
		/* Operation is idempotent; no need to check for aliasing: */
		min=Scalar(source.min);
		max=Scalar(source.max);
		deadMin=Scalar(source.deadMin);
		deadMax=Scalar(source.deadMax);
		return *this;
		}
	
	/* Methods: */
	Scalar map(Scalar source) const // Maps a source value to the destination range
		{
		if(source<deadMin)
			{
			if(source>min)
				return -(source-deadMin)/(min-deadMin);
			else
				return -1.0;
			}
		else if(source>deadMax)
			{
			if(source<max)
				return (source-deadMax)/(max-deadMax);
			else
				return 1.0;
			}
		else
			return 0.0;
		}
	};

}

#endif
