/***********************************************************************
GridCalibrator - Class for calibrators using a curvilinear grid of
tracker measurements with position and orientation corrections.
Copyright (c) 2004-2010 Oliver Kreylos

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

#ifndef GRIDCALIBRATOR_INCLUDED
#define GRIDCALIBRATOR_INCLUDED

#include <Vrui/Internal/VRDeviceState.h>

#include <VRDeviceDaemon/VRCalibrator.h>
#include <VRDeviceDaemon/VRCalibrators/Curvilinear.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFile;
}

class GridCalibrator:public VRCalibrator
	{
	/* Embedded classes: */
	public:
	typedef Vrui::VRDeviceState::TrackerState::PositionOrientation PositionOrientation; // Data type for tracker position/orientation
	typedef PositionOrientation::Scalar Scalar; // Data type for scalars
	typedef PositionOrientation::Vector Vector; // Data type for vectors
	typedef PositionOrientation::Point Point; // Data type for points
	typedef PositionOrientation::Rotation Rotation; // Data type for rotations
	
	struct CalibrationData // Structure for position and orientation corrections
		{
		/* Elements: */
		public:
		Vector positionOffset; // Offset vector from measured position to calibrated position
		Vector orientationOffset; // Scaled rotation axis from measured orientation to calibrated orientation
		
		/* Methods: */
		static CalibrationData interpolate(const CalibrationData& v0,const CalibrationData& v1,Scalar w1)
			{
			Scalar w0=Scalar(1)-w1;
			CalibrationData result;
			result.positionOffset=v0.positionOffset*w0+v1.positionOffset*w1;
			result.orientationOffset=v0.orientationOffset*w0+v1.orientationOffset*w1;
			return result;
			};
		};
	
	private:
	typedef Visualization::Curvilinear<Scalar,3,CalibrationData,CalibrationData> Grid; // Data type for grids of calibration data
	typedef Grid::Locator Locator; // Data type for locators in the calibration grid
	
	/* Elements: */
	int numDeviceTrackers; // Number of trackers on the associated device
	Grid* calibrationGrid; // Grid of calibration data
	Locator* trackerLocators; // Array of one locator for each tracker on the associated device
	
	/* Constructors and destructors: */
	public:
	GridCalibrator(VRCalibrator::Factory* sFactory,Misc::ConfigurationFile& configFile);
	virtual ~GridCalibrator(void);
	
	/* Methods: */
	virtual void setNumTrackers(int newNumTrackers);
	virtual Vrui::VRDeviceState::TrackerState& calibrate(int deviceTrackerIndex,Vrui::VRDeviceState::TrackerState& rawState);
	};

#endif
