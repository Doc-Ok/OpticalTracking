/***********************************************************************
IMUTest - Simple visualization utility for 6-DOF IMU tracking.
Copyright (c) 2013 Oliver Kreylos

This file is part of the optical/inertial sensor fusion tracking
package.

The optical/inertial sensor fusion tracking package is free software;
you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

The optical/inertial sensor fusion tracking package is distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the optical/inertial sensor fusion tracking package; if not, write
to the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
Boston, MA 02111-1307 USA
***********************************************************************/

#include <iostream>
#include <iomanip>
#include <Misc/FunctionCalls.h>
#include <Threads/TripleBuffer.h>
#include <GL/gl.h>
#include <GL/GLMaterialTemplates.h>
#include <GL/GLModels.h>
#include <GL/GLGeometryWrappers.h>
#include <Vrui/Application.h>

#include "IMU.h"
#include "PSMove.h"
#include "OculusRift.h"
#include "IMUTracker.h"

/**********************
Main application class:
**********************/

class IMUTest:public Vrui::Application
	{
	/* Elements: */
	private:
	IMU* imu; // Pointer to the connected inertial measurement unit
	IMUTracker* tracker; // 6-DOF tracker attached to the connected IMU
	Threads::TripleBuffer<IMU::CalibratedSample> samples; // Triple buffer of incoming calibrated samples
	
	/* Private methods: */
	void sampleCallback(const IMU::CalibratedSample& sample); // Callback called when a new calibrated sample from the IMU arrives
	
	/* Constructors and destructors: */
	public:
	IMUTest(int& argc,char**& argv);
	virtual ~IMUTest(void);
	
	/* Methods from Vrui::Application: */
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	virtual void eventCallback(EventID eventId,Vrui::InputDevice::ButtonCallbackData* cbData);
	};

/************************
Methods of class IMUTest:
************************/

void IMUTest::sampleCallback(const IMU::CalibratedSample& sample)
	{
	/* Store the calibrated sample: */
	samples.postNewValue(sample);
	
	/* Forward the calibrated sample to the 6-DOF tracker: */
	tracker->integrateSample(sample);
	
	#if 0
	
	static IMU::Vector accel(0.0,0.0,0.0);
	static IMU::Vector mag(0.0,0.0,0.0);
	const double w=1.0/1024.0;
	accel=accel*(1.0-w)+sample.accelerometer*w;
	mag=mag*(1.0-w)+sample.magnetometer*w;
	
	std::cout<<std::fixed;
	std::cout.precision(4);
	std::cout<<'\r';
	std::cout<<std::setw(10)<<accel.mag()<<", ";
	std::cout<<std::setw(10)<<mag.mag();
	std::cout<<std::flush;
	
	#endif
	
	#if 0
	
	std::cout<<std::fixed;
	std::cout.precision(4);
	std::cout<<'\r';
	for(int i=0;i<3;++i)
		std::cout<<std::setw(10)<<sample.magnetometer[i];
	std::cout<<std::flush;
	
	#endif
	
	Vrui::requestUpdate();
	}

IMUTest::IMUTest(int& argc,char**& argv)
	:Vrui::Application(argc,argv),
	 imu(0),tracker(0)
	{
	/* Parse the command line: */
	IMUTracker::Scalar gravity(9.81);
	IMUTracker::Scalar driftCorrectionWeight(0.001);
	bool useMagnetometer=true;
	for(int i=1;i<argc;++i)
		{
		if(argv[i][0]=='-')
			{
			if(strcasecmp(argv[i]+1,"psmove")==0)
				{
				++i;
				if(i<argc)
					{
					if(imu==0)
						{
						/* Connect to the PS Move device of the given index: */
						PSMove* move=new PSMove(atoi(argv[i]));
						imu=move;
						}
					else
						std::cerr<<"Ignoring additional -psmove "<<argv[i]<<" argument"<<std::endl;
					}
				else
					std::cerr<<"Ignoring dangling -psmove argument"<<std::endl;
				}
			else if(strcasecmp(argv[i]+1,"rift")==0)
				{
				++i;
				if(i<argc)
					{
					if(imu==0)
						{
						/* Connect to the Oculus Rift device of the given index: */
						OculusRift* rift=new OculusRift(atoi(argv[i]));
						imu=rift;
						}
					else
						std::cerr<<"Ignoring additional -rift "<<argv[i]<<" argument"<<std::endl;
					}
				else
					std::cerr<<"Ignoring dangling -rift argument"<<std::endl;
				}
			else if(strcasecmp(argv[i]+1,"gravity")==0)
				{
				++i;
				if(i<argc)
					{
					/* Update local gravity: */
					gravity=IMUTracker::Scalar(atof(argv[i]));
					}
				else
					std::cerr<<"Ignoring dangling -gravity argument"<<std::endl;
				}
			else if(strcasecmp(argv[i]+1,"nomag")==0)
				{
				/* Disable magnetometer-based drift correction: */
				useMagnetometer=false;
				}
			else if(strcasecmp(argv[i]+1,"drift")==0)
				{
				++i;
				if(i<argc)
					{
					/* Update the drift correction weight: */
					driftCorrectionWeight=IMUTracker::Scalar(atof(argv[i]));
					}
				else
					std::cerr<<"Ignoring dangling -drift argument"<<std::endl;
				}
			}
		}
	
	std::cout<<"Connected to IMU device "<<imu->getSerialNumber()<<std::endl;
	
	/* Set up the 6-DOF tracker: */
	tracker=new IMUTracker(*imu);
	tracker->setGravity(gravity);
	tracker->setDriftCorrectionWeight(driftCorrectionWeight);
	tracker->setUseMagnetometer(useMagnetometer);
	
	/* Start streaming IMU measurements: */
	imu->startStreamingCalibrated(Misc::createFunctionCall(this,&IMUTest::sampleCallback));
	
	/* Add event tool classes to control the application: */
	addEventTool("Reset Tracker",0,0);
	
	Vrui::setNavigationTransformation(Vrui::Point(0,0,0),Vrui::Scalar(15),Vrui::Vector(0,1,0));
	}

IMUTest::~IMUTest(void)
	{
	/* Stop sampling and disconnect from the IMU: */
	imu->stopStreaming();
	delete tracker;
	delete imu;
	}

void IMUTest::frame(void)
	{
	/* Lock the most recent sample and tracker state: */
	tracker->lockNewState();
	samples.lockNewValue();
	}

void IMUTest::display(GLContextData& contextData) const
	{
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
	glMaterialSpecular(GLMaterialEnums::FRONT,GLColor<GLfloat,4>(1.0f,1.0f,1.0f));
	glMaterialShininess(GLMaterialEnums::FRONT,25.0f);
	
	/* Draw a global coordinate frame: */
	glPushMatrix();
	glColor3f(1.0f,0.5f,0.5f);
	glRotated(90.0,0.0,1.0,0.0);
	glTranslated(0.0,0.0,5.0);
	glDrawArrow(0.5f,1.0f,1.5f,10.0f,16);
	glPopMatrix();
	
	glPushMatrix();
	glColor3f(0.5f,1.0f,0.5f);
	glRotated(-90.0,1.0,0.0,0.0);
	glTranslated(0.0,0.0,5.0);
	glDrawArrow(0.5f,1.0f,1.5f,10.0f,16);
	glPopMatrix();
	
	glPushMatrix();
	glColor3f(0.5f,0.5f,1.0f);
	glTranslated(0.0,0.0,5.0);
	glDrawArrow(0.5f,1.0f,1.5f,10.0f,16);
	glPopMatrix();
	
	/* Draw a local coordinate frame: */
	glPushMatrix();
	const IMUTracker::State& state=tracker->getLockedState();
	// if(lockPosition)
	// 	glTranslated(5.0,5.0,5.0);
	// else
	// 	glTranslate(state.translation*IMUTracker::Scalar(10));
	glRotate(state.rotation);
	
	glPushMatrix();
	glColor3f(1.0f,0.5f,0.5f);
	glRotated(90.0,0.0,1.0,0.0);
	glTranslated(0.0,0.0,2.5);
	glDrawArrow(0.5f,1.0f,1.5f,5.0f,16);
	glPopMatrix();
	
	glPushMatrix();
	glColor3f(0.5f,1.0f,0.5f);
	glRotated(-90.0,1.0,0.0,0.0);
	glTranslated(0.0,0.0,2.5);
	glDrawArrow(0.5f,1.0f,1.5f,5.0f,16);
	glPopMatrix();
	
	glPushMatrix();
	glColor3f(0.5f,0.5f,1.0f);
	glTranslated(0.0,0.0,2.5);
	glDrawArrow(0.5f,1.0f,1.5f,5.0f,16);
	glPopMatrix();
	
	const IMU::CalibratedSample& sample=samples.getLockedValue();
	
	/* Draw the current linear acceleration vector: */
	glPushMatrix();
	glColor3f(1.0f,1.0f,0.0f);
	GLfloat len=GLfloat(Geometry::mag(sample.accelerometer));
	glRotate(IMUTracker::Rotation::rotateFromTo(IMUTracker::Vector(0,0,1),sample.accelerometer));
	glTranslatef(0.0f,0.0f,len*0.5f);
	glDrawArrow(0.5f,1.0f,1.5f,len,16);
	glPopMatrix();
	
	/* Draw the current magnetic flux density vector: */
	glPushMatrix();
	glColor3f(1.0f,0.0f,1.0f);
	len=GLfloat(Geometry::mag(sample.magnetometer))*0.2f;
	glRotate(IMUTracker::Rotation::rotateFromTo(IMUTracker::Vector(0,0,1),sample.magnetometer));
	// glTranslatef(0.0f,0.0f,len*0.5f);
	glDrawArrow(0.5f,1.0f,1.5f,len*2.0f,16);
	glPopMatrix();
	
	glPopMatrix();
	
	glPopAttrib();
	}

void IMUTest::eventCallback(EventID eventId,Vrui::InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState)
		{
		switch(eventId)
			{
			case 0:
				break;
			}
		}
	else
		{
		switch(eventId)
			{
			}
		}
	}

VRUI_APPLICATION_RUN(IMUTest)
