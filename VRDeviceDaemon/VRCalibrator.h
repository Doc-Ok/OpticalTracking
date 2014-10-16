/***********************************************************************
VRCalibrator - Abstract base class for classes calibrating tracker
positions and orientations.
Copyright (c) 2002-2010 Oliver Kreylos

This file is part of the Vrui VR Device Driver Daemon (VRDeviceDaemon).

The Vrui VR Device Driver Daemon is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Vrui VR Device Driver Daemon is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Vrui VR Device Driver Daemon; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef VRCALIBRATOR_INCLUDED
#define VRCALIBRATOR_INCLUDED

#include <Vrui/Internal/VRDeviceState.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFile;
}
template <class BaseClassParam>
class VRFactory;

class VRCalibrator
	{
	/* Embedded classes: */
	public:
	typedef VRFactory<VRCalibrator> Factory;
	
	/* Elements: */
	private:
	Factory* factory; // Pointer to factory that created this object
	protected:
	bool calibratePositions; // Enable flag for position calibration
	bool calibrateOrientations; // Enable flag for orientation calibration
	bool calibrateVelocities; // Enable flag for velocity calibration
	
	/* Constructors and destructors: */
	public:
	VRCalibrator(Factory* sFactory,Misc::ConfigurationFile& configFile);
	virtual ~VRCalibrator(void);
	static void destroy(VRCalibrator* object); // Destroys an object
	
	/* Methods: */
	virtual void setNumTrackers(int newNumTrackers); // Sets the number of trackers on the associated device
	virtual Vrui::VRDeviceState::TrackerState& calibrate(int deviceTrackerIndex,Vrui::VRDeviceState::TrackerState& rawState) =0; // Calibrates a raw tracker measurement
	};

#endif
