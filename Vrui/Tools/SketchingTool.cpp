/***********************************************************************
SketchingTool - Tool to create and edit 3D curves.
Copyright (c) 2009-2013 Oliver Kreylos

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

#include <Vrui/Tools/SketchingTool.h>

#include <Misc/SelfDestructArray.h>
#include <Misc/File.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <IO/ValueSource.h>
#include <Cluster/MulticastPipe.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/Label.h>
#include <GLMotif/Button.h>
#include <GLMotif/TextField.h>
#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>
#include <Vrui/DisplayState.h>
#include <Vrui/OpenFile.h>

namespace Vrui {

/*************************************
Methods of class SketchingToolFactory:
*************************************/

SketchingToolFactory::SketchingToolFactory(ToolManager& toolManager)
	:ToolFactory("SketchingTool",toolManager),
	 detailSize(getUiSize()),
	 curveFileName("SketchingTool.curves")
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* toolFactory=toolManager.loadClass("UtilityTool");
	toolFactory->addChildClass(this);
	addParentClass(toolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	detailSize=cfs.retrieveValue<Scalar>("./detailSize",detailSize);
	curveFileName=cfs.retrieveString("./curveFileName",curveFileName);
	
	/* Set tool class' factory pointer: */
	SketchingTool::factory=this;
	}

SketchingToolFactory::~SketchingToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	SketchingTool::factory=0;
	}

const char* SketchingToolFactory::getName(void) const
	{
	return "Curve Editor";
	}

const char* SketchingToolFactory::getButtonFunction(int) const
	{
	return "Draw Curves";
	}

Tool* SketchingToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new SketchingTool(this,inputAssignment);
	}

void SketchingToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveSketchingToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("UtilityTool");
	}

extern "C" ToolFactory* createSketchingToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	SketchingToolFactory* sketchingToolFactory=new SketchingToolFactory(*toolManager);
	
	/* Return factory object: */
	return sketchingToolFactory;
	}

extern "C" void destroySketchingToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/**************************************
Static elements of class SketchingTool:
**************************************/

SketchingToolFactory* SketchingTool::factory=0;
const SketchingTool::Curve::Color SketchingTool::curveColors[8]=
	{
	SketchingTool::Curve::Color(0,0,0),SketchingTool::Curve::Color(255,0,0),
	SketchingTool::Curve::Color(255,255,0),SketchingTool::Curve::Color(0,255,0),
	SketchingTool::Curve::Color(0,255,255),SketchingTool::Curve::Color(0,0,255),
	SketchingTool::Curve::Color(255,0,255),SketchingTool::Curve::Color(255,255,255)
	};

/******************************
Methods of class SketchingTool:
******************************/

SketchingTool::SketchingTool(const ToolFactory* sFactory,const ToolInputAssignment& inputAssignment)
	:UtilityTool(sFactory,inputAssignment),
	 controlDialogPopup(0),lineWidthValue(0),colorBox(0),
	 newLineWidth(3.0f),newColor(255,0,0),
	 active(false),
	 currentCurve(0),
	 curvesSelectionHelper(factory->curveFileName.c_str(),".curves",openDirectory("."))
	{
	/* Get the style sheet: */
	const GLMotif::StyleSheet* ss=getWidgetManager()->getStyleSheet();
	
	/* Build the tool control dialog: */
	controlDialogPopup=new GLMotif::PopupWindow("SketchingToolControlDialog",getWidgetManager(),"Curve Editor Settings");
	controlDialogPopup->setResizableFlags(false,false);
	
	GLMotif::RowColumn* controlDialog=new GLMotif::RowColumn("ControlDialog",controlDialogPopup,false);
	controlDialog->setNumMinorWidgets(1);

	GLMotif::RowColumn* settingsBox=new GLMotif::RowColumn("SettingsBox",controlDialog,false);
	settingsBox->setNumMinorWidgets(2);
	
	/* Create a slider to set the line width: */
	new GLMotif::Label("LineWidthLabel",settingsBox,"Line Width");
	
	GLMotif::RowColumn* lineWidthBox=new GLMotif::RowColumn("LineWidthBox",settingsBox,false);
	lineWidthBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	
	lineWidthValue=new GLMotif::TextField("LineWidthValue",lineWidthBox,4);
	lineWidthValue->setFloatFormat(GLMotif::TextField::FIXED);
	lineWidthValue->setPrecision(1);
	lineWidthValue->setValue(newLineWidth);
	
	GLMotif::Slider* lineWidthSlider=new GLMotif::Slider("LineWidthSlider",lineWidthBox,GLMotif::Slider::HORIZONTAL,ss->fontHeight*10.0f);
	lineWidthSlider->setValueRange(0.5,11.0,0.5);
	lineWidthSlider->setValue(newLineWidth);
	lineWidthSlider->getValueChangedCallbacks().add(this,&SketchingTool::lineWidthSliderCallback);
	
	lineWidthBox->manageChild();
	
	/* Create a radio box to set the line color: */
	new GLMotif::Label("ColorLabel",settingsBox,"Color");
	
	colorBox=new GLMotif::RowColumn("ColorBox",settingsBox,false);
	colorBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	colorBox->setPacking(GLMotif::RowColumn::PACK_GRID);
	colorBox->setAlignment(GLMotif::Alignment::LEFT);
	
	/* Add the color buttons: */
	for(int i=0;i<8;++i)
		{
		char colorButtonName[16];
		snprintf(colorButtonName,sizeof(colorButtonName),"ColorButton%d",i);
		GLMotif::NewButton* colorButton=new GLMotif::NewButton(colorButtonName,colorBox,GLMotif::Vector(ss->fontHeight,ss->fontHeight,0.0f));
		colorButton->setBackgroundColor(GLMotif::Color(curveColors[i]));
		colorButton->getSelectCallbacks().add(this,&SketchingTool::colorButtonSelectCallback);
		}
	
	colorBox->manageChild();
	
	settingsBox->manageChild();
	
	GLMotif::RowColumn* buttonBox=new GLMotif::RowColumn("ButtonBox",controlDialog,false);
	buttonBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	buttonBox->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	buttonBox->setAlignment(GLMotif::Alignment::RIGHT);
	
	GLMotif::Button* saveCurvesButton=new GLMotif::Button("SaveCurvesButton",buttonBox,"Save Curves...");
	curvesSelectionHelper.addSaveCallback(saveCurvesButton,this,&SketchingTool::saveCurvesCallback);
	
	GLMotif::Button* loadCurvesButton=new GLMotif::Button("LoadCurvesButton",buttonBox,"Load Curves...");
	curvesSelectionHelper.addLoadCallback(loadCurvesButton,this,&SketchingTool::loadCurvesCallback);
	
	GLMotif::Button* deleteAllCurvesButton=new GLMotif::Button("DeleteAllCurvesButton",buttonBox,"Delete All Curves");
	deleteAllCurvesButton->getSelectCallbacks().add(this,&SketchingTool::deleteAllCurvesCallback);
	
	buttonBox->manageChild();
	
	controlDialog->manageChild();
	
	/* Pop up the control dialog: */
	popupPrimaryWidget(controlDialogPopup);
	}

SketchingTool::~SketchingTool(void)
	{
	delete controlDialogPopup;
	
	/* Delete all curves: */
	for(std::vector<Curve*>::iterator cIt=curves.begin();cIt!=curves.end();++cIt)
		delete *cIt;
	}

void SketchingTool::buttonCallback(int,InputDevice::ButtonCallbackData* cbData)
	{
	/* Check if the button has just been pressed: */
	if(cbData->newButtonState)
		{
		/* Activate the tool: */
		active=true;
		
		/* Start a new curve: */
		currentCurve=new Curve;
		currentCurve->lineWidth=newLineWidth;
		currentCurve->color=newColor;
		curves.push_back(currentCurve);
		
		/* Append the curve's first control point: */
		Curve::ControlPoint cp;
		const NavTransform& invNav=getInverseNavigationTransformation();
		cp.pos=lastPoint=invNav.transform(getButtonDevicePosition(0));
		cp.t=getApplicationTime();
		currentCurve->controlPoints.push_back(cp);
		}
	else
		{
		/* Append the final control point to the curve: */
		Curve::ControlPoint cp;
		cp.pos=currentPoint;
		cp.t=getApplicationTime();
		currentCurve->controlPoints.push_back(cp);
		
		/* Deactivate the tool: */
		active=false;
		currentCurve=0;
		}
	}

void SketchingTool::frame(void)
	{
	if(active)
		{
		/* Get the current dragging point: */
		const NavTransform& invNav=getInverseNavigationTransformation();
		currentPoint=invNav.transform(getButtonDevicePosition(0));
		
		/* Check if the dragging point is far enough away from the most recent curve vertex: */
		if(Geometry::sqrDist(currentPoint,lastPoint)>=Math::sqr(factory->detailSize*invNav.getScaling()))
			{
			/* Append the current dragging point to the curve: */
			Curve::ControlPoint cp;
			cp.pos=currentPoint;
			cp.t=getApplicationTime();
			currentCurve->controlPoints.push_back(cp);
			
			/* Remember the last added point: */
			lastPoint=currentPoint;
			}
		}
	}

void SketchingTool::display(GLContextData& contextData) const
	{
	/* Set up OpenGL state: */
	glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
	glDisable(GL_LIGHTING);
	
	/* Go to navigational coordinates: */
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMultMatrix(getDisplayState(contextData).modelviewNavigational);
	
	/* Render all curves: */
	for(std::vector<Curve*>::const_iterator cIt=curves.begin();cIt!=curves.end();++cIt)
		{
		const Curve* curve=*cIt;
		glLineWidth(curve->lineWidth);
		glColor(curve->color);
		glBegin(GL_LINE_STRIP);
		for(std::vector<Curve::ControlPoint>::const_iterator cpIt=curve->controlPoints.begin();cpIt!=curve->controlPoints.end();++cpIt)
			glVertex(cpIt->pos);
		if(active&&curve==currentCurve)
			glVertex(currentPoint);
		glEnd();
		}
	
	/* Go back to physical coordinates: */
	glPopMatrix();
	
	/* Reset OpenGL state: */
	glPopAttrib();
	}

void SketchingTool::lineWidthSliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData)
	{
	/* Get the new line width: */
	newLineWidth=GLfloat(cbData->value);
	
	/* Update the line width value: */
	lineWidthValue->setValue(newLineWidth);
	}

void SketchingTool::colorButtonSelectCallback(GLMotif::NewButton::SelectCallbackData* cbData)
	{
	/* Set the new line color: */
	newColor=curveColors[colorBox->getChildIndex(cbData->button)];
	}

void SketchingTool::saveCurvesCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData)
	{
	if(isMaster())
		{
		try
			{
			/* Save all curves to a curve file: */
			Misc::File curveFile(cbData->getSelectedPath().c_str(),"w");
			FILE* cf=curveFile.getFilePtr();
			
			/* Write the curve file header: */
			fprintf(cf,"Vrui Curve Editor Tool Curve File\n");
			
			/* Write all curves: */
			fprintf(cf,"%u\n",(unsigned int)curves.size());
			for(std::vector<Curve*>::const_iterator cIt=curves.begin();cIt!=curves.end();++cIt)
				{
				const Curve* c=*cIt;
				
				/* Write the curve's line width and color: */
				fprintf(cf,"\n");
				fprintf(cf,"%4.1f, %03u %03u %03u\n",c->lineWidth,c->color[0],c->color[1],c->color[2]);
				
				/* Write the curve's control points: */
				fprintf(cf,"%u\n",(unsigned int)c->controlPoints.size());
				for(std::vector<Curve::ControlPoint>::const_iterator cpIt=c->controlPoints.begin();cpIt!=c->controlPoints.end();++cpIt)
					fprintf(cf,"%f, %f %f %f\n",cpIt->t,cpIt->pos[0],cpIt->pos[1],cpIt->pos[2]);
				}
			
			if(getMainPipe()!=0)
				{
				/* Send a status message to the slave nodes: */
				Misc::writeCString(0,*getMainPipe());
				}
			}
		catch(std::runtime_error err)
			{
			if(getMainPipe()!=0)
				{
				/* Send an error message to the slaves: */
				Misc::writeCString(err.what(),*getMainPipe());
				}
			
			/* Re-throw the exception: */
			throw;
			}
		}
	else
		{
		/* Receive a status message from the master node: */
		Misc::SelfDestructArray<char> error(Misc::readCString(*getMainPipe()));
		if(error.getArray()!=0)
			{
			/* Throw an exception: */
			throw std::runtime_error(error.getArray());
			}
		}
	}

void SketchingTool::loadCurvesCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData)
	{
	/* Deactivate the tool just in case: */
	active=false;
	
	/* Delete all curves: */
	for(std::vector<Curve*>::iterator cIt=curves.begin();cIt!=curves.end();++cIt)
		delete *cIt;
	curves.clear();
	
	/* Open the curve file: */
	IO::ValueSource curvesSource(cbData->selectedDirectory->openFile(cbData->selectedFileName));
	curvesSource.setPunctuation(',',true);
	
	/* Read the curve file header: */
	if(!curvesSource.isString("Vrui Curve Editor Tool Curve File"))
		Misc::throwStdErr("SketchingTool::loadCurvesCallback: File is not a curve file");
	
	/* Read all curves from the file: */
	unsigned int numCurves=curvesSource.readUnsignedInteger();
	for(unsigned int curveIndex=0;curveIndex<numCurves;++curveIndex)
		{
		/* Create a new curve: */
		Curve* c=new Curve;
		
		/* Read the curve's line width and color: */
		c->lineWidth=GLfloat(curvesSource.readNumber());
		if(curvesSource.readChar()!=',')
			Misc::throwStdErr("SketchingTool::loadCurvesCallback: File is not a curve file");
		for(int i=0;i<3;++i)
			c->color[i]=Curve::Color::Scalar(curvesSource.readUnsignedInteger());
		c->color[3]=Curve::Color::Scalar(255);
		
		/* Read the curve's control points: */
		unsigned int numControlPoints=curvesSource.readUnsignedInteger();
		for(unsigned int controlPointIndex=0;controlPointIndex<numControlPoints;++controlPointIndex)
			{
			Curve::ControlPoint cp;
			cp.t=Scalar(curvesSource.readNumber());
			if(curvesSource.readChar()!=',')
				Misc::throwStdErr("SketchingTool::loadCurvesCallback: File is not a curve file");
			for(int i=0;i<3;++i)
				cp.pos[i]=Point::Scalar(curvesSource.readNumber());
			c->controlPoints.push_back(cp);
			}
		
		/* Store the curve: */
		curves.push_back(c);
		}
	}

void SketchingTool::deleteAllCurvesCallback(Misc::CallbackData* cbData)
	{
	/* Deactivate the tool just in case: */
	active=false;
	
	/* Delete all curves: */
	for(std::vector<Curve*>::iterator cIt=curves.begin();cIt!=curves.end();++cIt)
		delete *cIt;
	curves.clear();
	}

}
