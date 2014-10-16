/***********************************************************************
WiimoteTracker - Class to use a Nintendo Wii controller and a special
infrared LED beacon as a 6-DOF tracking device.
Copyright (c) 2007-2012 Oliver Kreylos

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

#define GEOMETRY_NONSTANDARD_TEMPLATES

#include <VRDeviceDaemon/VRDevices/WiimoteTracker.h>

#include <stdio.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/CompoundValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/GeometryValueCoders.h>

#include <VRDeviceDaemon/VRDeviceManager.h>
#include <VRDeviceDaemon/VRDevices/Wiimote.h>

/*******************************
Methods of class WiimoteTracker:
*******************************/

void WiimoteTracker::wiimoteEventCallbackNoTracker(Misc::CallbackData* cbData)
	{
	if(reportEvents)
		{
		/* Update the VR device state: */
		for(int i=0;i<13;++i)
			setButtonState(i,wiimote->getButtonState(i));
		for(int i=0;i<2;++i)
			setValuatorState(i,wiimote->getJoystickValue(i));
		
		/* Mark manager state as complete: */
		updateState();
		}
	}

void WiimoteTracker::wiimoteEventCallback(Misc::CallbackData* cbData)
	{
	/* Read the current instantaneous acceleration vector: */
	Vector newAcceleration=wiimote->getAcceleration(0);
	
	/* Update the filtered acceleration vector: */
	if(firstEvent)
		acceleration=newAcceleration;
	else
		{
		Vector da=newAcceleration-lastAcceleration;
		Scalar trust=Math::exp(-Geometry::sqr(da)*Scalar(50))*Scalar(0.2);
		acceleration+=(newAcceleration-acceleration)*trust;
		}
	lastAcceleration=newAcceleration;
	
	/* Calculate an intermediate rotation based on the filtered acceleration vector: */
	Vector previousY=wiipos.getDirection(1);
	Scalar yaw=Math::acos(previousY[1]/Math::sqrt(Math::sqr(previousY[0])+Math::sqr(previousY[1])));
	if(previousY[0]>Scalar(0))
		yaw=-yaw;
	Scalar axz=Math::sqrt(Math::sqr(acceleration[0])+Math::sqr(acceleration[2]));
	Scalar roll=Math::acos(acceleration[2]/axz);
	if(acceleration[0]>Scalar(0))
		roll=-roll;
	Scalar pitch=Math::acos(axz/Math::sqrt(Math::sqr(acceleration[1])+Math::sqr(axz)));
	if(acceleration[1]<Scalar(0))
		pitch=-pitch;
	Transform::Rotation wiirot=Transform::Rotation::rotateZ(yaw);
	wiirot*=Transform::Rotation::rotateX(pitch);
	wiirot*=Transform::Rotation::rotateY(roll);
	
	/* Update the wiimote's orientation based on the acceleration vector only: */
	wiipos=Transform(wiipos.getTranslation(),wiirot);
	
	/* Store the IR camera targets: */
	int numValidTargets=0;
	for(int i=0;i<4;++i)
		{
		pixelValids[i]=wiimote->getIRTarget(i).valid;
		if(pixelValids[i])
			{
			for(int j=0;j<2;++j)
				pixels[i][j]=Scalar(wiimote->getIRTarget(i).pos[j]);
			++numValidTargets;
			}
		}
	
	if(numValidTargets>0)
		{
		if(numValidTargets==4)
			{
			/* Project the "up" vector into camera space: */
			typedef Geometry::Vector<CameraFitter::Scalar,2> PVector;
			PVector vy(acceleration[0],acceleration[2]);
			vy.normalize();
			PVector vx=-Geometry::normal(vy);
			vx.normalize();
			
			/* Find the leftmost, rightmost, and topmost points: */
			Scalar minX,maxX,minY,maxY;
			int minXIndex,maxXIndex,minYIndex,maxYIndex;
			minX=minY=Math::Constants<Scalar>::max;
			maxX=maxY=Math::Constants<Scalar>::min;
			minXIndex=maxXIndex=minYIndex=maxYIndex=-1;
			for(int i=0;i<4;++i)
				{
				Scalar x=pixels[i]*vx;
				Scalar y=pixels[i]*vy;
				if(minX>x)
					{
					minX=x;
					minXIndex=i;
					}
				if(maxX<x)
					{
					maxX=x;
					maxXIndex=i;
					}
				if(minY>y)
					{
					minY=y;
					minYIndex=i;
					}
				if(maxY<y)
					{
					maxY=y;
					maxYIndex=i;
					}
				}
			
			/* Create the pixel-target map: */
			pixelMap[minXIndex]=0;
			pixelMap[maxYIndex]=1;
			pixelMap[maxXIndex]=2;
			for(int i=0;i<4;++i)
				if(i!=minXIndex&&i!=maxYIndex&&i!=maxXIndex)
					pixelMap[i]=3;
			}
		else
			{
			/* Project the target points into camera space using the previous camera position/orientation and match closest pairs: */
			wiiCamera.setTransform(wiipos);
			for(int pixelIndex=0;pixelIndex<4;++pixelIndex)
				if(pixelValids[pixelIndex])
					{
					Scalar minDist2=Geometry::sqrDist(pixels[pixelIndex],wiiCamera.project(0));
					int minIndex=0;
					for(int i=1;i<4;++i)
						{
						Scalar dist2=Geometry::sqrDist(pixels[pixelIndex],wiiCamera.project(i));
						if(minDist2>dist2)
							{
							minDist2=dist2;
							minIndex=i;
							}
						}
					pixelMap[pixelIndex]=minIndex;
					}
			}
		
		/* Re-project the new pixel positions: */
		wiiCamera.setTransform(homeTransform);
		for(int i=0;i<4;++i)
			wiiCamera.invalidatePixel(i);
		for(int i=0;i<4;++i)
			if(pixelValids[i])
				wiiCamera.setPixel(pixelMap[i],pixels[i]);
		wiiCamera.setTransform(homeTransform);
		LMCamera::minimize(wiiCamera);
		
		if(firstEvent)
			wiipos=wiiCamera.getTransform();
		else
			{
			/* Filter the reconstructed camera transformation: */
			Transform deltaWP=Geometry::invert(wiipos);
			deltaWP.leftMultiply(wiiCamera.getTransform());
			Vector t=deltaWP.getTranslation();
			t*=Scalar(0.05);
			Vector r=deltaWP.getRotation().getScaledAxis();
			r*=Scalar(0.05);
			deltaWP=Transform(t,Transform::Rotation::rotateScaledAxis(r));
			wiipos.leftMultiply(deltaWP);
			}
		}
	wiipos.renormalize();
	firstEvent=false;
	
	if(wiimote->getButtonState(Wiimote::BUTTON_HOME))
		wiipos=homeTransform;
	
	if(reportEvents)
		{
		/* Update the VR device state: */
		for(int i=0;i<13;++i)
			setButtonState(i,wiimote->getButtonState(i));
		for(int i=0;i<2;++i)
			setValuatorState(i,wiimote->getJoystickValue(i));
		Vrui::VRDeviceState::TrackerState ts;
		ts.positionOrientation=PositionOrientation(wiipos);
		ts.linearVelocity=Vrui::VRDeviceState::TrackerState::LinearVelocity::zero;
		ts.angularVelocity=Vrui::VRDeviceState::TrackerState::AngularVelocity::zero;
		setTrackerState(0,ts);
		}
	}

WiimoteTracker::WiimoteTracker(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile)
	:VRDevice(sFactory,sDeviceManager,configFile),
	 wiimote(0),ledMask(configFile.retrieveValue<int>("./ledMask",0x1)),
	 enableTracker(configFile.retrieveValue<bool>("./enableTracker",false)),
	 wiiCamera(configFile.retrieveValue<Pixel>("./cameraCenter",Pixel(512,384)),
	           configFile.retrieveValue<Scalar>("./cameraFocalLength",Scalar(1280))),
	 homeTransform(configFile.retrieveValue<Transform>("./homeTransform",Transform::identity)),
	 firstEvent(true),
	 reportEvents(false)
	{
	/* Set device configuration: */
	if(enableTracker)
		setNumTrackers(1,configFile);
	setNumButtons(13,configFile); // Total number of buttons on Wii controller and Nunchuck extension
	setNumValuators(2,configFile); // Analog axes on Nunchuck extension
	
	/* Connect to the Wiimote device: */
	std::string deviceName=configFile.retrieveValue<std::string>("./deviceName","");
	if(deviceName!="")
		{
		printf("WiimoteTracker: Connecting to bluetooth device %s.\n",deviceName.c_str());
		printf("WiimoteTracker: Please press buttons 1 and 2 to initiate connection...");
		fflush(stdout);
		wiimote=new Wiimote(deviceName.c_str(),configFile);
		printf(" done\n");
		fflush(stdout);
		}
	else
		{
		printf("WiimoteTracker: Connecting to first compatible bluetooth device.\n");
		printf("WiimoteTracker: Please press buttons 1 and 2 to initiate connection...");
		fflush(stdout);
		wiimote=new Wiimote(0,configFile);
		printf(" done\n");
		fflush(stdout);
		}
	
	#ifdef VERBOSE
	/* Print the Wiimote's battery level: */
	printf("WiimoteTracker: Connected wiimote's battery level is %d%%\n",wiimote->getBatteryLevel());
	fflush(stdout);
	#endif
	
	if(enableTracker)
		{
		/* Initialize the camera tracker: */
		Transform targetTransform=configFile.retrieveValue<Transform>("./targetTransformation",Transform::identity);
		std::vector<Point> targetPoints=configFile.retrieveValue<std::vector<Point> >("./targetPoints");
		if(targetPoints.size()!=4)
			Misc::throwStdErr("WiimoteTracker: Wrong number of target points (%d, need 4)",int(targetPoints.size()));
		for(int i=0;i<4;++i)
			wiiCamera.setTargetPoint(i,targetTransform.transform(targetPoints[i]));
		wiiCamera.setTransform(homeTransform);
		wiipos=wiiCamera.getTransform();
		}
	
	/* Register an event callback with the Wiimote: */
	if(enableTracker)
		wiimote->getEventCallbacks().add(this,&WiimoteTracker::wiimoteEventCallback);
	else
		wiimote->getEventCallbacks().add(this,&WiimoteTracker::wiimoteEventCallbackNoTracker);
	}

WiimoteTracker::~WiimoteTracker(void)
	{
	delete wiimote;
	}

void WiimoteTracker::start(void)
	{
	#ifdef VERBOSE
	/* Print the Wiimote's battery level: */
	printf("WiimoteTracker: Connected wiimote's battery level is %d%%\n",wiimote->getBatteryLevel());
	fflush(stdout);
	#endif
	
	/* Turn on the Wiimote's LEDs: */
	wiimote->setLEDState(ledMask);
	
	/* Set Wiimote reporting mode: */
	firstEvent=true;
	if(enableTracker)
		{
		wiimote->requestAccelerometers(true);
		wiimote->requestIRTracking(true);
		}
	
	/* Enable event reporting: */
	reportEvents=true;
	}

void WiimoteTracker::stop(void)
	{
	/* Disable event reporting: */
	reportEvents=false;
	
	/* Set Wiimote reporting mode: */
	if(enableTracker)
		{
		wiimote->requestAccelerometers(false);
		wiimote->requestIRTracking(false);
		}
	
	/* Turn off the Wiimote's LEDs: */
	wiimote->setLEDState(0x0);
	
	#ifdef VERBOSE
	/* Print the Wiimote's battery level: */
	printf("WiimoteTracker: Connected wiimote's battery level is %d%%\n",wiimote->getBatteryLevel());
	fflush(stdout);
	#endif
	}

/*************************************
Object creation/destruction functions:
*************************************/

extern "C" VRDevice* createObjectWiimoteTracker(VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager,Misc::ConfigurationFile& configFile)
	{
	VRDeviceManager* deviceManager=static_cast<VRDeviceManager::DeviceFactoryManager*>(factoryManager)->getDeviceManager();
	return new WiimoteTracker(factory,deviceManager,configFile);
	}

extern "C" void destroyObjectWiimoteTracker(VRDevice* device,VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager)
	{
	delete device;
	}
