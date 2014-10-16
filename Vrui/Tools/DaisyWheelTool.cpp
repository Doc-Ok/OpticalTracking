/***********************************************************************
DaisyWheelTool - Class for tools to enter text by pointing at characters
on a dynamic daisy wheel.
Copyright (c) 2008-2010 Oliver Kreylos

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

#include <Vrui/Tools/DaisyWheelTool.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <GL/gl.h>
#include <GL/GLFont.h>
#include <GL/GLLabel.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <GLMotif/TextEvent.h>
#include <GLMotif/TextControlEvent.h>
#include <GLMotif/WidgetManager.h>
#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/**************************************
Methods of class DaisyWheelToolFactory:
**************************************/

DaisyWheelToolFactory::DaisyWheelToolFactory(ToolManager& toolManager)
	:ToolFactory("DaisyWheelTool",toolManager),
	 petalSize(getUiFont()->getTextHeight()),
	 innerRadius(petalSize*Scalar(1)),outerRadius(petalSize*Scalar(7)),
	 maxYOffset(getUiSize())
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* toolFactory=toolManager.loadClass("UserInterfaceTool");
	toolFactory->addChildClass(this);
	addParentClass(toolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	innerRadius=cfs.retrieveValue<Scalar>("./innerRadius",innerRadius);
	outerRadius=cfs.retrieveValue<Scalar>("./outerRadius",outerRadius);
	maxYOffset=cfs.retrieveValue<Scalar>("./maxYOffset",maxYOffset);
	
	/* Calculate the angle spanned by a petal label: */
	maxPetalAngle=Scalar(2)*Math::tan(Math::sqrt(Scalar(2))*petalSize/(innerRadius+outerRadius));
	
	/* Set tool class' factory pointer: */
	DaisyWheelTool::factory=this;
	}

DaisyWheelToolFactory::~DaisyWheelToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	DaisyWheelTool::factory=0;
	}

const char* DaisyWheelToolFactory::getName(void) const
	{
	return "Daisy Wheel";
	}

Tool* DaisyWheelToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new DaisyWheelTool(this,inputAssignment);
	}

void DaisyWheelToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveDaisyWheelToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("UserInterfaceTool");
	}

extern "C" ToolFactory* createDaisyWheelToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	DaisyWheelToolFactory* daisyWheelToolFactory=new DaisyWheelToolFactory(*toolManager);
	
	/* Return factory object: */
	return daisyWheelToolFactory;
	}

extern "C" void destroyDaisyWheelToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/***************************************
Static elements of class DaisyWheelTool:
***************************************/

DaisyWheelToolFactory* DaisyWheelTool::factory=0;

namespace {

/****************
Helper functions:
****************/

inline Scalar wrapAngle(Scalar angle)
	{
	if(angle<-Math::Constants<Scalar>::pi)
		angle+=Scalar(2)*Math::Constants<Scalar>::pi;
	if(angle>Math::Constants<Scalar>::pi)
		angle-=Scalar(2)*Math::Constants<Scalar>::pi;
	return angle;
	}

}

/*******************************
Methods of class DaisyWheelTool:
*******************************/

Scalar DaisyWheelTool::calcPetalAngle(Scalar petal) const
	{
	Scalar deltaAngle=wrapAngle((petal-Scalar(selectedPetal))*angleStep);
	if(deltaAngle>=Scalar(0))
		deltaAngle=Math::pow(deltaAngle/Math::Constants<Scalar>::pi,zoomStrength)*Math::Constants<Scalar>::pi;
	else
		deltaAngle=-Math::pow(-deltaAngle/Math::Constants<Scalar>::pi,zoomStrength)*Math::Constants<Scalar>::pi;
	
	return selectedAngle+deltaAngle;
	}

DaisyWheelTool::DaisyWheelTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:UserInterfaceTool(factory,inputAssignment),
	 numPetals(94),angleStep(Scalar(2)*Math::Constants<Scalar>::pi/Scalar(numPetals)),
	 petals(new GLLabel[numPetals]),
	 active(false),buttonDown(false),
	 selectedPetal(0)
	{
	/* Set the interaction device: */
	interactionDevice=getButtonDevice(0);
	
	/* Initialize the petal labels: */
	for(int i=0;i<26;++i)
		{
		char string[2];
		string[0]='A'+i;
		string[1]='\0';
		petals[i].setString(string,*getUiFont());
		}
	for(int i=0;i<26;++i)
		{
		char string[2];
		string[0]='a'+i;
		string[1]='\0';
		petals[i+26].setString(string,*getUiFont());
		}
	for(int i=0;i<10;++i)
		{
		char string[2];
		string[0]='0'+i;
		string[1]='\0';
		petals[i+52].setString(string,*getUiFont());
		}
	static const char puncts[]=",./<>?;:'\"[]{}\\|`~!@#$%^&*()-_=+";
	for(int i=0;i<32;++i)
		{
		char string[2];
		string[0]=puncts[i];
		string[1]='\0';
		petals[i+62].setString(string,*getUiFont());
		}
	for(int i=0;i<numPetals;++i)
		{
		GLLabel::Box::Vector labelSize=petals[i].getLabelSize();
		petals[i].setBackground(GLLabel::Color(0.667f,0.667f,0.667f));
		petals[i].setForeground(GLLabel::Color(0.0f,0.0f,0.0f));
		petals[i].setOrigin(GLLabel::Box::Vector(-labelSize[0]*0.5f,-labelSize[1]*0.5f,0.0f));
		}
	}

DaisyWheelTool::~DaisyWheelTool(void)
	{
	delete[] petals;
	}

const ToolFactory* DaisyWheelTool::getFactory(void) const
	{
	return factory;
	}

void DaisyWheelTool::buttonCallback(int,InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState) // Button has just been pressed
		{
		if(!active)
			{
			/* Activate the tool: */
			active=true;
			
			/* Store the daisy wheel transformation: */
			wheelTransform=calcHUDTransform(getInteractionPosition());
			
			/* Initialize the daisy wheel: */
			petals[selectedPetal].setBackground(GLLabel::Color(0.667f,0.667f,0.667f));
			selectedPetal=0;
			petals[selectedPetal].setBackground(GLLabel::Color(1.0f,0.5f,0.5f));
			selectedAngle=Scalar(0);
			
			hasEnteredWheel=true;
			}
		else
			hasEnteredWheel=false;
		
		buttonDown=true;
		}
	else // Button has just been released
		{
		if(!hasEnteredWheel)
			{
			/* Deactivate the tool: */
			active=false;
			
			/* Send a confirmation event to the widget manager: */
			getWidgetManager()->textControl(GLMotif::TextControlEvent(GLMotif::TextControlEvent::CONFIRM));
			}
		else if(rayInPetal)
			{
			/* Send the character associated with the selected petal to the widget manager: */
			getWidgetManager()->text(GLMotif::TextEvent(petals[selectedPetal].getString()));
			}
		
		buttonDown=false;
		}
	}

void DaisyWheelTool::frame(void)
	{
	if(active)
		{
		/* Update the selection ray: */
		selectionRay=calcInteractionRay();
		
		/* Calculate the intersection point of selection ray and daisy wheel: */
		Ray wheelRay=selectionRay;
		wheelRay.inverseTransform(wheelTransform);
		if(wheelRay.getDirection()[2]!=Scalar(0))
			{
			Scalar lambda=(Scalar(0)-wheelRay.getOrigin()[2])/wheelRay.getDirection()[2];
			if(lambda>=Scalar(0))
				{
				/* Calculate the intersection point's polar coordinates: */
				Point wheelPoint=wheelRay(lambda);
				Scalar pointRadius=Math::sqrt(Math::sqr(wheelPoint[0])+Math::sqr(wheelPoint[1]));
				
				if(pointRadius<factory->innerRadius)
					zoomStrength=Scalar(1);
				else if(pointRadius>factory->outerRadius)
					zoomStrength=Math::log(factory->maxPetalAngle/Math::Constants<Scalar>::pi)/-Math::log(Scalar(numPetals));
				else
					{
					/* Remember if the tool enters the daisy wheel while the button is down: */
					if(buttonDown)
						hasEnteredWheel=true;
					
					/* Calculate the new zoom strength: */
					Scalar baseAngle=Math::Constants<Scalar>::pi/double(numPetals);
					Scalar zoomFactor=(pointRadius-factory->innerRadius)/(factory->outerRadius-factory->innerRadius);
					Scalar morphedBaseAngle=baseAngle*(Scalar(1)-zoomFactor)+factory->maxPetalAngle*zoomFactor;
					zoomStrength=Math::log(morphedBaseAngle/Math::Constants<Scalar>::pi)/Math::log(baseAngle/Math::Constants<Scalar>::pi);
					
					/* Calculate the point angle: */
					Scalar pointAngle=Math::atan2(wheelPoint[0],wheelPoint[1]);
					
					/* Find the selected petal: */
					int l=0;
					int r=numPetals;
					while(r-l>1)
						{
						int m=(l+r)/2;
						Scalar petalAngle=calcPetalAngle(Scalar(m)-Scalar(0.5));
						if(wrapAngle(pointAngle-petalAngle)>=Scalar(0))
							l=m;
						else
							r=m;
						}
					
					/* Check if the selected petal changed: */
					if(selectedPetal!=l)
						{
						/* Calculate the angle at which the new selected petal appears: */
						selectedAngle=wrapAngle(calcPetalAngle(l));
						
						/* Update the selected petal: */
						petals[selectedPetal].setBackground(GLLabel::Color(0.667f,0.667f,0.667f));
						selectedPetal=l;
						petals[selectedPetal].setBackground(GLLabel::Color(1.0f,0.5f,0.5f));
						}
					
					/* Check if the selection ray is actually inside the selected petal: */
					Scalar px=Math::sin(selectedAngle)*Math::div2(factory->innerRadius+factory->outerRadius);
					Scalar py=Math::cos(selectedAngle)*Math::div2(factory->innerRadius+factory->outerRadius);
					rayInPetal=Math::abs(wheelPoint[0]-px)<=factory->petalSize&&Math::abs(wheelPoint[1]-py)<=factory->petalSize;
					}
				}
			}
		
		/* Update the positions of all petals: */
		for(int i=0;i<numPetals;++i)
			{
			/* Calculate the petal label's center point: */
			Scalar angle=calcPetalAngle(i);
			int selectedOffset=i-selectedPetal;
			if(selectedOffset<-numPetals/2)
				selectedOffset+=numPetals;
			if(selectedOffset>numPetals/2)
				selectedOffset-=numPetals;
			Scalar px=Math::sin(angle)*Math::div2(factory->innerRadius+factory->outerRadius);
			Scalar py=Math::cos(angle)*Math::div2(factory->innerRadius+factory->outerRadius);
			Scalar pz=(Scalar(1)-Scalar(4)*Scalar(Math::abs(selectedOffset))/Scalar(numPetals))*factory->maxYOffset;
			
			/* Set the petal label's origin: */
			GLLabel::Box::Vector origin=petals[i].getLabelSize();
			origin[0]=px-origin[0]*0.5f;
			origin[1]=py-origin[1]*0.5f;
			origin[2]=pz;
			petals[i].setOrigin(origin);
			}
		}
	}

void DaisyWheelTool::display(GLContextData& contextData) const
	{
	if(active)
		{
		/* Set up OpenGL state: */
		glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
		glDisable(GL_LIGHTING);
		glLineWidth(1.0f);
		
		/* Draw the daisy wheel: */
		glPushMatrix();
		glMultMatrix(wheelTransform);
		
		GLLabel::DeferredRenderer dr(contextData);
		for(int i=0;i<numPetals;++i)
			{
			/* Get the petal label's center point: */
			GLLabel::Box::Vector p=petals[i].getLabelBox().origin;
			p[0]+=petals[i].getLabelSize()[0]*0.5f;
			p[1]+=petals[i].getLabelSize()[1]*0.5f;
			
			#if 1
			/* Draw the petal box: */
			glBegin(GL_QUAD_STRIP);
			if(i==selectedPetal)
				glColor3f(1.0f,0.5f,0.5f);
			else if(i==0)
				glColor3f(0.5f,1.0f,0.5f);
			else
				glColor3f(0.667f,0.667f,0.667f);
			glVertex(petals[i].getLabelBox().getCorner(0));
			glVertex3d(p[0]-factory->petalSize,p[1]-factory->petalSize,p[2]);
			glVertex(petals[i].getLabelBox().getCorner(1));
			glVertex3d(p[0]+factory->petalSize,p[1]-factory->petalSize,p[2]);
			glVertex(petals[i].getLabelBox().getCorner(3));
			glVertex3d(p[0]+factory->petalSize,p[1]+factory->petalSize,p[2]);
			glVertex(petals[i].getLabelBox().getCorner(2));
			glVertex3d(p[0]-factory->petalSize,p[1]+factory->petalSize,p[2]);
			glVertex(petals[i].getLabelBox().getCorner(0));
			glVertex3d(p[0]-factory->petalSize,p[1]-factory->petalSize,p[2]);
			glEnd();
			#endif
			
			/* Draw the petal label: */
			petals[i].draw(contextData);
			
			glBegin(GL_LINES);
			glVertex3d(Math::div2(p[0]),Math::div2(p[1]),p[2]-0.01);
			glVertex3d(p[0],p[1],p[2]-0.01);
			glEnd();
			}
		dr.draw();
		
		glPopMatrix();
		
		/* Draw the menu selection ray: */
		glLineWidth(3.0f);
		glColor3f(1.0f,0.0f,0.0f);
		glBegin(GL_LINES);
		glVertex(selectionRay.getOrigin());
		glVertex(selectionRay(getDisplaySize()*Scalar(5)));
		glEnd();
		
		/* Restore OpenGL state: */
		glPopAttrib();
		}
	}

}
