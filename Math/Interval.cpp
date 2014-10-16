/***********************************************************************
Interval - Class for closed intervals of arbitrary scalar types.
Copyright (c) 2003-2010 Oliver Kreylos

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

#include <Math/Interval.icpp>

namespace Math {

/******************************************************************
Force instantiation of all standard Interval classes and functions:
******************************************************************/

template class Interval<int>;

template class Interval<float>;

template class Interval<double>;

}
