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

#include <VRDeviceDaemon/VRCalibrators/ForwardGridCalibrator.h>

#include <iostream>
#include <fstream>
#include <Misc/ConfigurationFile.h>

/* Forward declarations: */
template <class BaseClassParam>
class VRFactoryManager;

/**************************************
Methods of class ForwardGridCalibrator:
**************************************/

ForwardGridCalibrator::ForwardGridCalibrator(VRCalibrator::Factory* sFactory,Misc::ConfigurationFile& configFile)
	:VRCalibrator(sFactory,configFile)
	{
	/* Load the calibration data from a grid file: */
	std::string calibrationFileName=configFile.retrieveString("./calibrationFileName");
	std::ifstream calibrationFile(calibrationFileName.c_str());
	
	/* Read the grid size: */
	calibrationFile>>gridSize[0]>>gridSize[1]>>gridSize[2];
	
	/* Read the grid base point: */
	calibrationFile>>gridBase[0]>>gridBase[1]>>gridBase[2];
	
	/* Read the grid cell size: */
	calibrationFile>>gridCellSize[0]>>gridCellSize[1]>>gridCellSize[2];
	
	/* Read the calibration grid: */
	grid.resize(gridSize);
	Misc::Array<bool,3> vertexValids(gridSize);
	Index i;
	for(i=grid.beginIndex();i!=grid.endIndex();grid.preInc(i))
		{
		/* Check if the grid vertex is valid: */
		char validFlag;
		calibrationFile>>validFlag;
		if(validFlag=='V')
			{
			/* Read the vertex transformation: */
			char separator;
			Vector translation;
			Vector rotationAxis;
			Scalar rotationAngle;
			
			calibrationFile>>separator; // Skip '{'
			calibrationFile>>separator>>translation[0]>>separator>>translation[1]>>separator>>translation[2]>>separator;
			calibrationFile>>separator; // Skip ','
			
			calibrationFile>>separator; // Skip '{'
			calibrationFile>>separator>>rotationAxis[0]>>separator>>rotationAxis[1]>>separator>>rotationAxis[2]>>separator;
			calibrationFile>>separator; // Skip ','
			calibrationFile>>rotationAngle;
			calibrationFile>>separator; // Skip '}'
			calibrationFile>>separator; // Skip '}'
			
			/* Calculate the correction transformation: */
			Vector calibratedTranslation;
			for(int j=0;j<3;++j)
				calibratedTranslation[j]=gridBase[j]+Scalar(i[j])*gridCellSize[j];
			
			grid(i).positionOffset=translation-calibratedTranslation;
			grid(i).orientationOffset=Rotation::rotateAxis(rotationAxis,rotationAngle).getScaledAxis();
			
			vertexValids(i)=true;
			}
		else
			vertexValids(i)=false;
		}
	
	/* Calculate the grid cell vertex offsets: */
	for(int vertexIndex=0;vertexIndex<8;++vertexIndex)
		{
		cellVertexOffsets[vertexIndex]=0;
		for(int i=0;i<3;++i)
		if(vertexIndex&(0x1<<i))
			cellVertexOffsets[vertexIndex]+=grid.getIncrement(i);
		}
	
	/* Determine all valid grid cells: */
	cellValids.resize(gridSize-Index(1,1,1));
	for(i=cellValids.beginIndex();i!=cellValids.endIndex();cellValids.preInc(i))
		{
		/* Check if all corner vertices of this grid cell are valid: */
		bool valid=true;
		for(int j=0x0;j<0x8;++j)
			valid=valid&&vertexValids(i+Index((j&0x4)>>2,(j&0x2)>>1,j&0x1));
		cellValids(i)=valid;
		}
	}

Vrui::VRDeviceState::TrackerState& ForwardGridCalibrator::calibrate(int deviceTrackerIndex,Vrui::VRDeviceState::TrackerState& rawState)
	{
	/* Retrieve raw tracker position and orientation: */
	Point rawPosition=rawState.positionOrientation.getOrigin();
	
	/* Find the grid cell containing the raw tracker position: */
	Index cellIndex;
	Size cellPos;
	for(int i=0;i<3;++i)
		{
		Scalar gridPos=(rawPosition[i]-gridBase[i])/gridCellSize[i];
		cellIndex[i]=int(Math::floor(gridPos));
		if(cellIndex[i]<0)
			cellIndex[i]=0;
		else if(cellIndex[i]>gridSize[i]-2)
			cellIndex[i]=gridSize[i]-2;
		cellPos[i]=gridPos-Scalar(cellIndex[i]);
		}
	
	/* Check if the grid cell is valid: */
	if(cellValids(cellIndex))
		{
		/* Trilinearly interpolate the correction transformation: */
		const CalibrationData* base=&grid(cellIndex);
		CalibrationData v[4];
		v[0]=CalibrationData::interpolate(base[cellVertexOffsets[0]],base[cellVertexOffsets[1]],cellPos[0]);
		v[1]=CalibrationData::interpolate(base[cellVertexOffsets[2]],base[cellVertexOffsets[3]],cellPos[0]);
		v[2]=CalibrationData::interpolate(base[cellVertexOffsets[4]],base[cellVertexOffsets[5]],cellPos[0]);
		v[3]=CalibrationData::interpolate(base[cellVertexOffsets[6]],base[cellVertexOffsets[7]],cellPos[0]);
		
		v[0]=CalibrationData::interpolate(v[0],v[1],cellPos[1]);
		v[2]=CalibrationData::interpolate(v[2],v[3],cellPos[1]);
		
		v[0]=CalibrationData::interpolate(v[0],v[2],cellPos[2]);
		
		/* Apply the correction transformation: */
		PositionOrientation correction(v[0].positionOffset,Rotation(v[0].orientationOffset));
		rawState.positionOrientation.leftMultiply(correction);
		if(calibrateVelocities)
			{
			rawState.linearVelocity=correction.transform(rawState.linearVelocity);
			rawState.angularVelocity=correction.transform(rawState.angularVelocity);
			}
		}
	
	return rawState;
	}

/*************************************
Object creation/destruction functions:
*************************************/

extern "C" VRCalibrator* createObjectForwardGridCalibrator(VRFactory<VRCalibrator>* factory,VRFactoryManager<VRCalibrator>*,Misc::ConfigurationFile& configFile)
	{
	return new ForwardGridCalibrator(factory,configFile);
	}

extern "C" void destroyObjectForwardGridCalibrator(VRCalibrator* calibrator,VRFactory<VRCalibrator>*,VRFactoryManager<VRCalibrator>*)
	{
	delete calibrator;
	}
