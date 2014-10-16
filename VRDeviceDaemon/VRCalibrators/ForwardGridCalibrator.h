/***********************************************************************
ForwardGridCalibrator - Class for calibrators using a rectilinear grid
of "ground truth" tracker measurements for position and orientation
correction.
Copyright (c) 2014 Oliver Kreylos

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

#ifndef FORWARDGRIDCALIBRATOR_INCLUDED
#define FORWARDGRIDCALIBRATOR_INCLUDED

#include <Misc/Array.h>

#include <Vrui/Internal/VRDeviceState.h>

#include <VRDeviceDaemon/VRCalibrator.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFile;
}

class ForwardGridCalibrator:public VRCalibrator
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
	
	typedef Misc::Array<CalibrationData,3> CalibrationArray;
	typedef CalibrationArray::Index Index;
	typedef Geometry::ComponentArray<Scalar,3> Size; // Data type for grid cell sizes
	
	/* Elements: */
	Index gridSize; // Size of calibration grid
	Point gridBase; // Base point of calibration grid
	Size gridCellSize; // Cell size of calibration grid
	CalibrationArray grid; // Calibration grid
	ptrdiff_t cellVertexOffsets[8]; // Offsets from a grid cell's base vertex to the cell's corner vertices
	Misc::Array<bool,3> cellValids; // Array of valid flags for each calibration grid cell
	
	/* Constructors and destructors: */
	public:
	ForwardGridCalibrator(VRCalibrator::Factory* sFactory,Misc::ConfigurationFile& configFile);
	
	/* Methods: */
	virtual Vrui::VRDeviceState::TrackerState& calibrate(int deviceTrackerIndex,Vrui::VRDeviceState::TrackerState& rawState);
	};

#endif
