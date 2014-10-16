/***********************************************************************
AffineTransformation - Class for general affine transformations.
Copyright (c) 2001-2011 Oliver Kreylos

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

#include <Geometry/AffineTransformation.icpp>

namespace Geometry {

/******************************************************************************
Force instantiation of all standard AffineTransformation classes and functions:
******************************************************************************/

template class AffineTransformationOperations<float,2>;
template class AffineTransformationOperations<double,2>;
template class AffineTransformationOperations<float,3>;
template class AffineTransformationOperations<double,3>;

template class AffineTransformation<float,2>;
template AffineTransformation<float,2>::AffineTransformation(const TranslationTransformation<float,2>&);
template AffineTransformation<float,2>::AffineTransformation(const RotationTransformation<float,2>&);
template AffineTransformation<float,2>::AffineTransformation(const OrthonormalTransformation<float,2>&);
template AffineTransformation<float,2>::AffineTransformation(const UniformScalingTransformation<float,2>&);
template AffineTransformation<float,2>::AffineTransformation(const OrthogonalTransformation<float,2>&);
template AffineTransformation<float,2>::AffineTransformation(const ScalingTransformation<float,2>&);
template AffineTransformation<float,2>::AffineTransformation(const TranslationTransformation<double,2>&);
template AffineTransformation<float,2>::AffineTransformation(const RotationTransformation<double,2>&);
template AffineTransformation<float,2>::AffineTransformation(const OrthonormalTransformation<double,2>&);
template AffineTransformation<float,2>::AffineTransformation(const UniformScalingTransformation<double,2>&);
template AffineTransformation<float,2>::AffineTransformation(const OrthogonalTransformation<double,2>&);
template AffineTransformation<float,2>::AffineTransformation(const ScalingTransformation<double,2>&);
template AffineTransformation<float,2> operator*(const AffineTransformation<float,2>&,const AffineTransformation<float,2>&);
template AffineTransformation<float,2> invert(const AffineTransformation<float,2>&);

template class AffineTransformation<double,2>;
template AffineTransformation<double,2>::AffineTransformation(const TranslationTransformation<float,2>&);
template AffineTransformation<double,2>::AffineTransformation(const RotationTransformation<float,2>&);
template AffineTransformation<double,2>::AffineTransformation(const OrthonormalTransformation<float,2>&);
template AffineTransformation<double,2>::AffineTransformation(const UniformScalingTransformation<float,2>&);
template AffineTransformation<double,2>::AffineTransformation(const OrthogonalTransformation<float,2>&);
template AffineTransformation<double,2>::AffineTransformation(const ScalingTransformation<float,2>&);
template AffineTransformation<double,2>::AffineTransformation(const TranslationTransformation<double,2>&);
template AffineTransformation<double,2>::AffineTransformation(const RotationTransformation<double,2>&);
template AffineTransformation<double,2>::AffineTransformation(const OrthonormalTransformation<double,2>&);
template AffineTransformation<double,2>::AffineTransformation(const UniformScalingTransformation<double,2>&);
template AffineTransformation<double,2>::AffineTransformation(const OrthogonalTransformation<double,2>&);
template AffineTransformation<double,2>::AffineTransformation(const ScalingTransformation<double,2>&);
template AffineTransformation<double,2> operator*(const AffineTransformation<double,2>&,const AffineTransformation<double,2>&);
template AffineTransformation<double,2> invert(const AffineTransformation<double,2>&);

template class AffineTransformation<float,3>;
template AffineTransformation<float,3>::AffineTransformation(const TranslationTransformation<float,3>&);
template AffineTransformation<float,3>::AffineTransformation(const RotationTransformation<float,3>&);
template AffineTransformation<float,3>::AffineTransformation(const OrthonormalTransformation<float,3>&);
template AffineTransformation<float,3>::AffineTransformation(const UniformScalingTransformation<float,3>&);
template AffineTransformation<float,3>::AffineTransformation(const OrthogonalTransformation<float,3>&);
template AffineTransformation<float,3>::AffineTransformation(const ScalingTransformation<float,3>&);
template AffineTransformation<float,3>::AffineTransformation(const TranslationTransformation<double,3>&);
template AffineTransformation<float,3>::AffineTransformation(const RotationTransformation<double,3>&);
template AffineTransformation<float,3>::AffineTransformation(const OrthonormalTransformation<double,3>&);
template AffineTransformation<float,3>::AffineTransformation(const UniformScalingTransformation<double,3>&);
template AffineTransformation<float,3>::AffineTransformation(const OrthogonalTransformation<double,3>&);
template AffineTransformation<float,3>::AffineTransformation(const ScalingTransformation<double,3>&);
template AffineTransformation<float,3> operator*(const AffineTransformation<float,3>&,const AffineTransformation<float,3>&);
template AffineTransformation<float,3> invert(const AffineTransformation<float,3>&);

template class AffineTransformation<double,3>;
template AffineTransformation<double,3>::AffineTransformation(const TranslationTransformation<float,3>&);
template AffineTransformation<double,3>::AffineTransformation(const RotationTransformation<float,3>&);
template AffineTransformation<double,3>::AffineTransformation(const OrthonormalTransformation<float,3>&);
template AffineTransformation<double,3>::AffineTransformation(const UniformScalingTransformation<float,3>&);
template AffineTransformation<double,3>::AffineTransformation(const OrthogonalTransformation<float,3>&);
template AffineTransformation<double,3>::AffineTransformation(const ScalingTransformation<float,3>&);
template AffineTransformation<double,3>::AffineTransformation(const TranslationTransformation<double,3>&);
template AffineTransformation<double,3>::AffineTransformation(const RotationTransformation<double,3>&);
template AffineTransformation<double,3>::AffineTransformation(const OrthonormalTransformation<double,3>&);
template AffineTransformation<double,3>::AffineTransformation(const UniformScalingTransformation<double,3>&);
template AffineTransformation<double,3>::AffineTransformation(const OrthogonalTransformation<double,3>&);
template AffineTransformation<double,3>::AffineTransformation(const ScalingTransformation<double,3>&);
template AffineTransformation<double,3> operator*(const AffineTransformation<double,3>&,const AffineTransformation<double,3>&);
template AffineTransformation<double,3> invert(const AffineTransformation<double,3>&);

}
