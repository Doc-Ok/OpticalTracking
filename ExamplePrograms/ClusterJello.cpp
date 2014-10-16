/***********************************************************************
ClusterJello - VR program to interact with "virtual Jell-O" using a
simplified force interaction model based on the Nanotech Construction
Kit. This version of Virtual Jell-O uses multithreading and explicit
cluster communication to split the computation work and rendering work
between the CPUs and nodes of a distributed rendering cluster.
Copyright (c) 2007-2013 Oliver Kreylos

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

#include "ClusterJello.h"

#include <stdlib.h>
#include <vector>
#include <Misc/Timer.h>
#include <Cluster/MulticastPipe.h>
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
#include <Vrui/ClusterSupport.h>

/*******************************************
Methods of class ClusterJello::DraggerState:
*******************************************/

void ClusterJello::DraggerStates::setNumDraggers(int newNumDraggers)
	{
	/* Check if the numbers are actually different: */
	if(numDraggers!=newNumDraggers)
		{
		/* Delete the old arrays: */
		delete[] draggerIDs;
		delete[] draggerRayBaseds;
		delete[] draggerRays;
		delete[] draggerTransformations;
		delete[] draggerActives;
		
		/* Allocate new arrays: */
		numDraggers=newNumDraggers;
		draggerIDs=numDraggers>0?new unsigned int[numDraggers]:0;
		draggerRayBaseds=numDraggers>0?new bool[numDraggers]:0;
		draggerRays=numDraggers>0?new Ray[numDraggers]:0;
		draggerTransformations=numDraggers>0?new ONTransform[numDraggers]:0;
		draggerActives=numDraggers>0?new bool[numDraggers]:0;
		}
	}

/******************************************
Methods of class ClusterJello::AtomDragger:
******************************************/

ClusterJello::AtomDragger::AtomDragger(Vrui::DraggingTool* sTool,ClusterJello* sApplication,unsigned int sDraggerID)
	:Vrui::DraggingToolAdapter(sTool),
	 application(sApplication),
	 draggerID(sDraggerID),
	 draggerRayBased(false),
	 active(false)
	{
	}

void ClusterJello::AtomDragger::idleMotionCallback(Vrui::DraggingTool::IdleMotionCallbackData* cbData)
	{
	/* Update the dragger position: */
	draggerTransformation=ONTransform(cbData->currentTransformation.getTranslation(),cbData->currentTransformation.getRotation());
	}

void ClusterJello::AtomDragger::dragStartCallback(Vrui::DraggingTool::DragStartCallbackData* cbData)
	{
	/* Store the dragger's selection ray if it is ray-based: */
	draggerRayBased=cbData->rayBased;
	if(draggerRayBased)
		draggerRay=cbData->ray;
	
	/* Activate this dragger: */
	active=true;
	}

void ClusterJello::AtomDragger::dragCallback(Vrui::DraggingTool::DragCallbackData* cbData)
	{
	/* Update the dragger position: */
	draggerTransformation=ONTransform(cbData->currentTransformation.getTranslation(),cbData->currentTransformation.getRotation());
	}

void ClusterJello::AtomDragger::dragEndCallback(Vrui::DraggingTool::DragEndCallbackData* cbData)
	{
	/* Deactivate this dragger: */
	active=false;
	}

/*****************************
Methods of class ClusterJello:
*****************************/

GLMotif::PopupMenu* ClusterJello::createMainMenu(void)
	{
	GLMotif::PopupMenu* mainMenuPopup=new GLMotif::PopupMenu("MainMenuPopup",Vrui::getWidgetManager());
	mainMenuPopup->setTitle("Virtual Jell-O");
	
	GLMotif::Menu* mainMenu=new GLMotif::Menu("MainMenu",mainMenuPopup,false);
	
	GLMotif::Button* centerDisplayButton=new GLMotif::Button("CenterDisplayButton",mainMenu,"Center Display");
	centerDisplayButton->getSelectCallbacks().add(this,&ClusterJello::centerDisplayCallback);
	
	showSettingsDialogToggle=new GLMotif::ToggleButton("ShowSettingsDialogToggle",mainMenu,"Show Settings Dialog");
	showSettingsDialogToggle->getValueChangedCallbacks().add(this,&ClusterJello::showSettingsDialogCallback);
	
	mainMenu->manageChild();
	
	return mainMenuPopup;
	}

GLMotif::PopupWindow* ClusterJello::createSettingsDialog(void)
	{
	const GLMotif::StyleSheet& ss=*Vrui::getWidgetManager()->getStyleSheet();
	
	settingsDialog=new GLMotif::PopupWindow("SettingsDialog",Vrui::getWidgetManager(),"Settings Dialog");
	settingsDialog->setCloseButton(true);
	settingsDialog->setResizableFlags(true,false);
	settingsDialog->getCloseCallbacks().add(this,&ClusterJello::settingsDialogCloseCallback);
	
	GLMotif::RowColumn* settings=new GLMotif::RowColumn("Settings",settingsDialog,false);
	settings->setNumMinorWidgets(2);
	
	new GLMotif::Label("JigglinessLabel",settings,"Jiggliness");
	
	jigglinessSlider=new GLMotif::TextFieldSlider("JigglinessSlider",settings,5,ss.fontHeight*10.0f);
	jigglinessSlider->getTextField()->setFloatFormat(GLMotif::TextField::FIXED);
	jigglinessSlider->getTextField()->setFieldWidth(4);
	jigglinessSlider->getTextField()->setPrecision(2);
	jigglinessSlider->setValueRange(0.0,1.0,0.01);
	jigglinessSlider->setValue((Math::log(double(currentSimulationParameters.atomMass))/Math::log(1.1)+32.0)/64.0);
	jigglinessSlider->getValueChangedCallbacks().add(this,&ClusterJello::jigglinessSliderCallback);
	
	new GLMotif::Label("ViscosityLabel",settings,"Viscosity");
	
	viscositySlider=new GLMotif::TextFieldSlider("ViscositySlider",settings,5,ss.fontHeight*10.0f);
	viscositySlider->getTextField()->setFloatFormat(GLMotif::TextField::FIXED);
	viscositySlider->getTextField()->setFieldWidth(4);
	viscositySlider->getTextField()->setPrecision(2);
	viscositySlider->setValueRange(0.0,1.0,0.01);
	viscositySlider->setValue(1.0-double(currentSimulationParameters.attenuation));
	viscositySlider->getValueChangedCallbacks().add(this,&ClusterJello::viscositySliderCallback);
	
	new GLMotif::Label("GravityLabel",settings,"Gravity");
	
	gravitySlider=new GLMotif::TextFieldSlider("GravitySlider",settings,5,ss.fontHeight*10.0f);
	gravitySlider->getTextField()->setFloatFormat(GLMotif::TextField::FIXED);
	gravitySlider->getTextField()->setFieldWidth(4);
	gravitySlider->getTextField()->setPrecision(1);
	gravitySlider->setValueRange(0.0,40.0,0.5);
	gravitySlider->setValue(double(currentSimulationParameters.gravity));
	gravitySlider->getValueChangedCallbacks().add(this,&ClusterJello::gravitySliderCallback);
	
	settings->manageChild();
	
	return settingsDialog;
	}

void* ClusterJello::simulationThreadMethodMaster(void)
	{
	/* Enable immediate cancellation of this thread: */
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	/* Simulate the crystal state until interrupted: */
	Misc::Timer timer;
	double lastFrameTime=timer.peekTime();
	double nextUpdateTime=timer.peekTime()+updateTime;
	while(true)
		{
		/* Calculate the current time step duration: */
		double newFrameTime=timer.peekTime();
		double timeStep=newFrameTime-lastFrameTime;
		lastFrameTime=newFrameTime;
		
		/* Check if the simulation parameters have been changed: */
		if(simulationParameters.hasNewValue())
			{
			/* Update the Jell-O crystal's simulation parameters: */
			const SimulationParameters& sp=simulationParameters.lockNewValue();
			crystal->setAtomMass(sp.atomMass);
			crystal->setAttenuation(sp.attenuation);
			crystal->setGravity(sp.gravity);
			}
		
		/* Check if the application has delivered new dragger states: */
		if(draggerStates.hasNewValue())
			{
			/* Process the new dragger states: */
			const DraggerStates& ds=draggerStates.lockNewValue();
			for(int draggerIndex=0;draggerIndex<ds.numDraggers;++draggerIndex)
				{
				if(ds.draggerActives[draggerIndex])
					{
					/* Check if this dragger has just become active: */
					if(!atomLocks.isEntry(ds.draggerIDs[draggerIndex]))
						{
						/* Find the atom picked by the dragger: */
						AtomLock al;
						if(ds.draggerRayBaseds[draggerIndex])
							al.draggedAtom=crystal->pickAtom(ds.draggerRays[draggerIndex]);
						else
							al.draggedAtom=crystal->pickAtom(ds.draggerTransformations[draggerIndex].getOrigin());
						
						/* Try locking the atom: */
						if(crystal->lockAtom(al.draggedAtom))
							{
							/* Calculate the dragging transformation: */
							al.dragTransformation=ds.draggerTransformations[draggerIndex];
							al.dragTransformation.doInvert();
							al.dragTransformation*=crystal->getAtomState(al.draggedAtom);
							
							/* Store the atom lock in the hash table: */
							atomLocks.setEntry(AtomLockHasher::Entry(ds.draggerIDs[draggerIndex],al));
							}
						}
					
					/* Check if the dragger has an atom lock: */
					AtomLockHasher::Iterator alIt=atomLocks.findEntry(ds.draggerIDs[draggerIndex]);
					if(!alIt.isFinished())
						{
						/* Set the position/orientation of the locked atom: */
						ONTransform transform=ds.draggerTransformations[draggerIndex];
						transform*=alIt->getDest().dragTransformation;
						crystal->setAtomState(alIt->getDest().draggedAtom,transform);
						}
					}
				else
					{
					/* Check if this dragger has just become inactive: */
					AtomLockHasher::Iterator alIt=atomLocks.findEntry(ds.draggerIDs[draggerIndex]);
					if(!alIt.isFinished())
						{
						/* Release the atom lock: */
						crystal->unlockAtom(alIt->getDest().draggedAtom);
						atomLocks.removeEntry(alIt);
						}
					}
				}
			}
		
		/* Advance the simulation time by the last frame time: */
		crystal->simulate(timeStep);
		
		/* Update the application's Jell-O state if the update interval is over: */
		if(lastFrameTime>=nextUpdateTime)
			{
			if(clusterPipe!=0)
				{
				/* Broadcast the crystal state to all slave nodes: */
				crystal->writeAtomStates(*clusterPipe);
				clusterPipe->flush();
				}
			
			/* Update the application's proxy crystal state: */
			JelloCrystal& pc=proxyCrystal.startWrite();
			pc.copyAtomStates(*crystal);
			proxyCrystal.finishWrite();
			Vrui::requestUpdate();
			
			/* Start the next update interval: */
			nextUpdateTime+=updateTime;
			}
		}
	
	return 0;
	}

void* ClusterJello::simulationThreadMethodSlave(void)
	{
	/* Enable immediate cancellation of this thread: */
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	/* Receive crystal state updates from the master node until interrupted: */
	while(true)
		{
		/* Receive the next crystal state update from the master and write it into the application's proxy crystal state: */
		JelloCrystal& pc=proxyCrystal.startWrite();
		pc.readAtomStates(*clusterPipe);
		proxyCrystal.finishWrite();
		Vrui::requestUpdate();
		}
	
	return 0;
	}

ClusterJello::ClusterJello(int& argc,char**& argv)
	:Vrui::Application(argc,argv),
	 clusterPipe(Vrui::openPipe()),
	 crystal(0),
	 atomLocks(17),
	 updateTime(0.02),
	 renderer(0),
	 mainMenu(0),settingsDialog(0),
	 nextDraggerID(0)
	{
	/* Target frame rate is only (optional) command line parameter: */
	if(argc>=2)
		updateTime=1.0/atof(argv[1]);
	
	/* Initialize the proxy crystal states: */
	for(int i=0;i<3;++i)
		proxyCrystal.accessSlot(i).setNumAtoms(JelloCrystal::Index(4,4,8));
	
	/* Initialize the crystal renderer: */
	renderer=new JelloRenderer(proxyCrystal.getLockedValue());
	
	/* Determine a good color to draw the domain box: */
	GLColor<GLfloat,3> domainBoxColor;
	for(int i=0;i<3;++i)
		domainBoxColor[i]=1.0f-Vrui::getBackgroundColor()[i];
	renderer->setDomainBoxColor(domainBoxColor);
	
	if(Vrui::isMaster())
		{
		/* Initialize the simulated Jell-O crystal: */
		crystal=new JelloCrystal(JelloCrystal::Index(4,4,8));
		currentSimulationParameters.atomMass=crystal->getAtomMass();
		currentSimulationParameters.attenuation=crystal->getAttenuation();
		currentSimulationParameters.gravity=crystal->getGravity();
		Vrui::write(clusterPipe,currentSimulationParameters.atomMass);
		Vrui::write(clusterPipe,currentSimulationParameters.attenuation);
		Vrui::write(clusterPipe,currentSimulationParameters.gravity);
		Vrui::flush(clusterPipe);
		
		/* Start the simulation thread: */
		simulationThread.start(this,&ClusterJello::simulationThreadMethodMaster);
		}
	else
		{
		/* Receive the initial simulation parameters: */
		currentSimulationParameters.atomMass=clusterPipe->read<Scalar>();
		currentSimulationParameters.attenuation=clusterPipe->read<Scalar>();
		currentSimulationParameters.gravity=clusterPipe->read<Scalar>();
		
		/* Start the simulation thread: */
		simulationThread.start(this,&ClusterJello::simulationThreadMethodSlave);
		}
	
	/* Create the program's user interface: */
	mainMenu=createMainMenu();
	Vrui::setMainMenu(mainMenu);
	settingsDialog=createSettingsDialog();
	
	/* Initialize the navigation transformation: */
	centerDisplayCallback(0);
	}

ClusterJello::~ClusterJello(void)
	{
	/* Delete all atom draggers: */
	for(AtomDraggerList::iterator adIt=atomDraggers.begin();adIt!=atomDraggers.end();++adIt)
		delete *adIt;
	
	/* Delete the user interface: */
	delete mainMenu;
	delete settingsDialog;
	
	/* Shut down the simulation thread: */
	simulationThread.cancel();
	simulationThread.join();
	if(Vrui::isMaster())
		delete crystal;
	
	delete renderer;
	
	/* Shut down cluster communication: */
	delete clusterPipe;
	}

void ClusterJello::toolCreationCallback(Vrui::ToolManager::ToolCreationCallbackData* cbData)
	{
	/* Check if the new tool is a dragging tool: */
	Vrui::DraggingTool* tool=dynamic_cast<Vrui::DraggingTool*>(cbData->tool);
	if(tool!=0)
		{
		/* Create an atom dragger object and associate it with the new tool: */
		AtomDragger* newDragger=new AtomDragger(tool,this,nextDraggerID);
		++nextDraggerID;
		
		/* Add new dragger to list: */
		atomDraggers.push_back(newDragger);
		}
	}

void ClusterJello::toolDestructionCallback(Vrui::ToolManager::ToolDestructionCallbackData* cbData)
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

void ClusterJello::frame(void)
	{
	/* Send the current states of all draggers to the simulation thread: */
	DraggerStates& ds=draggerStates.startWrite();
	ds.setNumDraggers(atomDraggers.size());
	for(int i=0;i<ds.numDraggers;++i)
		{
		const AtomDragger* ad=atomDraggers[i];
		ds.draggerIDs[i]=ad->draggerID;
		ds.draggerRayBaseds[i]=ad->draggerRayBased;
		ds.draggerRays[i]=ad->draggerRay;
		ds.draggerTransformations[i]=ad->draggerTransformation;
		ds.draggerActives[i]=ad->active;
		}
	draggerStates.finishWrite();
	
	/* Check if the simulation thread has delivered a new crystal state: */
	if(proxyCrystal.hasNewValue())
		{
		/* Update the Jell-O renderer to draw the new crystal state: */
		renderer->setCrystal(&proxyCrystal.lockNewValue());
		renderer->update();
		}
	}

void ClusterJello::display(GLContextData& contextData) const
	{
	/* Render the Jell-O crystal: */
	renderer->glRenderAction(contextData);
	}

void ClusterJello::centerDisplayCallback(Misc::CallbackData* cbData)
	{
	Vrui::setNavigationTransformation(Vrui::NavTransform::identity);
	}

void ClusterJello::showSettingsDialogCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	/* Hide or show settings dialog based on toggle button state: */
	if(cbData->set)
		{
		/* Pop up the settings dialog at the same position as the main menu: */
		Vrui::popupPrimaryWidget(settingsDialog);
		}
	else
		Vrui::popdownPrimaryWidget(settingsDialog);
	}

void ClusterJello::jigglinessSliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData)
	{
	/* Compute and set the atom mass: */
	currentSimulationParameters.atomMass=Scalar(Math::exp(Math::log(1.1)*(cbData->value*64.0-32.0)));
	
	/* Update the simulation parameters (only relevant on the master node): */
	simulationParameters.write(currentSimulationParameters);
	}

void ClusterJello::viscositySliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData)
	{
	/* Set the attenuation: */
	currentSimulationParameters.attenuation=Scalar(1.0-cbData->value);
	
	/* Update the simulation parameters (only relevant on the master node): */
	simulationParameters.write(currentSimulationParameters);
	}

void ClusterJello::gravitySliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData)
	{
	/* Set the gravity: */
	currentSimulationParameters.gravity=Scalar(cbData->value);
	
	/* Update the simulation parameters (only relevant on the master node): */
	simulationParameters.write(currentSimulationParameters);
	}

void ClusterJello::settingsDialogCloseCallback(Misc::CallbackData* cbData)
	{
	showSettingsDialogToggle->setToggle(false);
	}

/* Create and execute an application object: */
VRUI_APPLICATION_RUN(ClusterJello)
