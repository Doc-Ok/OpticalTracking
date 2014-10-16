/***********************************************************************
Random - Functions to create random points or vectors according to
several probability distributions.
Copyright (c) 2007-2010 Oliver Kreylos

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

#include <Geometry/Random.icpp>

namespace Geometry {

/*********************************************
Force instantiation of all standard functions:
*********************************************/

template Point<float,2> randPointUniformCO<float,2>(const Point<float,2>&,const Point<float,2>&);
template Point<double,2> randPointUniformCO<double,2>(const Point<double,2>&,const Point<double,2>&);
template Point<float,3> randPointUniformCO<float,3>(const Point<float,3>&,const Point<float,3>&);
template Point<double,3> randPointUniformCO<double,3>(const Point<double,3>&,const Point<double,3>&);

template Point<float,2> randPointUniformCC<float,2>(const Point<float,2>&,const Point<float,2>&);
template Point<double,2> randPointUniformCC<double,2>(const Point<double,2>&,const Point<double,2>&);
template Point<float,3> randPointUniformCC<float,3>(const Point<float,3>&,const Point<float,3>&);
template Point<double,3> randPointUniformCC<double,3>(const Point<double,3>&,const Point<double,3>&);

template Point<float,2> randPointUniformCO<float,2>(const Box<float,2>&);
template Point<double,2> randPointUniformCO<double,2>(const Box<double,2>&);
template Point<float,3> randPointUniformCO<float,3>(const Box<float,3>&);
template Point<double,3> randPointUniformCO<double,3>(const Box<double,3>&);

template Point<float,2> randPointUniformCC<float,2>(const Box<float,2>&);
template Point<double,2> randPointUniformCC<double,2>(const Box<double,2>&);
template Point<float,3> randPointUniformCC<float,3>(const Box<float,3>&);
template Point<double,3> randPointUniformCC<double,3>(const Box<double,3>&);

template Vector<float,2> randVectorUniform<float,2>(float);
template Vector<double,2> randVectorUniform<double,2>(double);
template Vector<float,3> randVectorUniform<float,3>(float);
template Vector<double,3> randVectorUniform<double,3>(double);

template Vector<float,2> randUnitVectorUniform<float,2>(void);
template Vector<double,2> randUnitVectorUniform<double,2>(void);
template Vector<float,3> randUnitVectorUniform<float,3>(void);
template Vector<double,3> randUnitVectorUniform<double,3>(void);

template Vector<float,2> randVectorNormal<float,2>(float);
template Vector<double,2> randVectorNormal<double,2>(double);
template Vector<float,3> randVectorNormal<float,3>(float);
template Vector<double,3> randVectorNormal<double,3>(double);

}
