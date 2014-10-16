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

#ifndef GEOMETRY_LINEARUNIT_INCLUDED
#define GEOMETRY_LINEARUNIT_INCLUDED

namespace Geometry {

class LinearUnit
	{
	/* Embedded classes: */
	public:
	enum Unit // Enumerated type for linear units
		{
		UNKNOWN=0,
		
		/* Metric units: */
		PICOMETER,NANOMETER,MICROMETER,MILLIMETER,CENTIMETER,METER,KILOMETER,
		
		/* Imperial units: */
		POINT,INCH,FOOT,YARD,MILE,
		
		/* Other units: */
		ANGSTROM,POTRZEBIE,SMOOT,ASTRONOMICAL_UNIT,LIGHT_YEAR,PARSEC,
		
		/* List terminator: */
		NUM_UNITS
		};
	
	typedef double Scalar; // Scalar type for coordinates
	
	/* Elements: */
	private:
	
	/* Static unit definitions: */
	static const char* names[NUM_UNITS]; // Full names of linear units for display
	static const char* abbreviations[NUM_UNITS]; // Abbreviated names of linear units for display
	static const bool metrics[NUM_UNITS]; // Flags whether each unit is metric
	static const Scalar meterFactors[NUM_UNITS]; // Conversion factors from linear units to meters
	static const bool imperials[NUM_UNITS]; // Flags whether each unit is imperial
	static const Scalar inchFactors[NUM_UNITS]; // Conversion factors from linear units to inches
	
	/* Unit specification: */
	public:
	Unit unit; // Linear unit
	Scalar factor; // Multiplication factor for linear coordinates, i.e., 1 coordinate = unitFactor * unit
	
	/* Constructors and destructors: */
	LinearUnit(void) // Creates an unknown unit
		:unit(UNKNOWN),factor(1)
		{
		}
	LinearUnit(Unit sUnit,Scalar sFactor) // Elementwise initialization
		:unit(sUnit),factor(sFactor)
		{
		}
	LinearUnit(const char* name,Scalar sFactor); // Sets unit from full or abbreviated name
	
	/* Methods: */
	const char* getName(void) const // Returns the unit's full name
		{
		return names[unit];
		}
	const char* getAbbreviation(void) const // Returns the unit's abbreviated name
		{
		return abbreviations[unit];
		}
	Scalar getFactor(void) const // Returns the unit's scaling factor
		{
		return factor;
		}
	bool isMetric(void) const // Returns true if the unit is metric
		{
		return metrics[unit];
		}
	Scalar getMeterFactor(void) const // Returns length of a meter in scaled linear units
		{
		return meterFactors[unit]/factor;
		}
	bool isImperial(void) const // Returns true if the unit is imperial
		{
		return imperials[unit];
		}
	Scalar getInchFactor(void) const // Returns length of an inch in scaled linear units
		{
		return inchFactors[unit]/factor;
		}
	Scalar convert(Scalar coordinate,const LinearUnit& other) const // Converts the given coordinate to this unit
		{
		return coordinate*other.factor*meterFactors[other.unit]/(factor*meterFactors[unit]);
		}
	};

}

#endif
