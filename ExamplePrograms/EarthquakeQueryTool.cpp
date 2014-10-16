/***********************************************************************
EarthquakeQueryTool - Vrui tool class to query and display meta data
about earthquake events.
Copyright (c) 2013 Oliver Kreylos

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

#include "EarthquakeQueryTool.h"

#include <Math/Constants.h>
#include <Geometry/Ray.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLColor.h>
#include <GL/GLVertexTemplates.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/Label.h>
#include <GLMotif/Button.h>
#include <GLMotif/TextField.h>
#include <Vrui/Vrui.h>
#include <Vrui/InputDevice.h>
#include <Vrui/ToolManager.h>

#include "EarthquakeSet.h"

/*******************************************
Methods of class EarthquakeQueryToolFactory:
*******************************************/

EarthquakeQueryToolFactory::EarthquakeQueryToolFactory(Vrui::ToolManager& toolManager,const std::vector<EarthquakeSet*>& sEarthquakeSets,SetTimeFunction* sSetTimeFunction)
	:Vrui::ToolFactory("EarthquakeQueryTool",toolManager),
	 earthquakeSets(sEarthquakeSets),
	 setTimeFunction(sSetTimeFunction)
	{
	/* Insert class into class hierarchy: */
	#if 0
	Vrui::TransformToolFactory* toolFactory=dynamic_cast<Vrui::ToolFactory*>(toolManager.loadClass("Tool"));
	toolFactory->addChildClass(this);
	addParentClass(toolFactory);
	#endif
	
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Set the custom tool class' factory pointer: */
	EarthquakeQueryTool::factory=this;
	}

EarthquakeQueryToolFactory::~EarthquakeQueryToolFactory(void)
	{
	/* Delete the set time function: */
	delete setTimeFunction;
	
	/* Reset the custom tool class' factory pointer: */
	EarthquakeQueryTool::factory=0;
	}

const char* EarthquakeQueryToolFactory::getName(void) const
	{
	return "Query Earthquakes";
	}

Vrui::Tool* EarthquakeQueryToolFactory::createTool(const Vrui::ToolInputAssignment& inputAssignment) const
	{
	/* Create a new object of the custom tool class: */
	EarthquakeQueryTool* newTool=new EarthquakeQueryTool(this,inputAssignment);
	
	return newTool;
	}

void EarthquakeQueryToolFactory::destroyTool(Vrui::Tool* tool) const
	{
	/* Cast the tool pointer to the Earthquake tool class (not really necessary): */
	EarthquakeQueryTool* earthquakeTool=dynamic_cast<EarthquakeQueryTool*>(tool);
	
	/* Destroy the tool: */
	delete earthquakeTool;
	}

/********************************************
Static elements of class EarthquakeQueryTool:
********************************************/

EarthquakeQueryToolFactory* EarthquakeQueryTool::factory=0;

/************************************
Methods of class EarthquakeQueryTool:
************************************/

void EarthquakeQueryTool::setTimeButtonCallback(Misc::CallbackData* cbData)
	{
	if(selectedEvent!=0&&factory->setTimeFunction!=0)
		{
		/* Call the time setting function with the selected event's time: */
		(*factory->setTimeFunction)(selectedEvent->time);
		}
	}

EarthquakeQueryTool::EarthquakeQueryTool(const Vrui::ToolFactory* factory,const Vrui::ToolInputAssignment& inputAssignment)
	:Vrui::Tool(factory,inputAssignment),
	 selectedEvent(0)
	{
	/* Create the event data display dialog: */
	dataDialog=new GLMotif::PopupWindow("DataDialog",Vrui::getWidgetManager(),"Earthquake Data");
	
	GLMotif::RowColumn* data=new GLMotif::RowColumn("Data",dataDialog,false);
	data->setOrientation(GLMotif::RowColumn::VERTICAL);
	data->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	data->setNumMinorWidgets(2);
	
	new GLMotif::Label("TimeLabel",data,"Time");
	timeTextField=new GLMotif::TextField("TimeValue",data,19);
	
	new GLMotif::Label("MagnitudeLabel",data,"Magnitude");
	magnitudeTextField=new GLMotif::TextField("MagnitudeValue",data,5);
	magnitudeTextField->setFieldWidth(5);
	magnitudeTextField->setPrecision(2);
	
	GLMotif::Button* setTimeButton=new GLMotif::Button("SetTimeButton",data,"Set Time");
	setTimeButton->getSelectCallbacks().add(this,&EarthquakeQueryTool::setTimeButtonCallback);
	
	data->manageChild();
	
	/* Pop up the data dialog: */
	Vrui::popupPrimaryWidget(dataDialog);
	}

EarthquakeQueryTool::~EarthquakeQueryTool(void)
	{
	delete dataDialog;
	}

const Vrui::ToolFactory* EarthquakeQueryTool::getFactory(void) const
	{
	return factory;
	}

void EarthquakeQueryTool::frame(void)
	{
	if(getButtonState(0))
		{
		const EarthquakeSet::Event* newEvent=0;
		
		if(getButtonDevice(0)->is6DOFDevice())
			{
			/* Select the closest earthquake event to the device's position: */
			EarthquakeSet::Point position=EarthquakeSet::Point(Vrui::getNavigationTransformation().inverseTransform(getButtonDevicePosition(0)));
			float maxDistance=float(Vrui::getPointPickDistance());
			for(std::vector<EarthquakeSet*>::const_iterator eqsIt=factory->earthquakeSets.begin();eqsIt!=factory->earthquakeSets.end();++eqsIt)
				{
				const EarthquakeSet::Event* e=(*eqsIt)->selectEvent(position,maxDistance);
				if(e!=0)
					{
					newEvent=e;
					maxDistance=Geometry::dist(position,newEvent->position);
					}
				}
			}
		else
			{
			/* Select the closest earthquake event along the device's ray: */
			Vrui::Ray ray=getButtonDeviceRay(0);
			ray.transform(Vrui::getInverseNavigationTransformation());
			Vrui::Scalar rayLength=Geometry::mag(ray.getDirection());
			ray.normalizeDirection();
			EarthquakeSet::Ray eqRay(ray);
			Vrui::Scalar rayParameter=Math::Constants<Vrui::Scalar>::max;
			for(std::vector<EarthquakeSet*>::const_iterator eqsIt=factory->earthquakeSets.begin();eqsIt!=factory->earthquakeSets.end();++eqsIt)
				{
				const EarthquakeSet::Event* e=(*eqsIt)->selectEvent(ray,float(Vrui::getRayPickCosine()));
				if(e!=0)
					{
					/* Calculate the test event's ray parameter: */
					Vrui::Scalar rp=((Vrui::Point(e->position)-ray.getOrigin())*ray.getDirection())/rayLength;
					if(rayParameter>rp)
						{
						newEvent=e;
						rayParameter=rp;
						}
					}
				}
			}
		
		if(newEvent!=0&&selectedEvent!=newEvent)
			{
			/* Display the newly-selected event's meta data in the meta data dialog: */
			selectedEvent=newEvent;
			
			/* Display the event time: */
			time_t tT=time_t(selectedEvent->time);
			struct tm tTm;
			localtime_r(&tT,&tTm);
			char tBuffer[20];
			snprintf(tBuffer,sizeof(tBuffer),"%04d/%02d/%02d %02d:%02d:%02d",tTm.tm_year+1900,tTm.tm_mon+1,tTm.tm_mday,tTm.tm_hour,tTm.tm_min,tTm.tm_sec);
			timeTextField->setString(tBuffer);
			
			/* Display the event magnitude: */
			magnitudeTextField->setValue(selectedEvent->magnitude);
			}
		}
	}

void EarthquakeQueryTool::display(GLContextData& contextData) const
	{
	if(selectedEvent!=0)
		{
		/* Save and set up OpenGL state: */
		glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
		glDisable(GL_LIGHTING);
		
		/* Determine the marker color: */
		Vrui::Color bgColor=Vrui::getBackgroundColor();
		Vrui::Color fgColor;
		for(int i=0;i<3;++i)
			fgColor[i]=1.0f-bgColor[i];
		fgColor[3]=bgColor[3];
		
		/* Draw the marker: */
		Vrui::Point pos=Vrui::getNavigationTransformation().transform(Vrui::Point(selectedEvent->position));
		Vrui::Scalar markerSize=Vrui::getUiSize();
		
		glLineWidth(3.0f);
		glColor(bgColor);
		glBegin(GL_LINES);
		glVertex(pos[0]-markerSize,pos[1],pos[2]);
		glVertex(pos[0]+markerSize,pos[1],pos[2]);
		glVertex(pos[0],pos[1]-markerSize,pos[2]);
		glVertex(pos[0],pos[1]+markerSize,pos[2]);
		glVertex(pos[0],pos[1],pos[2]-markerSize);
		glVertex(pos[0],pos[1],pos[2]+markerSize);
		glEnd();
		
		glLineWidth(1.0f);
		glColor(fgColor);
		glBegin(GL_LINES);
		glVertex(pos[0]-markerSize,pos[1],pos[2]);
		glVertex(pos[0]+markerSize,pos[1],pos[2]);
		glVertex(pos[0],pos[1]-markerSize,pos[2]);
		glVertex(pos[0],pos[1]+markerSize,pos[2]);
		glVertex(pos[0],pos[1],pos[2]-markerSize);
		glVertex(pos[0],pos[1],pos[2]+markerSize);
		glEnd();
		
		/* Reset OpenGL state: */
		glPopAttrib();
		}
	}
