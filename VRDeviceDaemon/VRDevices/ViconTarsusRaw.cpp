/***********************************************************************
ViconTarsusRaw - Class for Vicon optical trackers using the raw
real-time streaming protocol.
Copyright (c) 2007-2011 Oliver Kreylos

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

#include <VRDeviceDaemon/VRDevices/ViconTarsusRaw.h>

#include <stdio.h>
#include <Misc/Endianness.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Geometry/GeometryValueCoders.h>

#include <VRDeviceDaemon/VRDeviceManager.h>

namespace {

/**************
Helper classes:
**************/

enum ViconTarsusRequestTypes // Bit field flags to request independent data streams from realtime engin
	{
	SupportedFlags=0x1U,
	Time=0x2U,
	GeneralStatus=0x4U,
	Reconstructions=0x8U,
	ReconPoints=0x10U,
	Bodies=0x20U,
	MultiState=0x40U,
	Labeling=0x80U,
	KinematicState=0x100U,
	TVD=0x200U,
	Edges=0x400U,
	Circles=0x800U,
	RawAnalog=0x1000U,
	ExtraDiagnostics=0x2000U,
	CharacterNames=0x4000U,
	StatusFlags=0x8000U,
	LabelingByCharacter=0x10000U,
	RayAssignments=0x20000U,
	Version=0x40000U,
	X2D=0x80000U,
	IQVersion=0x100000U,
	StartStreaming=0x40000000U,
	StopStreaming=0x80000000U
	};

enum ViconTarsusStatusFlags // Reply values on StatusFlags request? Unsure
	{
	ASAP=0x1U
	};

}

/*******************************
Methods of class ViconTarsusRaw:
*******************************/

void ViconTarsusRaw::deviceThreadMethod(void)
	{
	unsigned int lastFrame=~0U;
	while(true)
		{
		/* Increment the frame index: */
		unsigned int currentFrame=lastFrame+1U;
		
		/* Receive all reconstructed 3D points, up to the maximum number: */
		int numPoints=pipe.read<int>();
		for(int i=0;i<numPoints;++i)
			{
			/* Read the point's position: */
			double pv[3];
			pipe.read(pv,3);
			Point p(pv);
			
			/* Find the closest predicted marker position: */
			Scalar minDist2=matchTolerance2;
			int closestMarker=-1;
			int reclaimableMarker=-1;
			for(int j=0;j<maxNumMarkers;++j)
				{
				unsigned int markerAge=currentFrame-markerStates[j].lastVisibleFrame;
				if(markerAge<=markerTimeout) // Check markers that haven't timed out yet
					{
					Point predictedPos=markerStates[j].position+markerStates[j].velocity;
					Scalar dist2=Geometry::sqrDist(p,predictedPos);
					if(minDist2>dist2)
						{
						minDist2=dist2;
						closestMarker=j;
						}
					}
				else if(reclaimableMarker==-1)
					reclaimableMarker=j;
				}
			
			if(closestMarker>=0)
				{
				/* Update the found marker if it hasn't been used yet: */
				if(markerStates[closestMarker].lastVisibleFrame!=currentFrame)
					{
					markerStates[closestMarker].velocity=p-markerStates[closestMarker].position;
					markerStates[closestMarker].position=p;
					markerStates[closestMarker].lastVisibleFrame=currentFrame;
					}
				}
			else if(reclaimableMarker>=0)
				{
				/* Start a new marker: */
				markerStates[reclaimableMarker].velocity=Vector::zero;
				markerStates[reclaimableMarker].position=p;
				markerStates[reclaimableMarker].lastVisibleFrame=currentFrame;
				}
			}
		
		/* Update all tracker states in the device manager: */
		for(int i=0;i<maxNumMarkers;++i)
			{
			unsigned int markerAge=currentFrame-markerStates[i].lastVisibleFrame;
			setButtonState(i,markerAge<=predictionLimit);
			if(markerAge>markerTimeout)
				markerStates[i].position=defaultPosition;
			else if(markerAge>0&&markerAge<=predictionLimit)
				markerStates[i].position+=markerStates[i].velocity; // Move marker to predicted position
			trackerStateTemplate.positionOrientation.getTranslation()=markerStates[i].position-Point::origin;
			setTrackerState(i,trackerStateTemplate);
			}
		
		lastFrame=currentFrame;
		}
	}

ViconTarsusRaw::ViconTarsusRaw(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile)
	:VRDevice(sFactory,sDeviceManager,configFile),
	 pipe(configFile.retrieveString("./serverName").c_str(),configFile.retrieveValue<int>("./serverPort",803)),
	 predictionLimit(1U),
	 markerTimeout(6U),
	 defaultPosition(Point::origin),
	 markerStates(0)
	{
	/* Set the pipe's endianness: */
	pipe.setEndianness(Misc::LittleEndian);
	
	/* Read the maximum number of supported markers: */
	maxNumMarkers=configFile.retrieveValue<int>("./maxNumMarkers");
	
	/* Set tracker's layout: */
	setNumTrackers(maxNumMarkers,configFile);
	
	/* Create one button to signal each marker's visible/invisible state: */
	setNumButtons(maxNumMarkers,configFile);
	
	/* Read the marker match tolerance and the marker timeout: */
	matchTolerance2=Math::sqr(configFile.retrieveValue<Scalar>("./matchTolerance"));
	predictionLimit=configFile.retrieveValue<unsigned int>("./predictionLimit",predictionLimit);
	markerTimeout=configFile.retrieveValue<unsigned int>("./markerTimeout",markerTimeout);
	
	/* Read the default marker position and marker orientation: */
	defaultPosition=configFile.retrieveValue<Point>("./defaultPosition",defaultPosition);
	Rotation defaultOrientation=configFile.retrieveValue<Rotation>("./defaultOrientation",Rotation::identity);
	
	/* Create the tracker state template: */
	trackerStateTemplate.positionOrientation.getRotation()=defaultOrientation;
	trackerStateTemplate.angularVelocity=Vrui::VRDeviceState::TrackerState::AngularVelocity::zero;
	
	/* Create marker state array: */
	markerStates=new MarkerState[maxNumMarkers];
	}

ViconTarsusRaw::~ViconTarsusRaw(void)
	{
	delete[] markerStates;
	}

void ViconTarsusRaw::start(void)
	{
	/* Reset all marker states: */
	for(int i=0;i<maxNumMarkers;++i)
		{
		markerStates[i].position=defaultPosition;
		markerStates[i].velocity=Vector::zero;
		markerStates[i].lastVisibleFrame=(~0U)-markerTimeout;
		}
	
	/* Start device communication thread: */
	startDeviceThread();
	
	/* Start streaming data: */
	unsigned int request=ReconPoints|StartStreaming;
	pipe.write(request);
	pipe.flush();
	}

void ViconTarsusRaw::stop(void)
	{
	/* Stop streaming data: */
	unsigned int request=ReconPoints|StopStreaming;
	pipe.write(request);
	pipe.flush();
	
	/* Stop device communication thread: */
	stopDeviceThread();
	
	/* Flush the communications pipe: */
	while(pipe.waitForData(Misc::Time(0,100000000)))
		{
		char buffer[256];
		pipe.readUpTo(buffer,sizeof(buffer));
		}
	}

/*************************************
Object creation/destruction functions:
*************************************/

extern "C" VRDevice* createObjectViconTarsusRaw(VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager,Misc::ConfigurationFile& configFile)
	{
	VRDeviceManager* deviceManager=static_cast<VRDeviceManager::DeviceFactoryManager*>(factoryManager)->getDeviceManager();
	return new ViconTarsusRaw(factory,deviceManager,configFile);
	}

extern "C" void destroyObjectViconTarsusRaw(VRDevice* device,VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager)
	{
	delete device;
	}
