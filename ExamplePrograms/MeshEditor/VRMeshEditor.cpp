/***********************************************************************
VRMeshEditor - VR application to manipulate triangle meshes.
Copyright (c) 2003-2005 Oliver Kreylos
***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <iostream>
#include <Misc/File.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <Math/Random.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLNormalTemplates.h>
#include <GL/GLVertexTemplates.h>
#define GLVERTEX_NONSTANDARD_TEMPLATES
#include <GL/GLVertexArrayParts.h>
#include <GL/GLVertex.h>
#include <GL/GLLight.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>
#include <GL/GLModels.h>
#include <GL/GLGeometryWrappers.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/Menu.h>
#include <GLMotif/Button.h>
#include <GLMotif/CascadeButton.h>
#include <Vrui/LightsourceManager.h>
#include <Vrui/Vrui.h>

#include "MeshGenerators.h"
#include "CatmullClark.h"
#include "BallPivoting.h"
#include "SphereRenderer.h"

#include "VRMeshEditor.h"

/***************************************
Methods of class VRMeshEditor::DataItem:
***************************************/

VRMeshEditor::DataItem::DataItem(void)
	:vertexBufferId(0)
	{
	if(GLARBVertexBufferObject::isSupported())
		{
		/* Initialize the vertex buffer object extension: */
		GLARBVertexBufferObject::initExtension();
		
		/* Create a vertex buffer object: */
		glGenBuffersARB(1,&vertexBufferId);
		}
	}

VRMeshEditor::DataItem::~DataItem(void)
	{
	if(vertexBufferId>0)
		{
		/* Delete the vertex buffer object: */
		glDeleteBuffersARB(1,&vertexBufferId);
		}
	}

/*****************************
Methods of class VRMeshEditor:
*****************************/

GLMotif::Popup* VRMeshEditor::createLocatorTypesMenu(void)
	{
	GLMotif::Popup* locatorTypesMenuPopup=new GLMotif::Popup("LocatorTypesMenuPopup",Vrui::getWidgetManager());
	
	GLMotif::RadioBox* locatorTypes=new GLMotif::RadioBox("Locator Types",locatorTypesMenuPopup,false);
	locatorTypes->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
	
	locatorTypes->addToggle("Ball Pivot Locator");
	locatorTypes->addToggle("Vertex Removal Locator");
	locatorTypes->addToggle("Face Removal Locator");
	
	switch(defaultLocatorType)
		{
		case BALLPIVOTLOCATOR:
			locatorTypes->setSelectedToggle(0);
			break;
		
		case VERTEXREMOVALLOCATOR:
			locatorTypes->setSelectedToggle(1);
			break;
		
		case FACEREMOVALLOCATOR:
			locatorTypes->setSelectedToggle(2);
			break;
		}
	
	locatorTypes->getValueChangedCallbacks().add(this,&VRMeshEditor::locatorTypeValueChangedCallback);
	locatorTypes->manageChild();
	
	return locatorTypesMenuPopup;
	}

GLMotif::Popup* VRMeshEditor::createDraggerTypesMenu(void)
	{
	GLMotif::Popup* draggerTypesMenuPopup=new GLMotif::Popup("DraggerTypesMenuPopup",Vrui::getWidgetManager());
	
	GLMotif::RadioBox* draggerTypes=new GLMotif::RadioBox("Dragger Types",draggerTypesMenuPopup,false);
	draggerTypes->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
	
	draggerTypes->addToggle("Mesh Dragger");
	draggerTypes->addToggle("Morph Box Dragger");
	
	switch(defaultDraggerType)
		{
		case MESHDRAGGER:
			draggerTypes->setSelectedToggle(0);
			break;
		
		case MORPHBOXDRAGGER:
			draggerTypes->setSelectedToggle(1);
			break;
		}
	
	draggerTypes->getValueChangedCallbacks().add(this,&VRMeshEditor::draggerTypeValueChangedCallback);
	draggerTypes->manageChild();
	
	return draggerTypesMenuPopup;
	}

GLMotif::Popup* VRMeshEditor::createInfluenceActionsMenu(void)
	{
	GLMotif::Popup* influenceActionsMenuPopup=new GLMotif::Popup("InfluenceActionsMenuPopup",Vrui::getWidgetManager());
	
	GLMotif::RowColumn* influenceActionsMenu=new GLMotif::RowColumn("InfluenceActionsMenu",influenceActionsMenuPopup,false);
	
	GLMotif::RadioBox* influenceActions=new GLMotif::RadioBox("InfluenceActions",influenceActionsMenu,false);
	influenceActions->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
	
	influenceActions->addToggle("Drag");
	influenceActions->addToggle("Explode");
	influenceActions->addToggle("Smooth");
	
	switch(defaultActionType)
		{
		case Influence::DRAG:
			influenceActions->setSelectedToggle(0);
			break;
		
		case Influence::EXPLODE:
			influenceActions->setSelectedToggle(1);
			break;
		
		case Influence::WHITTLE:
			influenceActions->setSelectedToggle(2);
			break;
		}
	
	influenceActions->getValueChangedCallbacks().add(this,&VRMeshEditor::influenceActionValueChangedCallback);
	influenceActions->manageChild();
	
	GLMotif::ToggleButton* overrideToolsToggle=new GLMotif::ToggleButton("OverrideToolsToggle",influenceActionsMenu,"Override Tools");
	overrideToolsToggle->setBorderWidth(0.0f);
	overrideToolsToggle->setToggleType(GLMotif::ToggleButton::TOGGLE_BUTTON);
	overrideToolsToggle->setToggle(overrideTools);
	overrideToolsToggle->getValueChangedCallbacks().add(this,&VRMeshEditor::toggleButtonValueChangedCallback);
	
	influenceActionsMenu->manageChild();
	
	return influenceActionsMenuPopup;
	}

GLMotif::Popup* VRMeshEditor::createSettingsMenu(void)
	{
	GLMotif::Popup* settingsMenuPopup=new GLMotif::Popup("SettingsMenuPopup",Vrui::getWidgetManager());
	
	GLMotif::RowColumn* settingsMenu=new GLMotif::RowColumn("SettingsMenu",settingsMenuPopup,false);
	
	GLMotif::RadioBox* renderModes=new GLMotif::RadioBox("RenderModes",settingsMenu,false);
	renderModes->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
	
	renderModes->addToggle("Draw Wireframe");
	renderModes->addToggle("Draw Shaded Surface");
	
	renderModes->manageChild();
	switch(renderMode)
		{
		case WIREFRAME:
			renderModes->setSelectedToggle(0);
			break;
		
		case SHADED:
			renderModes->setSelectedToggle(1);
			break;
		}
	renderModes->getValueChangedCallbacks().add(this,&VRMeshEditor::renderModeValueChangedCallback);
	
	GLMotif::ToggleButton* showVertexColorsToggle=new GLMotif::ToggleButton("ShowVertexColorsToggle",settingsMenu,"Show Vertex Colors");
	showVertexColorsToggle->setBorderWidth(0.0f);
	showVertexColorsToggle->setToggleType(GLMotif::ToggleButton::TOGGLE_BUTTON);
	showVertexColorsToggle->setToggle(showVertexColors);
	showVertexColorsToggle->getValueChangedCallbacks().add(this,&VRMeshEditor::toggleButtonValueChangedCallback);
	
	GLMotif::ToggleButton* renderMeshVerticesToggle=new GLMotif::ToggleButton("RenderMeshVerticesToggle",settingsMenu,"Display Boundary Vertices");
	renderMeshVerticesToggle->setBorderWidth(0.0f);
	renderMeshVerticesToggle->setToggleType(GLMotif::ToggleButton::TOGGLE_BUTTON);
	renderMeshVerticesToggle->setToggle(renderMeshVertices);
	renderMeshVerticesToggle->getValueChangedCallbacks().add(this,&VRMeshEditor::toggleButtonValueChangedCallback);
	
	GLMotif::ToggleButton* renderMeshVerticesTransparentToggle=new GLMotif::ToggleButton("RenderMeshVerticesTransparentToggle",settingsMenu,"Display Boundary Vertices Transparent");
	renderMeshVerticesTransparentToggle->setBorderWidth(0.0f);
	renderMeshVerticesTransparentToggle->setToggleType(GLMotif::ToggleButton::TOGGLE_BUTTON);
	renderMeshVerticesTransparentToggle->setToggle(renderMeshVerticesTransparent);
	renderMeshVerticesTransparentToggle->getValueChangedCallbacks().add(this,&VRMeshEditor::toggleButtonValueChangedCallback);
	
	settingsMenu->manageChild();
	
	return settingsMenuPopup;
	}

GLMotif::PopupMenu* VRMeshEditor::createMainMenu(void)
	{
	GLMotif::PopupMenu* mainMenuPopup=new GLMotif::PopupMenu("MainMenuPopup",Vrui::getWidgetManager());
	mainMenuPopup->setTitle("Liquid Metal Editing");
	
	GLMotif::Menu* mainMenu=new GLMotif::Menu("MainMenu",mainMenuPopup,false);
	
	GLMotif::Button* centerDisplayButton=new GLMotif::Button("CenterDisplayButton",mainMenu,"Center Display");
	centerDisplayButton->getSelectCallbacks().add(this,&VRMeshEditor::centerDisplayCallback);
	
	GLMotif::CascadeButton* locatorTypesCascade=new GLMotif::CascadeButton("LocatorTypesCascade",mainMenu,"Locator Types");
	locatorTypesCascade->setPopup(createLocatorTypesMenu());
	
	GLMotif::CascadeButton* draggerTypesCascade=new GLMotif::CascadeButton("DraggerTypesCascade",mainMenu,"Dragger Types");
	draggerTypesCascade->setPopup(createDraggerTypesMenu());
	
	GLMotif::CascadeButton* influenceActionsCascade=new GLMotif::CascadeButton("InfluenceActionsCascade",mainMenu,"Influence Actions");
	influenceActionsCascade->setPopup(createInfluenceActionsMenu());
	
	GLMotif::Button* createMorphBoxButton=new GLMotif::Button("CreateMorphBoxButton",mainMenu,"Create Morph Box");
	createMorphBoxButton->getSelectCallbacks().add(this,&VRMeshEditor::createMorphBoxCallback);
	
	GLMotif::Button* deleteMorphBoxButton=new GLMotif::Button("DeleteMorphBoxButton",mainMenu,"Delete Morph Box");
	deleteMorphBoxButton->getSelectCallbacks().add(this,&VRMeshEditor::deleteMorphBoxCallback);
	
	GLMotif::CascadeButton* settingsCascade=new GLMotif::CascadeButton("SettingsCascade",mainMenu,"Settings");
	settingsCascade->setPopup(createSettingsMenu());
	
	GLMotif::Button* saveMeshButton=new GLMotif::Button("SaveMeshButton",mainMenu,"Save Mesh");
	saveMeshButton->getSelectCallbacks().add(this,&VRMeshEditor::saveMeshCallback);
	
	GLMotif::Button* createInputDeviceButton=new GLMotif::Button("CreateInputDeviceButton",mainMenu,"Create Input Device");
	createInputDeviceButton->getSelectCallbacks().add(this,&VRMeshEditor::createInputDeviceCallback);
	
	mainMenu->manageChild();
	
	return mainMenuPopup;
	}

void VRMeshEditor::renderMesh(VRMeshEditor::DataItem* dataItem) const
	{
	if(dataItem->vertexBufferId>0)
		{
		/* Stream the point data into the vertex buffer: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferId);
		int vertexPartsMask=GLVertexArrayParts::Normal|GLVertexArrayParts::Position;
		if(showVertexColors)
			vertexPartsMask|=GLVertexArrayParts::Color;
		GLVertexArrayParts::enable(vertexPartsMask);
		
		const int bufferSize=5000; // Number of triangles per vertex buffer
		
		if(showVertexColors)
			{
			/* Render all triangles with vertex color data: */
			typedef GLVertex<void,0,GLubyte,4,GLfloat,GLfloat,3> MyVertex;
			
			/* Prepare and map a streaming vertex buffer: */
			glBufferDataARB(GL_ARRAY_BUFFER_ARB,bufferSize*3*sizeof(MyVertex),0,GL_STREAM_DRAW_ARB);
			MyVertex* vertices=static_cast<MyVertex*>(glMapBufferARB(GL_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
			int bufferSizeLeft=bufferSize;
			MyVertex* vPtr=vertices;
			
			/* Pass all triangles through the vertex buffer: */
			for(MyMesh::ConstFaceIterator fIt=mesh->beginFaces();fIt!=mesh->endFaces();++fIt)
				{
				/* Swap the buffer if the current buffer is full: */
				if(bufferSizeLeft==0)
					{
					/* Render the current buffer: */
					glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
					glVertexPointer((MyVertex*)0);
					glDrawArrays(GL_TRIANGLES,0,bufferSize*3);
					
					/* Prepare a new buffer: */
					glBufferDataARB(GL_ARRAY_BUFFER_ARB,bufferSize*3*sizeof(MyVertex),0,GL_STREAM_DRAW_ARB);
					vertices=static_cast<MyVertex*>(glMapBufferARB(GL_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
					bufferSizeLeft=bufferSize;
					vPtr=vertices;
					}
				
				/* Put the triangle into the buffer: */
				for(MyMesh::ConstFaceEdgeIterator feIt=fIt.beginEdges();feIt!=fIt.endEdges();++feIt,++vPtr)
					{
					for(int i=0;i<4;++i)
						vPtr->color[i]=feIt->getStart()->color[i];
					for(int i=0;i<3;++i)
						vPtr->normal[i]=feIt->getStart()->normal[i];
					for(int i=0;i<3;++i)
						vPtr->position[i]=(*feIt->getStart())[i];
					}
				--bufferSizeLeft;
				}
			
			glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
			if(bufferSizeLeft<bufferSize)
				{
				/* Render the last buffer: */
				glVertexPointer((MyVertex*)0);
				glDrawArrays(GL_TRIANGLES,0,(bufferSize-bufferSizeLeft)*3);
				}
			}
		else
			{
			/* Render all triangles without vertex color data: */
			typedef GLVertex<void,0,void,0,GLfloat,GLfloat,3> MyVertex;
			
			/* Prepare and map a streaming vertex buffer: */
			glBufferDataARB(GL_ARRAY_BUFFER_ARB,bufferSize*3*sizeof(MyVertex),0,GL_STREAM_DRAW_ARB);
			MyVertex* vertices=static_cast<MyVertex*>(glMapBufferARB(GL_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
			int bufferSizeLeft=bufferSize;
			MyVertex* vPtr=vertices;
			
			/* Pass all triangles through the vertex buffer: */
			for(MyMesh::ConstFaceIterator fIt=mesh->beginFaces();fIt!=mesh->endFaces();++fIt)
				{
				/* Swap the buffer if the current buffer is full: */
				if(bufferSizeLeft==0)
					{
					/* Render the current buffer: */
					glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
					glVertexPointer((MyVertex*)0);
					glDrawArrays(GL_TRIANGLES,0,bufferSize*3);
					
					/* Prepare a new buffer: */
					glBufferDataARB(GL_ARRAY_BUFFER_ARB,bufferSize*3*sizeof(MyVertex),0,GL_STREAM_DRAW_ARB);
					vertices=static_cast<MyVertex*>(glMapBufferARB(GL_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
					bufferSizeLeft=bufferSize;
					vPtr=vertices;
					}
				
				/* Put the triangle into the buffer: */
				for(MyMesh::ConstFaceEdgeIterator feIt=fIt.beginEdges();feIt!=fIt.endEdges();++feIt,++vPtr)
					{
					for(int i=0;i<3;++i)
						vPtr->normal[i]=feIt->getStart()->normal[i];
					for(int i=0;i<3;++i)
						vPtr->position[i]=(*feIt->getStart())[i];
					}
				--bufferSizeLeft;
				}
			
			glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
			if(bufferSizeLeft<bufferSize)
				{
				/* Render the last buffer: */
				glVertexPointer((MyVertex*)0);
				glDrawArrays(GL_TRIANGLES,0,(bufferSize-bufferSizeLeft)*3);
				}
			}
		
		GLVertexArrayParts::disable(vertexPartsMask);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		}
	else
		{
		glBegin(GL_TRIANGLES);
		if(showVertexColors)
			for(MyMesh::ConstFaceIterator fIt=mesh->beginFaces();fIt!=mesh->endFaces();++fIt)
				for(MyMesh::ConstFaceEdgeIterator feIt=fIt.beginEdges();feIt!=fIt.endEdges();++feIt)
					{
					glColor(feIt->getStart()->color);
					glNormal(feIt->getStart()->normal);
					glVertex(*feIt->getStart());
					}
		else
			for(MyMesh::ConstFaceIterator fIt=mesh->beginFaces();fIt!=mesh->endFaces();++fIt)
				for(MyMesh::ConstFaceEdgeIterator feIt=fIt.beginEdges();feIt!=fIt.endEdges();++feIt)
					{
					glNormal(feIt->getStart()->normal);
					glVertex(*feIt->getStart());
					}
		glEnd();
		}
	}

void VRMeshEditor::renderMeshWireframe(VRMeshEditor::DataItem* dataItem) const
	{
	glDisable(GL_LIGHTING);
	
	/* Render all mesh edges: */
	glLineWidth(1.0f);
	glBegin(GL_LINES);
	for(MyMesh::ConstFaceIterator fIt=mesh->beginFaces();fIt!=mesh->endFaces();++fIt)
		for(MyMesh::ConstFaceEdgeIterator feIt=fIt.beginEdges();feIt!=fIt.endEdges();++feIt)
			if(feIt.isUpperHalf())
				{
				if(feIt->getOpposite()==0)
					glColor3f(1.0f,1.0f,0.0f);
				else
					glColor3f(0.5f,0.5f,0.5f);
				glVertex(*feIt->getStart());
				glVertex(*feIt->getEnd());
				}
	glEnd();
	}

void VRMeshEditor::renderMeshBoundaryVertices(VRMeshEditor::DataItem* dataItem) const
	{
	glDisable(GL_LIGHTING);
	if(renderMeshVerticesTransparent)
		{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE,GL_ONE);
		glDepthMask(GL_FALSE);
		glPointSize(1.0f);
		glColor3f(0.2f,0.2f,0.2f);
		}
	else
		{
		glPointSize(3.0f);
		glColor3f(0.0f,1.0f,0.0f);
		}
	
	/* Render all boundary vertices: */
	if(dataItem->vertexBufferId>0)
		{
		/* Stream the point data into the vertex buffer: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferId);
		int vertexPartsMask=GLVertexArrayParts::Position;
		if(showVertexColors)
			vertexPartsMask|=GLVertexArrayParts::Color;
		GLVertexArrayParts::enable(vertexPartsMask);
		
		const int bufferSize=10000; // Number of vertices per vertex buffer
		
		if(showVertexColors&&!renderMeshVerticesTransparent)
			{
			/* Render all vertices with color data: */
			typedef GLVertex<void,0,GLubyte,4,void,GLfloat,3> MyVertex;
			
			/* Prepare and map a streaming vertex buffer: */
			glBufferDataARB(GL_ARRAY_BUFFER_ARB,bufferSize*sizeof(MyVertex),0,GL_STREAM_DRAW_ARB);
			MyVertex* vertices=static_cast<MyVertex*>(glMapBufferARB(GL_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
			int bufferSizeLeft=bufferSize;
			MyVertex* vPtr=vertices;
			
			/* Pass all vertices through the vertex buffer: */
			for(MyMesh::ConstVertexIterator vIt=mesh->beginVertices();vIt!=mesh->endVertices();++vIt)
				if(!vIt->isInterior())
					{
					/* Swap the buffer if the current buffer is full: */
					if(bufferSizeLeft==0)
						{
						/* Render the current buffer: */
						glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
						glVertexPointer((MyVertex*)0);
						glDrawArrays(GL_POINTS,0,bufferSize);
						
						/* Prepare a new buffer: */
						glBufferDataARB(GL_ARRAY_BUFFER_ARB,bufferSize*sizeof(MyVertex),0,GL_STREAM_DRAW_ARB);
						vertices=static_cast<MyVertex*>(glMapBufferARB(GL_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
						bufferSizeLeft=bufferSize;
						vPtr=vertices;
						}
					
					/* Put the vertex into the buffer: */
					for(int i=0;i<4;++i)
						vPtr->color[i]=vIt->color[i];
					for(int i=0;i<3;++i)
						vPtr->position[i]=(*vIt)[i];
					++vPtr;
					--bufferSizeLeft;
					}
			
			glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
			if(bufferSizeLeft<bufferSize)
				{
				/* Render the last buffer: */
				glVertexPointer((MyVertex*)0);
				glDrawArrays(GL_POINTS,0,bufferSize-bufferSizeLeft);
				}
			}
		else
			{
			/* Render all vertices without color data: */
			typedef GLVertex<void,0,void,0,void,GLfloat,3> MyVertex;
			
			/* Prepare and map a streaming vertex buffer: */
			glBufferDataARB(GL_ARRAY_BUFFER_ARB,bufferSize*sizeof(MyVertex),0,GL_STREAM_DRAW_ARB);
			MyVertex* vertices=static_cast<MyVertex*>(glMapBufferARB(GL_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
			int bufferSizeLeft=bufferSize;
			MyVertex* vPtr=vertices;
			
			/* Pass all vertices through the vertex buffer: */
			for(MyMesh::ConstVertexIterator vIt=mesh->beginVertices();vIt!=mesh->endVertices();++vIt)
				if(!vIt->isInterior())
					{
					/* Swap the buffer if the current buffer is full: */
					if(bufferSizeLeft==0)
						{
						/* Render the current buffer: */
						glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
						glVertexPointer((MyVertex*)0);
						glDrawArrays(GL_POINTS,0,bufferSize);
						
						/* Prepare a new buffer: */
						glBufferDataARB(GL_ARRAY_BUFFER_ARB,bufferSize*sizeof(MyVertex),0,GL_STREAM_DRAW_ARB);
						vertices=static_cast<MyVertex*>(glMapBufferARB(GL_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
						bufferSizeLeft=bufferSize;
						vPtr=vertices;
						}
					
					/* Put the vertex into the buffer: */
					for(int i=0;i<3;++i)
						vPtr->position[i]=(*vIt)[i];
					++vPtr;
					--bufferSizeLeft;
					}
			
			glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
			if(bufferSizeLeft<bufferSize)
				{
				/* Render the last buffer: */
				glVertexPointer((MyVertex*)0);
				glDrawArrays(GL_POINTS,0,bufferSize-bufferSizeLeft);
				}
			}
		
		GLVertexArrayParts::disable(vertexPartsMask);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		}
	else
		{
		glBegin(GL_POINTS);
		if(showVertexColors&&!renderMeshVerticesTransparent)
			{
			for(MyMesh::ConstVertexIterator vIt=mesh->beginVertices();vIt!=mesh->endVertices();++vIt)
				if(!vIt->isInterior())
					{
					glColor(vIt->color);
					glVertex(*vIt);
					}
			}
		else
			{
			for(MyMesh::ConstVertexIterator vIt=mesh->beginVertices();vIt!=mesh->endVertices();++vIt)
				if(!vIt->isInterior())
					glVertex(*vIt);
			}
		glEnd();
		}
	
	if(renderMeshVerticesTransparent)
		{
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		}
	}

void VRMeshEditor::setMeshDraggerActionType(Influence::ActionType newActionType)
	{
	/* Set the new default action type: */
	defaultActionType=newActionType;
	
	if(overrideTools)
		{
		/* Set action type of all mesh draggers to the new default: */
		for(DraggerList::iterator dIt=draggers.begin();dIt!=draggers.end();++dIt)
			if(dynamic_cast<MeshDragger*>(*dIt)!=0)
				dynamic_cast<MeshDragger*>(*dIt)->setAction(newActionType);
		}
	}

AutoTriangleMesh* VRMeshEditor::loadEarthquakeEvents(const char* filename)
	{
	/* Open the earthquake file: */
	Misc::File earthquakeFile(filename,"rt");

	/* Skip the file header: */
	char line[256];
	do
		{
		earthquakeFile.gets(line,sizeof(line));
		}
	while(strncmp(line,"----------",10)!=0);

	/* Create the result mesh: */
	AutoTriangleMesh* result=new AutoTriangleMesh;

	/* Parse all earthquake events: */
	const double a=6378.14e3; // Equatorial radius in m
	const double f=1.0/298.247; // Geoid flattening factor
	const double scaleFactor=0.001;
	while(!earthquakeFile.eof())
		{
		earthquakeFile.gets(line,sizeof(line));

		double coord[3];
		double magnitude;

		bool validEvent=true;

		/* Ignore date and time for now: */
		line[22]='\0';

		/* Parse latitude: */
		line[31]='\0';
		if(sscanf(line+23,"%lf",&coord[0])!=1)
			validEvent=false;

		/* Parse longitude: */
		line[41]='\0';
		if(sscanf(line+32,"%lf",&coord[1])!=1)
			validEvent=false;

		/* Parse depth: */
		line[48]='\0';
		if(sscanf(line+42,"%lf",&coord[2])!=1)
			validEvent=false;

		/* Parse magnitude: */
		line[54]='\0';
		if(sscanf(line+49,"%lf",&magnitude)!=1)
			validEvent=false;

		if(validEvent)
			{
			double lat=(coord[0]*Math::Constants<double>::pi)/180.0;
			double lng=(coord[1]*Math::Constants<double>::pi)/180.0;
			double s=Math::sin(lat);
			double c=Math::cos(lat);
			double r=(a*(1.0-f*s*s)-coord[2]*1000.0)*scaleFactor;
			double xy=r*c;
			double cl=Math::cos(lng);
			double sl=Math::sin(lng);
			AutoTriangleMesh::Point p(xy*cl,xy*sl,r*s);
			result->addVertex(p,AutoTriangleMesh::Color(255,255,255));
			}
		}
	
	return result;
	}

AutoTriangleMesh* VRMeshEditor::loadSphericalPoints(const char* filename)
	{
	/* Open the point file: */
	Misc::File pointFile(filename,"rt");

	/* Create the result mesh: */
	AutoTriangleMesh* result=new AutoTriangleMesh;

	/* Parse all points: */
	const double a=6378.14e3; // Equatorial radius in m
	const double f=1.0/298.247; // Geoid flattening factor
	const double scaleFactor=0.001;
	char line[256];
	while(!pointFile.eof())
		{
		pointFile.gets(line,sizeof(line));
		
		/* Parse the spherical point coordinates from the line: */
		float sp[3];
		sscanf(line,"%f %f %f",&sp[0],&sp[1],&sp[2]);
		
		if(!isnan(sp[2]))
			{
			/* Convert spherical coordinates (longitude, latitude, depth) to Cartesian coordinates: */
			double lng=Math::rad(double(sp[0]));
			double lat=Math::rad(double(sp[1]));
			double s=Math::sin(lat);
			double c=Math::cos(lat);
			double r=(a*(1.0-f*s*s)+double(sp[2])*1000.0)*scaleFactor;
			double xy=r*c;
			double cl=Math::cos(lng);
			double sl=Math::sin(lng);
			
			/* Store the point in the set: */
			AutoTriangleMesh::Point p(xy*cl,xy*sl,r*s);
			result->addVertex(p,AutoTriangleMesh::Color(255,255,255));
			}
		}
	
	return result;
	}

AutoTriangleMesh* VRMeshEditor::loadVolumePoints(const char* filename)
	{
	/* Open the volume points file: */
	Misc::File volumePointsFile(filename,"rt");
	
	#if 1
	int size[3];
	volumePointsFile.read(size,3);
	int borderSize=volumePointsFile.read<int>();
	float realSize[3];
	volumePointsFile.read(realSize,3);
	unsigned char* volume=new unsigned char[size[0]*size[1]*size[2]];
	volumePointsFile.read(volume,size[0]*size[1]*size[2]);
	float cellSize[3];
	for(int i=0;i<3;++i)
		cellSize[i]=realSize[i]/float(size[i]-1);
	
	/* Create the result mesh: */
	AutoTriangleMesh* result=new AutoTriangleMesh;
	
	for(int z=0;z<size[0];++z)
		for(int y=0;y<size[1];++y)
			for(int x=0;x<size[2];++x)
				{
				unsigned char intensity=volume[(z*size[1]+y)*size[2]+x];
				if(intensity>0)
					{
					AutoTriangleMesh::Point p(float(z)*cellSize[0],float(y)*cellSize[1],float(x)*cellSize[2]);
					for(int i=0;i<3;++i)
						p[i]+=float(Math::randUniformCC(-0.05*cellSize[i],0.05*cellSize[i]));
					
					result->addVertex(p,AutoTriangleMesh::Color(intensity,intensity,intensity));
					}
				}
	delete[] volume;
	
	return result;
	#else
	/* Skip the file header: */
	char line[256];
	do
		{
		volumePointsFile.gets(line,sizeof(line));
		}
	while(line[0]=='#');
	
	/* Create the result mesh: */
	AutoTriangleMesh* result=new AutoTriangleMesh;
	
	/* Parse all volume points: */
	while(!volumePointsFile.eof())
		{
		int coord[3];
		int intensity;
		sscanf(line,"%d %d %d %d",&coord[0],&coord[1],&coord[2],&intensity);
		
		//if(coord[2]>=10&&coord[2]<=13&&intensity<256)
			{
			AutoTriangleMesh::Point p;
			for(int i=0;i<3;++i)
				p[i]=float(coord[i])+float(Math::randUniformCC(-0.05,0.05));
			
			result->addVertex(p,AutoTriangleMesh::Color(intensity,intensity,intensity));
			}
		
		volumePointsFile.gets(line,sizeof(line));
		}
	
	return result;
	#endif
	}

VRMeshEditor::VRMeshEditor(int& argc,char**& argv,char**& appDefaults)
	:Vrui::Application(argc,argv,appDefaults),
	 mesh(0),
	 morphBox(0),
	 defaultLocatorType(BALLPIVOTLOCATOR),
	 defaultDraggerType(MESHDRAGGER),
	 defaultActionType(Influence::DRAG),
	 overrideTools(true),
	 sphereRenderer(new SphereRenderer(1.0f,5)),
	 renderMode(SHADED),
	 showVertexColors(false),
	 meshMaterial(GLMaterial::Color(0.7f,0.7f,0.7f),GLMaterial::Color(0.5f,0.5f,0.5f),20.0f),
	 frontMaterial(GLMaterial::Color(1.0f,0.0f,0.0f),GLMaterial::Color(0.5f,0.5f,0.5f),20.0f),
	 backMaterial(GLMaterial::Color(0.5f,0.5f,1.0f),GLMaterial::Color(0.5f,0.5f,0.5f),20.0f),
	 renderMeshVertices(false),renderMeshVerticesTransparent(false),
	 mainMenu(0)
	{
	/* Parse the command line: */
	const char* meshFileName=0;
	int subdivisionDepth=0;
	int inputFileType=0;
	int numEdges=0;
	for(int i=1;i<argc;++i)
		{
		if(argv[i][0]=='-')
			{
			if(strcasecmp(argv[i]+1,"QUAKES")==0)
				inputFileType=1;
			else if(strcasecmp(argv[i]+1,"POINTS")==0)
				inputFileType=2;
			else if(strcasecmp(argv[i]+1,"VOLUME")==0)
				inputFileType=3;
			else if(strcasecmp(argv[i]+1,"DEPTH")==0)
				{
				subdivisionDepth=atoi(argv[i+1]);
				++i;
				}
			else if(strcasecmp(argv[i]+1,"PIVOT")==0)
				{
				numEdges=atoi(argv[i+1]);
				++i;
				}
			}
		else
			meshFileName=argv[i];
		}
	if(meshFileName==0)
		throw std::runtime_error("VRMeshEditor: No mesh file name supplied");
	
	/* Get the mesh file name's extension: */
	const char* extPtr;
	for(const char* cPtr=meshFileName;*cPtr!='\0';++cPtr)
		if(*cPtr=='.')
			extPtr=cPtr;
	
	switch(inputFileType)
		{
		case 0:
			{
			/* Load the base mesh: */
			MyMesh::BaseMesh* baseMesh=0;
			if(strcasecmp(extPtr,".mesh")==0)
				baseMesh=loadMeshfile(meshFileName);
			else if(strcasecmp(extPtr,".obj")==0)
				baseMesh=loadObjMeshfile(meshFileName);
			else if(strcasecmp(extPtr,".gts")==0)
				baseMesh=loadGtsMeshfile(meshFileName);
			else if(strcasecmp(extPtr,".ply")==0)
				baseMesh=loadPlyMeshfile(meshFileName);
			
			/* Subdivide the base mesh: */
			for(int i=0;i<subdivisionDepth;++i)
				subdivideCatmullClark(*baseMesh);
			mesh=new MyMesh(*baseMesh);
			delete baseMesh;
			break;
			}
		
		case 1:
			mesh=loadEarthquakeEvents(meshFileName);
			break;
		
		case 2:
			mesh=loadSphericalPoints(meshFileName);
			break;
		
		case 3:
			mesh=loadVolumePoints(meshFileName);
			break;
		}
	
	/* Create the main menu: */
	mainMenu=createMainMenu();
	Vrui::setMainMenu(mainMenu);
	
	/* Set additional physical-space light sources: */
	Vrui::Point l1=Vrui::getDisplayCenter()+Vrui::Vector(-1.0,-0.1,0.5)*Vrui::getDisplaySize();
	Vrui::Point l2=Vrui::getDisplayCenter()+Vrui::Vector(1.0,-0.1,0.5)*Vrui::getDisplaySize();
	Vrui::getLightsourceManager()->createLightsource(true,GLLight(GLLight::Color(0.5f,0.25f,0.25f),GLLight::Position(l1[0],l1[1],l1[2],1.0f)));
	Vrui::getLightsourceManager()->createLightsource(true,GLLight(GLLight::Color(0.25f,0.25f,0.5f),GLLight::Position(l2[0],l2[1],l2[2],1.0f)));
	
	/* Initialize navigation transformation: */
	centerDisplayCallback(0);
	
	#if 0
	/* Debugging code starts here: */
	{
	AutoTriangleMesh::Point ballStart(-8.46468, -44.6383, -1034.78);
	AutoTriangleMesh::Vector ballDirection(0.0550971, -0.0453198, -0.867932);
	double ballRadius=1.44998;
	ShootBallResult sbr=shootBall(*mesh,ballStart,ballDirection,ballRadius);
	
	BPState* bpState=startBallPivoting(*mesh,sbr);
	pivotOnce(bpState,numEdges);
	finishBallPivoting(bpState);
	bpState=0;
	}
	#endif
	}

VRMeshEditor::~VRMeshEditor(void)
	{
	delete morphBox;
	delete mesh;
	delete sphereRenderer;
	delete mainMenu;
	}

void VRMeshEditor::initContext(GLContextData& contextData) const
	{
	/* Create a new context entry: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	}

void VRMeshEditor::toolCreationCallback(Vrui::ToolManager::ToolCreationCallbackData* cbData)
	{
	/* Check if the new tool is a locator tool: */
	Vrui::LocatorTool* ltool=dynamic_cast<Vrui::LocatorTool*>(cbData->tool);
	if(ltool!=0)
		{
		Locator* newLocator=0;
		switch(defaultLocatorType)
			{
			case BALLPIVOTLOCATOR:
				/* Create a ball pivot locator object and associate it with the new tool: */
				newLocator=new BallPivotLocator(ltool,this);
				break;
			
			case VERTEXREMOVALLOCATOR:
				/* Create a vertex removal locator object and associate it with the new tool: */
				newLocator=new VertexRemovalLocator(ltool,this);
				break;
			
			case FACEREMOVALLOCATOR:
				/* Create a face removal locator object and associate it with the new tool: */
				newLocator=new FaceRemovalLocator(ltool,this);
				break;
			}
		
		/* Add new locator to list: */
		locators.push_back(newLocator);
		}
	
	/* Check if the new tool is a dragging tool: */
	Vrui::DraggingTool* dtool=dynamic_cast<Vrui::DraggingTool*>(cbData->tool);
	if(dtool!=0)
		{
		Dragger* newDragger=0;
		switch(defaultDraggerType)
			{
			case MESHDRAGGER:
				/* Create a mesh dragger object and associate it with the new tool: */
				newDragger=new MeshDragger(dtool,this);
				break;
			
			case MORPHBOXDRAGGER:
				/* Create a morph box dragger object and associate it with the new tool: */
				newDragger=new MorphBoxDragger(dtool,this);
				break;
			}
		
		/* Add new dragger to list: */
		draggers.push_back(newDragger);
		}
	}

void VRMeshEditor::toolDestructionCallback(Vrui::ToolManager::ToolDestructionCallbackData* cbData)
	{
	/* Check if the to-be-destroyed tool is a locator tool: */
	Vrui::LocatorTool* ltool=dynamic_cast<Vrui::LocatorTool*>(cbData->tool);
	if(ltool!=0)
		{
		/* Find the locator associated with the tool in the list: */
		LocatorList::iterator dIt;
		for(dIt=locators.begin();dIt!=locators.end()&&(*dIt)->getTool()!=ltool;++dIt)
			;
		if(dIt!=locators.end())
			{
			/* Remove the locator: */
			delete *dIt;
			locators.erase(dIt);
			}
		}
	
	/* Check if the to-be-destroyed tool is a dragging tool: */
	Vrui::DraggingTool* dtool=dynamic_cast<Vrui::DraggingTool*>(cbData->tool);
	if(dtool!=0)
		{
		/* Find the dragger associated with the tool in the list: */
		DraggerList::iterator dIt;
		for(dIt=draggers.begin();dIt!=draggers.end()&&(*dIt)->getTool()!=dtool;++dIt)
			;
		if(dIt!=draggers.end())
			{
			/* Remove the dragger: */
			delete *dIt;
			draggers.erase(dIt);
			}
		}
	}

void VRMeshEditor::frame(void)
	{
	}

void VRMeshEditor::display(GLContextData& contextData) const
	{
	/* Retrieve context entry: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	/* Render mesh: */
	glPushAttrib(GL_ENABLE_BIT|GL_LIGHTING_BIT|GL_POLYGON_BIT);
	switch(renderMode)
		{
		case SHADED:
			glEnable(GL_LIGHTING);
			glEnable(GL_NORMALIZE);
			glDisable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(GL_CCW);
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
			if(showVertexColors)
				{
				glMaterial(GLMaterialEnums::FRONT_AND_BACK,meshMaterial);
				glEnable(GL_COLOR_MATERIAL);
				glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
				}
			else
				{
				glMaterial(GLMaterialEnums::FRONT,frontMaterial);
				glMaterial(GLMaterialEnums::BACK,backMaterial);
				}
			renderMesh(dataItem);
			break;
		
		case WIREFRAME:
			renderMeshWireframe(dataItem);
			break;
		}
	
	if(renderMeshVertices)
		renderMeshBoundaryVertices(dataItem);
	
	glPopAttrib();
	
	/* Render all locators: */
	for(LocatorList::const_iterator lIt=locators.begin();lIt!=locators.end();++lIt)
		(*lIt)->glRenderAction(contextData);
	
	/* Render all draggers: */
	for(DraggerList::const_iterator dIt=draggers.begin();dIt!=draggers.end();++dIt)
		(*dIt)->glRenderAction(contextData);
	
	/* Render the morph box: */
	if(morphBox!=0)
		morphBox->glRenderAction(contextData);
	}

void VRMeshEditor::locatorTypeValueChangedCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData)
	{
	switch(cbData->radioBox->getToggleIndex(cbData->newSelectedToggle))
		{
		case 0:
			defaultLocatorType=BALLPIVOTLOCATOR;
			break;
		
		case 1:
			defaultLocatorType=VERTEXREMOVALLOCATOR;
			break;
		
		case 2:
			defaultLocatorType=FACEREMOVALLOCATOR;
			break;
		}
	}

void VRMeshEditor::draggerTypeValueChangedCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData)
	{
	switch(cbData->radioBox->getToggleIndex(cbData->newSelectedToggle))
		{
		case 0:
			defaultDraggerType=MESHDRAGGER;
			break;
		
		case 1:
			defaultDraggerType=MORPHBOXDRAGGER;
			break;
		}
	}

void VRMeshEditor::influenceActionValueChangedCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData)
	{
	switch(cbData->radioBox->getToggleIndex(cbData->newSelectedToggle))
		{
		case 0:
			setMeshDraggerActionType(Influence::DRAG);
			break;
		
		case 1:
			setMeshDraggerActionType(Influence::EXPLODE);
			break;
		
		case 2:
			setMeshDraggerActionType(Influence::WHITTLE);
			break;
		}
	}

void VRMeshEditor::renderModeValueChangedCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData)
	{
	switch(cbData->radioBox->getToggleIndex(cbData->newSelectedToggle))
		{
		case 0:
			renderMode=WIREFRAME;
			break;
		
		case 1:
			renderMode=SHADED;
			break;
		}
	}

void VRMeshEditor::toggleButtonValueChangedCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	if(strcmp(cbData->toggle->getName(),"OverrideToolsToggle")==0)
		overrideTools=cbData->set;
	else if(strcmp(cbData->toggle->getName(),"ShowVertexColorsToggle")==0)
		showVertexColors=cbData->set;
	else if(strcmp(cbData->toggle->getName(),"RenderMeshVerticesToggle")==0)
		renderMeshVertices=cbData->set;
	else if(strcmp(cbData->toggle->getName(),"RenderMeshVerticesTransparentToggle")==0)
		renderMeshVerticesTransparent=cbData->set;
	}

void VRMeshEditor::centerDisplayCallback(Misc::CallbackData* cbData)
	{
	/* Calculate bounding box of current mesh: */
	Vrui::Point bbMin,bbMax;
	MyVIt vIt=mesh->beginVertices();
	for(int i=0;i<3;++i)
		bbMin[i]=bbMax[i]=(*vIt)[i];
	for(++vIt;vIt!=mesh->endVertices();++vIt)
		{
		for(int i=0;i<3;++i)
			{
			if(bbMin[i]>(*vIt)[i])
				bbMin[i]=(*vIt)[i];
			else if(bbMax[i]<(*vIt)[i])
				bbMax[i]=(*vIt)[i];
			}
		}
	Vrui::Point modelCenter=Geometry::mid(bbMin,bbMax);
	Vrui::Scalar modelSize=Geometry::dist(modelCenter,bbMax);
	
	/* Calculate navigation transformation: */
	Vrui::NavTransform t=Vrui::NavTransform::translateFromOriginTo(Vrui::getDisplayCenter());
	t*=Vrui::NavTransform::scale(Vrui::Scalar(0.5)*Vrui::getDisplaySize()/modelSize);
	t*=Vrui::NavTransform::translateToOriginFrom(modelCenter);
	Vrui::setNavigationTransformation(t);
	}

void VRMeshEditor::createMorphBoxCallback(Misc::CallbackData* cbData)
	{
	/* Delete the old morph box: */
	if(morphBox!=0)
		{
		delete morphBox;
		morphBox=0;
		}
	
	/* Calculate bounding box of current mesh: */
	MyMorphBox::Point bbMin,bbMax;
	MyVIt vIt=mesh->beginVertices();
	for(int i=0;i<3;++i)
		bbMin[i]=bbMax[i]=(*vIt)[i];
	for(++vIt;vIt!=mesh->endVertices();++vIt)
		{
		for(int i=0;i<3;++i)
			{
			if(bbMin[i]>(*vIt)[i])
				bbMin[i]=(*vIt)[i];
			else if(bbMax[i]<(*vIt)[i])
				bbMax[i]=(*vIt)[i];
			}
		}
	
	/* Create a new morph box: */
	MyMorphBox::Scalar size[3];
	for(int i=0;i<3;++i)
		size[i]=bbMax[i]-bbMin[i];
	morphBox=new MyMorphBox(mesh,bbMin,size);
	}

void VRMeshEditor::deleteMorphBoxCallback(Misc::CallbackData* cbData)
	{
	if(morphBox!=0)
		{
		delete morphBox;
		morphBox=0;
		}
	}

void VRMeshEditor::saveMeshCallback(Misc::CallbackData* cbData)
	{
	savePlyMeshfile("SavedMesh.ply",*mesh);
	}

void VRMeshEditor::createInputDeviceCallback(Misc::CallbackData* cbData)
	{
	/* Create a new input device: */
	Vrui::addVirtualInputDevice("Virtual",1,0);
	}

int main(int argc,char* argv[])
	{
	try
		{
		char** appDefaults=0;
		VRMeshEditor vme(argc,argv,appDefaults);
		vme.run();
		}
	catch(std::runtime_error err)
		{
		fprintf(stderr,"Caught exception %s\n",err.what());
		return 1;
		}
	
	return 0;
	}
