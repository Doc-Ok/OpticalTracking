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

#include <VRDeviceDaemon/VRCalibrators/GridCalibrator.h>

#include <Misc/File.h>
#include <Misc/ConfigurationFile.h>

/* Forward declarations: */
template <class BaseClassParam>
class VRFactoryManager;

/*******************************
Methods of class GridCalibrator:
*******************************/

GridCalibrator::GridCalibrator(VRCalibrator::Factory* sFactory,Misc::ConfigurationFile& configFile)
	:VRCalibrator(sFactory,configFile),
	 numDeviceTrackers(0),calibrationGrid(0),trackerLocators(0)
	{
	/* Load the calibration data from file: */
	Misc::File calibrationFile(configFile.retrieveString("./calibrationFileName").c_str(),"rb",Misc::File::LittleEndian);
	Grid::Index gridSize;
	for(int i=0;i<3;++i)
		gridSize[i]=calibrationFile.read<int>();
	calibrationGrid=new Grid(gridSize);
	
	/* Read the calibration data: */
	for(Grid::Index index(0);index[0]<gridSize[0];index.preInc(gridSize))
		{
		Grid::GridVertex& v=calibrationGrid->getVertex(index);
		calibrationFile.read(v.pos.getComponents(),3);
		float quat[4];
		calibrationFile.read(quat,4);
		calibrationFile.read(v.value.positionOffset.getComponents(),3);
		calibrationFile.read(v.value.orientationOffset.getComponents(),3);
		}
	calibrationGrid->finalizeGrid();
	}

GridCalibrator::~GridCalibrator(void)
	{
	delete[] trackerLocators;
	delete calibrationGrid;
	}

void GridCalibrator::setNumTrackers(int newNumTrackers)
	{
	/* Delete current array of tracker locators: */
	delete[] trackerLocators;
	
	/* Allocate new array of tracker locators: */
	numDeviceTrackers=newNumTrackers;
	trackerLocators=new Locator[numDeviceTrackers];
	
	/* Initialize the locators: */
	for(int i=0;i<numDeviceTrackers;++i)
		trackerLocators[i]=calibrationGrid->getLocator();
	}

Vrui::VRDeviceState::TrackerState& GridCalibrator::calibrate(int deviceTrackerIndex,Vrui::VRDeviceState::TrackerState& rawState)
	{
	/* Retrieve raw tracker position and orientation: */
	Point rawPosition=rawState.positionOrientation.getOrigin();
	Rotation rawOrientation=rawState.positionOrientation.getRotation();
	
	/* Calculate the correction values at the raw tracker position: */
	trackerLocators[deviceTrackerIndex].locatePoint(rawPosition,true);
	CalibrationData correction=trackerLocators[deviceTrackerIndex].calcValue();
	Rotation orientationOffset(correction.orientationOffset);
	
	/* Calibrate position/orientation: */
	Point calPosition=rawPosition;
	if(calibratePositions)
		calPosition+=correction.positionOffset;
	Rotation calOrientation=rawOrientation;
	if(calibrateOrientations)
		calOrientation.leftMultiply(orientationOffset);
	rawState.positionOrientation=PositionOrientation(calPosition-Point::origin,calOrientation);
	
	if(calibrateVelocities)
		{
		/* Calibrate velocities: */
		rawState.linearVelocity=calOrientation.transform(rawState.linearVelocity);
		rawState.angularVelocity=calOrientation.transform(rawState.angularVelocity);
		}
	
	return rawState;
	}

/*************************************
Object creation/destruction functions:
*************************************/

extern "C" VRCalibrator* createObjectGridCalibrator(VRFactory<VRCalibrator>* factory,VRFactoryManager<VRCalibrator>*,Misc::ConfigurationFile& configFile)
	{
	return new GridCalibrator(factory,configFile);
	}

extern "C" void destroyObjectGridCalibrator(VRCalibrator* calibrator,VRFactory<VRCalibrator>*,VRFactoryManager<VRCalibrator>*)
	{
	delete calibrator;
	}
