/***********************************************************************
GeometryMarshallers - Marshaller classes for templatized geometry
objects.
Copyright (c) 2010 Oliver Kreylos

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

#ifndef GEOMETRY_GEOMETRYMARSHALLERS_INCLUDED
#define GEOMETRY_GEOMETRYMARSHALLERS_INCLUDED

#include <Misc/Marshaller.h>

/* Forward declarations: */
namespace Geometry {
template <class ScalarParam,int dimensionParam>
class ComponentArray;
template <class ScalarParam,int dimensionParam>
class Vector;
template <class ScalarParam,int dimensionParam>
class Point;
template <class ScalarParam,int dimensionParam>
class HVector;
template <class ScalarParam,int dimensionParam>
class Box;
template <class ScalarParam,int dimensionParam>
class Ray;
template <class ScalarParam,int dimensionParam>
class Plane;
template <class ScalarParam,int numRowsParam,int numColumnsParam>
class Matrix;
template <class ScalarParam,int dimensionParam>
class Rotation;
template <class ScalarParam,int dimensionParam>
class OrthonormalTransformation;
template <class ScalarParam,int dimensionParam>
class OrthogonalTransformation;
template <class ScalarParam,int dimensionParam>
class AffineTransformation;
template <class ScalarParam,int dimensionParam>
class ProjectiveTransformation;
}

namespace Misc {

template <class ScalarParam,int dimensionParam>
class Marshaller<Geometry::ComponentArray<ScalarParam,dimensionParam> >
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar;
	static const int dimension=dimensionParam;
	typedef Geometry::ComponentArray<ScalarParam,dimensionParam> Value;
	
	/* Methods: */
	static size_t getSize(const Value& value);
	template <class DataSinkParam>
	static void write(const Value& value,DataSinkParam& sink);
	template <class DataSourceParam>
	static Value read(DataSourceParam& source);
	};

template <class ScalarParam,int dimensionParam>
class Marshaller<Geometry::Vector<ScalarParam,dimensionParam> >
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar;
	static const int dimension=dimensionParam;
	typedef Geometry::Vector<ScalarParam,dimensionParam> Value;
	
	/* Methods: */
	static size_t getSize(const Value& value);
	template <class DataSinkParam>
	static void write(const Value& value,DataSinkParam& sink);
	template <class DataSourceParam>
	static Value read(DataSourceParam& source);
	};

template <class ScalarParam,int dimensionParam>
class Marshaller<Geometry::Point<ScalarParam,dimensionParam> >
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar;
	static const int dimension=dimensionParam;
	typedef Geometry::Point<ScalarParam,dimensionParam> Value;
	
	/* Methods: */
	static size_t getSize(const Value& value);
	template <class DataSinkParam>
	static void write(const Value& value,DataSinkParam& sink);
	template <class DataSourceParam>
	static Value read(DataSourceParam& source);
	};

template <class ScalarParam,int dimensionParam>
class Marshaller<Geometry::HVector<ScalarParam,dimensionParam> >
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar;
	static const int dimension=dimensionParam;
	typedef Geometry::HVector<ScalarParam,dimensionParam> Value;
	
	/* Methods: */
	static size_t getSize(const Value& value);
	template <class DataSinkParam>
	static void write(const Value& value,DataSinkParam& sink);
	template <class DataSourceParam>
	static Value read(DataSourceParam& source);
	};

template <class ScalarParam,int dimensionParam>
class Marshaller<Geometry::Box<ScalarParam,dimensionParam> >
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar;
	static const int dimension=dimensionParam;
	typedef Geometry::Box<ScalarParam,dimensionParam> Value;
	typedef typename Value::Point Point;
	
	/* Methods: */
	static size_t getSize(const Value& value);
	template <class DataSinkParam>
	static void write(const Value& value,DataSinkParam& sink);
	template <class DataSourceParam>
	static Value read(DataSourceParam& source);
	};

template <class ScalarParam,int dimensionParam>
class Marshaller<Geometry::Ray<ScalarParam,dimensionParam> >
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar;
	static const int dimension=dimensionParam;
	typedef Geometry::Ray<ScalarParam,dimensionParam> Value;
	typedef typename Value::Point Point;
	typedef typename Value::Vector Vector;
	
	/* Methods: */
	static size_t getSize(const Value& value);
	template <class DataSinkParam>
	static void write(const Value& value,DataSinkParam& sink);
	template <class DataSourceParam>
	static Value read(DataSourceParam& source);
	};

template <class ScalarParam,int dimensionParam>
class Marshaller<Geometry::Plane<ScalarParam,dimensionParam> >
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar;
	static const int dimension=dimensionParam;
	typedef Geometry::Plane<ScalarParam,dimensionParam> Value;
	typedef typename Value::Vector Vector;
	
	/* Methods: */
	static size_t getSize(const Value& value);
	template <class DataSinkParam>
	static void write(const Value& value,DataSinkParam& sink);
	template <class DataSourceParam>
	static Value read(DataSourceParam& source);
	};

template <class ScalarParam,int numRowsParam,int numColumnsParam>
class Marshaller<Geometry::Matrix<ScalarParam,numRowsParam,numColumnsParam> >
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar;
	static const int numRows=numRowsParam;
	static const int numColumns=numColumnsParam;
	typedef Geometry::Matrix<ScalarParam,numRowsParam,numColumnsParam> Value;
	
	/* Methods: */
	static size_t getSize(const Value& value);
	template <class DataSinkParam>
	static void write(const Value& value,DataSinkParam& sink);
	template <class DataSourceParam>
	static Value read(DataSourceParam& source);
	};

template <class ScalarParam>
class Marshaller<Geometry::Rotation<ScalarParam,2> >
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar;
	static const int dimension=2;
	typedef Geometry::Rotation<ScalarParam,2> Value;
	
	/* Methods: */
	static size_t getSize(const Value& value);
	template <class DataSinkParam>
	static void write(const Value& value,DataSinkParam& sink);
	template <class DataSourceParam>
	static Value read(DataSourceParam& source);
	};

template <class ScalarParam>
class Marshaller<Geometry::Rotation<ScalarParam,3> >
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar;
	static const int dimension=3;
	typedef Geometry::Rotation<ScalarParam,3> Value;
	
	/* Methods: */
	static size_t getSize(const Value& value);
	template <class DataSinkParam>
	static void write(const Value& value,DataSinkParam& sink);
	template <class DataSourceParam>
	static Value read(DataSourceParam& source);
	};

template <class ScalarParam,int dimensionParam>
class Marshaller<Geometry::OrthonormalTransformation<ScalarParam,dimensionParam> >
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar;
	static const int dimension=dimensionParam;
	typedef Geometry::OrthonormalTransformation<ScalarParam,dimensionParam> Value;
	typedef typename Value::Vector Vector;
	typedef typename Value::Rotation Rotation;
	
	/* Methods: */
	static size_t getSize(const Value& value);
	template <class DataSinkParam>
	static void write(const Value& value,DataSinkParam& sink);
	template <class DataSourceParam>
	static Value read(DataSourceParam& source);
	};

template <class ScalarParam,int dimensionParam>
class Marshaller<Geometry::OrthogonalTransformation<ScalarParam,dimensionParam> >
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar;
	static const int dimension=dimensionParam;
	typedef Geometry::OrthogonalTransformation<ScalarParam,dimensionParam> Value;
	typedef typename Value::Vector Vector;
	typedef typename Value::Rotation Rotation;
	
	/* Methods: */
	static size_t getSize(const Value& value);
	template <class DataSinkParam>
	static void write(const Value& value,DataSinkParam& sink);
	template <class DataSourceParam>
	static Value read(DataSourceParam& source);
	};

template <class ScalarParam,int dimensionParam>
class Marshaller<Geometry::AffineTransformation<ScalarParam,dimensionParam> >
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar;
	static const int dimension=dimensionParam;
	typedef Geometry::AffineTransformation<ScalarParam,dimensionParam> Value;
	typedef typename Value::Matrix Matrix;
	
	/* Methods: */
	static size_t getSize(const Value& value);
	template <class DataSinkParam>
	static void write(const Value& value,DataSinkParam& sink);
	template <class DataSourceParam>
	static Value read(DataSourceParam& source);
	};

template <class ScalarParam,int dimensionParam>
class Marshaller<Geometry::ProjectiveTransformation<ScalarParam,dimensionParam> >
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar;
	static const int dimension=dimensionParam;
	typedef Geometry::ProjectiveTransformation<ScalarParam,dimensionParam> Value;
	typedef typename Value::Matrix Matrix;
	
	/* Methods: */
	static size_t getSize(const Value& value);
	template <class DataSinkParam>
	static void write(const Value& value,DataSinkParam& sink);
	template <class DataSourceParam>
	static Value read(DataSourceParam& source);
	};

}

#if !defined(GEOMETRY_GEOMETRYMARSHALLERS_IMPLEMENTATION)
#include <Geometry/GeometryMarshallers.icpp>
#endif

#endif
