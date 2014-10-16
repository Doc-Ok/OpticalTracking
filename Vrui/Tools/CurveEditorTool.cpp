/***********************************************************************
CurveEditorTool - Tool to create and edit 3D curves (represented as
splines in hermite form).
Copyright (c) 2007-2013 Oliver Kreylos

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

#include <Vrui/Tools/CurveEditorTool.h>

#include <stdio.h>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <fstream>
#include <Misc/SelfDestructArray.h>
#include <Misc/StringMarshaller.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <IO/ValueSource.h>
#include <Cluster/MulticastPipe.h>
#include <Math/Math.h>
#include <Math/Matrix.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/gl.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/Blind.h>
#include <GLMotif/Button.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/TextField.h>
#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>
#include <Vrui/DisplayState.h>
#include <Vrui/OpenFile.h>

namespace Vrui {

/***************************************
Methods of class CurveEditorToolFactory:
***************************************/

CurveEditorToolFactory::CurveEditorToolFactory(ToolManager& toolManager)
	:ToolFactory("CurveEditorTool",toolManager),
	 curveFileName("CurveEditorTool.curve"),
	 vertexRadius(getUiSize()*Scalar(2)),
	 handleRadius(getUiSize()*Scalar(1.5)),
	 curveRadius(getUiSize())
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* toolFactory=toolManager.loadClass("UtilityTool");
	toolFactory->addChildClass(this);
	addParentClass(toolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	curveFileName=cfs.retrieveString("./curveFileName",curveFileName);
	vertexRadius=cfs.retrieveValue("./vertexRadius",vertexRadius);
	handleRadius=cfs.retrieveValue("./handleRadius",handleRadius);
	curveRadius=cfs.retrieveValue("./curveRadius",curveRadius);
	
	/* Set tool class' factory pointer: */
	CurveEditorTool::factory=this;
	}

CurveEditorToolFactory::~CurveEditorToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	CurveEditorTool::factory=0;
	}

const char* CurveEditorToolFactory::getName(void) const
	{
	return "Viewpoint Curve Editor";
	}

const char* CurveEditorToolFactory::getButtonFunction(int) const
	{
	return "Pick Keyframe";
	}

Tool* CurveEditorToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new CurveEditorTool(this,inputAssignment);
	}

void CurveEditorToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveCurveEditorToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("UtilityTool");
	}

extern "C" ToolFactory* createCurveEditorToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	CurveEditorToolFactory* curveEditorToolFactory=new CurveEditorToolFactory(*toolManager);
	
	/* Return factory object: */
	return curveEditorToolFactory;
	}

extern "C" void destroyCurveEditorToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/****************************************
Static elements of class CurveEditorTool:
****************************************/

CurveEditorToolFactory* CurveEditorTool::factory=0;

/********************************
Methods of class CurveEditorTool:
********************************/

void CurveEditorTool::writeControlPoint(const CurveEditorTool::ControlPoint& cp,Math::Matrix& b,unsigned int rowIndex)
	{
	for(unsigned int j=0;j<3;++j)
		b(rowIndex,j)=cp.center[j];
	b(rowIndex,3)=cp.size;
	for(unsigned int j=0;j<3;++j)
		b(rowIndex,4+j)=cp.forward[j];
	for(unsigned int j=0;j<3;++j)
		b(rowIndex,7+j)=cp.up[j];
	}

void CurveEditorTool::calculateC2Spline(void)
	{
	if(numVertices>1)
		{
		/* Create a big matrix to solve the C^2 spline problem: */
		unsigned int numSegments=numVertices-1;
		Math::Matrix A(4*numSegments,4*numSegments,0.0);
		Math::Matrix b(4*numSegments,10,0.0);
		
		Vertex* v0=firstVertex;
		Segment* s0=firstVertex->segments[1];
		unsigned int rowIndex=0;
		unsigned int base=0;
		
		/* Interpolate the first curve vertex: */
		A(rowIndex,base+0)=1.0;
		writeControlPoint(*v0,b,rowIndex);
		++rowIndex;
		
		switch(c2BoundaryCondition)
			{
			case ZERO_VELOCITY:
				/* Force zero velocity at beginning of curve: */
				A(rowIndex,base+0)=-3.0/double(s0->parameterInterval);
				A(rowIndex,base+1)=3.0/double(s0->parameterInterval);
				break;
			
			case ZERO_ACCELERATION:
				{
				/* Force zero acceleration at beginning of curve: */
				double pi2=Math::sqr(double(s0->parameterInterval));
				A(rowIndex,base+0)=6.0/pi2;
				A(rowIndex,base+1)=-12.0/pi2;
				A(rowIndex,base+2)=6.0/pi2;
				break;
				}
			}
		++rowIndex;
		base+=4;
		
		for(unsigned int segmentIndex=1;segmentIndex<numSegments;++segmentIndex)
			{
			Vertex* v1=s0->vertices[1];
			Segment* s1=v1->segments[1];
			
			/* Force acceleration continuity between the two segments: */
			double pi02=Math::sqr(double(s0->parameterInterval));
			A(rowIndex,base-3)=6.0/pi02;
			A(rowIndex,base-2)=-12.0/pi02;
			A(rowIndex,base-1)=6.0/pi02;
			double pi12=Math::sqr(double(s1->parameterInterval));
			A(rowIndex,base+0)=-6.0/pi12;
			A(rowIndex,base+1)=12.0/pi12;
			A(rowIndex,base+2)=-6.0/pi12;
			++rowIndex;
			
			/* Force velocity continuity between the two segments: */
			A(rowIndex,base-2)=-3.0/double(s0->parameterInterval);
			A(rowIndex,base-1)=3.0/double(s0->parameterInterval);
			A(rowIndex,base+0)=3.0/double(s1->parameterInterval);
			A(rowIndex,base+1)=-3.0/double(s1->parameterInterval);
			++rowIndex;
			
			/* Interpolate the vertex from the left: */
			A(rowIndex,base-1)=1.0;
			writeControlPoint(*v1,b,rowIndex);
			++rowIndex;
			
			/* Interpolate the vertex from the right: */
			A(rowIndex,base+0)=1.0;
			writeControlPoint(*v1,b,rowIndex);
			++rowIndex;
			
			/* Go to the next segment: */
			s0=s1;
			base+=4;
			}
		
		switch(c2BoundaryCondition)
			{
			case ZERO_VELOCITY:
				/* Force zero velocity at end of curve: */
				A(rowIndex,base-2)=-3.0/double(s0->parameterInterval);
				A(rowIndex,base-1)=3.0/double(s0->parameterInterval);
				break;
			
			case ZERO_ACCELERATION:
				{
				/* Force zero acceleration at end of curve: */
				double pi2=Math::sqr(double(s0->parameterInterval));
				A(rowIndex,base-3)=6.0/pi2;
				A(rowIndex,base-2)=-12.0/pi2;
				A(rowIndex,base-1)=6.0/pi2;
				break;
				}
			}
		++rowIndex;
		
		/* Interpolate the last curve vertex: */
		Vertex* v1=s0->vertices[1];
		A(rowIndex,base-1)=1.0;
		writeControlPoint(*v1,b,rowIndex);
		
		/* Solve the system of equations: */
		Math::Matrix x=b/A;
		Math::Matrix bp=A*x;
		
		/* Update the curve representation: */
		v0=firstVertex;
		rowIndex=0;
		for(unsigned int segmentIndex=0;segmentIndex<numSegments;++segmentIndex)
			{
			s0=v0->segments[1];
			
			/* Update the segment's start vertex: */
			for(int j=0;j<3;++j)
				v0->center[j]=Scalar(x(rowIndex,0+j));
			v0->size=Scalar(x(rowIndex,3));
			for(int j=0;j<3;++j)
				v0->forward[j]=Scalar(x(rowIndex,4+j));
			v0->forward.normalize();
			for(int j=0;j<3;++j)
				v0->up[j]=Scalar(x(rowIndex,7+j));
			v0->up.normalize();
			++rowIndex;
			
			/* Update the segment's intermediate control points: */
			for(int midIndex=0;midIndex<2;++midIndex)
				{
				for(int j=0;j<3;++j)
					s0->mid[midIndex].center[j]=Scalar(x(rowIndex,0+j));
				s0->mid[midIndex].size=Scalar(x(rowIndex,3));
				for(int j=0;j<3;++j)
					s0->mid[midIndex].forward[j]=Scalar(x(rowIndex,4+j));
				s0->mid[midIndex].forward.normalize();
				for(int j=0;j<3;++j)
					s0->mid[midIndex].up[j]=Scalar(x(rowIndex,7+j));
				s0->mid[midIndex].up.normalize();
				++rowIndex;
				}
			
			Vertex* v1=s0->vertices[1];
			if(segmentIndex==numSegments-1)
				{
				/* Update the segment's end vertex: */
				for(int j=0;j<3;++j)
					v1->center[j]=Scalar(x(rowIndex,0+j));
				v1->size=Scalar(x(rowIndex,3));
				for(int j=0;j<3;++j)
					v1->forward[j]=Scalar(x(rowIndex,4+j));
				v1->forward.normalize();
				for(int j=0;j<3;++j)
					v1->up[j]=Scalar(x(rowIndex,7+j));
				v1->up.normalize();
				}
			++rowIndex;
			
			/* Go to the next segment: */
			v0=v1;
			}
		}
	}

void CurveEditorTool::updateCurve(void)
	{
	/* Calculate the total curve parameter interval: */
	parameterInterval=Scalar(0);
	for(Segment* s=firstVertex->segments[1];s!=0;s=s->vertices[1]->segments[1])
		parameterInterval+=s->parameterInterval;
	}

void CurveEditorTool::moveToControlPoint(const CurveEditorTool::ControlPoint& cp)
	{
	/* Set the navigation transformation to the picked vertex: */
	NavTransform nav=NavTransform::identity;
	nav*=NavTransform::translateFromOriginTo(getDisplayCenter());
	nav*=NavTransform::rotate(Rotation::fromBaseVectors(getForwardDirection()^getUpDirection(),getForwardDirection()));
	nav*=NavTransform::scale(getDisplaySize()/Math::exp(cp.size));
	nav*=NavTransform::rotate(Geometry::invert(Rotation::fromBaseVectors(cp.forward^cp.up,cp.forward)));
	nav*=NavTransform::translateToOriginFrom(cp.center);
	setNavigationTransformation(nav);
	}

void CurveEditorTool::pickSegment(Scalar parameterValue)
	{
	if(numVertices>1)
		{
		/* Pick the curve segment that contains the new parameter value: */
		Scalar startParameter(0);
		Segment* s;
		for(s=firstVertex->segments[1];s!=0&&parameterValue>startParameter+s->parameterInterval;s=s->vertices[1]->segments[1])
			startParameter+=s->parameterInterval;
		pickedSegment=s;
		if(pickedSegment!=0)
			{
			pickedVertex=0;
			pickedHandleSegment=0;
			pickedSegmentParameter=(parameterValue-startParameter)/pickedSegment->parameterInterval;
			}
		}
	}

void CurveEditorTool::setParameterValue(Scalar newParameterValue)
	{
	/* Update the slider: */
	parameterValueSlider->setValue(newParameterValue);
	
	/* Update the text field: */
	parameterValueText->setValue(newParameterValue);
	
	/* Pick the selected curve segment: */
	pickSegment(newParameterValue);
	
	/* Update the user interface: */
	updateDialog();
	}

void CurveEditorTool::forceC2ContinuityToggleValueChangedCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	/* Set the continuity flag: */
	forceC2Continuity=cbData->set;
	
	/* Force C^2 continuity if the flag is now set: */
	if(forceC2Continuity)
		calculateC2Spline();
	}

void CurveEditorTool::c2BoundaryConditionBoxValueChangedCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData)
	{
	switch(cbData->radioBox->getToggleIndex(cbData->newSelectedToggle))
		{
		case 0:
			c2BoundaryCondition=ZERO_VELOCITY;
			break;
		
		case 1:
			c2BoundaryCondition=ZERO_ACCELERATION;
			break;
		}
	
	/* Force C^2 continuity if requested: */
	if(forceC2Continuity)
		calculateC2Spline();
	}

void CurveEditorTool::previousControlPointCallback(Misc::CallbackData* cbData)
	{
	Vertex* previous=lastVertex;
	if(numVertices>1)
		{
		/* Get the current parameter value: */
		Scalar parameter=Scalar(parameterValueSlider->getValue());
		
		/* Find the next control point to the left of the current parameter value: */
		Scalar startParameter(0);
		Segment* s;
		for(s=firstVertex->segments[1];s!=0&&startParameter<parameter;s=s->vertices[1]->segments[1])
			{
			startParameter+=s->parameterInterval;
			previous=s->vertices[0];
			}
		}
	
	/* Pick the found vertex: */
	pickedVertex=previous;
	pickedHandleSegment=0;
	pickedSegment=0;
	if(scrub||snapVertexToView)
		{
		/* Set the navigation transformation to the picked vertex: */
		moveToControlPoint(*pickedVertex);
		
		snapVertexToView=true;
		}
	
	updateDialog();
	}

void CurveEditorTool::parameterValueSliderValueChangedCallback(GLMotif::Slider::ValueChangedCallbackData* cbData)
	{
	/* Update the text field: */
	parameterValueText->setValue(cbData->value);
	
	/* Pick the curve segment: */
	pickSegment(cbData->value);
	
	if(numVertices>1)
		updateDialog();
	}

void CurveEditorTool::nextControlPointCallback(Misc::CallbackData* cbData)
	{
	Vertex* next=firstVertex;
	if(numVertices>1)
		{
		/* Get the current parameter value: */
		Scalar parameter=Scalar(parameterValueSlider->getValue());
		
		/* Find the next control point to the left of the current parameter value: */
		Scalar startParameter(0);
		Segment* s;
		for(s=firstVertex->segments[1];s!=0&&startParameter<=parameter;s=s->vertices[1]->segments[1])
			{
			startParameter+=s->parameterInterval;
			next=s->vertices[1];
			}
		}
	
	/* Pick the found vertex: */
	pickedVertex=next;
	pickedHandleSegment=0;
	pickedSegment=0;
	if(scrub||snapVertexToView)
		{
		/* Set the navigation transformation to the picked vertex: */
		moveToControlPoint(*pickedVertex);
		
		snapVertexToView=true;
		}
	
	updateDialog();
	}

void CurveEditorTool::scrubToggleValueChangedCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	if(numVertices>1)
		{
		/* Set the scrubbing flag: */
		scrub=cbData->set;
		if(scrub)
			{
			/* Pick the curve segment that contains the current parameter value: */
			pickSegment(Scalar(parameterValueSlider->getValue()));
			
			updateDialog();
			}
		}
	else
		scrubToggle->setToggle(false);
	}

void CurveEditorTool::autoPlayToggleValueChangedCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	if(numVertices>1)
		{
		play=cbData->set;
		if(play)
			{
			/* Calculate the time offset: */
			playStartTime=Scalar(getApplicationTime())-Scalar(parameterValueSlider->getValue());
			}
		}
	else
		autoPlayToggle->setToggle(false);
	}

namespace {

/****************
Helper functions:
****************/

inline void expect(IO::ValueSource& source,char literal)
	{
	if(!source.isLiteral(literal))
		throw std::runtime_error("File is not a curve file");
	}

inline void readCA(IO::ValueSource& source,Geometry::ComponentArray<Scalar,3>& ca)
	{
	expect(source,'(');
	ca[0]=source.readNumber();
	for(int i=1;i<3;++i)
		{
		expect(source,',');
		ca[i]=source.readNumber();
		}
	expect(source,')');
	}

}

void CurveEditorTool::loadCurveCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData)
	{
	/* Open the curve file: */
	IO::ValueSource curveFile(cbData->selectedDirectory->openFile(cbData->selectedFileName));
	curveFile.setPunctuation("(),");
	curveFile.skipWs();
	
	/* Create intermediate lists of vertices and segments: */
	std::vector<Vertex> vertices;
	std::vector<Segment> segments;
	
	/* Read the first vertex: */
	Vertex v;
	readCA(curveFile,v.center);
	v.size=Math::log(curveFile.readNumber());
	readCA(curveFile,v.forward);
	readCA(curveFile,v.up);
	v.continuity=Vertex::TANGENT;
	
	/* Store the first vertex: */
	vertices.push_back(v);
	
	/* Read all curve segments: */
	while(!curveFile.eof())
		{
		/* Read the next segment: */
		Segment s;
		s.parameterInterval=curveFile.readNumber();
		s.forceStraight=false;
		
		/* Read the segment's intermediate vertices: */
		for(int i=0;i<2;++i)
			{
			readCA(curveFile,s.mid[i].center);
			s.mid[i].size=Math::log(curveFile.readNumber());
			readCA(curveFile,s.mid[i].forward);
			readCA(curveFile,s.mid[i].up);
			}
		
		/* Read the next vertex: */
		readCA(curveFile,v.center);
		v.size=Math::log(curveFile.readNumber());
		readCA(curveFile,v.forward);
		readCA(curveFile,v.up);
		v.continuity=Vertex::TANGENT;
		
		/* Store the segment and vertex: */
		segments.push_back(s);
		vertices.push_back(v);
		}
	
	/* Delete the current curve: */
	while(firstVertex!=0)
		{
		Segment* s=firstVertex->segments[1];
		Vertex* nextVertex;
		if(s!=0)
			{
			nextVertex=s->vertices[1];
			delete s;
			}
		else
			nextVertex=0;
		delete firstVertex;
		firstVertex=nextVertex;
		}
	lastVertex=0;
	parameterInterval=Scalar(0);
	numVertices=0;
	forceC2Continuity=false;
	pickedVertex=0;
	pickedHandleSegment=0;
	pickedSegment=0;
	
	/* Create the new curve: */
	if(!vertices.empty())
		{
		firstVertex=new Vertex;
		*firstVertex=vertices[0];
		for(int i=0;i<2;++i)
			firstVertex->segments[i]=0;
		lastVertex=firstVertex;
		}
	for(unsigned int vertexIndex=1;vertexIndex<vertices.size();++vertexIndex)
		{
		/* Create the next segment: */
		Segment* s=new Segment;
		*s=segments[vertexIndex-1];
		s->vertices[0]=lastVertex;
		lastVertex->segments[1]=s;
		
		/* Create the next vertex: */
		Vertex* newVertex=new Vertex;
		*newVertex=vertices[vertexIndex];
		newVertex->segments[0]=s;
		newVertex->segments[1]=0;
		s->vertices[1]=newVertex;
		
		parameterInterval+=s->parameterInterval;
		lastVertex=newVertex;
		}
	numVertices=vertices.size();
	
	/* Update derived curve state: */
	updateCurve();
	
	/* Update the curve editor dialog: */
	updateDialog();
	}

void CurveEditorTool::saveCurveCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData)
	{
	/* Bail out if there is no curve: */
	if(firstVertex==0)
		return;
	
	if(isMaster())
		{
		try
			{
			/* Write the curve to a text file: */
			std::ofstream file(cbData->getSelectedPath().c_str(),std::ios::trunc);
			
			const Vertex* v0=firstVertex;
			
			/* Write the first vertex: */
			file<<"("<<v0->center[0]<<", "<<v0->center[1]<<", "<<v0->center[2]<<")";
			file<<" "<<Math::exp(v0->size);
			file<<" ("<<v0->forward[0]<<", "<<v0->forward[1]<<", "<<v0->forward[2]<<")";
			file<<" ("<<v0->up[0]<<", "<<v0->up[1]<<", "<<v0->up[2]<<")"<<std::endl;
			
			/* Write all segments: */
			for(const Segment* s0=v0->segments[1];s0!=0;s0=v0->segments[1])
				{
				/* Write the segment's parameter interval: */
				file<<s0->parameterInterval<<std::endl;
				
				/* Write the segment's intermediate control points: */
				for(int i=0;i<2;++i)
					{
					file<<"("<<s0->mid[i].center[0]<<", "<<s0->mid[i].center[1]<<", "<<s0->mid[i].center[2]<<")";
					file<<" "<<Math::exp(s0->mid[i].size);
					file<<" ("<<s0->mid[i].forward[0]<<", "<<s0->mid[i].forward[1]<<", "<<s0->mid[i].forward[2]<<")";
					file<<" ("<<s0->mid[i].up[0]<<", "<<s0->mid[i].up[1]<<", "<<s0->mid[i].up[2]<<")"<<std::endl;
					}
				
				/* Write the segment's end vertex: */
				v0=s0->vertices[1];
				file<<"("<<v0->center[0]<<", "<<v0->center[1]<<", "<<v0->center[2]<<")";
				file<<" "<<Math::exp(v0->size);
				file<<" ("<<v0->forward[0]<<", "<<v0->forward[1]<<", "<<v0->forward[2]<<")";
				file<<" ("<<v0->up[0]<<", "<<v0->up[1]<<", "<<v0->up[2]<<")"<<std::endl;
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

void CurveEditorTool::appendVertexCallback(Misc::CallbackData* cbData)
	{
	/* Create a new vertex: */
	Vertex* newVertex=new Vertex;
	
	/* Set the vertex to the current view position: */
	newVertex->center=getInverseNavigationTransformation().transform(getDisplayCenter());
	newVertex->size=Math::log(getDisplaySize()*getInverseNavigationTransformation().getScaling());
	newVertex->forward=getInverseNavigationTransformation().transform(getForwardDirection());
	newVertex->forward.normalize();
	newVertex->up=getInverseNavigationTransformation().transform(getUpDirection());
	newVertex->up.normalize();
	
	/* Append the new vertex to the curve: */
	if(lastVertex!=0)
		{
		/* Append a new segment to the curve: */
		Segment* newSegment=new Segment;
		newSegment->vertices[0]=lastVertex;
		newSegment->vertices[1]=newVertex;
		for(int handleIndex=0;handleIndex<2;++handleIndex)
			newSegment->mid[handleIndex]=ControlPoint::affineCombination(*lastVertex,*newVertex,Scalar(handleIndex+1)/Scalar(3));
		if(lastVertex->segments[0]!=0)
			newSegment->parameterInterval=lastVertex->segments[0]->parameterInterval;
		else
			newSegment->parameterInterval=Scalar(60);
		newSegment->forceStraight=false;
		lastVertex->segments[1]=newSegment;
		newVertex->segments[0]=newSegment;
		}
	else
		{
		firstVertex=newVertex;
		newVertex->segments[0]=0;
		}
	newVertex->segments[1]=0;
	newVertex->continuity=Vertex::TANGENT;
	lastVertex=newVertex;
	++numVertices;
	
	if(forceC2Continuity)
		{
		/* Force the curve to C^2 continuity if requested: */
		calculateC2Spline();
		}
	else
		{
		/* Adjust the last segment's tangent vectors: */
		Segment* s1=lastVertex->segments[0];
		if(s1!=0)
			{
			Vertex* v1=s1->vertices[0];
			Segment* s0=v1->segments[0];
			if(s0!=0)
				{
				/* Segment is dragged as a Catmull-Rom curve: */
				Vertex* v0=s0->vertices[0];
				
				Vector c20=lastVertex->center-v0->center;
				Vector c10=v1->center-v0->center;
				Vector c21=lastVertex->center-v1->center;
				Vector ct1=c20/Scalar(6);
				s0->mid[1].center=v1->center-ct1;
				s1->mid[0].center=v1->center+ct1;
				Vector ct2=(c21-c10)/Scalar(3)+ct1;
				s1->mid[1].center=lastVertex->center-ct2;
				
				if(v0->segments[0]==0)
					{
					/* Adjust the first vertex' tangent as well: */
					Vector ct3=(c10-c21)/Scalar(3)+ct1;
					s0->mid[0].center=v0->center+ct3;
					}
				}
			else
				{
				/* First segment is dragged as a straight line: */
				for(int handleIndex=0;handleIndex<2;++handleIndex)
					s1->mid[handleIndex]=ControlPoint::affineCombination(*v1,*lastVertex,Scalar(handleIndex+1)/Scalar(3));
				}
			}
		}
	
	/* Update derived curve state: */
	updateCurve();
	
	/* Pick the new vertex: */
	pickedVertex=newVertex;
	pickedHandleSegment=0;
	pickedSegment=0;
	
	updateDialog();
	}

void CurveEditorTool::snapVertexToViewToggleValueChangedCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	if(pickedVertex!=0)
		{
		snapVertexToView=cbData->set;
		
		if(snapVertexToView)
			{
			/* Set the navigation transformation to the picked vertex: */
			moveToControlPoint(*pickedVertex);
			}
		}
	else
		snapVertexToViewToggle->setToggle(false);
	}

void CurveEditorTool::deleteVertexCallback(Misc::CallbackData* cbData)
	{
	if(pickedVertex!=0)
		{
		/* Delete the picked vertex: */
		Segment* s0=pickedVertex->segments[0];
		Segment* s1=pickedVertex->segments[1];
		if(s0!=0&&s1!=0)
			{
			/* Merge the segments to either side of the vertex: */
			s0->vertices[1]=s1->vertices[1];
			s0->vertices[1]->segments[0]=s0;
			s0->mid[1]=s1->mid[1];
			s0->parameterInterval+=s1->parameterInterval;
			s0->forceStraight=s0->forceStraight&&s1->forceStraight;
			delete s1;
			}
		else if(s0!=0)
			{
			/* Delete the segment to the left of the vertex: */
			lastVertex=s0->vertices[0];
			lastVertex->segments[1]=0;
			delete s0;
			}
		else if(s1!=0)
			{
			/* Delete the segment to the right of the vertex: */
			firstVertex=s1->vertices[1];
			firstVertex->segments[0]=0;
			delete s1;
			}
		else
			{
			firstVertex=0;
			lastVertex=0;
			}
		delete pickedVertex;
		--numVertices;
		pickedVertex=0;
		
		if(forceC2Continuity)
			{
			/* Force the curve to C^2 continuity if requested: */
			calculateC2Spline();
			}
		
		/* Update derived curve state: */
		updateCurve();
		
		updateDialog();
		}
	}

void CurveEditorTool::vertexContinuityBoxValueChangedCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData)
	{
	#if 0
	if(pickedVertex!=0)
		{
		switch(cbData->radioBox->getToggleIndex(cbData->newSelectedToggle))
			{
			case 0:
				pickedVertex->continuity=Vertex::NONE;
				break;
			
			case 1:
				pickedVertex->continuity=Vertex::TANGENT;
				if(pickedVertex->segs[0]!=0&&pickedVertex->segs[1]!=0)
					{
					/* Adjust the vertex' tangent directions: */
					Scalar tangentLen0=Geometry::dist(pickedVertex->segs[0]->mid[1],pickedVertex->pos);
					Scalar tangentLen1=Geometry::dist(pickedVertex->pos,pickedVertex->segs[1]->mid[0]);
					Vector tangent=pickedVertex->segs[1]->mid[0]-pickedVertex->segs[0]->mid[1];
					tangent.normalize();
					pickedVertex->segs[0]->mid[1]=pickedVertex->pos-tangent*tangentLen0;
					pickedVertex->segs[1]->mid[0]=pickedVertex->pos+tangent*tangentLen1;
					}
				break;
			
			case 2:
				pickedVertex->continuity=Vertex::DERIVATIVE;
				if(pickedVertex->segs[0]!=0&&pickedVertex->segs[1]!=0)
					{
					/* Adjust the vertex' tangents: */
					Vector tangent=pickedVertex->segs[1]->mid[0]-pickedVertex->segs[0]->mid[1];
					tangent/=Scalar(2);
					pickedVertex->segs[0]->mid[1]=pickedVertex->pos-tangent;
					pickedVertex->segs[1]->mid[0]=pickedVertex->pos+tangent;
					}
				break;
			}
		}
	#endif
	}

void CurveEditorTool::forceSegmentStraightToggleValueChangedCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	if(pickedSegment!=0)
		{
		/* Set the picked segment's straight line flag: */
		pickedSegment->forceStraight=cbData->set;
		}
	}

void CurveEditorTool::splitSegmentCallback(Misc::CallbackData* cbData)
	{
	if(pickedSegment!=0)
		{
		/* Split the picked segment at the local segment parameter: */
		ControlPoint cp[6];
		cp[0]=ControlPoint::affineCombination(*pickedSegment->vertices[0],pickedSegment->mid[0],pickedSegmentParameter);
		cp[1]=ControlPoint::affineCombination(pickedSegment->mid[0],pickedSegment->mid[1],pickedSegmentParameter);
		cp[2]=ControlPoint::affineCombination(pickedSegment->mid[1],*pickedSegment->vertices[1],pickedSegmentParameter);
		for(int i=0;i<2;++i)
			cp[3+i]=ControlPoint::affineCombination(cp[i],cp[i+1],pickedSegmentParameter);
		cp[5]=ControlPoint::affineCombination(cp[3],cp[4],pickedSegmentParameter);
		
		/* Create the new vertex and a new segment: */
		Segment* newSegment=new Segment;
		newSegment->vertices[1]=pickedSegment->vertices[1];
		newSegment->vertices[1]->segments[0]=newSegment;
		newSegment->forceStraight=pickedSegment->forceStraight;
		Vertex* newVertex=new Vertex;
		newVertex->segments[0]=pickedSegment;
		newVertex->segments[1]=newSegment;
		newVertex->continuity=Vertex::TANGENT;
		pickedSegment->vertices[1]=newVertex;
		newSegment->vertices[0]=newVertex;
		newSegment->parameterInterval=pickedSegment->parameterInterval*(Scalar(1)-pickedSegmentParameter);
		pickedSegment->parameterInterval=pickedSegment->parameterInterval*pickedSegmentParameter;
		pickedSegment->mid[0]=cp[0];
		pickedSegment->mid[1]=cp[3];
		newVertex->ControlPoint::operator=(cp[5]);
		newSegment->mid[0]=cp[4];
		newSegment->mid[1]=cp[2];
		++numVertices;
		
		/* Pick the new vertex: */
		pickedVertex=newVertex;
		pickedSegment=0;
		
		/* Update derived curve state: */
		updateCurve();
		
		updateDialog();
		}
	}

void CurveEditorTool::segmentParameterIntervalSliderValueChangedCallback(GLMotif::Slider::ValueChangedCallbackData* cbData)
	{
	/* Update the text field: */
	segmentParameterIntervalText->setValue(cbData->value);
	
	if(pickedSegment!=0)
		{
		/* Set the picked segment's parameter interval width: */
		pickedSegment->parameterInterval=Scalar(cbData->value);
		
		if(forceC2Continuity)
			{
			/* Force C^2 continuity: */
			calculateC2Spline();
			}
		
		/* Update derived curve state: */
		updateCurve();
		
		updateDialog();
		}
	}

void CurveEditorTool::updateDialog(void)
	{
	forceC2ContinuityToggle->setToggle(forceC2Continuity);
	
	/* Adjust the parameter value slider: */
	if(numVertices>1)
		{
		parameterValueSlider->setValueRange(0.0f,float(parameterInterval),0.0f);
		}
	else
		{
		parameterValueText->setValue(0.5f);
		parameterValueSlider->setValueRange(0.0f,1.0f,0.0f);
		parameterValueSlider->setValue(0.5f);
		}
	
	if(pickedVertex!=0)
		{
		if(numVertices>1)
			{
			/* Set the parameter value slider to the picked vertex' parameter: */
			Scalar vertexParameter(0);
			for(const Vertex* v=firstVertex;v!=pickedVertex;v=v->segments[1]->vertices[1])
				vertexParameter+=v->segments[1]->parameterInterval;
			parameterValueText->setValue(float(vertexParameter));
			parameterValueSlider->setValue(float(vertexParameter));
			}
		
		/* Show the vertex' continuity setting: */
		switch(pickedVertex->continuity)
			{
			case Vertex::NONE:
				vertexContinuityBox->setSelectedToggle(0);
				break;
			
			case Vertex::TANGENT:
				vertexContinuityBox->setSelectedToggle(1);
				break;
			
			case Vertex::DERIVATIVE:
				vertexContinuityBox->setSelectedToggle(2);
				break;
			}
		}
	else
		{
		snapVertexToView=false;
		vertexContinuityBox->setSelectedToggle(0);
		}
	snapVertexToViewToggle->setToggle(snapVertexToView);
	
	if(pickedSegment!=0)
		{
		/* Set the straight line toggle: */
		forceSegmentStraightToggle->setToggle(pickedSegment->forceStraight);
		
		/* Set the parameter value slider to the picked segment's local parameter value: */
		Scalar segmentParameter(0);
		for(const Segment* s=firstVertex->segments[1];s!=pickedSegment;s=s->vertices[1]->segments[1])
			segmentParameter+=s->parameterInterval;
		segmentParameter+=pickedSegmentParameter*pickedSegment->parameterInterval;
		parameterValueText->setValue(float(segmentParameter));
		parameterValueSlider->setValue(float(segmentParameter));
		
		/* Set the segment interval width slider: */
		segmentParameterIntervalText->setValue(float(pickedSegment->parameterInterval));
		segmentParameterIntervalSlider->setValue(float(pickedSegment->parameterInterval));
		
		if(scrub)
			{
			/* Set the navigation transformation to the current curve vertex: */
			ControlPoint cp[6];
			cp[0]=ControlPoint::affineCombination(*pickedSegment->vertices[0],pickedSegment->mid[0],pickedSegmentParameter);
			cp[1]=ControlPoint::affineCombination(pickedSegment->mid[0],pickedSegment->mid[1],pickedSegmentParameter);
			cp[2]=ControlPoint::affineCombination(pickedSegment->mid[1],*pickedSegment->vertices[1],pickedSegmentParameter);
			for(int i=0;i<2;++i)
				cp[3+i]=ControlPoint::affineCombination(cp[i],cp[i+1],pickedSegmentParameter);
			cp[5]=ControlPoint::affineCombination(cp[3],cp[4],pickedSegmentParameter);
			moveToControlPoint(cp[5]);
			}
		}
	else
		{
		/* Disable scrubbing: */
		scrubToggle->setToggle(false);
		scrub=false;
		
		/* Set the straight line toggle: */
		forceSegmentStraightToggle->setToggle(false);
		
		/* Set the segment interval width slider: */
		segmentParameterIntervalText->setValue(0.0f);
		segmentParameterIntervalSlider->setValue(0.0f);
		}
	}

void CurveEditorTool::renderSegment(const Point& p0,const Point& p1,const Point& p2,const Point& p3,int level) const
	{
	if(level==0)
		{
		/* Draw a straight line between the segment's start and end points: */
		glVertex(p0);
		glVertex(p3);
		}
	else
		{
		/* Subdivide the segment: */
		Point m10=Geometry::mid(p0,p1);
		Point m11=Geometry::mid(p1,p2);
		Point m12=Geometry::mid(p2,p3);
		Point m20=Geometry::mid(m10,m11);
		Point m21=Geometry::mid(m11,m12);
		Point m30=Geometry::mid(m20,m21);
		
		/* Render the two segments recursively: */
		renderSegment(p0,m10,m20,m30,level-1);
		renderSegment(m30,m21,m12,p3,level-1);
		}
	}

CurveEditorTool::CurveEditorTool(const ToolFactory* sFactory,const ToolInputAssignment& inputAssignment)
	:UtilityTool(sFactory,inputAssignment),
	 curveEditorDialogPopup(0),
	 forceC2ContinuityToggle(0),
	 c2BoundaryConditionBox(0),
	 parameterValueText(0),
	 parameterValueSlider(0),
	 scrubToggle(0),
	 snapVertexToViewToggle(0),
	 vertexContinuityBox(0),
	 forceSegmentStraightToggle(0),
	 segmentParameterIntervalText(0),
	 segmentParameterIntervalSlider(0),
	 numVertices(0),
	 firstVertex(0),lastVertex(0),
	 parameterInterval(0),
	 forceC2Continuity(true),
	 c2BoundaryCondition(ZERO_ACCELERATION),
	 pickedVertex(0),
	 pickedHandleSegment(0),
	 pickedSegment(0),
	 scrub(false),
	 play(false),playStartTime(0.0),
	 curveSelectionHelper(factory->curveFileName.c_str(),".curve",openDirectory(".")),
	 editingMode(IDLE),
	 snapVertexToView(false)
	{
	/* Create the curve editor dialog window: */
	const GLMotif::StyleSheet& ss=*getWidgetManager()->getStyleSheet();
	curveEditorDialogPopup=new GLMotif::PopupWindow("CurveEditorDialogPopup",getWidgetManager(),"Curve Editor Dialog");
	
	GLMotif::RowColumn* curveEditorDialog=new GLMotif::RowColumn("CurveEditorDialog",curveEditorDialogPopup,false);
	curveEditorDialog->setOrientation(GLMotif::RowColumn::VERTICAL);
	curveEditorDialog->setNumMinorWidgets(2);
	curveEditorDialog->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	
	new GLMotif::Label("VertexLabel",curveEditorDialog,"Vertex");
	
	GLMotif::RowColumn* vertexBox=new GLMotif::RowColumn("VertexBox",curveEditorDialog,false);
	vertexBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	vertexBox->setPacking(GLMotif::RowColumn::PACK_GRID);
	
	GLMotif::Button* appendVertexButton=new GLMotif::Button("AppendVertexButton",vertexBox,"Append Vertex");
	appendVertexButton->getSelectCallbacks().add(this,&CurveEditorTool::appendVertexCallback);
	
	snapVertexToViewToggle=new GLMotif::ToggleButton("SnapVertexToViewToggle",vertexBox,"Snap to View");
	snapVertexToViewToggle->setToggle(false);
	snapVertexToViewToggle->getValueChangedCallbacks().add(this,&CurveEditorTool::snapVertexToViewToggleValueChangedCallback);
	
	GLMotif::Button* deleteVertexButton=new GLMotif::Button("DeleteVertexButton",vertexBox,"Delete Vertex");
	deleteVertexButton->getSelectCallbacks().add(this,&CurveEditorTool::deleteVertexCallback);
	
	vertexBox->manageChild();
	
	new GLMotif::Label("SplineContinuityLabel",curveEditorDialog,"Spline Continuity");
	
	GLMotif::RowColumn* splineContinuityBox=new GLMotif::RowColumn("SplineContinuityBox",curveEditorDialog,false);
	splineContinuityBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	splineContinuityBox->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	
	forceC2ContinuityToggle=new GLMotif::ToggleButton("ForceC2ContinuityToggle",splineContinuityBox,"Force C2");
	forceC2ContinuityToggle->setBorderType(GLMotif::Widget::PLAIN);
	forceC2ContinuityToggle->setBorderWidth(0.0f);
	forceC2ContinuityToggle->setToggle(forceC2Continuity);
	forceC2ContinuityToggle->getValueChangedCallbacks().add(this,&CurveEditorTool::forceC2ContinuityToggleValueChangedCallback);
	
	c2BoundaryConditionBox=new GLMotif::RadioBox("C2BoundaryConditionBox",splineContinuityBox,false);
	c2BoundaryConditionBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	c2BoundaryConditionBox->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	c2BoundaryConditionBox->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
	
	c2BoundaryConditionBox->addToggle("Zero End Velocity");
	c2BoundaryConditionBox->addToggle("Zero End Acceleration");
	
	switch(c2BoundaryCondition)
		{
		case ZERO_VELOCITY:
			c2BoundaryConditionBox->setSelectedToggle(0);
			break;
		
		case ZERO_ACCELERATION:
			c2BoundaryConditionBox->setSelectedToggle(1);
			break;
		}
	c2BoundaryConditionBox->getValueChangedCallbacks().add(this,&CurveEditorTool::c2BoundaryConditionBoxValueChangedCallback);
	c2BoundaryConditionBox->manageChild();
	
	new GLMotif::Blind("Filler",splineContinuityBox);
	
	splineContinuityBox->manageChild();
	
	new GLMotif::Label("VertexContinuityLabel",curveEditorDialog,"Vertex Continuity");
	
	vertexContinuityBox=new GLMotif::RadioBox("VertexContinuityBox",curveEditorDialog,false);
	vertexContinuityBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	vertexContinuityBox->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	vertexContinuityBox->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
	
	vertexContinuityBox->addToggle("None");
	vertexContinuityBox->addToggle("Tangent");
	vertexContinuityBox->addToggle("Derivative");
	
	vertexContinuityBox->setSelectedToggle(0);
	vertexContinuityBox->getValueChangedCallbacks().add(this,&CurveEditorTool::vertexContinuityBoxValueChangedCallback);
	vertexContinuityBox->manageChild();
	
	new GLMotif::Label("SegmentLabel",curveEditorDialog,"Segment");
	
	GLMotif::RowColumn* segmentBox=new GLMotif::RowColumn("SegmentBox",curveEditorDialog,false);
	segmentBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	segmentBox->setPacking(GLMotif::RowColumn::PACK_GRID);
	
	forceSegmentStraightToggle=new GLMotif::ToggleButton("ForceSegmentStraightToggle",segmentBox,"Force Straight");
	forceSegmentStraightToggle->setToggle(false);
	forceSegmentStraightToggle->getValueChangedCallbacks().add(this,&CurveEditorTool::forceSegmentStraightToggleValueChangedCallback);
	
	GLMotif::Button* splitSegmentButton=new GLMotif::Button("SplitSegmentButton",segmentBox,"Split Segment");
	splitSegmentButton->getSelectCallbacks().add(this,&CurveEditorTool::splitSegmentCallback);
	
	segmentBox->manageChild();
	
	new GLMotif::Label("SegmentParameterIntervalLabel",curveEditorDialog,"Segment Length");
	
	GLMotif::RowColumn* segmentParameterIntervalBox=new GLMotif::RowColumn("SegmentParameterIntervalBox",curveEditorDialog,false);
	segmentParameterIntervalBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	segmentParameterIntervalBox->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	
	segmentParameterIntervalText=new GLMotif::TextField("SegmentParameterIntervalText",segmentParameterIntervalBox,5);
	segmentParameterIntervalText->setFloatFormat(GLMotif::TextField::FIXED);
	segmentParameterIntervalText->setFieldWidth(5);
	segmentParameterIntervalText->setPrecision(1);
	segmentParameterIntervalText->setValue(0.0f);
	
	segmentParameterIntervalSlider=new GLMotif::Slider("SegmentParameterIntervalSlider",segmentParameterIntervalBox,GLMotif::Slider::HORIZONTAL,ss.fontHeight*20.0f);
	segmentParameterIntervalSlider->setValueRange(0.5f,120.0f,0.5f);
	segmentParameterIntervalSlider->setValue(0.0f);
	segmentParameterIntervalSlider->getValueChangedCallbacks().add(this,&CurveEditorTool::segmentParameterIntervalSliderValueChangedCallback);
	
	segmentParameterIntervalBox->manageChild();
	
	new GLMotif::Label("ParameterValueLabel",curveEditorDialog,"Parameter Value");
	
	GLMotif::RowColumn* parameterValueBox=new GLMotif::RowColumn("ParameterValueBox",curveEditorDialog,false);
	parameterValueBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	parameterValueBox->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	
	parameterValueText=new GLMotif::TextField("ParameterValueText",parameterValueBox,7);
	parameterValueText->setFloatFormat(GLMotif::TextField::FIXED);
	parameterValueText->setFieldWidth(7);
	parameterValueText->setPrecision(2);
	parameterValueText->setValue(0.5f);
	
	GLMotif::Button* previousControlPointButton=new GLMotif::Button("PreviousControlPointButton",parameterValueBox,"<");
	previousControlPointButton->getSelectCallbacks().add(this,&CurveEditorTool::previousControlPointCallback);
	
	parameterValueSlider=new GLMotif::Slider("ParameterValueSlider",parameterValueBox,GLMotif::Slider::HORIZONTAL,ss.fontHeight*20.0f);
	parameterValueSlider->setValueRange(0.0f,1.0f,0.0f);
	parameterValueSlider->setValue(0.5f);
	parameterValueSlider->getValueChangedCallbacks().add(this,&CurveEditorTool::parameterValueSliderValueChangedCallback);
	
	GLMotif::Button* nextControlPointButton=new GLMotif::Button("NextControlPointButton",parameterValueBox,">");
	nextControlPointButton->getSelectCallbacks().add(this,&CurveEditorTool::nextControlPointCallback);
	
	scrubToggle=new GLMotif::ToggleButton("ScrubToggle",parameterValueBox,"Scrub");
	scrubToggle->setToggle(false);
	scrubToggle->getValueChangedCallbacks().add(this,&CurveEditorTool::scrubToggleValueChangedCallback);
	
	parameterValueBox->manageChild();
	
	new GLMotif::Label("PlayLabel",curveEditorDialog,"Autoplay");
	
	autoPlayToggle=new GLMotif::ToggleButton("AutoPlayToggle",curveEditorDialog,"Play From Current Position");
	autoPlayToggle->setToggle(false);
	autoPlayToggle->getValueChangedCallbacks().add(this,&CurveEditorTool::autoPlayToggleValueChangedCallback);
	
	new GLMotif::Label("FileLabel",curveEditorDialog,"File");
	
	GLMotif::RowColumn* ioBox=new GLMotif::RowColumn("IoBox",curveEditorDialog,false);
	ioBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
	ioBox->setPacking(GLMotif::RowColumn::PACK_GRID);
	
	GLMotif::Button* loadCurveButton=new GLMotif::Button("LoadCurveButton",ioBox,"Load Curve...");
	curveSelectionHelper.addLoadCallback(loadCurveButton,this,&CurveEditorTool::loadCurveCallback);
	
	GLMotif::Button* saveCurveButton=new GLMotif::Button("SaveCurveButton",ioBox,"Save Curve...");
	curveSelectionHelper.addSaveCallback(saveCurveButton,this,&CurveEditorTool::saveCurveCallback);
	
	ioBox->manageChild();
	
	curveEditorDialog->manageChild();
	
	/* Pop up the curve editor dialog: */
	popupPrimaryWidget(curveEditorDialogPopup);
	}

CurveEditorTool::~CurveEditorTool(void)
	{
	/* Delete the curve: */
	while(firstVertex!=0)
		{
		Segment* s=firstVertex->segments[1];
		Vertex* nextVertex;
		if(s!=0)
			{
			nextVertex=s->vertices[1];
			delete s;
			}
		else
			nextVertex=0;
		delete firstVertex;
		firstVertex=nextVertex;
		}
	
	/* Delete the curve editor dialog: */
	delete curveEditorDialogPopup;
	}

const ToolFactory* CurveEditorTool::getFactory(void) const
	{
	return factory;
	}

void CurveEditorTool::buttonCallback(int,InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState) // Button has just been pressed
		{
		Point p=getInverseNavigationTransformation().transform(getButtonDevicePosition(0));
		Scalar scale=getInverseNavigationTransformation().getScaling();
		
		/* Check if the device selected an existing tangent handle, an existing control point, or the curve (in that order): */
		pickedVertex=0;
		Scalar minVertexDist2=Math::sqr(factory->vertexRadius*scale);
		pickedHandleSegment=0;
		Scalar minHandleDist2=Math::sqr(factory->handleRadius*scale);
		pickedSegment=0;
		// Scalar minSegmentDist2=Math::sqr(factory->curveRadius*scale);
		
		Vertex* v1=firstVertex;
		while(v1!=0)
			{
			/* Check if the vertex was picked: */
			Scalar vertexDist2=Geometry::sqrDist(p,v1->center);
			if(minVertexDist2>vertexDist2)
				{
				pickedVertex=v1;
				minVertexDist2=vertexDist2;
				}
			
			/* Get the next segment and vertex: */
			Segment* s=v1->segments[1];
			if(s==0)
				break;
			
			/* Check if a vertex tangent handle was picked: */
			for(int handleIndex=0;handleIndex<2;++handleIndex)
				{
				Scalar handleDist2=Geometry::sqrDist(p,s->mid[handleIndex].center);
				if(minHandleDist2>handleDist2)
					{
					pickedHandleSegment=s;
					pickedHandleIndex=handleIndex;
					minHandleDist2=handleDist2;
					}
				}
			
			/* Check if the curve itself was picked: */
			/* Well, maybe not... */
			
			/* Go to the next vertex: */
			v1=s->vertices[1];
			}
		
		/* React to the picked object: */
		if(pickedHandleSegment!=0)
			{
			/* Start dragging the picked vertex tangent handle: */
			editingMode=DRAGGING_HANDLE;
			dragHandleOffset=pickedHandleSegment->mid[pickedHandleIndex].center-p;
			}
		else if(pickedVertex!=0)
			{
			/* Start dragging the picked vertex: */
			editingMode=DRAGGING_VERTEX;
			dragVertexOffset=pickedVertex->center-p;
			}
		else if(pickedSegment!=0)
			{
			/* Don't do anything for now: */
			}
		
		/* Update the curve editor dialog: */
		updateDialog();
		}
	else // Button has just been released
		{
		/* Stop dragging: */
		editingMode=IDLE;
		}
	}

void CurveEditorTool::frame(void)
	{
	if(play)
		{
		/* Calculate the new curve parameter based on the time offset: */
		Scalar newParameterValue=Scalar(getApplicationTime()-playStartTime);
		if(newParameterValue>=parameterInterval)
			{
			/* Move to the last control point and stop playing: */
			setParameterValue(parameterInterval);
			play=false;
			autoPlayToggle->setToggle(false);
			}
		else
			{
			/* Move to the current parameter value: */
			setParameterValue(newParameterValue);
			
			/* Request another frame: */
			scheduleUpdate(getApplicationTime()+1.0/125.0);
			}
		}
	
	if(pickedVertex!=0&&snapVertexToView)
		{
		/* Set the picked vertex to the current view position: */
		pickedVertex->center=getInverseNavigationTransformation().transform(getDisplayCenter());
		pickedVertex->size=Math::log(getDisplaySize()*getInverseNavigationTransformation().getScaling());
		pickedVertex->forward=getInverseNavigationTransformation().transform(getForwardDirection());
		pickedVertex->forward.normalize();
		pickedVertex->up=getInverseNavigationTransformation().transform(getUpDirection());
		pickedVertex->up.normalize();
		
		if(forceC2Continuity)
			{
			/* Force the curve to C^2 continuity if requested: */
			calculateC2Spline();
			}
		
		/* Update derived curve state: */
		updateCurve();
		}
	else if(editingMode!=IDLE)
		{
		Point p=getInverseNavigationTransformation().transform(getButtonDevicePosition(0));
		
		switch(editingMode)
			{
			case DRAGGING_VERTEX:
				{
				/* Move the picked vertex to the device's position: */
				Vector delta=(p+dragVertexOffset)-pickedVertex->center;
				pickedVertex->center+=delta;
				
				if(forceC2Continuity)
					{
					/* Force the curve to C^2 continuity if requested: */
					calculateC2Spline();
					}
				else
					{
					/* Drag the vertex' tangent handles: */
					if(pickedVertex->segments[0]!=0)
						pickedVertex->segments[0]->mid[1].center+=delta;
					if(pickedVertex->segments[1]!=0)
						pickedVertex->segments[1]->mid[0].center+=delta;
					}
				
				/* Update derived curve state: */
				updateCurve();
				
				break;
				}
			
			default:
				; // Just to make the compiler happy
			}
		}
	}

void CurveEditorTool::display(GLContextData& contextData) const
	{
	/* Set up and save OpenGL state: */
	glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT|GL_POINT_BIT);
	glDisable(GL_LIGHTING);
	
	/* Show the environment's display center and forward and up directions: */
	glLineWidth(3.0f);
	glBegin(GL_LINES);
	glColor3f(1.0f,0.0f,0.0f);
	glVertex(getDisplayCenter());
	glVertex(getDisplayCenter()+getForwardDirection()*getDisplaySize()*Scalar(0.25));
	glColor3f(0.0f,1.0f,0.0f);
	glVertex(getDisplayCenter());
	glVertex(getDisplayCenter()+getUpDirection()*getDisplaySize()*Scalar(0.25));
	glEnd();
	
	/* Go to navigational coordinates: */
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMultMatrix(getDisplayState(contextData).modelviewNavigational);
	
	/* Render all curve segments: */
	glLineWidth(3.0);
	glBegin(GL_LINES);
	const Vertex* v1=firstVertex;
	while(v1!=0)
		{
		/* Get the next segment and vertex: */
		const Segment* s=v1->segments[1];
		if(s==0)
			break;
		const Vertex* v2=s->vertices[1];
		
		/* Render the curve segment: */
		if(pickedSegment==s)
			glColor3f(1.0f,0.0f,0.0f);
		else
			glColor3f(0.5f,0.5f,0.5f);
		renderSegment(v1->center,s->mid[0].center,s->mid[1].center,v2->center,6);
		
		/* Go to the next segment: */
		v1=v2;
		}
	glEnd();
	
	if(!forceC2Continuity)
		{
		/* Render all tangents: */
		glLineWidth(1.0f);
		glBegin(GL_LINES);
		v1=firstVertex;
		while(v1!=0)
			{
			/* Get the next segment and vertex: */
			const Segment* s=v1->segments[1];
			if(s==0)
				break;

			/* Render the segment's tangent handles: */
			if(pickedHandleSegment==s&&pickedHandleIndex==0)
				glColor3f(1.0f,0.0f,0.0f);
			else
				glColor3f(1.0f,1.0f,0.0f);
			glVertex(v1->center);
			glVertex(s->mid[0].center);
			if(pickedHandleSegment==s&&pickedHandleIndex==1)
				glColor3f(1.0f,0.0f,0.0f);
			else
				glColor3f(1.0f,1.0f,0.0f);
			glVertex(s->mid[1].center);
			glVertex(s->vertices[1]->center);

			v1=s->vertices[1];
			}
		glEnd();
		
		/* Render all tangent handles: */
		glPointSize(3.0f);
		glBegin(GL_POINTS);
		v1=firstVertex;
		while(v1!=0)
			{
			/* Get the next segment and vertex: */
			const Segment* s=v1->segments[1];
			if(s==0)
				break;
			const Vertex* v2=s->vertices[1];

			/* Render the segment's tangent handles: */
			for(int handleIndex=0;handleIndex<2;++handleIndex)
				{
				if(pickedHandleSegment==s&&pickedHandleIndex==handleIndex)
					glColor3f(1.0f,0.0f,0.0f);
				else
					glColor3f(1.0f,1.0f,0.0f);
				glVertex(s->mid[handleIndex].center);
				}
			
			/* Go to the next segment: */
			v1=v2;
			}
		glEnd();
		}
	
	/* Render all control points: */
	glLineWidth(3.0f);
	glBegin(GL_LINES);
	v1=firstVertex;
	while(v1!=0)
		{
		/* Render the control point: */
		if(pickedVertex==v1)
			glColor3f(1.0f,0.0f,0.0f);
		else
			glColor3f(1.0f,1.0f,0.0f);
		glVertex(v1->center);
		glVertex(v1->center+v1->forward*Math::exp(v1->size)*Scalar(0.25));
		if(pickedVertex==v1)
			glColor3f(0.0f,1.0f,0.0f);
		else
			glColor3f(1.0f,1.0f,0.0f);
		glVertex(v1->center);
		glVertex(v1->center+v1->up*Math::exp(v1->size)*Scalar(0.25));
		
		/* Get the next segment and vertex: */
		const Segment* s=v1->segments[1];
		if(s==0)
			break;
		v1=s->vertices[1];
		}
	glEnd();
	
	if(pickedSegment!=0)
		{
		/* Render the picked point on the picked segment: */
		ControlPoint cp[6];
		cp[0]=ControlPoint::affineCombination(*pickedSegment->vertices[0],pickedSegment->mid[0],pickedSegmentParameter);
		cp[1]=ControlPoint::affineCombination(pickedSegment->mid[0],pickedSegment->mid[1],pickedSegmentParameter);
		cp[2]=ControlPoint::affineCombination(pickedSegment->mid[1],*pickedSegment->vertices[1],pickedSegmentParameter);
		for(int i=0;i<2;++i)
			cp[3+i]=ControlPoint::affineCombination(cp[i],cp[i+1],pickedSegmentParameter);
		cp[5]=ControlPoint::affineCombination(cp[3],cp[4],pickedSegmentParameter);
		
		/* Render the curve point: */
		glBegin(GL_LINES);
		glColor3f(1.0f,0.0f,0.0f);
		glVertex(cp[5].center);
		glVertex(cp[5].center+cp[5].forward*Math::exp(cp[5].size)*Scalar(0.25));
		glColor3f(0.0f,1.0f,0.0f);
		glVertex(cp[5].center);
		glVertex(cp[5].center+cp[5].up*Math::exp(cp[5].size)*Scalar(0.25));
		glEnd();
		}
	
	/* Restore OpenGL state: */
	glPopMatrix();
	glPopAttrib();
	}

}
