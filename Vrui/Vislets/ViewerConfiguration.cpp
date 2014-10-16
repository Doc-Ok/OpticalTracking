/***********************************************************************
ViewerConfiguration - Vislet class to configure the settings of a Vrui
Viewer object from inside a running Vrui application.
Copyright (c) 2013-2014 Oliver Kreylos

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

#include <Vrui/Vislets/ViewerConfiguration.h>

#include <string>
#include <Misc/StandardValueCoders.h>
#include <Misc/CompoundValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthonormalTransformation.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/Margin.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/Blind.h>
#include <GLMotif/Separator.h>
#include <GLMotif/Label.h>
#include <Vrui/Vrui.h>
#include <Vrui/Viewer.h>
#include <Vrui/VRScreen.h>
#include <Vrui/VisletManager.h>

namespace Vrui {

namespace Vislets {

/*******************************************
Methods of class ViewerConfigurationFactory:
*******************************************/

ViewerConfigurationFactory::ViewerConfigurationFactory(VisletManager& visletManager)
	:VisletFactory("ViewerConfiguration",visletManager)
	{
	#if 0
	/* Insert class into class hierarchy: */
	VisletFactory* visletFactory=visletManager.loadClass("Vislet");
	visletFactory->addChildClass(this);
	addParentClass(visletFactory);
	#endif
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=visletManager.getVisletClassSection(getClassName());
	
	/* Read the configuration unit of measurement: */
	std::string unitName=cfs.retrieveString("./unitName","inch");
	Scalar unitFactor=cfs.retrieveValue<Scalar>("./unitFactor",Scalar(1));
	configUnit=Geometry::LinearUnit(unitName.c_str(),unitFactor);
	
	/* Get the list of Oculus Rift-associated viewers: */
	std::vector<std::string> oculusRiftViewerNames=cfs.retrieveValue<std::vector<std::string> >("./oculusRiftViewerNames",std::vector<std::string>());
	for(std::vector<std::string>::iterator orvnIt=oculusRiftViewerNames.begin();orvnIt!=oculusRiftViewerNames.end();++orvnIt)
		{
		/* Go to the viewer's configuration section: */
		Misc::ConfigurationFileSection orvnCfs=cfs.getSection(orvnIt->c_str());
		
		/* Configure the Oculus Rift-associated viewer: */
		OculusRiftViewer newViewer;
		std::string viewerName=orvnCfs.retrieveString("./viewerName");
		newViewer.viewer=findViewer(viewerName.c_str());
		if(newViewer.viewer==0)
			Misc::throwStdErr("Vrui::ViewerConfiguration: Viewer %s not found",viewerName.c_str());
		std::string leftScreenName=orvnCfs.retrieveString("./leftScreenName");
		newViewer.screens[0]=findScreen(leftScreenName.c_str());
		if(newViewer.screens[0]==0)
			Misc::throwStdErr("Vrui::ViewerConfiguration: Screen %s not found",leftScreenName.c_str());
		std::string rightScreenName=orvnCfs.retrieveString("./rightScreenName");
		newViewer.screens[1]=findScreen(rightScreenName.c_str());
		if(newViewer.screens[1]==0)
			Misc::throwStdErr("Vrui::ViewerConfiguration: Screen %s not found",rightScreenName.c_str());
		newViewer.lensDist=orvnCfs.retrieveValue<Scalar>("./lensDist",configUnit.getInchFactor()*Geometry::LinearUnit::Scalar(2.5));
		
		/* Convert the lens distance to Vrui units by going through inches (USA! USA! USA!): */
		newViewer.lensDist=newViewer.lensDist*getInchFactor()/configUnit.getInchFactor();
		
		/* Store the viewer association: */
		oculusRiftViewers.push_back(newViewer);
		}
	
	/* Set vislet class' factory pointer: */
	ViewerConfiguration::factory=this;
	}

ViewerConfigurationFactory::~ViewerConfigurationFactory(void)
	{
	/* Reset vislet class' factory pointer: */
	ViewerConfiguration::factory=0;
	}

Vislet* ViewerConfigurationFactory::createVislet(int numArguments,const char* const arguments[]) const
	{
	return new ViewerConfiguration(numArguments,arguments);
	}

void ViewerConfigurationFactory::destroyVislet(Vislet* vislet) const
	{
	delete vislet;
	}

extern "C" void resolveViewerConfigurationDependencies(Plugins::FactoryManager<VisletFactory>& manager)
	{
	#if 0
	/* Load base classes: */
	manager.loadClass("Vislet");
	#endif
	}

extern "C" VisletFactory* createViewerConfigurationFactory(Plugins::FactoryManager<VisletFactory>& manager)
	{
	/* Get pointer to vislet manager: */
	VisletManager* visletManager=static_cast<VisletManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	ViewerConfigurationFactory* factory=new ViewerConfigurationFactory(*visletManager);
	
	/* Return factory object: */
	return factory;
	}

extern "C" void destroyViewerConfigurationFactory(VisletFactory* factory)
	{
	delete factory;
	}


/********************************************
Static elements of class ViewerConfiguration:
********************************************/

ViewerConfigurationFactory* ViewerConfiguration::factory=0;

/************************************
Methods of class ViewerConfiguration:
************************************/

void ViewerConfiguration::updateViewer(void)
	{
	if(viewer!=0)
		{
		/* Update the controlled viewer: */
		Vector currentViewDirection=viewer->getHeadTransformation().inverseTransform(viewer->getViewDirection());
		viewer->setEyes(currentViewDirection,eyePos[0],(eyePos[2]-eyePos[1])*Scalar(0.5));
		
		if(oculusRiftViewer!=0)
			{
			/* Shift the left and right screen halves based on the new eye distance: */
			Point origins[2];
			for(int i=0;i<2;++i)
				origins[i]=oculusRiftViewer->screens[i]->getTransform().getOrigin();
			Scalar delta=Math::div2(eyeDist-oculusRiftViewer->lensDist);
			origins[0][0]=-oculusRiftViewer->screens[0]->getWidth()-delta;
			origins[1][0]=delta;
			
			for(int i=0;i<2;++i)
				{
				const ONTransform& t=oculusRiftViewer->screens[i]->getTransform();
				oculusRiftViewer->screens[i]->setTransform(ONTransform(origins[i]-Point::origin,t.getRotation()));
				}
			}
		}
	}

void ViewerConfiguration::setViewer(Viewer* newViewer)
	{
	viewer=newViewer;
	oculusRiftViewer=0;
	
	if(viewer!=0)
		{
		/* Get the current eye positions: */
		eyePos[1]=viewer->getDeviceEyePosition(Viewer::LEFT);
		eyePos[2]=viewer->getDeviceEyePosition(Viewer::RIGHT);
		
		/* Calculate the mono eye position and the eye distance: */
		eyePos[0]=Geometry::mid(eyePos[1],eyePos[2]);
		eyeDist=Geometry::dist(eyePos[1],eyePos[2]);
		
		/* Check if the new viewer is associated with an Oculus Rift HMD: */
		for(std::vector<ViewerConfigurationFactory::OculusRiftViewer>::iterator orvIt=factory->oculusRiftViewers.begin();orvIt!=factory->oculusRiftViewers.end();++orvIt)
			if(viewer==orvIt->viewer)
				{
				oculusRiftViewer=&(*orvIt);
				break;
				}
		}
	else
		{
		/* Reset all positions: */
		for(int eyeIndex=0;eyeIndex<3;++eyeIndex)
			eyePos[eyeIndex]=Point::origin;
		eyeDist=Scalar(0);
		}
	
	/* Update the eye position sliders: */
	for(int eyeIndex=0;eyeIndex<3;++eyeIndex)
		for(int i=0;i<3;++i)
			eyePosSliders[eyeIndex][i]->setValue(eyePos[eyeIndex][i]*unitScale);
	
	/* Update the eye distance slider: */
	eyeDistanceSlider->setValue(eyeDist*unitScale);
	}

void ViewerConfiguration::viewerMenuCallback(GLMotif::DropdownBox::ValueChangedCallbackData* cbData)
	{
	/* Select a new viewer: */
	setViewer(cbData->newSelectedItem!=0?findViewer(cbData->getItem()):0);
	}

void ViewerConfiguration::eyePosSliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData,const int& sliderIndex)
	{
	/* Determine which eye position has changed: */
	int eyeIndex=sliderIndex/3;
	int component=sliderIndex%3;
	
	/* Update the changed eye: */
	eyePos[eyeIndex][component]=cbData->value/unitScale;
	
	/* Update dependent state: */
	switch(eyeIndex)
		{
		case 0: // Mono eye
			{
			/* Re-position the left and right eyes: */
			Scalar offset=(eyePos[2][component]-eyePos[1][component])*Scalar(0.5);
			eyePos[1][component]=eyePos[0][component]-offset;
			eyePos[2][component]=eyePos[0][component]+offset;
			
			/* Update the GUI: */
			for(int updateEyeIndex=1;updateEyeIndex<3;++updateEyeIndex)
				eyePosSliders[updateEyeIndex][component]->setValue(eyePos[updateEyeIndex][component]*unitScale);
			
			break;
			}
		
		case 1: // Left eye
		case 2: // Right eye
			/* Recalculate the mono eye position and the eye distance: */
			eyePos[0][component]=Math::mid(eyePos[1][component],eyePos[2][component]);
			eyeDist=Geometry::dist(eyePos[1],eyePos[2]);
			
			/* Update the GUI: */
			eyePosSliders[0][component]->setValue(eyePos[0][component]*unitScale);
			eyeDistanceSlider->setValue(eyeDist*unitScale);
			break;
		}
	
	/* Update the controlled viewer: */
	updateViewer();
	}

void ViewerConfiguration::eyeDistanceSliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData)
	{
	/* Update the eye distance: */
	eyeDist=cbData->value/unitScale;
	
	/* Re-position the left and right eyes: */
	Vector eyeOffset=eyePos[2]-eyePos[1];
	eyeOffset.normalize();
	eyeOffset*=eyeDist*Scalar(0.5);
	eyePos[1]=eyePos[0]-eyeOffset;
	eyePos[2]=eyePos[0]+eyeOffset;
	
	/* Update the GUI: */
	for(int eyeIndex=1;eyeIndex<3;++eyeIndex)
		for(int i=0;i<3;++i)
			eyePosSliders[eyeIndex][i]->setValue(eyePos[eyeIndex][i]*unitScale);
	
	/* Update the controlled viewer: */
	updateViewer();
	}

void ViewerConfiguration::buildViewerConfigurationControls(void)
	{
	/* Build the graphical user interface: */
	const GLMotif::StyleSheet& ss=*getUiStyleSheet();
	
	dialogWindow=new GLMotif::PopupWindow("ViewerConfigurationDialog",getWidgetManager(),"Viewer Configuration");
	dialogWindow->setHideButton(true);
	dialogWindow->setResizableFlags(true,false);
	
	GLMotif::RowColumn* viewerConfiguration=new GLMotif::RowColumn("ViewerConfiguration",dialogWindow,false);
	viewerConfiguration->setOrientation(GLMotif::RowColumn::VERTICAL);
	viewerConfiguration->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	viewerConfiguration->setNumMinorWidgets(2);
	
	/* Create a drop-down menu to select a viewer: */
	new GLMotif::Label("ViewerLabel",viewerConfiguration,"Viewer");
	viewerMenu=new GLMotif::DropdownBox("ViewerMenu",viewerConfiguration);
	int mainViewerIndex=0;
	for(int viewerIndex=0;viewerIndex<getNumViewers();++viewerIndex)
		{
		Viewer* viewer=getViewer(viewerIndex);
		viewerMenu->addItem(viewer->getName());
		if(viewer==getMainViewer())
			mainViewerIndex=viewerIndex;
		}
	viewerMenu->setSelectedItem(mainViewerIndex);
	viewerMenu->getValueChangedCallbacks().add(this,&ViewerConfiguration::viewerMenuCallback);
	
	/* Calculate an appropriate slider range and granularity: */
	Scalar sliderRange=Scalar(18)*factory->configUnit.getInchFactor(); // Slider range is at least 18"
	Scalar sliderRangeFactor=Math::pow(Scalar(10),Math::floor(Math::log10(sliderRange)));
	sliderRange=Math::ceil(sliderRange/sliderRangeFactor)*sliderRangeFactor;
	Scalar sliderStep=Scalar(0.01)*factory->configUnit.getInchFactor(); // Slider granularity is at most 0.01"
	int sliderStepDigits=int(Math::floor(Math::log10(sliderStep)));
	Scalar sliderStepFactor=Math::pow(Scalar(10),Scalar(sliderStepDigits));
	sliderStep=Math::floor(sliderStep/sliderStepFactor)*sliderStepFactor;
	sliderStepDigits=sliderStepDigits<0?-sliderStepDigits:0;
	
	/* Create three sliders to set the mono eye position: */
	new GLMotif::Label("MonoEyePosLabel",viewerConfiguration,"Mono Eye");
	
	GLMotif::RowColumn* monoEyePosBox=new GLMotif::RowColumn("MonoEyePosBox",viewerConfiguration,false);
	monoEyePosBox->setPacking(GLMotif::RowColumn::PACK_GRID);
	for(int i=0;i<3;++i)
		{
		char epsName[14]="EyePosSlider ";
		epsName[12]=char(i+'0');
		eyePosSliders[0][i]=new GLMotif::TextFieldSlider(epsName,monoEyePosBox,7,ss.fontHeight*10.0f);
		eyePosSliders[0][i]->getTextField()->setFieldWidth(6);
		eyePosSliders[0][i]->getTextField()->setPrecision(sliderStepDigits);
		eyePosSliders[0][i]->getTextField()->setFloatFormat(GLMotif::TextField::FIXED);
		eyePosSliders[0][i]->setSliderMapping(GLMotif::TextFieldSlider::LINEAR);
		eyePosSliders[0][i]->setValueType(GLMotif::TextFieldSlider::FLOAT);
		eyePosSliders[0][i]->setValueRange(-sliderRange,sliderRange,sliderStep);
		eyePosSliders[0][i]->getValueChangedCallbacks().add(this,&ViewerConfiguration::eyePosSliderCallback,i);
		}
	monoEyePosBox->manageChild();
	
	/* Create a slider to set the eye separation distance: */
	new GLMotif::Label("EyeDistLabel",viewerConfiguration,"Eye Distance");
	
	eyeDistanceSlider=new GLMotif::TextFieldSlider("EyeDistanceSlider",viewerConfiguration,7,ss.fontHeight*10.0f);
	eyeDistanceSlider->getTextField()->setFieldWidth(6);
	eyeDistanceSlider->getTextField()->setPrecision(sliderStepDigits);
	eyeDistanceSlider->getTextField()->setFloatFormat(GLMotif::TextField::FIXED);
	eyeDistanceSlider->setSliderMapping(GLMotif::TextFieldSlider::LINEAR);
	eyeDistanceSlider->setValueType(GLMotif::TextFieldSlider::FLOAT);
	eyeDistanceSlider->setValueRange(sliderStep*Scalar(10),sliderRange,sliderStep);
	eyeDistanceSlider->getValueChangedCallbacks().add(this,&ViewerConfiguration::eyeDistanceSliderCallback);
	
	/* Create two triples of sliders to set the left and right eye positions: */
	for(int eyeIndex=1;eyeIndex<3;++eyeIndex)
		{
		/* Create a separator: */
		new GLMotif::Blind(eyeIndex==1?"Blind1":"Blind2",viewerConfiguration);
		new GLMotif::Separator(eyeIndex==1?"Separator1":"Separator2",viewerConfiguration,GLMotif::Separator::HORIZONTAL,ss.fontHeight,GLMotif::Separator::LOWERED);
		
		/* Create three sliders to set the left or right eye position: */
		new GLMotif::Label(eyeIndex==1?"LeftEyePosLabel":"RightEyePosLabel",viewerConfiguration,eyeIndex==1?"Left Eye":"Right Eye");
		
		GLMotif::RowColumn* eyePosBox=new GLMotif::RowColumn(eyeIndex==1?"LeftEyePosBox":"RightEyePosBox",viewerConfiguration,false);
		eyePosBox->setPacking(GLMotif::RowColumn::PACK_GRID);
		for(int i=0;i<3;++i)
			{
			char epsName[14]="EyePosSlider ";
			epsName[12]=char(eyeIndex*3+i+'0');
			eyePosSliders[eyeIndex][i]=new GLMotif::TextFieldSlider(epsName,eyePosBox,7,ss.fontHeight*10.0f);
			eyePosSliders[eyeIndex][i]->getTextField()->setFieldWidth(6);
			eyePosSliders[eyeIndex][i]->getTextField()->setPrecision(sliderStepDigits);
			eyePosSliders[eyeIndex][i]->getTextField()->setFloatFormat(GLMotif::TextField::FIXED);
			eyePosSliders[eyeIndex][i]->setSliderMapping(GLMotif::TextFieldSlider::LINEAR);
			eyePosSliders[eyeIndex][i]->setValueType(GLMotif::TextFieldSlider::FLOAT);
			eyePosSliders[eyeIndex][i]->setValueRange(-sliderRange,sliderRange,sliderStep);
			eyePosSliders[eyeIndex][i]->getValueChangedCallbacks().add(this,&ViewerConfiguration::eyePosSliderCallback,eyeIndex*3+i);
			}
		eyePosBox->manageChild();
		}
	
	viewerConfiguration->manageChild();
	
	/* Initialize vislet state and GUI: */
	setViewer(getViewer(mainViewerIndex));
	}

ViewerConfiguration::ViewerConfiguration(int numArguments,const char* const arguments[])
	:unitScale(factory->configUnit.getInchFactor()/getInchFactor()),
	 firstEnable(true),
	 viewer(0),
	 dialogWindow(0)
	{
	}

ViewerConfiguration::~ViewerConfiguration(void)
	{
	delete dialogWindow;
	}

VisletFactory* ViewerConfiguration::getFactory(void) const
	{
	return factory;
	}

void ViewerConfiguration::disable(void)
	{
	if(dialogWindow!=0)
		{
		/* Hide the viewer configuration dialog: */
		popdownPrimaryWidget(dialogWindow);
		}
	
	active=false;
	}

void ViewerConfiguration::enable(void)
	{
	/* Ignore the first time the vislet is enabled: */
	if(firstEnable)
		{
		firstEnable=false;
		return;
		}
	
	/* Check if the filming control GUI needs to be created: */
	if(dialogWindow==0)
		buildViewerConfigurationControls();
	
	/* Show the viewer configuration dialog: */
	popupPrimaryWidget(dialogWindow);
	
	active=true;
	}

}

}
