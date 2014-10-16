/***********************************************************************
Types - Type definitions for GLMotif UI components.
Copyright (c) 2001-2005 Oliver Kreylos

This file is part of the GLMotif Widget Library (GLMotif).

The GLMotif Widget Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GLMotif Widget Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the GLMotif Widget Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef GLMOTIF_TYPES_INCLUDED
#define GLMOTIF_TYPES_INCLUDED

#include <utility>
#include <Geometry/Point.h>
#include <Geometry/Ray.h>
#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLVector.h>
#include <GL/GLBox.h>

namespace GLMotif {

/*****************
Type declarations:
*****************/

typedef double Scalar;
typedef Geometry::Point<Scalar,3> Point;
typedef Geometry::Ray<Scalar,3> Ray;
typedef GLColor<GLfloat,4> Color;
typedef GLVector<GLfloat,3> Vector;
typedef GLBox<GLfloat,3> Box;
typedef std::pair<GLfloat,GLfloat> ZRange;

/****************
Helper functions:
****************/

inline ZRange& operator+=(ZRange& zr1,const ZRange& zr2) // Calculates union of two Z ranges; changes first range
	{
	if(zr1.first>zr2.first)
		zr1.first=zr2.first;
	if(zr1.second<zr2.second)
		zr1.second=zr2.second;
	return zr1;
	}

inline ZRange operator+(const ZRange& zr1,const ZRange& zr2) // Calculates union of two Z ranges
	{
	return ZRange(zr1.first<=zr2.first?zr1.first:zr2.first,zr1.second>=zr2.second?zr1.second:zr2.second);
	}

}

#endif
