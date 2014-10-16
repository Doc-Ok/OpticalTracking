/***********************************************************************
MathValueCoders - Value coder classes for math objects.
Copyright (c) 2009-2010 Oliver Kreylos

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

#ifndef MATH_MATHVALUECODERS_INCLUDED
#define MATH_MATHVALUECODERS_INCLUDED

#include <Misc/ValueCoder.h>

/* Forward declarations: */
namespace Math {
template <class ScalarParam>
class BrokenLine;
}

namespace Misc {

template <class ScalarParam>
class ValueCoder<Math::BrokenLine<ScalarParam> >
	{
	/* Methods: */
	public:
	static std::string encode(const Math::BrokenLine<ScalarParam>& value);
	static Math::BrokenLine<ScalarParam> decode(const char* start,const char* end,const char** decodeEnd =0);
	};

}

#if defined(MATH_NONSTANDARD_TEMPLATES) && !defined(MATH_MATHVALUECODERS_IMPLEMENTATION)
#include <Math/MathValueCoders.icpp>
#endif

#endif
