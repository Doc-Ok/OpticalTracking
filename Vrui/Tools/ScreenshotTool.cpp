/***********************************************************************
ScreenshotTool - Class for tools to save save screenshots from immersive
environments by overriding a selected window's screen and viewer with
virtual ones attached to an input device.
Copyright (c) 2008-2013 Oliver Kreylos

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

#include <Vrui/Tools/ScreenshotTool.h>

#include <Images/Config.h>

#include <Misc/CreateNumberedFileName.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Cluster/MulticastPipe.h>
#include <Math/Math.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Geometry/GeometryValueCoders.h>
#include <GL/gl.h>
#include <GL/GLGeometryWrappers.h>
#include <Vrui/Vrui.h>
#include <Vrui/InputDevice.h>
#include <Vrui/ToolManager.h>
#include <Vrui/VRScreen.h>
#include <Vrui/Viewer.h>
#include <Vrui/VRWindow.h>

namespace Vrui {

/**************************************
Methods of class ScreenshotToolFactory:
**************************************/

ScreenshotToolFactory::ScreenshotToolFactory(ToolManager& toolManager)
	:ToolFactory("ScreenshotTool",toolManager),
	 #if IMAGES_CONFIG_HAVE_PNG
	 screenshotFileName("ScreenshotTool.png"),
	 #else
	 screenshotFileName("ScreenshotTool.ppm"),
	 #endif
	 windowIndex(0),
	 screenSize(getDisplaySize()*Scalar(0.25)),
	 horizontal(1,0,0),vertical(0,1,0),
	 useMainViewer(true),
	 monoEyePosition(0,0,screenSize),
	 eyeOffset(screenSize*Scalar(0.1),0,0)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* toolFactory=toolManager.loadClass("UtilityTool");
	toolFactory->addChildClass(this);
	addParentClass(toolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	screenshotFileName=cfs.retrieveString("./screenshotFileName",screenshotFileName);
	windowIndex=cfs.retrieveValue<int>("./windowIndex",windowIndex);
	screenSize=cfs.retrieveValue<Scalar>("./screenSize",screenSize);
	horizontal=cfs.retrieveValue<Vector>("./horizontal",horizontal);
	horizontal.normalize();
	vertical=cfs.retrieveValue<Vector>("./vertical",vertical);
	vertical.normalize();
	useMainViewer=cfs.retrieveValue<bool>("./useMainViewer",useMainViewer);
	monoEyePosition=cfs.retrieveValue<Point>("./monoEyePosition",monoEyePosition);
	eyeOffset=cfs.retrieveValue<Vector>("./eyeOffset",eyeOffset);
	
	/* Set tool class' factory pointer: */
	ScreenshotTool::factory=this;
	}

ScreenshotToolFactory::~ScreenshotToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	ScreenshotTool::factory=0;
	}

const char* ScreenshotToolFactory::getName(void) const
	{
	return "Virtual Still Camera";
	}

const char* ScreenshotToolFactory::getButtonFunction(int) const
	{
	return "Take Picture";
	}

Tool* ScreenshotToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new ScreenshotTool(this,inputAssignment);
	}

void ScreenshotToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveScreenshotToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("UtilityTool");
	}

extern "C" ToolFactory* createScreenshotToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	ScreenshotToolFactory* screenshotToolFactory=new ScreenshotToolFactory(*toolManager);
	
	/* Return factory object: */
	return screenshotToolFactory;
	}

extern "C" void destroyScreenshotToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/***************************************
Static elements of class ScreenshotTool:
***************************************/

ScreenshotToolFactory* ScreenshotTool::factory=0;

/*******************************
Methods of class ScreenshotTool:
*******************************/

ScreenshotTool::ScreenshotTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:UtilityTool(factory,inputAssignment),
	 window(0),
	 originalScreen(0),originalViewer(0),
	 virtualScreen(0),virtualViewer(0),
	 eyePosition(Point::origin),
	 showFrameTime(0.0)
	{
	}

void ScreenshotTool::initialize(void)
	{
	if(isMaster())
		{
		/* Get the screenshot window: */
		window=getWindow(factory->windowIndex);
		
		/* Save the window's original screen, screen viewport, and viewer: */
		originalScreen=window->getVRScreen();
		for(int i=0;i<4;++i)
			originalViewport[i]=window->getScreenViewport()[i];
		originalViewer=window->getViewer();
		
		/* Get the window's size in pixels: */
		Scalar windowW=Scalar(window->getViewportSize(0));
		Scalar windowH=Scalar(window->getViewportSize(1));
		
		/* Calculate an appropriate virtual screen size maintaining the window's aspect ratio: */
		Scalar scale=factory->screenSize/Math::sqrt(Math::sqr(windowW)+Math::sqr(windowH));
		
		/* Create the virtual screen: */
		virtualScreen=new VRScreen;
		
		/* Attach the virtual screen to the input device: */
		virtualScreen->attachToDevice(getButtonDevice(0));
		
		/* Set the screen size: */
		screenW=windowW*scale;
		screenH=windowH*scale;
		virtualScreen->setSize(screenW,screenH);
		
		/* Calculate the virtual screen's center point: */
		screenCenter=Point::origin+factory->vertical*(screenH*Scalar(0.75));
		
		/* Set the screen transformation: */
		Vector vertical,horizontal;
		if(factory->useMainViewer)
			{
			/* Calculate the initial screen transformation by aligning the screen'z z direction with the viewing direction: */
			eyePosition=getButtonDeviceTransformation(0).inverseTransform(getMainViewer()->getHeadPosition());
			Vector viewDir=eyePosition-screenCenter;
			vertical=factory->vertical-viewDir*((factory->vertical*viewDir)/Geometry::sqr(viewDir));
			vertical.normalize();
			horizontal=vertical^viewDir;
			horizontal.normalize();
			}
		else
			{
			vertical=factory->vertical;
			horizontal=factory->horizontal;
			}
		ONTransform::Vector screenTranslation=screenCenter-horizontal*Math::div2(screenW)-vertical*Math::div2(screenH)-Point::origin;
		ONTransform::Rotation screenRotation=ONTransform::Rotation::fromBaseVectors(horizontal,vertical);
		ONTransform screenTransform(screenTranslation,screenRotation);
		virtualScreen->setTransform(screenTransform);
		
		/* Override the screenshot window's screen and screen viewport: */
		window->setVRScreen(virtualScreen);
		Scalar newViewport[4];
		newViewport[0]=Scalar(0);
		newViewport[1]=windowW*scale;
		newViewport[2]=Scalar(0);
		newViewport[3]=windowH*scale;
		window->setScreenViewport(newViewport);
		
		if(factory->useMainViewer)
			{
			/* Override the screenshot window's viewer with the main viewer: */
			window->setViewer(getMainViewer());
			}
		else
			{
			/* Create the virtual viewer: */
			virtualViewer=new Viewer;
			
			/* Attach the virtual viewer to the input device: */
			virtualViewer->attachToDevice(getButtonDevice(0));
			
			/* Calculate the viewer's eye positions in device coordinates: */
			eyePosition=factory->monoEyePosition;
			eyePosition[0]+=Math::div2(windowW*scale);
			eyePosition[1]+=Math::div2(windowH*scale);
			eyePosition=screenTransform.transform(eyePosition);
			Point screenCenter=screenTransform.transform(Point(Math::div2(windowW*scale),Math::div2(windowH*scale),0));
			Vector viewDirection=screenCenter-eyePosition;
			viewDirection.normalize();
			Vector eyeOffset=screenTransform.transform(factory->eyeOffset);
			virtualViewer->setEyes(viewDirection,eyePosition,eyeOffset);
			
			/* Override the screenshot window's viewer: */
			window->setViewer(virtualViewer);
			}
		
		/* Calculate the screen rectangle in device coordinates: */
		screenBox[0]=screenTransform.transform(Point(0,0,0));
		screenBox[1]=screenTransform.transform(Point(screenW,0,0));
		screenBox[2]=screenTransform.transform(Point(0,screenH,0));
		screenBox[3]=screenTransform.transform(Point(screenW,screenH,0));
		
		if(getMainPipe()!=0)
			{
			/* Send the screen center, screen rectangle, and eye position to the slave nodes: */
			getMainPipe()->write(screenCenter.getComponents(),3);
			for(int i=0;i<4;++i)
				getMainPipe()->write(screenBox[i].getComponents(),3);
			getMainPipe()->write(eyePosition.getComponents(),3);
			getMainPipe()->flush();
			}
		}
	else
		{
		/* Receive the eye position and screen rectangle from the master node: */
		getMainPipe()->read(screenCenter.getComponents(),3);
		for(int i=0;i<4;++i)
			getMainPipe()->read(screenBox[i].getComponents(),3);
		getMainPipe()->read(eyePosition.getComponents(),3);
		}
	}

void ScreenshotTool::deinitialize(void)
	{
	if(isMaster())
		{
		/* Restore the screenshot window's original screen, screen viewport, and viewer: */
		window->setVRScreen(originalScreen);
		window->setScreenViewport(originalViewport);
		window->setViewer(originalViewer);
		
		/* Destroy the virtual screen and viewer: */
		delete virtualScreen;
		if(!factory->useMainViewer)
			delete virtualViewer;
		}
	}

const ToolFactory* ScreenshotTool::getFactory(void) const
	{
	return factory;
	}

void ScreenshotTool::buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState) // Screenshot button has just been pressed
		{
		if(isMaster())
			{
			/* Create a unique file name based on the given screenshot file name: */
			char screenshotFileName[1024];
			Misc::createNumberedFileName(factory->screenshotFileName.c_str(),4,screenshotFileName);
			
			/* Request a screenshot from the screenshot window: */
			window->requestScreenshot(screenshotFileName);
			}
		
		/* Don't draw the screen frame for one second: */
		showFrameTime=getApplicationTime()+1.0;
		}
	}

void ScreenshotTool::frame(void)
	{
	/* Update the virtual viewer: */
	if(factory->useMainViewer)
		{
		/* Update the virtual screen's transformation to always align with the main viewer: */
		if(isMaster())
			{
			/* Calculate the current screen transformation by aligning the screen'z z direction with the viewing direction: */
			eyePosition=getButtonDeviceTransformation(0).inverseTransform(getMainViewer()->getHeadPosition());
			Vector viewDir=eyePosition-screenCenter;
			Vector vertical=factory->vertical-viewDir*((factory->vertical*viewDir)/Geometry::sqr(viewDir));
			vertical.normalize();
			Vector horizontal=vertical^viewDir;
			horizontal.normalize();
			ONTransform::Vector screenTranslation=screenCenter-horizontal*Math::div2(screenW)-vertical*Math::div2(screenH)-Point::origin;
			ONTransform::Rotation screenRotation=ONTransform::Rotation::fromBaseVectors(horizontal,vertical);
			ONTransform screenTransform(screenTranslation,screenRotation);
			virtualScreen->setTransform(screenTransform);
			
			/* Calculate the screen rectangle in device coordinates: */
			screenBox[0]=screenTransform.transform(Point(0,0,0));
			screenBox[1]=screenTransform.transform(Point(screenW,0,0));
			screenBox[2]=screenTransform.transform(Point(0,screenH,0));
			screenBox[3]=screenTransform.transform(Point(screenW,screenH,0));
			
			if(getMainPipe()!=0)
				{
				/* Send the screen center, screen rectangle, and eye position to the slave nodes: */
				getMainPipe()->write(screenCenter.getComponents(),3);
				for(int i=0;i<4;++i)
					getMainPipe()->write(screenBox[i].getComponents(),3);
				getMainPipe()->write(eyePosition.getComponents(),3);
				getMainPipe()->flush();
				}
			}
		else
			{
			/* Receive the eye position and screen rectangle from the master node: */
			getMainPipe()->read(screenCenter.getComponents(),3);
			for(int i=0;i<4;++i)
				getMainPipe()->read(screenBox[i].getComponents(),3);
			getMainPipe()->read(eyePosition.getComponents(),3);
			}
		}
	else if(isMaster())
		virtualViewer->update();
	}

void ScreenshotTool::display(GLContextData& contextData) const
	{
	if(getApplicationTime()>=showFrameTime)
		{
		/* Get the input device's current transformation: */
		const Vrui::TrackerState& transformation=getButtonDeviceTransformation(0);
		
		glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
		glDisable(GL_LIGHTING);
		glLineWidth(3.0f);
		
		/* Draw the virtual screen's frame: */
		glBegin(GL_LINE_LOOP);
		glColor3f(1.0f,0.0f,0.0f);
		glVertex(transformation.transform(screenBox[0]));
		glVertex(transformation.transform(screenBox[1]));
		glVertex(transformation.transform(screenBox[3]));
		glVertex(transformation.transform(screenBox[2]));
		glEnd();
		
		if(!factory->useMainViewer)
			{
			/* Draw the virtual screen frustum: */
			glBegin(GL_LINES);
			for(int i=0;i<4;++i)
				{
				glVertex(transformation.transform(eyePosition));
				glVertex(transformation.transform(eyePosition+(screenBox[i]-eyePosition)*Scalar(100)));
				}
			glEnd();
			}
		glPopAttrib();
		}
	}

}
