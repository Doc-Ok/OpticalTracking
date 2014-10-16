/***********************************************************************
Random - Functions to create random numbers of several probability
distributions.
Copyright (c) 2003-2005 Oliver Kreylos

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

#ifndef MATH_RANDOM_INCLUDED
#define MATH_RANDOM_INCLUDED

namespace Math {

double randUniformCO(void); // Uniform distribution in interval [0,1)
double randUniformCC(void); // Uniform distribution in interval [0,1]
int randUniformCO(int min,int max); // Uniform distribution in interval [min,max)
double randUniformCO(double min,double max); // Uniform distribution in interval [min,max)
int randUniformCC(int min,int max); // Uniform distribution in interval [min,max]
double randUniformCC(double min,double max); // Uniform distribution in interval [min,max]
double randNormal(double mean,double stddev); // Normal distribution with mean and standard deviation

}

#endif
