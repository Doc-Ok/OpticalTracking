/***********************************************************************
VruiEventToolDemo - VR application showing how to handle simple events
(key or mouse button presses) using the "event tool" mechanism.
Copyright (c) 2012-2013 Oliver Kreylos

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

#include <Geometry/Point.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Geometry/OutputOperators.h>
#include <GL/gl.h>
#include <GL/GLMaterialTemplates.h>
#include <GL/GLModels.h>
#include <Vrui/Vrui.h>
#include <Vrui/Application.h>
#include <Vrui/InputDevice.h>

class VruiEventToolDemo:public Vrui::Application
	{
	/* Elements: */
	private:
	int drawMode; // Draw mode index: 0: draw nothing, 1: draw cube, 2: draw sphere, 3: draw cube and sphere
	bool useSpecular; // Flag whether to make materials specular
	int shininess; // Current shininess for specular highlights
	
	/* Private methods: */
	void changeDrawMode(Vrui::Application::EventID eventId,Vrui::InputDevice::ButtonCallbackData* cbData); // Method called when an event is received
	void changeMaterial(Vrui::Application::EventID eventId,Vrui::InputDevice::ButtonCallbackData* cbData); // Method called when an event is received
	
	/* Constructors and destructors: */
	public:
	VruiEventToolDemo(int& argc,char**& argv);
	
	/* Methods from Vrui::Application: */
	virtual void display(GLContextData& contextData) const;
	virtual void eventCallback(EventID eventId,Vrui::InputDevice::ButtonCallbackData* cbData);
	};

/**********************************
Methods of class VruiEventToolDemo:
**********************************/

void VruiEventToolDemo::changeDrawMode(Vrui::Application::EventID,Vrui::InputDevice::ButtonCallbackData* cbData)
	{
	/* Check if the event was a button press (as opposed to a button release): */
	if(cbData->newButtonState)
		{
		/* Go to the next draw mode: */
		drawMode=(drawMode+1)%4;
		}
	}

void VruiEventToolDemo::changeMaterial(Vrui::Application::EventID eventId,Vrui::InputDevice::ButtonCallbackData* cbData)
	{
	/* Check if the event was a button press (as opposed to a button release): */
	if(cbData->newButtonState)
		{
		switch(eventId)
			{
			case 0:
				/* Switch specular setting: */
				useSpecular=!useSpecular;
				break;
			
			case 1:
				/* Change shininess: */
				shininess=(shininess+10)%60;
				break;
			}
		}
	}

VruiEventToolDemo::VruiEventToolDemo(int& argc,char**& argv)
	:Vrui::Application(argc,argv),
	 drawMode(0x3),useSpecular(false),shininess(20)
	{
	/* Create an event tool class to change the draw mode using a dedicated callback method: */
	addEventTool("Change Draw Mode",0,this,&VruiEventToolDemo::changeDrawMode,0);
	
	/* Create two event tool classes to change material properties using a shared callback method: */
	addEventTool("Toggle Specular",0,this,&VruiEventToolDemo::changeMaterial,0);
	addEventTool("Change Shininess",0,this,&VruiEventToolDemo::changeMaterial,1);
	
	/* Add three more event tool classes using the virtual event callback method mechanism: */
	addEventTool("Dummy Event 0",0,0);
	addEventTool("Dummy Event 1",0,1);
	addEventTool("Dummy Event 2",0,2);
	
	/* Set the navigation transformation to show the entire scene: */
	Vrui::setNavigationTransformation(Vrui::Point::origin,Vrui::Scalar(12));
	}

void VruiEventToolDemo::display(GLContextData& contextData) const
	{
	/* Draw a red cube and a blue sphere: */
	glPushMatrix();
	
	if(useSpecular)
		{
		glMaterialSpecular(GLMaterialEnums::FRONT,GLColor<GLfloat,4>(1.0f,1.0f,1.0f));
		glMaterialShininess(GLMaterialEnums::FRONT,GLfloat(shininess));
		}
	else
		{
		glMaterialSpecular(GLMaterialEnums::FRONT,GLColor<GLfloat,4>(0.0f,0.0f,0.0f));
		glMaterialShininess(GLMaterialEnums::FRONT,0.0f);
		}
	
	glTranslated(-5.0,0.0,0.0);
	if(drawMode&0x1)
		{
		/* Draw the cube: */
		glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT,GLColor<GLfloat,4>(1.0f,0.5f,0.5f));
		glDrawCube(7.5f);
		}
	
	if(drawMode&0x2)
		{
		/* Draw the sphere: */
		glTranslated(10.0,0.0,0.0);
		glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT,GLColor<GLfloat,4>(0.5f,0.5f,1.0f));
		glDrawSphereIcosahedron(4.5f,6);
		}
	
	glPopMatrix();
	}

void VruiEventToolDemo::eventCallback(Vrui::Application::EventID eventId,Vrui::InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState)
		{
		switch(eventId)
			{
			case 0:
				Vrui::showErrorMessage("VruiEventToolDemo","Dummy event 0 occurred");
				break;
			
			case 1:
				Vrui::showErrorMessage("VruiEventToolDemo","Dummy event 1 occurred");
				break;
			
			case 2:
				Vrui::showErrorMessage("VruiEventToolDemo","Dummy event 2 occurred");
				break;
			}
		
		/* Print the navigation space position where the event occurred: */
		std::cout<<Vrui::getInverseNavigationTransformation().transform(cbData->inputDevice->getPosition())<<std::endl;
		}
	}

VRUI_APPLICATION_RUN(VruiEventToolDemo)
