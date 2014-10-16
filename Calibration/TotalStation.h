/***********************************************************************
TotalStation - Class to represent a Leica Total Station survey
instrument to measure points for coordinate system calibration.
Copyright (c) 2009-2011 Oliver Kreylos

This file is part of the Vrui calibration utility package.

The Vrui calibration utility package is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Vrui calibration utility package is distributed in the hope that it
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Vrui calibration utility package; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef TOTALSTATION_INCLUDED
#define TOTALSTATION_INCLUDED

#include <string>
#include <Comm/SerialPort.h>
#include <Geometry/Point.h>

class TotalStation
	{
	/* Embedded classes: */
	private:
	enum AngleUnit
		{
		GON,DEGREE_DECIMAL,DEGREE_SEXAGESIMAL,MIL,RADIAN
		};
	
	enum HorizAngleDirection
		{
		CLOCKWISE,COUNTER_CLOCKWISE
		};
	
	enum DistanceUnit
		{
		METER,USFOOT_DECIMAL,INTLFOOT_DECIMAL,USFOOT_INCH,INTLFOOT_INCH
		};
	
	public:
	typedef double Scalar;
	typedef Geometry::Point<double,3> Point;
	
	/* Elements: */
	private:
	Comm::SerialPort devicePort; // Serial port to which the Total Station is connected
	AngleUnit angleUnit; // Total Station's angle measurement unit
	HorizAngleDirection horizAngleDirection; // Direction of increasing horizontal angles
	DistanceUnit distanceUnit; // Total Station's distance measurement unit
	Scalar unitScale; // Scaling factor for measurements and distances used at the Total Station API
	Scalar prismOffset; // The prism offset configured in the Total Station
	
	/* Private methods: */
	void sendRequest(const char* format,...);
	std::string readReply(void);
	unsigned int conf(unsigned int confId);
	std::string getString(unsigned int getId,bool requestMeasurement);
	unsigned int getUInt(unsigned int getId,bool requestMeasurement);
	int getInt(unsigned int getId,bool requestMeasurement);
	double getMeasurement(unsigned int getId,bool requestMeasurement);
	void getMultiMeasurement(size_t numGets,const unsigned int getIds[],double values[],bool requestMeasurement);
	Point calcCartesianPoint(const double measurements[3]) const;
	
	/* Constructors and destructors: */
	public:
	TotalStation(const char* devicePortName,int deviceBaudRate); // Connects to a Total Station on the given serial port with the given baud rate
	~TotalStation(void); // Disconnects from the Total Station
	
	/* Methods: */
	Scalar getUnitScale(void) const // Returns the scaling factor from Total Station's internal distance unit to reported unit
		{
		return unitScale;
		}
	Scalar getPrismOffset(void); // Returns the Total Station's currently configured prism offset in millimeters
	unsigned int getEDMMode(void); // Returns the Total Station's currently configured EDM (laser range finder) mode
	void setUnitScale(Scalar newUnitScale); // Sets the scaling factor from Total Station's internal distance unit to reported unit
	void setPrismOffset(Scalar newPrismOffset); // Sets the Total Station's prism offset in millimeters
	void setEDMMode(unsigned int newEDMMode); // Sets the Total Station's EDM mode
	Point getLastMeasurement(void); // Returns the last valid measurement in Cartesian coordinates; throws exception if there is no valid measurement
	Point requestMeasurement(void); // Takes and returns a measurement in Cartesian coordinates
	void startRecording(void); // Records any measurements made from the Total Station
	void stopRecording(void); // Stops recording mode
	Point readNextMeasurement(void); // Waits for and returns the next measurement in recording mode
	};

#endif

