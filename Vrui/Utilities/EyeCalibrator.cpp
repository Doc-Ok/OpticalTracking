/***********************************************************************
EyeCalibrator - Utility to calculate the precise eye positions of a
viewer in the head tracker's local coordinate frame, to generate
personalized Vrui configurations.
Copyright (c) 2013 Oliver Kreylos

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

#include <string.h>
#include <stdlib.h>
#include <vector>
#include <Misc/ThrowStdErr.h>
#include <Misc/HashTable.h>
#include <Math/Math.h>
#include <Math/Matrix.h>
#include <Geometry/Point.h>
#include <Geometry/OrthonormalTransformation.h>
#include <GL/gl.h>
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

class EyeCalibrator:public Vrui::Application
	{
	/* Embedded classes: */
	private:
	typedef double Scalar;
	typedef Geometry::Point<Scalar,3> Point;
	typedef Geometry::OrthonormalTransformation<Scalar,3> ONTransform;
	
	struct TiePoint
		{
		/* Elements: */
		public:
		Point screenPoint; // Point on screen in viewer-local coordinates
		Point devicePoint; // Input device position in viewer-local coordinates
		};
	
	/* Elements: */
	static const Scalar pointAngles[6]; // Angles from center point at which to show the tie points
	Vrui::VRScreen* screen; // Screen used to show the big eye target, and the screen points
	std::vector<TiePoint> tiePoints; // List of collected calibration tie points
	Point centerPoint; // Calibration pattern's center point in screen coordinates
	Scalar radius; // Calibration pattern's radius
	Misc::HashTable<Vrui::InputDevice*,int> deviceTools; // Hash map from input devices to the number of event tools bound to them
	int nextPoint; // Index of next tie point to be collected
	
	/* Constructors and destructors: */
	public:
	EyeCalibrator(int& argc,char**& argv);
	virtual ~EyeCalibrator(void);
	
	/* Methods from Vrui::Application: */
	virtual void display(GLContextData& contextData) const;
	virtual void eventToolCreationCallback(EventID eventId,Vrui::ToolManager::ToolCreationCallbackData* cbData);
	virtual void eventToolDestructionCallback(EventID eventId,Vrui::ToolManager::ToolDestructionCallbackData* cbData);
	virtual void eventCallback(EventID eventId,Vrui::InputDevice::ButtonCallbackData* cbData);
	};

/**************************************
Static elements of class EyeCalibrator:
**************************************/

const EyeCalibrator::Scalar EyeCalibrator::pointAngles[6]=
	{
	0.0,180.0,60.0,240.0,120.0,300.0
	};

/******************************
Methods of class EyeCalibrator:
******************************/

EyeCalibrator::EyeCalibrator(int& argc,char**& argv)
	:Vrui::Application(argc,argv),
	 screen(0),radius(0),
	 deviceTools(17),
	 nextPoint(0)
	{
	/* Parse the command line: */
	const char* screenName=0;
	for(int i=1;i<argc;++i)
		if(strcasecmp(argv[i],"-radius")==0)
			{
			++i;
			radius=Scalar(atof(argv[i]));
			}
		else if(screenName==0)
			screenName=argv[i];
	
	/* Get the central screen: */
	if(screenName==0)
		screen=Vrui::getMainScreen();
	else
		screen=Vrui::findScreen(screenName);
	if(screen==0)
		Misc::throwStdErr("EyeCalibrator: Screen %s not found",screenName);
	
	/* Initialize the calibration pattern: */
	if(radius==Scalar(0))
		radius=Math::div2(Math::min(screen->getWidth(),screen->getHeight()))*Scalar(0.75);
	centerPoint=Point(Math::div2(screen->getWidth()),Math::div2(screen->getHeight()),Scalar(0));
	
	/* Set the background color to white: */
	Vrui::setBackgroundColor(Vrui::Color(1.0f,1.0f,1.0f));
	
	/* Create the tie point capture tool class: */
	addEventTool("Capture Tie Point",0,0);
	}

EyeCalibrator::~EyeCalibrator(void)
	{
	}

void EyeCalibrator::display(GLContextData& contextData) const
	{
	glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
	glDisable(GL_LIGHTING);
	glLineWidth(1.0f);
	
	glPushMatrix();
	glLoadMatrix(Vrui::getDisplayState(contextData).modelviewPhysical);
	glMultMatrix(screen->getScreenTransformation());
	
	/* Draw the central eye pattern: */
	Scalar radiush=Math::div2(radius);
	glColor3f(0.0f,0.0f,0.0f);
	glBegin(GL_LINES);
	glVertex(centerPoint[0]-radiush,centerPoint[1]);
	glVertex(centerPoint[0]+radiush,centerPoint[1]);
	glVertex(centerPoint[0],centerPoint[1]-radiush);
	glVertex(centerPoint[0],centerPoint[1]+radiush);
	glEnd();
	
	/* Draw the current calibration target point: */
	Scalar x=centerPoint[0]+Math::cos(Math::rad(pointAngles[nextPoint]))*radius;
	Scalar y=centerPoint[1]+Math::sin(Math::rad(pointAngles[nextPoint]))*radius;
	Scalar radiusq=Math::div2(radiush);
	glBegin(GL_LINES);
	glVertex(x-radiusq,y);
	glVertex(x+radiusq,y);
	glVertex(x,y-radiusq);
	glVertex(x,y+radiusq);
	glEnd();
	
	glPopMatrix();
	
	glPopAttrib();
	}

void EyeCalibrator::eventToolCreationCallback(Vrui::Application::EventID eventId,Vrui::ToolManager::ToolCreationCallbackData* cbData)
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

void EyeCalibrator::eventToolDestructionCallback(Vrui::Application::EventID eventId,Vrui::ToolManager::ToolDestructionCallbackData* cbData)
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

void EyeCalibrator::eventCallback(Vrui::Application::EventID eventId,Vrui::InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState)
		{
		/* Create a tie point: */
		Vrui::TrackerState ht=Vrui::getMainViewer()->getHeadTransformation();
		TiePoint newTiePoint;
		Scalar x=centerPoint[0]+Math::cos(Math::rad(pointAngles[nextPoint]))*radius;
		Scalar y=centerPoint[1]+Math::sin(Math::rad(pointAngles[nextPoint]))*radius;
		newTiePoint.screenPoint=ht.inverseTransform(screen->getScreenTransformation().transform(Point(x,y,Scalar(0))));
		newTiePoint.devicePoint=ht.inverseTransform(cbData->inputDevice->getPosition());
		
		/* Store the tie point: */
		tiePoints.push_back(newTiePoint);
		
		/* Go to the next tie point: */
		++nextPoint;
		if(nextPoint==6)
			{
			/* Create a least-squares system to calculate the position of the current eye: */
			Math::Matrix ata(3,3,0.0);
			Math::Matrix atb(3,1,0.0);
			for(int tp=0;tp<6;++tp)
				{
				/* Enter the tie points into the least-squares system: */
				Point::Vector dir=tiePoints[tp].screenPoint-tiePoints[tp].devicePoint;
				Point::Vector n1=Geometry::normalize(Geometry::normal(dir));
				Point::Vector n2=Geometry::normalize(n1^dir);
				for(int i=0;i<3;++i)
					{
					for(int j=0;j<3;++j)
						ata(i,j)+=n1[i]*n1[j]+n2[i]*n2[j];
					atb(i)+=n1[i]*(n1*tiePoints[tp].devicePoint)+n2[i]*(n2*tiePoints[tp].devicePoint);
					}
				}
			
			/* Solve the least-squares system: */
			Math::Matrix x=atb.divideFullPivot(ata);
			std::cout<<"Eye position in viewer-local coordinates: ("<<x(0)<<", "<<x(1)<<", "<<x(2)<<")"<<std::endl;
			
			/* Start over: */
			tiePoints.clear();
			nextPoint=0;
			}
		}
	}

VRUI_APPLICATION_RUN(EyeCalibrator)
