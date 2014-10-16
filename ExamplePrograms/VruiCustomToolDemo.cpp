/***********************************************************************
VruiCustomToolDemo - VR application showing how to create application-
specific tools and register them with the Vrui tool manager, and how
custom tools can interact with the VR application.
Copyright (c) 2006-2012 Oliver Kreylos

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

#include <iostream>
#include <Vrui/Tool.h>
#include <Vrui/GenericToolFactory.h>
#include <Vrui/ToolManager.h>
#include <Vrui/Vrui.h>
#include <Vrui/Application.h>

class VruiCustomToolDemo:public Vrui::Application
	{
	/* Embedded classes: */
	private:
	class MyTool; // Forward declaration
	typedef Vrui::GenericToolFactory<MyTool> MyToolFactory; // Tool class uses the generic factory class
	
	class MyTool:public Vrui::Tool,public Vrui::Application::Tool<VruiCustomToolDemo> // The custom tool class, derived from application tool class
		{
		friend class Vrui::GenericToolFactory<MyTool>;
		
		/* Elements: */
		private:
		static MyToolFactory* factory; // Pointer to the factory object for this class
		
		/* Constructors and destructors: */
		public:
		MyTool(const Vrui::ToolFactory* factory,const Vrui::ToolInputAssignment& inputAssignment);
		
		/* Methods: */
		virtual const Vrui::ToolFactory* getFactory(void) const;
		virtual void buttonCallback(int buttonSlotIndex,Vrui::InputDevice::ButtonCallbackData* cbData);
		};
	
	/* Constructors and destructors: */
	public:
	VruiCustomToolDemo(int& argc,char**& argv);
	
	/* Methods: */
	void selectApplicationObject(void); // Dummy method to show how custom tools can interact with the application
	};

/***************************************************
Static elements of class VruiCustomToolDemo::MyTool:
***************************************************/

VruiCustomToolDemo::MyToolFactory* VruiCustomToolDemo::MyTool::factory=0;

/*******************************************
Methods of class VruiCustomToolDemo::MyTool:
*******************************************/

VruiCustomToolDemo::MyTool::MyTool(const Vrui::ToolFactory* factory,const Vrui::ToolInputAssignment& inputAssignment)
	:Vrui::Tool(factory,inputAssignment)
	{
	}

const Vrui::ToolFactory* VruiCustomToolDemo::MyTool::getFactory(void) const
	{
	return factory;
	}

void VruiCustomToolDemo::MyTool::buttonCallback(int buttonSlotIndex,Vrui::InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState) // Button has just been pressed
		{
		std::cout<<"MyTool: Button "<<buttonSlotIndex<<" has just been pressed"<<std::endl;
		
		/* Call an application method if the second button was pressed: */
		if(buttonSlotIndex==1)
			application->selectApplicationObject();
		}
	else // Button has just been released
		{
		std::cout<<"MyTool: Button "<<buttonSlotIndex<<" has just been released"<<std::endl;
		}
	}

/***********************************
Methods of class VruiCustomToolDemo:
***********************************/

VruiCustomToolDemo::VruiCustomToolDemo(int& argc,char**& argv)
	:Vrui::Application(argc,argv)
	{
	/* Create a factory object for the custom tool class: */
	MyToolFactory* myToolFactory=new MyToolFactory("MyTool","Demo Application Tool",0,*Vrui::getToolManager());
	
	/* Set the custom tool class' input layout: */
	myToolFactory->setNumButtons(2,true); // Needs two buttons and can take optional buttons
	myToolFactory->setButtonFunction(0,"Does nothing");
	myToolFactory->setButtonFunction(1,"Select Application Object");
	myToolFactory->setButtonFunction(2,"Optional Button");
	
	/* Register the custom tool class with the Vrui tool manager: */
	Vrui::getToolManager()->addClass(myToolFactory,Vrui::ToolManager::defaultToolFactoryDestructor);
	}

void VruiCustomToolDemo::selectApplicationObject(void)
	{
	std::cout<<"VruiCustomToolDemo: selectApplicationObject has just been called"<<std::endl;
	}

/* Create and execute an application object: */
VRUI_APPLICATION_RUN(VruiCustomToolDemo)
