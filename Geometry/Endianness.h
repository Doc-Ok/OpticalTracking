/***********************************************************************
Endianness - Helper functions to deal with endianness conversion of
geometry data types.
Copyright (c) 2006 Oliver Kreylos

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

#ifndef GEOMETRY_ENDIANNESS_INCLUDED
#define GEOMETRY_ENDIANNESS_INCLUDED

#include <Misc/Endianness.h>

/* Forward declarations: */
namespace Misc {
template <class ValueParam>
class EndiannessSwapper;
}
namespace Geometry {
template <class ScalarParam,int dimensionParam>
class ComponentArray;
template <class ScalarParam,int dimensionParam>
class Vector;
template <class ScalarParam,int dimensionParam>
class Point;
template <class ScalarParam,int dimensionParam>
class HVector;
template <class ScalarParam,int numRowsParam,int numColumnsParam>
class Matrix;
template <class PointParam,class ValueParam>
class ValuedPoint;
}

namespace Misc {

template <class ScalarParam,int dimensionParam>
class EndiannessSwapper<Geometry::ComponentArray<ScalarParam,dimensionParam> >
	{
	/* Methods: */
	public:
	static void swap(Geometry::ComponentArray<ScalarParam,dimensionParam>& ca)
		{
		swapEndianness(ca.getComponents(),dimensionParam);
		}
	static void swap(Geometry::ComponentArray<ScalarParam,dimensionParam>* cas,size_t numCas)
		{
		for(size_t i=0;i<numCas;++i)
			swapEndianness(cas[i].getComponents(),dimensionParam);
		}
	};

template <class ScalarParam,int dimensionParam>
class EndiannessSwapper<Geometry::Vector<ScalarParam,dimensionParam> >
	{
	/* Methods: */
	public:
	static void swap(Geometry::Vector<ScalarParam,dimensionParam>& v)
		{
		swapEndianness(v.getComponents(),dimensionParam);
		}
	static void swap(Geometry::Vector<ScalarParam,dimensionParam>* vs,size_t numVs)
		{
		for(size_t i=0;i<numVs;++i)
			swapEndianness(vs[i].getComponents(),dimensionParam);
		}
	};

template <class ScalarParam,int dimensionParam>
class EndiannessSwapper<Geometry::Point<ScalarParam,dimensionParam> >
	{
	/* Methods: */
	public:
	static void swap(Geometry::Point<ScalarParam,dimensionParam>& p)
		{
		swapEndianness(p.getComponents(),dimensionParam);
		}
	static void swap(Geometry::Point<ScalarParam,dimensionParam>* ps,size_t numPs)
		{
		for(size_t i=0;i<numPs;++i)
			swapEndianness(ps[i].getComponents(),dimensionParam);
		}
	};

template <class ScalarParam,int dimensionParam>
class EndiannessSwapper<Geometry::HVector<ScalarParam,dimensionParam> >
	{
	/* Methods: */
	public:
	static void swap(Geometry::HVector<ScalarParam,dimensionParam>& v)
		{
		swapEndianness(v.getComponents(),dimensionParam+1);
		}
	static void swap(Geometry::HVector<ScalarParam,dimensionParam>* vs,size_t numVs)
		{
		for(size_t i=0;i<numVs;++i)
			swapEndianness(vs[i].getComponents(),dimensionParam+1);
		}
	};

template <class ScalarParam,int numRowsParam,int numColumnsParam>
class EndiannessSwapper<Geometry::Matrix<ScalarParam,numRowsParam,numColumnsParam> >
	{
	/* Methods: */
	public:
	static void swap(Geometry::Matrix<ScalarParam,numRowsParam,numColumnsParam>& m)
		{
		for(int row=0;row<numRowsParam;++row)
			for(int column=0;column<numColumnsParam;++column)
				swapEndianness(m(row,column));
		}
	static void swap(Geometry::Matrix<ScalarParam,numRowsParam,numColumnsParam>& ms,size_t numMs)
		{
		for(size_t i=0;i<numMs;++i)
			for(int row=0;row<numRowsParam;++row)
				for(int column=0;column<numColumnsParam;++column)
					swapEndianness(ms[i](row,column));
		}
	};

template <class PointParam,class ValueParam>
class EndiannessSwapper<Geometry::ValuedPoint<PointParam,ValueParam> >
	{
	/* Methods: */
	public:
	static void swap(Geometry::ValuedPoint<PointParam,ValueParam>& vp)
		{
		swapEndianness(vp.getComponents(),PointParam::dimension);
		swapEndianness(vp.value);
		}
	static void swap(Geometry::ValuedPoint<PointParam,ValueParam>* vps,size_t numVps)
		{
		for(size_t i=0;i<numVps;++i)
			{
			swapEndianness(vps[i].getComponents(),PointParam::dimension);
			swapEndianness(vps[i].value);
			}
		}
	};

}

#endif
