/***********************************************************************
Rotation - Class for 2D and 3D rotations.
Copyright (c) 2002-2013 Oliver Kreylos

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

#include <Geometry/Rotation.icpp>

namespace Geometry {

/******************************************************************
Force instantiation of all standard Rotation classes and functions:
******************************************************************/

template class Rotation<float,2>;

template class Rotation<double,2>;

template class Rotation<float,3>;
template Rotation<float,3> Rotation<float,3>::fromMatrix(const Matrix<float,3,3>&);
template Rotation<float,3> Rotation<float,3>::fromMatrix(const Matrix<float,3,4>&);
template Rotation<float,3> Rotation<float,3>::fromMatrix(const Matrix<float,4,4>&);
template Rotation<float,3> Rotation<float,3>::fromMatrix(const Matrix<double,3,3>&);
template Rotation<float,3> Rotation<float,3>::fromMatrix(const Matrix<double,3,4>&);
template Rotation<float,3> Rotation<float,3>::fromMatrix(const Matrix<double,4,4>&);
template Matrix<float,3,3>& Rotation<float,3>::writeMatrix(Matrix<float,3,3>&) const;
template Matrix<float,3,4>& Rotation<float,3>::writeMatrix(Matrix<float,3,4>&) const;
template Matrix<float,4,4>& Rotation<float,3>::writeMatrix(Matrix<float,4,4>&) const;
template Matrix<double,3,3>& Rotation<float,3>::writeMatrix(Matrix<double,3,3>&) const;
template Matrix<double,3,4>& Rotation<float,3>::writeMatrix(Matrix<double,3,4>&) const;
template Matrix<double,4,4>& Rotation<float,3>::writeMatrix(Matrix<double,4,4>&) const;
template Rotation<float,3> operator*(const Rotation<float,3>&,const Rotation<float,3>&);
template Rotation<float,3> operator/(const Rotation<float,3>&,const Rotation<float,3>&);

template class Rotation<double,3>;
template Rotation<double,3> Rotation<double,3>::fromMatrix(const Matrix<float,3,3>&);
template Rotation<double,3> Rotation<double,3>::fromMatrix(const Matrix<float,3,4>&);
template Rotation<double,3> Rotation<double,3>::fromMatrix(const Matrix<float,4,4>&);
template Rotation<double,3> Rotation<double,3>::fromMatrix(const Matrix<double,3,3>&);
template Rotation<double,3> Rotation<double,3>::fromMatrix(const Matrix<double,3,4>&);
template Rotation<double,3> Rotation<double,3>::fromMatrix(const Matrix<double,4,4>&);
template Matrix<float,3,3>& Rotation<double,3>::writeMatrix(Matrix<float,3,3>&) const;
template Matrix<float,3,4>& Rotation<double,3>::writeMatrix(Matrix<float,3,4>&) const;
template Matrix<float,4,4>& Rotation<double,3>::writeMatrix(Matrix<float,4,4>&) const;
template Matrix<double,3,3>& Rotation<double,3>::writeMatrix(Matrix<double,3,3>&) const;
template Matrix<double,3,4>& Rotation<double,3>::writeMatrix(Matrix<double,3,4>&) const;
template Matrix<double,4,4>& Rotation<double,3>::writeMatrix(Matrix<double,4,4>&) const;
template Rotation<double,3> operator*(const Rotation<double,3>&,const Rotation<double,3>&);
template Rotation<double,3> operator/(const Rotation<double,3>&,const Rotation<double,3>&);

}
