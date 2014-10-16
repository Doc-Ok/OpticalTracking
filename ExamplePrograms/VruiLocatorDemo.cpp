/***********************************************************************
VruiLocatorDemo - VR application showing how to use locator tools in
Vrui.
Copyright (c) 2006-2013 Oliver Kreylos

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <vector>
#include <iostream>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/gl.h>
#include <GLMotif/Button.h>
#include <GLMotif/Menu.h>
#include <GLMotif/PopupMenu.h>
#include <Vrui/LocatorTool.h>
#include <Vrui/LocatorToolAdapter.h>
#include <Vrui/ToolManager.h>
#include <Vrui/Vrui.h>
#include <Vrui/Application.h>

class VruiLocatorDemo:public Vrui::Application
	{
	/* Embedded classes: */
	private:
	class Locator:public Vrui::LocatorToolAdapter // Class containing application-specific locator behavior
		{
		/* Elements: */
		private:
		VruiLocatorDemo* application; // Pointer to the application "owning" this locator
		bool active; // Flag if the locator is active (button is pressed)
		
		/* Constructors and destructors: */
		public:
		Locator(Vrui::LocatorTool* sTool,VruiLocatorDemo* sApplication);
		
		/* Methods: */
		virtual void motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData);
		virtual void buttonPressCallback(Vrui::LocatorTool::ButtonPressCallbackData* cbData);
		virtual void buttonReleaseCallback(Vrui::LocatorTool::ButtonReleaseCallbackData* cbData);
		};
	
	typedef std::vector<Locator*> LocatorList; // Type for lists of locators
	
	/* Elements: */
	private:
	LocatorList locators; // List of all locators
	
	/* Vrui parameters: */
	GLMotif::PopupMenu* mainMenu; // The program's main menu
	
	/* Private methods: */
	GLMotif::PopupMenu* createMainMenu(void); // Creates the program's main menu
	
	/* Constructors and destructors: */
	public:
	VruiLocatorDemo(int& argc,char**& argv); // Initializes the Vrui toolkit and the application
	virtual ~VruiLocatorDemo(void); // Shuts down the Vrui toolkit
	
	/* Methods: */
	virtual void toolCreationCallback(Vrui::ToolManager::ToolCreationCallbackData* cbData); // Called when a new tool is created
	virtual void toolDestructionCallback(Vrui::ToolManager::ToolDestructionCallbackData* cbData); // Called when a tool is destroyed
	virtual void display(GLContextData& contextData) const; // Called for every eye and every window on every frame
	void resetNavigationCallback(Misc::CallbackData* cbData); // Method to reset the Vrui navigation transformation to its default
	};

/*****************************************
Methods of class VruiLocatorDemo::Locator:
*****************************************/

VruiLocatorDemo::Locator::Locator(Vrui::LocatorTool* sTool,VruiLocatorDemo* sApplication)
	:Vrui::LocatorToolAdapter(sTool),
	 application(sApplication),
	 active(false)
	{
	}

void VruiLocatorDemo::Locator::motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData)
	{
	if(active)
		{
		/* Get the locator's new position and orientation in navigation coordinates: */
		Vrui::NavTrackerState coord=cbData->currentTransformation;
		
		/* Print the locator's position: */
		Vrui::Point pos=coord.getOrigin();
		std::cout<<"Locator position: ("<<pos[0]<<", "<<pos[1]<<", "<<pos[2]<<")"<<std::endl;
		}
	}

void VruiLocatorDemo::Locator::buttonPressCallback(Vrui::LocatorTool::ButtonPressCallbackData* cbData)
	{
	/* Activate the locator: */
	active=true;
	}

void VruiLocatorDemo::Locator::buttonReleaseCallback(Vrui::LocatorTool::ButtonReleaseCallbackData* cbData)
	{
	/* Deactivate the locator: */
	active=false;
	}

/********************************
Methods of class VruiLocatorDemo:
********************************/

GLMotif::PopupMenu* VruiLocatorDemo::createMainMenu(void)
	{
	/* Create a popup shell to hold the main menu: */
	GLMotif::PopupMenu* mainMenuPopup=new GLMotif::PopupMenu("MainMenuPopup",Vrui::getWidgetManager());
	mainMenuPopup->setTitle("Vrui Demonstration");
	
	/* Create the main menu itself: */
	GLMotif::Menu* mainMenu=new GLMotif::Menu("MainMenu",mainMenuPopup,false);
	
	/* Create a button: */
	GLMotif::Button* resetNavigationButton=new GLMotif::Button("ResetNavigationButton",mainMenu,"Reset Navigation");
	
	/* Add a callback to the button: */
	resetNavigationButton->getSelectCallbacks().add(this,&VruiLocatorDemo::resetNavigationCallback);
	
	/* Finish building the main menu: */
	mainMenu->manageChild();
	
	return mainMenuPopup;
	}

VruiLocatorDemo::VruiLocatorDemo(int& argc,char**& argv)
	:Vrui::Application(argc,argv),
	 mainMenu(0)
	{
	/* Create the user interface: */
	mainMenu=createMainMenu();
	
	/* Install the main menu: */
	Vrui::setMainMenu(mainMenu);
	
	/* Set the navigation transformation: */
	resetNavigationCallback(0);
	}

VruiLocatorDemo::~VruiLocatorDemo(void)
	{
	delete mainMenu;
	}

void VruiLocatorDemo::toolCreationCallback(Vrui::ToolManager::ToolCreationCallbackData* cbData)
	{
	/* Check if the new tool is a locator tool: */
	Vrui::LocatorTool* locatorTool=dynamic_cast<Vrui::LocatorTool*>(cbData->tool);
	if(locatorTool!=0)
		{
		/* Create a new locator and associate it with the new tool and this application: */
		Locator* locator=new Locator(locatorTool,this);
		
		/* Store the new locator in the list: */
		locators.push_back(locator);
		}
	}

void VruiLocatorDemo::toolDestructionCallback(Vrui::ToolManager::ToolDestructionCallbackData* cbData)
	{
	/* Check if the destroyed tool is a locator tool: */
	Vrui::LocatorTool* locatorTool=dynamic_cast<Vrui::LocatorTool*>(cbData->tool);
	if(locatorTool!=0)
		{
		/* Find the locator in the list: */
		for(LocatorList::iterator lIt=locators.begin();lIt!=locators.end();++lIt)
			if((*lIt)->getTool()==locatorTool)
				{
				/* Remove the locator: */
				delete *lIt;
				locators.erase(lIt);
				break;
				}
		}
	}

void VruiLocatorDemo::display(GLContextData& contextData) const
	{
	/* Set up OpenGL state: */
	glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
	glDisable(GL_LIGHTING);
	glLineWidth(1.0f);
	
	/* Draw something: */
	glBegin(GL_LINE_STRIP);
	glVertex3f(-10.0f,-10.0f,-10.0f);
	glVertex3f( 10.0f,-10.0f,-10.0f);
	glVertex3f( 10.0f, 10.0f,-10.0f);
	glVertex3f(-10.0f, 10.0f,-10.0f);
	glVertex3f(-10.0f,-10.0f,-10.0f);
	glVertex3f(-10.0f,-10.0f, 10.0f);
	glVertex3f( 10.0f,-10.0f, 10.0f);
	glVertex3f( 10.0f, 10.0f, 10.0f);
	glVertex3f(-10.0f, 10.0f, 10.0f);
	glVertex3f(-10.0f,-10.0f, 10.0f);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f( 10.0f,-10.0f,-10.0f);
	glVertex3f( 10.0f,-10.0f, 10.0f);
	glVertex3f( 10.0f, 10.0f,-10.0f);
	glVertex3f( 10.0f, 10.0f, 10.0f);
	glVertex3f(-10.0f, 10.0f,-10.0f);
	glVertex3f(-10.0f, 10.0f, 10.0f);
	glEnd();
	
	/* Reset OpenGL state: */
	glPopAttrib();
	}

void VruiLocatorDemo::resetNavigationCallback(Misc::CallbackData* cbData)
	{
	/* Reset the Vrui navigation transformation: */
	Vrui::NavTransform t=Vrui::NavTransform::identity;
	t*=Vrui::NavTransform::translateFromOriginTo(Vrui::getDisplayCenter());
	t*=Vrui::NavTransform::scale(Vrui::getInchFactor());
	Vrui::setNavigationTransformation(t);
	}

VRUI_APPLICATION_RUN(VruiLocatorDemo)
