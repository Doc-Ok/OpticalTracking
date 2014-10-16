/***********************************************************************
MathMarshallers - Marshaller classes for math objects.
Copyright (c) 2010 Oliver Kreylos

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

#ifndef MATH_MATHMARSHALLERS_INCLUDED
#define MATH_MATHMARSHALLERS_INCLUDED

#include <Misc/Marshaller.h>
#include <Math/BrokenLine.h>

namespace Misc {

template <class ScalarParam>
class Marshaller<Math::BrokenLine<ScalarParam> >
	{
	/* Embedded classes: */
	public:
	typedef ScalarParam Scalar; // Scalar type for broken line
	typedef Math::BrokenLine<ScalarParam> Value; // Value type
	
	/* Methods: */
	public:
	static size_t getSize(const Value& value);
		{
		return Marshaller<Scalar>::getSize(value.min)+Marshaller<Scalar>::getSize(value.deadMin)+Marshaller<Scalar>::getSize(value.deadMax)+Marshaller<Scalar>::getSize(value.max);
		}
	template <class DataSinkParam>
	static void write(const Value& sink)
		{
		Marshaller<Scalar>::write(value.min,sink);
		Marshaller<Scalar>::write(value.deadMin,sink);
		Marshaller<Scalar>::write(value.deadMax,sink);
		Marshaller<Scalar>::write(value.max,sink);
		}
	template <class DataSourceParam>
	static Value read(DataSourceParam& source)
		{
		Value result;
		result.min=Marshaller<Scalar>::read(source);
		result.deadMin=Marshaller<Scalar>::read(source);
		result.deadMax=Marshaller<Scalar>::read(source);
		result.max=Marshaller<Scalar>::read(source);
		return result;
		}
	};

}

#endif
