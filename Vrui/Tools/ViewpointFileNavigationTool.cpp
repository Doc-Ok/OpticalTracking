/***********************************************************************
ViewpointFileNavigationTool - Class for tools to play back previously
saved viewpoint data files.
Copyright (c) 2007-2014 Oliver Kreylos

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

#include <Vrui/Tools/ViewpointFileNavigationTool.h>

#include <stdio.h>
#include <stdexcept>
#include <Misc/FileNameExtensions.h>
#include <Misc/File.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <IO/File.h>
#include <Math/Math.h>
#include <Math/Matrix.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/gl.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/Label.h>
#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>
#include <Vrui/OpenFile.h>

namespace Vrui {

/******************************************************************
Methods of class ViewpointFileNavigationToolFactory::Configuration:
******************************************************************/

ViewpointFileNavigationToolFactory::Configuration::Configuration(void)
	:showGui(false),showKeyframes(true),
	 pauseFileName("ViewpointFileNavigation.pauses"),
	 autostart(false)
	{
	}

void ViewpointFileNavigationToolFactory::Configuration::read(const Misc::ConfigurationFileSection& cfs)
	{
	viewpointFileName=cfs.retrieveString("./viewpointFileName",viewpointFileName);
	showGui=cfs.retrieveValue<bool>("./showGui",showGui);
	showKeyframes=cfs.retrieveValue<bool>("./showKeyframes",showKeyframes);
	pauseFileName=cfs.retrieveString("./pauseFileName",pauseFileName);
	autostart=cfs.retrieveValue<bool>("./autostart",autostart);
	}

void ViewpointFileNavigationToolFactory::Configuration::write(Misc::ConfigurationFileSection& cfs) const
	{
	cfs.storeString("./viewpointFileName",viewpointFileName);
	cfs.retrieveValue<bool>("./showGui",showGui);
	cfs.retrieveValue<bool>("./showKeyframes",showKeyframes);
	cfs.storeString("./pauseFileName",pauseFileName);
	cfs.retrieveValue<bool>("./autostart",autostart);
	}

/***************************************************
Methods of class ViewpointFileNavigationToolFactory:
***************************************************/

ViewpointFileNavigationToolFactory::ViewpointFileNavigationToolFactory(ToolManager& toolManager)
	:ToolFactory("ViewpointFileNavigationTool",toolManager),
	 viewpointSelectionHelper("",".view,.views,.curve",openDirectory("."))
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* toolFactory=toolManager.loadClass("NavigationTool");
	toolFactory->addChildClass(this);
	addParentClass(toolFactory);
	
	/* Load class settings: */
	configuration.read(toolManager.getToolClassSection(getClassName()));
	
	/* Set tool class' factory pointer: */
	ViewpointFileNavigationTool::factory=this;
	}

ViewpointFileNavigationToolFactory::~ViewpointFileNavigationToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	ViewpointFileNavigationTool::factory=0;
	}

const char* ViewpointFileNavigationToolFactory::getName(void) const
	{
	return "Curve File Animation";
	}

const char* ViewpointFileNavigationToolFactory::getButtonFunction(int) const
	{
	return "Start / Stop";
	}

Tool* ViewpointFileNavigationToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new ViewpointFileNavigationTool(this,inputAssignment);
	}

void ViewpointFileNavigationToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveViewpointFileNavigationToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("NavigationTool");
	}

extern "C" ToolFactory* createViewpointFileNavigationToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	ViewpointFileNavigationToolFactory* viewpointFileNavigationToolFactory=new ViewpointFileNavigationToolFactory(*toolManager);
	
	/* Return factory object: */
	return viewpointFileNavigationToolFactory;
	}

extern "C" void destroyViewpointFileNavigationToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/****************************************************
Static elements of class ViewpointFileNavigationTool:
****************************************************/

ViewpointFileNavigationToolFactory* ViewpointFileNavigationTool::factory=0;

/********************************************
Methods of class ViewpointFileNavigationTool:
********************************************/

void ViewpointFileNavigationTool::positionSliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData)
	{
	/* Set the current curve parameter: */
	parameter=Scalar(cbData->value);
	
	/* Navigate to the new parameter: */
	navigate(parameter);
	}

void ViewpointFileNavigationTool::speedSliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData)
	{
	/* Set the speed: */
	speed=Scalar(cbData->value);
	}

void ViewpointFileNavigationTool::createGui(void)
	{
	const GLMotif::StyleSheet& ss=*getWidgetManager()->getStyleSheet();
	
	/* Create the playback control dialog window: */
	controlDialogPopup=new GLMotif::PopupWindow("ControlDialogPopup",getWidgetManager(),"Playback Control");
	controlDialogPopup->setResizableFlags(true,false);
	
	GLMotif::RowColumn* controlDialog=new GLMotif::RowColumn("ControlDialog",controlDialogPopup,false);
	controlDialog->setOrientation(GLMotif::RowColumn::VERTICAL);
	controlDialog->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	controlDialog->setNumMinorWidgets(2);
	
	new GLMotif::Label("PositionLabel",controlDialog,"Position");
	
	positionSlider=new GLMotif::TextFieldSlider("PositionSlider",controlDialog,8,ss.fontHeight*10.0f);
	positionSlider->getTextField()->setFloatFormat(GLMotif::TextField::FIXED);
	positionSlider->getTextField()->setFieldWidth(7);
	positionSlider->getTextField()->setPrecision(1);
	positionSlider->setValueRange(splines.front().t[0],splines.back().t[1],1.0);
	positionSlider->setValue(parameter);
	positionSlider->getValueChangedCallbacks().add(this,&ViewpointFileNavigationTool::positionSliderCallback);
	
	new GLMotif::Label("SpeedLabel",controlDialog,"Speed");
	
	GLMotif::TextFieldSlider* speedSlider=new GLMotif::TextFieldSlider("SpeedSlider",controlDialog,8,ss.fontHeight*10.0f);
	speedSlider->getTextField()->setFloatFormat(GLMotif::TextField::FIXED);
	speedSlider->getTextField()->setFieldWidth(7);
	speedSlider->getTextField()->setPrecision(2);
	speedSlider->setValueRange(-2.0,2.0,0.01);
	speedSlider->getSlider()->addNotch(-1.0f);
	speedSlider->getSlider()->addNotch(1.0f);
	speedSlider->setValue(speed);
	speedSlider->getValueChangedCallbacks().add(this,&ViewpointFileNavigationTool::speedSliderCallback);
	
	controlDialog->manageChild();
	
	popupPrimaryWidget(controlDialogPopup);
	}

void ViewpointFileNavigationTool::readViewpointFile(const char* fileName)
	{
	try
		{
		if(Misc::hasCaseExtension(fileName,".view"))
			{
			/* Load a single viewpoint keyframe from the binary view file: */
			IO::FilePtr viewpointFile=Vrui::openFile(fileName);
			viewpointFile->setEndianness(Misc::LittleEndian);
			
			/* Check the header: */
			static const char* vruiViewpointFileHeader="Vrui viewpoint file v1.0\n";
			char header[80];
			viewpointFile->read(header,strlen(vruiViewpointFileHeader));
			header[strlen(vruiViewpointFileHeader)]='\0';
			if(strcmp(header,vruiViewpointFileHeader)==0)
				{
				/* Read the viewpoint as a control point: */
				ControlPoint v;
				viewpointFile->read<Scalar>(v.center.getComponents(),3);
				v.size=Math::log(viewpointFile->read<Scalar>()); // Sizes are interpolated logarithmically
				viewpointFile->read<Scalar>(v.forward.getComponents(),3);
				viewpointFile->read<Scalar>(v.up.getComponents(),3);
				
				viewpoints.push_back(v);
				}
			else
				{
				/* Display an error message: */
				std::string message="File ";
				message.append(fileName);
				message.append(" is not a viewpoint file.");
				showErrorMessage("Curve File Animation",message.c_str());
				}
			}
		else if(Misc::hasCaseExtension(fileName,".views"))
			{
			/* Load all viewpoint keyframes from the file: */
			Misc::File viewpointFile(fileName,"rt");
			
			Scalar time(0);
			while(true)
				{
				/* Read the next viewpoint: */
				Scalar timeInterval;
				ControlPoint v;
				if(fscanf(viewpointFile.getFilePtr(),"%lf (%lf, %lf, %lf) %lf (%lf, %lf, %lf) (%lf, %lf, %lf)\n",&timeInterval,&v.center[0],&v.center[1],&v.center[2],&v.size,&v.forward[0],&v.forward[1],&v.forward[2],&v.up[0],&v.up[1],&v.up[2])!=11)
					break;
				
				/* Store the viewpoint: */
				time+=timeInterval;
				times.push_back(time);
				v.size=Math::log(v.size); // Sizes are interpolated logarithmically
				viewpoints.push_back(v);
				}
			
			if(viewpoints.size()>1)
				{
				/* Create a big matrix to solve the C^2 spline problem: */
				unsigned int n=viewpoints.size()-1;
				Math::Matrix A(4*n,4*n,0.0);
				Math::Matrix b(4*n,10,0.0);
				
				A(0,0)=1.0;
				writeControlPoint(viewpoints[0],b,0);
				
				double dt1=double(times[1])-double(times[0]);
				#if 1
				/* Zero velocity at start: */
				A(1,0)=-3.0/dt1;
				A(1,1)=3.0/dt1;
				#else
				/* Zero acceleration at start: */
				A(1,0)=6.0/Math::sqr(dt1);
				A(1,1)=-12.0/Math::sqr(dt1);
				A(1,2)=6.0/Math::sqr(dt1);
				#endif
				
				for(unsigned int i=1;i<n;++i)
					{
					double dt0=double(times[i])-double(times[i-1]);
					double dt1=double(times[i+1])-double(times[i]);
					A(i*4-2,i*4-3)=6.0/Math::sqr(dt0);
					A(i*4-2,i*4-2)=-12.0/Math::sqr(dt0);
					A(i*4-2,i*4-1)=6.0/Math::sqr(dt0);
					A(i*4-2,i*4+0)=-6.0/Math::sqr(dt1);
					A(i*4-2,i*4+1)=12.0/Math::sqr(dt1);
					A(i*4-2,i*4+2)=-6.0/Math::sqr(dt1);
					
					A(i*4-1,i*4-2)=-3.0/dt0;
					A(i*4-1,i*4-1)=3.0/dt0;
					A(i*4-1,i*4+0)=3/dt1;
					A(i*4-1,i*4+1)=-3/dt1;
					
					A(i*4+0,i*4-1)=1.0;
					writeControlPoint(viewpoints[i],b,i*4+0);
					
					A(i*4+1,i*4+0)=1.0;
					writeControlPoint(viewpoints[i],b,i*4+1);
					}
				
				double dtn=double(times[n])-double(times[n-1]);
				#if 1
				/* Zero velocity at end: */
				A(n*4-2,n*4-2)=-3.0/dtn;
				A(n*4-2,n*4-1)=3.0/dtn;
				#else
				/* Zero acceleration at end: */
				A(n*4-2,n*4-3)=6.0/Math::sqr(dtn);
				A(n*4-2,n*4-2)=-12.0/Math::sqr(dtn);
				A(n*4-2,n*4-1)=6.0/Math::sqr(dtn);
				#endif
				
				A(n*4-1,n*4-1)=1.0;
				writeControlPoint(viewpoints[n],b,n*4-1);
				
				/* Solve the system of equations: */
				Math::Matrix x=b/A;
				
				/* Create the spline segment list: */
				for(unsigned int i=0;i<n;++i)
					{
					SplineSegment s;
					for(int j=0;j<2;++j)
						s.t[j]=times[i+j];
					for(int cp=0;cp<4;++cp)
						{
						for(int j=0;j<3;++j)
							s.p[cp].center[j]=x(i*4+cp,j);
						s.p[cp].size=x(i*4+cp,3);
						for(int j=0;j<3;++j)
							s.p[cp].forward[j]=x(i*4+cp,4+j);
						for(int j=0;j<3;++j)
							s.p[cp].up[j]=x(i*4+cp,7+j);
						}
					splines.push_back(s);
					}
				}
			}
		else if(Misc::hasCaseExtension(fileName,".curve"))
			{
			/* Load all spline segments from the file: */
			Misc::File viewpointFile(fileName,"rt");
			
			while(true)
				{
				SplineSegment s;
				if(splines.empty())
					{
					/* Read the first control point: */
					ControlPoint cp;
					if(fscanf(viewpointFile.getFilePtr(),"(%lf, %lf, %lf) %lf (%lf, %lf, %lf) (%lf, %lf, %lf)\n",&cp.center[0],&cp.center[1],&cp.center[2],&cp.size,&cp.forward[0],&cp.forward[1],&cp.forward[2],&cp.up[0],&cp.up[1],&cp.up[2])!=10)
						break;
					cp.size=Math::log(cp.size); // Sizes are interpolated logarithmically
					viewpoints.push_back(cp);
					times.push_back(Scalar(0));
					s.t[0]=Scalar(0);
					s.p[0]=cp;
					}
				else
					{
					/* Copy the last control point from the previous segment: */
					s.t[0]=splines.back().t[1];
					s.p[0]=splines.back().p[3];
					}
				
				/* Read the segment's parameter interval: */
				double pi;
				if(fscanf(viewpointFile.getFilePtr(),"%lf\n",&pi)!=1)
					break;
				s.t[1]=s.t[0]+Scalar(pi);
				
				/* Read the intermediate control points: */
				ControlPoint m0;
				if(fscanf(viewpointFile.getFilePtr(),"(%lf, %lf, %lf) %lf (%lf, %lf, %lf) (%lf, %lf, %lf)\n",&m0.center[0],&m0.center[1],&m0.center[2],&m0.size,&m0.forward[0],&m0.forward[1],&m0.forward[2],&m0.up[0],&m0.up[1],&m0.up[2])!=10)
					break;
				m0.size=Math::log(m0.size); // Sizes are interpolated logarithmically
				s.p[1]=m0;
				ControlPoint m1;
				if(fscanf(viewpointFile.getFilePtr(),"(%lf, %lf, %lf) %lf (%lf, %lf, %lf) (%lf, %lf, %lf)\n",&m1.center[0],&m1.center[1],&m1.center[2],&m1.size,&m1.forward[0],&m1.forward[1],&m1.forward[2],&m1.up[0],&m1.up[1],&m1.up[2])!=10)
					break;
				m1.size=Math::log(m1.size); // Sizes are interpolated logarithmically
				s.p[2]=m1;
				
				/* Read the last control point: */
				ControlPoint cp;
				if(fscanf(viewpointFile.getFilePtr(),"(%lf, %lf, %lf) %lf (%lf, %lf, %lf) (%lf, %lf, %lf)\n",&cp.center[0],&cp.center[1],&cp.center[2],&cp.size,&cp.forward[0],&cp.forward[1],&cp.forward[2],&cp.up[0],&cp.up[1],&cp.up[2])!=10)
					break;
				cp.size=Math::log(cp.size); // Sizes are interpolated logarithmically
				viewpoints.push_back(cp);
				times.push_back(s.t[1]);
				s.p[3]=cp;
				
				/* Save the spline segment: */
				splines.push_back(s);
				}
			}
		else
			{
			/* Display an error message: */
			std::string message="Curve file ";
			message.append(fileName);
			message.append(" has unrecognized extension \"");
			message.append(Misc::getExtension(fileName));
			message.push_back('"');
			showErrorMessage("Curve File Animation",message.c_str());
			}
		}
	catch(std::runtime_error err)
		{
		/* Display an error message: */
		std::string message="Could not read curve file ";
		message.append(fileName);
		message.append(" due to exception ");
		message.append(err.what());
		showErrorMessage("Curve File Animation",message.c_str());
		}
	
	if(!splines.empty())
		{
		/* Start animating from the beginning: */
		paused=false;
		parameter=splines.front().t[0];
		
		/* Create playback control dialog if requested: */
		if(configuration.showGui)
			createGui();
		
		/* Start animating if requested: */
		if(configuration.autostart)
			{
			firstFrame=true;
			activate();
			}
		}
	else if(!viewpoints.empty()&&configuration.autostart&&activate())
		{
		/* Go to the first viewpoint: */
		const ControlPoint& v=viewpoints[0];
		NavTransform nav=NavTransform::identity;
		nav*=NavTransform::translateFromOriginTo(getDisplayCenter());
		nav*=NavTransform::rotate(Rotation::fromBaseVectors(getForwardDirection()^getUpDirection(),getForwardDirection()));
		nav*=NavTransform::scale(getDisplaySize()/Math::exp(v.size)); // Scales are interpolated logarithmically
		nav*=NavTransform::rotate(Geometry::invert(Rotation::fromBaseVectors(v.forward^v.up,v.forward)));
		nav*=NavTransform::translateToOriginFrom(v.center);
		setNavigationTransformation(nav);
		
		deactivate();
		}
	}

void ViewpointFileNavigationTool::loadViewpointFileCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData)
	{
	/* Load the selected viewpoint file: */
	readViewpointFile(cbData->selectedDirectory->getPath(cbData->selectedFileName).c_str());
	}

void ViewpointFileNavigationTool::writeControlPoint(const ViewpointFileNavigationTool::ControlPoint& cp,Math::Matrix& b,unsigned int rowIndex)
	{
	for(int j=0;j<3;++j)
		b(rowIndex,j)=cp.center[j];
	b(rowIndex,3)=cp.size;
	Vector forward=Geometry::normalize(cp.forward);
	for(int j=0;j<3;++j)
		b(rowIndex,4+j)=forward[j];
	Vector up=Geometry::normalize(cp.up);
	for(int j=0;j<3;++j)
		b(rowIndex,7+j)=up[j];
	}

void ViewpointFileNavigationTool::interpolate(const ViewpointFileNavigationTool::ControlPoint& p0,const ViewpointFileNavigationTool::ControlPoint& p1,Scalar t,ViewpointFileNavigationTool::ControlPoint& result)
	{
	result.center=Geometry::affineCombination(p0.center,p1.center,t);
	result.size=p0.size*(Scalar(1)-t)+p1.size*t;
	result.forward=p0.forward*(Scalar(1)-t)+p1.forward*t;
	result.up=p0.up*(Scalar(1)-t)+p1.up*t;
	}

bool ViewpointFileNavigationTool::navigate(Scalar parameter)
	{
	/* Find the spline segment containing the given parameter: */
	int l=0;
	int r=splines.size();
	while(r-l>1)
		{
		int m=(l+r)>>1;
		if(parameter>=splines[m].t[0])
			l=m;
		else
			r=m;
		}
	const SplineSegment& s=splines[l];
	if(parameter>=s.t[0]&&parameter<=s.t[1])
		{
		/* Evaluate the spline segment at the current time: */
		Scalar t=(parameter-s.t[0])/(s.t[1]-s.t[0]);
		ControlPoint cp[6];
		for(int i=0;i<3;++i)
			interpolate(s.p[i],s.p[i+1],t,cp[i]);
		for(int i=0;i<2;++i)
			interpolate(cp[i],cp[i+1],t,cp[3+i]);
		interpolate(cp[3],cp[4],t,cp[5]);

		/* Compute the appropriate navigation transformation from the next viewpoint: */
		NavTransform nav=NavTransform::identity;
		nav*=NavTransform::translateFromOriginTo(getDisplayCenter());
		nav*=NavTransform::rotate(Rotation::fromBaseVectors(getForwardDirection()^getUpDirection(),getForwardDirection()));
		nav*=NavTransform::scale(getDisplaySize()/Math::exp(cp[5].size)); // Scales are interpolated logarithmically
		nav*=NavTransform::rotate(Geometry::invert(Rotation::fromBaseVectors(cp[5].forward^cp[5].up,cp[5].forward)));
		nav*=NavTransform::translateToOriginFrom(cp[5].center);
		
		if(isActive())
			{
			/* Set the viewpoint: */
			setNavigationTransformation(nav);
			}
		else if(activate())
			{
			/* Set the viewpoint: */
			setNavigationTransformation(nav);
			
			/* Deactivate again: */
			deactivate();
			}
		
		nextViewpointIndex=l+1;
		return true;
		}
	else
		{
		/* Stop animating; spline is over: */
		nextViewpointIndex=0;
		return false;
		}
	}

ViewpointFileNavigationTool::ViewpointFileNavigationTool(const ToolFactory* sFactory,const ToolInputAssignment& inputAssignment)
	:NavigationTool(sFactory,inputAssignment),
	 configuration(factory->configuration),
	 controlDialogPopup(0),positionSlider(0),
	 nextViewpointIndex(0U),
	 speed(1),firstFrame(false),paused(false),parameter(0),
	 loadViewpointFileDialog(0)
	{
	}

ViewpointFileNavigationTool::~ViewpointFileNavigationTool(void)
	{
	delete controlDialogPopup;
	}

void ViewpointFileNavigationTool::configure(const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Override per-class configuration settings: */
	configuration.read(configFileSection);
	}

void ViewpointFileNavigationTool::storeState(Misc::ConfigurationFileSection& configFileSection) const
	{
	/* Store configuration settings: */
	configuration.write(configFileSection);
	}

void ViewpointFileNavigationTool::initialize(void)
	{
	/* Load scheduled pauses if the file exists: */
	try
		{
		Misc::File pauseFile(configuration.pauseFileName.c_str(),"rt");
		while(true)
			{
			double pauseTime;
			if(fscanf(pauseFile.getFilePtr(),"%lf",&pauseTime)!=1)
				break;
			pauses.push_back(Scalar(pauseTime));
			}
		}
	catch(std::runtime_error)
		{
		/* Ignore the error */
		}
	
	/* Bring up a file selection dialog if there is no pre-configured viewpoint file: */
	if(configuration.viewpointFileName.empty())
		{
		/* Load a viewpoint file: */
		factory->viewpointSelectionHelper.loadFile("Load Viewpoint File...",this,&ViewpointFileNavigationTool::loadViewpointFileCallback);
		}
	else
		{
		/* Load the configured viewpoint file: */
		readViewpointFile(configuration.viewpointFileName.c_str());
		}
	}

void ViewpointFileNavigationTool::deinitialize(void)
	{
	}

const ToolFactory* ViewpointFileNavigationTool::getFactory(void) const
	{
	return factory;
	}

void ViewpointFileNavigationTool::buttonCallback(int,InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState) // Activation button has just been pressed
		{
		/* Start animating the viewpoint if there are spline segments and the tool can be activated, or go to the next viewpoint: */
		if(!splines.empty())
			{
			if(isActive())
				{
				/* Pause the animation: */
				paused=true;
				deactivate();
				}
			else if(activate())
				{
				if(!paused)
					{
					/* Animate from the beginning: */
					parameter=splines.front().t[0];
					}
				
				/* Resume the animation: */
				firstFrame=true;
				paused=false;
				}
			}
		else if(!viewpoints.empty()&&activate())
			{
			/* Compute the appropriate navigation transformation from the next viewpoint: */
			const ControlPoint& v=viewpoints[nextViewpointIndex];
			NavTransform nav=NavTransform::identity;
			nav*=NavTransform::translateFromOriginTo(getDisplayCenter());
			nav*=NavTransform::rotate(Rotation::fromBaseVectors(getForwardDirection()^getUpDirection(),getForwardDirection()));
			nav*=NavTransform::scale(getDisplaySize()/Math::exp(v.size)); // Scales are interpolated logarithmically
			nav*=NavTransform::rotate(Geometry::invert(Rotation::fromBaseVectors(v.forward^v.up,v.forward)));
			nav*=NavTransform::translateToOriginFrom(v.center);
			
			/* Set the viewpoint: */
			setNavigationTransformation(nav);
			
			/* Go to the next viewpoint: */
			++nextViewpointIndex;
			if(nextViewpointIndex==viewpoints.size())
				nextViewpointIndex=0U;
			
			/* Deactivate the tool: */
			deactivate();
			}
		}
	}

void ViewpointFileNavigationTool::frame(void)
	{
	/* Animate the navigation transformation if the tool is active: */
	if(isActive())
		{
		/* Get the next curve parameter: */
		Scalar newParameter=parameter+Scalar(getFrameTime())*speed;
		if(firstFrame)
			{
			newParameter=parameter;
			firstFrame=false;
			}
		
		/* Check if a pause was scheduled between the last frame and this one: */
		bool passedPause=false;
		for(std::vector<Scalar>::const_iterator pIt=pauses.begin();pIt!=pauses.end();++pIt)
			if(parameter<*pIt&&*pIt<=newParameter)
				{
				passedPause=true;
				newParameter=*pIt;
				break;
				}
		
		/* Navigate to the new curve parameter: */
		if(!navigate(newParameter))
			{
			/* Stop animating, curve is over: */
			deactivate();
			}
		else if(passedPause)
			{
			paused=true;
			deactivate();
			}
		else
			{
			/* Request another frame: */
			scheduleUpdate(getApplicationTime()+1.0/125.0);
			}
		
		/* Update the curve parameter and the GUI: */
		parameter=newParameter;
		if(positionSlider!=0)
			positionSlider->setValue(parameter);
		}
	}

void ViewpointFileNavigationTool::display(GLContextData& contextData) const
	{
	if(!viewpoints.empty()&&configuration.showKeyframes)
		{
		/* Display the next keyframe viewpoint in navigational coordinates: */
		glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
		glDisable(GL_LIGHTING);
		glLineWidth(3.0f);
		glPushMatrix();
		glMultMatrix(getNavigationTransformation());
		
		glBegin(GL_LINES);
		glColor3f(1.0f,0.0f,0.0f);
		glVertex(viewpoints[nextViewpointIndex].center);
		glVertex(viewpoints[nextViewpointIndex].center+viewpoints[nextViewpointIndex].forward*Math::exp(viewpoints[nextViewpointIndex].size)*Scalar(0.25));
		glColor3f(0.0f,1.0f,0.0f);
		glVertex(viewpoints[nextViewpointIndex].center);
		glVertex(viewpoints[nextViewpointIndex].center+viewpoints[nextViewpointIndex].up*Math::exp(viewpoints[nextViewpointIndex].size)*Scalar(0.25));
		glEnd();
		
		glPopMatrix();
		glPopAttrib();
		}
	}

}
