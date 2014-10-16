/***********************************************************************
SampleTrackerField - A Vrui application to sample the global 3D field of
a 6-DOF tracker, specifically an electromagnetic one like the Polhemus
Fastrak or the Razer Hydra, to visualize and hopefully correct large-
scale distortions due to presence of magnetic objects in the
environment.
Copyright (c) 2014 Oliver Kreylos

This file is part of the Vrui calibration utility package.

The Vrui calibration utility package is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Vrui calibration utility package is distributed in the hope that it
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Vrui calibration utility package; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#include <utility>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <Misc/Array.h>
#include <Math/Math.h>
#include <Geometry/Point.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Geometry/OutputOperators.h>
#include <GL/gl.h>
#include <GL/GLModels.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <GLMotif/FileSelectionDialog.h>
#include <Vrui/Vrui.h>
#include <Vrui/Application.h>
#include <Vrui/InputDevice.h>
#include <Vrui/Tool.h>
#include <Vrui/GenericToolFactory.h>
#include <Vrui/ToolManager.h>
#include <Vrui/DisplayState.h>
#include <Vrui/OpenFile.h>

class SampleTrackerField:public Vrui::Application
	{
	/* Embedded classes: */
	private:
	typedef Geometry::ComponentArray<Vrui::Scalar,3> Size;
	typedef Misc::Array<std::pair<bool,Vrui::ONTransform>,3> SampleArray;
	typedef SampleArray::Index Index;
	
	class SampleTool;
	typedef Vrui::GenericToolFactory<SampleTool> SampleToolFactory;
	
	class SampleTool:public Vrui::Tool,public Vrui::Application::Tool<SampleTrackerField>
		{
		friend class Vrui::GenericToolFactory<SampleTool>;
		
		/* Elements: */
		private:
		static SampleToolFactory* factory; // Pointer to the factory object for this class
		
		/* Constructors and destructors: */
		public:
		static void initClass(void);
		SampleTool(const Vrui::ToolFactory* factory,const Vrui::ToolInputAssignment& inputAssignment);
		
		/* Methods: */
		virtual const Vrui::ToolFactory* getFactory(void) const;
		virtual void buttonCallback(int buttonSlotIndex,Vrui::InputDevice::ButtonCallbackData* cbData);
		virtual void display(GLContextData& contextData) const;
		};
	
	friend class SampleTool;
	
	/* Elements: */
	private:
	Index gridSize; // Number of grid points to sample
	Vrui::Point gridBase; // Position of sample grid's base corner
	Size cellSize; // Size of one grid cell
	SampleArray samples; // Array holding sampled grid positions
	Index nextSample; // Index of next grid position to sample
	Vrui::InputDevice* groundTruthDevice; // The input device whose position and orientation to treat as "ground truth"
	
	/* Private methods: */
	void centerView(void); // Centers the view on the next sample position
	void drawCross(const Vrui::ONTransform& t,GLfloat radius,GLfloat length) const;
	void saveGridOKCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData);
	
	/* Constructors and destructors: */
	public:
	SampleTrackerField(int& argc,char**& argv);
	
	/* Methods from Vrui::Application: */
	virtual void display(GLContextData& contextData) const;
	
	/* New methods: */
	void sample(bool valid); // Takes a valid or invalid sample and advances the sampling pointer
	};

/*******************************************************
Static elements of class SampleTrackerField::SampleTool:
*******************************************************/

SampleTrackerField::SampleToolFactory* SampleTrackerField::SampleTool::factory=0;

/***********************************************
Methods of class SampleTrackerField::SampleTool:
***********************************************/

void SampleTrackerField::SampleTool::initClass(void)
	{
	/* Create a factory object for the sample tool class: */
	SampleToolFactory* toolFactory=new SampleToolFactory("SampleTool","Sample Tracker Field",0,*Vrui::getToolManager());
	
	/* Set the custom tool class' input layout: */
	toolFactory->setNumButtons(2);
	toolFactory->setButtonFunction(0,"Sample");
	toolFactory->setButtonFunction(1,"Skip");
	
	/* Register the custom tool class with the Vrui tool manager: */
	Vrui::getToolManager()->addClass(toolFactory,Vrui::ToolManager::defaultToolFactoryDestructor);
	}

SampleTrackerField::SampleTool::SampleTool(const Vrui::ToolFactory* factory,const Vrui::ToolInputAssignment& inputAssignment)
	:Vrui::Tool(factory,inputAssignment)
	{
	}

const Vrui::ToolFactory* SampleTrackerField::SampleTool::getFactory(void) const
	{
	return factory;
	}

void SampleTrackerField::SampleTool::buttonCallback(int buttonSlotIndex,Vrui::InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState) // Button has just been pressed
		{
		/* Take a sample: */
		application->sample(buttonSlotIndex==0);
		}
	}

void SampleTrackerField::SampleTool::display(GLContextData& contextData) const
	{
	/* Go to navigation coordinates: */
	glPushMatrix();
	glLoadMatrix(Vrui::getDisplayState(contextData).modelviewNavigational);
	
	/* Draw the cross: */
	application->drawCross(getButtonDeviceTransformation(0),GLfloat(Vrui::getUiSize()),GLfloat(Vrui::getUiSize())*20.0f);
	
	/* Go back to physical coordinates: */
	glPopMatrix();
	}

/***********************************
Methods of class SampleTrackerField:
***********************************/

void SampleTrackerField::centerView(void)
	{
	Vrui::Point samplePos=gridBase;
	for(int i=0;i<3;++i)
		samplePos[i]+=Vrui::Scalar(nextSample[i])*Vrui::Scalar(cellSize[i]);
	
	Vrui::NavTransform nav=Vrui::NavTransform::translateFromOriginTo(Vrui::getDisplayCenter());
	nav*=Vrui::NavTransform::scale(Vrui::getDisplaySize()/Vrui::Scalar(48));
	nav*=Vrui::NavTransform::rotate(Vrui::Rotation::rotateFromTo(Vrui::Vector(0,0,1),Vrui::getUpDirection()));
	nav*=Vrui::NavTransform::rotate(Vrui::Rotation::rotateX(Math::rad(Vrui::Scalar(15))));
	nav*=Vrui::NavTransform::rotate(Vrui::Rotation::rotateZ(Math::rad(Vrui::Scalar(-15))));
	nav*=Vrui::NavTransform::translateToOriginFrom(samplePos);
	Vrui::setNavigationTransformation(nav);
	}

void SampleTrackerField::drawCross(const Vrui::ONTransform& t,GLfloat radius,GLfloat length) const
	{
	glPushAttrib(GL_ENABLE_BIT|GL_LIGHTING_BIT|GL_LINE_BIT);
	
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
	
	glPushMatrix();
	
	/* Go to the given coordinates: */
	glMultMatrix(t);
	
	glPushMatrix();
	glRotated(90.0,0.0,1.0,0.0);
	glColor3f(1.0f,0.5f,0.5f);
	glDrawArrow(radius,radius*2.0f,radius*3.5f,length,16);
	glPopMatrix();
	
	glPushMatrix();
	glRotated(-90.0,1.0,0.0,0.0);
	glColor3f(0.5f,1.0f,0.5f);
	glDrawArrow(radius,radius*2.0f,radius*3.5f,length,16);
	glPopMatrix();
	
	glColor3f(0.5f,0.5f,1.0f);
	glDrawArrow(radius,radius*2.0f,radius*3.5f,length,16);
	
	glPopMatrix();
	
	glPopAttrib();
	}

void SampleTrackerField::saveGridOKCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData)
	{
	/* Open the output file: */
	std::ofstream gridFile(cbData->getSelectedPath().c_str());
	
	/* Write the grid size: */
	gridFile<<gridSize[0]<<' '<<gridSize[1]<<' '<<gridSize[2]<<std::endl;
	
	/* Write the grid base point: */
	gridFile<<gridBase[0]<<' '<<gridBase[1]<<' '<<gridBase[2]<<std::endl;
	
	/* Write the grid cell size: */
	gridFile<<cellSize[0]<<' '<<cellSize[1]<<' '<<cellSize[2]<<std::endl;
	
	/* Write all grid points: */
	Index i;
	for(i=samples.beginIndex();i!=samples.endIndex();samples.preInc(i))
		{
		if(samples(i).first)
			{
			gridFile<<"V "<<samples(i).second<<std::endl;
			}
		else
			{
			gridFile<<'I'<<std::endl;
			}
		}
	
	/* Close the file selection dialog: */
	cbData->fileSelectionDialog->close();
	}

SampleTrackerField::SampleTrackerField(int& argc,char**& argv)
	:Vrui::Application(argc,argv),
	 groundTruthDevice(0)
	{
	/* Parse the command line: */
	gridSize=Index(5,5,3);
	cellSize=Size(Vrui::Scalar(12)*Vrui::getInchFactor(),Vrui::Scalar(12)*Vrui::getInchFactor(),Vrui::Scalar(12)*Vrui::getInchFactor());
	gridBase=Vrui::Point(0,0,Vrui::Scalar(6)*Vrui::getInchFactor());
	
	for(int i=1;i<argc;++i)
		{
		if(argv[i][0]=='-')
			{
			if(strcasecmp(argv[i]+1,"groundTruthDevice")==0||strcasecmp(argv[i]+1,"gtd")==0)
				{
				++i;
				if(i<argc)
					groundTruthDevice=Vrui::findInputDevice(argv[i]);
				else
					std::cerr<<"Ignoring dangling "<<argv[i-1]<<" option"<<std::endl;
				}
			else if(strcasecmp(argv[i]+1,"gridCenter")==0||strcasecmp(argv[i]+1,"gc")==0)
				{
				i+=3;
				if(i<argc)
					{
					for(int j=0;j<3;++j)
						gridBase[j]=Vrui::Scalar(atof(argv[i-2+j]));
					}
				else
					std::cerr<<"Ignoring dangling "<<argv[i-3]<<" option"<<std::endl;
				}
			else if(strcasecmp(argv[i]+1,"gridSize")==0||strcasecmp(argv[i]+1,"gs")==0)
				{
				i+=3;
				if(i<argc)
					{
					for(int j=0;j<3;++j)
						gridSize[j]=atoi(argv[i-2+j]);
					}
				else
					std::cerr<<"Ignoring dangling "<<argv[i-3]<<" option"<<std::endl;
				}
			else if(strcasecmp(argv[i]+1,"gridCellSize")==0||strcasecmp(argv[i]+1,"gcs")==0)
				{
				i+=3;
				if(i<argc)
					{
					for(int j=0;j<3;++j)
						cellSize[j]=Vrui::Scalar(atof(argv[i-2+j]));
					}
				else
					std::cerr<<"Ignoring dangling "<<argv[i-3]<<" option"<<std::endl;
				}
			else
				std::cerr<<"Ignoring unknown option "<<argv[i]<<std::endl;
			}
		else
			std::cerr<<"Ignoring unknown parameter "<<argv[i]<<std::endl;
		}
	for(int i=0;i<2;++i)
		gridBase[i]-=Math::div2(Vrui::Scalar(gridSize[i]-1))*cellSize[i];
	
	if(groundTruthDevice==0)
		throw std::runtime_error("SampleTrackerField: No \"ground truth\" input device selected");
	
	/* Initialize the sample array: */
	samples.resize(gridSize);
	nextSample=Index(0,0,0);
	
	/* Create the sampling tool class: */
	SampleTool::initClass();
	
	/* Center the view on the first sample point: */
	centerView();
	}

void SampleTrackerField::display(GLContextData& contextData) const
	{
	glPushAttrib(GL_ENABLE_BIT|GL_LIGHTING_BIT|GL_LINE_BIT);
	
	/* Draw the entire sampling grid: */
	glDisable(GL_LIGHTING);
	glLineWidth(1.0f);
	glColor3f(0.0f,1.0f,0.0f);
	
	glPushMatrix();
	glTranslate(gridBase-Vrui::Point::origin);
	glScale(cellSize);
	
	glBegin(GL_LINES);
	
	/* Draw lines along X: */
	for(int y=0;y<gridSize[1];++y)
		for(int z=0;z<gridSize[2];++z)
			{
			glVertex(0,y,z);
			glVertex(gridSize[0]-1,y,z);
			}
	
	/* Draw lines along Y: */
	for(int z=0;z<gridSize[2];++z)
		for(int x=0;x<gridSize[0];++x)
			{
			glVertex(x,0,z);
			glVertex(x,gridSize[1]-1,z);
			}
	
	/* Draw lines along Z: */
	for(int x=0;x<gridSize[0];++x)
		for(int y=0;y<gridSize[1];++y)
			{
			glVertex(x,y,0);
			glVertex(x,y,gridSize[2]-1);
			}
	
	glEnd();
	
	glPopMatrix();
	
	/* Draw the already-sampled grid: */
	glBegin(GL_LINES);
	Index i;
	for(i[0]=0;i[0]<gridSize[0];++i[0])
		for(i[1]=0;i[1]<gridSize[1];++i[1])
			for(i[2]=0;i[2]<gridSize[2];++i[2])
				if(samples(i).first)
					{
					glColor3f(1.0f,0.0f,1.0f);
					if(i[0]+1<gridSize[0]&&samples(i+Index(1,0,0)).first)
						{
						glVertex(samples(i).second.getOrigin());
						glVertex(samples(i+Index(1,0,0)).second.getOrigin());
						}
					if(i[1]+1<gridSize[1]&&samples(i+Index(0,1,0)).first)
						{
						glVertex(samples(i).second.getOrigin());
						glVertex(samples(i+Index(0,1,0)).second.getOrigin());
						}
					if(i[2]+1<gridSize[2]&&samples(i+Index(0,0,1)).first)
						{
						glVertex(samples(i).second.getOrigin());
						glVertex(samples(i+Index(0,0,1)).second.getOrigin());
						}
					
					glColor3f(1.0f,0.5f,0.5f);
					glVertex(samples(i).second.getOrigin()-samples(i).second.getRotation().getDirection(0)*Vrui::Scalar(2));
					glVertex(samples(i).second.getOrigin()+samples(i).second.getRotation().getDirection(0)*Vrui::Scalar(2));
					
					glColor3f(0.5f,1.0f,0.5f);
					glVertex(samples(i).second.getOrigin()-samples(i).second.getRotation().getDirection(1)*Vrui::Scalar(2));
					glVertex(samples(i).second.getOrigin()+samples(i).second.getRotation().getDirection(1)*Vrui::Scalar(2));
					
					glColor3f(0.5f,0.5f,1.0f);
					glVertex(samples(i).second.getOrigin()-samples(i).second.getRotation().getDirection(2)*Vrui::Scalar(2));
					glVertex(samples(i).second.getOrigin()+samples(i).second.getRotation().getDirection(2)*Vrui::Scalar(2));
					}
	glEnd();
	
	glPointSize(3.0f);
	glBegin(GL_POINTS);
	for(SampleArray::const_iterator saIt=samples.begin();saIt!=samples.end();++saIt)
		if(saIt->first)
			glVertex(saIt->second.getOrigin());
	glEnd();
	
	glPopAttrib();
	
	/* Draw the position and orientation of the "ground truth" device: */
	drawCross(groundTruthDevice->getTransformation(),GLfloat(Vrui::getUiSize())*0.5f,GLfloat(Vrui::getUiSize())*15.0f);
	
	if(nextSample[0]<gridSize[0])
		{
		/* Highlight the next grid position to be sampled: */
		drawCross(Vrui::ONTransform::translateFromOriginTo(gridBase+Vrui::Vector(double(nextSample[0])*cellSize[0],double(nextSample[1])*cellSize[1],double(nextSample[2])*cellSize[2])),GLfloat(Vrui::getUiSize()),GLfloat(Vrui::getUiSize())*20.0f);
		}
	}

void SampleTrackerField::sample(bool valid)
	{
	/* Store the sample and advance the sample pointer: */
	samples(nextSample)=std::make_pair(valid,groundTruthDevice->getTransformation());
	samples.preInc(nextSample);
	
	/* If sampling is complete, select a file name to which to save the sampled grid: */
	if(nextSample==samples.endIndex())
		{
		GLMotif::FileSelectionDialog* saveGridDialog=new GLMotif::FileSelectionDialog(Vrui::getWidgetManager(),"Save Sampled Grid...",Vrui::openDirectory("."),"SampledGrid.grid",".grid");
		saveGridDialog->getOKCallbacks().add(this,&SampleTrackerField::saveGridOKCallback);
		saveGridDialog->deleteOnCancel();
		Vrui::popupPrimaryWidget(saveGridDialog);
		}
	else
		{
		/* Center the view on the next sample point: */
		centerView();
		}
	}

VRUI_APPLICATION_RUN(SampleTrackerField)
