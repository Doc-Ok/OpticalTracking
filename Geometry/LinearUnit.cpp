/***********************************************************************
LinearUnit - Class defining units of linear measurement, and conversions
between them.
Copyright (c) 2010 Oliver Kreylos

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

#include <Geometry/LinearUnit.h>

#include <string.h>

namespace Geometry {

/***********************************
Static elements of class LinearUnit:
***********************************/

const char* LinearUnit::names[]=
	{
	"",
	"picometer","nanometer","micrometer","millimeter","centimeter","meter","kilometer",
	"point","inch","foot","yard","mile",
	"Angstrom","potrzebie","smoot","astronomical unit","light year","parsec"
	};

const char* LinearUnit::abbreviations[]=
	{
	"",
	"pm","nm","um","mm","cm","m","km",
	"pt","in","ft","yd","mi",
	"A","pz","st","au","ly","pc"
	};

const bool LinearUnit::metrics[]=
	{
	false,
	true,true,true,true,true,true,true,
	false,false,false,false,false,
	false,false,false,false,false,false
	};

const LinearUnit::Scalar LinearUnit::meterFactors[]=
	{
	1.0e3/25.4,
	1.0e12,1.0e9,1.0e6,1.0e3,1.0e2,1.0,1.0e-3,
	1.0e3*72.0/25.4,1.0e3/25.4,1.0e3/(25.4*12.0),1.0e3/(25.4*36.0),1.0e3/(25.4*36.0*1760.0),
	1.0e10,1.0e3/2.263348517438173216473,1.0e3/(25.4*67.0),1.0/149597870691.0,
	1.0e-3/9460730472580.8,1.0/3.085678e16
	};

const bool LinearUnit::imperials[]=
	{
	false,
	false,false,false,false,false,false,false,
	true,true,true,true,true,
	false,false,false,false,false,false
	};

const LinearUnit::Scalar LinearUnit::inchFactors[]=
	{
	1,
	25.4e9,25.4e6,25.4e3,25.4,25.4e-1,25.4e-3,25.4e-6,
	72,1,1.0/12.0,1.0/36.0,1.0/(36.0*1760.0),
	25.4e7,25.4/2.263348517438173216473,1.0/67.0,25.4e-3/149597870691.0,
	25.4e-6/9460730472580.8,25.4e-3/3.085678e16
	};

/***************************
Methods of class LinearUnit:
***************************/

LinearUnit::LinearUnit(const char* name,LinearUnit::Scalar sFactor)
	:unit(UNKNOWN),factor(sFactor)
	{
	/* Search among full unit names first: */
	for(int i=0;i<NUM_UNITS&&unit==UNKNOWN;++i)
		if(strcasecmp(names[i],name)==0)
			unit=(Unit)i;
	
	/* Search among abbreviated unit names next: */
	for(int i=0;i<NUM_UNITS&&unit==UNKNOWN;++i)
		if(strcasecmp(abbreviations[i],name)==0)
			unit=(Unit)i;
	}

}
