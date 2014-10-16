/***********************************************************************
VruiSceneGraphDemo - Demonstration program for the Vrui scene graph
architecture; shows how to construct a scene graph programmatically, or
load one from one or more VRML 2.0 / 97 files.
Copyright (c) 2010-2013 Oliver Kreylos

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

#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <Geometry/ComponentArray.h>
#include <Geometry/Point.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GLMotif/PopupMenu.h>
#include <GLMotif/Menu.h>
#include <GLMotif/ToggleButton.h>
#include <SceneGraph/GroupNode.h>
#include <SceneGraph/TransformNode.h>
#include <SceneGraph/AppearanceNode.h>
#include <SceneGraph/MaterialNode.h>
#include <SceneGraph/BoxNode.h>
#include <SceneGraph/ShapeNode.h>
#include <SceneGraph/NodeCreator.h>
#include <SceneGraph/VRMLFile.h>
#include <Vrui/Vrui.h>
#include <Vrui/OpenFile.h>
#include <Vrui/Application.h>
#include <Vrui/SceneGraphSupport.h>

class VruiSceneGraphDemo:public Vrui::Application
	{
	/* Embedded classes: */
	private:
	typedef std::vector<SceneGraph::GroupNodePointer> SGList; // Type for lists of scene graph roots
	
	/* Elements: */
	private:
	SGList sceneGraphs; // List of root nodes of all loaded scene graphs
	std::vector<bool> sceneGraphEnableds; // List of enable flags for each loaded scene graph
	GLMotif::PopupMenu* mainMenuPopup;
	
	/* Private methods: */
	void sceneGraphToggleCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData,const unsigned int& index);
	
	/* Constructors and destructors: */
	public:
	VruiSceneGraphDemo(int& argc,char**& argv);
	~VruiSceneGraphDemo(void);
	
	/* Methods: */
	virtual void display(GLContextData& contextData) const;
	};

/***********************************
Methods of class VruiSceneGraphDemo:
***********************************/

void VruiSceneGraphDemo::sceneGraphToggleCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData,const unsigned int& index)
	{
	/* Enable or disable the selected scene graph: */
	sceneGraphEnableds[index]=cbData->set;
	}

VruiSceneGraphDemo::VruiSceneGraphDemo(int& argc,char**& argv)
	:Vrui::Application(argc,argv),
	 mainMenuPopup(0)
	{
	/* Collect a list of scene graph names to build a menu later: */
	std::vector<std::string> sceneGraphNames;
	
	if(argc>1)
		{
		/*************************************************
		Load scene graphs from one or more VRML 2.0 files:
		*************************************************/
		
		/* Create a node creator to parse the VRML files: */
		SceneGraph::NodeCreator nodeCreator;
		
		/* Load all VRML files from the command line: */
		for(int i=1;i<argc;++i)
			{
			try
				{
				/* Create the new scene graph's root node: */
				SceneGraph::GroupNodePointer root=new SceneGraph::GroupNode;
				
				/* Load and parse the VRML file: */
				SceneGraph::VRMLFile vrmlFile(argv[i],Vrui::openFile(argv[i]),nodeCreator,Vrui::getClusterMultiplexer());
				vrmlFile.parse(root);
				
				/* Add the new scene graph to the list: */
				sceneGraphs.push_back(root);
				sceneGraphEnableds.push_back(true);
				}
			catch(std::runtime_error err)
				{
				/* Print an error message and try the next file: */
				std::cerr<<"Ignoring input file "<<argv[i]<<" due to exception "<<err.what()<<std::endl;
				}
			
			/* Remember the name of this scene graph: */
			const char* start=argv[i];
			const char* end=0;
			for(const char* cPtr=argv[i];*cPtr!='\0';++cPtr)
				{
				if(*cPtr=='/')
					{
					start=cPtr+1;
					end=0;
					}
				if(*cPtr=='.')
					end=cPtr;
				}
			if(end!=0)
				sceneGraphNames.push_back(std::string(start,end));
			else
				sceneGraphNames.push_back(std::string(start));
			}
		}
	else
		{
		/* Create a small scene graph: */
		SceneGraph::TransformNode* transform=new SceneGraph::TransformNode;
		SceneGraph::GroupNodePointer root=transform;
		
		SceneGraph::ShapeNode* shape=new SceneGraph::ShapeNode;
		root->children.appendValue(shape);
		
		SceneGraph::AppearanceNode* appearance=new SceneGraph::AppearanceNode;
		shape->appearance.setValue(appearance);
		
		SceneGraph::MaterialNode* material=new SceneGraph::MaterialNode;
		appearance->material.setValue(material);
		material->ambientIntensity.setValue(1.0f);
		material->diffuseColor.setValue(SceneGraph::Color(1.0f,0.0f,0.0f));
		material->specularColor.setValue(SceneGraph::Color(1.0f,1.0f,1.0f));
		material->shininess.setValue(0.2f);
		material->update();
		
		appearance->update();
		
		SceneGraph::BoxNode* box=new SceneGraph::BoxNode;
		shape->geometry.setValue(box);
		box->size.setValue(SceneGraph::Size(2,2,2));
		box->update();
		
		shape->update();
		
		root->update();
		
		sceneGraphs.push_back(root);
		sceneGraphEnableds.push_back(true);
		sceneGraphNames.push_back("Box");
		}
	
	/* Create a popup shell to hold the main menu: */
	mainMenuPopup=new GLMotif::PopupMenu("MainMenuPopup",Vrui::getWidgetManager());
	mainMenuPopup->setTitle("Scene Graph Viewer");
	
	/* Create the main menu itself: */
	GLMotif::Menu* mainMenu=new GLMotif::Menu("MainMenu",mainMenuPopup,false);
	
	/* Add a toggle button for each scene graph: */
	unsigned int numSceneGraphs=sceneGraphs.size();
	for(unsigned int i=0;i<numSceneGraphs;++i)
		{
		char toggleName[40];
		snprintf(toggleName,sizeof(toggleName),"SceneGraphToggle%u",i);
		GLMotif::ToggleButton* sceneGraphToggle=new GLMotif::ToggleButton(toggleName,mainMenu,sceneGraphNames[i].c_str());
		sceneGraphToggle->setToggle(sceneGraphEnableds[i]);
		sceneGraphToggle->getValueChangedCallbacks().add(this,&VruiSceneGraphDemo::sceneGraphToggleCallback,i);
		}
	
	mainMenu->manageChild();
	Vrui::setMainMenu(mainMenuPopup);
	
	/* Set the navigation transformation: */
	SceneGraph::Box bbox=SceneGraph::Box::empty;
	for(unsigned int i=0;i<numSceneGraphs;++i)
		bbox.addBox(sceneGraphs[i]->calcBoundingBox());
	Vrui::setNavigationTransformation(Geometry::mid(bbox.min,bbox.max),Geometry::dist(bbox.min,bbox.max));
	}

VruiSceneGraphDemo::~VruiSceneGraphDemo(void)
	{
	delete mainMenuPopup;
	}

void VruiSceneGraphDemo::display(GLContextData& contextData) const
	{
	/* Save OpenGL state: */
	glPushAttrib(GL_ENABLE_BIT|GL_LIGHTING_BIT|GL_TEXTURE_BIT);
	
	/* Render all scene graphs: */
	unsigned int numSceneGraphs=sceneGraphs.size();
	for(unsigned int i=0;i<numSceneGraphs;++i)
		if(sceneGraphEnableds[i])
			Vrui::renderSceneGraph(sceneGraphs[i].getPointer(),true,contextData);
	
	/* Restore OpenGL state: */
	glPopAttrib();
	}

VRUI_APPLICATION_RUN(VruiSceneGraphDemo)
