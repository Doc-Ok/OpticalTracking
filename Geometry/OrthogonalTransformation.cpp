/***********************************************************************
OrthogonalTransformation - Class for transformations constructed from
only translations, rotations and uniform scalings.
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

#include <Geometry/OrthogonalTransformation.icpp>

namespace Geometry {

/**********************************************************************************
Force instantiation of all standard OrthogonalTransformation classes and functions:
**********************************************************************************/

template class OrthogonalTransformation<float,2>;
template OrthogonalTransformation<float,2>::OrthogonalTransformation(const TranslationTransformation<float,2>&);
template OrthogonalTransformation<float,2>::OrthogonalTransformation(const RotationTransformation<float,2>&);
template OrthogonalTransformation<float,2>::OrthogonalTransformation(const OrthonormalTransformation<float,2>&);
template OrthogonalTransformation<float,2>::OrthogonalTransformation(const UniformScalingTransformation<float,2>&);
template OrthogonalTransformation<float,2>::OrthogonalTransformation(const TranslationTransformation<double,2>&);
template OrthogonalTransformation<float,2>::OrthogonalTransformation(const RotationTransformation<double,2>&);
template OrthogonalTransformation<float,2>::OrthogonalTransformation(const OrthonormalTransformation<double,2>&);
template OrthogonalTransformation<float,2>::OrthogonalTransformation(const UniformScalingTransformation<double,2>&);

template class OrthogonalTransformation<double,2>;
template OrthogonalTransformation<double,2>::OrthogonalTransformation(const TranslationTransformation<float,2>&);
template OrthogonalTransformation<double,2>::OrthogonalTransformation(const RotationTransformation<float,2>&);
template OrthogonalTransformation<double,2>::OrthogonalTransformation(const OrthonormalTransformation<float,2>&);
template OrthogonalTransformation<double,2>::OrthogonalTransformation(const UniformScalingTransformation<float,2>&);
template OrthogonalTransformation<double,2>::OrthogonalTransformation(const TranslationTransformation<double,2>&);
template OrthogonalTransformation<double,2>::OrthogonalTransformation(const RotationTransformation<double,2>&);
template OrthogonalTransformation<double,2>::OrthogonalTransformation(const OrthonormalTransformation<double,2>&);
template OrthogonalTransformation<double,2>::OrthogonalTransformation(const UniformScalingTransformation<double,2>&);

template class OrthogonalTransformation<float,3>;
template OrthogonalTransformation<float,3>::OrthogonalTransformation(const TranslationTransformation<float,3>&);
template OrthogonalTransformation<float,3>::OrthogonalTransformation(const RotationTransformation<float,3>&);
template OrthogonalTransformation<float,3>::OrthogonalTransformation(const OrthonormalTransformation<float,3>&);
template OrthogonalTransformation<float,3>::OrthogonalTransformation(const UniformScalingTransformation<float,3>&);
template OrthogonalTransformation<float,3>::OrthogonalTransformation(const TranslationTransformation<double,3>&);
template OrthogonalTransformation<float,3>::OrthogonalTransformation(const RotationTransformation<double,3>&);
template OrthogonalTransformation<float,3>::OrthogonalTransformation(const OrthonormalTransformation<double,3>&);
template OrthogonalTransformation<float,3>::OrthogonalTransformation(const UniformScalingTransformation<double,3>&);

template class OrthogonalTransformation<double,3>;
template OrthogonalTransformation<double,3>::OrthogonalTransformation(const TranslationTransformation<float,3>&);
template OrthogonalTransformation<double,3>::OrthogonalTransformation(const RotationTransformation<float,3>&);
template OrthogonalTransformation<double,3>::OrthogonalTransformation(const OrthonormalTransformation<float,3>&);
template OrthogonalTransformation<double,3>::OrthogonalTransformation(const UniformScalingTransformation<float,3>&);
template OrthogonalTransformation<double,3>::OrthogonalTransformation(const TranslationTransformation<double,3>&);
template OrthogonalTransformation<double,3>::OrthogonalTransformation(const RotationTransformation<double,3>&);
template OrthogonalTransformation<double,3>::OrthogonalTransformation(const OrthonormalTransformation<double,3>&);
template OrthogonalTransformation<double,3>::OrthogonalTransformation(const UniformScalingTransformation<double,3>&);

}
