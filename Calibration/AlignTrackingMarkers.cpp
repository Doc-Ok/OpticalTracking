/***********************************************************************
AlignTrackingMarkers - Utility to define a reasonable coordinate system
based on tracking marker positions detected by an optical tracking
system.
Copyright (c) 2008-2013 Oliver Kreylos

This file is part of the Vrui calibration utility package.

The Vrui calibration utility package is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Vrui calibration utility package is distributed in the hope that it
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Vrui calibration utility package; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdexcept>
#include <utility>
#include <vector>
#include <iostream>
#include <Misc/ThrowStdErr.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/Point.h>
#include <Geometry/AffineCombiner.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Geometry/Ray.h>
#include <Geometry/SolidHitResult.h>
#include <Geometry/Sphere.h>
#include <Geometry/Cylinder.h>
#include <Geometry/GeometryValueCoders.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLColor.h>
#include <GL/GLMaterial.h>
#include <GL/GLModels.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <GLMotif/Blind.h>
#include <GLMotif/Label.h>
#include <GLMotif/Button.h>
#include <GLMotif/ToggleButton.h>
#include <GLMotif/TextField.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/Menu.h>
#include <GLMotif/PopupMenu.h>
#include <GLMotif/PopupWindow.h>
#include <Vrui/Vrui.h>
#include <Vrui/InputDevice.h>
#include <Vrui/ToolManager.h>
#include <Vrui/Tool.h>
#include <Vrui/GenericToolFactory.h>
#include <Vrui/Application.h>

#include "ReadOptiTrackMarkerFile.h"
#include "NaturalPointClient.h"

namespace {

/**************************************************************************************
Helper function to query relative marker positions from a NaturalPoint tracking server:
**************************************************************************************/

void queryRigidBody(const char* naturalPointServerName,int rigidBodyId,double scale,bool flipZ,std::vector<Geometry::Point<double,3> >& markers)
	{
	/* Open a connection to the NaturalPoint server: */
	NaturalPointClient npc(naturalPointServerName,1510,"224.0.0.1",1511);
	
	std::cout<<"Server name: "<<npc.getServerName()<<std::endl;
	std::cout<<"Server version: "<<npc.getServerVersion()[0]<<'.'<<npc.getServerVersion()[1]<<'.'<<npc.getServerVersion()[2]<<'.'<<npc.getServerVersion()[3]<<std::endl;
	std::cout<<"Protocol version: "<<npc.getProtocolVersion()[0]<<'.'<<npc.getProtocolVersion()[1]<<'.'<<npc.getProtocolVersion()[2]<<'.'<<npc.getProtocolVersion()[3]<<std::endl;
	
	/* Track the requested rigid body for a number of frames to calculate average relative marker positions: */
	std::cout<<"Please show the rigid body with ID "<<rigidBodyId<<" to the OptiTrack system"<<std::endl;
	unsigned int numFrames=0;
	std::vector<NaturalPointClient::Point> initialMarkers;
	std::vector<NaturalPointClient::Point::AffineCombiner> markerCombiners;
	while(numFrames<50)
		{
		/* Wait for the next frame from the NaturalPoint engine: */
		const NaturalPointClient::Frame& frame=npc.waitForNextFrame();
		
		/* Check if the frame contains the requested rigid body: */
		for(std::vector<NaturalPointClient::RigidBody>::const_iterator rbIt=frame.rigidBodies.begin();rbIt!=frame.rigidBodies.end();++rbIt)
			if(rbIt->id==rigidBodyId)
				{
				/* Check if this is the first frame: */
				if(numFrames==0)
					{
					std::cout<<"Found rigid body "<<rigidBodyId<<", capturing frames..."<<std::flush;
					for(std::vector<NaturalPointClient::Point>::const_iterator mIt=rbIt->markers.begin();mIt!=rbIt->markers.end();++mIt)
						{
						/* Transform the marker to rigid body coordinates: */
						NaturalPointClient::Point m=NaturalPointClient::Point::origin+rbIt->orientation.inverseTransform(*mIt-rbIt->position);
						
						/* Store the initial marker position and create an accumulator: */
						initialMarkers.push_back(m);
						markerCombiners.push_back(NaturalPointClient::Point::AffineCombiner());
						}
					}
				
				/* Find the best match for each marker in the initial configuration and accumulate their positions: */
				for(std::vector<NaturalPointClient::Point>::const_iterator mIt=rbIt->markers.begin();mIt!=rbIt->markers.end();++mIt)
					{
					/* Transform the marker to rigid body coordinates: */
					NaturalPointClient::Point m=NaturalPointClient::Point::origin+rbIt->orientation.inverseTransform(*mIt-rbIt->position);
					
					/* Find the best-matching initial marker: */
					size_t bestIndex=0;
					NaturalPointClient::Scalar bestDist2=Geometry::sqrDist(m,initialMarkers[0]);
					for(size_t i=1;i<initialMarkers.size();++i)
						{
						NaturalPointClient::Scalar dist2=Geometry::sqrDist(m,initialMarkers[i]);
						if(bestDist2>dist2)
							{
							bestIndex=i;
							bestDist2=dist2;
							}
						}
					
					/* Accumulate the point: */
					markerCombiners[bestIndex].addPoint(m);
					}
				
				++numFrames;
				}
		}
	std::cout<<" done"<<std::endl;
	
	/* Store the averaged marker positions: */
	for(std::vector<NaturalPointClient::Point::AffineCombiner>::iterator mcIt=markerCombiners.begin();mcIt!=markerCombiners.end();++mcIt)
		{
		Geometry::Point<double,3> m;
		for(int i=0;i<3;++i)
			m[i]=double(mcIt->getPoint()[i])*scale;
		if(flipZ)
			m[2]=-m[2];
		markers.push_back(m);
		}
	}

}

class AlignTrackingMarkers:public Vrui::Application
	{
	/* Embedded classes: */
	private:
	typedef double Scalar;
	typedef Geometry::Point<Scalar,3> Point;
	typedef Geometry::Vector<Scalar,3> Vector;
	typedef Geometry::OrthonormalTransformation<Scalar,3> ONTransform;
	typedef Geometry::Ray<Scalar,3> Ray;
	typedef std::vector<Point> PointList;
	typedef std::pair<Point,Point> Line;
	typedef std::vector<Line> LineList;
	
	class MarkerTool;
	typedef Vrui::GenericToolFactory<MarkerTool> MarkerToolFactory;
	
	class MarkerTool:public Vrui::Tool,public Vrui::Application::Tool<AlignTrackingMarkers>
		{
		friend class Vrui::GenericToolFactory<MarkerTool>;
		
		/* Elements: */
		private:
		static MarkerToolFactory* factory;
		
		GLMotif::PopupWindow* dialogPopup; // Marker tool's measurement dialog box
		GLMotif::TextField* pos[2][3]; // Current position text fields
		GLMotif::TextField* dist; // Current distance text field
		bool dragging; // Flag whether the tool is currently dragging a line or the origin
		Point start; // Starting point of the currently dragged line
		Point current; // Current end point of currently dragged line
		
		/* Constructors and destructors: */
		public:
		MarkerTool(const Vrui::ToolFactory* sFactory,const Vrui::ToolInputAssignment& inputAssignment);
		virtual ~MarkerTool(void);
		
		/* Methods from class Vrui::Tool: */
		virtual const Vrui::ToolFactory* getFactory(void) const
			{
			return factory;
			}
		virtual void buttonCallback(int buttonSlotIndex,Vrui::InputDevice::ButtonCallbackData* cbData);
		virtual void frame(void);
		virtual void display(GLContextData& contextData) const;
		};
	
	/* Elements: */
	private:
	ONTransform transform; // Transformation from original coordinate system to aligned coordinate system
	Scalar axisLength; // Length for the initial coordinate axes
	Scalar markerSize; // Size to draw markers in scaled local tracker coordinates
	Scalar lineSize; // Size to draw lines in scaled local tracker coordinates
	PointList markers; // List of marker positions in scaled local tracker coordinates
	LineList lines; // List of lines used to define coordinate axes
	GLMotif::PopupMenu* mainMenuPopup; // The program's main menu
	bool moveOrigin; // Flag whether marker tools will move the transformation's origin
	
	/* Private methods: */
	GLMotif::PopupMenu* createMainMenu(void); // Creates the program's main menu
	
	/* Constructors and destructors: */
	public:
	AlignTrackingMarkers(int& argc,char**& argv,char**& appDefaults);
	virtual ~AlignTrackingMarkers(void);
	
	/* Methods from class Vrui::Application: */
	virtual void display(GLContextData& contextData) const;
	
	/* New methods: */
	Point snap(const Point& p) const; // Snaps a 3D point to the existing markers/lines
	Point snap(const Ray& ray) const; // Snaps a 3D ray to the existing markers/lines
	Point snap(const Point& start,const Point& p) const; // Snaps a 3D point to the existing markers/lines, with line start point for angle snapping
	Point snap(const Point& start,const Ray& ray) const; // Snaps a 3D ray to the existing markers/lines, with line start point for angle snapping
	void addLine(const Point& p1,const Point& p2); // Adds a line to the list
	void setOrigin(const Point& p); // Sets the current coordinate system's origin
	void removeLastLineCallback(Misc::CallbackData* cbData); // Removes the most recently added line
	void alignXAxisCallback(Misc::CallbackData* cbData); // Aligns the X axis to the most recently added line
	void alignNegXAxisCallback(Misc::CallbackData* cbData); // Aligns the negative X axis to the most recently added line
	void alignYAxisCallback(Misc::CallbackData* cbData); // Aligns the Y axis to the most recently added line
	void alignNegYAxisCallback(Misc::CallbackData* cbData); // Aligns the negative Y axis to the most recently added line
	void alignZAxisCallback(Misc::CallbackData* cbData); // Aligns the Z axis to the most recently added line
	void alignNegZAxisCallback(Misc::CallbackData* cbData); // Aligns the negative Z axis to the most recently added line
	void moveOriginCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData); // Moves the origin of the coordinate system
	};

/*********************************************************
Static elements of class AlignTrackingMarkers::MarkerTool:
*********************************************************/

AlignTrackingMarkers::MarkerToolFactory* AlignTrackingMarkers::MarkerTool::factory=0;

/*************************************************
Methods of class AlignTrackingMarkers::MarkerTool:
*************************************************/

AlignTrackingMarkers::MarkerTool::MarkerTool(const Vrui::ToolFactory* sFactory,const Vrui::ToolInputAssignment& inputAssignment)
	:Vrui::Tool(sFactory,inputAssignment),
	 dialogPopup(0),
	 dragging(false)
	{
	/* Create the measurement dialog: */
	dialogPopup=new GLMotif::PopupWindow("DialogPopup",Vrui::getWidgetManager(),"Marker Measurements");
	dialogPopup->setResizableFlags(true,false);
	
	GLMotif::RowColumn* dialog=new GLMotif::RowColumn("Dialog",dialogPopup,false);
	dialog->setNumMinorWidgets(2);
	
	new GLMotif::Label("Pos1Label",dialog,"Point 1");
	
	GLMotif::RowColumn* pos1Box=new GLMotif::RowColumn("Pos1Box",dialog,false);
	pos1Box->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	pos1Box->setPacking(GLMotif::RowColumn::PACK_GRID);
	
	for(int i=0;i<3;++i)
		{
		char labelName[40];
		snprintf(labelName,sizeof(labelName),"Pos1-%d",i+1);
		pos[0][i]=new GLMotif::TextField(labelName,pos1Box,12);
		pos[0][i]->setPrecision(6);
		}
	
	pos1Box->manageChild();
	
	new GLMotif::Label("Pos2Label",dialog,"Point 2");
	
	GLMotif::RowColumn* pos2Box=new GLMotif::RowColumn("Pos2Box",dialog,false);
	pos2Box->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	pos2Box->setPacking(GLMotif::RowColumn::PACK_GRID);
	
	for(int i=0;i<3;++i)
		{
		char labelName[40];
		snprintf(labelName,sizeof(labelName),"Pos2-%d",i+1);
		pos[1][i]=new GLMotif::TextField(labelName,pos2Box,12);
		pos[1][i]->setPrecision(6);
		}
	
	pos2Box->manageChild();
	
	new GLMotif::Label("DistLabel",dialog,"Distance");
	
	GLMotif::RowColumn* distBox=new GLMotif::RowColumn("DistBox",dialog,false);
	distBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	distBox->setPacking(GLMotif::RowColumn::PACK_GRID);
	
	dist=new GLMotif::TextField("Dist",distBox,16);
	dist->setPrecision(10);
	
	new GLMotif::Blind("Blind",distBox);
	
	distBox->manageChild();
	
	dialog->manageChild();
	
	/* Pop up the measurement dialog: */
	Vrui::popupPrimaryWidget(dialogPopup);
	}

AlignTrackingMarkers::MarkerTool::~MarkerTool(void)
	{
	/* Pop down and destroy the measurement dialog: */
	Vrui::popdownPrimaryWidget(dialogPopup);
	delete dialogPopup;
	}

void AlignTrackingMarkers::MarkerTool::buttonCallback(int,Vrui::InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState)
		{
		/* Begin dragging a line or moving the origin: */
		start=current;
		
		/* Clear the measurement dialog: */
		Point s=application->transform.inverseTransform(start);
		for(int i=0;i<3;++i)
			{
			pos[0][i]->setValue(double(s[i]));
			pos[1][i]->setString("");
			}
		dist->setString("");
		
		dragging=true;
		}
	else
		{
		if(!application->moveOrigin)
			{ 
			/* Stop dragging the current line: */
			application->addLine(start,current);
			}
		
		dragging=false;
		}
	}

void AlignTrackingMarkers::MarkerTool::frame(void)
	{
	/* Get pointer to input device that caused the event: */
	Vrui::InputDevice* device=getButtonDevice(0);
	
	/* Snap the current input device to the existing marker set: */
	Vrui::NavTrackerState transform=Vrui::getDeviceTransformation(device);
	if(device->isRayDevice())
		{
		if(dragging)
			current=application->snap(start,Ray(transform.getOrigin(),transform.transform(device->getDeviceRayDirection())));
		else
			current=application->snap(Ray(transform.getOrigin(),transform.transform(device->getDeviceRayDirection())));
		}
	else
		{
		if(dragging)
			current=application->snap(start,transform.getOrigin());
		else
			current=application->snap(transform.getOrigin());
		}
	
	/* Update the measurement dialog: */
	Point c=application->transform.inverseTransform(current);
	if(dragging&&!application->moveOrigin)
		{
		for(int i=0;i<3;++i)
			pos[1][i]->setValue(double(c[i]));
		dist->setValue(double(Geometry::dist(start,current)));
		}
	else
		{
		for(int i=0;i<3;++i)
			pos[0][i]->setValue(double(c[i]));
		}
	
	if(dragging&&application->moveOrigin)
		application->setOrigin(current);
	}

void AlignTrackingMarkers::MarkerTool::display(GLContextData& contextData) const
	{
	if(dragging&&!application->moveOrigin)
		{
		/* Draw the currently dragged line: */
		glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
		glDisable(GL_LIGHTING);
		glLineWidth(1.0f);
		
		glPushMatrix();
		glMultMatrix(Vrui::getNavigationTransformation());
		
		glBegin(GL_LINES);
		glColor3f(1.0f,0.0f,0.0f);
		glVertex(start);
		glVertex(current);
		glEnd();
		
		glPopMatrix();
		glPopAttrib();
		}
	}

/*************************************
Methods of class AlignTrackingMarkers:
*************************************/

GLMotif::PopupMenu* AlignTrackingMarkers::createMainMenu(void)
	{
	/* Create a popup shell to hold the main menu: */
	GLMotif::PopupMenu* mainMenuPopup=new GLMotif::PopupMenu("MainMenuPopup",Vrui::getWidgetManager());
	mainMenuPopup->setTitle("Marker Alignment");
	
	/* Create the main menu itself: */
	GLMotif::Menu* mainMenu=new GLMotif::Menu("MainMenu",mainMenuPopup,false);
	
	/* Create the menu buttons: */
	GLMotif::Button* removeLastLineButton=new GLMotif::Button("RemoveLastLineButton",mainMenu,"Remove Last Line");
	removeLastLineButton->getSelectCallbacks().add(this,&AlignTrackingMarkers::removeLastLineCallback);
	
	GLMotif::Button* alignXAxisButton=new GLMotif::Button("AlignXAxisButton",mainMenu,"Align X Axis");
	alignXAxisButton->getSelectCallbacks().add(this,&AlignTrackingMarkers::alignXAxisCallback);
	
	GLMotif::Button* alignNegXAxisButton=new GLMotif::Button("AlignNegXAxisButton",mainMenu,"Align -X Axis");
	alignNegXAxisButton->getSelectCallbacks().add(this,&AlignTrackingMarkers::alignNegXAxisCallback);
	
	GLMotif::Button* alignYAxisButton=new GLMotif::Button("AlignYAxisButton",mainMenu,"Align Y Axis");
	alignYAxisButton->getSelectCallbacks().add(this,&AlignTrackingMarkers::alignYAxisCallback);
	
	GLMotif::Button* alignNegYAxisButton=new GLMotif::Button("AlignNegYAxisButton",mainMenu,"Align -Y Axis");
	alignNegYAxisButton->getSelectCallbacks().add(this,&AlignTrackingMarkers::alignNegYAxisCallback);
	
	GLMotif::Button* alignZAxisButton=new GLMotif::Button("AlignZAxisButton",mainMenu,"Align Z Axis");
	alignZAxisButton->getSelectCallbacks().add(this,&AlignTrackingMarkers::alignZAxisCallback);
	
	GLMotif::Button* alignNegZAxisButton=new GLMotif::Button("AlignNegZAxisButton",mainMenu,"Align -Z Axis");
	alignNegZAxisButton->getSelectCallbacks().add(this,&AlignTrackingMarkers::alignNegZAxisCallback);
	
	GLMotif::ToggleButton* moveOriginToggle=new GLMotif::ToggleButton("MoveOriginToggle",mainMenu,"Move Origin");
	moveOriginToggle->getValueChangedCallbacks().add(this,&AlignTrackingMarkers::moveOriginCallback);
	
	/* Finish building the main menu: */
	mainMenu->manageChild();
	
	return mainMenuPopup;
	}

AlignTrackingMarkers::AlignTrackingMarkers(int& argc,char**& argv,char**& appDefaults)
	:Vrui::Application(argc,argv,appDefaults),
	 transform(ONTransform::identity),
	 markerSize(Scalar(Vrui::getInchFactor())*Scalar(0.25)),
	 lineSize(markerSize/Scalar(3)),
	 mainMenuPopup(0),
	 moveOrigin(false)
	{
	/* Create and register the marker tool class: */
	MarkerToolFactory* markerToolFactory=new MarkerToolFactory("MarkerTool","Marker Selector",0,*Vrui::getToolManager());
	markerToolFactory->setNumButtons(1);
	markerToolFactory->setButtonFunction(0,"Select Markers");
	Vrui::getToolManager()->addClass(markerToolFactory,Vrui::ToolManager::defaultToolFactoryDestructor);
	
	/* Parse the command line: */
	const char* fileName=0;
	const char* bodyName=0;
	const char* naturalPointServerName=0;
	int naturalPointRigidBodyId=-1;
	Scalar scale=Scalar(1);
	bool flipZ=false;
	for(int i=1;i<argc;++i)
		{
		if(argv[i][0]=='-')
			{
			if(strcasecmp(argv[i]+1,"size")==0)
				{
				++i;
				markerSize=Scalar(atof(argv[i]));
				}
			else if(strcasecmp(argv[i]+1,"scale")==0)
				{
				++i;
				scale=Scalar(atof(argv[i]));
				}
			else if(strcasecmp(argv[i]+1,"inches")==0)
				{
				/* Set scale factor from meters to inches: */
				scale=Scalar(1000)/Scalar(25.4);
				}
			else if(strcasecmp(argv[i]+1,"flipZ")==0)
				flipZ=true;
			else if(strcasecmp(argv[i]+1,"npc")==0)
				{
				++i;
				naturalPointServerName=argv[i];
				++i;
				naturalPointRigidBodyId=atoi(argv[i]);
				}
			}
		else if(fileName==0)
			fileName=argv[i];
		else
			bodyName=argv[i];
		}
	
	if((fileName==0||bodyName==0)&&(naturalPointServerName==0||naturalPointRigidBodyId==-1))
		{
		std::cerr<<"Usage: "<<argv[0]<<" ( <rigid body definition file name> <rigid body name> ) | ( -npc <NaturalPoint server name> <rigid body ID> ) [-scale <unit scale factor>] [-inches] [-flipZ] [-size <marker size>]"<<std::endl;
		Misc::throwStdErr("AlignTrackingMarkers::AlignTrackingMarkers: No file name and rigid body name or NaturalPoint server name and rigid body ID provided");
		}
	
	if(fileName!=0&&bodyName!=0)
		{
		/* Determine the marker file name's extension: */
		const char* extPtr=0;
		for(const char* fnPtr=fileName;*fnPtr!='\0';++fnPtr)
			if(*fnPtr=='.')
				extPtr=fnPtr;
		
		/* Read the marker file: */
		if(extPtr!=0&&strcasecmp(extPtr,".rdef")==0)
			readOptiTrackMarkerFile(fileName,bodyName,scale,flipZ,markers);
		else
			Misc::throwStdErr("AlignTrackingMarkers::AlignTrackingMarkers: marker file %s has unrecognized extension",fileName);
		}
	else
		{
		/* Get a rigid body definition from the NaturalPoint server: */
		queryRigidBody(naturalPointServerName,naturalPointRigidBodyId,scale,flipZ,markers);
		}
	
	/* Create the main menu: */
	mainMenuPopup=createMainMenu();
	Vrui::setMainMenu(mainMenuPopup);
	
	/* Initialize the navigation transformation: */
	Point::AffineCombiner centroidC;
	for(PointList::const_iterator mIt=markers.begin();mIt!=markers.end();++mIt)
		centroidC.addPoint(*mIt);
	Point centroid=centroidC.getPoint();
	Scalar maxSqrDist=Scalar(0);
	for(PointList::const_iterator mIt=markers.begin();mIt!=markers.end();++mIt)
		{
		Scalar sqrDist=Geometry::sqrDist(centroid,*mIt);
		if(maxSqrDist<sqrDist)
			maxSqrDist=sqrDist;
		}
	axisLength=Math::sqrt(maxSqrDist)*Scalar(2);
	Vrui::setNavigationTransformation(centroid,axisLength);
	}

AlignTrackingMarkers::~AlignTrackingMarkers(void)
	{
	/* Print the updated transformation: */
	std::cout<<"Final transformation: "<<Misc::ValueCoder<ONTransform>::encode(transform)<<std::endl;
	
	/* Delete the main menu: */
	delete mainMenuPopup;
	}

void AlignTrackingMarkers::display(GLContextData& contextData) const
	{
	/* Set up OpenGL state: */
	glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
	glDisable(GL_LIGHTING);
	glLineWidth(1.0f);
	
	/* Draw the current coordinate axes: */
	glPushMatrix();
	glMultMatrix(transform);
	
	glBegin(GL_LINES);
	glColor3f(1.0f,0.0f,0.0f);
	glVertex3f(-axisLength,0.0f,0.0f);
	glVertex3f( axisLength,0.0f,0.0f);
	glColor3f(0.0f,1.0f,0.0f);
	glVertex3f(0.0f,-axisLength,0.0f);
	glVertex3f(0.0f, axisLength,0.0f);
	glColor3f(0.0f,0.0f,1.0f);
	glVertex3f(0.0f,0.0f,-axisLength);
	glVertex3f(0.0f,0.0f, axisLength);
	glEnd();
	
	glPopMatrix();
	
	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glMaterial(GLMaterialEnums::FRONT,GLMaterial(GLMaterial::Color(1.0f,1.0f,1.0f),GLMaterial::Color(1.0f,1.0f,1.0f),25.0f));
	static const GLColor<GLfloat,3> markerColors[8]=
		{
		GLColor<GLfloat,3>(0.75f,0.25f,0.25f),
		GLColor<GLfloat,3>(0.25f,0.75f,0.25f),
		GLColor<GLfloat,3>(0.25f,0.25f,0.75f),
		GLColor<GLfloat,3>(0.75f,0.75f,0.25f),
		GLColor<GLfloat,3>(0.25f,0.75f,0.75f),
		GLColor<GLfloat,3>(0.75f,0.25f,0.75f),
		GLColor<GLfloat,3>(0.33f,0.33f,0.33f),
		GLColor<GLfloat,3>(0.67f,0.67f,0.67f)
		};
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
	
	/* Draw all markers: */
	int colorIndex=0;
	for(PointList::const_iterator mIt=markers.begin();mIt!=markers.end();++mIt,++colorIndex)
		{
		glPushMatrix();
		glTranslate(*mIt-Point::origin);
		glColor(markerColors[colorIndex%8]);
		glDrawSphereIcosahedron(markerSize,4);
		glPopMatrix();
		}
	
	/* Draw all lines: */
	for(LineList::const_iterator lIt=lines.begin();lIt!=lines.end();++lIt)
		{
		Point p1=lIt->first;
		Point p2=lIt->second;
		Vector axis=p2-p1;
		Scalar height=Geometry::mag(axis);
		axis/=height;
		Vector x=Geometry::normal(axis);
		x.normalize();
		Vector y=axis^x;
		y.normalize();
		glBegin(GL_QUAD_STRIP);
		glColor3f(0.5f,0.5f,0.5f);
		for(int i=0;i<=12;++i)
			{
			Scalar angle=Scalar(2)*Math::Constants<Scalar>::pi*Scalar(i)/Scalar(12);
			Vector normal=x*Math::cos(angle)+y*Math::sin(angle);
			glNormal(normal);
			glVertex(p2+normal*lineSize);
			glVertex(p1+normal*lineSize);
			}
		glEnd();
		}
	
	/* Reset OpenGL state: */
	glPopAttrib();
	}

AlignTrackingMarkers::Point AlignTrackingMarkers::snap(const AlignTrackingMarkers::Point& p) const
	{
	Point bestPoint=p;
	Scalar bestSqrDist=Math::sqr(markerSize);
	
	/* Compare the point against all lines: */
	for(LineList::const_iterator lIt=lines.begin();lIt!=lines.end();++lIt)
		{
		Point p1=lIt->first;
		Point p2=lIt->second;
		Vector axis=p2-p1;
		Scalar height=Geometry::mag(axis);
		axis/=height;
		
		Scalar sqrDist=Scalar(0);
		Vector pp1=p-p1;
		Scalar along=pp1*axis;
		if(along<Scalar(0))
			sqrDist+=Math::sqr(along);
		else if(along>height)
			sqrDist+=Math::sqr(along-height);
		sqrDist+=Geometry::sqr(pp1)-Math::sqr(along);
		sqrDist*=Scalar(9);
		
		if(sqrDist<bestSqrDist)
			{
			if(along<=Scalar(0))
				bestPoint=p1;
			else if(along>=height)
				bestPoint=p2;
			else
				bestPoint=p1+axis*along;
			bestSqrDist=sqrDist;
			}
		}
	
	/* Compare the point against all markers: */
	for(PointList::const_iterator mIt=markers.begin();mIt!=markers.end();++mIt)
		{
		Scalar sqrDist=Geometry::sqrDist(*mIt,p);
		if(sqrDist<bestSqrDist)
			{
			bestPoint=*mIt;
			bestSqrDist=sqrDist;
			}
		}
	
	return bestPoint;
	}

AlignTrackingMarkers::Point AlignTrackingMarkers::snap(const AlignTrackingMarkers::Ray& ray) const
	{
	Point bestPoint=ray.getOrigin();
	Scalar bestLambda=Math::Constants<Scalar>::max;
	
	/* Compare the ray against all lines: */
	for(LineList::const_iterator lIt=lines.begin();lIt!=lines.end();++lIt)
		{
		Geometry::Cylinder<Scalar,3> cylinder(lIt->first,lIt->second,lineSize);
		Geometry::Cylinder<Scalar,3>::HitResult hr=cylinder.intersectRay(ray);
		if(hr.isValid()&&hr.getParameter()<bestLambda)
			{
			bestPoint=cylinder.getP1()+cylinder.getAxis()*((ray(hr.getParameter())-cylinder.getP1())*cylinder.getAxis());
			bestLambda=hr.getParameter();
			}
		}
	
	/* Compare the ray against all markers: */
	for(PointList::const_iterator mIt=markers.begin();mIt!=markers.end();++mIt)
		{
		Geometry::Sphere<Scalar,3> sphere(*mIt,markerSize);
		Geometry::Sphere<Scalar,3>::HitResult hr=sphere.intersectRay(ray);
		if(hr.isValid()&&hr.getParameter()<bestLambda)
			{
			bestPoint=*mIt;
			bestLambda=hr.getParameter();
			}
		}
	
	return bestPoint;
	}

AlignTrackingMarkers::Point AlignTrackingMarkers::snap(const AlignTrackingMarkers::Point& start,const AlignTrackingMarkers::Point& p) const
	{
	Point bestPoint=p;
	Scalar bestSqrDist=Math::sqr(markerSize);
	
	/* Compare the point against all lines: */
	for(LineList::const_iterator lIt=lines.begin();lIt!=lines.end();++lIt)
		{
		Point p1=lIt->first;
		Point p2=lIt->second;
		Vector axis=p2-p1;
		Scalar height=Geometry::mag(axis);
		axis/=height;
		
		Scalar sqrDist=Scalar(0);
		Vector pp1=p-p1;
		Scalar along=pp1*axis;
		if(along<Scalar(0))
			sqrDist+=Math::sqr(along);
		else if(along>height)
			sqrDist+=Math::sqr(along-height);
		sqrDist+=Geometry::sqr(pp1)-Math::sqr(along);
		sqrDist*=Scalar(9);
		
		if(sqrDist<bestSqrDist)
			{
			if(along<=Scalar(0))
				bestPoint=p1;
			else if(along>=height)
				bestPoint=p2;
			else
				bestPoint=p1+axis*along;
			
			/* Check if the two lines' angles should be snapped to a right angle: */
			Vector line=start-bestPoint;
			Scalar cosAngle=(axis*line)/Geometry::mag(line);
			if(Math::abs(cosAngle)<Math::cos(Math::rad(Scalar(85))))
				bestPoint=p1+axis*((start-p1)*axis);
			
			bestSqrDist=sqrDist;
			}
		}
	
	/* Compare the point against all markers: */
	for(PointList::const_iterator mIt=markers.begin();mIt!=markers.end();++mIt)
		{
		Scalar sqrDist=Geometry::sqrDist(*mIt,p);
		if(sqrDist<bestSqrDist)
			{
			bestPoint=*mIt;
			bestSqrDist=sqrDist;
			}
		}
	
	return bestPoint;
	}

AlignTrackingMarkers::Point AlignTrackingMarkers::snap(const AlignTrackingMarkers::Point& start,const AlignTrackingMarkers::Ray& ray) const
	{
	Point bestPoint=ray.getOrigin();
	Scalar bestLambda=Math::Constants<Scalar>::max;
	
	/* Compare the ray against all lines: */
	for(LineList::const_iterator lIt=lines.begin();lIt!=lines.end();++lIt)
		{
		Geometry::Cylinder<Scalar,3> cylinder(lIt->first,lIt->second,lineSize);
		Geometry::Cylinder<Scalar,3>::HitResult hr=cylinder.intersectRay(ray);
		if(hr.isValid()&&hr.getParameter()<bestLambda)
			{
			bestPoint=cylinder.getP1()+cylinder.getAxis()*((ray(hr.getParameter())-cylinder.getP1())*cylinder.getAxis());
			
			/* Check if the two lines' angles should be snapped to a right angle: */
			Vector line=start-bestPoint;
			Scalar cosAngle=(cylinder.getAxis()*line)/Geometry::mag(line);
			if(Math::abs(cosAngle)<Math::cos(Math::rad(Scalar(85))))
				bestPoint=cylinder.getP1()+cylinder.getAxis()*((start-cylinder.getP1())*cylinder.getAxis());
			
			bestLambda=hr.getParameter();
			}
		}
	
	/* Compare the ray against all markers: */
	for(PointList::const_iterator mIt=markers.begin();mIt!=markers.end();++mIt)
		{
		Geometry::Sphere<Scalar,3> sphere(*mIt,markerSize);
		Geometry::Sphere<Scalar,3>::HitResult hr=sphere.intersectRay(ray);
		if(hr.isValid()&&hr.getParameter()<bestLambda)
			{
			bestPoint=*mIt;
			bestLambda=hr.getParameter();
			}
		}
	
	return bestPoint;
	}

void AlignTrackingMarkers::addLine(const AlignTrackingMarkers::Point& p1,const AlignTrackingMarkers::Point& p2)
	{
	/* Store the line: */
	lines.push_back(std::make_pair(p1,p2));
	
	Vrui::requestUpdate();
	}

void AlignTrackingMarkers::setOrigin(const AlignTrackingMarkers::Point& p)
	{
	transform.leftMultiply(ONTransform::translate(p-transform.getOrigin()));
	transform.renormalize();
	
	Vrui::requestUpdate();
	}

void AlignTrackingMarkers::removeLastLineCallback(Misc::CallbackData* cbData)
	{
	if(!lines.empty())
		lines.pop_back();
	
	Vrui::requestUpdate();
	}

void AlignTrackingMarkers::alignXAxisCallback(Misc::CallbackData* cbData)
	{
	if(!lines.empty())
		{
		/* Get the last line's direction in current system coordinates: */
		Vector line=transform.inverseTransform(lines.back().second-lines.back().first);
		
		/* Align the coordinate system's X axis with the line direction: */
		transform*=ONTransform::rotate(ONTransform::Rotation::rotateFromTo(Vector(1,0,0),line));
		transform.renormalize();
		}
	
	Vrui::requestUpdate();
	}

void AlignTrackingMarkers::alignNegXAxisCallback(Misc::CallbackData* cbData)
	{
	if(!lines.empty())
		{
		/* Get the last line's direction in current system coordinates: */
		Vector line=transform.inverseTransform(lines.back().second-lines.back().first);
		
		/* Align the coordinate system's -X axis with the line direction: */
		transform*=ONTransform::rotate(ONTransform::Rotation::rotateFromTo(Vector(-1,0,0),line));
		transform.renormalize();
		}
	
	Vrui::requestUpdate();
	}

void AlignTrackingMarkers::alignYAxisCallback(Misc::CallbackData* cbData)
	{
	if(!lines.empty())
		{
		/* Get the last line's direction in current system coordinates: */
		Vector line=transform.inverseTransform(lines.back().second-lines.back().first);
		
		/* Align the coordinate system's Y axis with the line direction: */
		transform*=ONTransform::rotate(ONTransform::Rotation::rotateFromTo(Vector(0,1,0),line));
		transform.renormalize();
		}
	
	Vrui::requestUpdate();
	}

void AlignTrackingMarkers::alignNegYAxisCallback(Misc::CallbackData* cbData)
	{
	if(!lines.empty())
		{
		/* Get the last line's direction in current system coordinates: */
		Vector line=transform.inverseTransform(lines.back().second-lines.back().first);
		
		/* Align the coordinate system's -Y axis with the line direction: */
		transform*=ONTransform::rotate(ONTransform::Rotation::rotateFromTo(Vector(0,-1,0),line));
		transform.renormalize();
		}
	
	Vrui::requestUpdate();
	}

void AlignTrackingMarkers::alignZAxisCallback(Misc::CallbackData* cbData)
	{
	if(!lines.empty())
		{
		/* Get the last line's direction in current system coordinates: */
		Vector line=transform.inverseTransform(lines.back().second-lines.back().first);
		
		/* Align the coordinate system's Z axis with the line direction: */
		transform*=ONTransform::rotate(ONTransform::Rotation::rotateFromTo(Vector(0,0,1),line));
		transform.renormalize();
		}
	
	Vrui::requestUpdate();
	}

void AlignTrackingMarkers::alignNegZAxisCallback(Misc::CallbackData* cbData)
	{
	if(!lines.empty())
		{
		/* Get the last line's direction in current system coordinates: */
		Vector line=transform.inverseTransform(lines.back().second-lines.back().first);
		
		/* Align the coordinate system's -Z axis with the line direction: */
		transform*=ONTransform::rotate(ONTransform::Rotation::rotateFromTo(Vector(0,0,-1),line));
		transform.renormalize();
		}
	
	Vrui::requestUpdate();
	}

void AlignTrackingMarkers::moveOriginCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	moveOrigin=cbData->set;
	}

/*************
Main function:
*************/

int main(int argc,char* argv[])
	{
	try
		{
		char** appDefaults=0;
		AlignTrackingMarkers app(argc,argv,appDefaults);
		app.run();
		}
	catch(std::runtime_error err)
		{
		std::cerr<<"Caught exception "<<err.what()<<std::endl;
		return 1;
		}
	
	return 0;
	}
