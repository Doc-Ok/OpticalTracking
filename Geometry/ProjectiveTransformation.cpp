/***********************************************************************
ProjectiveTransformation - Class for n-dimensional projective
transformations.
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

#include <Geometry/ProjectiveTransformation.icpp>

namespace Geometry {

/**********************************************************************************
Force instantiation of all standard ProjectiveTransformation classes and functions:
**********************************************************************************/

template class ProjectiveTransformationOperations<float,2>;
template class ProjectiveTransformationOperations<double,2>;
template class ProjectiveTransformationOperationsBase<float,3>;
template class ProjectiveTransformationOperationsBase<double,3>;
template class ProjectiveTransformationOperations<float,3>;
template class ProjectiveTransformationOperations<double,3>;

template class ProjectiveTransformation<float,2>;
template ProjectiveTransformation<float,2>::ProjectiveTransformation(const TranslationTransformation<float,2>&);
template ProjectiveTransformation<float,2>::ProjectiveTransformation(const RotationTransformation<float,2>&);
template ProjectiveTransformation<float,2>::ProjectiveTransformation(const OrthonormalTransformation<float,2>&);
template ProjectiveTransformation<float,2>::ProjectiveTransformation(const UniformScalingTransformation<float,2>&);
template ProjectiveTransformation<float,2>::ProjectiveTransformation(const OrthogonalTransformation<float,2>&);
template ProjectiveTransformation<float,2>::ProjectiveTransformation(const ScalingTransformation<float,2>&);
template ProjectiveTransformation<float,2>::ProjectiveTransformation(const AffineTransformation<float,2>&);
template ProjectiveTransformation<float,2>::ProjectiveTransformation(const TranslationTransformation<double,2>&);
template ProjectiveTransformation<float,2>::ProjectiveTransformation(const RotationTransformation<double,2>&);
template ProjectiveTransformation<float,2>::ProjectiveTransformation(const OrthonormalTransformation<double,2>&);
template ProjectiveTransformation<float,2>::ProjectiveTransformation(const UniformScalingTransformation<double,2>&);
template ProjectiveTransformation<float,2>::ProjectiveTransformation(const OrthogonalTransformation<double,2>&);
template ProjectiveTransformation<float,2>::ProjectiveTransformation(const ScalingTransformation<double,2>&);
template ProjectiveTransformation<float,2>::ProjectiveTransformation(const AffineTransformation<double,2>&);

template class ProjectiveTransformation<double,2>;
template ProjectiveTransformation<double,2>::ProjectiveTransformation(const TranslationTransformation<float,2>&);
template ProjectiveTransformation<double,2>::ProjectiveTransformation(const RotationTransformation<float,2>&);
template ProjectiveTransformation<double,2>::ProjectiveTransformation(const OrthonormalTransformation<float,2>&);
template ProjectiveTransformation<double,2>::ProjectiveTransformation(const UniformScalingTransformation<float,2>&);
template ProjectiveTransformation<double,2>::ProjectiveTransformation(const OrthogonalTransformation<float,2>&);
template ProjectiveTransformation<double,2>::ProjectiveTransformation(const ScalingTransformation<float,2>&);
template ProjectiveTransformation<double,2>::ProjectiveTransformation(const AffineTransformation<float,2>&);
template ProjectiveTransformation<double,2>::ProjectiveTransformation(const TranslationTransformation<double,2>&);
template ProjectiveTransformation<double,2>::ProjectiveTransformation(const RotationTransformation<double,2>&);
template ProjectiveTransformation<double,2>::ProjectiveTransformation(const OrthonormalTransformation<double,2>&);
template ProjectiveTransformation<double,2>::ProjectiveTransformation(const UniformScalingTransformation<double,2>&);
template ProjectiveTransformation<double,2>::ProjectiveTransformation(const OrthogonalTransformation<double,2>&);
template ProjectiveTransformation<double,2>::ProjectiveTransformation(const ScalingTransformation<double,2>&);
template ProjectiveTransformation<double,2>::ProjectiveTransformation(const AffineTransformation<double,2>&);

template class ProjectiveTransformation<float,3>;
template ProjectiveTransformation<float,3>::ProjectiveTransformation(const TranslationTransformation<float,3>&);
template ProjectiveTransformation<float,3>::ProjectiveTransformation(const RotationTransformation<float,3>&);
template ProjectiveTransformation<float,3>::ProjectiveTransformation(const OrthonormalTransformation<float,3>&);
template ProjectiveTransformation<float,3>::ProjectiveTransformation(const UniformScalingTransformation<float,3>&);
template ProjectiveTransformation<float,3>::ProjectiveTransformation(const OrthogonalTransformation<float,3>&);
template ProjectiveTransformation<float,3>::ProjectiveTransformation(const ScalingTransformation<float,3>&);
template ProjectiveTransformation<float,3>::ProjectiveTransformation(const AffineTransformation<float,3>&);
template ProjectiveTransformation<float,3>::ProjectiveTransformation(const TranslationTransformation<double,3>&);
template ProjectiveTransformation<float,3>::ProjectiveTransformation(const RotationTransformation<double,3>&);
template ProjectiveTransformation<float,3>::ProjectiveTransformation(const OrthonormalTransformation<double,3>&);
template ProjectiveTransformation<float,3>::ProjectiveTransformation(const UniformScalingTransformation<double,3>&);
template ProjectiveTransformation<float,3>::ProjectiveTransformation(const OrthogonalTransformation<double,3>&);
template ProjectiveTransformation<float,3>::ProjectiveTransformation(const ScalingTransformation<double,3>&);
template ProjectiveTransformation<float,3>::ProjectiveTransformation(const AffineTransformation<double,3>&);

template class ProjectiveTransformation<double,3>;
template ProjectiveTransformation<double,3>::ProjectiveTransformation(const TranslationTransformation<float,3>&);
template ProjectiveTransformation<double,3>::ProjectiveTransformation(const RotationTransformation<float,3>&);
template ProjectiveTransformation<double,3>::ProjectiveTransformation(const OrthonormalTransformation<float,3>&);
template ProjectiveTransformation<double,3>::ProjectiveTransformation(const UniformScalingTransformation<float,3>&);
template ProjectiveTransformation<double,3>::ProjectiveTransformation(const OrthogonalTransformation<float,3>&);
template ProjectiveTransformation<double,3>::ProjectiveTransformation(const ScalingTransformation<float,3>&);
template ProjectiveTransformation<double,3>::ProjectiveTransformation(const AffineTransformation<float,3>&);
template ProjectiveTransformation<double,3>::ProjectiveTransformation(const TranslationTransformation<double,3>&);
template ProjectiveTransformation<double,3>::ProjectiveTransformation(const RotationTransformation<double,3>&);
template ProjectiveTransformation<double,3>::ProjectiveTransformation(const OrthonormalTransformation<double,3>&);
template ProjectiveTransformation<double,3>::ProjectiveTransformation(const UniformScalingTransformation<double,3>&);
template ProjectiveTransformation<double,3>::ProjectiveTransformation(const OrthogonalTransformation<double,3>&);
template ProjectiveTransformation<double,3>::ProjectiveTransformation(const ScalingTransformation<double,3>&);
template ProjectiveTransformation<double,3>::ProjectiveTransformation(const AffineTransformation<double,3>&);

}
