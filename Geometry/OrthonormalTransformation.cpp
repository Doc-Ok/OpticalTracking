/***********************************************************************
OrthonormalTransformation - Class for transformations constructed from
only translations and rotations.
Copyright (c) 2002-2011 Oliver Kreylos

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

#include <Geometry/OrthonormalTransformation.icpp>

namespace Geometry {

/***********************************************************************************
Force instantiation of all standard OrthonormalTransformation classes and functions:
***********************************************************************************/

template class OrthonormalTransformation<float,2>;
template OrthonormalTransformation<float,2>::OrthonormalTransformation(const TranslationTransformation<float,2>&);
template OrthonormalTransformation<float,2>::OrthonormalTransformation(const RotationTransformation<float,2>&);
template OrthonormalTransformation<float,2>::OrthonormalTransformation(const TranslationTransformation<double,2>&);
template OrthonormalTransformation<float,2>::OrthonormalTransformation(const RotationTransformation<double,2>&);
template OrthonormalTransformation<float,2> operator*(const TranslationTransformation<float,2>&,const RotationTransformation<float,2>&);
template OrthonormalTransformation<float,2> operator*(const RotationTransformation<float,2>&,const TranslationTransformation<float,2>&);

template class OrthonormalTransformation<double,2>;
template OrthonormalTransformation<double,2>::OrthonormalTransformation(const TranslationTransformation<float,2>&);
template OrthonormalTransformation<double,2>::OrthonormalTransformation(const RotationTransformation<float,2>&);
template OrthonormalTransformation<double,2>::OrthonormalTransformation(const TranslationTransformation<double,2>&);
template OrthonormalTransformation<double,2>::OrthonormalTransformation(const RotationTransformation<double,2>&);
template OrthonormalTransformation<double,2> operator*(const TranslationTransformation<double,2>&,const RotationTransformation<double,2>&);
template OrthonormalTransformation<double,2> operator*(const RotationTransformation<double,2>&,const TranslationTransformation<double,2>&);

template class OrthonormalTransformation<float,3>;
template OrthonormalTransformation<float,3>::OrthonormalTransformation(const TranslationTransformation<float,3>&);
template OrthonormalTransformation<float,3>::OrthonormalTransformation(const RotationTransformation<float,3>&);
template OrthonormalTransformation<float,3>::OrthonormalTransformation(const TranslationTransformation<double,3>&);
template OrthonormalTransformation<float,3>::OrthonormalTransformation(const RotationTransformation<double,3>&);
template OrthonormalTransformation<float,3> operator*(const TranslationTransformation<float,3>&,const RotationTransformation<float,3>&);
template OrthonormalTransformation<float,3> operator*(const RotationTransformation<float,3>&,const TranslationTransformation<float,3>&);

template class OrthonormalTransformation<double,3>;
template OrthonormalTransformation<double,3>::OrthonormalTransformation(const TranslationTransformation<float,3>&);
template OrthonormalTransformation<double,3>::OrthonormalTransformation(const RotationTransformation<float,3>&);
template OrthonormalTransformation<double,3>::OrthonormalTransformation(const TranslationTransformation<double,3>&);
template OrthonormalTransformation<double,3>::OrthonormalTransformation(const RotationTransformation<double,3>&);
template OrthonormalTransformation<double,3> operator*(const TranslationTransformation<double,3>&,const RotationTransformation<double,3>&);
template OrthonormalTransformation<double,3> operator*(const RotationTransformation<double,3>&,const TranslationTransformation<double,3>&);

}
