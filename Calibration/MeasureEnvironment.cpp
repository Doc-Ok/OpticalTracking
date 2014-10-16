/***********************************************************************
MeasureEnvironment - Utility for guided surveys of a single-screen
VR environment using a Total Station.
Copyright (c) 2009-2013 Oliver Kreylos

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

#include "MeasureEnvironment.h"

#include <stdlib.h>
#include <utility>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <Misc/ThrowStdErr.h>
#include <IO/TokenSource.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <Math/Matrix.h>
#include <Geometry/AffineCombiner.h>
#include <Geometry/Vector.h>
#include <Geometry/ProjectiveTransformation.h>
#include <Geometry/PCACalculator.h>
#include <Geometry/PointPicker.h>
#include <Geometry/RayPicker.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/PopupMenu.h>
#include <GLMotif/Menu.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/Margin.h>
#include <GLMotif/Label.h>
#include <GLMotif/Button.h>
#include <Vrui/InputDevice.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/ToolManager.h>
#include <Vrui/DisplayState.h>
#include <Vrui/Vrui.h>
#include <Vrui/OpenFile.h>

#include "NaturalPointClient.h"
#include "PTransformFitter.h"
#include "LevenbergMarquardtMinimizer.h"

namespace {

/****************
Helper functions:
****************/

template <class ScalarParam,int dimensionParam>
inline
std::ostream&
operator<<(
	std::ostream& os,
	const Geometry::Point<ScalarParam,dimensionParam>& p)
	{
	std::streamsize width=os.width();
	os<<std::setw(width)<<p[0];
	for(int i=1;i<dimensionParam;++i)
		os<<','<<std::setw(width)<<p[i];
	return os;
	}

}

/*************************************************************
Static elements of class MeasureEnvironment::PointSnapperTool:
*************************************************************/

MeasureEnvironment::PointSnapperToolFactory* MeasureEnvironment::PointSnapperTool::factory=0;

/*****************************************************
Methods of class MeasureEnvironment::PointSnapperTool:
*****************************************************/

MeasureEnvironment::PointSnapperTool::PointSnapperTool(const Vrui::ToolFactory* factory,const Vrui::ToolInputAssignment& inputAssignment)
	:Vrui::TransformTool(factory,inputAssignment)
	{
	/* Set the source device: */
	if(input.getNumButtonSlots()>0)
		sourceDevice=getButtonDevice(0);
	else
		sourceDevice=getValuatorDevice(0);
	}

void MeasureEnvironment::PointSnapperTool::initialize(void)
	{
	/* Initialize the base tool: */
	TransformTool::initialize();
	
	/* Disable the transformed device's glyph: */
	Vrui::getInputGraphManager()->getInputDeviceGlyph(transformedDevice).disable();
	}

const Vrui::ToolFactory* MeasureEnvironment::PointSnapperTool::getFactory(void) const
	{
	return factory;
	}

void MeasureEnvironment::PointSnapperTool::frame(void)
	{
	/* Pick a point: */
	PickResult pr;
	if(sourceDevice->isRayDevice())
		{
		/* Calculate the device's selection ray in navigational coordinates: */
		Ray ray(sourceDevice->getRay());
		ray.transform(Vrui::getInverseNavigationTransformation());
		
		/* Pick a point along a ray: */
		pr=application->pickPoint(ray);
		}
	else
		{
		/* Calculate the device's selection position in navigational coordinates: */
		Point pos(Vrui::getInverseNavigationTransformation().transform(sourceDevice->getPosition()));
		
		/* Pick a point: */
		pr=application->pickPoint(pos);
		}
	
	/* Move the device's origin to the picked point: */
	Point devicePos(sourceDevice->getPosition());
	devicePos=application->snapToPoint(devicePos,pr);
	
	/* Set the transformed device's position to the intersection point: */
	transformedDevice->setDeviceRay(sourceDevice->getDeviceRayDirection(),sourceDevice->getDeviceRayStart());
	Vrui::TrackerState ts(Vrui::getNavigationTransformation().transform(devicePos)-Vrui::Point::origin,sourceDevice->getOrientation());
	transformedDevice->setTransformation(ts);
	}

/***********************************************************
Static elements of class MeasureEnvironment::PointQueryTool:
***********************************************************/

MeasureEnvironment::PointQueryToolFactory* MeasureEnvironment::PointQueryTool::factory=0;

/***************************************************
Methods of class MeasureEnvironment::PointQueryTool:
***************************************************/

void MeasureEnvironment::PointQueryTool::textFieldLayoutChangedCallback(GLMotif::TextField::LayoutChangedCallbackData* cbData)
	{
	/* Find the index of the position field: */
	for(int i=0;i<3;++i)
		if(position[i]==cbData->textField)
			{
			/* Update the field's number format to use the extra space: */
			int cw=cbData->charWidth;
			if(cw<8)
				cw=8;
			cbData->textField->setPrecision(cw/2);
			
			/* Check if the text field has a current value: */
			if(pickResult!=~PickResult(0))
				{
				/* Update the text field's display: */
				Point pos=application->snapToPoint(Point::origin,pickResult);
				position[i]->setValue(pos[i]);
				}
			}
	}

void MeasureEnvironment::PointQueryTool::deleteCallback(Misc::CallbackData* cbData)
	{
	/* Check if there is a currently selected point: */
	if(pickResult!=~PickResult(0))
		{
		/* Delete the currently selected point: */
		application->deletePoint(pickResult);
		
		/* Reset the data dialog: */
		pickResult=~PickResult(0);
		pointType->setString("");
		for(int i=0;i<3;++i)
			position[i]->setString("");
		}
	}

MeasureEnvironment::PointQueryTool::PointQueryTool(const Vrui::ToolFactory* factory,const Vrui::ToolInputAssignment& inputAssignment)
	:Vrui::Tool(factory,inputAssignment),
	 dialogPopup(0),
	 dragging(false),pickResult(~PickResult(0))
	{
	/* Create the data dialog window: */
	dialogPopup=new GLMotif::PopupWindow("PointQueryToolDialogPopup",Vrui::getWidgetManager(),"Point Data");
	dialogPopup->setResizableFlags(true,false);
	
	GLMotif::RowColumn* dialog=new GLMotif::RowColumn("Dialog",dialogPopup,false);
	dialog->setOrientation(GLMotif::RowColumn::VERTICAL);
	dialog->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	dialog->setNumMinorWidgets(1);
	
	GLMotif::RowColumn* data=new GLMotif::RowColumn("Data",dialog,false);
	data->setOrientation(GLMotif::RowColumn::VERTICAL);
	data->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	data->setNumMinorWidgets(2);
	
	/* Create a text field to display the current point's type: */
	new GLMotif::Label("PointTypeLabel",data,"Point Type");
	
	pointType=new GLMotif::TextField("PointType",data,10);
	
	/* Create a triple of text fields to display the current point's position: */
	new GLMotif::Label("PointPositionLabel",data,"Position");
	
	GLMotif::RowColumn* pointPosition=new GLMotif::RowColumn("PointPosition",data,false);
	pointPosition->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	pointPosition->setPacking(GLMotif::RowColumn::PACK_GRID);
	pointPosition->setNumMinorWidgets(1);
	
	for(int i=0;i<3;++i)
		{
		position[i]=new GLMotif::TextField("Position",pointPosition,8);
		position[i]->setFloatFormat(GLMotif::TextField::SMART);
		position[i]->setPrecision(4);
		position[i]->getLayoutChangedCallbacks().add(this,&MeasureEnvironment::PointQueryTool::textFieldLayoutChangedCallback);
		}
	
	pointPosition->manageChild();
	
	data->manageChild();
	
	/* Add a button to delete a point: */
	GLMotif::Margin* buttonsMargin=new GLMotif::Margin("ButtonsMargin",dialog,false);
	buttonsMargin->setAlignment(GLMotif::Alignment::RIGHT);
	
	GLMotif::RowColumn* buttons=new GLMotif::RowColumn("Buttons",buttonsMargin,false);
	
	GLMotif::Button* deleteButton=new GLMotif::Button("DeleteButton",buttons,"Delete");
	deleteButton->getSelectCallbacks().add(this,&MeasureEnvironment::PointQueryTool::deleteCallback);
	
	buttons->manageChild();
	
	buttonsMargin->manageChild();
	
	dialog->manageChild();
	
	/* Pop up the data dialog: */
	Vrui::popupPrimaryWidget(dialogPopup);
	}

MeasureEnvironment::PointQueryTool::~PointQueryTool(void)
	{
	/* Delete the data dialog: */
	delete dialogPopup;
	}

void MeasureEnvironment::PointQueryTool::buttonCallback(int,Vrui::InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState)
		{
		/* Start dragging: */
		dragging=true;
		}
	else
		{
		/* Stop dragging: */
		dragging=false;
		}
	}

void MeasureEnvironment::PointQueryTool::frame(void)
	{
	if(dragging)
		{
		/* Get pointer to input device: */
		Vrui::InputDevice* iDevice=getButtonDevice(0);
		
		/* Pick a point: */
		Vrui::NavTrackerState transform=Vrui::getDeviceTransformation(iDevice);
		Point pos=transform.getOrigin();
		PickResult newPickResult;
		if(iDevice->isRayDevice())
			newPickResult=application->pickPoint(Ray(pos,transform.transform(iDevice->getDeviceRayDirection())));
		else
			newPickResult=application->pickPoint(pos);
		
		/* Check if the pick result has changed: */
		if(pickResult!=newPickResult)
			{
			/* Update the data dialog: */
			if(newPickResult!=~PickResult(0))
				{
				/* Get the point type (and index): */
				std::pair<int,int> pointClass=application->classifyPickResult(newPickResult);
				
				/* Update the data dialog: */
				switch(pointClass.first)
					{
					case 0:
						pointType->setString("Floor");
						break;
					
					case 1:
						pointType->setString("Screen");
						break;
					
					case 2:
						pointType->setString("Ball");
						break;
					}
				Point ppos=application->snapToPoint(Point::origin,newPickResult);
				for(int i=0;i<3;++i)
					position[i]->setValue(ppos[i]);
				}
			else
				{
				/* Reset the data dialog: */
				pointType->setString("");
				for(int i=0;i<3;++i)
					position[i]->setString("");
				}
			
			pickResult=newPickResult;
			}
		}
	}

void MeasureEnvironment::PointQueryTool::display(GLContextData& contextData) const
	{
	/* Check if there is a current point: */
	if(pickResult!=~PickResult(0))
		{
		/* Set up and save OpenGL state: */
		glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
		glDisable(GL_LIGHTING);
		glLineWidth(1.0f);
		
		/* Calculate the marker size: */
		double markerSize=Vrui::getUiSize()/Vrui::getNavigationTransformation().getScaling();
		
		/* Go to navigational coordinates: */
		glPushMatrix();
		glLoadIdentity();
		glMultMatrix(Vrui::getDisplayState(contextData).modelviewNavigational);
		
		/* Determine the marker color: */
		Vrui::Color bgColor=Vrui::getBackgroundColor();
		Vrui::Color fgColor;
		for(int i=0;i<3;++i)
			fgColor[i]=1.0f-bgColor[i];
		fgColor[3]=bgColor[3];
		
		Point pos=application->snapToPoint(Point::origin,pickResult);
		glBegin(GL_LINES);
		glColor(fgColor);
		glVertex(pos[0]-markerSize,pos[1],pos[2]);
		glVertex(pos[0]+markerSize,pos[1],pos[2]);
		glVertex(pos[0],pos[1]-markerSize,pos[2]);
		glVertex(pos[0],pos[1]+markerSize,pos[2]);
		glVertex(pos[0],pos[1],pos[2]-markerSize);
		glVertex(pos[0],pos[1],pos[2]+markerSize);
		glEnd();
		
		/* Restore OpenGL state: */
		glPopMatrix();
		glPopAttrib();
		}
	}

/***********************************
Methods of class MeasureEnvironment:
***********************************/

MeasureEnvironment::PTransform MeasureEnvironment::calcNormalization(const MeasureEnvironment::PointList& points)
	{
	Point::AffineCombiner centroidC;
	for(PointList::const_iterator pIt=points.begin();pIt!=points.end();++pIt)
		centroidC.addPoint(*pIt);
	Point centroid=centroidC.getPoint();
	
	double averageDist=0.0;
	for(PointList::const_iterator pIt=points.begin();pIt!=points.end();++pIt)
		averageDist+=Geometry::dist(centroid,*pIt);
	averageDist/=double(points.size());
	
	PTransform result=PTransform::scale(Math::sqrt(2.0)/averageDist);
	result*=PTransform::translateToOriginFrom(centroid);
	return result;
	}

GLMotif::PopupMenu* MeasureEnvironment::createMainMenu(void)
	{
	/* Create a popup shell to hold the main menu: */
	GLMotif::PopupMenu* mainMenuPopup=new GLMotif::PopupMenu("MainMenuPopup",Vrui::getWidgetManager());
	mainMenuPopup->setTitle("Survey Buddy");
	
	GLMotif::Menu* mainMenu=new GLMotif::Menu("MainMenu",mainMenuPopup,false);
	
	GLMotif::RadioBox* measuringModes=new GLMotif::RadioBox("MeasuringModes",mainMenu,false);
	measuringModes->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
	
	measuringModes->addToggle("Measure Floor");
	measuringModes->addToggle("Measure Screen");
	measuringModes->addToggle("Measure Balls");
	
	measuringModes->setSelectedToggle(measuringMode);
	measuringModes->getValueChangedCallbacks().add(this,&MeasureEnvironment::changeMeasuringModeCallback);
	
	measuringModes->manageChild();
	
	GLMotif::Button* loadMeasurementFileButton=new GLMotif::Button("LoadMeasurementFileButton",mainMenu,"Load Measurement File");
	loadMeasurementFileButton->getSelectCallbacks().add(this,&MeasureEnvironment::loadMeasurementFileCallback);
	
	GLMotif::Button* loadOptitrackSampleFileButton=new GLMotif::Button("LoadOptitrackSampleFileButton",mainMenu,"Load Optitrack Sample File");
	loadOptitrackSampleFileButton->getSelectCallbacks().add(this,&MeasureEnvironment::loadOptitrackSampleFileCallback);
	
	GLMotif::Button* saveMeasurementFileButton=new GLMotif::Button("SaveMeasurementFileButton",mainMenu,"Save Measurement File");
	saveMeasurementFileButton->getSelectCallbacks().add(this,&MeasureEnvironment::saveMeasurementFileCallback);
	
	GLMotif::Button* createTransformationButton=new GLMotif::Button("CreateTransformationButton",mainMenu,"Create World Transformation");
	createTransformationButton->getSelectCallbacks().add(this,&MeasureEnvironment::createTransformationCallback);
	
	GLMotif::Button* fitScreenTransformationButton=new GLMotif::Button("FitScreenTransformationButton",mainMenu,"Fit Screen Transformation");
	fitScreenTransformationButton->getSelectCallbacks().add(this,&MeasureEnvironment::fitScreenTransformationCallback);
	
	/* Finish building the main menu: */
	mainMenu->manageChild();
	return mainMenuPopup;
	}

void* MeasureEnvironment::pointCollectorThreadMethod(void)
	{
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	// Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	while(true)
		{
		/* Read the next measurement point from the Total Station: */
		Point p=pointTransform.transform(totalStation->readNextMeasurement());
		
		/* Store the point: */
		{
		Threads::Mutex::Lock measuringLock(measuringMutex);
		switch(measuringMode)
			{
			case 0:
				floorPoints.push_back(p);
				measurementsDirty=true;
				break;
			
			case 1:
				screenPoints.push_back(p);
				measurementsDirty=true;
				break;
			
			case 2:
				{
				if(naturalPointClient!=0)
					{
					/* Request a frame from the NaturalPoint server: */
					const NaturalPointClient::Frame& frame=naturalPointClient->waitForNextFrame();
					if(frame.otherMarkers.size()==1)
						{
						Point tp=frame.otherMarkers[0];
						if(naturalPointFlipZ)
							tp[2]=-tp[2];
						trackerPoints.push_back(pointTransform.transform(tp));
						ballPoints.push_back(p);
						}
					else
						{
						/* Ignore the measurement and show an error message: */
						char message[256];
						snprintf(message,sizeof(message),"OptiTrack delivered %u points; ignoring measurement",(unsigned int)(frame.otherMarkers.size()));
						Vrui::showErrorMessage("NaturalPoint Client",message);
						}
					}
				else
					ballPoints.push_back(p);
				measurementsDirty=true;
				break;
				}
			}
		}
		
		Vrui::requestUpdate();
		}
	
	return 0;
	}

void MeasureEnvironment::loadMeasurementFile(IO::Directory& directory,const char* fileName)
	{
	Threads::Mutex::Lock measuringLock(measuringMutex);
	
	/* Open the input file: */
	IO::TokenSource tok(directory.openFile(fileName));
	tok.setPunctuation(",\n");
	tok.setQuotes("\"");
	tok.skipWs();
	
	while(!tok.eof())
		{
		Point p;
		for(int i=0;i<3;++i)
			{
			if(i>0)
				{
				tok.readNextToken();
				if(!tok.isToken(","))
					Misc::throwStdErr("MeasureEnvironment::MeasureEnvironment: Format error in input file %s",fileName);
				}
			p[i]=Scalar(atof(tok.readNextToken()));
			}
		
		tok.readNextToken();
		if(!tok.isToken(","))
			Misc::throwStdErr("MeasureEnvironment::MeasureEnvironment: Format error in input file %s",fileName);
		
		tok.readNextToken();
		if(tok.isCaseToken("FLOOR"))
			floorPoints.push_back(p);
		else if(tok.isCaseToken("SCREEN"))
			screenPoints.push_back(p);
		else if(tok.isCaseToken("BALLS"))
			ballPoints.push_back(p);
		else
			Misc::throwStdErr("MeasureEnvironment::MeasureEnvironment: Unknown point tag \"%s\" in input file %s",tok.getToken(),fileName);
		
		tok.readNextToken();
		if(!tok.isToken("\n"))
			Misc::throwStdErr("MeasureEnvironment::MeasureEnvironment: Format error in input file %s",fileName);
		}
	measurementsDirty=true;
	}

void MeasureEnvironment::saveMeasurementFile(const char* fileName)
	{
	Threads::Mutex::Lock measuringLock(measuringMutex);
	
	/* Save all measured points: */
	std::ofstream pointFile(fileName);
	pointFile.setf(std::ios::fixed);
	pointFile<<std::setprecision(6);
	for(PointList::const_iterator fpIt=floorPoints.begin();fpIt!=floorPoints.end();++fpIt)
		pointFile<<std::setw(12)<<*fpIt<<",\"FLOOR\""<<std::endl;
	for(PointList::const_iterator spIt=screenPoints.begin();spIt!=screenPoints.end();++spIt)
		pointFile<<std::setw(12)<<*spIt<<",\"SCREEN\""<<std::endl;
	for(PointList::const_iterator bpIt=ballPoints.begin();bpIt!=ballPoints.end();++bpIt)
		pointFile<<std::setw(12)<<*bpIt<<",\"BALLS\""<<std::endl;
	
	if(naturalPointClient!=0)
		{
		/* Save all Optitrack sample points: */
		std::ofstream pointFile("TrackingPoints.csv");
		pointFile.setf(std::ios::fixed);
		pointFile<<std::setprecision(6);
		size_t i=0;
		for(PointList::const_iterator tpIt=trackerPoints.begin();tpIt!=trackerPoints.end();++tpIt)
			{
			pointFile<<1<<","<<std::setw(4)<<i*10<<","<<std::setw(12)<<(*tpIt)[0]<<","<<std::setw(12)<<(*tpIt)[1]<<","<<std::setw(12)<<(*tpIt)[2]<<std::endl;
			++i;
			}
		}
	
	measurementsDirty=false;
	}

void MeasureEnvironment::loadOptitrackSampleFile(IO::Directory& directory,const char* fileName,bool flipZ)
	{
	Threads::Mutex::Lock measuringLock(measuringMutex);
	
	/* Open the CSV input file: */
	IO::TokenSource tok(directory.openFile(fileName));
	tok.setPunctuation(",\n");
	tok.setQuotes("\"");
	tok.skipWs();
	
	/* Read all point records from the file: */
	double lastTimeStamp=Math::Constants<double>::min;
	Point::AffineCombiner pac;
	unsigned int numPoints=0;
	unsigned int line=1;
	while(!tok.eof())
		{
		/* Read a point record: */
		
		/* Read the marker index: */
		int markerIndex=atoi(tok.readNextToken());
		
		if(strcmp(tok.readNextToken(),",")!=0)
			Misc::throwStdErr("readOptitrackSampleFile: missing comma in line %u",line);
		
		/* Read the sample timestamp: */
		double timeStamp=atof(tok.readNextToken());
		
		/* Read the point position: */
		Point p;
		for(int i=0;i<3;++i)
			{
			if(strcmp(tok.readNextToken(),",")!=0)
				Misc::throwStdErr("readOptitrackSampleFile: missing comma in line %u",line);
			
			p[i]=Scalar(atof(tok.readNextToken()));
			}
		
		if(flipZ)
			{
			/* Invert the z component to flip to a right-handed coordinate system: */
			p[2]=-p[2];
			}
		
		if(strcmp(tok.readNextToken(),"\n")!=0)
			Misc::throwStdErr("readOptitrackSampleFile: overlong point record in line %u",line);
		
		/* Check if the point record is valid: */
		if(markerIndex==1)
			{
			/* Check if this record started a new sampling sequence: */
			if(timeStamp>=lastTimeStamp+5.0&&numPoints>0)
				{
				/* Get the current average point position and reset the accumulator: */
				trackerPoints.push_back(pac.getPoint());
				pac.reset();
				numPoints=0;
				}
			
			/* Add the point to the current accumulator: */
			pac.addPoint(p);
			++numPoints;
			
			lastTimeStamp=timeStamp;
			}
		}
	
	/* Get the last average point position: */
	if(numPoints>0)
		trackerPoints.push_back(pac.getPoint());
	}

MeasureEnvironment::MeasureEnvironment(int& argc,char**& argv,char**& appDefaults)
	:Vrui::Application(argc,argv,appDefaults),
	 totalStation(0),basePrismOffset(34.4),initialPrismOffset(0),
	 naturalPointClient(0),naturalPointFlipZ(false),
	 pointTransform(OGTransform::identity),
	 measuringMode(0),
	 ballRadius(TotalStation::Scalar(25.4/4.0)),
	 gridSize(300),
	 measurementsDirty(false),
	 mainMenu(0)
	{
	/* Register the point snapper tool class with the Vrui tool manager: */
	PointSnapperToolFactory* pointSnapperToolFactory=new PointSnapperToolFactory("PointSnapperTool","Snap To Points",Vrui::getToolManager()->loadClass("TransformTool"),*Vrui::getToolManager());
	pointSnapperToolFactory->setNumButtons(0,true);
	pointSnapperToolFactory->setNumValuators(0,true);
	pointSnapperToolFactory->setButtonFunction(0,"Transformed Button");
	pointSnapperToolFactory->setValuatorFunction(0,"Transformed Valuator");
	Vrui::getToolManager()->addClass(pointSnapperToolFactory,Vrui::ToolManager::defaultToolFactoryDestructor);
	
	/* Register the point query tool class with the Vrui tool manager: */
	PointQueryToolFactory* pointQueryToolFactory=new PointQueryToolFactory("PointQueryTool","Query Points",0,*Vrui::getToolManager());
	pointQueryToolFactory->setNumButtons(1);
	pointQueryToolFactory->setButtonFunction(0,"Query Point");
	Vrui::getToolManager()->addClass(pointQueryToolFactory,Vrui::ToolManager::defaultToolFactoryDestructor);
	
	/* Parse the command line: */
	const char* totalStationDeviceName=0;
	const char* naturalPointServerName=0;
	int naturalPointCommandPort=1510;
	const char* naturalPointDataAddress="224.0.0.1";
	int naturalPointDataPort=1511;
	int totalStationBaudRate=19200;
	const char* measurementFileName=0;
	TotalStation::Scalar totalStationUnitScale=TotalStation::Scalar(1);
	screenPixelSize[0]=screenPixelSize[1]=-1;
	for(int i=1;i<argc;++i)
		{
		if(argv[i][0]=='-')
			{
			if(strcasecmp(argv[i]+1,"t")==0)
				{
				++i;
				if(i<argc)
					totalStationDeviceName=argv[i];
				else
					std::cerr<<"MeasureEnvironment: Ignoring dangling command line switch "<<argv[i-1]<<std::endl;
				}
			else if(strcasecmp(argv[i]+1,"baudRate")==0)
				{
				++i;
				if(i<argc)
					totalStationBaudRate=atoi(argv[i]);
				else
					std::cerr<<"MeasureEnvironment: Ignoring dangling command line switch "<<argv[i-1]<<std::endl;
				}
			else if(strcasecmp(argv[i]+1,"unitScale")==0)
				{
				++i;
				if(i<argc)
					totalStationUnitScale=TotalStation::Scalar(atof(argv[i]));
				else
					std::cerr<<"MeasureEnvironment: Ignoring dangling command line switch "<<argv[i-1]<<std::endl;
				}
			else if(strcasecmp(argv[i]+1,"prismOffset")==0)
				{
				++i;
				if(i<argc)
					basePrismOffset=TotalStation::Scalar(atof(argv[i]));
				else
					std::cerr<<"MeasureEnvironment: Ignoring dangling command line switch "<<argv[i-1]<<std::endl;
				}
			else if(strcasecmp(argv[i]+1,"ballRadius")==0)
				{
				++i;
				if(i<argc)
					ballRadius=TotalStation::Scalar(atof(argv[i]));
				else
					std::cerr<<"MeasureEnvironment: Ignoring dangling command line switch "<<argv[i-1]<<std::endl;
				}
			else if(strcasecmp(argv[i]+1,"npc")==0)
				{
				++i;
				if(i<argc)
					naturalPointServerName=argv[i];
				else
					std::cerr<<"MeasureEnvironment: Ignoring dangling command line switch "<<argv[i-1]<<std::endl;
				}
			else if(strcasecmp(argv[i]+1,"flipZ")==0)
				naturalPointFlipZ=true;
			else if(strcasecmp(argv[i]+1,"screenSize")==0)
				{
				if(i+2<argc)
					{
					for(int j=0;j<2;++j)
						screenPixelSize[j]=atoi(argv[i+1+j]);
					}
				else
					std::cerr<<"MeasureEnvironment: Ignoring dangling command line switch "<<argv[i]<<std::endl;
				i+=2;
				}
			else if(strcasecmp(argv[i]+1,"gridSize")==0)
				{
				++i;
				if(i<argc)
					gridSize=atoi(argv[i]);
				else
					std::cerr<<"MeasureEnvironment: Ignoring dangling command line switch "<<argv[i-1]<<std::endl;
				}
			else
				std::cerr<<"MeasureEnvironment: Unrecognized command line switch "<<argv[i]<<std::endl;
			}
		else if(measurementFileName==0)
			measurementFileName=argv[i];
		else
			std::cerr<<"MeasureEnvironment: Ignoring command line argument "<<argv[i]<<std::endl;
		}
	
	if(totalStationDeviceName!=0)
		{
		/* Connect to the Total Station: */
		totalStation=new TotalStation(totalStationDeviceName,totalStationBaudRate);
		totalStation->setUnitScale(totalStationUnitScale);
		
		/* Store the initial prism offset: */
		initialPrismOffset=totalStation->getPrismOffset();
		
		/* Set the prism offset to zero for floor or screen measurements: */
		totalStation->setPrismOffset(basePrismOffset);
		
		/* Start the point recording thread: */
		totalStation->startRecording();
		pointCollectorThread.start(this,&MeasureEnvironment::pointCollectorThreadMethod);
		}
	
	if(naturalPointServerName!=0)
		{
		/* Connect to the NaturalPoint server: */
		naturalPointClient=new NaturalPointClient(naturalPointServerName,naturalPointCommandPort,naturalPointDataAddress,naturalPointDataPort);
		}
	
	/* Import a measurement file if one is given: */
	if(measurementFileName!=0)
		{
		loadMeasurementFile(*Vrui::openDirectory("."),measurementFileName);
		measurementsDirty=false;
		}
	
	/* Create the main menu: */
	mainMenu=createMainMenu();
	Vrui::setMainMenu(mainMenu);
	}

MeasureEnvironment::~MeasureEnvironment(void)
	{
	delete mainMenu;
	
	if(totalStation!=0)
		{
		{
		/* Shut down the point collector thread: */
		Threads::Mutex::Lock measuringLock(measuringMutex);
		pointCollectorThread.cancel();
		pointCollectorThread.join();
		}
		
		/* Stop recording points: */
		totalStation->stopRecording();
		
		/* Reset the Total Station's prism offset to its initial value: */
		totalStation->setPrismOffset(initialPrismOffset);
		}
	delete totalStation;
	
	if(measurementsDirty)
		{
		/* Save all measured points: */
		saveMeasurementFile("MeasuredPoints.csv");
		}
	
	delete naturalPointClient;
	}

void MeasureEnvironment::frame(void)
	{
	}

void MeasureEnvironment::display(GLContextData& contextData) const
	{
	/* Set up OpenGL state: */
	glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT|GL_POINT_BIT);
	glDisable(GL_LIGHTING);
	glPointSize(3.0f);
	
	{
	Threads::Mutex::Lock measuringLock(measuringMutex);
	
	/* Draw all already collected points: */
	glBegin(GL_POINTS);
	glColor3f(1.0f,0.0f,0.0f);
	for(PointList::const_iterator fpIt=floorPoints.begin();fpIt!=floorPoints.end();++fpIt)
		glVertex(*fpIt);
	glColor3f(0.0f,1.0f,0.0f);
	for(PointList::const_iterator spIt=screenPoints.begin();spIt!=screenPoints.end();++spIt)
		glVertex(*spIt);
	glColor3f(1.0f,0.0f,1.0f);
	for(PointList::const_iterator bpIt=ballPoints.begin();bpIt!=ballPoints.end();++bpIt)
		glVertex(*bpIt);
	glEnd();
	
	}
	
	/* Reset OpenGL state: */
	glPopAttrib();
	}

MeasureEnvironment::PickResult MeasureEnvironment::pickPoint(const MeasureEnvironment::Point& point) const
	{
	Threads::Mutex::Lock measuringLock(measuringMutex);
	
	/* Create a point picker: */
	Geometry::PointPicker<Scalar,3> picker(point,Scalar(Vrui::getPointPickDistance()));
	
	/* Process all points: */
	for(PointList::const_iterator pIt=floorPoints.begin();pIt!=floorPoints.end();++pIt)
		picker(*pIt);
	for(PointList::const_iterator pIt=screenPoints.begin();pIt!=screenPoints.end();++pIt)
		picker(*pIt);
	for(PointList::const_iterator pIt=ballPoints.begin();pIt!=ballPoints.end();++pIt)
		picker(*pIt);
	
	/* Return the index of the picked point: */
	if(picker.havePickedPoint())
		return picker.getPickIndex();
	else
		return ~PickResult(0);
	}

MeasureEnvironment::PickResult MeasureEnvironment::pickPoint(const MeasureEnvironment::Ray& ray) const
	{
	Threads::Mutex::Lock measuringLock(measuringMutex);
	
	/* Create a ray picker: */
	Geometry::RayPicker<Scalar,3> picker(ray,Scalar(Vrui::getRayPickCosine()));
	
	/* Process all points: */
	for(PointList::const_iterator pIt=floorPoints.begin();pIt!=floorPoints.end();++pIt)
		picker(*pIt);
	for(PointList::const_iterator pIt=screenPoints.begin();pIt!=screenPoints.end();++pIt)
		picker(*pIt);
	for(PointList::const_iterator pIt=ballPoints.begin();pIt!=ballPoints.end();++pIt)
		picker(*pIt);
	
	/* Return the index of the picked point: */
	if(picker.havePickedPoint())
		return picker.getPickIndex();
	else
		return ~PickResult(0);
	}

std::pair<int,int> MeasureEnvironment::classifyPickResult(const MeasureEnvironment::PickResult& pickResult) const
	{
	Threads::Mutex::Lock measuringLock(measuringMutex);
	
	PickResult pr=pickResult;
	
	/* Check if the picked point is a floor point: */
	if(pr<floorPoints.size())
		return std::pair<int,int>(0,int(pr));
	pr-=floorPoints.size();
	
	/* Check if the picked point is a screen point: */
	if(pr<screenPoints.size())
		return std::pair<int,int>(1,int(pr));
	pr-=screenPoints.size();
	
	/* Check if the picked point is a ball point: */
	if(pr<ballPoints.size())
		return std::pair<int,int>(2,int(pr));
	pr-=screenPoints.size();
	
	/* Return an invalid classification: */
	return std::pair<int,int>(-1,-1);
	}

MeasureEnvironment::Point MeasureEnvironment::snapToPoint(const MeasureEnvironment::Point& point,const MeasureEnvironment::PickResult& pickResult) const
	{
	Threads::Mutex::Lock measuringLock(measuringMutex);
	
	PickResult pr=pickResult;
	
	/* Check if the picked point is a floor point: */
	if(pr<floorPoints.size())
		return floorPoints[pr];
	pr-=floorPoints.size();
	
	/* Check if the picked point is a screen point: */
	if(pr<screenPoints.size())
		return screenPoints[pr];
	pr-=screenPoints.size();
	
	/* Check if the picked point is a ball point: */
	if(pr<ballPoints.size())
		return ballPoints[pr];
	pr-=ballPoints.size();
	
	/* Otherwise return the unchanged position: */
	return point;
	}

void MeasureEnvironment::deletePoint(const MeasureEnvironment::PickResult& pickResult)
	{
	Threads::Mutex::Lock measuringLock(measuringMutex);
	
	PickResult pr=pickResult;
	
	/* Check if the picked point is a floor point: */
	if(pr<floorPoints.size())
		{
		floorPoints.erase(floorPoints.begin()+pr);
		measurementsDirty=true;
		return;
		}
	pr-=floorPoints.size();
	
	/* Check if the picked point is a screen point: */
	if(pr<screenPoints.size())
		{
		screenPoints.erase(screenPoints.begin()+pr);
		measurementsDirty=true;
		return;
		}
	pr-=screenPoints.size();
	
	/* Check if the picked point is a ball point: */
	if(pr<ballPoints.size())
		{
		ballPoints.erase(ballPoints.begin()+pr);
		if(naturalPointClient!=0)
			trackerPoints.erase(trackerPoints.begin()+pr);
		measurementsDirty=true;
		return;
		}
	pr-=ballPoints.size();
	}

void MeasureEnvironment::changeMeasuringModeCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData)
	{
	Threads::Mutex::Lock measuringLock(measuringMutex);
	
	/* Set the new measuring mode: */
	int newMeasuringMode=cbData->radioBox->getToggleIndex(cbData->newSelectedToggle);
	
	if(newMeasuringMode==2&&measuringMode!=2&&totalStation!=0)
		{
		/* Temporarily shut down the point collector thread: */
		pointCollectorThread.cancel();
		pointCollectorThread.join();
		
		/* Set the Total Station's prism offset to measure balls: */
		totalStation->setPrismOffset(basePrismOffset+ballRadius);
		
		/* Restart the point collector thread: */
		pointCollectorThread.start(this,&MeasureEnvironment::pointCollectorThreadMethod);
		}
	if(newMeasuringMode!=2&&measuringMode==2&&totalStation!=0)
		{
		/* Temporarily shut down the point collector thread: */
		pointCollectorThread.cancel();
		pointCollectorThread.join();
		
		/* Set the Total Station's prism offset to measure points on the floor or screen: */
		totalStation->setPrismOffset(basePrismOffset);
		
		/* Restart the point collector thread: */
		pointCollectorThread.start(this,&MeasureEnvironment::pointCollectorThreadMethod);
		}
	
	measuringMode=newMeasuringMode;
	
	if(measuringMode==1)
		screenPoints.clear();
	}

void MeasureEnvironment::loadMeasurementFileCallback(Misc::CallbackData* cbData)
	{
	/* Open a file selection dialog: */
	GLMotif::FileSelectionDialog* loadMeasurementFileDialog=new GLMotif::FileSelectionDialog(Vrui::getWidgetManager(),"Load Measurement File...",Vrui::openDirectory("."),".csv");
	loadMeasurementFileDialog->getOKCallbacks().add(this,&MeasureEnvironment::loadMeasurementFileOKCallback);
	loadMeasurementFileDialog->deleteOnCancel();
	
	/* Show the file selection dialog: */
	Vrui::popupPrimaryWidget(loadMeasurementFileDialog);
	}

void MeasureEnvironment::loadMeasurementFileOKCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData)
	{
	/* Load the selected measurement file: */
	loadMeasurementFile(*cbData->selectedDirectory,cbData->selectedFileName);
	
	/* Destroy the file selection dialog: */
	cbData->fileSelectionDialog->close();
	}

void MeasureEnvironment::loadOptitrackSampleFileCallback(Misc::CallbackData* cbData)
	{
	/* Open a file selection dialog: */
	GLMotif::FileSelectionDialog* loadOptitrackSampleFileDialog=new GLMotif::FileSelectionDialog(Vrui::getWidgetManager(),"Load Measurement File...",Vrui::openDirectory("."),".csv");
	loadOptitrackSampleFileDialog->getOKCallbacks().add(this,&MeasureEnvironment::loadOptitrackSampleFileOKCallback);
	loadOptitrackSampleFileDialog->deleteOnCancel();
	
	/* Show the file selection dialog: */
	Vrui::popupPrimaryWidget(loadOptitrackSampleFileDialog);
	}

void MeasureEnvironment::loadOptitrackSampleFileOKCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData)
	{
	/* Load the selected Optitrack sample file: */
	loadOptitrackSampleFile(*cbData->selectedDirectory,cbData->selectedFileName,naturalPointFlipZ);
	
	/* Destroy the file selection dialog: */
	cbData->fileSelectionDialog->close();
	}

void MeasureEnvironment::saveMeasurementFileCallback(Misc::CallbackData* cbData)
	{
	saveMeasurementFile("MeasuredPoints.csv");
	}

void MeasureEnvironment::createTransformationCallback(Misc::CallbackData* cbData)
	{
	/* Fit a plane to the floor points: */
	Geometry::PCACalculator<3> floorPca;
	for(PointList::const_iterator fpIt=floorPoints.begin();fpIt!=floorPoints.end();++fpIt)
		floorPca.accumulatePoint(*fpIt);
	Point floorCentroid=floorPca.calcCentroid();
	floorPca.calcCovariance();
	double floorEv[3];
	floorPca.calcEigenvalues(floorEv);
	Geometry::PCACalculator<3>::Vector floorNormal=floorPca.calcEigenvector(floorEv[2]);
	
	/* Fit a plane to the screen points: */
	Geometry::PCACalculator<3> screenPca;
	for(PointList::const_iterator spIt=screenPoints.begin();spIt!=screenPoints.end();++spIt)
		screenPca.accumulatePoint(*spIt);
	Point screenCentroid=screenPca.calcCentroid();
	screenPca.calcCovariance();
	double screenEv[3];
	screenPca.calcEigenvalues(screenEv);
	Geometry::PCACalculator<3>::Vector screenNormal=screenPca.calcEigenvector(screenEv[2]);
	
	/* Flip the floor normal such that it points towards the screen points: */
	if((screenCentroid-floorCentroid)*floorNormal<Scalar(0))
		floorNormal=-floorNormal;
	
	#if 0
	/* Flip the screen normal such that it points away from the ball points: */
	Point::AffineCombiner ballC;
	for(PointList::const_iterator bpIt=ballPoints.begin();bpIt!=ballPoints.end();++bpIt)
		ballC.addPoint(*bpIt);
	if((ballC.getPoint()-screenCentroid)*screenNormal>Scalar(0))
		screenNormal=-screenNormal;
	#endif
	
	/* Project the screen centroid onto the floor plane to get the coordinate system origin: */
	Point origin=screenCentroid-floorNormal*(((screenCentroid-floorCentroid)*floorNormal)/Geometry::sqr(floorNormal));
	
	/* Orthonormalize the screen normal against the floor normal: */
	Point::Vector y=screenNormal-floorNormal*((screenNormal*floorNormal)/Geometry::sqr(floorNormal));
	Point::Vector x=y^floorNormal;
	
	/* Calculate a rotation to align the floor normal with +z and the (horizontal) screen normal with +y: */
	OGTransform::Rotation rot=OGTransform::Rotation::fromBaseVectors(x,y);
	
	/*********************************************************************
	Calculate a transformation to move the Totalstation survey points into
	the normalized coordinate system:
	*********************************************************************/
	
	pointTransform=OGTransform(origin-Point::origin,rot,Scalar(1));
	pointTransform.doInvert();
	}

void MeasureEnvironment::fitScreenTransformationCallback(Misc::CallbackData* cbData)
	{
	typedef Geometry::ProjectiveTransformation<Scalar,3> PTransform;
	
	/* Create a list of theoretical screen points: */
	PointList idealPoints;
	int screenPixelOffset[2];
	for(int i=0;i<2;++i)
		screenPixelOffset[i]=((screenPixelSize[i]-1)%gridSize)/2;
	for(int y=screenPixelOffset[1];y<screenPixelSize[1];y+=gridSize)
		for(int x=screenPixelOffset[0];x<screenPixelSize[0];x+=gridSize)
			idealPoints.push_back(Point((Scalar(x)+Scalar(0.5))/Scalar(screenPixelSize[0]),Scalar(1)-(Scalar(y)+Scalar(0.5))/Scalar(screenPixelSize[1]),0));
	if(idealPoints.size()!=screenPoints.size())
		{
		std::cerr<<"Wrong number of screen points, got "<<screenPoints.size()<<" instead of "<<idealPoints.size()<<std::endl;
		return;
		}
	
	/* Calculate normalization transformations for the ideal and observed screen points: */
	PTransform idealNorm=calcNormalization(idealPoints);
	PTransform screenNorm=calcNormalization(screenPoints);
	
	/* Build the linear system: */
	Math::Matrix a(12,12,0.0);
	for(size_t index=0;index<idealPoints.size();++index)
		{
		/* Transform the theoretical and observed points: */
		Point ideal=idealNorm.transform(idealPoints[index]);
		Point screen=screenNorm.transform(screenPoints[index]);
		
		/* Enter the point pair's three linear equations into the matrix: */
		for(int i=0;i<3;++i)
			{
			double row[12];
			for(int k=0;k<12;++k)
				row[k]=0.0;
			for(int j=0;j<2;++j)
				{
				row[i*3+j]=ideal[j];
				row[3*3+j]=-screen[i]*ideal[j];
				}
			row[i*3+2]=1.0;
			row[3*3+2]=-screen[i];
			
			for(unsigned int k=0;k<12;++k)
				for(unsigned int l=0;l<12;++l)
					a(k,l)+=row[k]*row[l];
			}
		}
	
	/* Find the linear system's smallest eigenvalue: */
	std::pair<Math::Matrix,Math::Matrix> qe=a.jacobiIteration();
	unsigned int minEIndex=0;
	double minE=Math::abs(qe.second(0));
	std::cout<<qe.second(0);
	for(unsigned int i=1;i<12;++i)
		{
		std::cout<<", "<<qe.second(i);
		if(minE>Math::abs(qe.second(i)))
			{
			minEIndex=i;
			minE=Math::abs(qe.second(i));
			}
		}
	std::cout<<std::endl;
	
	/* Create the normalized homography: */
	PTransform nh;
	double scale=qe.first(11,minEIndex);
	for(int i=0;i<4;++i)
		{
		for(int j=0;j<2;++j)
			nh.getMatrix()(i,j)=qe.first(i*3+j,minEIndex)/scale;
		nh.getMatrix()(i,2)=i==2?1.0:0.0;
		nh.getMatrix()(i,3)=qe.first(i*3+2,minEIndex)/scale;
		}
	
	/* Un-normalize the homography: */
	PTransform pScreenTransform=Geometry::invert(screenNorm);
	pScreenTransform*=nh;
	pScreenTransform*=idealNorm;
	
	/* Refine the homography: */
	PTransformFitter ptf(idealPoints.size(),&idealPoints[0],&screenPoints[0]);
	ptf.setTransform(pScreenTransform);
	PTransformFitter::Scalar screenResult2=LevenbergMarquardtMinimizer<PTransformFitter>::minimize(ptf);
	std::cout<<"Projective transformation fitting final distance: "<<screenResult2<<std::endl;
	pScreenTransform=ptf.getTransform();
	
	/* Print the lengths of the screen's edges: */
	Point bl=pScreenTransform.transform(Point(0,0,0));
	Point br=pScreenTransform.transform(Point(1,0,0));
	Point tl=pScreenTransform.transform(Point(0,1,0));
	Point tr=pScreenTransform.transform(Point(1,1,0));
	std::cout<<"Bottom-left : "<<std::setw(10)<<bl<<std::endl;
	std::cout<<"Bottom-right: "<<std::setw(10)<<br<<std::endl;
	std::cout<<"Top-left    : "<<std::setw(10)<<tl<<std::endl;
	std::cout<<"Top-right   : "<<std::setw(10)<<tr<<std::endl;
	std::cout<<"Bottom edge: "<<Geometry::dist(bl,br)<<std::endl;
	std::cout<<"Top edge   : "<<Geometry::dist(tl,tr)<<std::endl;
	std::cout<<"Left edge  : "<<Geometry::dist(bl,tl)<<std::endl;
	std::cout<<"Right edge : "<<Geometry::dist(br,tr)<<std::endl;
	
	#if 0
	
	/* Find the best-fitting screen transformation for the measured screen points: */
	ScreenTransformFitter stf(screen.size(),&screen[0],&screenPoints[0]);
	ScreenTransformFitter::Scalar screenResult1=LevenbergMarquardtMinimizer<ScreenTransformFitter>::minimize(stf);
	std::cout<<"Screen transformation fitting final distance: "<<screenResult1<<std::endl;
	screenTransform=stf.getTransform();
	screenSize[0]=stf.getSize(0);
	screenSize[1]=stf.getSize(1);
	std::cout<<"Optimal screen size: "<<screenSize[0]<<", "<<screenSize[1]<<std::endl;
	std::cout<<"Optimal screen origin: "<<screenTransform.getOrigin()<<std::endl;
	std::cout<<"Optimal horizontal screen axis: "<<screenTransform.getDirection(0)<<std::endl;
	std::cout<<"Optimal vertical screen axis: "<<screenTransform.getDirection(1)<<std::endl;
	
	/*********************************************************************
	Calculate a homography matrix from the optimal screen transformation
	to the optimal projective transformation to correct screen
	misalignments:
	*********************************************************************/
	
	Point sCorners[4];
	Point pCorners[4];
	for(int i=0;i<4;++i)
		{
		sCorners[i][0]=i&0x1?screenSize[0]*unitScale:0.0;
		sCorners[i][1]=i&0x2?screenSize[1]*unitScale:0.0;
		sCorners[i][2]=0.0;
		pCorners[i][0]=i&0x1?1.0:0.0;
		pCorners[i][1]=i&0x2?1.0:0.0;
		pCorners[i][2]=0.0;
		pCorners[i]=screenTransform.inverseTransform(pScreenTransform.transform(pCorners[i]));
		pCorners[i][0]*=unitScale;
		pCorners[i][1]*=unitScale;
		}
	Geometry::ProjectiveTransformation<double,2> sHom=calcHomography(sCorners);
	Geometry::ProjectiveTransformation<double,2> pHom=calcHomography(pCorners);
	Geometry::ProjectiveTransformation<double,2> hom=pHom;
	hom.leftMultiply(Geometry::invert(sHom));
	for(int i=0;i<3;++i)
		for(int j=0;j<3;++j)
			hom.getMatrix()(i,j)/=hom.getMatrix()(2,2);
	
	#endif
	}

int main(int argc,char* argv[])
	{
	char** appDefaults=0;
	MeasureEnvironment app(argc,argv,appDefaults);
	app.run();
	
	return 0;
	}
