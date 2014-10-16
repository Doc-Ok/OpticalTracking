/***********************************************************************
HelicopterNavigationTool - Class for navigation tools using a simplified
helicopter flight model, a la Enemy Territory: Quake Wars' Anansi. Yeah,
I like that -- wanna fight about it?
Copyright (c) 2007-2013 Oliver Kreylos

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

#include <Vrui/Tools/HelicopterNavigationTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ArrayValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Geometry/GeometryValueCoders.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLValueCoders.h>
#include <GL/GLContextData.h>
#include <GL/GLNumberRenderer.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <Vrui/Viewer.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/***************************************************************
Methods of class HelicopterNavigationToolFactory::Configuration:
***************************************************************/

HelicopterNavigationToolFactory::Configuration::Configuration(void)
	:activationToggle(true),
	 g(getMeterFactor()*Scalar(9.81)),
	 collectiveMin(Scalar(0)),collectiveMax(g*Scalar(1.5)),
	 thrust(g*Scalar(1)),
	 brake(g*Scalar(0.5)),
	 probeSize(getMeterFactor()*Scalar(1.5)),
	 maxClimb(getMeterFactor()*Scalar(1.5)),
	 drawHud(true),hudColor(0.0f,1.0f,0.0f),
	 hudDist(Geometry::dist(getDisplayCenter(),getMainViewer()->getHeadPosition())),
	 hudRadius(getDisplaySize()),
	 hudFontSize(float(getUiSize())*1.5f)
	{
	/* Initialize rotation scaling factors: */
	rotateFactors[0]=Scalar(-60);
	rotateFactors[1]=Scalar(-60);
	rotateFactors[2]=Scalar(45);
	
	/* Initialize drag coefficients: */
	dragCoefficients[0]=Scalar(0.3);
	dragCoefficients[1]=Scalar(0.1);
	dragCoefficients[2]=Scalar(0.3);
	
	/* Initialize view angles: */
	viewAngleFactors[0]=Scalar(35);
	viewAngleFactors[1]=Scalar(-25);
	}

void HelicopterNavigationToolFactory::Configuration::load(const Misc::ConfigurationFileSection& cfs)
	{
	/* Get parameters: */
	activationToggle=cfs.retrieveValue<bool>("./activationToggle",activationToggle);
	rotateFactors=cfs.retrieveValue<Misc::FixedArray<Scalar,3> >("./rotateFactors",rotateFactors);
	g=cfs.retrieveValue<Scalar>("./g",g);
	collectiveMin=cfs.retrieveValue<Scalar>("./collectiveMin",collectiveMin);
	collectiveMax=cfs.retrieveValue<Scalar>("./collectiveMax",collectiveMax);
	thrust=cfs.retrieveValue<Scalar>("./thrust",thrust);
	brake=cfs.retrieveValue<Scalar>("./brake",brake);
	dragCoefficients=cfs.retrieveValue<Misc::FixedArray<Scalar,3> >("./dragCoefficients",dragCoefficients);
	for(int i=0;i<3;++i)
		dragCoefficients[i]=-Math::abs(dragCoefficients[i]);
	viewAngleFactors=cfs.retrieveValue<Misc::FixedArray<Scalar,2> >("./viewAngleFactors",viewAngleFactors);
	probeSize=cfs.retrieveValue<Scalar>("./probeSize",probeSize);
	maxClimb=cfs.retrieveValue<Scalar>("./maxClimb",maxClimb);
	drawHud=cfs.retrieveValue<bool>("./drawHud",drawHud);
	hudColor=cfs.retrieveValue<Color>("./hudColor",hudColor);
	hudDist=cfs.retrieveValue<float>("./hudDist",hudDist);
	hudRadius=cfs.retrieveValue<float>("./hudRadius",hudRadius);
	hudFontSize=cfs.retrieveValue<float>("./hudFontSize",hudFontSize);
	}

void HelicopterNavigationToolFactory::Configuration::save(Misc::ConfigurationFileSection& cfs) const
	{
	/* Save parameters: */
	cfs.storeValue<bool>("./activationToggle",activationToggle);
	cfs.storeValue<Misc::FixedArray<Scalar,3> >("./rotateFactors",rotateFactors);
	cfs.storeValue<Scalar>("./g",g);
	cfs.storeValue<Scalar>("./collectiveMin",collectiveMin);
	cfs.storeValue<Scalar>("./collectiveMax",collectiveMax);
	cfs.storeValue<Scalar>("./thrust",thrust);
	cfs.storeValue<Scalar>("./brake",brake);
	cfs.storeValue<Misc::FixedArray<Scalar,3> >("./dragCoefficients",dragCoefficients);
	cfs.storeValue<Misc::FixedArray<Scalar,2> >("./viewAngleFactors",viewAngleFactors);
	cfs.storeValue<Scalar>("./probeSize",probeSize);
	cfs.storeValue<Scalar>("./maxClimb",maxClimb);
	cfs.storeValue<bool>("./drawHud",drawHud);
	cfs.storeValue<Color>("./hudColor",hudColor);
	cfs.storeValue<float>("./hudDist",hudDist);
	cfs.storeValue<float>("./hudRadius",hudRadius);
	cfs.storeValue<float>("./hudFontSize",hudFontSize);
	}

/************************************************
Methods of class HelicopterNavigationToolFactory:
************************************************/

HelicopterNavigationToolFactory::HelicopterNavigationToolFactory(ToolManager& toolManager)
	:ToolFactory("HelicopterNavigationTool",toolManager)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(3);
	layout.setNumValuators(6);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	config.load(cfs);
	
	/* Insert class into class hierarchy: */
	ToolFactory* navigationToolFactory=toolManager.loadClass("SurfaceNavigationTool");
	navigationToolFactory->addChildClass(this);
	addParentClass(navigationToolFactory);
	
	/* Set tool class' factory pointer: */
	HelicopterNavigationTool::factory=this;
	}

HelicopterNavigationToolFactory::~HelicopterNavigationToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	HelicopterNavigationTool::factory=0;
	}

const char* HelicopterNavigationToolFactory::getName(void) const
	{
	return "Helicopter Flight";
	}

const char* HelicopterNavigationToolFactory::getButtonFunction(int buttonSlotIndex) const
	{
	switch(buttonSlotIndex)
		{
		case 0:
			return "Start / Stop";
		
		case 1:
			return "Thrusters";
		
		case 2:
			return "Brake";
		}
	
	/* Never reached; just to make compiler happy: */
	return 0;
	}

const char* HelicopterNavigationToolFactory::getValuatorFunction(int valuatorSlotIndex) const
	{
	switch(valuatorSlotIndex)
		{
		case 0:
			return "Cyclic Pitch";
		
		case 1:
			return "Cyclic Roll";
		
		case 2:
			return "Rudder Yaw";
		
		case 3:
			return "Collective";
		
		case 4:
			return "Look Left/Right";
		
		case 5:
			return "Look Up/Down";
		}
	
	/* Never reached; just to make compiler happy: */
	return 0;
	}

Tool* HelicopterNavigationToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new HelicopterNavigationTool(this,inputAssignment);
	}

void HelicopterNavigationToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveHelicopterNavigationToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("SurfaceNavigationTool");
	}

extern "C" ToolFactory* createHelicopterNavigationToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	HelicopterNavigationToolFactory* helicopterNavigationToolFactory=new HelicopterNavigationToolFactory(*toolManager);
	
	/* Return factory object: */
	return helicopterNavigationToolFactory;
	}

extern "C" void destroyHelicopterNavigationToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/*************************************************
Static elements of class HelicopterNavigationTool:
*************************************************/

HelicopterNavigationToolFactory* HelicopterNavigationTool::factory=0;

/*****************************************
Methods of class HelicopterNavigationTool:
*****************************************/

void HelicopterNavigationTool::applyNavState(void)
	{
	/* Compose and apply the navigation transformation: */
	NavTransform nav=physicalFrame;
	nav*=NavTransform::rotate(Rotation::rotateZ(getValuatorState(4)*Math::rad(config.viewAngleFactors[0])));
	nav*=NavTransform::rotate(Rotation::rotateX(getValuatorState(5)*Math::rad(config.viewAngleFactors[1])));
	nav*=NavTransform::rotate(orientation);
	nav*=Geometry::invert(surfaceFrame);
	setNavigationTransformation(nav);
	}

void HelicopterNavigationTool::initNavState(void)
	{
	/* Set up a physical navigation frame around the main viewer's current head position: */
	calcPhysicalFrame(getMainViewer()->getHeadPosition());
	
	/* Calculate the initial environment-aligned surface frame in navigation coordinates: */
	surfaceFrame=getInverseNavigationTransformation()*physicalFrame;
	NavTransform newSurfaceFrame=surfaceFrame;
	
	/* Align the initial frame with the application's surface: */
	AlignmentData ad(surfaceFrame,newSurfaceFrame,config.probeSize,config.maxClimb);
	align(ad);
	
	/* Calculate the orientation of the current navigation transformation in the aligned surface frame: */
	orientation=Geometry::invert(surfaceFrame.getRotation())*newSurfaceFrame.getRotation();
	
	/* Reset the movement velocity: */
	velocity=Vector::zero;
	
	/* If the initial surface frame was above the surface, lift it back up: */
	elevation=newSurfaceFrame.inverseTransform(surfaceFrame.getOrigin())[2];
	if(elevation<config.probeSize)
		{  
		/* Collide with the ground: */
		elevation=config.probeSize;
		Vector y=orientation.getDirection(1);
		Scalar azimuth=Math::atan2(y[0],y[1]);
		orientation=Rotation::rotateZ(-azimuth);
		}
	newSurfaceFrame*=NavTransform::translate(Vector(Scalar(0),Scalar(0),elevation));
	
	/* Apply the initial navigation state: */
	surfaceFrame=newSurfaceFrame;
	applyNavState();
	}

HelicopterNavigationTool::HelicopterNavigationTool(const ToolFactory* sFactory,const ToolInputAssignment& inputAssignment)
	:SurfaceNavigationTool(sFactory,inputAssignment),
	 numberRenderer(0),
	 config(factory->config)
	{
	}

HelicopterNavigationTool::~HelicopterNavigationTool(void)
	{
	delete numberRenderer;
	}

void HelicopterNavigationTool::configure(const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Update the configuration: */
	config.load(configFileSection);
	}

void HelicopterNavigationTool::storeState(Misc::ConfigurationFileSection& configFileSection) const
	{
	/* Save the current configuration: */
	config.save(configFileSection);
	}

void HelicopterNavigationTool::initialize(void)
	{
	numberRenderer=new GLNumberRenderer(config.hudFontSize,true);
	}

const ToolFactory* HelicopterNavigationTool::getFactory(void) const
	{
	return factory;
	}

void HelicopterNavigationTool::buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData)
	{
	/* Process based on which button was pressed: */
	if(buttonSlotIndex==0)
		{
		bool newActive;
		if(config.activationToggle)
			{
			newActive=isActive();
			if(cbData->newButtonState)
				newActive=!newActive;
			}
		else
			newActive=cbData->newButtonState;
		
		if(isActive())
			{
			if(!newActive)
				{
				/* Deactivate this tool: */
				deactivate();
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
	}

void HelicopterNavigationTool::frame(void)
	{
	/* Act depending on this tool's current state: */
	if(isActive())
		{
		/* Use the average frame time as simulation time: */
		Scalar dt=getCurrentFrameTime();
		
		/* Update the current position based on the current velocity: */
		NavTransform newSurfaceFrame=surfaceFrame;
		newSurfaceFrame*=NavTransform::translate(velocity*dt);
		
		/* Re-align the surface frame with the surface: */
		Point initialOrigin=newSurfaceFrame.getOrigin();
		AlignmentData ad(surfaceFrame,newSurfaceFrame,config.probeSize,config.maxClimb);
		align(ad);
		
		/* Update the orientation to reflect rotations in the surface frame: */
		orientation*=Geometry::invert(surfaceFrame.getRotation())*newSurfaceFrame.getRotation();
		
		/* Check if the initial surface frame was above the surface: */
		elevation=newSurfaceFrame.inverseTransform(initialOrigin)[2];
		if(elevation<config.probeSize)
			{
			/* Collide with the ground: */
			elevation=config.probeSize;
			Vector y=orientation.getDirection(1);
			Scalar azimuth=Math::atan2(y[0],y[1]);
			orientation=Rotation::rotateZ(-azimuth);
			velocity=Vector::zero;
			}
		
		/* Lift the aligned frame back up to the original altitude: */
		newSurfaceFrame*=NavTransform::translate(Vector(Scalar(0),Scalar(0),elevation));
		
		/* Update the current orientation based on the pitch, roll, and yaw controls: */
		Vector rot;
		for(int i=0;i<3;++i)
			rot[i]=getValuatorState(i)*Math::rad(config.rotateFactors[i]);
		orientation.leftMultiply(Rotation::rotateScaledAxis(rot*dt));
		orientation.renormalize();
		
		/* Calculate the current acceleration based on gravity, collective, thrust, and brake: */
		Vector accel=Vector(0,0,-config.g);
		Scalar collective=Scalar(0.5)*(Scalar(1)-getValuatorState(3))*(config.collectiveMax-config.collectiveMin)+config.collectiveMin;
		accel+=orientation.inverseTransform(Vector(0,0,collective));
		if(getButtonState(1))
			accel+=orientation.inverseTransform(Vector(0,config.thrust,0));
		if(getButtonState(2))
			accel+=orientation.inverseTransform(Vector(0,-config.brake,0));
		
		/* Calculate drag: */
		Vector localVelocity=orientation.transform(velocity);
		Vector drag;
		for(int i=0;i<3;++i)
			drag[i]=localVelocity[i]*config.dragCoefficients[i];
		accel+=orientation.inverseTransform(drag);
		
		/* Rotate the helicopter body slightly if there is off-axis drag: */
		Vector torque=Vector(0.0,-0.0002,0.0002)^localVelocity;
		orientation.leftMultiply(Rotation::rotateScaledAxis(torque*dt));
		orientation.renormalize();
		
		/* Update the current velocity: */
		velocity+=accel*dt;
		
		/* Apply the newly aligned surface frame: */
		surfaceFrame=newSurfaceFrame;
		applyNavState();
		
		/* Request another frame: */
		scheduleUpdate(getApplicationTime()+1.0/125.0);
		}
	}

void HelicopterNavigationTool::display(GLContextData& contextData) const
	{
	if(isActive()&&config.drawHud)
		{
		glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
		glDisable(GL_LIGHTING);
		glLineWidth(1.0f);
		glColor(config.hudColor);
		
		/* Get the HUD layout parameters: */
		float y=config.hudDist;
		float r=config.hudRadius;
		float s=config.hudFontSize;
		
		/* Go to the view-shifted physical frame: */
		glPushMatrix();
		glMultMatrix(physicalFrame);
		glRotate(getValuatorState(4)*config.viewAngleFactors[0],Vector(0,0,1));
		glRotate(getValuatorState(5)*config.viewAngleFactors[1],Vector(1,0,0));
		
		/* Go to the HUD frame: */
		glTranslatef(0.0f,y,0.0f);
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
		
		/* Get the helicopter's orientation Euler angles: */
		Scalar angles[3];
		calcEulerAngles(orientation,angles);
		float azimuth=Math::deg(angles[0]);
		float elevation=Math::deg(angles[1]);
		float roll=Math::deg(angles[2]);
		
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
			float dist=float(az)-azimuth;
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
			float dist=float(az)-azimuth;
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
		
		/* Draw the flight path marker: */
		Vector vel=orientation.transform(velocity);
		if(vel[1]>Scalar(0))
			{
			vel*=y/vel[1];
			Scalar maxVel=Math::max(Math::abs(vel[0]),Math::abs(vel[2]));
			if(maxVel>=Scalar(r))
				{
				vel[0]*=Scalar(r)/maxVel;
				vel[2]*=Scalar(r)/maxVel;
				glColor3f(1.0f,0.0f,0.0f);
				}
			
			glBegin(GL_LINE_LOOP);
			glVertex2f(vel[0]-r*0.02f,vel[2]+  0.00f);
			glVertex2f(vel[0]+  0.00f,vel[2]-r*0.02f);
			glVertex2f(vel[0]+r*0.02f,vel[2]+  0.00f);
			glVertex2f(vel[0]+  0.00f,vel[2]+r*0.02f);
			glEnd();
			}
		
		glColor(config.hudColor);
		
		glRotatef(-roll,0.0f,0.0f,1.0f);
		
		/* Draw the artificial horizon ladder: */
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(10,0xaaaaU);
		glBegin(GL_LINES);
		for(int el=-175;el<0;el+=5)
			{
			float dist=elevation+float(el);
			if(dist<-180.0f)
				dist+=360.0f;
			if(dist>180.0f)
				dist-=360.0f;
			if(Math::abs(dist)<90.0f)
				{
				float z=Math::tan(Math::rad(dist))*y;
				if(Math::abs(z)<=r)
					{
					float x=el%10==0?r*0.2f:r*0.1f;
					glVertex2f(-x,z);
					glVertex2f(x,z);
					}
				}
			}
		glEnd();
		glDisable(GL_LINE_STIPPLE);
		
		glBegin(GL_LINES);
		for(int el=0;el<=180;el+=5)
			{
			float dist=elevation+float(el);
			if(dist<-180.0f)
				dist+=360.0f;
			if(dist>180.0f)
				dist-=360.0f;
			if(Math::abs(dist)<90.0f)
				{
				float z=Math::tan(Math::rad(dist))*y;
				if(Math::abs(z)<=r)
					{
					float x=el%10==0?r*0.2f:r*0.1f;
					glVertex2f(-x,z);
					glVertex2f(x,z);
					}
				}
			}
		glEnd();
		
		/* Draw the artificial horizon labels: */
		pos[0]=r*0.2f+s;
		pos[2]=0.0f;
		for(int el=-170;el<=180;el+=10)
			{
			float dist=elevation+float(el);
			if(dist<-180.0f)
				dist+=360.0f;
			if(dist>180.0f)
				dist-=360.0f;
			if(Math::abs(dist)<90.0f)
				{
				float z=Math::tan(Math::rad(dist))*y;
				if(Math::abs(z)<=r)
					{
					pos[1]=z;
					int drawEl=el;
					if(drawEl>90)
						drawEl=180-el;
					else if(drawEl<-90)
						drawEl=-180-el;
					numberRenderer->drawNumber(pos,drawEl,contextData,-1,0);
					}
				}
			}
		
		glPopMatrix();
		glPopAttrib();
		}
	}

}
