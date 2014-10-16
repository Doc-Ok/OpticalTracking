/***********************************************************************
ScalingTransformation - Class for n-dimensional non-uniform scaling
transformations.
Copyright (c) 2003-2011 Oliver Kreylos

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

#include <Geometry/ScalingTransformation.icpp>

namespace Geometry {

/*******************************************************************************
Force instantiation of all standard ScalingTransformation classes and functions:
*******************************************************************************/

template class ScalingTransformation<float,2>;
template ScalingTransformation<float,2>::ScalingTransformation(const UniformScalingTransformation<float,2>&);
template ScalingTransformation<float,2>::ScalingTransformation(const UniformScalingTransformation<double,2>&);

template class ScalingTransformation<double,2>;
template ScalingTransformation<double,2>::ScalingTransformation(const UniformScalingTransformation<float,2>&);
template ScalingTransformation<double,2>::ScalingTransformation(const UniformScalingTransformation<double,2>&);

template class ScalingTransformation<float,3>;
template ScalingTransformation<float,3>::ScalingTransformation(const UniformScalingTransformation<float,3>&);
template ScalingTransformation<float,3>::ScalingTransformation(const UniformScalingTransformation<double,3>&);

template class ScalingTransformation<double,3>;
template ScalingTransformation<double,3>::ScalingTransformation(const UniformScalingTransformation<float,3>&);
template ScalingTransformation<double,3>::ScalingTransformation(const UniformScalingTransformation<double,3>&);

}
