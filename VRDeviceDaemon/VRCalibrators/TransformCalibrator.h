/***********************************************************************
TransformCalibrator - Class for calibrators using an orthonormal
calibration transformation.
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

#ifndef TRANSFORMCALIBRATOR_INCLUDED
#define TRANSFORMCALIBRATOR_INCLUDED

#include <Geometry/OrthogonalTransformation.h>
#include <Vrui/Geometry.h>
#include <Vrui/Internal/VRDeviceState.h>

#include <VRDeviceDaemon/VRCalibrator.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFile;
}

class TransformCalibrator:public VRCalibrator
	{
	/* Elements: */
	private:
	Vrui::OGTransform transformation; // The calibration transformation. The scaling component is used to scale coordinates, but stripped before reporting the final orientation
	
	/* Constructors and destructors: */
	public:
	TransformCalibrator(VRCalibrator::Factory* sFactory,Misc::ConfigurationFile& configFile);
	
	/* Methods: */
	virtual Vrui::VRDeviceState::TrackerState& calibrate(int deviceTrackerIndex,Vrui::VRDeviceState::TrackerState& rawState);
	};

#endif
