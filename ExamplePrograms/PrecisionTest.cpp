/***********************************************************************
PrecisionTest - Simple utility to measure the 3D picking accuracy of a
user in a holographic display.
Copyright (c) 2013 Oliver Kreylos

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <iostream>
#include <Misc/HashTable.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Box.h>
#include <Geometry/Random.h>
#include <Geometry/OutputOperators.h>
#include <GL/gl.h>
#include <GL/GLMaterial.h>
#include <GL/GLModels.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <Vrui/Vrui.h>
#include <Vrui/InputDevice.h>
#include <Vrui/GlyphRenderer.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/VRScreen.h>
#include <Vrui/Viewer.h>
#include <Vrui/DisplayState.h>
#include <Vrui/Application.h>

class PrecisionTest:public Vrui::Application
	{
	/* Embedded classes: */
	private:
	typedef double Scalar;
	typedef Geometry::Point<Scalar,3> Point;
	typedef Geometry::Box<Scalar,3> Box;
	
	/* Elements: */
	double traverseLength; // Length of the traversal period between points in seconds
	Misc::HashTable<Vrui::InputDevice*,int> deviceTools; // Hash map from input devices to the number of event tools bound to them
	Point lastPoint; // Last measurement position in physical space
	Point point; // Current measurement position in physical space
	double traverseStart,traverseEnd; // End time points of the traversal animation
	
	/* Private methods: */
	Point createPoint(void); // Creates a random point inside the "sampleable" area
	
	/* Constructors and destructors: */
	public:
	PrecisionTest(int& argc,char**& argv);
	virtual ~PrecisionTest(void);
	
	/* Methods from Vrui::Application: */
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	virtual void eventToolCreationCallback(EventID eventId,Vrui::ToolManager::ToolCreationCallbackData* cbData);
	virtual void eventToolDestructionCallback(EventID eventId,Vrui::ToolManager::ToolDestructionCallbackData* cbData);
	virtual void eventCallback(EventID eventId,Vrui::InputDevice::ButtonCallbackData* cbData);
	};

/******************************
Methods of class PrecisionTest:
******************************/

PrecisionTest::Point PrecisionTest::createPoint(void)
	{
	/* Create the bounding box of all screens and the main viewer position: */
	Box sampleBox=Box::empty;
	for(int screenIndex=0;screenIndex<Vrui::getNumScreens();++screenIndex)
		{
		/* Get the four corner points of the screen in physical coordinates: */
		const Vrui::VRScreen* screen=Vrui::getScreen(screenIndex);
		const Vrui::ONTransform& st=screen->getScreenTransformation();
		sampleBox.addPoint(st.transform(Vrui::Point(0,0,0)));
		sampleBox.addPoint(st.transform(Vrui::Point(screen->getWidth(),0,0)));
		sampleBox.addPoint(st.transform(Vrui::Point(screen->getWidth(),screen->getHeight(),0)));
		sampleBox.addPoint(st.transform(Vrui::Point(0,screen->getHeight(),0)));
		}
	sampleBox.addPoint(Vrui::getMainViewer()->getHeadPosition());
	
	/* Create random points inside the sample box until one is inside any screen-viewer pyramid: */
	Point result;
	bool inside;
	do
		{
		/* Create a candidate sample point: */
		result=Geometry::randPointUniformCC<Scalar,3>(sampleBox.min,sampleBox.max);
		
		/* Check the candidate point against all screen-viewer pyramids: */
		Point viewer=Vrui::getMainViewer()->getHeadPosition();
		inside=false;
		for(int screenIndex=0;screenIndex<Vrui::getNumScreens()&&!inside;++screenIndex)
			{
			/* Get the four corner points of the screen in physical coordinates: */
			const Vrui::VRScreen* screen=Vrui::getScreen(screenIndex);
			const Vrui::ONTransform& st=screen->getScreenTransformation();
			Point sp[4];
			sp[0]=st.transform(Vrui::Point(0,0,0));
			sp[1]=st.transform(Vrui::Point(screen->getWidth(),0,0));
			sp[2]=st.transform(Vrui::Point(screen->getWidth(),screen->getHeight(),0));
			sp[3]=st.transform(Vrui::Point(0,screen->getHeight(),0));
			
			/* Check the sample point against the four sides of the screen-viewer pyramid: */
			inside=true;
			for(int i=0;i<4&&inside;++i)
				{
				Point::Vector normal=(sp[(i+1)%3]-sp[i])^(sp[i]-viewer);
				inside=(result-viewer)*normal>=Scalar(0);
				}
			if(inside)
				{
				/* Check the point against the screen surface: */
				inside=st.inverseTransform(result)[2]>=Vrui::getInchFactor(); // Point must be at least 1" away from screen
				}
			}
		}
	while(!inside);
	
	return result;
	}

PrecisionTest::PrecisionTest(int& argc,char**& argv)
	:Vrui::Application(argc,argv),
	 deviceTools(17),
	 traverseLength(1)
	{
	/* Parse the command line: */
	const char* screenName=0;
	for(int i=1;i<argc;++i)
		if(strcasecmp(argv[i],"-traversalTime")==0)
			{
			++i;
			traverseLength=Scalar(atof(argv[i]));
			}
	
	/* Create the first sample position: */
	lastPoint=Vrui::getDisplayCenter();
	point=createPoint();
	traverseStart=Vrui::getApplicationTime();
	traverseEnd=traverseStart+traverseLength*3.0;
	
	/* Create the tie point capture tool class: */
	addEventTool("Capture Tie Point",0,0);
	}

PrecisionTest::~PrecisionTest(void)
	{
	}

void PrecisionTest::frame(void)
	{
	if(Vrui::getApplicationTime()<traverseEnd)
		Vrui::scheduleUpdate(Vrui::getApplicationTime()+1.0/125.0);
	}

void PrecisionTest::display(GLContextData& contextData) const
	{
	glPushAttrib(GL_ENABLE_BIT);
	
	/* Calculate the target marker's current position: */
	Point markerPos;
	GLMaterial::Color markerColor;
	if(Vrui::getApplicationTime()>=traverseEnd)
		{
		markerPos=point;
		markerColor=GLMaterial::Color(0.6f,0.8f,0.6f);
		}
	else
		{
		/* Move linearly using "smootherstep" blending function: */
		double t=(Vrui::getApplicationTime()-traverseStart)/(traverseEnd-traverseStart);
		markerPos=Geometry::affineCombination(lastPoint,point,t*t*t*(t*(t*6.0-15.0)+10.0));
		markerColor=GLMaterial::Color(0.8f,0.6f,0.6f);
		}
	
	/* Draw the marker: */
	glPushMatrix();
	
	/* Go to physical space: */
	glLoadMatrix(Vrui::getDisplayState(contextData).modelviewPhysical);
	
	/* Go to marker's position: */
	glTranslate(markerPos-Point::origin);
	
	/* Draw the marker: */
	glMaterial(GLMaterialEnums::FRONT,GLMaterial(markerColor,GLMaterial::Color(0.3f,0.3f,0.3f),25.0f));
	glDrawCylinder(Vrui::getUiSize()*Vrui::Scalar(0.25),Vrui::getUiSize()*Vrui::Scalar(4.0),12);
	glRotate(90.0,0.0,1.0,0.0);
	glDrawCylinder(Vrui::getUiSize()*Vrui::Scalar(0.25),Vrui::getUiSize()*Vrui::Scalar(4.0),12);
	glRotate(90.0,0.0,0.0,1.0);
	
	glPopMatrix();
	
	glPopAttrib();
	}

void PrecisionTest::eventToolCreationCallback(Vrui::Application::EventID eventId,Vrui::ToolManager::ToolCreationCallbackData* cbData)
	{
	/* Check if the tool's input device already has one of our event tools on it: */
	Vrui::InputDevice* device=cbData->tool->getInputAssignment().getButtonSlot(0).device;
	Misc::HashTable<Vrui::InputDevice*,int>::Iterator dtIt=deviceTools.findEntry(device);
	if(dtIt.isFinished())
		{
		/* Check if the input device's device glyph is currently enabled: */
		Vrui::Glyph& glyph=Vrui::getInputGraphManager()->getInputDeviceGlyph(device);
		if(glyph.isEnabled())
			{
			/* Disable the input device's device glyph: */
			glyph.disable();

			/* Remember that we did: */
			deviceTools[device]=1;
			}
		}
	else
		{
		/* Increment the number of event tools bound to this input device: */
		++(dtIt->getDest());
		}
	}

void PrecisionTest::eventToolDestructionCallback(Vrui::Application::EventID eventId,Vrui::ToolManager::ToolDestructionCallbackData* cbData)
	{
	/* Check if we earlier disabled the tool's glyph: */
	Vrui::InputDevice* device=cbData->tool->getInputAssignment().getButtonSlot(0).device;
	Misc::HashTable<Vrui::InputDevice*,int>::Iterator dtIt=deviceTools.findEntry(device);
	if(!dtIt.isFinished())
		{
		/* Decrement the number of event tools bound to this input device: */
		--(dtIt->getDest());
		
		/* Check if this was the last one: */
		if(dtIt->getDest()==0)
			{
			/* Enable the device's glyph again: */
			Vrui::getInputGraphManager()->getInputDeviceGlyph(device).enable();
			
			/* Remove the hash table entry: */
			deviceTools.removeEntry(dtIt);
			}
		}
	}

void PrecisionTest::eventCallback(Vrui::Application::EventID eventId,Vrui::InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState)
		{
		/* Check if the marker is at its final position: */
		if(Vrui::getApplicationTime()>=traverseEnd)
			{
			/* Record the current measurement point: */
			
			/* Go to the next target position: */
			lastPoint=point;
			point=createPoint();
			traverseStart=Vrui::getApplicationTime();
			traverseEnd=traverseStart+traverseLength;
			}
		}
	}

VRUI_APPLICATION_RUN(PrecisionTest)
