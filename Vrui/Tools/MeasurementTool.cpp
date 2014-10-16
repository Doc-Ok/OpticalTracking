/***********************************************************************
MeasurementTool - Tool to measure positions, distances and angles in
physical or navigational coordinates.
Copyright (c) 2006-2013 Oliver Kreylos

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

#include <Vrui/Tools/MeasurementTool.h>

#include <stdio.h>
#include <vector>
#include <Misc/File.h>
#include <Misc/CreateNumberedFileName.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/CompoundValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Geometry/LinearUnit.h>
#include <Geometry/GeometryValueCoders.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/Blind.h>
#include <GLMotif/Separator.h>
#include <GLMotif/Label.h>
#include <GLMotif/ToggleButton.h>
#include <GLMotif/TextField.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/WidgetStateHelper.h>
#include <Vrui/Vrui.h>
#include <Vrui/CoordinateTransform.h>
#include <Vrui/ToolManager.h>
#include <Vrui/DisplayState.h>

namespace Misc {

/****************************************
Helper class to decode measurement modes:
****************************************/

template <>
class ValueCoder<Vrui::MeasurementToolFactory::MeasurementMode>
	{
	/* Methods: */
	public:
	static std::string encode(const Vrui::MeasurementToolFactory::MeasurementMode& value)
		{
		switch(value)
			{
			case Vrui::MeasurementToolFactory::POSITION:
				return "Position";
			
			case Vrui::MeasurementToolFactory::DISTANCE:
				return "Distance";
			
			case Vrui::MeasurementToolFactory::ANGLE:
				return "Angle";
			
			default:
				return ""; // Never reached; just to make compiler happy
			}
		}
	static Vrui::MeasurementToolFactory::MeasurementMode decode(const char* start,const char* end,const char** decodeEnd =0)
		{
		if(end-start>=8&&strncasecmp(start,"Position",8)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+8;
			return Vrui::MeasurementToolFactory::POSITION;
			}
		else if(end-start>=8&&strncasecmp(start,"Distance",8)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+8;
			return Vrui::MeasurementToolFactory::DISTANCE;
			}
		else if(end-start>=5&&strncasecmp(start,"Angle",5)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+5;
			return Vrui::MeasurementToolFactory::ANGLE;
			}
		else
			throw DecodingError(std::string("Unable to convert \"")+std::string(start,end)+std::string("\" to MeasurementToolFactory::MeasurementMode"));
		}
	};

/***************************************
Helper class to decode coordinate modes:
***************************************/

template <>
class ValueCoder<Vrui::MeasurementToolFactory::CoordinateMode>
	{
	/* Methods: */
	public:
	static std::string encode(const Vrui::MeasurementToolFactory::CoordinateMode& value)
		{
		switch(value)
			{
			case Vrui::MeasurementToolFactory::PHYSICAL:
				return "Physical";
			
			case Vrui::MeasurementToolFactory::NAVIGATIONAL:
				return "Navigational";
			
			case Vrui::MeasurementToolFactory::USER:
				return "User";
			
			default:
				return ""; // Never reached; just to make compiler happy
			}
		}
	static Vrui::MeasurementToolFactory::CoordinateMode decode(const char* start,const char* end,const char** decodeEnd =0)
		{
		if(end-start>=8&&strncasecmp(start,"Physical",8)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+8;
			return Vrui::MeasurementToolFactory::PHYSICAL;
			}
		else if(end-start>=12&&strncasecmp(start,"Navigational",12)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+12;
			return Vrui::MeasurementToolFactory::NAVIGATIONAL;
			}
		else if(end-start>=4&&strncasecmp(start,"User",4)==0)
			{
			if(decodeEnd!=0)
				*decodeEnd=start+4;
			return Vrui::MeasurementToolFactory::USER;
			}
		else
			throw DecodingError(std::string("Unable to convert \"")+std::string(start,end)+std::string("\" to MeasurementToolFactory::CoordinateMode"));
		}
	};

}

namespace Vrui {

/***************************************
Methods of class MeasurementToolFactory:
***************************************/

MeasurementToolFactory::MeasurementToolFactory(ToolManager& toolManager)
	:ToolFactory("MeasurementTool",toolManager),
	 defaultMeasurementMode(POSITION),
	 defaultCoordinateMode(USER),
	 markerSize(getUiSize()),
	 saveMeasurements(false),
	 measurementFileName("MeasurementTool.dat"),
	 measurementFile(0)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* toolFactory=toolManager.loadClass("UtilityTool");
	toolFactory->addChildClass(this);
	addParentClass(toolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	defaultMeasurementMode=cfs.retrieveValue<MeasurementMode>("./defaultMeasurementMode",defaultMeasurementMode);
	defaultCoordinateMode=cfs.retrieveValue<CoordinateMode>("./defaultCoordinateMode",defaultCoordinateMode);
	markerSize=cfs.retrieveValue<Scalar>("./markerSize",markerSize);
	saveMeasurements=cfs.retrieveValue<bool>("./saveMeasurements",saveMeasurements);
	measurementFileName=cfs.retrieveString("./measurementFileName",measurementFileName);
	
	/* Set tool class' factory pointer: */
	MeasurementTool::factory=this;
	}

MeasurementToolFactory::~MeasurementToolFactory(void)
	{
	/* Close the measurement file: */
	delete measurementFile;
	
	/* Reset tool class' factory pointer: */
	MeasurementTool::factory=0;
	}

const char* MeasurementToolFactory::getName(void) const
	{
	return "Measurement Tool";
	}

const char* MeasurementToolFactory::getButtonFunction(int) const
	{
	return "Measure";
	}

Tool* MeasurementToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new MeasurementTool(this,inputAssignment);
	}

void MeasurementToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveMeasurementToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("UtilityTool");
	}

extern "C" ToolFactory* createMeasurementToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	MeasurementToolFactory* measurementToolFactory=new MeasurementToolFactory(*toolManager);
	
	/* Return factory object: */
	return measurementToolFactory;
	}

extern "C" void destroyMeasurementToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/****************************************
Static elements of class MeasurementTool:
****************************************/

MeasurementToolFactory* MeasurementTool::factory=0;

/********************************
Methods of class MeasurementTool:
********************************/

Vector MeasurementTool::calcDist(int i0,int i1) const
	{
	if(coordinateMode==MeasurementToolFactory::USER)
		return userTransform->inverseTransform(points[i1])-userTransform->inverseTransform(points[i0]);
	else
		return points[i1]-points[i0];
	}

void MeasurementTool::resetTool(void)
	{
	/* Reset the measurement state: */
	numPoints=0;
	
	/* Clear all coordinate fields: */
	for(int i=0;i<3;++i)
		for(int j=0;j<3;++j)
			pos[i][j]->setString("");
	for(int i=0;i<2;++i)
		dist[i]->setString("");
	angle->setString("");
	}

void MeasurementTool::updateUnits(void)
	{
	const char* xUnit="";
	const char* yUnit="";
	const char* zUnit="";
	const char* dUnit="";
	
	switch(coordinateMode)
		{
		case MeasurementToolFactory::PHYSICAL:
			/* Try to glean spatial units from Vrui's inchScale or meterScale settings: */
			if(Vrui::getInchFactor()==Scalar(1))
				xUnit=yUnit=zUnit=dUnit="in";
			else if(Vrui::getMeterFactor()==Scalar(1))
				xUnit=yUnit=zUnit=dUnit="m";
			else if(Vrui::getMeterFactor()==Scalar(100))
				xUnit=yUnit=zUnit=dUnit="cm";
			else if(Vrui::getMeterFactor()==Scalar(1000))
				xUnit=yUnit=zUnit=dUnit="mm";
			linearUnitScale=Scalar(1);
			break;
		
		case MeasurementToolFactory::NAVIGATIONAL:
			/* Set all spatial units to the coordinate manager's units: */
			xUnit=yUnit=zUnit=dUnit=getCoordinateManager()->getUnit().getAbbreviation();
			linearUnitScale=getCoordinateManager()->getUnit().getFactor();
			break;
		
		case MeasurementToolFactory::USER:
			/* Get positional unit names from the coordinate transformer: */
			xUnit=userTransform->getUnitAbbreviation(0);
			yUnit=userTransform->getUnitAbbreviation(1);
			zUnit=userTransform->getUnitAbbreviation(2);
			
			/* Distance unit is still navigational: */
			dUnit=getCoordinateManager()->getUnit().getAbbreviation();
			linearUnitScale=getCoordinateManager()->getUnit().getFactor();
			break;
		}
	
	/* Set all unit labels: */
	for(int i=0;i<3;++i)
		{
		posUnit[i][0]->setString(xUnit);
		posUnit[i][1]->setString(yUnit);
		posUnit[i][2]->setString(zUnit);
		}
	for(int i=0;i<2;++i)
		distUnit[i]->setString(dUnit);
	}

void MeasurementTool::updateCurrentPoint(void)
	{
	/* Update the current position display: */
	if(coordinateMode==MeasurementToolFactory::USER)
		{
		for(int j=0;j<3;++j)
			pos[numPoints-1][j]->setValue(double(points[numPoints-1][j])); // Don't apply linear unit scale to user coordinates
		}
	else
		{
		for(int j=0;j<3;++j)
			pos[numPoints-1][j]->setValue(double(points[numPoints-1][j])*linearUnitScale);
		}
	
	/* Update the current distance display: */
	if(numPoints>=2)
		{
		/* Calculate the distance between the last and the first measurement points: */
		Scalar distValue=calcDist(0,numPoints-1).mag();
		dist[numPoints-2]->setValue(double(distValue)*linearUnitScale);
		}

	/* Update the current angle display: */
	if(numPoints==3)
		{
		/* Calculate the angle between the three measurement points: */
		Vector d1=calcDist(0,1);
		Vector d2=calcDist(0,2);
		Scalar angleValue=(d1*d2)/(d1.mag()*d2.mag());
		if(angleValue<=Scalar(-1))
			angleValue=Scalar(180);
		else if(angleValue>=Scalar(1))
			angleValue=Scalar(0);
		else
			angleValue=Math::deg(Math::acos(angleValue));
		angle->setValue(double(angleValue));
		}
	}

void MeasurementTool::changeMeasurementModeCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData)
	{
	/* Change the measurement mode: */
	switch(cbData->radioBox->getToggleIndex(cbData->newSelectedToggle))
		{
		case 0:
			measurementMode=MeasurementToolFactory::POSITION;
			numMeasurementPoints=1;
			break;
		
		case 1:
			measurementMode=MeasurementToolFactory::DISTANCE;
			numMeasurementPoints=2;
			break;
		
		case 2:
			measurementMode=MeasurementToolFactory::ANGLE;
			numMeasurementPoints=3;
			break;
		}
	
	/* Reset the tool: */
	resetTool();
	}

void MeasurementTool::changeCoordinateModeCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData)
	{
	/* Change the coordinate mode: */
	switch(cbData->radioBox->getToggleIndex(cbData->newSelectedToggle))
		{
		case 0:
			coordinateMode=MeasurementToolFactory::PHYSICAL;
			break;
		
		case 1:
			coordinateMode=MeasurementToolFactory::NAVIGATIONAL;
			break;
		
		case 2:
			if(userTransform!=0)
				coordinateMode=MeasurementToolFactory::USER;
			else
				coordinateMode=MeasurementToolFactory::NAVIGATIONAL;
			break;
		}
	
	/* Reset the tool and update the displayed units: */
	resetTool();
	updateUnits();
	}

void MeasurementTool::posTextFieldLayoutChangedCallback(GLMotif::TextField::LayoutChangedCallbackData* cbData)
	{
	/* Find the index of the pos field: */
	int i,j;
	for(i=0;i<3;++i)
		for(j=0;j<3;++j)
			if(pos[i][j]==cbData->textField)
				goto found;
	found:
	
	/* Update the field's number format to use the extra space: */
	int cw=cbData->charWidth;
	if(cw<12)
		cw=12;
	cbData->textField->setPrecision(cw/2);
	
	/* Check if the text field has a current value: */
	if(i<numPoints)
		{
		/* Update the text field's display: */
		if(coordinateMode==MeasurementToolFactory::USER)
			pos[i][j]->setValue(double(points[i][j])); // Don't apply linear unit scale to user units
		else
			pos[i][j]->setValue(double(points[i][j])*double(linearUnitScale));
		}
	}

void MeasurementTool::coordTransformChangedCallback(CoordinateManager::CoordinateTransformChangedCallbackData* cbData)
	{
	/* Update the measurement dialog: */
	if(userTransform==0&&cbData->newTransform!=0)
		{
		/* Create a new toggle to select user coordinate mode: */
		coordinateModes->addToggle("User");
		}
	else if(userTransform!=0&&cbData->newTransform==0)
		{
		/* Go back to navigational coordinate mode if in user mode: */
		if(coordinateMode==MeasurementToolFactory::USER)
			{
			coordinateMode=MeasurementToolFactory::NAVIGATIONAL;
			coordinateModes->setSelectedToggle(1);
			}
		
		/* Remove the user coordinate mode toggle: */
		coordinateModes->removeWidgets(2);
		}
	
	/* Update the user transformation: */
	userTransform=cbData->newTransform;
	
	/* Reset the tool: */
	resetTool();
	updateUnits();
	}

void MeasurementTool::printPosition(Misc::File& file,const Point& pos) const
	{
	if(coordinateMode==MeasurementToolFactory::USER)
		fprintf(file.getFilePtr()," (%16.12g, %16.12g, %16.12g)\n",pos[0],pos[1],pos[2]); // Don't apply linear unit scale to user units
	else
		fprintf(file.getFilePtr()," (%16.12g, %16.12g, %16.12g)\n",pos[0]*linearUnitScale,pos[1]*linearUnitScale,pos[2]*linearUnitScale);
	}

namespace {

/****************
Helper functions:
****************/

GLMotif::RowColumn* createPosBox(const char* name,GLMotif::Container* parent,GLMotif::TextField* fields[3],GLMotif::Label* units[3])
	{
	GLMotif::RowColumn* result=new GLMotif::RowColumn(name,parent,false);
	result->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	result->setPacking(GLMotif::RowColumn::PACK_GRID);
	
	for(int i=0;i<3;++i)
		{
		/* Create a row column widget for the field value and unit: */
		char fieldName[40];
		snprintf(fieldName,sizeof(fieldName),"Pos-%d",i+1);
		GLMotif::RowColumn* fieldBox=new GLMotif::RowColumn(fieldName,result,false);
		fieldBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
		
		/* Create the field value text field: */
		fields[i]=new GLMotif::TextField("Field",fieldBox,12);
		fields[i]->setPrecision(6);
		
		fieldBox->setColumnWeight(0,1.0f);
		
		/* Create the unit label: */
		units[i]=new GLMotif::Label("Unit",fieldBox,"");
		
		fieldBox->manageChild();
		}
	
	result->manageChild();
	
	return result;
	}

GLMotif::RowColumn* createDistBox(const char* name,GLMotif::Container* parent,GLMotif::TextField*& field,GLMotif::Label*& unit)
	{
	GLMotif::RowColumn* result=new GLMotif::RowColumn(name,parent,false);
	result->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	result->setPacking(GLMotif::RowColumn::PACK_GRID);
	
	/* Create a row column widget for the field value and unit: */
	GLMotif::RowColumn* fieldBox=new GLMotif::RowColumn("Dist",result,false);
	fieldBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	
	/* Create the field value text field: */
	field=new GLMotif::TextField("Field",fieldBox,16);
	field->setPrecision(10);
	
	fieldBox->setColumnWeight(0,1.0f);
	
	/* Create the unit label: */
	unit=new GLMotif::Label("Unit",fieldBox,"");
	
	fieldBox->manageChild();
	
	/* Create a blind to keep the distance field half-size: */
	new GLMotif::Blind("Blind",result);
	
	result->manageChild();
	
	return result;
	}

}

MeasurementTool::MeasurementTool(const ToolFactory* sFactory,const ToolInputAssignment& inputAssignment)
	:UtilityTool(sFactory,inputAssignment),
	 measurementDialogPopup(0),
	 coordinateModes(0),
	 angle(0),
	 measurementMode(factory->defaultMeasurementMode),
	 coordinateMode(factory->defaultCoordinateMode),
	 userTransform(getCoordinateManager()->getCoordinateTransform()),
	 numPoints(0),dragging(false)
	{
	for(int i=0;i<3;++i)
		for(int j=0;j<3;++j)
			{
			pos[i][j]=0;
			posUnit[i][j]=0;
			}
	for(int i=0;i<2;++i)
		{
		dist[i]=0;
		distUnit[i]=0;
		}
	
	/* Don't use user coordinate mode if there are no user coordinates: */
	if(coordinateMode==MeasurementToolFactory::USER&&userTransform==0)
		coordinateMode=MeasurementToolFactory::NAVIGATIONAL;
	
	/* Create the measurement dialog window: */
	measurementDialogPopup=new GLMotif::PopupWindow("MeasurementDialogPopup",getWidgetManager(),"Measurement Dialog");
	measurementDialogPopup->setResizableFlags(true,false);
	
	GLMotif::RowColumn* measurementDialog=new GLMotif::RowColumn("MeasurementDialog",measurementDialogPopup,false);
	
	GLMotif::RowColumn* modeBox=new GLMotif::RowColumn("ModeBox",measurementDialog,false);
	modeBox->setNumMinorWidgets(2);
	
	new GLMotif::Label("MeasurementMode",modeBox,"Measurement Mode");
	
	GLMotif::RadioBox* measurementModes=new GLMotif::RadioBox("MeasurementModes",modeBox,false);
	measurementModes->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	measurementModes->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	measurementModes->setAlignment(GLMotif::Alignment::LEFT);
	measurementModes->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
	
	measurementModes->addToggle("Position");
	measurementModes->addToggle("Distance");
	measurementModes->addToggle("Angle");
	
	switch(measurementMode)
		{
		case MeasurementToolFactory::POSITION:
			measurementModes->setSelectedToggle(0);
			numMeasurementPoints=1;
			break;
		
		case MeasurementToolFactory::DISTANCE:
			measurementModes->setSelectedToggle(1);
			numMeasurementPoints=2;
			break;
		
		case MeasurementToolFactory::ANGLE:
			measurementModes->setSelectedToggle(2);
			numMeasurementPoints=3;
			break;
		}
	measurementModes->getValueChangedCallbacks().add(this,&MeasurementTool::changeMeasurementModeCallback);
	measurementModes->manageChild();
	
	new GLMotif::Label("CoordinateMode",modeBox,"Coordinate Mode");
	
	coordinateModes=new GLMotif::RadioBox("CoordinateModes",modeBox,false);
	coordinateModes->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	coordinateModes->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	coordinateModes->setAlignment(GLMotif::Alignment::LEFT);
	coordinateModes->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
	
	coordinateModes->addToggle("Physical");
	coordinateModes->addToggle("Navigational");
	if(userTransform!=0)
		coordinateModes->addToggle("User");
	
	switch(coordinateMode)
		{
		case MeasurementToolFactory::PHYSICAL:
			coordinateModes->setSelectedToggle(0);
			break;
		
		case MeasurementToolFactory::NAVIGATIONAL:
			coordinateModes->setSelectedToggle(1);
			break;
		
		case MeasurementToolFactory::USER:
			coordinateModes->setSelectedToggle(2);
			break;
		}
	coordinateModes->getValueChangedCallbacks().add(this,&MeasurementTool::changeCoordinateModeCallback);
	coordinateModes->manageChild();
	
	modeBox->manageChild();
	
	new GLMotif::Separator("Separator1",measurementDialog,GLMotif::Separator::HORIZONTAL,0.0f,GLMotif::Separator::LOWERED);
	
	GLMotif::RowColumn* measurementBox=new GLMotif::RowColumn("MeasurementBox",measurementDialog,false);
	measurementBox->setNumMinorWidgets(2);
	
	new GLMotif::Label("Pos1Label",measurementBox,"Position 1");
	
	createPosBox("Pos1Box",measurementBox,pos[0],posUnit[0]);
	
	new GLMotif::Label("Pos2Label",measurementBox,"Position 2");
	
	createPosBox("Pos1Box",measurementBox,pos[1],posUnit[1]);
	
	new GLMotif::Label("Dist1Label",measurementBox,"Distance 1");
	
	createDistBox("Dist1Box",measurementBox,dist[0],distUnit[0]);
	
	new GLMotif::Label("Pos3Label",measurementBox,"Position 3");
	
	createPosBox("Pos1Box",measurementBox,pos[2],posUnit[2]);
	
	new GLMotif::Label("Dist2Label",measurementBox,"Distance 2");
	
	createDistBox("Dist2Box",measurementBox,dist[1],distUnit[1]);
	
	new GLMotif::Label("AngleLabel",measurementBox,"Angle");
	
	GLMotif::Label* angleUnit=0;
	createDistBox("AngleBox",measurementBox,angle,angleUnit);
	angleUnit->setString("deg");
	
	/* Attach layout callbacks to all position text fields: */
	for(int i=0;i<3;++i)
		for(int j=0;j<3;++j)
			pos[i][j]->getLayoutChangedCallbacks().add(this,&MeasurementTool::posTextFieldLayoutChangedCallback);

	measurementBox->manageChild();
	
	measurementDialog->manageChild();
	
	/* Initialize the tool's state: */
	resetTool();
	updateUnits();
	
	/* Pop up the measurement dialog: */
	popupPrimaryWidget(measurementDialogPopup);
	
	/* Register a callback with the coordinate manager: */
	getCoordinateManager()->getCoordinateTransformChangedCallbacks().add(this,&MeasurementTool::coordTransformChangedCallback);
	}

MeasurementTool::~MeasurementTool(void)
	{
	/* Unregister the callback from the coordinate manager: */
	getCoordinateManager()->getCoordinateTransformChangedCallbacks().remove(this,&MeasurementTool::coordTransformChangedCallback);
	
	/* Delete the measurement dialog: */
	delete measurementDialogPopup;
	}

void MeasurementTool::configure(const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Read the measurement mode: */
	measurementMode=configFileSection.retrieveValue<MeasurementToolFactory::MeasurementMode>("./measurementMode",measurementMode);
	switch(measurementMode)
		{
		case MeasurementToolFactory::POSITION:
			numMeasurementPoints=1;
			break;
		
		case MeasurementToolFactory::DISTANCE:
			numMeasurementPoints=2;
			break;
		
		case MeasurementToolFactory::ANGLE:
			numMeasurementPoints=3;
			break;
		}
	
	/* Read the coordinate mode: */
	coordinateMode=configFileSection.retrieveValue<MeasurementToolFactory::CoordinateMode>("./coordinateMode",coordinateMode);
	if(coordinateMode==MeasurementToolFactory::USER&&userTransform==0)
		coordinateMode=MeasurementToolFactory::NAVIGATIONAL;
	
	/* Reset the tool and update the displayed units: */
	resetTool();
	updateUnits();
	
	/* Read the measurement dialog's position, orientation, and size: */
	GLMotif::readTopLevelPosition(measurementDialogPopup,configFileSection);
	
	/* Read and copy the saved measurement points: */
	std::vector<Point> savedPoints=configFileSection.retrieveValue<std::vector<Point> >("./points",std::vector<Point>());
	for(std::vector<Point>::iterator spIt=savedPoints.begin();spIt!=savedPoints.end()&&numPoints<numMeasurementPoints;++spIt)
		{
		/* Move to the next point: */
		++numPoints;
		
		/* Store the point: */
		points[numPoints-1]=*spIt;
		
		/* Update the display: */
		updateCurrentPoint();
		}
	}

void MeasurementTool::storeState(Misc::ConfigurationFileSection& configFileSection) const
	{
	/* Write the measurement mode and coordinate mode: */
	configFileSection.storeValue<MeasurementToolFactory::MeasurementMode>("./measurementMode",measurementMode);
	configFileSection.storeValue<MeasurementToolFactory::CoordinateMode>("./coordinateMode",coordinateMode);
	
	/* Write the measurement dialog's current position, orientation, and size: */
	GLMotif::writeTopLevelPosition(measurementDialogPopup,configFileSection);
	
	if(numPoints>0)
		{
		/* Store the currently measured points: */
		std::vector<Point> savedPoints;
		for(int i=0;i<numPoints;++i)
			savedPoints.push_back(points[i]);
		configFileSection.storeValue<std::vector<Point> >("./points",savedPoints);
		}
	}

const ToolFactory* MeasurementTool::getFactory(void) const
	{
	return factory;
	}

void MeasurementTool::buttonCallback(int,InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState) // Button has just been pressed
		{
		/* Go to the next measurement point: */
		if(numPoints==numMeasurementPoints)
			{
			/* Reset the tool: */
			resetTool();
			}
		++numPoints;
		
		/* Start dragging the current measurement point: */
		dragging=true;
		}
	else // Button has just been released
		{
		/* Stop dragging the current measurement point: */
		dragging=false;
		
		/* Check if a measurement entry has to be written to the measurement file: */
		if(factory->saveMeasurements&&numPoints==numMeasurementPoints&&isMaster())
			{
			/* Ensure the measurement file is open: */
			if(factory->measurementFile==0)
				{
				try
					{
					/* Create a uniquely named file based on the base name: */
					char numberedFileName[1024];
					factory->measurementFile=new Misc::File(Misc::createNumberedFileName(factory->measurementFileName.c_str(),4,numberedFileName),"wt");
					}
				catch(Misc::File::OpenError)
					{
					/* Just don't open the file, then! */
					showErrorMessage("Measurement Tool","Could not create measurement file; measurements will not be saved");
					}
				}
			
			/* Write the measurement to file if there is a file: */
			if(factory->measurementFile!=0)
				{
				switch(coordinateMode)
					{
					case MeasurementToolFactory::PHYSICAL:
						fprintf(factory->measurementFile->getFilePtr(),"Physical");
						break;
					
					case MeasurementToolFactory::NAVIGATIONAL:
						fprintf(factory->measurementFile->getFilePtr(),"Navigational");
						break;
					
					case MeasurementToolFactory::USER:
						fprintf(factory->measurementFile->getFilePtr(),"User");
						break;
					}
				
				switch(measurementMode)
					{
					case MeasurementToolFactory::POSITION:
						fprintf(factory->measurementFile->getFilePtr()," position");
						printPosition(*factory->measurementFile,points[0]);
						break;
					
					case MeasurementToolFactory::DISTANCE:
						fprintf(factory->measurementFile->getFilePtr()," distance");
						printPosition(*factory->measurementFile,points[0]);
						printPosition(*factory->measurementFile,points[1]);
						fprintf(factory->measurementFile->getFilePtr()," %16.12g\n",calcDist(0,1).mag()*linearUnitScale);
						break;
					
					case MeasurementToolFactory::ANGLE:
						{
						fprintf(factory->measurementFile->getFilePtr()," angle   ");
						printPosition(*factory->measurementFile,points[0]);
						printPosition(*factory->measurementFile,points[1]);
						Vector d1=calcDist(0,1);
						Scalar d1Len=Geometry::mag(d1);
						fprintf(factory->measurementFile->getFilePtr()," %16.12g",d1Len*linearUnitScale);
						printPosition(*factory->measurementFile,points[2]);
						Vector d2=calcDist(0,2);
						Scalar d2Len=Geometry::mag(d2);
						fprintf(factory->measurementFile->getFilePtr()," %16.12g",d2Len*linearUnitScale);
						Scalar angleValue=(d1*d2)/(d1Len*d2Len);
						if(angleValue<=Scalar(-1))
							angleValue=Scalar(180);
						else if(angleValue>=Scalar(1))
							angleValue=Scalar(0);
						else
							angleValue=Math::deg(Math::acos(angleValue));
						fprintf(factory->measurementFile->getFilePtr()," %16.12g\n",angleValue);
						break;
						}
					}
				
				/* Flush the file: */
				fflush(factory->measurementFile->getFilePtr());
				}
			}
		}
	}

void MeasurementTool::frame(void)
	{
	if(dragging)
		{
		/* Calculate the device position in the appropriate coordinate system: */
		points[numPoints-1]=getButtonDevicePosition(0);
		if(coordinateMode!=MeasurementToolFactory::PHYSICAL)
			{
			/* Transform the physical point to navigational coordinates: */
			points[numPoints-1]=getInverseNavigationTransformation().transform(points[numPoints-1]);
			
			if(coordinateMode==MeasurementToolFactory::USER)
				{
				/* Transform the navigational point to user coordinates: */
				points[numPoints-1]=userTransform->transform(points[numPoints-1]);
				}
			}
		
		/* Update the measurement dialog: */
		updateCurrentPoint();
		}
	}

void MeasurementTool::display(GLContextData& contextData) const
	{
	/* Set up and save OpenGL state: */
	GLboolean lightingEnabled=glIsEnabled(GL_LIGHTING);
	if(lightingEnabled)
		glDisable(GL_LIGHTING);
	GLfloat lineWidth;
	glGetFloatv(GL_LINE_WIDTH,&lineWidth);
	
	/* Calculate the marker size: */
	Scalar markerSize=factory->markerSize;
	
	/* Determine the marker color: */
	Color bgColor=getBackgroundColor();
	Color fgColor;
	for(int i=0;i<3;++i)
		fgColor[i]=1.0f-bgColor[i];
	fgColor[3]=bgColor[3];
	
	/* Transform all marker positions to physical coordinates: */
	Point physPoints[3];
	for(int i=0;i<3;++i)
		physPoints[i]=points[i];
	if(coordinateMode!=MeasurementToolFactory::PHYSICAL)
		{
		if(coordinateMode==MeasurementToolFactory::USER)
			{
			/* Transform points from user space to navigational space: */
			for(int i=0;i<3;++i)
				physPoints[i]=userTransform->inverseTransform(physPoints[i]);
			}
		
		/* Transform points from navigational space to physical space: */
		for(int i=0;i<3;++i)
			physPoints[i]=getNavigationTransformation().transform((physPoints[i]));
		}
	
	#if 1
	
	/* Draw a halo around the measurement markers and lines: */
	glLineWidth(3.0f);
	glColor(bgColor);
	glBegin(GL_LINES);
	
	/* Mark all measurement points: */
	for(int i=0;i<numPoints;++i)
		{
		glVertex(physPoints[i][0]-markerSize,physPoints[i][1],physPoints[i][2]);
		glVertex(physPoints[i][0]+markerSize,physPoints[i][1],physPoints[i][2]);
		glVertex(physPoints[i][0],physPoints[i][1]-markerSize,physPoints[i][2]);
		glVertex(physPoints[i][0],physPoints[i][1]+markerSize,physPoints[i][2]);
		glVertex(physPoints[i][0],physPoints[i][1],physPoints[i][2]-markerSize);
		glVertex(physPoints[i][0],physPoints[i][1],physPoints[i][2]+markerSize);
		}
	
	/* Draw all distance lines: */
	for(int i=1;i<numPoints;++i)
		{
		glVertex(physPoints[0]);
		glVertex(physPoints[i]);
		}
	
	glEnd();
	
	#endif
	
	/* Draw the markers and measurement lines themselves: */
	glLineWidth(1.0f);
	glColor(fgColor);
	glBegin(GL_LINES);
	
	/* Mark all measurement points: */
	for(int i=0;i<numPoints;++i)
		{
		glVertex(physPoints[i][0]-markerSize,physPoints[i][1],physPoints[i][2]);
		glVertex(physPoints[i][0]+markerSize,physPoints[i][1],physPoints[i][2]);
		glVertex(physPoints[i][0],physPoints[i][1]-markerSize,physPoints[i][2]);
		glVertex(physPoints[i][0],physPoints[i][1]+markerSize,physPoints[i][2]);
		glVertex(physPoints[i][0],physPoints[i][1],physPoints[i][2]-markerSize);
		glVertex(physPoints[i][0],physPoints[i][1],physPoints[i][2]+markerSize);
		}
	
	/* Draw all distance lines: */
	for(int i=1;i<numPoints;++i)
		{
		glVertex(physPoints[0]);
		glVertex(physPoints[i]);
		}
	
	glEnd();
	
	/* Restore OpenGL state: */
	glLineWidth(lineWidth);
	if(lightingEnabled)
		glEnable(GL_LIGHTING);
	}

}
