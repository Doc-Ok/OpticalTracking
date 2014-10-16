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

#include <VRDeviceDaemon/VRCalibrators/TransformCalibrator.h>

#include <Misc/ConfigurationFile.h>
#include <Geometry/GeometryValueCoders.h>

/* Forward declarations: */
template <class BaseClassParam>
class VRFactoryManager;

/************************************
Methods of class TransformCalibrator:
************************************/

TransformCalibrator::TransformCalibrator(VRCalibrator::Factory* sFactory,Misc::ConfigurationFile& configFile)
	:VRCalibrator(sFactory,configFile),
	 transformation(configFile.retrieveValue<Vrui::OGTransform>("./transformation"))
	{
	}

Vrui::VRDeviceState::TrackerState& TransformCalibrator::calibrate(int deviceTrackerIndex,Vrui::VRDeviceState::TrackerState& rawState)
	{
	if(calibratePositions||calibrateOrientations)
		{
		/* Transform position/orientation: */
		Vrui::OGTransform calibTrans=transformation;
		calibTrans*=rawState.positionOrientation;
		rawState.positionOrientation=Vrui::VRDeviceState::TrackerState::PositionOrientation(calibTrans.getTranslation(),calibTrans.getRotation());
		}
	
	if(calibrateVelocities)
		{
		/* Transform velocities: */
		rawState.linearVelocity=transformation.transform(rawState.linearVelocity);
		rawState.angularVelocity=transformation.transform(rawState.angularVelocity);
		}
	
	return rawState;
	}

/*************************************
Object creation/destruction functions:
*************************************/

extern "C" VRCalibrator* createObjectTransformCalibrator(VRFactory<VRCalibrator>* factory,VRFactoryManager<VRCalibrator>*,Misc::ConfigurationFile& configFile)
	{
	return new TransformCalibrator(factory,configFile);
	}

extern "C" void destroyObjectTransformCalibrator(VRCalibrator* calibrator,VRFactory<VRCalibrator>*,VRFactoryManager<VRCalibrator>*)
	{
	delete calibrator;
	}
