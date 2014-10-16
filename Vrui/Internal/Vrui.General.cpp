/***********************************************************************
Environment-independent part of Vrui virtual reality development
toolkit.
Copyright (c) 2000-2014 Oliver Kreylos

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

#define EVILHACK_LOCK_INPUTDEVICE_POS 1

#define DELAY_NAVIGATIONTRANSFORMATION 0
#define RENDERFRAMETIMES 0
#define SAVESHAREDVRUISTATE 0

#include <Vrui/Internal/Vrui.h>

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <iostream>
#include <Misc/SelfDestructPointer.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/StringPrintf.h>
#include <Misc/FileNameExtensions.h>
#include <Misc/CreateNumberedFileName.h>
#include <Misc/ValueCoder.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/CompoundValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Misc/Time.h>
#include <Misc/TimerEventScheduler.h>
#include <IO/File.h>
#include <IO/OpenFile.h>
#include <Cluster/Multiplexer.h>
#include <Cluster/MulticastPipe.h>
#include <Math/Constants.h>
#include <Geometry/GeometryValueCoders.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLLightModelTemplates.h>
#include <GL/GLLightTracker.h>
#include <GL/GLClipPlaneTracker.h>
#include <GL/GLContextData.h>
#include <GL/GLFont.h>
#include <GL/GLValueCoders.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <GLMotif/Event.h>
#include <GLMotif/Widget.h>
#include <GLMotif/Popup.h>
#include <GLMotif/PopupMenu.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/Container.h>
#include <GLMotif/Margin.h>
#include <GLMotif/Menu.h>
#include <GLMotif/Separator.h>
#include <GLMotif/Button.h>
#include <GLMotif/CascadeButton.h>
#include <AL/Config.h>
#include <AL/ALContextData.h>
#include <Vrui/TransparentObject.h>
#include <Vrui/VirtualInputDevice.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/Internal/InputDeviceAdapter.h>
#include <Vrui/Internal/InputDeviceAdapterMouse.h>
#include <Vrui/Internal/InputDeviceAdapterDeviceDaemon.h>
#include <Vrui/Internal/MultipipeDispatcher.h>
#include <Vrui/TextEventDispatcher.h>
#include <Vrui/LightsourceManager.h>
#include <Vrui/ClipPlaneManager.h>
#include <Vrui/Viewer.h>
#include <Vrui/VRScreen.h>
#include <Vrui/WindowProperties.h>
#include <Vrui/Listener.h>
#include <Vrui/MutexMenu.h>
#include <Vrui/CoordinateManager.h>
#include <Vrui/GUIInteractor.h>
#include <Vrui/Tool.h>
#include <Vrui/ToolManager.h>
#include <Vrui/Internal/ToolKillZone.h>
#include <Vrui/VisletManager.h>
#include <Vrui/Internal/InputDeviceDataSaver.h>
#include <Vrui/Internal/ScaleBar.h>
#include <Vrui/OpenFile.h>
#include <Vrui/Internal/Config.h>

#if EVILHACK_LOCK_INPUTDEVICE_POS

Vrui::InputDevice* lockedDevice=0;
Vrui::Vector lockedTranslation;

#endif

namespace Misc {

/***********************************************************************
Helper class to read screen protection values from a configuration file:
***********************************************************************/

template <>
class ValueCoder<Vrui::VruiState::ScreenProtector>
	{
	/* Methods: */
	public:
	static std::string encode(const Vrui::VruiState::ScreenProtector& value)
		{
		std::string result;
		result.push_back('(');
		result.append(ValueCoder<std::string>::encode(value.inputDevice->getDeviceName()));
		result.push_back(',');
		result.push_back(' ');
		result.append(ValueCoder<Vrui::Point>::encode(value.center));
		result.push_back(',');
		result.push_back(' ');
		result.append(ValueCoder<Vrui::Scalar>::encode(value.radius));
		result.push_back(')');
		return result;
		}
	static Vrui::VruiState::ScreenProtector decode(const char* start,const char* end,const char** decodeEnd =0)
		{
		try
			{
			Vrui::VruiState::ScreenProtector result;
			
			/* Check for opening parenthesis: */
			const char* cPtr=start;
			if(cPtr==end||*cPtr!='(')
				throw DecodingError("missing opening parenthesis");
			++cPtr;
			cPtr=skipWhitespace(cPtr,end);
			
			/* Read input device name: */
			std::string inputDeviceName=ValueCoder<std::string>::decode(cPtr,end,&cPtr);
			cPtr=skipWhitespace(cPtr,end);
			result.inputDevice=Vrui::findInputDevice(inputDeviceName.c_str());
			if(result.inputDevice==0)
				Misc::throwStdErr("unknown input device \"%s\"",inputDeviceName.c_str());
			
			cPtr=checkSeparator(',',cPtr,end);
			
			result.center=ValueCoder<Vrui::Point>::decode(cPtr,end,&cPtr);
			cPtr=skipWhitespace(cPtr,end);
			
			cPtr=checkSeparator(',',cPtr,end);
			
			result.radius=ValueCoder<Vrui::Scalar>::decode(cPtr,end,&cPtr);
			cPtr=skipWhitespace(cPtr,end);
			
			if(cPtr==end||*cPtr!=')')
				throw DecodingError("missing closing parenthesis");
			++cPtr;
			
			if(decodeEnd!=0)
				*decodeEnd=cPtr;
			return result;
			}
		catch(std::runtime_error err)
			{
			throw DecodingError(std::string("Unable to convert \"")+std::string(start,end)+std::string("\" to ScreenProtector due to ")+err.what());
			}
		}
	};

}

namespace Vrui {

/************
Global state:
************/

VruiState* vruiState=0;
const char* vruiViewpointFileHeader="Vrui viewpoint file v1.0\n";

#if RENDERFRAMETIMES
const int numFrameTimes=800;
double frameTimes[numFrameTimes];
int frameTimeIndex=-1;
#endif
#if SAVESHAREDVRUISTATE
IO::File* vruiSharedStateFile=0;
#endif

/**********************
Private Vrui functions:
**********************/

GLMotif::Popup* VruiState::buildDialogsMenu(void)
	{
	GLMotif::WidgetManager* wm=getWidgetManager();
	
	GLMotif::Popup* dialogsMenuPopup=new GLMotif::Popup("DialogsMenuPopup",wm);
	
	dialogsMenu=new GLMotif::SubMenu("Dialogs",dialogsMenuPopup,false);
	
	/* Add menu buttons for all popped-up dialog boxes: */
	poppedDialogs.clear();
	for(GLMotif::WidgetManager::PoppedWidgetIterator wIt=wm->beginPrimaryWidgets();wIt!=wm->endPrimaryWidgets();++wIt)
		{
		GLMotif::PopupWindow* dialog=dynamic_cast<GLMotif::PopupWindow*>(*wIt);
		if(dialog!=0)
			{
			/* Add an entry to the dialogs submenu: */
			dialogsMenu->addEntry(dialog->getTitleString());
			
			/* Save a pointer to the dialog window: */
			poppedDialogs.push_back(dialog);
			}
		}
	
	dialogsMenu->getEntrySelectCallbacks().add(this,&VruiState::dialogsMenuCallback);
	
	dialogsMenu->manageChild();
	
	return dialogsMenuPopup;
	}

GLMotif::Popup* VruiState::buildViewMenu(void)
	{
	GLMotif::Popup* viewMenuPopup=new GLMotif::Popup("ViewMenuPopup",getWidgetManager());
	
	GLMotif::SubMenu* viewMenu=new GLMotif::SubMenu("View",viewMenuPopup,false);
	
	GLMotif::Button* pushViewButton=new GLMotif::Button("PushViewButton",viewMenu,"Push View");
	pushViewButton->getSelectCallbacks().add(this,&VruiState::pushViewCallback);
	
	GLMotif::Button* popViewButton=new GLMotif::Button("PushViewButton",viewMenu,"Pop View");
	popViewButton->getSelectCallbacks().add(this,&VruiState::popViewCallback);
	
	new GLMotif::Separator("Separator1",viewMenu,GLMotif::Separator::HORIZONTAL,0.0f,GLMotif::Separator::LOWERED);
	
	GLMotif::Button* loadViewButton=new GLMotif::Button("LoadViewButton",viewMenu,"Load View...");
	viewSelectionHelper.addLoadCallback(loadViewButton,this,&VruiState::loadViewCallback);
	
	GLMotif::Button* saveViewButton=new GLMotif::Button("LoadViewButton",viewMenu,"Save View...");
	viewSelectionHelper.addSaveCallback(saveViewButton,this,&VruiState::saveViewCallback);
	
	viewMenu->manageChild();
	
	return viewMenuPopup;
	}

GLMotif::Popup* VruiState::buildDevicesMenu(void)
	{
	GLMotif::Popup* devicesMenuPopup=new GLMotif::Popup("DevicesMenuPopup",getWidgetManager());
	
	GLMotif::SubMenu* devicesMenu=new GLMotif::SubMenu("Devices",devicesMenuPopup,false);
	
	/* Create buttons to create or destroy virtual input device: */
	GLMotif::Button* createOneButtonDeviceButton=new GLMotif::Button("CreateOneButtonDeviceButton",devicesMenu,"Create One-Button Device");
	createOneButtonDeviceButton->getSelectCallbacks().add(this,&VruiState::createInputDeviceCallback,1);

	GLMotif::Button* createTwoButtonDeviceButton=new GLMotif::Button("CreateTwoButtonDeviceButton",devicesMenu,"Create Two-Button Device");
	createTwoButtonDeviceButton->getSelectCallbacks().add(this,&VruiState::createInputDeviceCallback,2);
	
	new GLMotif::Separator("Separator1",devicesMenu,GLMotif::Separator::HORIZONTAL,0.0f,GLMotif::Separator::LOWERED);
	
	GLMotif::Button* destroyDeviceButton=new GLMotif::Button("DestroyDeviceButton",devicesMenu,"Destroy Oldest Device");
	destroyDeviceButton->getSelectCallbacks().add(this,&VruiState::destroyInputDeviceCallback);
	
	new GLMotif::Separator("Separator2",devicesMenu,GLMotif::Separator::HORIZONTAL,0.0f,GLMotif::Separator::LOWERED);
	
	GLMotif::Button* loadInputGraphButton=new GLMotif::Button("LoadInputGraphButton",devicesMenu,"Load Input Graph...");
	inputGraphSelectionHelper.addLoadCallback(loadInputGraphButton,this,&VruiState::loadInputGraphCallback);
	
	GLMotif::Button* saveInputGraphButton=new GLMotif::Button("SaveInputGraphButton",devicesMenu,"Save Input Graph...");
	inputGraphSelectionHelper.addSaveCallback(saveInputGraphButton,this,&VruiState::saveInputGraphCallback);
	
	devicesMenu->manageChild();
	
	return devicesMenuPopup;
	}

void VruiState::buildSystemMenu(GLMotif::Container* parent)
	{
	/* Create the dialogs submenu: */
	dialogsMenuCascade=new GLMotif::CascadeButton("DialogsMenuCascade",parent,"Dialogs");
	dialogsMenuCascade->setPopup(buildDialogsMenu());
	if(dialogsMenu->getNumRows()==0)
		dialogsMenuCascade->setEnabled(false);
	
	/* Create the view submenu: */
	GLMotif::CascadeButton* viewMenuCascade=new GLMotif::CascadeButton("ViewMenuCascade",parent,"View");
	viewMenuCascade->setPopup(buildViewMenu());
	
	/* Create the devices submenu: */
	GLMotif::CascadeButton* devicesMenuCascade=new GLMotif::CascadeButton("DevicesMenuCascade",parent,"Devices");
	devicesMenuCascade->setPopup(buildDevicesMenu());
	
	/* Create a button to show the scale bar: */
	GLMotif::ToggleButton* showScaleBarToggle=new GLMotif::ToggleButton("ShowScaleBarToggle",parent,"Show Scale Bar");
	showScaleBarToggle->getValueChangedCallbacks().add(this,&VruiState::showScaleBarToggleCallback);
	
	if(visletManager->getNumVislets()>0)
		{
		/* Create the vislet submenu: */
		GLMotif::CascadeButton* visletMenuCascade=new GLMotif::CascadeButton("VisletMenuCascade",parent,"Vislets");
		visletMenuCascade->setPopup(visletManager->buildVisletMenu());
		}
	
	new GLMotif::Separator("QuitSeparator",parent,GLMotif::Separator::HORIZONTAL,0.0f,GLMotif::Separator::LOWERED);
	
	/* Create a button to quit the current application: */
	GLMotif::Button* quitButton=new GLMotif::Button("QuitButton",parent,"Quit Program");
	quitButton->getSelectCallbacks().add(this,&VruiState::quitCallback);
	}

void VruiState::updateNavigationTransformation(const NavTransform& newTransform)
	{
	/* Calculate the new inverse transformation: */
	NavTransform newInverseTransform=Geometry::invert(newTransform);
	
	/* Call all navigation changed callbacks: */
	NavigationTransformationChangedCallbackData cbData(navigationTransformation,inverseNavigationTransformation,newTransform,newInverseTransform);
	navigationTransformationChangedCallbacks.call(&cbData);
	
	/* Set the navigation transformation: */
	navigationTransformation=newTransform;
	inverseNavigationTransformation=newInverseTransform;
	}

void VruiState::loadViewpointFile(IO::Directory& directory,const char* viewpointFileName)
	{
	/* Open the viewpoint file: */
	IO::FilePtr viewpointFile=directory.openFile(viewpointFileName);
	viewpointFile->setEndianness(Misc::LittleEndian);
	
	/* Check the header: */
	char header[80];
	viewpointFile->read(header,strlen(vruiViewpointFileHeader));
	header[strlen(vruiViewpointFileHeader)]='\0';
	if(strcmp(header,vruiViewpointFileHeader)==0)
		{
		/* Read the environment's center point in navigational coordinates: */
		Point center;
		viewpointFile->read<Scalar>(center.getComponents(),3);
		
		/* Read the environment's size in navigational coordinates: */
		Scalar size=viewpointFile->read<Scalar>();
		
		/* Read the environment's forward direction in navigational coordinates: */
		Vector forward;
		viewpointFile->read<Scalar>(forward.getComponents(),3);
		
		/* Read the environment's up direction in navigational coordinates: */
		Vector up;
		viewpointFile->read<Scalar>(up.getComponents(),3);
		
		/* Construct the navigation transformation: */
		NavTransform nav=NavTransform::identity;
		nav*=NavTransform::translateFromOriginTo(getDisplayCenter());
		nav*=NavTransform::rotate(Rotation::fromBaseVectors(getForwardDirection()^getUpDirection(),getForwardDirection()));
		nav*=NavTransform::scale(getDisplaySize()/size);
		nav*=NavTransform::rotate(Geometry::invert(Rotation::fromBaseVectors(forward^up,forward)));
		nav*=NavTransform::translateToOriginFrom(center);
		setNavigationTransformation(nav);
		}
	}

void VruiState::toolDestructionCallback(ToolManager::ToolDestructionCallbackData* cbData)
	{
	/* Check if the to-be-destroyed tool has a GUI interactor: */
	GUIInteractor* interactor=dynamic_cast<GUIInteractor*>(cbData->tool);
	
	/* Check if it is the most recent one: */
	if(interactor!=0&&interactor==mostRecentGUIInteractor)
		{
		/* Remember the interactor's final hot spot: */
		mostRecentHotSpot=interactor->calcHotSpot();
		
		/* Forget the interactor: */
		mostRecentGUIInteractor=0;
		}
	}

VruiState::VruiState(Cluster::Multiplexer* sMultiplexer,Cluster::MulticastPipe* sPipe)
	:multiplexer(sMultiplexer),
	 master(multiplexer==0||multiplexer->isMaster()),
	 pipe(sPipe),
	 randomSeed(0),
	 inchScale(1.0),
	 meterScale(1000.0/25.4),
	 displayCenter(0.0,0.0,0.0),displaySize(1.0),
	 forwardDirection(0.0,1.0,0.0),
	 upDirection(0.0,0.0,1.0),
	 floorPlane(Vector(0.0,0.0,1.0),0.0),
	 glyphRenderer(0),
	 newInputDevicePosition(0.0,0.0,0.0),
	 virtualInputDevice(0),
	 inputGraphManager(0),
	 inputGraphSelectionHelper("SavedInputGraph.inputgraph",".inputgraph",0),
	 loadInputGraph(false),
	 textEventDispatcher(0),
	 inputDeviceManager(0),
	 inputDeviceDataSaver(0),
	 multipipeDispatcher(0),
	 lightsourceManager(0),
	 clipPlaneManager(0),
	 numViewers(0),viewers(0),mainViewer(0),
	 numScreens(0),screens(0),mainScreen(0),
	 numProtectors(0),protectors(0),
	 numListeners(0),listeners(0),mainListener(0),
	 frontplaneDist(1.0),
	 backplaneDist(1000.0),
	 backgroundColor(Color(0.0f,0.0f,0.0f,1.0f)),
	 ambientLightColor(Color(0.2f,0.2f,0.2f)),
	 useSound(false),
	 widgetMaterial(GLMaterial::Color(1.0f,1.0f,1.0f),GLMaterial::Color(0.5f,0.5f,0.5f),25.0f),
	 timerEventScheduler(0),
	 widgetManager(0),
	 popWidgetsOnScreen(false),
	 dialogsMenu(0),
	 systemMenuPopup(0),dialogsMenuCascade(0),
	 mainMenu(0),
	 viewSelectionHelper("SavedViewpoint.view",".view",0),
	 navigationTransformationEnabled(false),
	 delayNavigationTransformation(false),
	 navigationTransformationChangedMask(0x0),
	 navigationTransformation(NavTransform::identity),inverseNavigationTransformation(NavTransform::identity),
	 coordinateManager(0),scaleBar(0),
	 toolManager(0),
	 visletManager(0),
	 frameFunction(0),frameFunctionData(0),
	 displayFunction(0),displayFunctionData(0),
	 soundFunction(0),soundFunctionData(0),
	 minimumFrameTime(0.0),nextFrameTime(0.0),
	 synchFrameTime(0.0),synchWait(false),
	 numRecentFrameTimes(0),recentFrameTimes(0),nextFrameTimeIndex(0),sortedFrameTimes(0),
	 activeNavigationTool(0),
	 mostRecentGUIInteractor(0),mostRecentHotSpot(displayCenter),
	 updateContinuously(false)
	{
	#if SAVESHAREDVRUISTATE
	vruiSharedStateFile=IO::openFile("/tmp/VruiSharedState.dat",IO::File::WriteOnly);
	vruiSharedStateFile->setEndianness(IO::File::LittleEndian);
	#endif
	}

VruiState::~VruiState(void)
	{
	#if SAVESHAREDVRUISTATE
	delete vruiSharedStateFile;
	#endif
	
	/* Delete time management: */
	delete[] recentFrameTimes;
	delete[] sortedFrameTimes;
	
	/* Deregister the popup callback: */
	widgetManager->getWidgetPopCallbacks().remove(this,&VruiState::widgetPopCallback);
	
	/* Destroy the input graph: */
	inputGraphManager->clear();
	
	/* Delete tool management: */
	delete toolManager;
	
	/* Delete vislet management: */
	delete visletManager;
	
	/* Delete coordinate manager: */
	delete scaleBar;
	delete coordinateManager;
	
	/* Delete widget management: */
	delete systemMenuPopup;
	delete mainMenu;
	viewSelectionHelper.closeDialogs();
	inputGraphSelectionHelper.closeDialogs();
	delete uiStyleSheet.font;
	delete widgetManager;
	delete timerEventScheduler;
	
	/* Delete listeners: */
	delete[] listeners;
	
	/* Delete screen protection management: */
	delete[] protectors;
	
	/* Delete screen management: */
	delete[] screens;
	
	/* Delete viewer management: */
	delete[] viewers;
	
	/* Delete clipping plane management: */
	delete clipPlaneManager;
	
	/* Delete light source management: */
	delete lightsourceManager;
	
	/* Delete input device management: */
	delete multipipeDispatcher;
	delete inputDeviceDataSaver;
	delete inputDeviceManager;
	delete textEventDispatcher;
	
	/* Delete input graph management: */
	delete inputGraphManager;
	delete virtualInputDevice;
	
	/* Delete glyph management: */
	delete glyphRenderer;
	}

void VruiState::initialize(const Misc::ConfigurationFileSection& configFileSection)
	{
	typedef std::vector<std::string> StringList;
	
	if(multiplexer!=0)
		{
		/* Set the multiplexer's timeout values: */
		multiplexer->setConnectionWaitTimeout(configFileSection.retrieveValue<double>("./multipipeConnectionWaitTimeout",0.1));
		multiplexer->setPingTimeout(configFileSection.retrieveValue<double>("./multipipePingTimeout",10.0),configFileSection.retrieveValue<int>("./multipipePingRetries",3));
		multiplexer->setReceiveWaitTimeout(configFileSection.retrieveValue<double>("./multipipeReceiveWaitTimeout",0.01));
		multiplexer->setBarrierWaitTimeout(configFileSection.retrieveValue<double>("./multipipeBarrierWaitTimeout",0.01));
		}
	
	/* Initialize random number management: */
	if(master)
		randomSeed=(unsigned int)time(0);
	
	/* Read the conversion factors from Vrui physical coordinate units to inches and meters: */
	inchScale=configFileSection.retrieveValue<Scalar>("./inchScale",inchScale);
	Scalar readMeterScale=configFileSection.retrieveValue<Scalar>("./meterScale",Scalar(0));
	if(readMeterScale>Scalar(0))
		{
		/* Update meter scale, and calculate inch scale: */
		meterScale=readMeterScale;
		inchScale=meterScale*0.0254;
		}
	else
		{
		/* Calculate meter scale: */
		meterScale=inchScale/0.0254;
		}
	
	/* Initialize environment dimensions: */
	displayCenter=configFileSection.retrieveValue<Point>("./displayCenter");
	displaySize=configFileSection.retrieveValue<Scalar>("./displaySize");
	forwardDirection=configFileSection.retrieveValue<Vector>("./forwardDirection",forwardDirection);
	forwardDirection.normalize();
	upDirection=configFileSection.retrieveValue<Vector>("./upDirection",upDirection);
	upDirection.normalize();
	floorPlane=configFileSection.retrieveValue<Plane>("./floorPlane",floorPlane);
	floorPlane.normalize();
	
	/* Initialize rendering parameters: */
	frontplaneDist=configFileSection.retrieveValue<Scalar>("./frontplaneDist",frontplaneDist);
	backplaneDist=configFileSection.retrieveValue<Scalar>("./backplaneDist",backplaneDist);
	backgroundColor=configFileSection.retrieveValue<Color>("./backgroundColor",backgroundColor);
	ambientLightColor=configFileSection.retrieveValue<Color>("./ambientLightColor",ambientLightColor);
	widgetMaterial=configFileSection.retrieveValue<GLMaterial>("./widgetMaterial",widgetMaterial);
	
	/* Create Vrui's default widget style sheet: */
	GLFont* font=loadFont(configFileSection.retrieveString("./uiFontName","CenturySchoolbookBoldItalic").c_str());
	font->setTextHeight(configFileSection.retrieveValue<double>("./uiFontTextHeight",1.0*inchScale));
	font->setAntialiasing(configFileSection.retrieveValue<bool>("./uiFontAntialiasing",true));
	uiStyleSheet.setFont(font);
	uiStyleSheet.setSize(configFileSection.retrieveValue<float>("./uiSize",uiStyleSheet.size));
	uiStyleSheet.borderColor=uiStyleSheet.bgColor=configFileSection.retrieveValue<Color>("./uiBgColor",uiStyleSheet.bgColor);
	uiStyleSheet.fgColor=configFileSection.retrieveValue<Color>("./uiFgColor",uiStyleSheet.fgColor);
	uiStyleSheet.textfieldBgColor=configFileSection.retrieveValue<Color>("./uiTextFieldBgColor",uiStyleSheet.textfieldBgColor);
	uiStyleSheet.textfieldFgColor=configFileSection.retrieveValue<Color>("./uiTextFieldFgColor",uiStyleSheet.textfieldFgColor);
	uiStyleSheet.selectionBgColor=configFileSection.retrieveValue<Color>("./uiSelectionBgColor",uiStyleSheet.selectionBgColor);
	uiStyleSheet.selectionFgColor=configFileSection.retrieveValue<Color>("./uiSelectionFgColor",uiStyleSheet.selectionFgColor);
	uiStyleSheet.titlebarBgColor=configFileSection.retrieveValue<Color>("./uiTitleBarBgColor",uiStyleSheet.titlebarBgColor);
	uiStyleSheet.titlebarFgColor=configFileSection.retrieveValue<Color>("./uiTitleBarFgColor",uiStyleSheet.titlebarFgColor);
	uiStyleSheet.sliderHandleWidth=configFileSection.retrieveValue<double>("./uiSliderWidth",uiStyleSheet.sliderHandleWidth);
	uiStyleSheet.sliderHandleColor=configFileSection.retrieveValue<Color>("./uiSliderHandleColor",uiStyleSheet.sliderHandleColor);
	uiStyleSheet.sliderShaftColor=configFileSection.retrieveValue<Color>("./uiSliderShaftColor",uiStyleSheet.sliderShaftColor);
	
	/* Initialize the glyph renderer: */
	GLfloat glyphRendererGlyphSize=configFileSection.retrieveValue<GLfloat>("./glyphSize",GLfloat(inchScale));
	std::string glyphRendererCursorImageFileName=VRUI_INTERNAL_CONFIG_SHAREDIR;
	glyphRendererCursorImageFileName.append("/Textures/Cursor.Xcur");
	glyphRendererCursorImageFileName=configFileSection.retrieveString("./glyphCursorFileName",glyphRendererCursorImageFileName.c_str());
	unsigned int glyphRendererCursorNominalSize=configFileSection.retrieveValue<unsigned int>("./glyphCursorNominalSize",24);
	glyphRenderer=new GlyphRenderer(glyphRendererGlyphSize,glyphRendererCursorImageFileName,glyphRendererCursorNominalSize);
	
	/* Initialize input graph manager: */
	newInputDevicePosition=configFileSection.retrieveValue<Point>("./newInputDevicePosition",displayCenter);
	virtualInputDevice=new VirtualInputDevice(glyphRenderer,configFileSection);
	inputGraphManager=new InputGraphManager(glyphRenderer,virtualInputDevice);
	
	/* Create a text event dispatcher to manage GLMotif text and text control events in a cluster-transparent manner: */
	textEventDispatcher=new TextEventDispatcher(master);
	
	/* Initialize input device manager: */
	inputDeviceManager=new InputDeviceManager(inputGraphManager,textEventDispatcher);
	if(master)
		inputDeviceManager->initialize(configFileSection);
	
	/* If in cluster mode, create a dispatcher to send input device states to the slaves: */
	if(multiplexer!=0)
		{
		multipipeDispatcher=new MultipipeDispatcher(inputDeviceManager,pipe);
		if(!master)
			{
			/* On slaves, multipipe dispatcher is owned by input device manager: */
			multipipeDispatcher=0;
			}
		}
	
	if(master)
		{
		/* Check if the user wants to save input device data: */
		std::string iddsSectionName=configFileSection.retrieveString("./inputDeviceDataSaver","");
		if(!iddsSectionName.empty())
			{
			/* Go to input device data saver's section: */
			Misc::ConfigurationFileSection iddsSection=configFileSection.getSection(iddsSectionName.c_str());
			
			/* Initialize the input device data saver: */
			inputDeviceDataSaver=new InputDeviceDataSaver(iddsSection,*inputDeviceManager,textEventDispatcher,randomSeed);
			}
		}
	
	/* Update all physical input devices to get initial positions and orientations: */
	if(master)
		{
		inputDeviceManager->updateInputDevices();
		
		#if EVILHACK_LOCK_INPUTDEVICE_POS
		if(lockedDevice!=0)
			lockedDevice->setTransformation(Vrui::TrackerState(lockedTranslation,lockedDevice->getOrientation()));
		#endif
		
		if(multiplexer!=0)
			{
			multipipeDispatcher->updateInputDevices();
			textEventDispatcher->writeEventQueues(*pipe);
			pipe->flush();
			}
		}
	else
		{
		inputDeviceManager->updateInputDevices();
		textEventDispatcher->readEventQueues(*pipe);
		}
	
	/* Save input device states to data file if requested: */
	if(inputDeviceDataSaver!=0)
		inputDeviceDataSaver->saveCurrentState(0.0);
	
	/* Initialize the update regime: */
	if(master)
		updateContinuously=configFileSection.retrieveValue<bool>("./updateContinuously",updateContinuously);
	else
		updateContinuously=true; // Slave nodes always run in continuous mode; they will block on updates from the master
	
	/* Initialize the light source manager: */
	lightsourceManager=new LightsourceManager;
	
	/* Initialize the clipping plane manager: */
	clipPlaneManager=new ClipPlaneManager;
	
	/* Initialize the viewers: */
	StringList viewerNames=configFileSection.retrieveValue<StringList>("./viewerNames");
	numViewers=viewerNames.size();
	viewers=new Viewer[numViewers];
	for(int i=0;i<numViewers;++i)
		{
		/* Go to viewer's section: */
		Misc::ConfigurationFileSection viewerSection=configFileSection.getSection(viewerNames[i].c_str());
		
		/* Initialize viewer: */
		viewers[i].initialize(viewerSection);
		}
	mainViewer=&viewers[0];
	
	/* Initialize the screens: */
	StringList screenNames=configFileSection.retrieveValue<StringList>("./screenNames");
	numScreens=screenNames.size();
	screens=new VRScreen[numScreens];
	for(int i=0;i<numScreens;++i)
		{
		/* Go to screen's section: */
		Misc::ConfigurationFileSection screenSection=configFileSection.getSection(screenNames[i].c_str());
		
		/* Initialize screen: */
		screens[i].initialize(screenSection);
		}
	mainScreen=&screens[0];
	
	/* Initialize screen protection: */
	typedef std::vector<ScreenProtector> ScreenProtectorList;
	ScreenProtectorList spl=configFileSection.retrieveValue<ScreenProtectorList>("./screenProtectors",ScreenProtectorList());
	numProtectors=spl.size();
	protectors=new ScreenProtector[numProtectors];
	for(int i=0;i<numProtectors;++i)
		protectors[i]=spl[i];
	
	/* Initialize the listeners: */
	StringList listenerNames=configFileSection.retrieveValue<StringList>("./listenerNames",StringList());
	numListeners=listenerNames.size();
	listeners=new Listener[numListeners];
	for(int i=0;i<numListeners;++i)
		{
		/* Go to listener's section: */
		Misc::ConfigurationFileSection listenerSection=configFileSection.getSection(listenerNames[i].c_str());
		
		/* Initialize listener: */
		listeners[i].initialize(listenerSection);
		}
	mainListener=&listeners[0];
	
	/* Initialize widget management: */
	timerEventScheduler=new Misc::TimerEventScheduler;
	widgetManager=new GLMotif::WidgetManager;
	widgetManager->setStyleSheet(&uiStyleSheet);
	widgetManager->setTimerEventScheduler(timerEventScheduler);
	widgetManager->setDrawOverlayWidgets(configFileSection.retrieveValue<bool>("./drawOverlayWidgets",widgetManager->getDrawOverlayWidgets()));
	widgetManager->getWidgetPopCallbacks().add(this,&VruiState::widgetPopCallback);
	popWidgetsOnScreen=configFileSection.retrieveValue<bool>("./popWidgetsOnScreen",popWidgetsOnScreen);
	widgetPlane=ONTransform::translateFromOriginTo(displayCenter);
	widgetPlane*=ONTransform::rotate(ONTransform::Rotation::fromBaseVectors(forwardDirection^upDirection,upDirection));
	
	/* Dispatch any early text events: */
	textEventDispatcher->dispatchEvents(*widgetManager);
	
	/* Initialize the directories used to load files: */
	viewSelectionHelper.setCurrentDirectory(openDirectory("."));
	inputGraphSelectionHelper.setCurrentDirectory(openDirectory("."));
	
	/* Initialize 3D picking: */
	pointPickDistance=Scalar(uiStyleSheet.size*2.0f);
	pointPickDistance=configFileSection.retrieveValue<Scalar>("./pointPickDistance",pointPickDistance);
	Scalar rayPickAngle=Math::deg(Math::atan(pointPickDistance/mainScreen->getScreenTransformation().inverseTransform(mainViewer->getHeadPosition())[2]));
	rayPickAngle=configFileSection.retrieveValue<Scalar>("./rayPickAngle",rayPickAngle);
	if(rayPickAngle<Scalar(0))
		rayPickAngle=Scalar(0);
	if(rayPickAngle>Scalar(90))
		rayPickAngle=Scalar(90);
	rayPickCosine=Math::cos(Math::rad(rayPickAngle));
	
	/* Create the coordinate manager: */
	coordinateManager=new CoordinateManager;
	
	/* Go to tool manager's section: */
	Misc::ConfigurationFileSection toolSection=configFileSection.getSection(configFileSection.retrieveString("./tools").c_str());
	
	/* Initialize tool manager: */
	toolManager=new ToolManager(inputDeviceManager,toolSection);
	
	/* Register the tool destruction callback: */
	toolManager->getToolDestructionCallbacks().add(this,&VruiState::toolDestructionCallback);
	
	try
		{
		/* Go to vislet manager's section: */
		Misc::ConfigurationFileSection visletSection=configFileSection.getSection(configFileSection.retrieveString("./vislets").c_str());
		
		/* Initialize vislet manager: */
		visletManager=new VisletManager(visletSection);
		}
	catch(std::runtime_error err)
		{
		/* Ignore error and continue... */
		}
	
	/* Distribute the random seed and initialize the application timer: */
	lastFrame=appTime.peekTime();
	if(multiplexer!=0)
		{
		pipe->broadcast<unsigned int>(randomSeed);
		pipe->broadcast<double>(lastFrame);
		pipe->flush();
		}
	srand(randomSeed);
	lastFrameDelta=0.0;
	
	/* Check if there is a frame rate limit: */
	double maxFrameRate=configFileSection.retrieveValue<double>("./maximumFrameRate",0.0);
	if(maxFrameRate>0.0)
		{
		/* Calculate the minimum frame time: */
		minimumFrameTime=1.0/maxFrameRate;
		}
	
	/* Set the current application time in the timer event scheduler: */
	timerEventScheduler->triggerEvents(lastFrame);
	
	/* Initialize the frame time calculator: */
	numRecentFrameTimes=5;
	recentFrameTimes=new double[numRecentFrameTimes];
	for(int i=0;i<numRecentFrameTimes;++i)
		recentFrameTimes[i]=1.0;
	nextFrameTimeIndex=0;
	sortedFrameTimes=new double[numRecentFrameTimes];
	currentFrameTime=1.0;
	
	/* Initialize the hot spot position for dialog windows: */
	mostRecentHotSpot=displayCenter;
	}

void VruiState::createSystemMenu(void)
	{
	/* Create the Vrui system menu and install it as the main menu: */
	systemMenuPopup=new GLMotif::PopupMenu("VruiSystemMenuPopup",widgetManager);
	systemMenuPopup->setTitle("Vrui System");
	GLMotif::Menu* systemMenu=new GLMotif::Menu("VruiSystemMenu",systemMenuPopup,false);
	buildSystemMenu(systemMenu);
	systemMenu->manageChild();
	mainMenu=new MutexMenu(systemMenuPopup);
	}

DisplayState* VruiState::registerContext(GLContextData& contextData) const
	{
	/* Try retrieving an already existing display state mapper data item: */
	DisplayStateMapper::DataItem* dataItem=contextData.retrieveDataItem<DisplayStateMapper::DataItem>(&displayStateMapper);
	if(dataItem==0)
		{
		/* Create a new display state mapper data item: */
		dataItem=new DisplayStateMapper::DataItem;
		
		/* Associate it with the OpenGL context: */
		contextData.addDataItem(&displayStateMapper,dataItem);
		}
	
	/* Return a pointer to the display state structure: */
	return &dataItem->displayState;
	}

void VruiState::prepareMainLoop(void)
	{
	/* Create the system menu if the application didn't install one: */
	if(mainMenu==0)
		createSystemMenu();
	
	#if DELAY_NAVIGATIONTRANSFORMATION
	/* Start delaying the navigation transformation at this point: */
	delayNavigationTransformation=true;
	#endif
	
	if(loadInputGraph)
		{
		/* Load the requested input graph: */
		inputGraphManager->loadInputGraph(*inputGraphSelectionHelper.getCurrentDirectory(),inputGraphFileName.c_str(),"InputGraph");
		loadInputGraph=false;
		}
	else
		{
		/* Create default tool assignment: */
		toolManager->loadDefaultTools();
		}
	
	/* Check if the user gave a viewpoint file on the command line: */
	if(!viewpointFileName.empty())
		{
		/* Split the given name into directory and file name: */
		const char* vfn=viewpointFileName.c_str();
		const char* fileName=Misc::getFileName(vfn);
		std::string dirName(vfn,fileName);
		
		/* Override the navigation transformation: */
		try
			{
			viewSelectionHelper.setCurrentDirectory(openDirectory(dirName.c_str()));
			loadViewpointFile(*viewSelectionHelper.getCurrentDirectory(),fileName);
			}
		catch(std::runtime_error err)
			{
			/* Print a warning message and continue: */
			std::cerr<<"Unable to load viewpoint file "<<viewpointFileName<<" due to exception "<<err.what()<<std::endl;
			}
		}
	
	/* Enable all vislets: */
	visletManager->enable();
	}

void VruiState::update(void)
	{
	/*********************************************************************
	Update the application time and all related state:
	*********************************************************************/
	
	double lastLastFrame=lastFrame;
	if(master)
		{
		/* Take an application timer snapshot: */
		lastFrame=appTime.peekTime();
		if(synchFrameTime>0.0)
			{
			/* Check if the frame needs to be delayed: */
			if(synchWait&&lastFrame<synchFrameTime)
				{
				/* Sleep for a while to reach the synchronized frame time: */
				vruiDelay(synchFrameTime-lastFrame);
				}
			
			/* Override the free-running timer: */
			lastFrame=synchFrameTime;
			synchFrameTime=0.0;
			synchWait=false;
			}
		else if(minimumFrameTime>0.0)
			{
			/* Check if the time for the last frame was less than the allowed minimum: */
			if(lastFrame-lastLastFrame<minimumFrameTime)
				{
				/* Sleep for a while to reach the minimum frame time: */
				vruiDelay(minimumFrameTime-(lastFrame-lastLastFrame));
				
				/* Take another application timer snapshot: */
				lastFrame=appTime.peekTime();
				}
			}
		if(multiplexer!=0)
			pipe->write<double>(lastFrame);
		
		/* Update the Vrui application timer and the frame time history: */
		recentFrameTimes[nextFrameTimeIndex]=lastFrame-lastLastFrame;
		++nextFrameTimeIndex;
		if(nextFrameTimeIndex==numRecentFrameTimes)
			nextFrameTimeIndex=0;
		
		/* Calculate current median frame time: */
		for(int i=0;i<numRecentFrameTimes;++i)
			{
			int j;
			for(j=i-1;j>=0&&sortedFrameTimes[j]>recentFrameTimes[i];--j)
				sortedFrameTimes[j+1]=sortedFrameTimes[j];
			sortedFrameTimes[j+1]=recentFrameTimes[i];
			}
		currentFrameTime=sortedFrameTimes[numRecentFrameTimes/2];
		if(multiplexer!=0)
			pipe->write<double>(currentFrameTime);
		}
	else
		{
		/* Receive application time and current median frame time: */
		pipe->read<double>(lastFrame);
		pipe->read<double>(currentFrameTime);
		}
	
	/* Calculate the current frame time delta: */
	lastFrameDelta=lastFrame-lastLastFrame;
	
	#if RENDERFRAMETIMES
	/* Update the frame time graph: */
	++frameTimeIndex;
	if(frameTimeIndex==numFrameTimes)
		frameTimeIndex=0;
	frameTimes[frameTimeIndex]=lastFrame-lastLastFrame;
	#endif
	
	/* Reset the next scheduled frame time: */
	nextFrameTime=0.0;
	
	/*********************************************************************
	Update input device state and distribute all shared state:
	*********************************************************************/
	
	int navBroadcastMask=navigationTransformationChangedMask;
	if(master)
		{
		/* Update all physical input devices: */
		inputDeviceManager->updateInputDevices();
		
		#if EVILHACK_LOCK_INPUTDEVICE_POS
		if(lockedDevice!=0)
			lockedDevice->setTransformation(Vrui::TrackerState(lockedTranslation,lockedDevice->getOrientation()));
		#endif
		
		if(multiplexer!=0)
			{
			/* Write input device states and text events to all slaves: */
			multipipeDispatcher->updateInputDevices();
			textEventDispatcher->writeEventQueues(*pipe);
			}
		
		/* Save input device states to data file if requested: */
		if(inputDeviceDataSaver!=0)
			inputDeviceDataSaver->saveCurrentState(lastFrame);
		
		#if DELAY_NAVIGATIONTRANSFORMATION
		if(navigationTransformationEnabled&&(navigationTransformationChangedMask&0x1))
			{
			/* Update the navigation transformation: */
			updateNavigationTransformation(newNavigationTransformation);
			navigationTransformationChangedMask=0x0;
			}
		#endif
		}
	else
		{
		/* Receive input device states and text events from the master: */
		inputDeviceManager->updateInputDevices();
		textEventDispatcher->readEventQueues(*pipe);
		}
	
	if(multiplexer!=0)
		{
		/* Broadcast the current navigation transformation and/or display center/size: */
		pipe->broadcast<int>(navBroadcastMask);
		if(navBroadcastMask&0x1)
			{
			if(master)
				{
				/* Send the new navigation transformation: */
				pipe->write<Scalar>(navigationTransformation.getTranslation().getComponents(),3);
				pipe->write<Scalar>(navigationTransformation.getRotation().getQuaternion(),4);
				pipe->write<Scalar>(navigationTransformation.getScaling());
				}
			else
				{
				/* Receive the new navigation transformation: */
				Vector translation;
				pipe->read<Scalar>(translation.getComponents(),3);
				Scalar rotationQuaternion[4];
				pipe->read<Scalar>(rotationQuaternion,4);
				Scalar scaling=pipe->read<Scalar>();
				
				/* Update the navigation transformation: */
				navigationTransformationEnabled=true;
				updateNavigationTransformation(NavTransform(translation,Rotation(rotationQuaternion),scaling));
				}
			}
		if(navBroadcastMask&0x2)
			{
			/* Broadcast the new display center and size: */
			pipe->broadcast<Scalar>(displayCenter.getComponents(),3);
			pipe->broadcast<Scalar>(displaySize);
			}
		if(navBroadcastMask&0x4)
			{
			if(master)
				{
				/* Send the tool kill zone's new center: */
				pipe->write<Scalar>(toolManager->getToolKillZone()->getCenter().getComponents(),3);
				}
			else
				{
				/* Receive the tool kill zone' new center: */
				Point newCenter;
				pipe->read<Scalar>(newCenter.getComponents(),3);
				toolManager->getToolKillZone()->setCenter(newCenter);
				}
			}
		
		pipe->flush();
		}
	
	#if SAVESHAREDVRUISTATE
	/* Save shared state to a local file for post-mortem analysis purposes: */
	vruiSharedStateFile->write<double>(lastFrame);
	vruiSharedStateFile->write<double>(currentFrameTime);
	int numInputDevices=inputDeviceManager->getNumInputDevices();
	vruiSharedStateFile->write<int>(numInputDevices);
	for(int i=0;i<numInputDevices;++i)
		{
		InputDevice* id=inputDeviceManager->getInputDevice(i);
		vruiSharedStateFile->write<Scalar>(id->getPosition().getComponents(),3);
		vruiSharedStateFile->write<Scalar>(id->getOrientation().getQuaternion(),4);
		}
	#endif
	
	/*********************************************************************
	Update all managers:
	*********************************************************************/
	
	/* Set the widget manager's time: */
	widgetManager->setTime(lastFrame);
	
	/* Trigger all due timer events: */
	timerEventScheduler->triggerEvents(lastFrame);
	
	/* Dispatch all text events: */
	textEventDispatcher->dispatchEvents(*widgetManager);
	
	/* Update the input graph: */
	inputGraphManager->update();
	
	/* Update the tool manager: */
	toolManager->update();
	
	/* Check if a new input graph needs to be loaded: */
	if(loadInputGraph)
		{
		try
			{
			/* Load the input graph from the selected configuration file: */
			getInputGraphManager()->clear();
			getInputGraphManager()->loadInputGraph(*inputGraphSelectionHelper.getCurrentDirectory(),inputGraphFileName.c_str(),"InputGraph");
			}
		catch(std::runtime_error err)
			{
			/* Show an error message: */
			std::string message="Could not load input graph from file ";
			message.append(inputGraphFileName);
			message.append(" due to exception ");
			message.append(err.what());
			showErrorMessage("Load Input Graph",message.c_str());
			}
		
		loadInputGraph=false;
		}
	
	/* Update viewer states: */
	for(int i=0;i<numViewers;++i)
		viewers[i].update();
	
	/* Update listener states: */
	for(int i=0;i<numListeners;++i)
		listeners[i].update();
	
	/* Call frame functions of all loaded vislets: */
	if(visletManager!=0)
		visletManager->frame();
	
	/* Call frame function: */
	frameFunction(frameFunctionData);
	
	/* Finish any pending messages on the main pipe, in case an application didn't clean up: */
	if(multiplexer!=0)
		pipe->flush();
	}

void VruiState::display(DisplayState* displayState,GLContextData& contextData) const
	{
	/* Initialize lighting state through the display state's light tracker: */
	GLLightTracker* lt=contextData.getLightTracker();
	lt->setLightingEnabled(true);
	lt->setSpecularColorSeparate(false);
	lt->setLightingTwoSided(false);
	lt->setColorMaterials(false);
	lt->setColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
	lt->setNormalScalingMode(GLLightTracker::NormalScalingNormalize);
	
	/* Enable ambient light source: */
	glLightModelAmbient(ambientLightColor);
	
	/* Go to physical coordinates: */
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrix(displayState->modelviewPhysical);
	
	/* Set light sources: */
	lightsourceManager->setLightsources(navigationTransformationEnabled,displayState,contextData);
	
	/* Render input device manager's state: */
	inputDeviceManager->glRenderAction(contextData);
	
	/* Render input graph devices: */
	inputGraphManager->glRenderDevices(contextData);
	
	/* Display any realized widgets: */
	glMaterial(GLMaterialEnums::FRONT,widgetMaterial);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
	widgetManager->draw(contextData);
	glDisable(GL_COLOR_MATERIAL);
	
	/* Set clipping planes: */
	clipPlaneManager->setClipPlanes(navigationTransformationEnabled,displayState,contextData);
	
	/* Render tool manager's state: */
	toolManager->glRenderAction(contextData);
	
	/* Render input graph tools: */
	inputGraphManager->glRenderTools(contextData);
	
	/* Display all loaded vislets: */
	if(visletManager!=0)
		visletManager->display(contextData);
	
	/* Call the user display function: */
	if(displayFunction!=0)
		{
		if(navigationTransformationEnabled)
			{
			/* Go to navigational coordinates: */
			glLoadMatrix(displayState->modelviewNavigational);
			}
		displayFunction(contextData,displayFunctionData);
		if(navigationTransformationEnabled)
			{
			/* Go back to physical coordinates: */
			glLoadMatrix(displayState->modelviewPhysical);
			}
		}
	
	/* Execute the transparency rendering pass: */
	if(TransparentObject::needRenderPass())
		{
		/* Set up OpenGL state for transparency: */
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_FALSE);
		
		/* Execute transparent rendering pass: */
		TransparentObject::transparencyPass(contextData);
		
		/* Return to standard OpenGL state: */
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		}
	
	/* Temporarily disable all clipping planes, bypassing the OpenGL context's clipping plane tracker: */
	GLClipPlaneTracker* cpt=contextData.getClipPlaneTracker();
	for(int i=0;i<cpt->getMaxNumClipPlanes();++i)
		if(cpt->getClipPlaneState(i).isEnabled())
			glDisable(GL_CLIP_PLANE0+i);
	}

void VruiState::sound(ALContextData& contextData) const
	{
	#if ALSUPPORT_CONFIG_HAVE_OPENAL
	/* Display all loaded vislets: */
	if(visletManager!=0)
		visletManager->sound(contextData);
	
	/* Call the user sound function: */
	if(soundFunction!=0)
		{
		if(navigationTransformationEnabled)
			{
			/* Go to navigational coordinates: */
			contextData.pushMatrix();
			contextData.multMatrix(navigationTransformation);
			}
		soundFunction(contextData,soundFunctionData);
		if(navigationTransformationEnabled)
			{
			/* Go back to physical coordinates: */
			contextData.popMatrix();
			}
		}
	#endif
	}

void VruiState::finishMainLoop(void)
	{
	/* Disable all vislets: */
	visletManager->disable();
	
	/* Deregister the popup callback: */
	widgetManager->getWidgetPopCallbacks().remove(this,&VruiState::widgetPopCallback);
	}

void VruiState::dialogsMenuCallback(GLMotif::SubMenu::EntrySelectCallbackData* cbData)
	{
	/* Get the index of the selected button: */
	int index=dialogsMenu->getChildIndex(cbData->selectedButton);
	
	/* Get a pointer to the dialog window: */
	GLMotif::PopupWindow* dialog=poppedDialogs[index];
	
	GLMotif::WidgetManager* wm=getWidgetManager();
	if(wm->isVisible(dialog))
		{
		/* Initialize the pop-up position: */
		Point hotSpot=mostRecentHotSpot;
		
		/* Check if there is a most-recently used GUI interactor: */
		if(mostRecentGUIInteractor!=0)
			{
			/* Get the most recent interactor's hot spot: */
			hotSpot=mostRecentGUIInteractor->calcHotSpot();
			}
		
		/* Move the dialog window to the hot spot position: */
		ONTransform transform=calcHUDTransform(hotSpot);
		transform*=ONTransform::translate(-Vector(dialog->calcHotSpot().getXyzw()));
		wm->setPrimaryWidgetTransformation(dialog,transform);
		}
	else
		{
		/* Show the hidden dialog window at its previous position: */
		wm->show(dialog);
		}
	}

void VruiState::widgetPopCallback(GLMotif::WidgetManager::WidgetPopCallbackData* cbData)
	{
	/* Don't do anything if there is no dialogs menu yet: */
	if(dialogsMenu==0)
		return;
	
	/* Check if the widget is a dialog: */
	GLMotif::PopupWindow* dialog=dynamic_cast<GLMotif::PopupWindow*>(cbData->topLevelWidget);
	if(dialog==0)
		return;
	
	if(cbData->popup)
		{
		/* Append the newly popped-up dialog to the dialogs menu: */
		poppedDialogs.push_back(dialog);
		dialogsMenu->addEntry(dialog->getTitleString());
		
		/* Enable the dialogs menu if it has become non-empty: */
		if(dialogsMenu->getNumRows()==1)
			{
			/* Enable the "Dialogs" cascade button: */
			dialogsMenuCascade->setEnabled(true);
			}
		}
	else
		{
		/* Find the popped-down dialog in the dialogs menu: */
		int menuIndex=0;
		std::vector<GLMotif::PopupWindow*>::iterator dIt;
		for(dIt=poppedDialogs.begin();dIt!=poppedDialogs.end()&&*dIt!=dialog;++dIt,++menuIndex)
			;
		if(dIt!=poppedDialogs.end())
			{
			/* Remove the popped-down dialog from the dialogs menu: */
			poppedDialogs.erase(dIt);
			dialogsMenu->removeWidgets(menuIndex);
			
			/* Disable the dialogs menu if it has become empty: */
			if(dialogsMenu->getNumRows()==0)
				{
				/* Disable the "Dialogs" cascade button: */
				dialogsMenuCascade->setEnabled(false);
				}
			}
		}
	}

void VruiState::loadViewCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData)
	{
	/* Load the selected file only if there are no active navigation tools: */
	if(activeNavigationTool==0)
		{
		/* Load the selected viewpoint file: */
		loadViewpointFile(*cbData->selectedDirectory,cbData->selectedFileName);
		}
	}

void VruiState::saveViewCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData)
	{
	/* Write the viewpoint file: */
	IO::FilePtr viewpointFile(cbData->selectedDirectory->openFile(cbData->selectedFileName,IO::File::WriteOnly));
	viewpointFile->setEndianness(Misc::LittleEndian);
	
	/* Write a header identifying this as an environment-independent viewpoint file: */
	viewpointFile->write<char>(vruiViewpointFileHeader,strlen(vruiViewpointFileHeader));
	
	/* Write the environment's center point in navigational coordinates: */
	Point center=getInverseNavigationTransformation().transform(getDisplayCenter());
	viewpointFile->write<Scalar>(center.getComponents(),3);
	
	/* Write the environment's size in navigational coordinates: */
	Scalar size=getDisplaySize()*getInverseNavigationTransformation().getScaling();
	viewpointFile->write<Scalar>(size);
	
	/* Write the environment's forward direction in navigational coordinates: */
	Vector forward=getInverseNavigationTransformation().transform(getForwardDirection());
	viewpointFile->write<Scalar>(forward.getComponents(),3);
	
	/* Write the environment's up direction in navigational coordinates: */
	Vector up=getInverseNavigationTransformation().transform(getUpDirection());
	viewpointFile->write<Scalar>(up.getComponents(),3);
	}

void VruiState::pushViewCallback(Misc::CallbackData* cbData)
	{
	/* Push the current navigation transformation onto the stack of navigation transformations: */
	storedNavigationTransformations.push_back(getNavigationTransformation());
	}

void VruiState::popViewCallback(Misc::CallbackData* cbData)
	{
	/* Only restore if no navigation tools are active and the stack is not empty: */
	if(activeNavigationTool==0&&!storedNavigationTransformations.empty())
		{
		/* Pop the most recently stored navigation transformation off the stack: */
		setNavigationTransformation(storedNavigationTransformations.back());
		storedNavigationTransformations.pop_back();
		}
	}

void VruiState::createInputDeviceCallback(Misc::CallbackData* cbData,const int& numButtons)
	{
	/* Create a new one-button virtual input device: */
	createdVirtualInputDevices.push_back(addVirtualInputDevice("VirtualInputDevice",numButtons,0));
	}

void VruiState::destroyInputDeviceCallback(Misc::CallbackData* cbData)
	{
	/* Destroy the oldest virtual input device: */
	if(!createdVirtualInputDevices.empty())
		{
		getInputDeviceManager()->destroyInputDevice(createdVirtualInputDevices.front());
		createdVirtualInputDevices.pop_front();
		}
	}

void VruiState::loadInputGraphCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData)
	{
	/* Remember to load the given input graph file at the next opportune time: */
	loadInputGraph=true;
	inputGraphFileName=cbData->selectedFileName;
	}

void VruiState::saveInputGraphCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData)
	{
	/* Save the input graph: */
	getInputGraphManager()->saveInputGraph(*cbData->selectedDirectory,cbData->selectedFileName,"InputGraph");
	}

void VruiState::showScaleBarToggleCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	if(cbData->set)
		{
		/* Create a new scale bar: */
		scaleBar=new ScaleBar("VruiScaleBar",getWidgetManager());
		popupPrimaryWidget(scaleBar);
		}
	else
		{
		/* Destroy the scale bar: */
		delete scaleBar;
		scaleBar=0;
		}
	}

void VruiState::quitCallback(Misc::CallbackData* cbData)
	{
	/* Request Vrui to shut down cleanly: */
	shutdown();
	}

/********************************
Global Vrui kernel API functions:
********************************/

void setRandomSeed(unsigned int newRandomSeed)
	{
	vruiState->randomSeed=newRandomSeed;
	}

void vruiDelay(double interval)
	{
	#ifdef __SGI_IRIX__
	long intervalCount=(long)(interval*(double)CLK_TCK+0.5);
	while(intervalCount>0)
		intervalCount=sginap(intervalCount);
	#else
	int seconds=int(floor(interval));
	interval-=double(seconds);
	int microseconds=int(floor(interval*1000000.0+0.5));
	struct timeval tv;
	tv.tv_sec=seconds;
	tv.tv_usec=microseconds;
	select(0,0,0,0,&tv);
	#endif
	}

void synchronize(double nextFrameTime,bool wait)
	{
	vruiState->synchFrameTime=nextFrameTime;
	vruiState->synchWait=wait;
	}

void setDisplayCenter(const Point& newDisplayCenter,Scalar newDisplaySize)
	{
	/* Update the display center: */
	vruiState->displayCenter=newDisplayCenter;
	vruiState->displaySize=newDisplaySize;
	vruiState->navigationTransformationChangedMask|=0x2;
	
	/* Update the widget plane: */
	vruiState->widgetPlane=ONTransform::translateFromOriginTo(vruiState->displayCenter);
	vruiState->widgetPlane*=ONTransform::rotate(ONTransform::Rotation::fromBaseVectors(vruiState->forwardDirection^vruiState->upDirection,vruiState->upDirection));
	}

/**********************************
Call-in functions for user program:
**********************************/

void setFrameFunction(FrameFunctionType frameFunction,void* userData)
	{
	vruiState->frameFunction=frameFunction;
	vruiState->frameFunctionData=userData;
	}

void setDisplayFunction(DisplayFunctionType displayFunction,void* userData)
	{
	vruiState->displayFunction=displayFunction;
	vruiState->displayFunctionData=userData;
	}

void setSoundFunction(SoundFunctionType soundFunction,void* userData)
	{
	vruiState->soundFunction=soundFunction;
	vruiState->soundFunctionData=userData;
	}

Cluster::Multiplexer* getClusterMultiplexer(void)
	{
	return vruiState->multiplexer;
	}

bool isMaster(void)
	{
	return vruiState->master;
	}

int getNodeIndex(void)
	{
	if(vruiState->multiplexer!=0)
		return vruiState->multiplexer->getNodeIndex();
	else
		return 0;
	}

int getNumNodes(void)
	{
	if(vruiState->multiplexer!=0)
		return vruiState->multiplexer->getNumNodes();
	else
		return 1;
	}

Cluster::MulticastPipe* getMainPipe(void)
	{
	return vruiState->pipe;
	}

Cluster::MulticastPipe* openPipe(void)
	{
	if(vruiState->multiplexer!=0)
		return new Cluster::MulticastPipe(vruiState->multiplexer);
	else
		return 0;
	}

GlyphRenderer* getGlyphRenderer(void)
	{
	return vruiState->glyphRenderer;
	}

void renderGlyph(const Glyph& glyph,const OGTransform& transformation,GLContextData& contextData)
	{
	vruiState->glyphRenderer->renderGlyph(glyph,transformation,vruiState->glyphRenderer->getContextDataItem(contextData));
	}

VirtualInputDevice* getVirtualInputDevice(void)
	{
	return vruiState->virtualInputDevice;
	}

InputGraphManager* getInputGraphManager(void)
	{
	return vruiState->inputGraphManager;
	}

InputDeviceManager* getInputDeviceManager(void)
	{
	return vruiState->inputDeviceManager;
	}

int getNumInputDevices(void)
	{
	return vruiState->inputDeviceManager->getNumInputDevices();
	}

InputDevice* getInputDevice(int index)
	{
	return vruiState->inputDeviceManager->getInputDevice(index);
	}

InputDevice* findInputDevice(const char* name)
	{
	return vruiState->inputDeviceManager->findInputDevice(name);
	}

InputDevice* addVirtualInputDevice(const char* name,int numButtons,int numValuators)
	{
	InputDevice* newDevice=vruiState->inputDeviceManager->createInputDevice(name,InputDevice::TRACK_POS|InputDevice::TRACK_DIR|InputDevice::TRACK_ORIENT,numButtons,numValuators);
	newDevice->setTransformation(TrackerState::translateFromOriginTo(vruiState->newInputDevicePosition));
	vruiState->inputGraphManager->getInputDeviceGlyph(newDevice).enable(Glyph::BOX,vruiState->widgetMaterial);
	return newDevice;
	}

LightsourceManager* getLightsourceManager(void)
	{
	return vruiState->lightsourceManager;
	}

ClipPlaneManager* getClipPlaneManager(void)
	{
	return vruiState->clipPlaneManager;
	}

Viewer* getMainViewer(void)
	{
	return vruiState->mainViewer;
	}

int getNumViewers(void)
	{
	return vruiState->numViewers;
	}

Viewer* getViewer(int index)
	{
	return &vruiState->viewers[index];
	}

Viewer* findViewer(const char* name)
	{
	Viewer* result=0;
	for(int i=0;i<vruiState->numViewers;++i)
		if(strcmp(name,vruiState->viewers[i].getName())==0)
			{
			result=&vruiState->viewers[i];
			break;
			}
	return result;
	}

VRScreen* getMainScreen(void)
	{
	return vruiState->mainScreen;
	}

int getNumScreens(void)
	{
	return vruiState->numScreens;
	}

VRScreen* getScreen(int index)
	{
	return vruiState->screens+index;
	}

VRScreen* findScreen(const char* name)
	{
	VRScreen* result=0;
	for(int i=0;i<vruiState->numScreens;++i)
		if(strcmp(name,vruiState->screens[i].getName())==0)
			{
			result=&vruiState->screens[i];
			break;
			}
	return result;
	}

std::pair<VRScreen*,Scalar> findScreen(const Ray& ray)
	{
	/* Find the closest intersection with any screen: */
	VRScreen* closestScreen=0;
	Scalar closestLambda=Math::Constants<Scalar>::max;
	for(int screenIndex=0;screenIndex<vruiState->numScreens;++screenIndex)
		if(vruiState->screens[screenIndex].isIntersect())
			{
			VRScreen* screen=&vruiState->screens[screenIndex];
			
			/* Calculate screen plane: */
			ONTransform t=screen->getScreenTransformation();
			Vector screenNormal=t.getDirection(2);
			Scalar screenOffset=screenNormal*t.getOrigin();
			
			/* Intersect selection ray with screen plane: */
			Scalar divisor=screenNormal*ray.getDirection();
			if(divisor!=Scalar(0))
				{
				Scalar lambda=(screenOffset-screenNormal*ray.getOrigin())/divisor;
				if(lambda>=Scalar(0)&&lambda<closestLambda)
					{
					/* Check if the ray intersects the screen: */
					Point screenPos=t.inverseTransform(ray.getOrigin()+ray.getDirection()*lambda);
					if(screen->isOffAxis())
						{
						/* Check the intersection point against the projected screen quadrilateral: */
						VRScreen::PTransform2::Point sp(screenPos[0],screenPos[1]);
						sp=screen->getScreenHomography().inverseTransform(sp);
						if(sp[0]>=Scalar(0)&&sp[0]<=screen->getWidth()&&sp[1]>=Scalar(0)&&sp[1]<=screen->getHeight())
							{
							/* Save the intersection: */
							closestScreen=screen;
							closestLambda=lambda;
							}
						}
					else
						{
						/* Check the intersection point against the upright screen rectangle: */
						if(screenPos[0]>=Scalar(0)&&screenPos[0]<=screen->getWidth()&&screenPos[1]>=Scalar(0)&&screenPos[1]<=screen->getHeight())
							{
							/* Save the intersection: */
							closestScreen=screen;
							closestLambda=lambda;
							}
						}
					}
				}
			}
	
	return std::pair<VRScreen*,Scalar>(closestScreen,closestLambda);
	}

void requestWindowProperties(const WindowProperties& properties)
	{
	/* Merge the given properties with the accumulated properties: */
	vruiState->windowProperties.merge(properties);
	}

Listener* getMainListener(void)
	{
	return vruiState->mainListener;
	}

int getNumListeners(void)
	{
	return vruiState->numListeners;
	}

Listener* getListener(int index)
	{
	return &vruiState->listeners[index];
	}

Listener* findListener(const char* name)
	{
	Listener* result=0;
	for(int i=0;i<vruiState->numListeners;++i)
		if(strcmp(name,vruiState->listeners[i].getName())==0)
			{
			result=&vruiState->listeners[i];
			break;
			}
	return result;
	}

void requestSound(void)
	{
	vruiState->useSound=true;
	}

Scalar getInchFactor(void)
	{
	return vruiState->inchScale;
	}

Scalar getMeterFactor(void)
	{
	return vruiState->meterScale;
	}

Scalar getDisplaySize(void)
	{
	return vruiState->displaySize;
	}

const Point& getDisplayCenter(void)
	{
	return vruiState->displayCenter;
	}

const Vector& getForwardDirection(void)
	{
	return vruiState->forwardDirection;
	}

const Vector& getUpDirection(void)
	{
	return vruiState->upDirection;
	}

const Plane& getFloorPlane(void)
	{
	return vruiState->floorPlane;
	}

void setFrontplaneDist(Scalar newFrontplaneDist)
	{
	vruiState->frontplaneDist=newFrontplaneDist;
	}

Scalar getFrontplaneDist(void)
	{
	return vruiState->frontplaneDist;
	}

void setBackplaneDist(Scalar newBackplaneDist)
	{
	vruiState->backplaneDist=newBackplaneDist;
	}

Scalar getBackplaneDist(void)
	{
	return vruiState->backplaneDist;
	}

void setBackgroundColor(const Color& newBackgroundColor)
	{
	vruiState->backgroundColor=newBackgroundColor;
	}

const Color& getBackgroundColor(void)
	{
	return vruiState->backgroundColor;
	}

GLFont* loadFont(const char* fontName)
	{
	return new GLFont(fontName);
	}

const GLMotif::StyleSheet* getUiStyleSheet(void)
	{
	return &vruiState->uiStyleSheet;
	}

float getUiSize(void)
	{
	return vruiState->uiStyleSheet.size;
	}

const Color& getUiBgColor(void)
	{
	return vruiState->uiStyleSheet.bgColor;
	}

const Color& getUiFgColor(void)
	{
	return vruiState->uiStyleSheet.fgColor;
	}

const Color& getUiTextFieldBgColor(void)
	{
	return vruiState->uiStyleSheet.textfieldBgColor;
	}

const Color& getUiTextFieldFgColor(void)
	{
	return vruiState->uiStyleSheet.textfieldFgColor;
	}

GLFont* getUiFont(void)
	{
	return vruiState->uiStyleSheet.font;
	}

void setWidgetMaterial(const GLMaterial& newWidgetMaterial)
	{
	vruiState->widgetMaterial=newWidgetMaterial;
	}

const GLMaterial& getWidgetMaterial(void)
	{
	return vruiState->widgetMaterial;
	}

void setMainMenu(GLMotif::PopupMenu* newMainMenu)
	{
	/* Delete old main menu shell and system menu popup: */
	delete vruiState->mainMenu;
	delete vruiState->systemMenuPopup;
	vruiState->systemMenuPopup=0;
	
	/* Add the Vrui system menu to the end of the given main menu: */
	GLMotif::Menu* menuChild=dynamic_cast<GLMotif::Menu*>(newMainMenu->getChild());
	if(menuChild!=0)
		{
		/* Create the Vrui system menu (not saved, because it's deleted automatically by the cascade button): */
		GLMotif::Popup* systemMenuPopup=new GLMotif::Popup("VruiSystemMenuPopup",vruiState->widgetManager);
		GLMotif::SubMenu* systemMenu=new GLMotif::SubMenu("VruiSystemMenu",systemMenuPopup,false);
		vruiState->buildSystemMenu(systemMenu);
		systemMenu->manageChild();
		
		/* Create a cascade button at the end of the main menu: */
		new GLMotif::Separator("VruiSystemMenuSeparator",menuChild,GLMotif::Separator::HORIZONTAL,0.0f,GLMotif::Separator::LOWERED);
		
		GLMotif::CascadeButton* systemMenuCascade=new GLMotif::CascadeButton("VruiSystemMenuCascade",menuChild,"Vrui System");
		systemMenuCascade->setPopup(systemMenuPopup);
		}
	
	/* Create new main menu shell: */
	vruiState->mainMenu=new MutexMenu(newMainMenu);
	}

MutexMenu* getMainMenu(void)
	{
	return vruiState->mainMenu;
	}

Misc::TimerEventScheduler* getTimerEventScheduler(void)
	{
	return vruiState->timerEventScheduler;
	}

TextEventDispatcher* getTextEventDispatcher(void)
	{
	return vruiState->textEventDispatcher;
	}

GLMotif::WidgetManager* getWidgetManager(void)
	{
	return vruiState->widgetManager;
	}

const ONTransform& getUiPlane(void)
	{
	return vruiState->widgetPlane;
	}

Point calcUiPoint(const Ray& ray)
	{
	if(vruiState->popWidgetsOnScreen)
		{
		/* Intersect the ray with the widget plane: */
		Point planeCenter=vruiState->widgetPlane.getOrigin();
		Vector planeNormal=vruiState->widgetPlane.getDirection(2);
		Scalar lambda=((planeCenter-ray.getOrigin())*planeNormal)/(ray.getDirection()*planeNormal);
		return ray(lambda);
		}
	else
		{
		/* Return ray's start: */
		return ray.getOrigin();
		}
	}

ONTransform calcUiTransform(const Ray& ray)
	{
	if(vruiState->popWidgetsOnScreen)
		{
		/* Intersect the ray with the widget plane: */
		Point planeCenter=vruiState->widgetPlane.getOrigin();
		Vector planeNormal=vruiState->widgetPlane.getDirection(2);
		Scalar lambda=((planeCenter-ray.getOrigin())*planeNormal)/(ray.getDirection()*planeNormal);
		
		/* Move the widget plane transformation to the intersection point: */
		ONTransform result=vruiState->widgetPlane;
		result.getTranslation()=ray(lambda)-Point::origin;
		return result;
		}
	else
		{
		/* Align the transformation with the viewing direction: */
		Vector viewDirection=ray.getOrigin()-vruiState->mainViewer->getHeadPosition();
		Vector x=viewDirection^vruiState->upDirection;
		Vector y=x^viewDirection;
		ONTransform result=ONTransform::translateFromOriginTo(ray.getOrigin());
		result*=ONTransform::rotate(Rotation::fromBaseVectors(x,y));
		return result;
		}
	}

ONTransform calcHUDTransform(const Point& hotSpot)
	{
	ONTransform result;
	
	if(vruiState->popWidgetsOnScreen||Math::abs(vruiState->widgetPlane.inverseTransform(hotSpot)[2])<vruiState->uiStyleSheet.size)
		{
		/* Create a ray from the main viewer through the hot spot: */
		Point start=vruiState->mainViewer->getHeadPosition();
		Vector dir=hotSpot-start;
		
		/* Intersect the ray with the widget plane: */
		Point planeCenter=vruiState->widgetPlane.getOrigin();
		Vector planeNormal=vruiState->widgetPlane.getDirection(2);
		Scalar lambda=((planeCenter-start)*planeNormal)/(dir*planeNormal);
		
		/* Move the widget plane transformation to the intersection point: */
		ONTransform result=vruiState->widgetPlane;
		result.getTranslation()=(start+dir*lambda)-Point::origin;
		return result;
		}
	else
		{
		/* Align the transformation with the viewing direction: */
		Vector viewDirection=hotSpot-vruiState->mainViewer->getHeadPosition();
		Vector x=viewDirection^vruiState->upDirection;
		Vector y=x^viewDirection;
		result=ONTransform::translateFromOriginTo(hotSpot);
		result*=ONTransform::rotate(Rotation::fromBaseVectors(x,y));
		}
	
	return result;
	}

void popupPrimaryWidget(GLMotif::Widget* topLevel)
	{
	/* Initialize the pop-up position: */
	Point hotSpot=vruiState->mostRecentHotSpot;
	
	/* Check if there is a most-recently used GUI interactor: */
	if(vruiState->mostRecentGUIInteractor!=0)
		{
		/* Get the interactor's hot spot: */
		hotSpot=vruiState->mostRecentGUIInteractor->calcHotSpot();
		}
	
	/* Pop up the widget at the tool's hot spot position: */
	popupPrimaryWidget(topLevel,hotSpot,false);
	}

void popupPrimaryWidget(GLMotif::Widget* topLevel,const Point& hotSpot,bool navigational)
	{
	typedef GLMotif::WidgetManager::Transformation WTransform;
	typedef WTransform::Vector WVector;
	
	/* Calculate the hot spot in physical coordinates: */
	Point globalHotSpot=hotSpot;
	if(navigational&&vruiState->navigationTransformationEnabled)
		globalHotSpot=vruiState->inverseNavigationTransformation.transform(globalHotSpot);
	
	/* Calculate the widget transformation: */
	WTransform widgetTransformation=calcHUDTransform(globalHotSpot);
	
	/* Align the widget's hot spot with the given hot spot: */
	GLMotif::Vector widgetHotSpot=topLevel->calcHotSpot();
	widgetTransformation*=WTransform::translate(-WVector(widgetHotSpot.getXyzw()));
	
	/* Pop up the widget: */
	vruiState->widgetManager->popupPrimaryWidget(topLevel,widgetTransformation);
	}

void popupPrimaryScreenWidget(GLMotif::Widget* topLevel,Scalar x,Scalar y)
	{
	typedef GLMotif::WidgetManager::Transformation WTransform;
	typedef WTransform::Vector WVector;
	
	/* Calculate a transformation moving the widget to its given position on the screen: */
	Scalar screenX=x*(vruiState->mainScreen->getWidth()-topLevel->getExterior().size[0]);
	Scalar screenY=y*(vruiState->mainScreen->getHeight()-topLevel->getExterior().size[1]);
	WTransform widgetTransformation=vruiState->mainScreen->getTransform();
	widgetTransformation*=WTransform::translate(WVector(screenX,screenY,vruiState->inchScale));
	
	/* Pop up the widget: */
	vruiState->widgetManager->popupPrimaryWidget(topLevel,widgetTransformation);
	}

void popdownPrimaryWidget(GLMotif::Widget* topLevel)
	{
	/* Pop down the widget: */
	vruiState->widgetManager->popdownWidget(topLevel);
	}

namespace {

/**************************************
Helper function to close error dialogs:
**************************************/

void closeWindowCallback(Misc::CallbackData* cbData,void*)
	{
	/* Check if the callback came from a button: */
	GLMotif::Button::CallbackData* buttonCbData=dynamic_cast<GLMotif::Button::CallbackData*>(cbData);
	if(buttonCbData!=0)
		{
		/* Close the top-level widget to which the button belongs: */
		getWidgetManager()->deleteWidget(buttonCbData->button->getRoot());
		}
	
	/* Check if the callback came from a popup window: */
	GLMotif::PopupWindow::CallbackData* windowCbData=dynamic_cast<GLMotif::PopupWindow::CallbackData*>(cbData);
	if(windowCbData!=0)
		{
		/* Close the popup window: */
		getWidgetManager()->deleteWidget(windowCbData->popupWindow);
		}
	}

}

void showErrorMessage(const char* title,const char* message)
	{
	/* Create a popup window: */
	GLMotif::PopupWindow* errorDialog=new GLMotif::PopupWindow("VruiErrorMessage",getWidgetManager(),title);
	errorDialog->setResizableFlags(false,false);
	errorDialog->setHideButton(false);
	
	GLMotif::RowColumn* error=new GLMotif::RowColumn("Error",errorDialog,false);
	error->setOrientation(GLMotif::RowColumn::VERTICAL);
	error->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	
	/* Skip initial whitespace in the error message: */
	const char* linePtr=message;
	while(isspace(*linePtr)&&*linePtr!='\0')
		++linePtr;
	
	/* Break the error message into multiple lines: */
	while(*linePtr!='\0')
		{
		/* Find potential line break points: */
		const char* breakPtr=0;
		const char* cPtr=linePtr;
		do
			{
			/* Find the end of the current word: */
			while(!isspace(*cPtr)&&*cPtr!='-'&&*cPtr!='/'&&*cPtr!='\0')
				++cPtr;
			
			/* Skip past dashes and slashes: */
			while(*cPtr=='-'||*cPtr=='/')
				++cPtr;
			
			/* If the line is already too long, and there is a previous break point, break there: */
			if(cPtr-linePtr>=40&&breakPtr!=0)
				break;
			
			/* Mark the break point: */
			breakPtr=cPtr;
			
			/* Skip whitespace: */
			while(isspace(*cPtr)&&*cPtr!='\0')
				++cPtr;
			}
		while(cPtr-linePtr<40&&*breakPtr!='\n'&&*breakPtr!='\0');
		
		/* Add the current line: */
		new GLMotif::Label("ErrorLine",error,linePtr,breakPtr);
		
		/* Go to the beginning of the next line: */
		linePtr=breakPtr;
		while(isspace(*linePtr)&&*linePtr!='\0')
			++linePtr;
		}
	
	/* Add an acknowledgment button: */
	GLMotif::Margin* buttonMargin=new GLMotif::Margin("ButtonMargin",error,false);
	buttonMargin->setAlignment(GLMotif::Alignment::RIGHT);
	GLMotif::Button* okButton=new GLMotif::Button("OkButton",buttonMargin,"Too Sad!");
	okButton->getSelectCallbacks().add(closeWindowCallback,0);
	
	buttonMargin->manageChild();
	
	error->manageChild();
	
	/* Show the popup window: */
	popupPrimaryWidget(errorDialog);
	}

Scalar getPointPickDistance(void)
	{
	if(vruiState->navigationTransformationEnabled)
		return vruiState->pointPickDistance*vruiState->inverseNavigationTransformation.getScaling();
	else
		return vruiState->pointPickDistance;
	}

Scalar getRayPickCosine(void)
	{
	return vruiState->rayPickCosine;
	}

void setNavigationTransformation(const NavTransform& newNavigationTransformation)
	{
	#if DELAY_NAVIGATIONTRANSFORMATION
	if(vruiState->delayNavigationTransformation)
		{
		/* Schedule a change in navigation transformation for the next frame: */
		vruiState->newNavigationTransformation=newNavigationTransformation;
		vruiState->newNavigationTransformation.renormalize();
		if(!vruiState->navigationTransformationEnabled||vruiState->newNavigationTransformation!=vruiState->navigationTransformation)
			{
			vruiState->navigationTransformationChangedMask|=0x1;
			requestUpdate();	
			}
		}
	else
		{
		/* Change the navigation transformation right away: */
		vruiState->updateNavigationTransformation(newNavigationTransformation);
		}
	#else
	/* Change the navigation transformation right away: */
	vruiState->updateNavigationTransformation(newNavigationTransformation);
	#endif
	
	vruiState->navigationTransformationEnabled=true;
	}

void setNavigationTransformation(const Point& center,Scalar radius)
	{
	/* Assemble the new navigation transformation: */
	NavTransform t=NavTransform::translateFromOriginTo(vruiState->displayCenter);
	t*=NavTransform::scale(vruiState->displaySize/radius);
	t*=NavTransform::translateToOriginFrom(center);
	
	#if DELAY_NAVIGATIONTRANSFORMATION
	if(vruiState->delayNavigationTransformation)
		{
		/* Schedule a change in navigation transformation for the next frame: */
		vruiState->newNavigationTransformation=t;
		if(!vruiState->navigationTransformationEnabled||vruiState->newNavigationTransformation!=vruiState->navigationTransformation)
			{
			vruiState->navigationTransformationChangedMask|=0x1;
			requestUpdate();	
			}
		}
	else
		{
		/* Change the navigation transformation right away: */
		vruiState->updateNavigationTransformation(t);
		}
	#else
	/* Change the navigation transformation right away: */
	vruiState->updateNavigationTransformation(t);
	#endif
	
	vruiState->navigationTransformationEnabled=true;
	}

void setNavigationTransformation(const Point& center,Scalar radius,const Vector& up)
	{
	/* Assemble the new navigation transformation: */
	NavTransform t=NavTransform::translateFromOriginTo(vruiState->displayCenter);
	t*=NavTransform::scale(vruiState->displaySize/radius);
	t*=NavTransform::rotate(NavTransform::Rotation::rotateFromTo(up,vruiState->upDirection));
	t*=NavTransform::translateToOriginFrom(center);
	
	#if DELAY_NAVIGATIONTRANSFORMATION
	if(vruiState->delayNavigationTransformation)
		{
		/* Schedule a change in navigation transformation for the next frame: */
		vruiState->newNavigationTransformation=t;
		if(!vruiState->navigationTransformationEnabled||vruiState->newNavigationTransformation!=vruiState->navigationTransformation)
			{
			vruiState->navigationTransformationChangedMask|=0x1;
			requestUpdate();	
			}
		}
	else
		{
		/* Change the navigation transformation right away: */
		vruiState->updateNavigationTransformation(t);
		}
	#else
	/* Change the navigation transformation right away: */
	vruiState->updateNavigationTransformation(t);
	#endif
	
	vruiState->navigationTransformationEnabled=true;
	}

void concatenateNavigationTransformation(const NavTransform& t)
	{
	/* Bail out if the incremental transformation is the identity transformation: */
	if(t==NavTransform::identity)
		return;
	
	#if DELAY_NAVIGATIONTRANSFORMATION
	if(vruiState->delayNavigationTransformation)
		{
		/* Schedule a change in navigation transformation for the next frame: */
		if((vruiState->navigationTransformationChangedMask&0x1)==0)
			vruiState->newNavigationTransformation=vruiState->navigationTransformation;
		vruiState->newNavigationTransformation*=t;
		vruiState->newNavigationTransformation.renormalize();
		vruiState->navigationTransformationChangedMask|=0x1;
		requestUpdate();
		}
	else
		{
		/* Change the navigation transformation right away: */
		NavTransform newTransform=vruiState->navigationTransformation;
		newTransform*=t;
		newTransform.renormalize();
		vruiState->updateNavigationTransformation(newTransform);
		}
	#else
	/* Change the navigation transformation right away: */
	NavTransform newTransform=vruiState->navigationTransformation;
	newTransform*=t;
	newTransform.renormalize();
	vruiState->updateNavigationTransformation(newTransform);
	#endif
	}

void concatenateNavigationTransformationLeft(const NavTransform& t)
	{
	/* Bail out if the incremental transformation is the identity transformation: */
	if(t==NavTransform::identity)
		return;
	
	#if DELAY_NAVIGATIONTRANSFORMATION
	if(vruiState->delayNavigationTransformation)
		{
		/* Schedule a change in navigation transformation for the next frame: */
		if((vruiState->navigationTransformationChangedMask&0x1)==0)
			vruiState->newNavigationTransformation=vruiState->navigationTransformation;
		vruiState->newNavigationTransformation.leftMultiply(t);
		vruiState->newNavigationTransformation.renormalize();
		vruiState->navigationTransformationChangedMask|=0x1;
		requestUpdate();
		}
	else
		{
		/* Change the navigation transformation right away: */
		NavTransform newTransform=vruiState->navigationTransformation;
		newTransform.leftMultiply(t);
		newTransform.renormalize();
		vruiState->updateNavigationTransformation(newTransform);
		}
	#else
	/* Change the navigation transformation right away: */
	NavTransform newTransform=vruiState->navigationTransformation;
	newTransform.leftMultiply(t);
	newTransform.renormalize();
	vruiState->updateNavigationTransformation(newTransform);
	#endif
	}

const NavTransform& getNavigationTransformation(void)
	{
	if(vruiState->navigationTransformationEnabled)
		return vruiState->navigationTransformation;
	else
		return NavTransform::identity;
	}

const NavTransform& getInverseNavigationTransformation(void)
	{
	if(vruiState->navigationTransformationEnabled)
		return vruiState->inverseNavigationTransformation;
	else
		return NavTransform::identity;
	}

void disableNavigationTransformation(void)
	{
	/* Disable and reset the navigation transformation: */
	vruiState->navigationTransformationEnabled=false;
	vruiState->updateNavigationTransformation(NavTransform::identity);
	}

Point getHeadPosition(void)
	{
	if(vruiState->navigationTransformationEnabled)
		return vruiState->inverseNavigationTransformation.transform(vruiState->mainViewer->getHeadPosition());
	else
		return vruiState->mainViewer->getHeadPosition();
	}

Vector getViewDirection(void)
	{
	if(vruiState->navigationTransformationEnabled)
		return vruiState->inverseNavigationTransformation.transform(vruiState->mainViewer->getViewDirection());
	else
		return vruiState->mainViewer->getViewDirection();
	}

Point getDevicePosition(InputDevice* device)
	{
	if(vruiState->navigationTransformationEnabled)
		return vruiState->inverseNavigationTransformation.transform(device->getPosition());
	else
		return device->getPosition();
	}

NavTrackerState getDeviceTransformation(InputDevice* device)
	{
	if(vruiState->navigationTransformationEnabled)
		return vruiState->inverseNavigationTransformation*NavTransform(device->getTransformation());
	else
		return device->getTransformation();
	}

Misc::CallbackList& getNavigationTransformationChangedCallbacks(void)
	{
	return vruiState->navigationTransformationChangedCallbacks;
	}

CoordinateManager* getCoordinateManager(void)
	{
	return vruiState->coordinateManager;
	}

ToolManager* getToolManager(void)
	{
	return vruiState->toolManager;
	}

bool activateNavigationTool(const Tool* tool)
	{
	/* Can not activate the given tool if navigation is disabled: */
	if(!vruiState->navigationTransformationEnabled)
		return false;
	
	/* Can not activate the given tool if another navigation tool is already active: */
	if(vruiState->activeNavigationTool!=0&&vruiState->activeNavigationTool!=tool)
		return false;
	
	/* Activate the given tool: */
	vruiState->activeNavigationTool=tool;
	return true;
	}

void deactivateNavigationTool(const Tool* tool)
	{
	/* If the given tool is currently active, deactivate it: */
	if(vruiState->activeNavigationTool==tool)
		vruiState->activeNavigationTool=0;
	}

VisletManager* getVisletManager(void)
	{
	return vruiState->visletManager;
	}

Misc::Time getTimeOfDay(void)
	{
	Misc::Time result;
	
	if(vruiState->master)
		{
		/* Query the system's wall clock time: */
		result=Misc::Time::now();
		
		if(vruiState->multiplexer!=0)
			{
			/* Send the time value to the slaves: */
			vruiState->pipe->write(result.tv_sec);
			vruiState->pipe->write(result.tv_nsec);
			vruiState->pipe->flush();
			}
		}
	else
		{
		/* Receive the time value from the master: */
		vruiState->pipe->read(result.tv_sec);
		vruiState->pipe->read(result.tv_nsec);
		}
	
	return result;
	}

double getApplicationTime(void)
	{
	return vruiState->lastFrame;
	}

double getFrameTime(void)
	{
	return vruiState->lastFrameDelta;
	}

double getCurrentFrameTime(void)
	{
	return vruiState->currentFrameTime;
	}

void updateContinuously(void)
	{
	vruiState->updateContinuously=true;
	}

void scheduleUpdate(double nextFrameTime)
	{
	if(vruiState->nextFrameTime==0.0||vruiState->nextFrameTime>nextFrameTime)
		vruiState->nextFrameTime=nextFrameTime;
	}

const DisplayState& getDisplayState(GLContextData& contextData)
	{
	/* Retrieve the display state mapper's data item from the OpenGL context: */
	VruiState::DisplayStateMapper::DataItem* dataItem=contextData.retrieveDataItem<VruiState::DisplayStateMapper::DataItem>(&vruiState->displayStateMapper);
	
	/* Return the embedded display state object: */
	return dataItem->displayState;
	}

void setMostRecentGUIInteractor(GUIInteractor* interactor)
	{
	vruiState->mostRecentGUIInteractor=interactor;
	}

#if EVILHACK_LOCK_INPUTDEVICE_POS

void lockDevice(Vrui::InputDevice* device)
	{
	lockedDevice=device;
	if(lockedDevice!=0)
		lockedTranslation=lockedDevice->getTransformation().getTranslation();
	}
	
#endif

}
