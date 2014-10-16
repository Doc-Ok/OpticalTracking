/***********************************************************************
MouseSurfaceNavigationTool - Class for navigation tools that use the
mouse to move along an application-defined surface.
Copyright (c) 2009-2013 Oliver Kreylos

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

#include <Vrui/Tools/MouseSurfaceNavigationTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/GeometryValueCoders.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLContextData.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <GLMotif/Event.h>
#include <GLMotif/TitleBar.h>
#include <GLMotif/WidgetManager.h>
#include <Vrui/Vrui.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/Internal/InputDeviceAdapterMouse.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/**************************************************
Methods of class MouseSurfaceNavigationToolFactory:
**************************************************/

MouseSurfaceNavigationToolFactory::MouseSurfaceNavigationToolFactory(ToolManager& toolManager)
	:ToolFactory("MouseSurfaceNavigationTool",toolManager),
	 rotateFactor(getDisplaySize()/Scalar(4)),
	 screenScalingDirection(0,-1,0),
	 scaleFactor(getDisplaySize()/Scalar(4)),
	 wheelScaleFactor(Scalar(0.5)),
	 throwThreshold(getUiSize()*Scalar(2)),
	 probeSize(getUiSize()),
	 maxClimb(getDisplaySize()),
	 fixAzimuth(false),
	 showCompass(true),compassSize(getUiSize()*Scalar(5)),compassThickness(getUiSize()*Scalar(0.5)),
	 showScreenCenter(true),
	 interactWithWidgets(true)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(2);
	layout.setNumValuators(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* navigationToolFactory=toolManager.loadClass("SurfaceNavigationTool");
	navigationToolFactory->addChildClass(this);
	addParentClass(navigationToolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	rotateFactor=cfs.retrieveValue<Scalar>("./rotateFactor",rotateFactor);
	screenScalingDirection=cfs.retrieveValue<Vector>("./screenScalingDirection",screenScalingDirection);
	scaleFactor=cfs.retrieveValue<Scalar>("./scaleFactor",scaleFactor);
	wheelScaleFactor=cfs.retrieveValue<Scalar>("./wheelScaleFactor",wheelScaleFactor);
	throwThreshold=cfs.retrieveValue<Scalar>("./throwThreshold",throwThreshold);
	probeSize=cfs.retrieveValue<Scalar>("./probeSize",probeSize);
	maxClimb=cfs.retrieveValue<Scalar>("./maxClimb",maxClimb);
	fixAzimuth=cfs.retrieveValue<bool>("./fixAzimuth",fixAzimuth);
	showCompass=cfs.retrieveValue<bool>("./showCompass",showCompass);
	compassSize=cfs.retrieveValue<Scalar>("./compassSize",compassSize);
	compassThickness=cfs.retrieveValue<Scalar>("./compassThickness",compassThickness);
	showScreenCenter=cfs.retrieveValue<bool>("./showScreenCenter",showScreenCenter);
	interactWithWidgets=cfs.retrieveValue<bool>("./interactWithWidgets",interactWithWidgets);
	
	/* Set tool class' factory pointer: */
	MouseSurfaceNavigationTool::factory=this;
	}

MouseSurfaceNavigationToolFactory::~MouseSurfaceNavigationToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	MouseSurfaceNavigationTool::factory=0;
	}

const char* MouseSurfaceNavigationToolFactory::getName(void) const
	{
	return "Mouse (Multiple Buttons)";
	}

const char* MouseSurfaceNavigationToolFactory::getButtonFunction(int buttonSlotIndex) const
	{
	switch(buttonSlotIndex)
		{
		case 0:
			return "Rotate";
		
		case 1:
			return "Pan";
		}
	
	/* Never reached; just to make compiler happy: */
	return 0;
	}

const char* MouseSurfaceNavigationToolFactory::getValuatorFunction(int) const
	{
	return "Quick Zoom";
	}

Tool* MouseSurfaceNavigationToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new MouseSurfaceNavigationTool(this,inputAssignment);
	}

void MouseSurfaceNavigationToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveMouseSurfaceNavigationToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("SurfaceNavigationTool");
	}

extern "C" ToolFactory* createMouseSurfaceNavigationToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	MouseSurfaceNavigationToolFactory* mouseSurfaceNavigationToolFactory=new MouseSurfaceNavigationToolFactory(*toolManager);
	
	/* Return factory object: */
	return mouseSurfaceNavigationToolFactory;
	}

extern "C" void destroyMouseSurfaceNavigationToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/*****************************************************
Methods of class MouseSurfaceNavigationTool::DataItem:
*****************************************************/

MouseSurfaceNavigationTool::DataItem::DataItem(void)
	:compassDisplayList(glGenLists(1))
	{
	}

MouseSurfaceNavigationTool::DataItem::~DataItem(void)
	{
	glDeleteLists(compassDisplayList,1);
	}

/***************************************************
Static elements of class MouseSurfaceNavigationTool:
***************************************************/

MouseSurfaceNavigationToolFactory* MouseSurfaceNavigationTool::factory=0;

/*******************************************
Methods of class MouseSurfaceNavigationTool:
*******************************************/

Point MouseSurfaceNavigationTool::calcScreenPos(void) const
	{
	/* Calculate the ray equation: */
	Ray ray=getButtonDeviceRay(0);
	
	/* Get the transformation of the screen currently containing the input device: */
	Scalar viewport[4];
	ONTransform screenT=getMouseScreenTransform(mouseAdapter,viewport);
	
	/* Intersect the device ray with the screen: */
	Vector normal=screenT.getDirection(2);
	Scalar d=normal*screenT.getOrigin();
	Scalar divisor=normal*ray.getDirection();
	if(divisor==Scalar(0))
		return Point::origin;
	
	Scalar lambda=(d-ray.getOrigin()*normal)/divisor;
	if(lambda<Scalar(0))
		return Point::origin;
	
	return ray(lambda);
	}

void MouseSurfaceNavigationTool::applyNavState(void) const
	{
	/* Compose and apply the navigation transformation: */
	NavTransform nav=physicalFrame;
	nav*=NavTransform::rotate(Rotation::rotateX(elevation));
	nav*=NavTransform::rotate(Rotation::rotateZ(azimuth));
	nav*=Geometry::invert(surfaceFrame);
	setNavigationTransformation(nav);
	}

void MouseSurfaceNavigationTool::initNavState(void)
	{
	/* Set up a physical navigation frame around the display's center: */
	calcPhysicalFrame(getDisplayCenter());
	
	/* Calculate the initial environment-aligned surface frame in navigation coordinates: */
	surfaceFrame=getInverseNavigationTransformation()*physicalFrame;
	NavTransform newSurfaceFrame=surfaceFrame;
	
	/* Align the initial frame with the application's surface and calculate Euler angles: */
	AlignmentData ad(surfaceFrame,newSurfaceFrame,factory->probeSize,factory->maxClimb);
	Scalar roll;
	align(ad,azimuth,elevation,roll);
	
	/* Limit elevation angle to down direction: */
	if(elevation<Scalar(0))
		elevation=Scalar(0);
	
	if(factory->showCompass)
		{
		/* Start showing the virtual compass: */
		showCompass=true;
		}
	
	/* Apply the newly aligned surface frame: */
	surfaceFrame=newSurfaceFrame;
	applyNavState();
	}

void MouseSurfaceNavigationTool::realignSurfaceFrame(NavTransform& newSurfaceFrame)
	{
	/* Re-align the surface frame with the surface: */
	Rotation initialOrientation=newSurfaceFrame.getRotation();
	AlignmentData ad(surfaceFrame,newSurfaceFrame,factory->probeSize,factory->maxClimb);
	align(ad);
	
	if(!factory->fixAzimuth)
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

void MouseSurfaceNavigationTool::navigationTransformationChangedCallback(Misc::CallbackData* cbData)
	{
	/* Stop showing the virtual compass if this tool is no longer active: */
	if(!SurfaceNavigationTool::isActive())
		showCompass=false;
	}

MouseSurfaceNavigationTool::MouseSurfaceNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:SurfaceNavigationTool(factory,inputAssignment),
	 GUIInteractor(false,Scalar(0),getButtonDevice(0)),
	 mouseAdapter(0),
	 currentPos(Point::origin),currentValue(0),
	 navigationMode(IDLE),
	 showCompass(false)
	{
	/* Find the mouse input device adapter controlling the input device: */
	InputDevice* rootDevice=getInputGraphManager()->getRootDevice(getButtonDevice(0));
	mouseAdapter=dynamic_cast<InputDeviceAdapterMouse*>(getInputDeviceManager()->findInputDeviceAdapter(rootDevice));
	
	/* Register a callback when the navigation transformation changes: */
	getNavigationTransformationChangedCallbacks().add(this,&MouseSurfaceNavigationTool::navigationTransformationChangedCallback);
	}

MouseSurfaceNavigationTool::~MouseSurfaceNavigationTool(void)
	{
	/* Remove then navigation transformation change callback: */
	getNavigationTransformationChangedCallbacks().remove(this,&MouseSurfaceNavigationTool::navigationTransformationChangedCallback);
	}

const ToolFactory* MouseSurfaceNavigationTool::getFactory(void) const
	{
	return factory;
	}

void MouseSurfaceNavigationTool::buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData)
	{
	/* Process based on which button was pressed: */
	switch(buttonSlotIndex)
		{
		case 0:
			if(cbData->newButtonState) // Button has just been pressed
				{
				/* Act depending on this tool's current state: */
				switch(navigationMode)
					{
					case IDLE:
					case THROWING:
						if(factory->interactWithWidgets)
							{
							/* Check if the GUI interactor accepts the event: */
							GUIInteractor::updateRay();
							if(GUIInteractor::buttonDown(false))
								{
								/* Deactivate this tool if it is throwing: */
								if(navigationMode==THROWING)
									deactivate();
								
								/* Go to widget interaction mode: */
								navigationMode=WIDGETING;
								}
							else
								{
								/* Try activating this tool: */
								if(navigationMode==THROWING||activate())
									{
									initNavState();
									currentPos=calcScreenPos();
									navigationMode=ROTATING;
									}
								}
							}
						else
							{
							/* Try activating this tool: */
							if(navigationMode==THROWING||activate())
								{
								initNavState();
								currentPos=calcScreenPos();
								navigationMode=ROTATING;
								}
							}
						break;
					
					case PANNING:
						currentPos=calcScreenPos();
						navigationMode=SCALING;
						break;
					
					default:
						/* This shouldn't happen; just ignore the event */
						break;
					}
				}
			else // Button has just been released
				{
				/* Act depending on this tool's current state: */
				switch(navigationMode)
					{
					case WIDGETING:
						{
						if(GUIInteractor::isActive())
							{
							/* Deliver the event: */
							GUIInteractor::buttonUp();
							}
						
						/* Deactivate this tool: */
						navigationMode=IDLE;
						break;
						}
					
					case ROTATING:
						/* Deactivate this tool: */
						deactivate();
						
						/* Go to idle mode: */
						navigationMode=IDLE;
						break;
					
					case SCALING:
						currentPos=calcScreenPos();
						navigationMode=PANNING;
						break;
					
					default:
						/* This shouldn't happen; just ignore the event */
						break;
					}
				}
			break;
		
		case 1:
			if(cbData->newButtonState) // Button has just been pressed
				{
				/* Act depending on this tool's current state: */
				switch(navigationMode)
					{
					case IDLE:
					case THROWING:
						/* Try activating this tool: */
						if(navigationMode==THROWING||activate())
							{
							initNavState();
							currentPos=calcScreenPos();
							navigationMode=PANNING;
							}
						break;
					
					case ROTATING:
						currentPos=calcScreenPos();
						navigationMode=SCALING;
						break;
					
					default:
						/* This shouldn't happen; just ignore the event */
						break;
					}
				}
			else // Button has just been released
				{
				/* Act depending on this tool's current state: */
				switch(navigationMode)
					{
					case PANNING:
						{
						/* Check if the input device is still moving: */
						Point newCurrentPos=calcScreenPos();
						Vector delta=newCurrentPos-currentPos;
						if(Geometry::mag(delta)>factory->throwThreshold)
							{
							/* Calculate throwing velocity: */
							throwVelocity=delta/(getApplicationTime()-lastMoveTime);
							
							/* Go to throwing mode: */
							navigationMode=THROWING;
							}
						else
							{
							/* Deactivate this tool: */
							deactivate();
							
							/* Go to idle mode: */
							navigationMode=IDLE;
							}
						break;
						}
					
					case SCALING:
						currentPos=calcScreenPos();
						navigationMode=ROTATING;
						break;
					
					default:
						/* This shouldn't happen; just ignore the event */
						break;
					}
				}
			break;
		}
	}

void MouseSurfaceNavigationTool::valuatorCallback(int,InputDevice::ValuatorCallbackData* cbData)
	{
	currentValue=Scalar(cbData->newValuatorValue);
	if(currentValue!=Scalar(0))
		{
		/* Act depending on this tool's current state: */
		switch(navigationMode)
			{
			case IDLE:
			case THROWING:
				/* Try activating this tool: */
				if(navigationMode==THROWING||activate())
					{
					/* Go to wheel scaling mode: */
					initNavState();
					navigationMode=SCALING_WHEEL;
					}
				break;
			
			default:
				/* This can definitely happen; just ignore the event */
				break;
			}
		}
	else
		{
		/* Act depending on this tool's current state: */
		switch(navigationMode)
			{
			case SCALING_WHEEL:
				/* Deactivate this tool: */
				deactivate();
				
				/* Go to idle mode: */
				navigationMode=IDLE;
				break;
			
			default:
				/* This can definitely happen; just ignore the event */
				break;
			}
		}
	}

void MouseSurfaceNavigationTool::frame(void)
	{
	/* Calculate the new mouse position: */
	Point newCurrentPos=calcScreenPos();
	if(factory->interactWithWidgets)
		{
		/* Update the GUI interactor: */
		GUIInteractor::updateRay();
		GUIInteractor::move();
		}
	
	/* Act depending on this tool's current state: */
	switch(navigationMode)
		{
		case ROTATING:
			{
			/* Calculate the rotation vector: */
			Vector delta=newCurrentPos-currentPos;
			
			/* Adjust the azimuth angle: */
			azimuth=wrapAngle(azimuth+delta[0]/factory->rotateFactor);
			
			/* Adjust the elevation angle: */
			elevation-=delta[2]/factory->rotateFactor;
			if(elevation<Scalar(0))
				elevation=Scalar(0);
			else if(elevation>Math::rad(Scalar(90)))
				elevation=Math::rad(Scalar(90));
			
			/* Apply the new transformation: */
			applyNavState();
			break;
			}
		
		case PANNING:
			{
			NavTransform newSurfaceFrame=surfaceFrame;
			
			/* Calculate the translation vector: */
			Vector delta=newCurrentPos-currentPos;
			delta=Rotation::rotateX(Math::rad(Scalar(-90))).transform(delta);
			delta=Rotation::rotateZ(-azimuth).transform(delta);
			
			/* Translate the surface frame: */
			newSurfaceFrame*=NavTransform::translate(-delta);
			
			/* Re-align the surface frame with the surface: */
			realignSurfaceFrame(newSurfaceFrame);
			
			break;
			}
		
		case THROWING:
			{
			NavTransform newSurfaceFrame=surfaceFrame;
			
			/* Calculate the throw translation vector: */
			Vector delta=throwVelocity*getFrameTime();
			delta=Rotation::rotateX(Math::rad(Scalar(-90))).transform(delta);
			delta=Rotation::rotateZ(-azimuth).transform(delta);
			
			/* Translate the surface frame: */
			newSurfaceFrame*=NavTransform::translate(-delta);
			
			/* Re-align the surface frame with the surface: */
			realignSurfaceFrame(newSurfaceFrame);
			
			scheduleUpdate(getApplicationTime()+1.0/125.0);
			
			break;
			}
		
		case SCALING:
			{
			NavTransform newSurfaceFrame=surfaceFrame;
			
			/* Calculate the current scaling direction: */
			Scalar viewport[4];
			ONTransform screenT=getMouseScreenTransform(mouseAdapter,viewport);
			Vector scalingDirection=screenT.transform(factory->screenScalingDirection);
			
			/* Scale the surface frame: */
			Scalar scale=((newCurrentPos-currentPos)*scalingDirection)/factory->scaleFactor;
			newSurfaceFrame*=NavTrackerState::scale(Math::exp(-scale));
			
			/* Re-align the surface frame with the surface: */
			realignSurfaceFrame(newSurfaceFrame);
			
			break;
			}
		
		case SCALING_WHEEL:
			{
			NavTransform newSurfaceFrame=surfaceFrame;
			
			/* Scale the surface frame: */
			newSurfaceFrame*=NavTrackerState::scale(Math::pow(factory->wheelScaleFactor,-currentValue));
			
			/* Re-align the surface frame with the surface: */
			realignSurfaceFrame(newSurfaceFrame);
			
			break;
			}
		
		default:
			;
		}
	
	/* Update the current mouse position: */
	if(currentPos!=newCurrentPos)
		{
		currentPos=calcScreenPos();
		lastMoveTime=getApplicationTime();
		}
	}

void MouseSurfaceNavigationTool::display(GLContextData& contextData) const
	{
	#if 0 // Don't draw the interaction ray -- it's a mouse tool, it should be invisible
	if(factory->interactWithWidgets)
		{
		/* Draw the GUI interactor's state: */
		GUIInteractor::glRenderAction(3.0f,GLColor<GLfloat,4>(1.0f,0.0f,0.0f),contextData);
		}
	#endif
	
	if(showCompass||(factory->showScreenCenter&&navigationMode!=IDLE&&navigationMode!=WIDGETING))
		{
		/* Save and set up OpenGL state: */
		glPushAttrib(GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_LINE_BIT);
		glDisable(GL_LIGHTING);
		glDepthFunc(GL_LEQUAL);
		
		/* Go to screen coordinates: */
		glPushMatrix();
		Scalar viewport[4];
		glMultMatrix(getMouseScreenTransform(mouseAdapter,viewport));
		
		/* Determine the crosshair colors: */
		Color bgColor=getBackgroundColor();
		Color fgColor;
		for(int i=0;i<3;++i)
			fgColor[i]=1.0f-bgColor[i];
		fgColor[3]=bgColor[3];
		
		if(factory->showScreenCenter&&navigationMode!=IDLE&&navigationMode!=WIDGETING)
			{
			/* Calculate the window's or screen's center: */
			Scalar centerPos[2];
			for(int i=0;i<2;++i)
				centerPos[i]=Math::mid(viewport[2*i+0],viewport[2*i+1]);
			
			/* Calculate the endpoints of the screen's crosshair lines in screen coordinates: */
			Point l=Point(viewport[0],centerPos[1],Scalar(0));
			Point r=Point(viewport[1],centerPos[1],Scalar(0));
			Point b=Point(centerPos[0],viewport[2],Scalar(0));
			Point t=Point(centerPos[0],viewport[3],Scalar(0));
			
			/* Draw the screen crosshairs: */
			glLineWidth(3.0f);
			glColor(bgColor);
			glBegin(GL_LINES);
			glVertex(l);
			glVertex(r);
			glVertex(b);
			glVertex(t);
			glEnd();
			glLineWidth(1.0f);
			glColor(fgColor);
			glBegin(GL_LINES);
			glVertex(l);
			glVertex(r);
			glVertex(b);
			glVertex(t);
			glEnd();
			}
		
		if(showCompass)
			{
			/* Get the data item: */
			DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
			
			/* Position the compass rose: */
			glTranslate(viewport[1]-factory->compassSize*Scalar(3),viewport[3]-factory->compassSize*Scalar(3),Scalar(0));
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

void MouseSurfaceNavigationTool::initContext(GLContextData& contextData) const
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
		glVertex(Math::sin(angle)*(factory->compassSize+factory->compassThickness),Math::cos(angle)*(factory->compassSize+factory->compassThickness));
		}
	for(int i=0;i<30;++i)
		{
		Scalar angle=Scalar(2)*Math::Constants<Scalar>::pi*(Scalar(i)+Scalar(0.5))/Scalar(30);
		glVertex(Math::sin(angle)*(factory->compassSize-factory->compassThickness),Math::cos(angle)*(factory->compassSize-factory->compassThickness));
		}
	glEnd();
	
	/* Draw the compass arrow: */
	glBegin(GL_LINE_LOOP);
	glVertex(factory->compassThickness,factory->compassSize*Scalar(-1.25));
	glVertex(factory->compassThickness,factory->compassSize*Scalar(1.25));
	glVertex(factory->compassThickness*Scalar(2.5),factory->compassSize*Scalar(1.25));
	glVertex(Scalar(0),factory->compassSize*Scalar(1.75));
	glVertex(-factory->compassThickness*Scalar(2.5),factory->compassSize*Scalar(1.25));
	glVertex(-factory->compassThickness,factory->compassSize*Scalar(1.25));
	glVertex(-factory->compassThickness,factory->compassSize*Scalar(-1.25));
	glEnd();
	glBegin(GL_LINES);
	glVertex(-factory->compassSize*Scalar(1.25),Scalar(0));
	glVertex(factory->compassSize*Scalar(1.25),Scalar(0));
	glEnd();
	
	glEndList();
	}

}
