/***********************************************************************
ValuatorWalkSurfaceNavigationTool - Version of the
WalkSurfaceNavigationTool that uses a pair of valuators to move instead
of head position.
Copyright (c) 2013-2014 Oliver Kreylos

This file is part of the Virtual Reality User Interface Library (Vrui).

The Virtual Reality User Interface Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Virtual Reality User Interface Library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality User Interface Library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#include <Vrui/Tools/ValuatorWalkSurfaceNavigationTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/Rotation.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Geometry/GeometryValueCoders.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLContextData.h>
#include <GL/GLNumberRenderer.h>
#include <GL/GLValueCoders.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <Vrui/Viewer.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/DeviceForwarderCreator.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/************************************************************************
Methods of class ValuatorWalkSurfaceNavigationToolFactory::Configuration:
************************************************************************/

ValuatorWalkSurfaceNavigationToolFactory::Configuration::Configuration(void)
	:activationToggle(true),
	 centerOnActivation(false),
	 centerPoint(getDisplayCenter()),
	 moveSpeed(getDisplaySize()),
	 innerRadius(getDisplaySize()*Scalar(0.5)),outerRadius(getDisplaySize()*Scalar(0.75)),
	 valuatorViewFollowFactor(1),
	 centerViewDirection(getForwardDirection()),
	 rotateSpeed(Math::rad(Scalar(120))),
	 innerAngle(Math::rad(Scalar(30))),outerAngle(Math::rad(Scalar(120))),
	 valuatorSnapRotate(false),valuatorRotateSpeed(rotateSpeed),
	 fallAcceleration(getMeterFactor()*Scalar(9.81)),
	 jetpackAcceleration(fallAcceleration*Scalar(1.5)),
	 probeSize(getInchFactor()*Scalar(12)),
	 maxClimb(getInchFactor()*Scalar(12)),
	 fixAzimuth(false),
	 drawMovementCircles(true),
	 movementCircleColor(0.0f,1.0f,0.0f),
	 drawHud(true),
	 hudRadius(getDisplaySize()*2.0f),
	 hudFontSize(getUiSize()*2.0f)
	{
	for(int i=0;i<2;++i)
		valuatorMoveSpeeds[i]=moveSpeed;
	}

void ValuatorWalkSurfaceNavigationToolFactory::Configuration::read(const Misc::ConfigurationFileSection& cfs)
	{
	activationToggle=cfs.retrieveValue<bool>("./activationToggle",activationToggle);
	centerOnActivation=cfs.retrieveValue<bool>("./centerOnActivation",centerOnActivation);
	centerPoint=cfs.retrieveValue<Point>("./centerPoint",centerPoint);
	centerPoint=getFloorPlane().project(centerPoint);
	moveSpeed=cfs.retrieveValue<Scalar>("./moveSpeed",moveSpeed);
	innerRadius=cfs.retrieveValue<Scalar>("./innerRadius",innerRadius);
	outerRadius=cfs.retrieveValue<Scalar>("./outerRadius",outerRadius);
	Geometry::Vector<Scalar,2> vms=cfs.retrieveValue<Geometry::Vector<Scalar,2> >("./valuatorMoveSpeed",Geometry::Vector<Scalar,2>(valuatorMoveSpeeds));
	for(int i=0;i<2;++i)
		valuatorMoveSpeeds[i]=vms[i];
	valuatorViewFollowFactor=cfs.retrieveValue<Scalar>("./valuatorViewFollowFactor",valuatorViewFollowFactor);
	valuatorViewFollowFactor=Math::clamp(valuatorViewFollowFactor,Scalar(0),Scalar(1));
	centerViewDirection=cfs.retrieveValue<Vector>("./centerViewDirection",centerViewDirection);
	centerViewDirection-=getUpDirection()*((centerViewDirection*getUpDirection())/Geometry::sqr(getUpDirection()));
	centerViewDirection.normalize();
	rotateSpeed=Math::rad(cfs.retrieveValue<Scalar>("./rotateSpeed",Math::deg(rotateSpeed)));
	innerAngle=Math::rad(cfs.retrieveValue<Scalar>("./innerAngle",Math::deg(innerAngle)));
	outerAngle=Math::rad(cfs.retrieveValue<Scalar>("./outerAngle",Math::deg(outerAngle)));
	valuatorSnapRotate=cfs.retrieveValue<bool>("./valuatorSnapRotate",valuatorSnapRotate);
	valuatorRotateSpeed=Math::rad(cfs.retrieveValue<Scalar>("./valuatorRotateSpeed",Math::deg(valuatorRotateSpeed)));
	fallAcceleration=cfs.retrieveValue<Scalar>("./fallAcceleration",fallAcceleration);
	jetpackAcceleration=cfs.retrieveValue<Scalar>("./jetpackAcceleration",fallAcceleration*Scalar(1.5));
	probeSize=cfs.retrieveValue<Scalar>("./probeSize",probeSize);
	maxClimb=cfs.retrieveValue<Scalar>("./maxClimb",maxClimb);
	fixAzimuth=cfs.retrieveValue<bool>("./fixAzimuth",fixAzimuth);
	drawMovementCircles=cfs.retrieveValue<bool>("./drawMovementCircles",drawMovementCircles);
	movementCircleColor=cfs.retrieveValue<Color>("./movementCircleColor",movementCircleColor);
	drawHud=cfs.retrieveValue<bool>("./drawHud",drawHud);
	hudRadius=cfs.retrieveValue<float>("./hudRadius",hudRadius);
	hudFontSize=cfs.retrieveValue<float>("./hudFontSize",hudFontSize);
	}

void ValuatorWalkSurfaceNavigationToolFactory::Configuration::write(Misc::ConfigurationFileSection& cfs) const
	{
	cfs.storeValue<bool>("./activationToggle",activationToggle);
	cfs.storeValue<bool>("./centerOnActivation",centerOnActivation);
	cfs.storeValue<Point>("./centerPoint",centerPoint);
	getFloorPlane().project(centerPoint);
	cfs.storeValue<Scalar>("./moveSpeed",moveSpeed);
	cfs.storeValue<Scalar>("./innerRadius",innerRadius);
	cfs.storeValue<Scalar>("./outerRadius",outerRadius);
	cfs.storeValue<Geometry::Vector<Scalar,2> >("./valuatorMoveSpeed",Geometry::Vector<Scalar,2>(valuatorMoveSpeeds));
	cfs.storeValue<Scalar>("./valuatorViewFollowFactor",valuatorViewFollowFactor);
	cfs.storeValue<Vector>("./centerViewDirection",centerViewDirection);
	cfs.storeValue<Scalar>("./rotateSpeed",Math::deg(rotateSpeed));
	cfs.storeValue<Scalar>("./innerAngle",Math::deg(innerAngle));
	cfs.storeValue<Scalar>("./outerAngle",Math::deg(outerAngle));
	cfs.storeValue<bool>("./valuatorSnapRotate",valuatorSnapRotate);
	cfs.storeValue<Scalar>("./valuatorRotateSpeed",Math::deg(valuatorRotateSpeed));
	cfs.storeValue<Scalar>("./fallAcceleration",fallAcceleration);
	cfs.storeValue<Scalar>("./jetpackAcceleration",jetpackAcceleration);
	cfs.storeValue<Scalar>("./probeSize",probeSize);
	cfs.storeValue<Scalar>("./maxClimb",maxClimb);
	cfs.storeValue<bool>("./fixAzimuth",fixAzimuth);
	cfs.storeValue<bool>("./drawMovementCircles",drawMovementCircles);
	cfs.storeValue<Color>("./movementCircleColor",movementCircleColor);
	cfs.storeValue<bool>("./drawHud",drawHud);
	cfs.storeValue<float>("./hudRadius",hudRadius);
	cfs.storeValue<float>("./hudFontSize",hudFontSize);
	}

/*********************************************************
Methods of class ValuatorWalkSurfaceNavigationToolFactory:
*********************************************************/

ValuatorWalkSurfaceNavigationToolFactory::ValuatorWalkSurfaceNavigationToolFactory(ToolManager& toolManager)
	:ToolFactory("ValuatorWalkSurfaceNavigationTool",toolManager)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	layout.setNumValuators(2,true);
	
	/* Insert class into class hierarchy: */
	ToolFactory* navigationToolFactory=toolManager.loadClass("SurfaceNavigationTool");
	navigationToolFactory->addChildClass(this);
	addParentClass(navigationToolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	configuration.read(cfs);
	
	/* Set tool class' factory pointer: */
	ValuatorWalkSurfaceNavigationTool::factory=this;
	}

ValuatorWalkSurfaceNavigationToolFactory::~ValuatorWalkSurfaceNavigationToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	ValuatorWalkSurfaceNavigationTool::factory=0;
	}

const char* ValuatorWalkSurfaceNavigationToolFactory::getName(void) const
	{
	return "Walk & Valuators";
	}

const char* ValuatorWalkSurfaceNavigationToolFactory::getButtonFunction(int) const
	{
	return "Start / Stop";
	}

const char* ValuatorWalkSurfaceNavigationToolFactory::getValuatorFunction(int valuatorSlotIndex) const
	{
	switch(valuatorSlotIndex)
		{
		case 0:
			return "Move Right/Left";
		
		case 1:
			return "Move Forward/Backwards";
		
		case 2:
			return "Rotate Right/Left";
		
		case 3:
			return "Fire Jetpack";
		
		default:
			return "Unused";
		}
	}

Tool* ValuatorWalkSurfaceNavigationToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new ValuatorWalkSurfaceNavigationTool(this,inputAssignment);
	}

void ValuatorWalkSurfaceNavigationToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveValuatorWalkSurfaceNavigationToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("SurfaceNavigationTool");
	}

extern "C" ToolFactory* createValuatorWalkSurfaceNavigationToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	ValuatorWalkSurfaceNavigationToolFactory* valuatorWalkSurfaceNavigationToolFactory=new ValuatorWalkSurfaceNavigationToolFactory(*toolManager);
	
	/* Return factory object: */
	return valuatorWalkSurfaceNavigationToolFactory;
	}

extern "C" void destroyValuatorWalkSurfaceNavigationToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/************************************************************
Methods of class ValuatorWalkSurfaceNavigationTool::DataItem:
************************************************************/

ValuatorWalkSurfaceNavigationTool::DataItem::DataItem(void)
	{
	/* Create tools' model display list: */
	movementCircleListId=glGenLists(2);
	hudListId=movementCircleListId+1;
	}

ValuatorWalkSurfaceNavigationTool::DataItem::~DataItem(void)
	{
	/* Destroy tools' model display list: */
	glDeleteLists(movementCircleListId,2);
	}

/**********************************************************
Static elements of class ValuatorWalkSurfaceNavigationTool:
**********************************************************/

ValuatorWalkSurfaceNavigationToolFactory* ValuatorWalkSurfaceNavigationTool::factory=0;

/**************************************************
Methods of class ValuatorWalkSurfaceNavigationTool:
**************************************************/

void ValuatorWalkSurfaceNavigationTool::applyNavState(void) const
	{
	/* Compose and apply the navigation transformation: */
	NavTransform nav=physicalFrame;
	nav*=NavTransform::rotateAround(Point(0,0,headHeight),Rotation::rotateX(elevation));
	nav*=NavTransform::rotate(Rotation::rotateZ(azimuth));
	nav*=Geometry::invert(surfaceFrame);
	setNavigationTransformation(nav);
	}

void ValuatorWalkSurfaceNavigationTool::initNavState(void)
	{
	/* Calculate the main viewer's current head and foot positions: */
	Point headPos=getMainViewer()->getHeadPosition();
	footPos=projectToFloor(headPos);
	headHeight=Geometry::dist(headPos,footPos);
	
	/* Set up a physical navigation frame around the main viewer's current head position: */
	calcPhysicalFrame(headPos);
	
	/* Calculate the initial environment-aligned surface frame in navigation coordinates: */
	surfaceFrame=getInverseNavigationTransformation()*physicalFrame;
	NavTransform newSurfaceFrame=surfaceFrame;
	
	/* Align the initial frame with the application's surface and calculate Euler angles: */
	AlignmentData ad(surfaceFrame,newSurfaceFrame,configuration.probeSize,configuration.maxClimb);
	Scalar roll;
	align(ad,azimuth,elevation,roll);
	
	/* Limit the elevation angle to the horizontal: */
	elevation=Scalar(0);
	
	/* Reset the falling velocity: */
	fallVelocity=Scalar(0);
	
	/* If the initial surface frame was above the surface, lift it back up and start falling: */
	Scalar z=newSurfaceFrame.inverseTransform(surfaceFrame.getOrigin())[2];
	if(z>Scalar(0))
		{
		newSurfaceFrame*=NavTransform::translate(Vector(Scalar(0),Scalar(0),z));
		fallVelocity=-configuration.fallAcceleration*getCurrentFrameTime();
		}
	
	/* Move the physical frame to the foot position, and adjust the surface frame accordingly: */
	newSurfaceFrame*=Geometry::invert(physicalFrame)*NavTransform::translate(footPos-headPos)*physicalFrame;
	physicalFrame.leftMultiply(NavTransform::translate(footPos-headPos));
	
	/* Apply the initial navigation state: */
	surfaceFrame=newSurfaceFrame;
	applyNavState();
	}

ValuatorWalkSurfaceNavigationTool::ValuatorWalkSurfaceNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:SurfaceNavigationTool(factory,inputAssignment),
	 configuration(ValuatorWalkSurfaceNavigationTool::factory->configuration),
	 numValuatorDevices(0),valuatorDevices(0),forwardedValuators(0),
	 numberRenderer(0),
	 centerPoint(configuration.centerPoint),
	 rotate(0),lastSnapRotate(0),snapRotate(0),
	 jetpack(0)
	{
	}

ValuatorWalkSurfaceNavigationTool::~ValuatorWalkSurfaceNavigationTool(void)
	{
	}

void ValuatorWalkSurfaceNavigationTool::configure(const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Override private configuration data from given configuration file section: */
	configuration.read(configFileSection);
	}

void ValuatorWalkSurfaceNavigationTool::storeState(Misc::ConfigurationFileSection& configFileSection) const
	{
	/* Write private configuration data to given configuration file section: */
	configuration.write(configFileSection);
	}

void ValuatorWalkSurfaceNavigationTool::initialize(void)
	{
	/* Create the virtual input devices needed to forward the valuator slots: */
	DeviceForwarderCreator dfc(0,input.getNumValuatorSlots());
	for(int i=0;i<input.getNumValuatorSlots();++i)
		dfc.forwardValuator(i,input.getValuatorSlot(i).device,input.getValuatorSlot(i).index);
	dfc.createDevices();
	
	/* Copy the created devices: */
	numValuatorDevices=int(dfc.getNumDevices());
	valuatorDevices=new ForwardedDevice[numValuatorDevices];
	for(int i=0;i<numValuatorDevices;++i)
		{
		/* Retrieve the source and virtual devices: */
		InputDevice* sourceDevice=dfc.getSourceDevice(i);
		InputDevice* virtualDevice=dfc.collectDevice(sourceDevice);
		
		/* Copy the source device's tracking type: */
		virtualDevice->setTrackType(sourceDevice->getTrackType());
		
		/* Disable the virtual device's glyph: */
		getInputGraphManager()->getInputDeviceGlyph(virtualDevice).disable();
		
		/* Permanently grab the virtual input device: */
		getInputGraphManager()->grabInputDevice(virtualDevice,this);
		
		/* Initialize the virtual input device's position: */
		virtualDevice->setDeviceRay(sourceDevice->getDeviceRayDirection(),sourceDevice->getDeviceRayStart());
		virtualDevice->setTransformation(sourceDevice->getTransformation());
		
		/* Store the device association: */
		valuatorDevices[i].sourceDevice=sourceDevice;
		valuatorDevices[i].virtualDevice=virtualDevice;
		}
	
	/* Copy the valuator slot forwarding associations: */
	forwardedValuators=new ForwardedValuator[input.getNumValuatorSlots()];
	for(int i=0;i<input.getNumValuatorSlots();++i)
		{
		forwardedValuators[i].device=dfc.getValuatorSlots()[i].virtualDevice;
		forwardedValuators[i].valuatorIndex=dfc.getValuatorSlots()[i].virtualDeviceFeatureIndex;
		}
	
	/* Initialize the number renderer: */
	numberRenderer=new GLNumberRenderer(configuration.hudFontSize,true);
	dependsOn(numberRenderer);
	
	/* Initialize transient navigation state: */
	centerPoint=configuration.centerPoint;
	}

void ValuatorWalkSurfaceNavigationTool::deinitialize(void)
	{
	/* Release and destroy all virtual input devices: */
	for(int i=0;i<numValuatorDevices;++i)
		{
		getInputGraphManager()->releaseInputDevice(valuatorDevices[i].virtualDevice,this);
		getInputDeviceManager()->destroyInputDevice(valuatorDevices[i].virtualDevice);
		}
	
	/* Clean up device forwarding state: */
	delete[] valuatorDevices;
	valuatorDevices=0;
	delete[] forwardedValuators;
	forwardedValuators=0;
	
	/* Destroy the number renderer: */
	delete numberRenderer;
	numberRenderer=0;
	}

const ToolFactory* ValuatorWalkSurfaceNavigationTool::getFactory(void) const
	{
	return factory;
	}

void ValuatorWalkSurfaceNavigationTool::buttonCallback(int,InputDevice::ButtonCallbackData* cbData)
	{
	/* Determine the new activation state of this tool: */
	bool newActive;
	if(configuration.activationToggle)
		{
		/* Toggle the activation state: */
		newActive=isActive();
		if(cbData->newButtonState)
			newActive=!newActive;
		}
	else
		{
		/* Set the activation state to the new button state: */
		newActive=cbData->newButtonState;
		}
	
	/* Activate or deactivate the tool: */
	if(isActive())
		{
		if(!newActive)
			{
			/* Deactivate the tool: */
			deactivate();
			
			/* Set the forwarded valuators to the states of the source valuators: */
			for(int i=0;i<input.getNumValuatorSlots();++i)
				forwardedValuators[i].device->setValuator(forwardedValuators[i].valuatorIndex,getValuatorState(i));
			}
		}
	else
		{
		/* Try activating this tool: */
		if(newActive&&activate())
			{
			if(configuration.centerOnActivation)
				{
				/* Store the center point for this navigation sequence: */
				centerPoint=projectToFloor(getMainViewer()->getHeadPosition());
				}
			
			/* Initialize the navigation state: */
			initNavState();
			}
		}
	}

void ValuatorWalkSurfaceNavigationTool::valuatorCallback(int valuatorSlotIndex,InputDevice::ValuatorCallbackData* cbData)
	{
	if(isActive())
		{
		if(valuatorSlotIndex==2)
			{
			/* Update the current valuator rotation velocity: */
			rotate=Scalar(cbData->newValuatorValue)*configuration.valuatorRotateSpeed;
			
			if(configuration.valuatorSnapRotate)
				{
				/* Update the current snap rotation state: */
				if(cbData->newValuatorValue<-0.75)
					snapRotate=-1;
				else if(cbData->newValuatorValue>0.75)
					snapRotate=1;
				else if(Math::abs(cbData->newValuatorValue)<0.25)
					snapRotate=0;
				}
			}
		else if(valuatorSlotIndex==3)
			{
			/* Update the jetpack acceleration value: */
			jetpack=Scalar(cbData->newValuatorValue)*configuration.jetpackAcceleration;
			}
		}
	else
		{
		/* Forward the source valuator state to the forwarded input device: */
		forwardedValuators[valuatorSlotIndex].device->setValuator(forwardedValuators[valuatorSlotIndex].valuatorIndex,cbData->newValuatorValue);
		}
	}

void ValuatorWalkSurfaceNavigationTool::frame(void)
	{
	/* Act depending on this tool's current state: */
	if(isActive())
		{
		/* Calculate azimuth angle change based on the current viewing direction: */
		Vector viewDir=getMainViewer()->getViewDirection();
		viewDir-=getUpDirection()*((viewDir*getUpDirection())/Geometry::sqr(getUpDirection()));
		Scalar viewDir2=Geometry::sqr(viewDir);
		if(viewDir2!=Scalar(0))
			{
			viewDir/=Math::sqrt(viewDir2);
			
			/* Calculate the rotation speed: */
			Scalar viewAngleCos=(viewDir*configuration.centerViewDirection);
			Scalar viewAngle;
			if(viewAngleCos>Scalar(1)-Math::Constants<Scalar>::epsilon)
				viewAngle=Scalar(0);
			else if(viewAngleCos<Scalar(-1)+Math::Constants<Scalar>::epsilon)
				viewAngle=Math::Constants<Scalar>::pi;
			else
				viewAngle=Math::acos(viewAngleCos);
			Scalar rotateSpeed=Scalar(0);
			if(viewAngle>=configuration.outerAngle)
				rotateSpeed=configuration.rotateSpeed;
			else if(viewAngle>configuration.innerAngle)
				rotateSpeed=configuration.rotateSpeed*(viewAngle-configuration.innerAngle)/(configuration.outerAngle-configuration.innerAngle);
			Vector x=configuration.centerViewDirection^getUpDirection();
			if(viewDir*x<Scalar(0))
				rotateSpeed=-rotateSpeed;
			
			/* Update the azimuth angle: */
			azimuth=wrapAngle(azimuth+rotateSpeed*getFrameTime());
			}
		
		/* Calculate azimuth angle change based on valuators: */
		if(configuration.valuatorSnapRotate)
			{
			/* Check if the valuator just entered the positive or negative snap range: */
			if(lastSnapRotate!=snapRotate)
				{
				azimuth=wrapAngle(azimuth+Scalar(snapRotate)*configuration.valuatorRotateSpeed);
				lastSnapRotate=snapRotate;
				}
			}
		else
			azimuth=wrapAngle(azimuth+rotate*getFrameTime());
		
		/* Calculate the new head and foot positions: */
		Point newFootPos=projectToFloor(getMainViewer()->getHeadPosition());
		headHeight=Geometry::dist(getMainViewer()->getHeadPosition(),newFootPos);
		
		/* Create a physical navigation frame around the new foot position: */
		calcPhysicalFrame(newFootPos);
		
		/* Calculate the movement from walking: */
		Vector move=newFootPos-footPos;
		footPos=newFootPos;
		
		/* Calculate movement from virtual joystick: */
		Vector moveDir=footPos-centerPoint;
		Scalar moveDirLen=moveDir.mag();
		Scalar speed=Scalar(0);
		if(moveDirLen>=configuration.outerRadius)
			speed=configuration.moveSpeed;
		else if(moveDirLen>configuration.innerRadius)
			speed=configuration.moveSpeed*(moveDirLen-configuration.innerRadius)/(configuration.outerRadius-configuration.innerRadius);
		moveDir*=speed/moveDirLen;
		
		/* Calculate movement from valuators: */
		Vector valuatorMoveDir=configuration.centerViewDirection*(Scalar(1)-configuration.valuatorViewFollowFactor)+viewDir*configuration.valuatorViewFollowFactor;
		valuatorMoveDir.normalize();
		moveDir[0]+=valuatorMoveDir[0]*getValuatorState(1)*configuration.valuatorMoveSpeeds[1];
		moveDir[1]+=valuatorMoveDir[1]*getValuatorState(1)*configuration.valuatorMoveSpeeds[1];
		moveDir[0]+=valuatorMoveDir[1]*getValuatorState(0)*configuration.valuatorMoveSpeeds[0];
		moveDir[1]-=valuatorMoveDir[0]*getValuatorState(0)*configuration.valuatorMoveSpeeds[0];
		
		/* Add the current flying and falling velocities: */
		if(jetpack!=Scalar(0))
			moveDir+=getValuatorDeviceRayDirection(0)*jetpack;
		moveDir[2]+=fallVelocity;
		
		/* Calculate the complete movement vector: */
		move+=moveDir*getCurrentFrameTime();
		
		/* Transform the movement vector from physical space to the physical navigation frame: */
		move=physicalFrame.inverseTransform(move);
		
		/* Rotate by the current azimuth angle: */
		move=Rotation::rotateZ(-azimuth).transform(move);
		
		/* Move the surface frame: */
		NavTransform newSurfaceFrame=surfaceFrame;
		newSurfaceFrame*=NavTransform::translate(move);
		
		/* Re-align the surface frame with the surface: */
		Point initialOrigin=newSurfaceFrame.getOrigin();
		Rotation initialOrientation=newSurfaceFrame.getRotation();
		AlignmentData ad(surfaceFrame,newSurfaceFrame,configuration.probeSize,configuration.maxClimb);
		align(ad);
		
		if(!configuration.fixAzimuth)
			{
			/* Have the azimuth angle track changes in the surface frame's rotation: */
			Rotation rot=Geometry::invert(initialOrientation)*newSurfaceFrame.getRotation();
			rot.leftMultiply(Rotation::rotateFromTo(rot.getDirection(2),Vector(0,0,1)));
			Vector x=rot.getDirection(0);
			azimuth=wrapAngle(azimuth+Math::atan2(x[1],x[0]));
			}
		
		/* Check if the initial surface frame is above the surface: */
		Scalar z=newSurfaceFrame.inverseTransform(initialOrigin)[2];
		if(z>Scalar(0))
			{
			/* Lift the aligned frame back up to the original altitude and continue flying: */
			newSurfaceFrame*=NavTransform::translate(Vector(Scalar(0),Scalar(0),z));
			fallVelocity-=configuration.fallAcceleration*getCurrentFrameTime();
			}
		else
			{
			/* Stop falling: */
			fallVelocity=Scalar(0);
			}
		
		/* Apply the newly aligned surface frame: */
		surfaceFrame=newSurfaceFrame;
		applyNavState();
		
		if(speed!=Scalar(0)||z>Scalar(0)||jetpack!=Scalar(0))
			{
			/* Request another frame: */
			scheduleUpdate(getApplicationTime()+1.0/125.0);
			}
		}
	
	/* Update the forwarded virtual input devices: */
	for(int i=0;i<numValuatorDevices;++i)
		{
		InputDevice* sourceDevice=valuatorDevices[i].sourceDevice;
		InputDevice* virtualDevice=valuatorDevices[i].virtualDevice;
		virtualDevice->setDeviceRay(sourceDevice->getDeviceRayDirection(),sourceDevice->getDeviceRayStart());
		virtualDevice->setTransformation(sourceDevice->getTransformation());
		}
	}

void ValuatorWalkSurfaceNavigationTool::display(GLContextData& contextData) const
	{
	/* Get a pointer to the context data item and set up OpenGL state: */
	DataItem* dataItem=0;
	if(configuration.drawMovementCircles||(configuration.drawHud&&isActive()))
		{
		dataItem=contextData.retrieveDataItem<DataItem>(this);
		
		glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
		glDisable(GL_LIGHTING);
		glLineWidth(1.0f);
		}
	
	if(configuration.drawMovementCircles)
		{
		/* Translate to the center point: */
		glPushMatrix();
		glTranslate(centerPoint-Point::origin);
		
		/* Execute the movement circle display list: */
		glCallList(dataItem->movementCircleListId);
		
		glPopMatrix();
		}
	
	if(configuration.drawHud&&isActive())
		{
		/* Translate to the HUD's center point: */
		glPushMatrix();
		glMultMatrix(physicalFrame);
		glTranslate(0,0,headHeight);
		
		/* Rotate by the azimuth angle: */
		glRotate(Math::deg(azimuth),0,0,1);
		
		/* Execute the HUD display list: */
		glCallList(dataItem->hudListId);
		
		glPopMatrix();
		}
	
	/* Reset OpenGL state: */
	if(configuration.drawMovementCircles||(configuration.drawHud&&isActive()))
		glPopAttrib();
	}

void ValuatorWalkSurfaceNavigationTool::initContext(GLContextData& contextData) const
	{
	DataItem* dataItem=0;
	if(configuration.drawMovementCircles||configuration.drawHud)
		{
		/* Create a new data item: */
		dataItem=new DataItem;
		contextData.addDataItem(this,dataItem);
		}
		
	if(configuration.drawMovementCircles)
		{
		/* Create the movement circle display list: */
		glNewList(dataItem->movementCircleListId,GL_COMPILE);
		
		/* Create a coordinate system for the floor plane: */
		Vector y=configuration.centerViewDirection;
		Vector x=y^getFloorPlane().getNormal();
		x.normalize();
		
		/* Draw the inner circle: */
		glColor(configuration.movementCircleColor);
		glBegin(GL_LINE_LOOP);
		for(int i=0;i<64;++i)
			{
			Scalar angle=Scalar(2)*Math::Constants<Scalar>::pi*Scalar(i)/Scalar(64);
			glVertex(Point::origin-x*(Math::sin(angle)*configuration.innerRadius)+y*(Math::cos(angle)*configuration.innerRadius));
			}
		glEnd();
		
		/* Draw the outer circle: */
		glBegin(GL_LINE_LOOP);
		for(int i=0;i<64;++i)
			{
			Scalar angle=Scalar(2)*Math::Constants<Scalar>::pi*Scalar(i)/Scalar(64);
			glVertex(Point::origin-x*(Math::sin(angle)*configuration.outerRadius)+y*(Math::cos(angle)*configuration.outerRadius));
			}
		glEnd();
		
		/* Draw the inner angle: */
		glBegin(GL_LINE_STRIP);
		glVertex(Point::origin-x*(Math::sin(configuration.innerAngle)*configuration.innerRadius)+y*(Math::cos(configuration.innerAngle)*configuration.innerRadius));
		glVertex(Point::origin);
		glVertex(Point::origin-x*(Math::sin(-configuration.innerAngle)*configuration.innerRadius)+y*(Math::cos(-configuration.innerAngle)*configuration.innerRadius));
		glEnd();
		
		/* Draw the outer angle: */
		glBegin(GL_LINE_STRIP);
		glVertex(Point::origin-x*(Math::sin(configuration.outerAngle)*configuration.outerRadius)+y*(Math::cos(configuration.outerAngle)*configuration.outerRadius));
		glVertex(Point::origin);
		glVertex(Point::origin-x*(Math::sin(-configuration.outerAngle)*configuration.outerRadius)+y*(Math::cos(-configuration.outerAngle)*configuration.outerRadius));
		glEnd();
		
		glEndList();
		}
	
	if(configuration.drawHud)
		{
		/* Create the HUD display list: */
		glNewList(dataItem->hudListId,GL_COMPILE);
		
		/* Determine the HUD colors: */
		Color bgColor=getBackgroundColor();
		Color fgColor;
		for(int i=0;i<3;++i)
			fgColor[i]=1.0f-bgColor[i];
		fgColor[3]=bgColor[3];
		
		/* Calculate the HUD layout: */
		Scalar hudTickSize=configuration.hudFontSize;
		
		/* Draw the azimuth tick marks: */
		glColor(fgColor);
		glBegin(GL_LINES);
		for(int az=0;az<360;az+=10)
			{
			Scalar angle=Math::rad(Scalar(az));
			Scalar c=Math::cos(angle)*configuration.hudRadius;
			Scalar s=Math::sin(angle)*configuration.hudRadius;
			glVertex(s,c,Scalar(0));
			glVertex(s,c,Scalar(0)+(az%30==0?hudTickSize*Scalar(2):hudTickSize));
			}
		glEnd();
		
		/* Draw the azimuth labels: */
		for(int az=0;az<360;az+=30)
			{
			/* Move to the label's coordinate system: */
			glPushMatrix();
			Scalar angle=Math::rad(Scalar(az));
			Scalar c=Math::cos(angle)*configuration.hudRadius;
			Scalar s=Math::sin(angle)*configuration.hudRadius;
			glTranslate(s,c,hudTickSize*Scalar(2.5));
			glRotate(-double(az),0.0,0.0,1.0);
			glRotate(90.0,1.0,0.0,0.0);
			double width=Scalar(numberRenderer->calcNumberWidth(az));
			glTranslate(-width*0.5,0.0,0.0);
			
			/* Draw the azimuth label: */
			numberRenderer->drawNumber(az,contextData);
			
			/* Go back to original coordinate system: */
			glPopMatrix();
			}
		
		glEndList();
		}
	}

std::vector<InputDevice*> ValuatorWalkSurfaceNavigationTool::getForwardedDevices(void)
	{
	/* Return a list of all virtual devices: */
	std::vector<InputDevice*> result;
	for(int i=0;i<numValuatorDevices;++i)
		result.push_back(valuatorDevices[i].virtualDevice);
	return result;
	}

InputDeviceFeatureSet ValuatorWalkSurfaceNavigationTool::getSourceFeatures(const InputDeviceFeature& forwardedFeature)
	{
	/* Find the forwarded feature among the forwarded valuators: */
	int i;
	for(i=0;i<input.getNumValuatorSlots()&&(forwardedValuators[i].device!=forwardedFeature.getDevice()||forwardedValuators[i].valuatorIndex!=forwardedFeature.getIndex());++i)
		;
	if(i==input.getNumValuatorSlots())
		throw std::runtime_error("ValuatorWalkSurfaceNavigationTool::getSourceFeatures: Forwarded feature not found");
	
	/* Return the source feature: */
	InputDeviceFeatureSet result;
	result.push_back(input.getValuatorSlotFeature(i));
	return result;
	}

InputDevice* ValuatorWalkSurfaceNavigationTool::getSourceDevice(const InputDevice* forwardedDevice)
	{
	/* Find the forwarded input device among the virtual input devices: */
	int i;
	for(i=0;i<numValuatorDevices&&valuatorDevices[i].virtualDevice!=forwardedDevice;++i)
		;
	if(i==numValuatorDevices)
		throw std::runtime_error("ValuatorWalkSurfaceNavigationTool::getSourceDevice: Forwarded device not found");
	
	/* Return the source device: */
	return valuatorDevices[i].sourceDevice;
	}

InputDeviceFeatureSet ValuatorWalkSurfaceNavigationTool::getForwardedFeatures(const InputDeviceFeature& sourceFeature)
	{
	/* Get the source feature's assignment slot index: */
	int slotIndex=input.findFeature(sourceFeature);
	
	/* Paranoia: Check if the source feature belongs to this tool: */
	if(slotIndex<0)
		throw std::runtime_error("ValuatorWalkSurfaceNavigationTool::getForwardedFeatures: Source feature not found");
	
	/* Return the forwarded feature: */
	InputDeviceFeatureSet result;
	if(sourceFeature.isValuator())
		{
		int valuatorSlotIndex=input.getValuatorSlotIndex(slotIndex);
		result.push_back(InputDeviceFeature(forwardedValuators[valuatorSlotIndex].device,InputDevice::VALUATOR,forwardedValuators[valuatorSlotIndex].valuatorIndex));
		}
	return result;
	}

}
