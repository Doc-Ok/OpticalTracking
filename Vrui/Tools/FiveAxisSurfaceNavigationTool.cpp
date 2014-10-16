/***********************************************************************
FiveAxisSurfaceNavigationTool - Class for navigation tools that use a
six-axis spaceball or similar input device to move along an application-
defined surface.
Copyright (c) 2012-2013 Oliver Kreylos

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

#include <Vrui/Tools/FiveAxisSurfaceNavigationTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ArrayValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLContextData.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/********************************************************************
Methods of class FiveAxisSurfaceNavigationToolFactory::Configuration:
********************************************************************/

FiveAxisSurfaceNavigationToolFactory::Configuration::Configuration(void)
	:translateFactors(getDisplaySize()),
	 rotateFactors(Scalar(180)),
	 zoomFactor(Scalar(1)),
	 probeSize(getUiSize()),
	 maxClimb(getDisplaySize()),
	 fixAzimuth(false),
	 showCompass(true),compassSize(getUiSize()*Scalar(5)),compassThickness(getUiSize()*Scalar(0.5)),
	 showScreenCenter(true)
	{
	}

void FiveAxisSurfaceNavigationToolFactory::Configuration::load(const Misc::ConfigurationFileSection& cfs)
	{
	/* Get parameters: */
	translateFactors=cfs.retrieveValue<Misc::FixedArray<Scalar,2> >("./translateFactors",translateFactors);
	rotateFactors=cfs.retrieveValue<Misc::FixedArray<Scalar,2> >("./rotateFactors",rotateFactors);
	zoomFactor=cfs.retrieveValue<Scalar>("./zoomFactor",zoomFactor);
	probeSize=cfs.retrieveValue<Scalar>("./probeSize",probeSize);
	maxClimb=cfs.retrieveValue<Scalar>("./maxClimb",maxClimb);
	fixAzimuth=cfs.retrieveValue<bool>("./fixAzimuth",fixAzimuth);
	showCompass=cfs.retrieveValue<bool>("./showCompass",showCompass);
	compassSize=cfs.retrieveValue<Scalar>("./compassSize",compassSize);
	compassThickness=cfs.retrieveValue<Scalar>("./compassThickness",compassThickness);
	showScreenCenter=cfs.retrieveValue<bool>("./showScreenCenter",showScreenCenter);
	}

void FiveAxisSurfaceNavigationToolFactory::Configuration::save(Misc::ConfigurationFileSection& cfs) const
	{
	/* Save parameters: */
	cfs.storeValue<Misc::FixedArray<Scalar,2> >("./translateFactors",translateFactors);
	cfs.storeValue<Misc::FixedArray<Scalar,2> >("./rotateFactors",rotateFactors);
	cfs.storeValue<Scalar>("./zoomFactor",zoomFactor);
	cfs.storeValue<Scalar>("./probeSize",probeSize);
	cfs.storeValue<Scalar>("./maxClimb",maxClimb);
	cfs.storeValue<bool>("./fixAzimuth",fixAzimuth);
	cfs.storeValue<bool>("./showCompass",showCompass);
	cfs.storeValue<Scalar>("./compassSize",compassSize);
	cfs.storeValue<Scalar>("./compassThickness",compassThickness);
	cfs.storeValue<bool>("./showScreenCenter",showScreenCenter);
	}

/*****************************************************
Methods of class FiveAxisSurfaceNavigationToolFactory:
*****************************************************/

FiveAxisSurfaceNavigationToolFactory::FiveAxisSurfaceNavigationToolFactory(ToolManager& toolManager)
	:ToolFactory("FiveAxisSurfaceNavigationTool",toolManager)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(0);
	layout.setNumValuators(5);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	config.load(cfs);
	
	/* Insert class into class hierarchy: */
	ToolFactory* navigationToolFactory=toolManager.loadClass("SurfaceNavigationTool");
	navigationToolFactory->addChildClass(this);
	addParentClass(navigationToolFactory);
	
	/* Set tool class' factory pointer: */
	FiveAxisSurfaceNavigationTool::factory=this;
	}

FiveAxisSurfaceNavigationToolFactory::~FiveAxisSurfaceNavigationToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	FiveAxisSurfaceNavigationTool::factory=0;
	}

const char* FiveAxisSurfaceNavigationToolFactory::getName(void) const
	{
	return "Five-Axis";
	}

const char* FiveAxisSurfaceNavigationToolFactory::getValuatorFunction(int valuatorSlotIndex) const
	{
	switch(valuatorSlotIndex)
		{
		case 0:
			return "Translate X";
		
		case 1:
			return "Translate Y";
		
		case 2:
			return "Rotate Z (Yaw)";
		
		case 3:
			return "Rotate X (Pitch)";
		
		case 4:
			return "Zoom";
		}
	
	/* Never reached; just to make compiler happy: */
	return 0;
	}

Tool* FiveAxisSurfaceNavigationToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new FiveAxisSurfaceNavigationTool(this,inputAssignment);
	}

void FiveAxisSurfaceNavigationToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveFiveAxisSurfaceNavigationToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("SurfaceNavigationTool");
	}

extern "C" ToolFactory* createFiveAxisSurfaceNavigationToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	FiveAxisSurfaceNavigationToolFactory* fiveAxisSurfaceNavigationToolFactory=new FiveAxisSurfaceNavigationToolFactory(*toolManager);
	
	/* Return factory object: */
	return fiveAxisSurfaceNavigationToolFactory;
	}

extern "C" void destroyFiveAxisSurfaceNavigationToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/********************************************************
Methods of class FiveAxisSurfaceNavigationTool::DataItem:
********************************************************/

FiveAxisSurfaceNavigationTool::DataItem::DataItem(void)
	:compassDisplayList(glGenLists(1))
	{
	}

FiveAxisSurfaceNavigationTool::DataItem::~DataItem(void)
	{
	glDeleteLists(compassDisplayList,1);
	}

/******************************************************
Static elements of class FiveAxisSurfaceNavigationTool:
******************************************************/

FiveAxisSurfaceNavigationToolFactory* FiveAxisSurfaceNavigationTool::factory=0;

/**********************************************
Methods of class FiveAxisSurfaceNavigationTool:
**********************************************/

void FiveAxisSurfaceNavigationTool::applyNavState(void) const
	{
	/* Compose and apply the navigation transformation: */
	NavTransform nav=physicalFrame;
	nav*=NavTransform::rotate(Rotation::rotateX(elevation));
	nav*=NavTransform::rotate(Rotation::rotateZ(azimuth));
	nav*=Geometry::invert(surfaceFrame);
	setNavigationTransformation(nav);
	}

void FiveAxisSurfaceNavigationTool::initNavState(void)
	{
	/* Set up a physical navigation frame around the display's center: */
	calcPhysicalFrame(getDisplayCenter());
	
	/* Calculate the initial environment-aligned surface frame in navigation coordinates: */
	surfaceFrame=getInverseNavigationTransformation()*physicalFrame;
	NavTransform newSurfaceFrame=surfaceFrame;
	
	/* Align the initial frame with the application's surface and calculate Euler angles: */
	AlignmentData ad(surfaceFrame,newSurfaceFrame,config.probeSize,config.maxClimb);
	Scalar roll;
	align(ad,azimuth,elevation,roll);
	
	/* Limit elevation angle to down direction: */
	if(elevation<Scalar(0))
		elevation=Scalar(0);
	
	if(config.showCompass)
		{
		/* Start showing the virtual compass: */
		showCompass=true;
		}
	
	/* Apply the newly aligned surface frame: */
	surfaceFrame=newSurfaceFrame;
	applyNavState();
	}

void FiveAxisSurfaceNavigationTool::realignSurfaceFrame(NavTransform& newSurfaceFrame)
	{
	/* Re-align the surface frame with the surface: */
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
	
	/* Store and apply the newly aligned surface frame: */
	surfaceFrame=newSurfaceFrame;
	applyNavState();
	}

void FiveAxisSurfaceNavigationTool::navigationTransformationChangedCallback(Misc::CallbackData* cbData)
	{
	/* Stop showing the virtual compass if this tool is no longer active: */
	if(!isActive())
		showCompass=false;
	}

FiveAxisSurfaceNavigationTool::FiveAxisSurfaceNavigationTool(const ToolFactory* sFactory,const ToolInputAssignment& inputAssignment)
	:SurfaceNavigationTool(sFactory,inputAssignment),
	 config(factory->config),
	 numActiveAxes(0)
	{
	/* Register a callback when the navigation transformation changes: */
	getNavigationTransformationChangedCallbacks().add(this,&FiveAxisSurfaceNavigationTool::navigationTransformationChangedCallback);
	}

FiveAxisSurfaceNavigationTool::~FiveAxisSurfaceNavigationTool(void)
	{
	/* Remove then navigation transformation change callback: */
	getNavigationTransformationChangedCallbacks().remove(this,&FiveAxisSurfaceNavigationTool::navigationTransformationChangedCallback);
	}

void FiveAxisSurfaceNavigationTool::configure(const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Update the configuration: */
	config.load(configFileSection);
	}

void FiveAxisSurfaceNavigationTool::storeState(Misc::ConfigurationFileSection& configFileSection) const
	{
	/* Save the current configuration: */
	config.save(configFileSection);
	}

const ToolFactory* FiveAxisSurfaceNavigationTool::getFactory(void) const
	{
	return factory;
	}

void FiveAxisSurfaceNavigationTool::valuatorCallback(int valuatorSlotIndex,InputDevice::ValuatorCallbackData* cbData)
	{
	/* Check if the valuator became active or inactive: */
	if(cbData->oldValuatorValue==0.0&&cbData->newValuatorValue!=0.0)
		{
		/* Activate the axis: */
		++numActiveAxes;
		
		/* Try activating navigation if it is not already active: */
		if(!isActive()&&activate())
			{
			/* Initialize transient navigation state: */
			initNavState();
			}
		}
	if(cbData->oldValuatorValue!=0.0&&cbData->newValuatorValue==0.0)
		{
		/* Deactivate the axis: */
		--numActiveAxes;
		if(numActiveAxes==0)
			{
			/* Deactivate navigation: */
			deactivate();
			}
		}
	}

void FiveAxisSurfaceNavigationTool::frame(void)
	{
	if(isActive())
		{
		/* Calculate the translation vector: */
		Vector delta=Vector::zero;
		for(int i=0;i<2;++i)
			delta[i]=getValuatorState(i)*config.translateFactors[i]*getCurrentFrameTime();
		delta=Rotation::rotateZ(-azimuth).transform(delta);
		
		/* Calculate the new rotation angles: */
		azimuth=wrapAngle(azimuth+getValuatorState(2)*Math::rad(config.rotateFactors[0])*getCurrentFrameTime());
		elevation=Math::clamp(elevation+getValuatorState(3)*Math::rad(config.rotateFactors[1])*getCurrentFrameTime(),Scalar(0),Math::rad(Scalar(90)));
		
		/* Calculate the incremental scale factor: */
		Scalar scale=config.zoomFactor*Scalar(getValuatorState(4))*getCurrentFrameTime();
		
		/* Calculate a new surface frame: */
		NavTransform newSurfaceFrame=surfaceFrame;
		
		/* Translate the surface frame: */
		newSurfaceFrame*=NavTransform::translate(delta);
		
		/* Scale the surface frame: */
		newSurfaceFrame*=NavTrackerState::scale(Math::exp(scale));
		
		/* Re-align the surface frame with the surface: */
		realignSurfaceFrame(newSurfaceFrame);
		
		/* Schedule an update: */
		scheduleUpdate(getApplicationTime()+1.0/125.0);
		}
	}

void FiveAxisSurfaceNavigationTool::display(GLContextData& contextData) const
	{
	if(showCompass||(config.showScreenCenter&&isActive()))
		{
		/* Save and set up OpenGL state: */
		glPushAttrib(GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_LINE_BIT);
		glDisable(GL_LIGHTING);
		glDepthFunc(GL_LEQUAL);
		
		/* Determine the crosshair and compass colors: */
		Color bgColor=getBackgroundColor();
		Color fgColor;
		for(int i=0;i<3;++i)
			fgColor[i]=1.0f-bgColor[i];
		fgColor[3]=bgColor[3];
		
		/* Go to crosshair and compass space: */
		glPushMatrix();
		ONTransform trans=calcHUDTransform(getDisplayCenter());
		glMultMatrix(trans);
		
		if(config.showScreenCenter&&isActive())
			{
			glLineWidth(3.0f);
			glBegin(GL_LINES);
			glColor(bgColor);
			glVertex2d(-getDisplaySize(),0.0);
			glVertex2d(getDisplaySize(),0.0);
			glVertex2d(0.0,-getDisplaySize());
			glVertex2d(0.0,getDisplaySize());
			glEnd();
			
			glLineWidth(1.0f);
			glBegin(GL_LINES);
			glColor(fgColor);
			glVertex2d(-getDisplaySize(),0.0);
			glVertex2d(getDisplaySize(),0.0);
			glVertex2d(0.0,-getDisplaySize());
			glVertex2d(0.0,getDisplaySize());
			glEnd();
			}
		
		if(showCompass)
			{
			/* Get the data item: */
			DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
			
			/* Position the compass rose: */
			glTranslate(getDisplaySize()*Scalar(0.667),getDisplaySize()*Scalar(0.5),Scalar(0));
			glRotate(Math::deg(azimuth),0,0,1);
			
			/* Draw the compass rose's background: */
			glLineWidth(3.0f);
			glColor(bgColor);
			glCallList(dataItem->compassDisplayList);
			
			/* Draw the compass rose's foreground: */
			glLineWidth(1.0f);
			glColor(fgColor);
			glCallList(dataItem->compassDisplayList);
			}
		
		/* Go back to physical coordinates: */
		glPopMatrix();
		
		/* Restore OpenGL state: */
		glPopAttrib();
		}
	}

void FiveAxisSurfaceNavigationTool::initContext(GLContextData& contextData) const
	{
	/* Create a data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Create the compass rose display list: */
	glNewList(dataItem->compassDisplayList,GL_COMPILE);
	
	/* Draw the compass ring: */
	glBegin(GL_LINE_LOOP);
	for(int i=0;i<30;++i)
		{
		Scalar angle=Scalar(2)*Math::Constants<Scalar>::pi*(Scalar(i)+Scalar(0.5))/Scalar(30);
		glVertex(Math::sin(angle)*(config.compassSize+config.compassThickness),Math::cos(angle)*(config.compassSize+config.compassThickness));
		}
	for(int i=0;i<30;++i)
		{
		Scalar angle=Scalar(2)*Math::Constants<Scalar>::pi*(Scalar(i)+Scalar(0.5))/Scalar(30);
		glVertex(Math::sin(angle)*(config.compassSize-config.compassThickness),Math::cos(angle)*(config.compassSize-config.compassThickness));
		}
	glEnd();
	
	/* Draw the compass arrow: */
	glBegin(GL_LINE_LOOP);
	glVertex(config.compassThickness,config.compassSize*Scalar(-1.25));
	glVertex(config.compassThickness,config.compassSize*Scalar(1.25));
	glVertex(config.compassThickness*Scalar(2.5),config.compassSize*Scalar(1.25));
	glVertex(Scalar(0),config.compassSize*Scalar(1.75));
	glVertex(-config.compassThickness*Scalar(2.5),config.compassSize*Scalar(1.25));
	glVertex(-config.compassThickness,config.compassSize*Scalar(1.25));
	glVertex(-config.compassThickness,config.compassSize*Scalar(-1.25));
	glEnd();
	glBegin(GL_LINES);
	glVertex(-config.compassSize*Scalar(1.25),Scalar(0));
	glVertex(config.compassSize*Scalar(1.25),Scalar(0));
	glEnd();
	
	glEndList();
	}

}
