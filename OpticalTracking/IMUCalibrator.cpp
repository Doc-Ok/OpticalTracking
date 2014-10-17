/***********************************************************************
IMUCalibrator - Simple utility to calibrate an IMU's accelerometer and
magnetometer readings, and visualize 3-DOF and dead-reckoning 6-DOF
tracking results.
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
#include <Misc/SizedTypes.h>
#include <Misc/FunctionCalls.h>
#include <Threads/Mutex.h>
#include <Threads/TripleBuffer.h>
#include <IO/File.h>
#include <GL/gl.h>
#include <GL/GLMaterialTemplates.h>
#include <GL/GLModels.h>
#include <GL/GLGeometryWrappers.h>
#include <Vrui/Application.h>
#include <Vrui/OpenFile.h>

#include "IMU.h"
#include "EllipsoidFitter.h"
#include "PSMove.h"
#include "OculusRift.h"

/**********************
Main application class:
**********************/

class IMUCalibrator:public Vrui::Application
	{
	/* Embedded classes: */
	private:
	struct RawSample // Structure containing raw IMU measurements
		{
		/* Elements: */
		public:
		EllipsoidFitter::Point accel;
		EllipsoidFitter::Point mag;
		};
	
	/* Elements: */
	private:
	IMU* imu; // Pointer to the connected inertial measurement unit
	IMU::Scalar gravity; // Local gravity magnitude in m/s^2
	IMU::Scalar magneticFluxDensity; // Local magnetic flux density in uT
	EllipsoidFitter accelFitter; // Object to fit ellipsoids to raw accelerometer measurements
	EllipsoidFitter magFitter; // Object to fit ellipsoids to raw magnetometer measurements
	Threads::TripleBuffer<RawSample> currentSample; // Triple buffer of raw measurements received by the sample callback
	bool showAxes; // Flag to draw coordinate axes
	bool showRawSamples; // Flag to render saved ellipsoid fitter points
	volatile bool recordSamples; // Flag to add incoming raw measurements to their respective ellipsoid fitters
	Threads::Mutex gyroMutex; // Mutex protecting the gyroscope sampler
	bool sampleGyros; // Flag to enable gyroscope sampling
	int gyroSum[3]; // Accumulator to calculate average no-motion values for the three gyroscopes
	int gyroNumSamples; // Counter for gyro no-motion value calculation
	
	/* Private methods: */
	void sampleCallback(const IMU::RawSample& sample); // Callback called when a new raw sample from the IMU arrives
	
	/* Constructors and destructors: */
	public:
	IMUCalibrator(int& argc,char**& argv);
	virtual ~IMUCalibrator(void);
	
	/* Methods from Vrui::Application: */
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	virtual void eventCallback(EventID eventId,Vrui::InputDevice::ButtonCallbackData* cbData);
	};

/******************************
Methods of class IMUCalibrator:
******************************/

void IMUCalibrator::sampleCallback(const IMU::RawSample& sample)
	{
	/* Create raw acceleration and magnetometer measurements: */
	RawSample& rs=currentSample.startNewValue();
	for(int i=0;i<3;++i)
		{
		rs.accel[i]=EllipsoidFitter::Scalar(sample.accelerometer[i]);
		rs.mag[i]=EllipsoidFitter::Scalar(sample.magnetometer[i]);
		}
	
	if(recordSamples)
		{
		/* Add the new samples to their respective fitters: */
		accelFitter.addPoint(rs.accel);
		magFitter.addPoint(rs.mag);
		}
	
	{
	Threads::Mutex::Lock gyroLock(gyroMutex);
	if(sampleGyros)
		{
		/* Add the new gyroscope sample to the sum: */
		for(int i=0;i<3;++i)
			gyroSum[i]+=sample.gyroscope[i];
		++gyroNumSamples;
		}
	}
	
	/* Push the new sample to the main thread: */
	currentSample.postNewValue();
	Vrui::requestUpdate();
	}

IMUCalibrator::IMUCalibrator(int& argc,char**& argv)
	:Vrui::Application(argc,argv),
	 imu(0),gravity(9.81),magneticFluxDensity(50),
	 showAxes(true),showRawSamples(true),recordSamples(false),
	 sampleGyros(false)
	{
	/* Parse the command line: */
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
					gravity=IMU::Scalar(atof(argv[i]));
					}
				else
					std::cerr<<"Ignoring dangling -gravity argument"<<std::endl;
				}
			else if(strcasecmp(argv[i]+1,"magneticFluxDensity")==0)
				{
				++i;
				if(i<argc)
					{
					/* Update local magnetic flux density: */
					magneticFluxDensity=IMU::Scalar(atof(argv[i]));
					}
				else
					std::cerr<<"Ignoring dangling -magneticFluxDensity argument"<<std::endl;
				}
			}
		}
	
	std::cout<<"Connected to IMU device "<<imu->getSerialNumber()<<std::endl;
	
	/* Start streaming IMU measurements: */
	imu->startStreamingRaw(Misc::createFunctionCall(this,&IMUCalibrator::sampleCallback));
	
	/* Add event tool classes to control sampling: */
	addEventTool("Show Axes",0,0);
	addEventTool("Show Raw Samples",0,1);
	addEventTool("Start/Stop Recording",0,2);
	addEventTool("Run Calibration",0,3);
	addEventTool("Center Gyroscopes",0,4);
	
	Vrui::setNavigationTransformation(Vrui::Point(0,0,0),Vrui::Scalar(15),Vrui::Vector(0,1,0));
	}

IMUCalibrator::~IMUCalibrator(void)
	{
	/* Stop sampling and disconnect from the IMU: */
	imu->stopStreaming();
	delete imu;
	}

void IMUCalibrator::frame(void)
	{
	/* Lock the most recent sample: */
	currentSample.lockNewValue();
	}

void IMUCalibrator::display(GLContextData& contextData) const
	{
	glPushAttrib(GL_ENABLE_BIT|GL_POINT_BIT);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
	glMaterialSpecular(GLMaterialEnums::FRONT,GLColor<GLfloat,4>(1.0f,1.0f,1.0f));
	glMaterialShininess(GLMaterialEnums::FRONT,25.0f);
	
	if(showAxes)
		{
		/* Draw a global coordinate frame: */
		glPushMatrix();
		glColor3f(1.0f,0.5f,0.5f);
		glRotated(90.0,0.0,1.0,0.0);
		glTranslated(0.0,0.0,2.5);
		glDrawArrow(0.25f,0.5f,0.75f,5.0f,16);
		glPopMatrix();
		
		glPushMatrix();
		glColor3f(0.5f,1.0f,0.5f);
		glRotated(-90.0,1.0,0.0,0.0);
		glTranslated(0.0,0.0,2.5);
		glDrawArrow(0.25f,0.5f,0.75f,5.0f,16);
		glPopMatrix();
		
		glPushMatrix();
		glColor3f(0.5f,0.5f,1.0f);
		glTranslated(0.0,0.0,2.5);
		glDrawArrow(0.25f,0.5f,0.75f,5.0f,16);
		glPopMatrix();
		}
	
	#if 0
		
		/* Draw a local coordinate frame: */
		glPushMatrix();
		if(lockPosition)
			glTranslated(5.0,5.0,5.0);
		else
			glTranslate((orientations.getLockedValue().position-Tracker::Point::origin)*Tracker::Scalar(10));
		glRotate(orientations.getLockedValue().orientation);
		
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
		
		#if 0
		glPopMatrix();
		glPushMatrix();
		glRotate(orientations.getLockedValue().orientation);
		#endif
		
		#if 0
		glPopMatrix();
		#endif
		
		/* Draw the current linear acceleration vector: */
		glPushMatrix();
		glColor3f(1.0f,1.0f,0.0f);
		const Tracker::Vector& accel=orientations.getLockedValue().acceleration;
		GLfloat len=GLfloat(Geometry::mag(accel));
		glRotate(Tracker::Rotation::rotateFromTo(Tracker::Vector(0,0,1),accel));
		glTranslatef(0.0f,0.0f,len*0.5f);
		glDrawArrow(0.5f,1.0f,1.5f,len,16);
		glPopMatrix();
		
		/* Draw the current magnetic flux density vector: */
		glPushMatrix();
		glColor3f(1.0f,0.0f,1.0f);
		const Tracker::Vector& mag=orientations.getLockedValue().magneticFlux;
		len=GLfloat(Geometry::mag(mag))*20.0f;
		glRotate(Tracker::Rotation::rotateFromTo(Tracker::Vector(0,0,1),mag));
		// glTranslatef(0.0f,0.0f,len*0.5f);
		glDrawArrow(0.5f,1.0f,1.5f,len*2.0f,16);
		glPopMatrix();
		
		#if 1
		glPopMatrix();
		#endif
		}
	
	#endif
	
	/* Draw the current accelerometer and magnetometer samples: */
	glDisable(GL_LIGHTING);
	const IMU::CalibrationData& cd=imu->getCalibrationData();
	
	glPushMatrix();
	glScaled(cd.accelerometerFactor,cd.accelerometerFactor,cd.accelerometerFactor);
	
	glPointSize(5.0f);
	glBegin(GL_POINTS);
	glColor3f(1.0f,0.0f,0.0f);
	glVertex(currentSample.getLockedValue().accel);
	glEnd();
	
	if(showRawSamples)
		{
		/* Draw all samples stored in the accelerometer fitter: */
		glPointSize(3.0f);
		accelFitter.glRenderAction(contextData);
		}
	
	glPopMatrix();
	
	glPushMatrix();
	glScaled(cd.magnetometerFactor,cd.magnetometerFactor,cd.magnetometerFactor);
	
	glPointSize(5.0f);
	glBegin(GL_POINTS);
	glColor3f(0.0f,1.0f,0.0f);
	glVertex(currentSample.getLockedValue().mag);
	glEnd();
	
	if(showRawSamples)
		{
		/* Draw all samples stored in the magnetometer fitter: */
		glPointSize(3.0f);
		magFitter.glRenderAction(contextData);
		}
	
	glPopMatrix();
	
	glPopAttrib();
	}

void IMUCalibrator::eventCallback(EventID eventId,Vrui::InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState)
		{
		switch(eventId)
			{
			case 0:
				showAxes=!showAxes;
				break;
			
			case 1:
				showRawSamples=!showRawSamples;
				break;
			
			case 2:
				/* Start recording samples: */
				recordSamples=true;
				break;
			
			case 3:
				{
				/* Run the calibration: */
				std::cout<<"Accelerometer calibration:"<<std::endl;
				EllipsoidFitter::Calibration accelCalib=accelFitter.calculateFit();
				std::cout<<"Accelerometer average magnitude: "<<accelCalib.second<<std::endl;
				
				std::cout<<std::endl<<"Magnetometer calibration:"<<std::endl;
				EllipsoidFitter::Calibration magCalib=magFitter.calculateFit();
				std::cout<<"Magnetometer average magnitude: "<<magCalib.second<<std::endl;
				
				/* Write the calibration matrices to a calibration file: */
				std::string calibFileName="Calibration-";
				calibFileName.append(imu->getSerialNumber());
				IO::FilePtr calibFile=Vrui::openFile(calibFileName.c_str(),IO::File::WriteOnly);
				calibFile->setEndianness(Misc::LittleEndian);
				
				/* Write the accelerometer calibration matrix: */
				for(int i=0;i<3;++i)
					for(int j=0;j<4;++j)
						calibFile->write<Misc::Float64>(accelCalib.first(i,j)*gravity/accelCalib.second);
				
				/* Write a nominal gyroscope calibration matrix: */
				double gyroFactor=imu->getCalibrationData().gyroscopeFactor;
				{
				Threads::Mutex::Lock gyroLock(gyroMutex);
				for(int i=0;i<3;++i)
					{
					for(int j=0;j<3;++j)
						calibFile->write<Misc::Float64>(i==j?gyroFactor:0.0);
					if(gyroNumSamples>0)
						calibFile->write<Misc::Float64>(double(gyroSum[i])/double(gyroNumSamples)*gyroFactor);
					else
						calibFile->write<Misc::Float64>(0);
					}
				}
				
				/* Write the magnetometer calibration matrix: */
				for(int i=0;i<3;++i)
					for(int j=0;j<4;++j)
						calibFile->write<Misc::Float64>(magCalib.first(i,j)*magneticFluxDensity/magCalib.second);
				
				break;
				}
			
			case 4:
				{
				Threads::Mutex::Lock gyroLock(gyroMutex);
				
				sampleGyros=!sampleGyros;
				
				if(sampleGyros)
					{
					/* Prepare for gyroscope sampling: */
					for(int i=0;i<3;++i)
						gyroSum[i]=0;
					gyroNumSamples=0;
					}
				else
					{
					/* Print the results of gyroscope sampling: */
					std::cout<<"Gyroscope center value based on "<<gyroNumSamples<<" samples:";
					for(int i=0;i<3;++i)
						std::cout<<' '<<std::setw(8)<<double(gyroSum[i])/double(gyroNumSamples);
					std::cout<<std::endl;
					}
				
				break;
				}
			}
		}
	else
		{
		switch(eventId)
			{
			case 2:
				/* Stop recording samples: */
				recordSamples=false;
				break;
			}
		}
	}

VRUI_APPLICATION_RUN(IMUCalibrator)
