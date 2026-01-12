/***********************************************************************
IMUTest - Simple visualization utility for 6-DOF IMU tracking.
Copyright (c) 2013-2026 Oliver Kreylos

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
#include <Realtime/Time.h>
#include <Threads/Spinlock.h>
#include <Math/Constants.h>
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
	
	// DEBUGGING
	Realtime::TimePointMonotonic firstSample; // Time at which the first sample arrived
	size_t numSamples; // Number of received samples
	Realtime::TimePointMonotonic lastSample; // Time at which the last sample arrived
	
	unsigned int sampleHistorySize; // Size of sample history buffer
	IMU::CalibratedSample* sampleHistory; // Array of recently-received calibrated IMU samples
	Threads::Spinlock sampleHistoryMutex; // Mutex protecting the most recent IMU sample
	unsigned int mostRecentSample; // Index of most recent IMU sample in history buffer
	bool trackPosition; // Flag whether positional tracking is enabled
	
	/* Private methods: */
	void sampleCallback(const IMU::CalibratedSample& sample); // Callback called when a new calibrated sample from the IMU arrives
	
	/* Constructors and destructors: */
	public:
	IMUTest(int& argc,char**& argv);
	virtual ~IMUTest(void);
	
	/* Methods from Vrui::Application: */
	virtual void resetNavigation();
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	virtual void eventCallback(EventID eventId,Vrui::InputDevice::ButtonCallbackData* cbData);
	};

/************************
Methods of class IMUTest:
************************/

void IMUTest::sampleCallback(const IMU::CalibratedSample& sample)
	{
	// DEBUGGING
	if(numSamples==0)
		firstSample.set();
	++numSamples;
	lastSample.set();
	
	// std::cout<<sample.timeStamp<<std::endl;
	
	/* Store the calibrated sample in the history buffer: */
	{
	Threads::Spinlock::Lock sampleHistoryLock(sampleHistoryMutex);
	unsigned int nextSample=mostRecentSample+1;
	if(nextSample==sampleHistorySize)
		nextSample=0;
	sampleHistory[nextSample]=sample;
	sampleHistory[nextSample].gyroscope-=tracker->getGyroscopeBias();
	mostRecentSample=nextSample;
	}
	
	/* Forward the calibrated sample to the 6-DOF tracker: */
	tracker->integrateSample(sample);
	
	Vrui::requestUpdate();
	}

IMUTest::IMUTest(int& argc,char**& argv)
	:Vrui::Application(argc,argv),
	 imu(0),tracker(0),
	 // DEBUGGING
	 numSamples(0),
	 sampleHistorySize(1000),sampleHistory(new IMU::CalibratedSample[sampleHistorySize]),mostRecentSample(0),
	 trackPosition(false)
	{
	/* Parse the command line: */
	IMUTracker::Scalar gravity(9.81);
	IMUTracker::Scalar zeta(0.001*Math::sqrt(0.75));
	IMUTracker::Scalar beta(0.5*Math::sqrt(0.75));
	bool useMagnetometer=true;
	bool optical=false;
	unsigned char psMoveLedColor[3]={0,0,0};
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
			else if(strcasecmp(argv[i]+1,"ledColor")==0)
				{
				if(i+3<argc)
					{
					for(int j=0;j<3;++j)
						psMoveLedColor[j]=(unsigned char)(atoi(argv[i+1+j]));
					i+=3;
					}
				else
					std::cerr<<"Ignoring dangling -ledColor argument"<<std::endl;
				}
			else if(strcasecmp(argv[i]+1,"optical")==0)
				{
				/* Set up an Oculus Rift DK2 for optical tracking: */
				optical=true;
				}
			else if(strcasecmp(argv[i]+1,"drift")==0)
				{
				++i;
				if(i+1<argc)
					{
					/* Update the drift correction weights: */
					zeta=IMUTracker::Scalar(atof(argv[i]));
					++i;
					beta=IMUTracker::Scalar(atof(argv[i]));
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
	tracker->setBiasDriftGain(zeta);
	tracker->setOrientationDriftGain(beta);
	tracker->setUseMagnetometer(useMagnetometer);
	
	/* If the IMU is a Playstation Move controller, set its LED color: */
	PSMove* psMove=dynamic_cast<PSMove*>(imu);
	if(psMove!=0)
		psMove->setLedColor(psMoveLedColor[0],psMoveLedColor[1],psMoveLedColor[2]);
	
	/* If the IMU is an Oculus Rift, enable optical tracking if requested: */
	OculusRift* rift=dynamic_cast<OculusRift*>(imu);
	if(rift!=0&&optical)
		rift->startOpticalTracking();
	
	/* Initialize the sample history buffer: */
	for(unsigned int i=0;i<sampleHistorySize;++i)
		{
		IMU::CalibratedSample& s=sampleHistory[i];
		s.accelerometer=IMU::Vector::zero;
		s.gyroscope=IMU::Vector::zero;
		s.magnetometer=IMU::Vector::zero;
		}
	
	/* Start streaming IMU measurements: */
	imu->startStreamingCalibrated(Misc::createFunctionCall(this,&IMUTest::sampleCallback));
	
	/* Add event tool classes to control the application: */
	addEventTool("Reset Tracker",0,0);
	addEventTool("Zero Velocities",0,1);
	addEventTool("Toggle Position Tracking",0,2);
	}

IMUTest::~IMUTest(void)
	{
	/* If the IMU is a Playstation Move controller, reset its LED color: */
	PSMove* psMove=dynamic_cast<PSMove*>(imu);
	if(psMove!=0)
		psMove->setLedColor(0,0,0);
	
	/* Stop sampling and disconnect from the IMU: */
	imu->stopStreaming();
	delete tracker;
	delete imu;
	
	delete[] sampleHistory;
	
	// DEBUGGING
	std::cout<<"Average sample interval: "<<double(lastSample-firstSample)*1000.0/double(numSamples-1)<<std::endl;
	}

void IMUTest::resetNavigation(void)
	{
	Vrui::setNavigationTransformation(Vrui::Point(0,0,0),Vrui::Scalar(15),Vrui::Vector(0,1,0));
	}

void IMUTest::frame(void)
	{
	/* Lock the most recent tracker state: */
	tracker->lockNewState();
	}

void IMUTest::display(GLContextData& contextData) const
	{
	glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
	glMaterialSpecular(GLMaterialEnums::FRONT,GLColor<GLfloat,4>(1.0f,1.0f,1.0f));
	glMaterialShininess(GLMaterialEnums::FRONT,25.0f);
	
	#if 1
	
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
	if(trackPosition)
		glTranslate(state.translation*IMUTracker::Scalar(100));
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
	
	const IMU::CalibratedSample& sample=sampleHistory[mostRecentSample];
	
	#if 0
	
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
	
	#endif
	
	glPopMatrix();
	
	#endif
	
	/* Draw the recent sample history: */
	glDisable(GL_LIGHTING);
	glLineWidth(1.0f);
	
	glPushMatrix();
	
	glTranslated(-50.0,20.0,0.0);
	
	/* Draw accelerometer history: */
	glBegin(GL_LINES);
	glColor3f(0.5f,0.5f,0.5f);
	glVertex2d(-5.0,0.0);
	glVertex2d(double(sampleHistorySize-1)*0.1+5.0,0.0);
	glVertex2d(0.0,-9.81*0.5);
	glVertex2d(double(sampleHistorySize-1)*0.1,-9.81*0.5);
	glVertex2d(0.0,9.81*0.5);
	glVertex2d(double(sampleHistorySize-1)*0.1,9.81*0.5);
	glVertex2d(0.0,-9.81*0.5);
	glVertex2d(0.0,9.81*0.5);
	glEnd();
	
	for(int axis=0;axis<3;++axis)
		{
		glBegin(GL_LINE_STRIP);
		glColor3f(axis==0?1.0f:0.0f,axis==1?1.0f:0.0f,axis==2?1.0f:0.0f);
		for(unsigned int i=0;i<sampleHistorySize;++i)
			glVertex2d(double(i)*0.1,sampleHistory[i].accelerometer[axis]*0.5);
		glEnd();
		}
	
	glTranslated(0.0,20.0,0.0);
	
	/* Draw gyroscope history: */
	glBegin(GL_LINES);
	glColor3f(0.5f,0.5f,0.5f);
	glVertex2d(-5.0,0.0);
	glVertex2d(double(sampleHistorySize-1)*0.1+5.0,0.0);
	glVertex2d(0.0,-2.0*Math::Constants<double>::pi);
	glVertex2d(double(sampleHistorySize-1)*0.1,-2.0*Math::Constants<double>::pi);
	glVertex2d(0.0,2.0*Math::Constants<double>::pi);
	glVertex2d(double(sampleHistorySize-1)*0.1,2.0*Math::Constants<double>::pi);
	glVertex2d(0.0,-2.0*Math::Constants<double>::pi);
	glVertex2d(0.0,2.0*Math::Constants<double>::pi);
	glEnd();
	for(int axis=0;axis<3;++axis)
		{
		glBegin(GL_LINE_STRIP);
		glColor3f(axis==0?1.0f:0.0f,axis==1?1.0f:0.0f,axis==2?1.0f:0.0f);
		for(unsigned int i=0;i<sampleHistorySize;++i)
			glVertex2d(double(i)*0.1,sampleHistory[i].gyroscope[axis]);
		glEnd();
		}
	
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
				// tracker->restart(IMUTracker::Vector::zero);
				tracker->restart(IMUTracker::Vector::zero,IMUTracker::Rotation::identity);
				break;
			
			case 1:
				tracker->restart();
				break;
			
			case 2:
				trackPosition=!trackPosition;
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
