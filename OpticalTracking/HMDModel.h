/***********************************************************************
HMDModel - Class to represent 3D models of head-mounted displays for
optical tracking purposes, including LED positions and emission vectors
and the position of the integrated inertial measurement unit.
Copyright (c) 2014 Oliver Kreylos

This file is part of the optical/inertial sensor fusion tracking
package.

The optical/inertial sensor fusion tracking package is free software;
you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

The optical/inertial sensor fusion tracking package is distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the optical/inertial sensor fusion tracking package; if not, write
to the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef HMDMODEL_INCLUDED
#define HMDMODEL_INCLUDED

#include <Geometry/Point.h>
#include <Geometry/Vector.h>

/* Forward declarations: */
namespace RawHID {
class Device;
}

class HMDModel
	{
	/* Embedded classes: */
	public:
	typedef float Scalar;
	typedef Geometry::Point<Scalar,3> Point;
	typedef Geometry::Vector<Scalar,3> Vector;
	
	struct Marker // Structure to represent a single passive tracking marker or active tracking LED
		{
		/* Elements: */
		public:
		unsigned int pattern; // 10-bit blinking pattern used to identify this marker
		Point pos; // Marker position in HMD coordinate system in meters
		Vector dir; // Direction of optimal marker visibility or LED emission
		};
	
	/* Elements: */
	private:
	Point imu; // Position of the integrated IMU in HMD coordinate system in meters
	unsigned int numMarkers; // Number of tracking markers attached to the HMD
	Marker* markers; // Array of tracking markers attached to the HMD
	unsigned char* patternTable; // Look-up table to translate blinked 10-bit pattern into marker indices
	
	/* Constructors and destructors: */
	public:
	HMDModel(void); // Creates an empty HMD model
	private:
	HMDModel(const HMDModel& source); // Prohibit copy constructor
	HMDModel& operator=(const HMDModel& source); // Prohibit assignment operator
	public:
	~HMDModel(void); // Destroys the HMD model
	
	/* Methods: */
	void readFromRiftDK2(RawHID::Device& rift); // Reads HMD model from an Oculus Rift DK2 via HID feature reports
	
	const Point& getIMU(void) const // Returns the IMU position
		{
		return imu;
		}
	unsigned int getNumMarkers(void) const // Returns the number of tracking markers
		{
		return numMarkers;
		}
	unsigned int getMarkerPattern(unsigned int markerIndex) const // Returns the 10-bit blinking pattern of the given tracking marker
		{
		return markers[markerIndex].pattern;
		}
	unsigned int getMarkerIndex(unsigned int pattern) const // Returns the marker index associated with the given 10-bit pattern, or ~0 if the pattern is invalid
		{
		return patternTable[pattern];
		}
	const Point& getMarkerPos(unsigned int markerIndex) const // Returns the position of the given tracking marker
		{
		return markers[markerIndex].pos;
		}
	const Vector& getMarkerDir(unsigned int markerIndex) const // Returns the direction of the given tracking marker
		{
		return markers[markerIndex].dir;
		}
	};

#endif
