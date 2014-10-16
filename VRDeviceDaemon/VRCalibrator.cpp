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

#include <VRDeviceDaemon/VRCalibrator.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>

#include <VRDeviceDaemon/VRFactory.h>

/*****************************
Methods of class VRCalibrator:
*****************************/

VRCalibrator::VRCalibrator(VRCalibrator::Factory* sFactory,Misc::ConfigurationFile& configFile)
	:factory(sFactory),
	 calibratePositions(configFile.retrieveValue<bool>("./calibratePositions",true)),
	 calibrateOrientations(configFile.retrieveValue<bool>("./calibrateOrientations",true)),
	 calibrateVelocities(configFile.retrieveValue<bool>("./calibrateVelocities",true))
	{
	}

VRCalibrator::~VRCalibrator(void)
	{
	}

void VRCalibrator::destroy(VRCalibrator* object)
	{
	object->factory->destroyObject(object);
	}

void VRCalibrator::setNumTrackers(int newNumTrackers)
	{
	}
