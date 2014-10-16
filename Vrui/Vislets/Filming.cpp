/***********************************************************************
Filming - Vislet class to assist shooting of video inside an immersive
environment by providing run-time control over viewers and environment
settings.
Copyright (c) 2012-2013 Oliver Kreylos

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

#include <Vrui/Vislets/Filming.h>

#include <vector>
#include <Misc/SelfDestructPointer.h>
#include <Misc/SelfDestructArray.h>
#include <Misc/PrintInteger.h>
#include <Misc/StringPrintf.h>
#include <Misc/StringMarshaller.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/CompoundValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Cluster/MulticastPipe.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Geometry/GeometryValueCoders.h>
#include <GL/gl.h>
#include <GL/GLMatrixTemplates.h>
#include <GL/GLTransformationWrappers.h>
#include <GL/GLValueCoders.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/Margin.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/Blind.h>
#include <GLMotif/Label.h>
#include <GLMotif/Button.h>
#include <Vrui/OpenFile.h>
#include <Vrui/InputDevice.h>
#include <Vrui/Lightsource.h>
#include <Vrui/Viewer.h>
#include <Vrui/VRWindow.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/VisletManager.h>

namespace Vrui {

namespace Vislets {

/*******************************
Methods of class FilmingFactory:
*******************************/

FilmingFactory::FilmingFactory(VisletManager& visletManager)
	:VisletFactory("Filming",visletManager),
	 initialViewerPosition(getDisplayCenter()-getForwardDirection()*getDisplaySize()),
	 moveViewerSpeed(getInchFactor()*Scalar(2))
	{
	#if 0
	/* Insert class into class hierarchy: */
	VisletFactory* visletFactory=visletManager.loadClass("Vislet");
	visletFactory->addChildClass(this);
	addParentClass(visletFactory);
	#endif
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=visletManager.getVisletClassSection(getClassName());
	initialViewerPosition=cfs.retrieveValue<Point>("./initialViewerPosition",initialViewerPosition);
	moveViewerSpeed=cfs.retrieveValue<Scalar>("./moveViewerSpeed",moveViewerSpeed);
	
	/* Initialize the filming tool classes: */
	Filming::MoveViewerTool::initClass();
	Filming::MoveGridTool::initClass();
	
	/* Set tool class' factory pointer: */
	Filming::factory=this;
	}

FilmingFactory::~FilmingFactory(void)
	{
	/* Reset tool class' factory pointer: */
	Filming::factory=0;
	}

Vislet* FilmingFactory::createVislet(int numArguments,const char* const arguments[]) const
	{
	return new Filming(numArguments,arguments);
	}

void FilmingFactory::destroyVislet(Vislet* vislet) const
	{
	delete vislet;
	}

extern "C" void resolveFilmingDependencies(Plugins::FactoryManager<VisletFactory>& manager)
	{
	#if 0
	/* Load base classes: */
	manager.loadClass("Vislet");
	#endif
	}

extern "C" VisletFactory* createFilmingFactory(Plugins::FactoryManager<VisletFactory>& manager)
	{
	/* Get pointer to vislet manager: */
	VisletManager* visletManager=static_cast<VisletManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	FilmingFactory* factory=new FilmingFactory(*visletManager);
	
	/* Return factory object: */
	return factory;
	}

extern "C" void destroyFilmingFactory(VisletFactory* factory)
	{
	delete factory;
	}

/************************************************
Static elements of class Filming::MoveViewerTool:
************************************************/

Filming::MoveViewerToolFactory* Filming::MoveViewerTool::factory=0;

/****************************************
Methods of class Filming::MoveViewerTool:
****************************************/

void Filming::MoveViewerTool::initClass(void)
	{
	MoveViewerToolFactory* moveViewerToolFactory=new MoveViewerToolFactory("FilmingMoveViewerTool","Move Filming Viewer",0,*getToolManager());
	moveViewerToolFactory->setNumValuators(3);
	moveViewerToolFactory->setValuatorFunction(0,"Move X");
	moveViewerToolFactory->setValuatorFunction(1,"Move Y");
	moveViewerToolFactory->setValuatorFunction(2,"Move Z");
	getToolManager()->addClass(moveViewerToolFactory,ToolManager::defaultToolFactoryDestructor);
	}

Filming::MoveViewerTool::MoveViewerTool(const ToolFactory* sFactory,const ToolInputAssignment& inputAssignment)
	:Vrui::Tool(sFactory,inputAssignment)
	{
	}

const Vrui::ToolFactory* Filming::MoveViewerTool::getFactory(void) const
	{
	return factory;
	}

void Filming::MoveViewerTool::frame(void)
	{
	if(vislet==0)
		return;
	
	if(vislet->viewerDevice!=0)
		{
		/* Adjust the vislet's head-tracked eye position: */
		bool changed=false;
		for(int axis=0;axis<3;++axis)
			if(getValuatorState(axis)!=0.0)
				{
				vislet->eyePosition[axis]+=getValuatorState(axis)*vislet->factory->moveViewerSpeed*getFrameTime();
				vislet->posSliders[axis]->setValue(vislet->eyePosition[axis]);
				changed=true;
				}
		
		if(changed)
			{
			/* Update the filming viewer: */
			vislet->viewer->setEyes(vislet->viewer->getViewDirection(),vislet->eyePosition,Vector::zero);
			}
		}
	else
		{
		/* Adjust the vislet's fixed viewer position: */
		bool changed=false;
		for(int axis=0;axis<3;++axis)
			if(getValuatorState(axis)!=0.0)
				{
				vislet->viewerPosition[axis]+=getValuatorState(axis)*vislet->factory->moveViewerSpeed*getFrameTime();
				vislet->posSliders[axis]->setValue(vislet->viewerPosition[axis]);
				changed=true;
				}
		
		if(changed)
			{
			/* Update the filming viewer: */
			vislet->viewer->detachFromDevice(TrackerState::translateFromOriginTo(vislet->viewerPosition));
			}
		}
	}

/**********************************************
Static elements of class Filming::MoveGridTool:
**********************************************/

Filming::MoveGridToolFactory* Filming::MoveGridTool::factory=0;

/**************************************
Methods of class Filming::MoveGridTool:
**************************************/

void Filming::MoveGridTool::initClass(void)
	{
	MoveGridToolFactory* moveGridToolFactory=new MoveGridToolFactory("FilmingMoveGridTool","Move Calibration Grid",0,*getToolManager());
	moveGridToolFactory->setNumButtons(1);
	moveGridToolFactory->setButtonFunction(0,"Grab Grid");
	getToolManager()->addClass(moveGridToolFactory,ToolManager::defaultToolFactoryDestructor);
	}

Filming::MoveGridTool::MoveGridTool(const ToolFactory* sFactory,const ToolInputAssignment& inputAssignment)
	:Vrui::Tool(sFactory,inputAssignment)
	{
	}

const Vrui::ToolFactory* Filming::MoveGridTool::getFactory(void) const
	{
	return factory;
	}

void Filming::MoveGridTool::buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData)
	{
	if(vislet==0)
		return;
	
	if(cbData->newButtonState)
		{
		/* Check if the grid was picked: */
		Point pickPosLocal=vislet->gridTransform.inverseTransform(cbData->inputDevice->getPosition());
		if(Math::abs(pickPosLocal[2])<getPointPickDistance()*getNavigationTransformation().getScaling()*Scalar(5)&&vislet->gridDragger==0)
			{
			/* Start dragging the grid: */
			vislet->gridDragger=this;
			
			/* Calculate the dragging transformation: */
			dragTransform=getButtonDeviceTransformation(0);
			dragTransform.doInvert();
			dragTransform*=vislet->gridTransform;
			}
		}
	else if(vislet->gridDragger==this)
		{
		/* Stop dragging: */
		vislet->gridDragger=0;
		}
	}

void Filming::MoveGridTool::frame(void)
	{
	if(vislet==0)
		return;
	
	if(vislet->gridDragger==this)
		{
		/* Update the grid transformation: */
		ONTransform gt=getButtonDeviceTransformation(0)*dragTransform;
		
		/* Snap the grid transformation to the primary axes: */
		gt.leftMultiply(ONTransform::translateToOriginFrom(getButtonDevicePosition(0)));
		Vector gridX=gt.getDirection(0);
		int xAxis=Geometry::findParallelAxis(gridX);
		Vector newGridX=Vector::zero;
		newGridX[xAxis]=gridX[xAxis]<Scalar(0)?Scalar(-1):Scalar(1);
		gt.leftMultiply(ONTransform::rotate(Rotation::rotateFromTo(gridX,newGridX)));
		Vector gridY=gt.getDirection(1);
		int yAxis=Geometry::findParallelAxis(gridY);
		Vector newGridY=Vector::zero;
		newGridY[yAxis]=gridY[yAxis]<Scalar(0)?Scalar(-1):Scalar(1);
		gt.leftMultiply(ONTransform::rotate(Rotation::rotateFromTo(gridY,newGridY)));
		gt.leftMultiply(ONTransform::translateFromOriginTo(getButtonDevicePosition(0)));
		vislet->gridTransform=gt;
		}
	}

/********************************
Static elements of class Filming:
********************************/

FilmingFactory* Filming::factory=0;

/************************
Methods of class Filming:
************************/

void Filming::changeViewerMode(void)
	{
	if(viewerDevice!=0)
		{
		/* Enable head tracking: */
		viewer->attachToDevice(viewerDevice);
		viewer->setEyes(viewer->getViewDirection(),eyePosition,Vector::zero);
		
		/* Set the sliders to change the head-relative eye position: */
		for(int i=0;i<3;++i)
			{
			posSliders[i]->setValueRange(-12.0,12.0,0.05);
			posSliders[i]->setValue(eyePosition[i]);
			}
		}
	else
		{
		/* Disable head tracking: */
		viewer->detachFromDevice(TrackerState::translateFromOriginTo(viewerPosition));
		viewer->setEyes(viewer->getViewDirection(),Point::origin,Vector::zero);
		
		/* Set the sliders to change the physical-coordinate fixed viewing position: */
		for(int i=0;i<3;++i)
			{
			posSliders[i]->setValueRange(getDisplayCenter()[i]-getDisplaySize()*Scalar(4),getDisplayCenter()[i]+getDisplaySize()*Scalar(4),0.1);
			posSliders[i]->setValue(viewerPosition[i]);
			}
		}
	}

void Filming::viewerDeviceMenuCallback(GLMotif::DropdownBox::ValueChangedCallbackData* cbData)
	{
	/* Set the new viewer tracking device: */
	if(cbData->newSelectedItem==0)
		{
		/* Disable head tracking: */
		viewerDevice=0;
		}
	else
		{
		/* Find the new viewer device: */
		viewerDevice=findInputDevice(cbData->getItem());
		}
	
	/* Update the GUI: */
	changeViewerMode();
	}

void Filming::posSliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData,const int& sliderIndex)
	{
	if(viewerDevice!=0)
		{
		/* Update the filming viewer's eye position: */
		eyePosition[sliderIndex]=cbData->value;
		viewer->setEyes(viewer->getViewDirection(),eyePosition,Vector::zero);
		}
	else
		{
		/* Update the filming viewer's fixed position: */
		viewerPosition[sliderIndex]=cbData->value;
		viewer->detachFromDevice(TrackerState::translateFromOriginTo(viewerPosition));
		}
	}

void Filming::windowToggleCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData,const int& windowIndex)
	{
	/* Select or deselect the window: */
	windowFilmings[windowIndex]=cbData->set;
	if(active&&getWindow(windowIndex)!=0)
		{
		if(windowFilmings[windowIndex])
			{
			/* Set the window's viewer to the filming viewer: */
			getWindow(windowIndex)->setViewer(viewer);
			}
		else
			{
			/* Reset the window's viewers to the original values: */
			for(int i=0;i<2;++i)
				getWindow(windowIndex)->setViewer(i,windowViewers[windowIndex*2+i]);
			}
		}
	}

void Filming::headlightToggleCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData,const int& viewerIndex)
	{
	headlightStates[viewerIndex]=cbData->set;
	
	if(active)
		{
		/* Set the headlight to the new state: */
		if(viewerIndex==0)
			viewer->setHeadlightState(cbData->set);
		else
			getViewer(viewerIndex-1)->setHeadlightState(cbData->set);
		}
	}

void Filming::backgroundColorSelectorCallback(GLMotif::HSVColorSelector::ValueChangedCallbackData* cbData)
	{
	backgroundColor=cbData->newColor;
	
	if(active)
		{
		/* Set the environment's background color: */
		setBackgroundColor(backgroundColor);
		}
	}

void Filming::drawGridToggleCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	drawGrid=cbData->set;
	}

void Filming::resetGridCallback(Misc::CallbackData* cbData)
	{
	if(gridDragger==0)
		{
		/* Initialize the grid transformation: */
		gridTransform=ONTransform::translateFromOriginTo(getDisplayCenter());
		gridTransform*=ONTransform::rotate(Rotation::fromBaseVectors(getUpDirection()^getForwardDirection(),getUpDirection()));
		}
	}

void Filming::drawDevicesToggleCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	drawDevices=cbData->set;
	}

void Filming::loadSettingsCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData)
	{
	Misc::ConfigurationFile settingsFile;
	if(isMaster())
		{
		try
			{
			/* Open the selected settings file: */
			settingsFile.load(cbData->getSelectedPath().c_str());
			
			if(getMainPipe()!=0)
				{
				/* Send the settings file to the slave nodes: */
				Misc::writeCString(0,*getMainPipe());
				settingsFile.writeToPipe(*getMainPipe());
				}
			}
		catch(std::runtime_error err)
			{
			/* Send an error message to the slave nodes: */
			if(getMainPipe()!=0)
				Misc::writeCString(err.what(),*getMainPipe());
			
			/* Re-throw the exception: */
			throw;
			}
		}
	else
		{
		/* Receive the selected settings file from the master node: */
		Misc::SelfDestructArray<char> error(Misc::readCString(*getMainPipe()));
		if(error.getArray()==0)
			settingsFile.readFromPipe(*getMainPipe());
		else
			{
			/* Throw an exception: */
			throw std::runtime_error(error.getArray());
			}
		}
	
	/* Load all filming settings: */
	/* Get the viewer device: */
	std::string viewerDeviceName=settingsFile.retrieveValue<std::string>("./viewerDevice");
	viewerDevice=0;
	int viewerDeviceIndex=0;
	for(int i=1;viewerDeviceIndex==0&&i<viewerDeviceMenu->getNumItems();++i)
		if(viewerDeviceName==viewerDeviceMenu->getItem(i))
			{
			viewerDevice=findInputDevice(viewerDeviceMenu->getItem(i));
			if(viewerDevice!=0)
				viewerDeviceIndex=i;
			}
	viewerDeviceMenu->setSelectedItem(viewerDeviceIndex);
	
	/* Read the fixed-position viewer position and head-tracked eye position: */
	viewerPosition=settingsFile.retrieveValue<Point>("./viewerPosition");
	eyePosition=settingsFile.retrieveValue<Point>("./eyePosition");
	
	/* Update the viewer GUI: */
	changeViewerMode();
	
	/* Read the window flags: */
	std::vector<bool> windowFilmingsVector=settingsFile.retrieveValue<std::vector<bool> >("./windowFilmingFlags");
	for(int i=0;i<getNumWindows()&&size_t(i)<windowFilmingsVector.size();++i)
		{
		windowFilmings[i]=windowFilmingsVector[i];
		static_cast<GLMotif::ToggleButton*>(windowButtonBox->getChild(i))->setToggle(windowFilmings[i]);
		}
	
	/* Read the headlight states: */
	std::vector<bool> headlightStatesVector=settingsFile.retrieveValue<std::vector<bool> >("./headlightStates");
	for(int i=0;i<getNumViewers()+1&&size_t(i)<headlightStatesVector.size();++i)
		{
		headlightStates[i]=headlightStatesVector[i];
		static_cast<GLMotif::ToggleButton*>(headlightButtonBox->getChild(i))->setToggle(headlightStates[i]);
		}
	
	/* Read the background color: */
	backgroundColor=settingsFile.retrieveValue<Color>("./backgroundColor");
	backgroundColorSelector->setCurrentColor(backgroundColor);
	
	/* Read the grid state: */
	drawGrid=settingsFile.retrieveValue<bool>("./drawGrid");
	drawGridToggle->setToggle(drawGrid);
	gridTransform=settingsFile.retrieveValue<ONTransform>("./gridTransform");
	
	/* Read the device drawing flag: */
	drawDevices=settingsFile.retrieveValue<bool>("./drawDevices");
	drawDevicesToggle->setToggle(drawDevices);
	
	if(active)
		{
		/* Update the current environment state: */
		for(int windowIndex=0;windowIndex<getNumWindows();++windowIndex)
			if(getWindow(windowIndex)!=0)
				{
				if(windowFilmings[windowIndex])
					{
					/* Set the window's viewer to the filming viewer: */
					getWindow(windowIndex)->setViewer(viewer);
					}
				else
					{
					/* Reset the window's viewers to the original values: */
					for(int i=0;i<2;++i)
						getWindow(windowIndex)->setViewer(i,windowViewers[windowIndex*2+i]);
					}
				}
		viewer->setHeadlightState(headlightStates[0]);
		for(int i=0;i<getNumViewers();++i)
			getViewer(i)->setHeadlightState(headlightStates[i+1]);
		setBackgroundColor(backgroundColor);
		}
	}

void Filming::saveSettingsCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData)
	{
	if(isMaster())
		{
		try
			{
			/* Store the current filming settings: */
			Misc::ConfigurationFile settingsFile;
			settingsFile.storeValue<std::string>("./viewerDevice",viewerDevice!=0?viewerDevice->getDeviceName():"Fixed Position");
			settingsFile.storeValue<Point>("./viewerPosition",viewerPosition);
			settingsFile.storeValue<Point>("./eyePosition",eyePosition);
			std::vector<bool> windowFilmingsVector;
			for(int i=0;i<getNumWindows();++i)
				windowFilmingsVector.push_back(windowFilmings[i]);
			settingsFile.storeValue<std::vector<bool> >("./windowFilmingFlags",windowFilmingsVector);
			std::vector<bool> headlightStatesVector;
			for(int i=0;i<getNumViewers()+1;++i)
				headlightStatesVector.push_back(headlightStates[i]);
			settingsFile.storeValue<std::vector<bool> >("./headlightStates",headlightStatesVector);
			settingsFile.storeValue<Color>("./backgroundColor",backgroundColor);
			settingsFile.storeValue<bool>("./drawGrid",drawGrid);
			settingsFile.storeValue<ONTransform>("./gridTransform",gridTransform);
			settingsFile.storeValue<bool>("./drawDevices",drawDevices);
			
			/* Write the settings file: */
			settingsFile.saveAs(cbData->getSelectedPath().c_str());
			
			if(getMainPipe()!=0)
				{
				/* Send a status message to the slave nodes: */
				Misc::writeCString(0,*getMainPipe());
				}
			}
		catch(std::runtime_error err)
			{
			if(getMainPipe()!=0)
				{
				/* Send an error message to the slaves: */
				Misc::writeCString(err.what(),*getMainPipe());
				}
			
			/* Re-throw the exception: */
			throw;
			}
		}
	else
		{
		/* Receive a status message from the master node: */
		Misc::SelfDestructArray<char> error(Misc::readCString(*getMainPipe()));
		if(error.getArray()!=0)
			{
			/* Throw an exception: */
			throw std::runtime_error(error.getArray());
			}
		}
	}

void Filming::buildFilmingControls(void)
	{
	/* Build the graphical user interface: */
	const GLMotif::StyleSheet& ss=*getUiStyleSheet();
	
	dialogWindow=new GLMotif::PopupWindow("FilmingControlDialog",getWidgetManager(),"Filming Controls");
	dialogWindow->setHideButton(true);
	dialogWindow->setResizableFlags(true,false);
	
	GLMotif::RowColumn* filmingControls=new GLMotif::RowColumn("FilmingControls",dialogWindow,false);
	filmingControls->setOrientation(GLMotif::RowColumn::VERTICAL);
	filmingControls->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	filmingControls->setNumMinorWidgets(2);
	
	/* Create a drop-down menu to select a tracking device for the filming viewer: */
	new GLMotif::Label("ViewerDeviceLabel",filmingControls,"Viewer Device");
	viewerDeviceMenu=new GLMotif::DropdownBox("ViewerDeviceMenu",filmingControls);
	viewerDeviceMenu->addItem("Fixed Position");
	for(int deviceIndex=0;deviceIndex<getNumInputDevices();++deviceIndex)
		if(getInputGraphManager()->isReal(getInputDevice(deviceIndex)))
			viewerDeviceMenu->addItem(getInputDevice(deviceIndex)->getDeviceName());
	viewerDeviceMenu->setSelectedItem(0);
	viewerDeviceMenu->getValueChangedCallbacks().add(this,&Filming::viewerDeviceMenuCallback);
	
	/* Create three sliders to set the filming viewer's position: */
	new GLMotif::Label("ViewerPositionLabel",filmingControls,"Viewer Position");
	GLMotif::RowColumn* viewerPositionBox=new GLMotif::RowColumn("ViewerPositionBox",filmingControls,false);
	
	for(int i=0;i<3;++i)
		{
		char psName[11]="PosSlider ";
		psName[9]=char(i+'0');
		posSliders[i]=new GLMotif::TextFieldSlider(psName,viewerPositionBox,7,ss.fontHeight*10.0f);
		posSliders[i]->getTextField()->setFieldWidth(6);
		posSliders[i]->getTextField()->setPrecision(1);
		posSliders[i]->getTextField()->setFloatFormat(GLMotif::TextField::FIXED);
		posSliders[i]->setSliderMapping(GLMotif::TextFieldSlider::LINEAR);
		posSliders[i]->setValueType(GLMotif::TextFieldSlider::FLOAT);
		posSliders[i]->getValueChangedCallbacks().add(this,&Filming::posSliderCallback,i);
		}
	
	viewerPositionBox->manageChild();
	
	/* Update the viewer tracking GUI: */
	changeViewerMode();
	
	/* Create toggle buttons to select filming windows: */
	new GLMotif::Label("WindowButtonLabel",filmingControls,"Filming Windows");
	windowButtonBox=new GLMotif::RowColumn("WindowButtonBox",filmingControls,false);
	windowButtonBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	windowButtonBox->setPacking(GLMotif::RowColumn::PACK_GRID);
	windowButtonBox->setAlignment(GLMotif::Alignment::LEFT);
	windowButtonBox->setNumMinorWidgets(1);
	
	for(int windowIndex=0;windowIndex<getNumWindows();++windowIndex)
		{
		char windowToggleName[15]="WindowToggle  ";
		char* wtnPtr=Misc::print(windowIndex,windowToggleName+14);
		while(wtnPtr>windowToggleName+12)
			*(wtnPtr--)='0';
		char windowToggleLabel[3];
		GLMotif::ToggleButton* windowToggle=new GLMotif::ToggleButton(windowToggleName,windowButtonBox,Misc::print(windowIndex+1,windowToggleLabel+2));
		windowToggle->setToggle(windowFilmings[windowIndex]);
		windowToggle->getValueChangedCallbacks().add(this,&Filming::windowToggleCallback,windowIndex);
		}
	
	windowButtonBox->manageChild();
	
	/* Create toggle buttons to toggle viewer headlights: */
	new GLMotif::Label("HeadlightButtonLabel",filmingControls,"Headlights");
	headlightButtonBox=new GLMotif::RowColumn("HeadlightButtonBox",filmingControls,false);
	headlightButtonBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	headlightButtonBox->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	headlightButtonBox->setAlignment(GLMotif::Alignment::LEFT);
	headlightButtonBox->setNumMinorWidgets(1);
	
	for(int viewerIndex=0;viewerIndex<getNumViewers()+1;++viewerIndex)
		{
		char headlightToggleName[18]="HeadlightToggle  ";
		char* htnPtr=Misc::print(viewerIndex,headlightToggleName+17);
		while(htnPtr>headlightToggleName+15)
			*(htnPtr--)='0';
		Viewer* v=viewerIndex==0?viewer:getViewer(viewerIndex-1);
		GLMotif::ToggleButton* headlightToggle=new GLMotif::ToggleButton(headlightToggleName,headlightButtonBox,viewerIndex==0?"FilmingViewer":v->getName());
		headlightToggle->setToggle(v->getHeadlight().isEnabled());
		headlightToggle->getValueChangedCallbacks().add(this,&Filming::headlightToggleCallback,viewerIndex);
		}
	
	headlightButtonBox->manageChild();
	
	/* Create a color selector to change the environment's background color: */
	new GLMotif::Label("BackgroundColorLabel",filmingControls,"Background Color");
	GLMotif::Margin* backgroundColorMargin=new GLMotif::Margin("BackgroundColorMargin",filmingControls,false);
	backgroundColorMargin->setAlignment(GLMotif::Alignment::LEFT);
	
	backgroundColorSelector=new GLMotif::HSVColorSelector("BackgroundColorSelector",backgroundColorMargin);
	backgroundColorSelector->setPreferredSize(ss.fontHeight*4.0f);
	backgroundColorSelector->setCurrentColor(backgroundColor);
	backgroundColorSelector->getValueChangedCallbacks().add(this,&Filming::backgroundColorSelectorCallback);
	
	backgroundColorMargin->manageChild();
	
	/* Create toggles for various flags: */
	new GLMotif::Blind("ToggleBoxBlind",filmingControls);
	GLMotif::RowColumn* toggleBox=new GLMotif::RowColumn("ToggleBox",filmingControls,false);
	toggleBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	toggleBox->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	toggleBox->setAlignment(GLMotif::Alignment::LEFT);
	toggleBox->setNumMinorWidgets(1);
	
	drawGridToggle=new GLMotif::ToggleButton("DrawGridToggle",toggleBox,"Draw Grid");
	drawGridToggle->setToggle(drawGrid);
	drawGridToggle->getValueChangedCallbacks().add(this,&Filming::drawGridToggleCallback);
	
	GLMotif::Button* resetGridButton=new GLMotif::Button("ResetGridButton",toggleBox,"Reset Grid");
	resetGridButton->getSelectCallbacks().add(this,&Filming::resetGridCallback);
	
	drawDevicesToggle=new GLMotif::ToggleButton("DrawDevicesToggle",toggleBox,"Draw Devices");
	drawDevicesToggle->setToggle(drawDevices);
	drawDevicesToggle->getValueChangedCallbacks().add(this,&Filming::drawDevicesToggleCallback);
	
	toggleBox->manageChild();
	
	/* Create buttons to load or save the vislet's current state: */
	new GLMotif::Blind("IOBoxBlind",filmingControls);
	GLMotif::RowColumn* ioBox=new GLMotif::RowColumn("IOBox",filmingControls,false);
	ioBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	ioBox->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	ioBox->setAlignment(GLMotif::Alignment::LEFT);
	ioBox->setNumMinorWidgets(1);
	
	GLMotif::Button* loadSettingsButton=new GLMotif::Button("loadSettingsButton",ioBox,"Load Settings...");
	settingsSelectionHelper.addLoadCallback(loadSettingsButton,this,&Filming::loadSettingsCallback);
	
	GLMotif::Button* saveSettingsButton=new GLMotif::Button("saveSettingsButton",ioBox,"Save Settings...");
	settingsSelectionHelper.addSaveCallback(saveSettingsButton,this,&Filming::saveSettingsCallback);
	
	ioBox->manageChild();
	
	filmingControls->manageChild();
	}

void Filming::toolCreationCallback(ToolManager::ToolCreationCallbackData* cbData)
	{
	/* Check if the new tool is a filming tool: */
	Tool* tool=dynamic_cast<Tool*>(cbData->tool);
	if(tool!=0)
		{
		/* Associate the tool with this vislet: */
		tool->setVislet(this);
		}
	}

Filming::Filming(int numArguments,const char* const arguments[])
	:viewer(0),viewerDevice(0),viewerPosition(factory->initialViewerPosition),eyePosition(Point::origin),
	 windowViewers(0),windowFilmings(0),
	 originalHeadlightStates(0),headlightStates(0),
	 drawGrid(false),gridDragger(0),
	 drawDevices(false),
	 dialogWindow(0),
	 settingsSelectionHelper("SavedFilmingSettings.cfg",".cfg",openDirectory("."))
	{
	/* Create the private filming viewer: */
	viewer=new Viewer;
	viewer->setHeadlightState(false);
	
	/* Initialize the grid transformation: */
	resetGridCallback(0);
	
	/* Install callbacks with the tool manager: */
	getToolManager()->getToolCreationCallbacks().add(this,&Filming::toolCreationCallback);
	}

Filming::~Filming(void)
	{
	delete dialogWindow;
	
	/* Uninstall tool manager callbacks: */
	getToolManager()->getToolCreationCallbacks().remove(this,&Filming::toolCreationCallback);
	
	delete viewer;
	delete[] windowViewers;
	delete[] windowFilmings;
	delete[] originalHeadlightStates;
	delete[] headlightStates;
	}

VisletFactory* Filming::getFactory(void) const
	{
	return factory;
	}

void Filming::disable(void)
	{
	/* Reset the viewers of all filming windows: */
	for(int windowIndex=0;windowIndex<getNumWindows();++windowIndex)
		if(windowFilmings[windowIndex]&&getWindow(windowIndex)!=0)
			{
			for(int i=0;i<2;++i)
				getWindow(windowIndex)->setViewer(i,windowViewers[windowIndex*2+i]);
			}
	
	/* Reset all viewer's headlight states: */
	viewer->setHeadlightState(false);
	for(int viewerIndex=0;viewerIndex<getNumViewers();++viewerIndex)
		getViewer(viewerIndex)->setHeadlightState(originalHeadlightStates[viewerIndex]);
	
	/* Restore the environment's background color: */
	setBackgroundColor(originalBackgroundColor);
	
	active=false;
	}

void Filming::enable(void)
	{
	/* Check if the filming control GUI needs to be created: */
	if(dialogWindow==0)
		{
		/* Initialize the filming viewer associations: */
		windowViewers=new Viewer*[getNumWindows()*2];
		windowFilmings=new bool[getNumWindows()];
		for(int windowIndex=0;windowIndex<getNumWindows();++windowIndex)
			windowFilmings[windowIndex]=true;
		
		/* Initialize the headlight state arrays: */
		originalHeadlightStates=new bool[getNumViewers()];
		for(int viewerIndex=0;viewerIndex<getNumViewers();++viewerIndex)
			originalHeadlightStates[viewerIndex]=getViewer(viewerIndex)->getHeadlight().isEnabled();
		headlightStates=new bool[getNumViewers()+1];
		headlightStates[0]=viewer->getHeadlight().isEnabled();
		for(int viewerIndex=0;viewerIndex<getNumViewers();++viewerIndex)
			headlightStates[viewerIndex+1]=originalHeadlightStates[viewerIndex];
		
		/* Save the environment's background color: */
		originalBackgroundColor=getBackgroundColor();
		backgroundColor=originalBackgroundColor;
		
		/* Build and show the GUI: */
		buildFilmingControls();
		popupPrimaryWidget(dialogWindow);
		}
	else
		{
		/* Store the viewers currently attached to each window and override the viewers of filming windows: */
		for(int windowIndex=0;windowIndex<getNumWindows();++windowIndex)
			{
			if(getWindow(windowIndex)!=0)
				{
				for(int i=0;i<2;++i)
					windowViewers[windowIndex*2+i]=getWindow(windowIndex)->getViewer(i);
				if(windowFilmings[windowIndex])
					getWindow(windowIndex)->setViewer(viewer);
				}
			else
				windowViewers[windowIndex*2+0]=windowViewers[windowIndex*2+1]=0;
			}
		
		/* Set all viewer's headlight states: */
		viewer->setHeadlightState(headlightStates[0]);
		for(int viewerIndex=0;viewerIndex<getNumViewers();++viewerIndex)
			getViewer(viewerIndex)->setHeadlightState(headlightStates[viewerIndex+1]);
		
		/* Override the environment's background color: */
		setBackgroundColor(backgroundColor);
		
		active=true;
		}
	}

void Filming::frame(void)
	{
	/* Update the filming viewer: */
	viewer->update();
	}

void Filming::display(GLContextData& contextData) const
	{
	glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
	glDisable(GL_LIGHTING);
	glLineWidth(1.0f);
	
	if(drawGrid)
		{
		/* Draw the calibration grid: */
		glPushMatrix();
		glMultMatrix(gridTransform);
		
		float gs=float(getDisplaySize())*3.0f;
		glColor3f(1.0f,1.0f,0.0f);
		glBegin(GL_LINES);
		for(int x=-8;x<=8;++x)
			{
			glVertex3f(float(x)*gs*0.125f,-gs,0.0f);
			glVertex3f(float(x)*gs*0.125f,gs,0.0f);
			}
		for(int y=-8;y<=8;++y)
			{
			glVertex3f(-gs,float(y)*gs*0.125f,0.0f);
			glVertex3f(gs,float(y)*gs*0.125f,0.0f);
			}
		glEnd();
		
		glPopMatrix();
		}
	
	if(drawDevices)
		{
		/* Draw coordinate axes for each real device: */
		for(int i=0;i<Vrui::getNumInputDevices();++i)
			{
			Vrui::InputDevice* id=Vrui::getInputDevice(i);
			if(id->is6DOFDevice()&&getInputGraphManager()->isReal(id))
				{
				glPushMatrix();
				glMultMatrix(id->getTransformation());
				glScale(getInchFactor());
				glBegin(GL_LINES);
				glColor3f(1.0f,0.0f,0.0f);
				glVertex3f(-5.0f,0.0f,0.0f);
				glVertex3f( 5.0f,0.0f,0.0f);
				glColor3f(0.0f,1.0f,0.0f);
				glVertex3f(0.0f,-5.0f,0.0f);
				glVertex3f(0.0f, 5.0f,0.0f);
				glColor3f(0.0f,0.0f,1.0f);
				glVertex3f(0.0f,0.0f,-5.0f);
				glVertex3f(0.0f,0.0f, 5.0f);
				glEnd();
				glPopMatrix();
				}
			}
		}
	
	glPopAttrib();
	}

}

}
