/***********************************************************************
Jello - VR program to interact with "virtual Jell-O" using a simplified
force interaction model based on the Nanotech Construction Kit.
Copyright (c) 2006-2013 Oliver Kreylos

This file is part of the Virtual Jell-O interactive VR demonstration.

Virtual Jell-O is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

Virtual Jell-O is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with Virtual Jell-O; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include "Jello.h"

#include <stdlib.h>
#include <vector>
#include <Math/Math.h>
#include <GL/gl.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/PopupMenu.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/Menu.h>
#include <GLMotif/Label.h>
#include <GLMotif/Button.h>
#include <Vrui/Vrui.h>

/***********************************
Methods of class Jello::AtomDragger:
***********************************/

Jello::AtomDragger::AtomDragger(Vrui::DraggingTool* sTool,Jello* sApplication)
	:Vrui::DraggingToolAdapter(sTool),
	 application(sApplication),
	 dragging(false)
	{
	}

void Jello::AtomDragger::dragStartCallback(Vrui::DraggingTool::DragStartCallbackData* cbData)
	{
	/* Find the picked atom: */
	if(cbData->rayBased)
		draggedAtom=application->crystal.pickAtom(cbData->ray);
	else
		draggedAtom=application->crystal.pickAtom(cbData->startTransformation.getOrigin());
	
	/* Try locking the atom: */
	if(application->crystal.lockAtom(draggedAtom))
		{
		dragging=true;
		
		/* Calculate the initial transformation from the dragger to the dragged atom: */
		dragTransform=ONTransform(cbData->startTransformation.getTranslation(),cbData->startTransformation.getRotation());
		dragTransform.doInvert();
		dragTransform*=application->crystal.getAtomState(draggedAtom);
		}
	}

void Jello::AtomDragger::dragCallback(Vrui::DraggingTool::DragCallbackData* cbData)
	{
	if(dragging)
		{
		/* Apply the dragging transformation to the dragged atom: */
		ONTransform transform=ONTransform(cbData->currentTransformation.getTranslation(),cbData->currentTransformation.getRotation());
		transform*=dragTransform;
		application->crystal.setAtomState(draggedAtom,transform);
		}
	}

void Jello::AtomDragger::dragEndCallback(Vrui::DraggingTool::DragEndCallbackData* cbData)
	{
	if(dragging)
		{
		/* Release the previously dragged atom: */
		application->crystal.unlockAtom(draggedAtom);
		dragging=false;
		}
	}

/**********************
Methods of class Jello:
**********************/

GLMotif::PopupMenu* Jello::createMainMenu(void)
	{
	GLMotif::PopupMenu* mainMenuPopup=new GLMotif::PopupMenu("MainMenuPopup",Vrui::getWidgetManager());
	mainMenuPopup->setTitle("Virtual Jell-O");
	
	GLMotif::Menu* mainMenu=new GLMotif::Menu("MainMenu",mainMenuPopup,false);
	
	GLMotif::Button* centerDisplayButton=new GLMotif::Button("CenterDisplayButton",mainMenu,"Center Display");
	centerDisplayButton->getSelectCallbacks().add(this,&Jello::centerDisplayCallback);
	
	showSettingsDialogToggle=new GLMotif::ToggleButton("ShowSettingsDialogToggle",mainMenu,"Show Settings Dialog");
	showSettingsDialogToggle->getValueChangedCallbacks().add(this,&Jello::showSettingsDialogCallback);
	
	mainMenu->manageChild();
	
	return mainMenuPopup;
	}

GLMotif::PopupWindow* Jello::createSettingsDialog(void)
	{
	const GLMotif::StyleSheet& ss=*Vrui::getWidgetManager()->getStyleSheet();
	
	settingsDialog=new GLMotif::PopupWindow("SettingsDialog",Vrui::getWidgetManager(),"Settings Dialog");
	settingsDialog->setCloseButton(true);
	settingsDialog->setResizableFlags(true,false);
	settingsDialog->getCloseCallbacks().add(this,&Jello::settingsDialogCloseCallback);
	
	GLMotif::RowColumn* settings=new GLMotif::RowColumn("Settings",settingsDialog,false);
	settings->setNumMinorWidgets(2);
	
	new GLMotif::Label("JigglinessLabel",settings,"Jiggliness");
	
	jigglinessSlider=new GLMotif::TextFieldSlider("JigglinessSlider",settings,5,ss.fontHeight*10.0f);
	jigglinessSlider->getTextField()->setFloatFormat(GLMotif::TextField::FIXED);
	jigglinessSlider->getTextField()->setFieldWidth(4);
	jigglinessSlider->getTextField()->setPrecision(2);
	jigglinessSlider->setValueRange(0.0,1.0,0.01);
	jigglinessSlider->setValue((Math::log(double(crystal.getAtomMass()))/Math::log(1.1)+32.0)/64.0);
	jigglinessSlider->getValueChangedCallbacks().add(this,&Jello::jigglinessSliderCallback);
	
	new GLMotif::Label("ViscosityLabel",settings,"Viscosity");
	
	viscositySlider=new GLMotif::TextFieldSlider("ViscositySlider",settings,5,ss.fontHeight*10.0f);
	viscositySlider->getTextField()->setFloatFormat(GLMotif::TextField::FIXED);
	viscositySlider->getTextField()->setFieldWidth(4);
	viscositySlider->getTextField()->setPrecision(2);
	viscositySlider->setValueRange(0.0,1.0,0.01);
	viscositySlider->setValue(1.0-double(crystal.getAttenuation()));
	viscositySlider->getValueChangedCallbacks().add(this,&Jello::viscositySliderCallback);
	
	new GLMotif::Label("GravityLabel",settings,"Gravity");
	
	gravitySlider=new GLMotif::TextFieldSlider("GravitySlider",settings,5,ss.fontHeight*10.0f);
	gravitySlider->getTextField()->setFloatFormat(GLMotif::TextField::FIXED);
	gravitySlider->getTextField()->setFieldWidth(4);
	gravitySlider->getTextField()->setPrecision(1);
	gravitySlider->setValueRange(0.0,40.0,0.5);
	gravitySlider->setValue(double(crystal.getGravity()));
	gravitySlider->getValueChangedCallbacks().add(this,&Jello::gravitySliderCallback);
	
	settings->manageChild();
	
	return settingsDialog;
	}

Jello::Jello(int& argc,char**& argv)
	:Vrui::Application(argc,argv),
	 crystal(JelloCrystal::Index(4,4,8)),
	 renderer(crystal),
	 targetFrameRate(50.0),
	 numMiniSteps(1),
	 mainMenu(0),settingsDialog(0)
	{
	/* Target frame rate is only (optional) command line parameter: */
	if(argc>=2)
		targetFrameRate=atof(argv[1]);
	
	/* Determine a good color to draw the domain box: */
	GLColor<GLfloat,3> domainBoxColor;
	for(int i=0;i<3;++i)
		domainBoxColor[i]=1.0f-Vrui::getBackgroundColor()[i];
	renderer.setDomainBoxColor(domainBoxColor);
	
	/* Create the program's user interface: */
	mainMenu=createMainMenu();
	Vrui::setMainMenu(mainMenu);
	settingsDialog=createSettingsDialog();
	
	/* Initialize the navigation transformation: */
	centerDisplayCallback(0);
	
	/* Tell Vrui to run in a continuous frame sequence: */
	Vrui::updateContinuously();
	
	/* Initialize the frame time calculator: */
	lastFrameTime=Vrui::getApplicationTime();
	}

Jello::~Jello(void)
	{
	/* Delete all atom draggers: */
	for(AtomDraggerList::iterator adIt=atomDraggers.begin();adIt!=atomDraggers.end();++adIt)
		delete *adIt;
	
	/* Delete the user interface: */
	delete mainMenu;
	delete settingsDialog;
	}

void Jello::toolCreationCallback(Vrui::ToolManager::ToolCreationCallbackData* cbData)
	{
	/* Check if the new tool is a dragging tool: */
	Vrui::DraggingTool* tool=dynamic_cast<Vrui::DraggingTool*>(cbData->tool);
	if(tool!=0)
		{
		/* Create an atom dragger object and associate it with the new tool: */
		AtomDragger* newDragger=new AtomDragger(tool,this);
		
		/* Add new dragger to list: */
		atomDraggers.push_back(newDragger);
		}
	}

void Jello::toolDestructionCallback(Vrui::ToolManager::ToolDestructionCallbackData* cbData)
	{
	/* Check if the to-be-destroyed tool is a dragging tool: */
	Vrui::DraggingTool* tool=dynamic_cast<Vrui::DraggingTool*>(cbData->tool);
	if(tool!=0)
		{
		/* Find the atom dragger associated with the tool in the list: */
		AtomDraggerList::iterator adIt;
		for(adIt=atomDraggers.begin();adIt!=atomDraggers.end()&&(*adIt)->getTool()!=tool;++adIt)
			;
		if(adIt!=atomDraggers.end())
			{
			/* Remove the atom dragger: */
			delete *adIt;
			atomDraggers.erase(adIt);
			}
		}
	}

void Jello::frame(void)
	{
	/* Calculate the current frame time: */
	double newFrameTime=Vrui::getApplicationTime();
	Scalar timeStep=Scalar(newFrameTime-lastFrameTime);
	lastFrameTime=newFrameTime;
	
	/* Adjust the number of mini steps based on the target frame rate: */
	if(timeStep>1.0/(targetFrameRate+0.5)&&numMiniSteps>1)
		--numMiniSteps;
	else if(timeStep<1.0/(targetFrameRate-0.5))
		++numMiniSteps;
	
	/* Simulate the mini steps: */
	timeStep/=Scalar(numMiniSteps);
	for(int i=0;i<numMiniSteps;++i)
		crystal.simulate(timeStep);
	
	/* Update the Jell-O renderer: */
	renderer.update();
	}

void Jello::display(GLContextData& contextData) const
	{
	/* Render the Jell-O crystal: */
	renderer.glRenderAction(contextData);
	}

void Jello::centerDisplayCallback(Misc::CallbackData* cbData)
	{
	Vrui::setNavigationTransformation(Vrui::NavTransform::identity);
	}

void Jello::showSettingsDialogCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	/* Hide or show settings dialog based on toggle button state: */
	if(cbData->set)
		{
		/* Pop up the settings dialog: */
		Vrui::popupPrimaryWidget(settingsDialog);
		}
	else
		Vrui::popdownPrimaryWidget(settingsDialog);
	}

void Jello::jigglinessSliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData)
	{
	/* Compute and set the atom mass: */
	double atomMass=Math::exp(Math::log(1.1)*(cbData->value*64.0-32.0));
	crystal.setAtomMass(atomMass);
	}

void Jello::viscositySliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData)
	{
	/* Set the attenuation: */
	crystal.setAttenuation(Scalar(1.0-cbData->value));
	}

void Jello::gravitySliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData)
	{
	/* Set the gravity: */
	crystal.setGravity(Scalar(cbData->value));
	}

void Jello::settingsDialogCloseCallback(Misc::CallbackData* cbData)
	{
	showSettingsDialogToggle->setToggle(false);
	}

/* Create and execute an application object: */
VRUI_APPLICATION_RUN(Jello)
