/***********************************************************************
Application - Base class for Vrui application objects.
Copyright (c) 2004-2013 Oliver Kreylos

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

#include <string.h>
#include <Misc/PrintInteger.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Vrui/Geometry.h>
#include <Vrui/Vrui.h>

#include <Vrui/Application.h>

namespace Vrui {

/****************************
Methods of class Application:
****************************/

void Application::frameWrapper(void* userData)
	{
	static_cast<Application*>(userData)->frame();
	}

void Application::displayWrapper(GLContextData& contextData,void* userData)
	{
	static_cast<Application*>(userData)->display(contextData);
	}

void Application::soundWrapper(ALContextData& contextData,void* userData)
	{
	static_cast<Application*>(userData)->sound(contextData);
	}

char* Application::createEventToolClassName(void)
	{
	char numberBuffer[5];
	char* number=Misc::print(nextEventToolClassIndex,numberBuffer+4);
	++nextEventToolClassIndex;
	static const char etcnPrefix[]="VruiApplicationEventToolClass";
	size_t etcnpLen=strlen(etcnPrefix);
	char* result=new char[etcnpLen+((numberBuffer+4)-number)+1];
	strcpy(result,etcnPrefix);
	strcpy(result+etcnpLen,number);
	return result;
	}

void Application::addEventTool(const char* toolName,ToolFactory* parentClass,Application::EventID eventId)
	{
	typedef EventToolFactory<Application> ETF;
	
	/* Create a new event tool factory class: */
	char* toolClassName=createEventToolClassName();
	ETF* toolFactory=new ETF(toolClassName,toolName,parentClass,this,&Application::eventCallback,eventId);
	delete[] toolClassName;
	
	/* Register the tool factory class with the tool manager: */
	getToolManager()->addClass(toolFactory,ToolManager::defaultToolFactoryDestructor);
	}

Application::Application(int& argc,char**& argv,char**& appDefaults)
	:nextEventToolClassIndex(0)
	{
	/* Initialize Vrui: */
	init(argc,argv,appDefaults);
	
	/* Install callbacks with the tool manager: */
	ToolManager* toolManager=getToolManager();
	toolManager->getToolCreationCallbacks().add(this,&Application::toolCreationCallback);
	toolManager->getToolDestructionCallbacks().add(this,&Application::toolDestructionCallback);
	
	/* Enable navigation per default: */
	setNavigationTransformation(NavTransform::identity);
	}

Application::Application(int& argc,char**& argv)
	:nextEventToolClassIndex(0)
	{
	/* Initialize Vrui: */
	char** appDefaults=0;
	init(argc,argv,appDefaults);
	
	/* Install callbacks with the tool manager: */
	ToolManager* toolManager=getToolManager();
	toolManager->getToolCreationCallbacks().add(this,&Application::toolCreationCallback);
	toolManager->getToolDestructionCallbacks().add(this,&Application::toolDestructionCallback);
	
	/* Enable navigation per default: */
	setNavigationTransformation(NavTransform::identity);
	}

Application::~Application(void)
	{
	/* Uninstall tool manager callbacks: */
	ToolManager* toolManager=getToolManager();
	toolManager->getToolCreationCallbacks().remove(this,&Application::toolCreationCallback);
	toolManager->getToolDestructionCallbacks().remove(this,&Application::toolDestructionCallback);
	
	/* Deinitialize Vrui: */
	deinit();
	}

void Application::run(void)
	{
	/* Install Vrui callbacks: */
	setFrameFunction(frameWrapper,this);
	setDisplayFunction(displayWrapper,this);
	setSoundFunction(soundWrapper,this);
	
	/* Run the Vrui main loop: */
	mainLoop();
	}

void Application::toolCreationCallback(ToolManager::ToolCreationCallbackData* cbData)
	{
	/* Check if the new tool is an application tool: */
	ToolBase* applicationTool=dynamic_cast<ToolBase*>(cbData->tool);
	if(applicationTool!=0)
		{
		/* Set the application tool's application pointer: */
		applicationTool->setApplication(this);
		}
	
	/* Check if the new tool is an event tool: */
	EventToolBase* eventTool=dynamic_cast<EventToolBase*>(cbData->tool);
	const EventToolFactoryBase* eventToolFactory=dynamic_cast<const EventToolFactoryBase*>(cbData->tool->getFactory());
	if(eventTool!=0&&eventToolFactory!=0)
		{
		/* Call the event tool creation callback: */
		eventToolCreationCallback(eventToolFactory->getEventId(),cbData);
		}
	}

void Application::toolDestructionCallback(ToolManager::ToolDestructionCallbackData* cbData)
	{
	/* Check if the destroyed tool is an event tool: */
	EventToolBase* eventTool=dynamic_cast<EventToolBase*>(cbData->tool);
	const EventToolFactoryBase* eventToolFactory=dynamic_cast<const EventToolFactoryBase*>(cbData->tool->getFactory());
	if(eventTool!=0&&eventToolFactory!=0)
		{
		/* Call the event tool destruction callback: */
		eventToolDestructionCallback(eventToolFactory->getEventId(),cbData);
		}
	}

void Application::frame(void)
	{
	}

void Application::display(GLContextData&) const
	{
	}

void Application::sound(ALContextData&) const
	{
	}

void Application::eventToolCreationCallback(Application::EventID eventId,ToolManager::ToolCreationCallbackData* cbData)
	{
	}

void Application::eventToolDestructionCallback(Application::EventID eventId,ToolManager::ToolDestructionCallbackData* cbData)
	{
	}

void Application::eventCallback(Application::EventID eventId,InputDevice::ButtonCallbackData* cbData)
	{
	}

}
