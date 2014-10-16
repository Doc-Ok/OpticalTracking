/***********************************************************************
FPSNavigationTool - Class encapsulating the navigation behaviour of a
typical first-person shooter (FPS) game.
Copyright (c) 2005-2014 Oliver Kreylos

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

#include <Vrui/Tools/FPSNavigationTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ArrayValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Geometry/Ray.h>
#include <Geometry/Plane.h>
#include <Geometry/Rotation.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLValueCoders.h>
#include <GL/GLContextData.h>
#include <GL/GLNumberRenderer.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <Vrui/Vrui.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/Internal/InputDeviceAdapterMouse.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/Viewer.h>
#include <Vrui/VRWindow.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/********************************************************
Methods of class FPSNavigationToolFactory::Configuration:
********************************************************/

FPSNavigationToolFactory::Configuration::Configuration(void)
	:activationToggle(true),
	 rotateFactors(getDisplaySize()/Scalar(2)),
	 moveSpeeds(getInchFactor()*Scalar(200)),
	 fallAcceleration(getMeterFactor()*Scalar(9.81)),
	 jumpVelocity(getMeterFactor()*Scalar(4)),
	 probeSize(getInchFactor()*Scalar(12)),
	 maxClimb(getInchFactor()*Scalar(12)),
	 azimuthStep(0),
	 fixAzimuth(false),
	 levelOnExit(false),
	 drawHud(true),hudColor(0.0f,1.0f,0.0f),
	 hudDist(Geometry::dist(getDisplayCenter(),getMainViewer()->getHeadPosition())),
	 hudRadius(getDisplaySize()*0.5f),
	 hudFontSize(float(getUiSize())*1.5f)
	{
	}

void FPSNavigationToolFactory::Configuration::load(const Misc::ConfigurationFileSection& cfs)
	{
	activationToggle=cfs.retrieveValue<bool>("./activationToggle",activationToggle);
	rotateFactors=cfs.retrieveValue<Misc::FixedArray<Scalar,2> >("./rotateFactors",rotateFactors);
	moveSpeeds=cfs.retrieveValue<Misc::FixedArray<Scalar,2> >("./moveSpeeds",moveSpeeds);
	fallAcceleration=cfs.retrieveValue<Scalar>("./fallAcceleration",fallAcceleration);
	jumpVelocity=cfs.retrieveValue<Scalar>("./jumpVelocity",jumpVelocity);
	probeSize=cfs.retrieveValue<Scalar>("./probeSize",probeSize);
	maxClimb=cfs.retrieveValue<Scalar>("./maxClimb",maxClimb);
	azimuthStep=Math::rad(cfs.retrieveValue<Scalar>("./azimuthStep",Math::deg(azimuthStep)));
	fixAzimuth=cfs.retrieveValue<bool>("./fixAzimuth",fixAzimuth);
	levelOnExit=cfs.retrieveValue<bool>("./levelOnExit",levelOnExit);
	drawHud=cfs.retrieveValue<bool>("./drawHud",drawHud);
	hudColor=cfs.retrieveValue<Color>("./hudColor",hudColor);
	hudDist=cfs.retrieveValue<float>("./hudDist",hudDist);
	hudRadius=cfs.retrieveValue<float>("./hudRadius",hudRadius);
	hudFontSize=cfs.retrieveValue<float>("./hudFontSize",hudFontSize);
	}

void FPSNavigationToolFactory::Configuration::save(Misc::ConfigurationFileSection& cfs) const
	{
	cfs.storeValue<bool>("./activationToggle",activationToggle);
	cfs.storeValue<Misc::FixedArray<Scalar,2> >("./rotateFactors",rotateFactors);
	cfs.storeValue<Misc::FixedArray<Scalar,2> >("./moveSpeeds",moveSpeeds);
	cfs.storeValue<Scalar>("./fallAcceleration",fallAcceleration);
	cfs.storeValue<Scalar>("./jumpVelocity",jumpVelocity);
	cfs.storeValue<Scalar>("./probeSize",probeSize);
	cfs.storeValue<Scalar>("./maxClimb",maxClimb);
	cfs.storeValue<Scalar>("./azimuthStep",Math::deg(azimuthStep));
	cfs.storeValue<bool>("./fixAzimuth",fixAzimuth);
	cfs.storeValue<bool>("./levelOnExit",levelOnExit);
	cfs.storeValue<bool>("./drawHud",drawHud);
	cfs.storeValue<Color>("./hudColor",hudColor);
	cfs.storeValue<float>("./hudDist",hudDist);
	cfs.storeValue<float>("./hudRadius",hudRadius);
	cfs.storeValue<float>("./hudFontSize",hudFontSize);
	}

/*****************************************
Methods of class FPSNavigationToolFactory:
*****************************************/

FPSNavigationToolFactory::FPSNavigationToolFactory(ToolManager& toolManager)
	:ToolFactory("FPSNavigationTool",toolManager)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(6);
	
	/* Insert class into class hierarchy: */
	ToolFactory* navigationToolFactory=toolManager.loadClass("SurfaceNavigationTool");
	navigationToolFactory->addChildClass(this);
	addParentClass(navigationToolFactory);
	
	/* Load class settings: */
	config.load(toolManager.getToolClassSection(getClassName()));
	
	/* Set tool class' factory pointer: */
	FPSNavigationTool::factory=this;
	}

FPSNavigationToolFactory::~FPSNavigationToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	FPSNavigationTool::factory=0;
	}

const char* FPSNavigationToolFactory::getName(void) const
	{
	return "FPS (Mouse Look + Buttons)";
	}

const char* FPSNavigationToolFactory::getButtonFunction(int buttonSlotIndex) const
	{
	switch(buttonSlotIndex)
		{
		case 0:
			return "Start / Stop";
		
		case 1:
			return "Strafe Left";
		
		case 2:
			return "Strafe Right";
		
		case 3:
			return "Walk Backwards";
		
		case 4:
			return "Walk Forward";
		
		case 5:
			return "Jump";
		}
	
	/* Never reached; just to make compiler happy: */
	return 0;
	}

Tool* FPSNavigationToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new FPSNavigationTool(this,inputAssignment);
	}

void FPSNavigationToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveFPSNavigationToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("SurfaceNavigationTool");
	}

extern "C" ToolFactory* createFPSNavigationToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	FPSNavigationToolFactory* fpsNavigationToolFactory=new FPSNavigationToolFactory(*toolManager);
	
	/* Return factory object: */
	return fpsNavigationToolFactory;
	}

extern "C" void destroyFPSNavigationToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/******************************************
Static elements of class FPSNavigationTool:
******************************************/

FPSNavigationToolFactory* FPSNavigationTool::factory=0;

/**********************************
Methods of class FPSNavigationTool:
**********************************/

void FPSNavigationTool::applyNavState(void)
	{
	/* Compose and apply the navigation transformation: */
	NavTransform nav=physicalFrame;
	nav*=NavTransform::rotateAround(Point(0,0,headHeight),Rotation::rotateX(elevation));
	
	#if 0
	Vector up(0,0,config.fallAcceleration);
	for(int i=0;i<2;++i)
		if(controlVelocity[i]!=moveVelocity[i])
			up[i]=Math::copysign(Scalar(4)*getMeterFactor(),controlVelocity[i]-moveVelocity[i]);
	nav*=NavTransform::rotateAround(Point(0,0,headHeight),Rotation::rotateFromTo(up,Vector(0,0,1)));
	#endif
	
	if(config.azimuthStep>Scalar(0.1))
		ratchetedAzimuth=Math::floor((azimuth+Math::div2(config.azimuthStep))/config.azimuthStep)*config.azimuthStep;
	else
		ratchetedAzimuth=azimuth;
	nav*=NavTransform::rotate(Rotation::rotateZ(ratchetedAzimuth));
	nav*=Geometry::invert(surfaceFrame);
	setNavigationTransformation(nav);
	}

void FPSNavigationTool::initNavState(void)
	{
	/* Initialize the navigation state: */
	if(mouseAdapter!=0)
		{
		/* Lock the mouse to the center of the window: */
		mouseAdapter->lockMouse();
		}
	
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
	AlignmentData ad(surfaceFrame,newSurfaceFrame,config.probeSize,config.maxClimb);
	Scalar roll;
	align(ad,azimuth,elevation,roll);
	
	/* Reset the movement velocity: */
	controlVelocity=Vector::zero;
	moveVelocity=Vector::zero;
	jump=false;
	
	/* If the initial surface frame was above the surface, lift it back up and start falling: */
	Scalar z=newSurfaceFrame.inverseTransform(surfaceFrame.getOrigin())[2];
	if(z>Scalar(0))
		{
		newSurfaceFrame*=NavTransform::translate(Vector(Scalar(0),Scalar(0),z));
		moveVelocity[2]-=config.fallAcceleration*getCurrentFrameTime();
		}
	
	/* Move the physical frame to the foot position, and adjust the surface frame accordingly: */
	newSurfaceFrame*=Geometry::invert(physicalFrame)*NavTransform::translate(footPos-headPos)*physicalFrame;
	physicalFrame.leftMultiply(NavTransform::translate(footPos-headPos));
	
	/* Apply the initial navigation state: */
	surfaceFrame=newSurfaceFrame;
	applyNavState();
	}

void FPSNavigationTool::stopNavState(void)
	{
	if(config.levelOnExit)
		{
		/* Calculate the main viewer's current head and foot positions: */
		Point headPos=getMainViewer()->getHeadPosition();
		footPos=projectToFloor(headPos);
		headHeight=Geometry::dist(headPos,footPos);
		
		/* Reset the elevation angle: */
		elevation=Scalar(0);
		
		/* Apply the final navigation state: */
		applyNavState();
		}
	
	if(mouseAdapter!=0)
		{
		/* Unlock the mouse pointer: */
		mouseAdapter->unlockMouse();
		}
	}

FPSNavigationTool::FPSNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:SurfaceNavigationTool(factory,inputAssignment),
	 config(FPSNavigationTool::factory->config),
	 buttonDevice(0),
	 mouseAdapter(0),
	 numberRenderer(0)
	{
	}

FPSNavigationTool::~FPSNavigationTool(void)
	{
	}

void FPSNavigationTool::configure(const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Override the current configuration from the given configuration file section: */
	config.load(configFileSection);
	}

void FPSNavigationTool::storeState(Misc::ConfigurationFileSection& configFileSection) const
	{
	/* Save the current configuration to the given configuration file section: */
	config.save(configFileSection);
	}

void FPSNavigationTool::initialize(void)
	{
	/* Get the source input device: */
	InputDevice* device=getButtonDevice(1);
	
	/* Create a virtual input device to shadow the movement buttons: */
	buttonDevice=addVirtualInputDevice("FPSNavigationToolButtonDevice",5,0);
	
	/* Copy the source device's tracking type: */
	buttonDevice->setTrackType(device->getTrackType());
	
	/* Disable the virtual device's glyph: */
	getInputGraphManager()->getInputDeviceGlyph(buttonDevice).disable();
	
	/* Permanently grab the virtual input device: */
	getInputGraphManager()->grabInputDevice(buttonDevice,this);
	
	/* Initialize the virtual input device's position: */
	buttonDevice->setDeviceRay(device->getDeviceRayDirection(),device->getDeviceRayStart());
	buttonDevice->setTransformation(device->getTransformation());
	
	/* Get a pointer to the input device's controlling adapter: */
	mouseAdapter=dynamic_cast<InputDeviceAdapterMouse*>(getInputDeviceManager()->findInputDeviceAdapter(getButtonDevice(0)));
	
	/* Create the number renderer: */
	numberRenderer=new GLNumberRenderer(config.hudFontSize,true);
	}

void FPSNavigationTool::deinitialize(void)
	{
	/* Release the virtual input device: */
	getInputGraphManager()->releaseInputDevice(buttonDevice,this);
	
	/* Destroy the virtual input device: */
	getInputDeviceManager()->destroyInputDevice(buttonDevice);
	buttonDevice=0;
	
	/* Destroy the number renderer: */
	delete numberRenderer;
	numberRenderer=0;
	}

const ToolFactory* FPSNavigationTool::getFactory(void) const
	{
	return factory;
	}

void FPSNavigationTool::buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData)
	{
	if(buttonSlotIndex==0)
		{
		/* Determine the new activation state of the tool: */
		bool newActive;
		if(config.activationToggle)
			{
			newActive=isActive();
			if(cbData->newButtonState)
				newActive=!newActive;
			}
		else
			newActive=cbData->newButtonState;
		
		/* Activate or deactivate the tool: */
		if(isActive())
			{
			if(!newActive)
				{
				/* Deactivate this tool: */
				stopNavState();
				deactivate();
				
				/* Set the forwarded buttons to the states of the real buttons: */
				for(int i=0;i<4;++i)
					buttonDevice->setButtonState(i,getButtonState(1+i));
				}
			}
		else
			{
			/* Try activating this tool: */
			if(newActive&&activate())
				{
				/* Initialize the navigation: */
				initNavState();
				}
			}
		}
	else if(isActive())
		{
		/* Process based on which button was pressed: */
		switch(buttonSlotIndex)
			{
			case 1:
				if(cbData->newButtonState) // Button has just been pressed
					controlVelocity[0]-=config.moveSpeeds[0];
				else // Button has just been released
					controlVelocity[0]+=config.moveSpeeds[0];
				break;
			
			case 2:
				if(cbData->newButtonState) // Button has just been pressed
					controlVelocity[0]+=config.moveSpeeds[0];
				else // Button has just been released
					controlVelocity[0]-=config.moveSpeeds[0];
				break;
			
			case 3:
				if(cbData->newButtonState) // Button has just been pressed
					controlVelocity[1]-=config.moveSpeeds[1];
				else // Button has just been released
					controlVelocity[1]+=config.moveSpeeds[1];
				break;
			
			case 4:
				if(cbData->newButtonState) // Button has just been pressed
					controlVelocity[1]+=config.moveSpeeds[1];
				else // Button has just been released
					controlVelocity[1]-=config.moveSpeeds[1];
				break;
			
			case 5:
				if(cbData->newButtonState) // Button has just been pressed
					{
					/* Request to jump on the next frame: */
					jump=true;
					Vrui::requestUpdate();
					}
				break;
			}
		}
	else
		{
		/* Forward the movement buttons to the virtual input device: */
		buttonDevice->setButtonState(buttonSlotIndex-1,cbData->newButtonState);
		}
	}

void FPSNavigationTool::frame(void)
	{
	InputDevice* device=getButtonDevice(1);
	
	/* Act depending on this tool's current state: */
	if(isActive())
		{
		bool update=false;
		
		/* Get the device's linear velocity and calculate its left/right and up/down components: */
		Vector right=getForwardDirection()^getUpDirection();
		right.normalize();
		Scalar x=(right*device->getLinearVelocity())*getFrameTime();
		Scalar y=(getUpDirection()*device->getLinearVelocity())*getFrameTime();
		if(x!=Scalar(0)||y!=Scalar(0))
			{
			/* Update the azimuth angle: */
			if(config.rotateFactors[0]!=Scalar(0))
				azimuth=wrapAngle(azimuth+x/config.rotateFactors[0]);
			
			/* Update the elevation angle: */
			if(config.rotateFactors[1]!=Scalar(0))
				{
				Scalar zenith=Math::rad(Scalar(90));
				elevation=Math::clamp(elevation+y/config.rotateFactors[1],-zenith,zenith);
				}
			
			update=true;
			}
		
		/* Calculate the new head and foot positions: */
		Point newHeadPos=getMainViewer()->getHeadPosition();
		Point newFootPos=projectToFloor(newHeadPos);
		headHeight=Geometry::dist(newHeadPos,newFootPos);
		
		/* Check for movement: */
		if(controlVelocity!=Vector::zero||moveVelocity!=Vector::zero||newFootPos!=footPos||jump)
			update=true;
		
		/* Update the movement velocity based on the control velocity: */
		#if 0
		Scalar maxAccel=Scalar(4)*getMeterFactor()*getCurrentFrameTime();
		for(int i=0;i<2;++i)
			{
			Scalar dv=controlVelocity[i]-moveVelocity[i];
			if(Math::abs(dv)<maxAccel)
				moveVelocity[i]=controlVelocity[i];
			else
				moveVelocity[i]+=Math::copysign(maxAccel,dv);
			}
		#else
		for(int i=0;i<2;++i)
			moveVelocity[i]=controlVelocity[i];
		#endif
		
		if(update)
			{
			/* Create a physical navigation frame around the new foot position: */
			calcPhysicalFrame(newFootPos);
			
			/* Calculate the movement from walking: */
			Vector move=newFootPos-footPos;
			footPos=newFootPos;
			
			/* Add movement velocity: */
			move+=moveVelocity*getCurrentFrameTime();
			
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
			AlignmentData ad(surfaceFrame,newSurfaceFrame,config.probeSize,config.maxClimb);
			align(ad);
			
			if(!config.fixAzimuth)
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
				/* Lift the aligned frame back up to the original altitude and fall: */
				newSurfaceFrame*=NavTransform::translate(Vector(Scalar(0),Scalar(0),z));
				moveVelocity[2]-=config.fallAcceleration*getCurrentFrameTime();
				}
			else
				{
				/* Stop falling: */
				moveVelocity[2]=Scalar(0);
				
				/* Check if the user wants to jump: */
				if(jump)
					moveVelocity[2]=config.jumpVelocity;
				}
			
			/* Apply the newly aligned surface frame: */
			surfaceFrame=newSurfaceFrame;
			applyNavState();
			
			if(moveVelocity[0]!=Scalar(0)||moveVelocity[1]!=Scalar(0)||z>Scalar(0))
				{
				/* Request another frame: */
				scheduleUpdate(getApplicationTime()+1.0/125.0);
				}
			}
		
		/* Reset the jump request flag: */
		jump=false;
		}
	
	/* Update the virtual input device: */
	buttonDevice->setDeviceRay(device->getDeviceRayDirection(),device->getDeviceRayStart());
	buttonDevice->setTransformation(device->getTransformation());
	}

void FPSNavigationTool::display(GLContextData& contextData) const
	{
	if(isActive()&&config.drawHud)
		{
		glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
		glDisable(GL_LIGHTING);
		glDepthRange(0.0,0.0);
		glLineWidth(1.0f);
		glColor(config.hudColor);
		
		/* Get the HUD layout parameters: */
		float y=config.hudDist;
		float r=config.hudRadius;
		float s=config.hudFontSize;
		
		/* Go to the physical frame: */
		glPushMatrix();
		glMultMatrix(physicalFrame);
		
		/* Go to the HUD frame: */
		glRotatef(Math::deg(azimuth-ratchetedAzimuth),0.0f,0.0f,-1.0f);
		glTranslatef(0.0f,y,float(headHeight));
		glRotatef(90.0f,1.0f,0.0f,0.0f);
		
		/* Draw the boresight crosshairs: */
		glBegin(GL_LINES);
		glVertex2f(-r*0.05f,   0.00f);
		glVertex2f(-r*0.02f,   0.00f);
		glVertex2f( r*0.02f,   0.00f);
		glVertex2f( r*0.05f,   0.00f);
		glVertex2f(   0.00f,-r*0.05f);
		glVertex2f(   0.00f,-r*0.02f);
		glVertex2f(   0.00f, r*0.02f);
		glVertex2f(   0.00f, r*0.05f);
		glEnd();
		
		/* Get the tool's orientation Euler angles in degrees: */
		float azimuthDeg=Math::deg(azimuth);
		
		/* Draw the compass ribbon: */
		glBegin(GL_LINES);
		glVertex2f(-r,r);
		glVertex2f(r,r);
		glEnd();
		glBegin(GL_LINE_STRIP);
		glVertex2f(-s*0.5f,r+s);
		glVertex2f(0.0f,r);
		glVertex2f(s*0.5f,r+s);
		glEnd();
		
		/* Draw the azimuth tick marks: */
		glBegin(GL_LINES);
		for(int az=0;az<360;az+=10)
			{
			float dist=float(az)-azimuthDeg;
			if(dist<-180.0f)
				dist+=360.0f;
			if(dist>180.0f)
				dist-=360.0f;
			if(Math::abs(dist)<=60.0f)
				{
				float x=dist*r/60.0f;
				glVertex2f(x,r);
				glVertex2f(x,r-(az%30==0?s*1.5f:s));
				}
			}
		glEnd();
		
		/* Draw the azimuth labels: */
		GLNumberRenderer::Vector pos;
		pos[1]=r-s*2.0f;
		pos[2]=0.0f;
		for(int az=0;az<360;az+=30)
			{
			float dist=float(az)-azimuthDeg;
			if(dist<-180.0f)
				dist+=360.0f;
			if(dist>180.0f)
				dist-=360.0f;
			if(Math::abs(dist)<=60.0f)
				{
				pos[0]=dist*r/60.0f;
				numberRenderer->drawNumber(pos,az,contextData,0,1);
				}
			}
		
		glPopMatrix();
		glDepthRange(0.0,1.0);
		glPopAttrib();
		}
	}

std::vector<InputDevice*> FPSNavigationTool::getForwardedDevices(void)
	{
	std::vector<InputDevice*> result;
	result.push_back(buttonDevice);
	return result;
	}

InputDeviceFeatureSet FPSNavigationTool::getSourceFeatures(const InputDeviceFeature& forwardedFeature)
	{
	/* Paranoia: Check if the forwarded feature is on the transformed device: */
	if(forwardedFeature.getDevice()!=buttonDevice)
		Misc::throwStdErr("FPSNavigationTool::getSourceFeatures: Forwarded feature is not on transformed device");
	
	/* Return the source feature: */
	InputDeviceFeatureSet result;
	result.push_back(input.getButtonSlotFeature(1+forwardedFeature.getIndex()));
	return result;
	}

InputDevice* FPSNavigationTool::getSourceDevice(const InputDevice* forwardedDevice)
	{
	/* Paranoia: Check if the forwarded device is the same as the transformed device: */
	if(forwardedDevice!=buttonDevice)
		Misc::throwStdErr("FPSNavigationTool::getSourceDevice: Given forwarded device is not transformed device");
	
	/* Return the source device: */
	return getButtonDevice(1);
	}

InputDeviceFeatureSet FPSNavigationTool::getForwardedFeatures(const InputDeviceFeature& sourceFeature)
	{
	/* Get the source feature's assignment slot index: */
	int slotIndex=input.findFeature(sourceFeature);
	
	/* Paranoia: Check if the source feature belongs to this tool: */
	if(slotIndex<0)
		Misc::throwStdErr("FPSNavigationTool::getForwardedFeatures: Source feature is not part of tool's input assignment");
	
	/* Return the forwarded feature: */
	InputDeviceFeatureSet result;
	if(slotIndex>=1)
		result.push_back(InputDeviceFeature(buttonDevice,InputDevice::BUTTON,slotIndex-1));
	return result;
	}

}
