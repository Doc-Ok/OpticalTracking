/***********************************************************************
ToolManager - Class to manage tool classes, and dynamic assignment of
tools to input devices.
Copyright (c) 2004-2014 Oliver Kreylos

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

#include <Vrui/ToolManager.h>

#include <stdio.h>
#include <iostream>
#include <Misc/ThrowStdErr.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/CompoundValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/Label.h>
#include <GLMotif/Button.h>
#include <GLMotif/CascadeButton.h>
#include <GLMotif/Popup.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/Menu.h>
#include <GLMotif/SubMenu.h>
#include <GLMotif/PopupMenu.h>
#include <GLMotif/PopupWindow.h>
#include <Vrui/Vrui.h>
#include <Vrui/InputDevice.h>
#include <Vrui/InputDeviceFeature.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/MutexMenu.h>
#include <Vrui/DeviceForwarder.h>
#include <Vrui/LocatorTool.h>
#include <Vrui/DraggingTool.h>
#include <Vrui/NavigationTool.h>
#include <Vrui/SurfaceNavigationTool.h>
#include <Vrui/TransformTool.h>
#include <Vrui/UserInterfaceTool.h>
#include <Vrui/MenuTool.h>
#include <Vrui/InputDeviceTool.h>
#include <Vrui/PointingTool.h>
#include <Vrui/UtilityTool.h>
#include <Vrui/Internal/ToolKillZone.h>
#include <Vrui/Internal/ToolKillZoneBox.h>
#include <Vrui/Internal/ToolKillZoneFrustum.h>
#include <Vrui/Internal/Config.h>

#define DEBUGGING 0

namespace {

/**************
Helper classes:
**************/

struct ToolAssignment // Structure to parse tool button or valuator assignments from configuration files
	{
	/* Elements: */
	public:
	Vrui::InputDevice* device; // Input device containing the following features
	std::vector<int> featureIndices; // List of feature indices on the device
	};

}

namespace Misc {

/***************************************
Helper class to decode tool assignments:
***************************************/

template <>
class ValueCoder<ToolAssignment>
	{
	/* Methods: */
	public:
	static std::string encode(const ToolAssignment& value)
		{
		std::string result;
		
		result.push_back('(');
		
		/* Write the device name: */
		result.append(ValueCoder<std::string>::encode(value.device->getDeviceName()));
		
		/* Write the feature names: */
		for(std::vector<int>::const_iterator fiIt=value.featureIndices.begin();fiIt!=value.featureIndices.end();++fiIt)
			{
			result.append(", ");
			std::string featureName=Vrui::getInputDeviceManager()->getFeatureName(Vrui::InputDeviceFeature(value.device,*fiIt));
			result.append(ValueCoder<std::string>::encode(featureName));
			}
		
		result.push_back(')');
		return result;
		}
	static ToolAssignment decode(const char* start,const char* end,const char** decodeEnd)
		{
		ToolAssignment result;
		
		/* Check for opening parenthesis: */
		const char* cPtr=start;
		if(*cPtr!='(')
			throw DecodingError(std::string("Missing opening parenthesis in ")+std::string(start,end));
		++cPtr;
		while(*cPtr!='\0'&&isspace(*cPtr))
			++cPtr;
		
		/* Get the device pointer: */
		std::string deviceName=ValueCoder<std::string>::decode(cPtr,end,&cPtr);
		result.device=Vrui::getInputDeviceManager()->findInputDevice(deviceName.c_str());
		if(result.device==0)
			throw DecodingError(std::string("Unknown input device ")+deviceName);
		
		/* Read all feature names: */
		while(true)
			{
			/* Check for a comma separator: */
			while(*cPtr!='\0'&&isspace(*cPtr))
				++cPtr;
			if(*cPtr==')')
				{
				++cPtr;
				break;
				}
			else if(*cPtr!=',')
				throw DecodingError(std::string("Missing opening parenthesis in ")+std::string(start,end));
			++cPtr;
			while(*cPtr!='\0'&&isspace(*cPtr))
				++cPtr;
			
			/* Get the feature index: */
			std::string featureName=ValueCoder<std::string>::decode(cPtr,end,&cPtr);
			int featureIndex=Vrui::getInputDeviceManager()->getFeatureIndex(result.device,featureName.c_str());
			if(featureIndex==-1)
				throw DecodingError(std::string("Unknown feature ")+featureName);
			result.featureIndices.push_back(featureIndex);
			}
		
		if(decodeEnd!=0)
			*decodeEnd=cPtr;
		return result;
		}
	};

}

namespace Vrui {

/**********************************************************************
Helper class to keep track of the state of the tool creation algorithm:
**********************************************************************/

struct ToolManagerToolCreationState
	{
	/* Elements: */
	public:
	InputDeviceFeature firstFeature; // First assigned input device feature
	std::string firstFeatureName; // Name of first feature to guide users to confirm or cancel
	InputDevice* toolSelectionDevice; // Input device used to select from the tool selection menu
	ToolFactory* factory; // Pointer to the factory object for the selected new tool class
	ToolInputAssignment* tia; // Pointer to the input assignment for the new tool
	int buttonSlotIndex; // Index of next button slot to assign
	int valuatorSlotIndex; // Index of next valuator slot to assign
	InputDeviceFeature preparedFeature; // The currently prepared input device feature, to prevent parallel assignments
	GLMotif::PopupWindow* progressDialog; // Pointer to the tool creation progress dialog window
	GLMotif::RowColumn* progressBox; // Pointer to the process dialog's top-level container
	GLMotif::RowColumn* buttonBox; // Pointer to the container asking for a button press
	GLMotif::RowColumn* valuatorBox; // Pointer to the container asking for a valuator push
	GLMotif::Label* cancelLine; // Pointer to the cancellation / confirmation advisory line
	
	/* Private methods: */
	void createProgressDialog(void);
	void updateProgressDialog(void);
	
	/* Constructors and destructors: */
	ToolManagerToolCreationState(const InputDeviceManager& inputDeviceManager,const InputDeviceFeature& sFirstFeature); // Creates a tool creation state in the initial state
	~ToolManagerToolCreationState(void); // Destroys the tool creation state
	
	/* Methods: */
	bool assignFeature(void); // Assigns the prepared feature to the next available tool input assignment slot; returns true if assignment is finished
	bool isComplete(void) const; // Returns true if the tool creation process is complete and the tool can be created
	};

/*********************************************
Methods of class ToolManagerToolCreationState:
*********************************************/

void ToolManagerToolCreationState::createProgressDialog(void)
	{
	char titleBuffer[256];
	snprintf(titleBuffer,sizeof(titleBuffer),"Creating \"%s\" Tool...",factory->getName());
	progressDialog=new GLMotif::PopupWindow("ToolCreationProgressDialog",getWidgetManager(),titleBuffer);
	progressDialog->setResizableFlags(false,false);
	progressDialog->setHideButton(false);
	
	progressBox=new GLMotif::RowColumn("ProgressBox",progressDialog,false);
	progressBox->setOrientation(GLMotif::RowColumn::VERTICAL);
	progressBox->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	
	bool requireButtons=buttonSlotIndex<factory->getLayout().getNumButtons();
	bool moreButtons=requireButtons||factory->getLayout().hasOptionalButtons();
	bool requireValuators=valuatorSlotIndex<factory->getLayout().getNumValuators();
	bool moreValuators=requireValuators||factory->getLayout().hasOptionalValuators();
	
	/* Check if there are still button slots to assign: */
	if(moreButtons)
		{
		/* Create a box asking for more button bindings: */
		buttonBox=new GLMotif::RowColumn("ButtonBox",progressBox,false);
		buttonBox->setOrientation(GLMotif::RowColumn::VERTICAL);
		buttonBox->setPacking(GLMotif::RowColumn::PACK_TIGHT);
		
		new GLMotif::Label("Line1",buttonBox,"Please press the button");
		if(requireButtons)
			new GLMotif::Label("Line2",buttonBox,"to assign to tool function");
		else
			new GLMotif::Label("Line2",buttonBox,"to assign to optional tool function");
		new GLMotif::Label("Line3",buttonBox,factory->getButtonFunction(buttonSlotIndex));
		
		buttonBox->manageChild();
		}

	if(moreButtons&&moreValuators)
		new GLMotif::Label("OrLine",progressBox,"-- or --");
	
	/* Check if there are still valuator slots to assign: */
	if(moreValuators)
		{
		/* Create a box asking for more valuator bindings: */
		valuatorBox=new GLMotif::RowColumn("ValuatorBox",progressBox,false);
		valuatorBox->setOrientation(GLMotif::RowColumn::VERTICAL);
		valuatorBox->setPacking(GLMotif::RowColumn::PACK_TIGHT);
		
		new GLMotif::Label("Line1",valuatorBox,"Please move the valuator");
		if(requireValuators)
			new GLMotif::Label("Line2",valuatorBox,"to assign to tool function");
		else
			new GLMotif::Label("Line2",valuatorBox,"to assign to optional tool function");
		new GLMotif::Label("Line3",valuatorBox,factory->getValuatorFunction(valuatorSlotIndex));
		
		valuatorBox->manageChild();
		}
	
	/* Create the cancellation / confirmation line: */
	char cancelLineBuffer[256];
	bool isButton=firstFeature.isButton();
	snprintf(cancelLineBuffer,sizeof(cancelLineBuffer),"%s %s again to %s",isButton?"Press":"Move",firstFeatureName.c_str(),(requireButtons||requireValuators)?"cancel":"confirm");
	cancelLine=new GLMotif::Label("CancelLine",progressBox,cancelLineBuffer);
	
	progressBox->manageChild();
	
	/* Show the popup window: */
	popupPrimaryWidget(progressDialog);
	}

void ToolManagerToolCreationState::updateProgressDialog(void)
	{
	/* Check if there are still button slots to assign: */
	bool requireButtons=buttonSlotIndex<factory->getLayout().getNumButtons();
	if(requireButtons||factory->getLayout().hasOptionalButtons())
		{
		/* Change the prompt text if this is the first optional button: */
		if(buttonSlotIndex==factory->getLayout().getNumButtons())
			{
			GLMotif::Label* buttonPrompt=static_cast<GLMotif::Label*>(buttonBox->getChild(1));
			buttonPrompt->setString("to assign to optional tool function");
			}
		
		/* Show the function for the next assigned button: */
		GLMotif::Label* buttonFunction=static_cast<GLMotif::Label*>(buttonBox->getChild(2));
		buttonFunction->setString(factory->getButtonFunction(buttonSlotIndex));
		}
	else if(buttonBox!=0)
		{
		/* Remove the button box: */
		if(valuatorBox!=0)
			progressBox->removeWidgets(1);
		progressBox->removeWidgets(0);
		buttonBox=0;
		}
	
	/* Check if there are still valuator slots to assign: */
	bool requireValuators=valuatorSlotIndex<factory->getLayout().getNumValuators();
	if(requireValuators||factory->getLayout().hasOptionalValuators())
		{
		/* Change the prompt text if this is the first optional valuator: */
		if(valuatorSlotIndex==factory->getLayout().getNumValuators())
			{
			GLMotif::Label* valuatorPrompt=static_cast<GLMotif::Label*>(valuatorBox->getChild(1));
			valuatorPrompt->setString("to assign to optional tool function");
			}
		
		/* Show the function for the next assigned valuator: */
		GLMotif::Label* valuatorFunction=static_cast<GLMotif::Label*>(valuatorBox->getChild(2));
		valuatorFunction->setString(factory->getValuatorFunction(valuatorSlotIndex));
		}
	else if(valuatorBox!=0)
		{
		/* Remove the valuator box: */
		if(buttonBox!=0)
			{
			progressBox->removeWidgets(2);
			progressBox->removeWidgets(1);
			}
		else
			progressBox->removeWidgets(0);
		valuatorBox=0;
		}
	
	/* Check if all required slots have been assigned: */
	if(!(requireButtons||requireValuators))
		{
		char cancelLineBuffer[256];
		snprintf(cancelLineBuffer,sizeof(cancelLineBuffer),"%s %s again to confirm",firstFeature.isButton()?"Press":"Move",firstFeatureName.c_str());
		cancelLine->setString(cancelLineBuffer);
		}
	}

ToolManagerToolCreationState::ToolManagerToolCreationState(const InputDeviceManager& inputDeviceManager,const InputDeviceFeature& sFirstFeature)
	:firstFeature(sFirstFeature),
	 firstFeatureName(inputDeviceManager.getFeatureName(firstFeature)),
	 toolSelectionDevice(0),
	 factory(0),tia(0),
	 buttonSlotIndex(0),valuatorSlotIndex(0),
	 preparedFeature(firstFeature),
	 progressDialog(0),progressBox(0),buttonBox(0),valuatorBox(0),cancelLine(0)
	{
	}

ToolManagerToolCreationState::~ToolManagerToolCreationState(void)
	{
	delete tia;
	if(progressDialog!=0)
		getWidgetManager()->deleteWidget(progressDialog);
	}

bool ToolManagerToolCreationState::assignFeature(void)
	{
	/* Remember if this is the first assigned feature: */
	bool firstAssignment=buttonSlotIndex==0&&valuatorSlotIndex==0;
	
	/* Check if the initial tool assignment slot is assigned again: */
	if(!firstAssignment&&preparedFeature==firstFeature)
		{
		/* Confirm or cancel, tool creation process is done either way: */
		return true;
		}
	
	/* Check if the feature has already been used in this tool creation process: */
	if(tia->isAssigned(preparedFeature))
		return false;
	
	if(preparedFeature.isButton())
		{
		/* Get the feature's button index: */
		int buttonIndex=preparedFeature.getIndex();
		
		/* Assign the button slot: */
		if(buttonSlotIndex<factory->getLayout().getNumButtons())
			{
			tia->setButtonSlot(buttonSlotIndex,preparedFeature.getDevice(),buttonIndex);
			++buttonSlotIndex;
			}
		else if(factory->getLayout().hasOptionalButtons())
			{
			tia->addButtonSlot(preparedFeature.getDevice(),buttonIndex);
			++buttonSlotIndex;
			}
		}
	
	if(preparedFeature.isValuator())
		{
		/* Get the feature's valuator index: */
		int valuatorIndex=preparedFeature.getIndex();
		
		/* Assign the valuator slot: */
		if(valuatorSlotIndex<factory->getLayout().getNumValuators())
			{
			tia->setValuatorSlot(valuatorSlotIndex,preparedFeature.getDevice(),valuatorIndex);
			++valuatorSlotIndex;
			}
		else if(factory->getLayout().hasOptionalValuators())
			{
			tia->addValuatorSlot(preparedFeature.getDevice(),valuatorIndex);
			++valuatorSlotIndex;
			}
		}
	
	/* Check if the tool creation process is incomplete: */
	if(buttonSlotIndex<factory->getLayout().getNumButtons()||factory->getLayout().hasOptionalButtons()||valuatorSlotIndex<factory->getLayout().getNumValuators()||factory->getLayout().hasOptionalValuators())
		{
		/* Check if the tool creation process dialog needs to be created or updated: */
		if(firstAssignment)
			createProgressDialog();
		else
			updateProgressDialog();
		
		return false;
		}
	else
		{
		/* Signal completion: */
		return true;
		}
	}

bool ToolManagerToolCreationState::isComplete(void) const
	{
	/* Check if there are a factory and a tool input assignment: */
	if(factory==0||tia==0)
		return false;
	
	/* Check if enough buttons and valuators were assigned: */
	return buttonSlotIndex>=factory->getLayout().getNumButtons()&&valuatorSlotIndex>=factory->getLayout().getNumValuators();
	}

/****************************
Methods of class ToolManager:
****************************/

GLMotif::Popup* ToolManager::createToolSubmenu(const Plugins::Factory& factory)
	{
	char popupName[256];
	snprintf(popupName,sizeof(popupName),"%sSubmenuPopup",factory.getClassName());
	GLMotif::Popup* toolSubmenuPopup=new GLMotif::Popup(popupName,getWidgetManager());
	
	GLMotif::SubMenu* toolSubmenu=new GLMotif::SubMenu("ToolSubmenu",toolSubmenuPopup,false);
	
	/* Create entries for all tool subclasses: */
	for(Plugins::Factory::ClassList::const_iterator chIt=factory.childrenBegin();chIt!=factory.childrenEnd();++chIt)
		{
		/* Get a pointer to the tool factory: */
		const ToolFactory* factory=dynamic_cast<const ToolFactory*>(*chIt);
		if(factory==0)
			Misc::throwStdErr("ToolManager::createToolSubmenu: factory class %s is not a Vrui tool factory class",(*chIt)->getClassName());
		
		/* Check if current class is leaf class: */
		if((*chIt)->getChildren().empty())
			{
			/* Create button for tool class: */
			GLMotif::Button* toolButton=new GLMotif::Button((*chIt)->getClassName(),toolSubmenu,factory->getName());
			toolButton->getSelectCallbacks().add(this,&ToolManager::toolMenuSelectionCallback);
			}
		else
			{
			/* Create cascade button and submenu for tool class: */
			GLMotif::CascadeButton* toolCascade=new GLMotif::CascadeButton((*chIt)->getClassName(),toolSubmenu,factory->getName());
			toolCascade->setPopup(createToolSubmenu(**chIt));
			}
		}
	
	toolSubmenu->manageChild();
	
	return toolSubmenuPopup;
	}

GLMotif::PopupMenu* ToolManager::createToolMenu(void)
	{
	/* Create menu shell: */
	GLMotif::PopupMenu* toolSelectionMenuPopup=new GLMotif::PopupMenu("ToolSelectionMenuPopup",getWidgetManager());
	toolSelectionMenuPopup->setTitle("Tool Selection Menu");
	
	GLMotif::Menu* toolSelectionMenu=new GLMotif::Menu("ToolSelectionMenu",toolSelectionMenuPopup,false);
	
	/* Create entries for all root tool classes: */
	for(FactoryIterator fIt=begin();fIt!=end();++fIt)
		{
		/* Check if current class is root class: */
		if(fIt->getParents().empty())
			{
			/* Check if current class is leaf class: */
			if(fIt->getChildren().empty())
				{
				/* Create button for tool class: */
				GLMotif::Button* toolButton=new GLMotif::Button(fIt->getClassName(),toolSelectionMenu,fIt->getName());
				toolButton->getSelectCallbacks().add(this,&ToolManager::toolMenuSelectionCallback);
				}
			else
				{
				/* Create cascade button and submenu for tool class: */
				GLMotif::CascadeButton* toolCascade=new GLMotif::CascadeButton(fIt->getClassName(),toolSelectionMenu,fIt->getName());
				toolCascade->setPopup(createToolSubmenu(*fIt));
				}
			}
		}
	
	toolSelectionMenu->manageChild();
	
	return toolSelectionMenuPopup;
	}

void ToolManager::inputDeviceDestructionCallback(Misc::CallbackData* cbData)
	{
	/* Check if the tool manager is in tool creation mode: */
	if(toolCreationState!=0)
		{
		/* Get pointer to the device to be destroyed: */
		InputDevice* device=static_cast<InputDeviceManager::InputDeviceDestructionCallbackData*>(cbData)->inputDevice;
		
		/* Check if the device is involved in the ongoing tool creation process: */
		bool mustCancel=device==toolCreationState->firstFeature.getDevice()||device==toolCreationState->toolSelectionDevice;
		if(toolCreationState->tia!=0)
			{
			for(int slotIndex=0;!mustCancel&&slotIndex<toolCreationState->tia->getNumSlots();++slotIndex)
				mustCancel=toolCreationState->tia->getSlotDevice(slotIndex)==device;
			}
		
		if(mustCancel)
			{
			/* Cancel the tool creation process: */
			if(toolCreationState->toolSelectionDevice!=0)
				toolCreationState->toolSelectionDevice->getTrackingCallbacks().remove(this,&ToolManager::toolCreationDeviceMotionCallback);
			delete toolCreationState;
			toolCreationState=0;
			}
		}
	}

void ToolManager::toolMenuSelectionCallback(Misc::CallbackData* cbData)
	{
	/* Check if the tool manager is in tool creation mode (paranoia): */
	if(toolCreationState!=0)
		{
		/* Get the pointer to the selected tool class' factory: */
		ToolFactory* factory=loadClass(static_cast<GLMotif::Button::SelectCallbackData*>(cbData)->button->getName());
		
		/* Check if the selected tool class is compatible with the tool creation state's first assigned feature: */
		const ToolInputLayout& layout=factory->getLayout();
		if(toolCreationState->firstFeature.isButton()&&layout.getNumButtons()==0&&!layout.hasOptionalButtons())
			{
			/* Show an error message: */
			std::string message="The selected tool class \"";
			message.append(factory->getName());
			message.append("\" has no assignable button slots");
			showErrorMessage("Tool Creation",message.c_str());
			}
		else if(toolCreationState->firstFeature.isValuator()&&layout.getNumValuators()==0&&!layout.hasOptionalValuators())
			{
			/* Show an error message: */
			std::string message="The selected tool class \"";
			message.append(factory->getName());
			message.append("\" has no assignable valuator slots");
			showErrorMessage("Tool Creation",message.c_str());
			}
		else
			{
			/* Store the tool factory in the tool creation state so that tool creation can continue: */
			toolCreationState->factory=factory;
			}
		}
	}

void ToolManager::toolCreationDeviceMotionCallback(Misc::CallbackData* cbData)
	{
	/* Get a pointer to the input device: */
	InputDevice* device=static_cast<InputDevice::CallbackData*>(cbData)->inputDevice;
	
	/* Check if the tool manager is in tool creation mode, and the callback comes from the selection device (paranoia): */
	if(toolCreationState!=0&&device==toolCreationState->toolSelectionDevice)
		{
		/* Update the tool creation device's state: */
		toolCreationDevice->setDeviceRay(device->getDeviceRayDirection(),device->getDeviceRayStart());
		toolCreationDevice->setTransformation(device->getTransformation());
		
		/* Call the tool creation tool's frame method: */
		toolCreationTool->frame();
		}
	}

ToolManager::ToolManager(InputDeviceManager* sInputDeviceManager,const Misc::ConfigurationFileSection& sConfigFileSection)
	:Plugins::FactoryManager<ToolFactory>(sConfigFileSection.retrieveString("./toolDsoNameTemplate",VRUI_INTERNAL_CONFIG_TOOLDSONAMETEMPLATE)),
	 inputGraphManager(sInputDeviceManager->getInputGraphManager()),
	 inputDeviceManager(sInputDeviceManager),
	 configFileSection(new Misc::ConfigurationFileSection(sConfigFileSection)),
	 toolCreationDevice(0),toolCreationTool(0),
	 toolMenuPopup(0),toolMenu(0),toolCreationState(0),
	 toolKillZone(0)
	{
	typedef std::vector<std::string> StringList;
	
	/* Get additional search paths from configuration file section and add them to the factory manager: */
	StringList toolSearchPaths=configFileSection->retrieveValue<StringList>("./toolSearchPaths",StringList());
	for(StringList::const_iterator tspIt=toolSearchPaths.begin();tspIt!=toolSearchPaths.end();++tspIt)
		{
		/* Add the path: */
		getDsoLocator().addPath(*tspIt);
		}
	
	/* Instantiate basic tool classes: */
	addClass(new LocatorToolFactory(*this),defaultToolFactoryDestructor);
	addClass(new DraggingToolFactory(*this),defaultToolFactoryDestructor);
	addClass(new NavigationToolFactory(*this),defaultToolFactoryDestructor);
	addClass(new SurfaceNavigationToolFactory(*this),defaultToolFactoryDestructor);
	addClass(new TransformToolFactory(*this),defaultToolFactoryDestructor);
	addClass(new UserInterfaceToolFactory(*this),defaultToolFactoryDestructor);
	addClass(new MenuToolFactory(*this),defaultToolFactoryDestructor);
	addClass(new InputDeviceToolFactory(*this),defaultToolFactoryDestructor);
	addClass(new PointingToolFactory(*this),defaultToolFactoryDestructor);
	addClass(new UtilityToolFactory(*this),defaultToolFactoryDestructor);
	
	/* Load default tool classes: */
	StringList toolClassNames=configFileSection->retrieveValue<StringList>("./toolClassNames");
	for(StringList::const_iterator tcnIt=toolClassNames.begin();tcnIt!=toolClassNames.end();++tcnIt)
		{
		/* Load tool class: */
		loadClass(tcnIt->c_str());
		}
	
	/* Get factory for tool selection menu tools: */
	ToolFactory* toolSelectionMenuFactory=loadClass(configFileSection->retrieveString("./toolSelectionMenuToolClass").c_str());
	if(!toolSelectionMenuFactory->isDerivedFrom("MenuTool"))
		Misc::throwStdErr("ToolManager::loadDefaultTools: Tool selection menu tool class is not a menu tool class");
	const ToolInputLayout& menuToolLayout=toolSelectionMenuFactory->getLayout();
	if(menuToolLayout.getNumButtons()!=1||menuToolLayout.getNumValuators()!=0)
		Misc::throwStdErr("ToolManager::loadDefaultTools: Tool selection menu tool class has wrong input layout");
	
	/* Create the tool creation device, bypassing the input device manager and input graph manager: */
	toolCreationDevice=new InputDevice("ToolCreationDevice",InputDevice::TRACK_POS|InputDevice::TRACK_DIR|InputDevice::TRACK_ORIENT,1,0);
	
	/* Create a tool selection tool and attach it to the tool creation device: */
	ToolInputAssignment tia(toolSelectionMenuFactory->getLayout());
	tia.setButtonSlot(0,toolCreationDevice,0);
	Tool* tool=toolSelectionMenuFactory->createTool(tia);
	#if DEBUGGING
	std::cout<<"Creating tool selection tool "<<tool<<" of class "<<tool->getFactory()->getName()<<std::endl;
	#endif
	toolCreationTool=dynamic_cast<MenuTool*>(tool);
	if(toolCreationTool==0)
		{
		delete tool;
		Misc::throwStdErr("ToolManager::loadDefaultTools: Tool selection menu tool class is not a menu tool class");
		}
	
	/* Create the tool selection menu: */
	toolMenuPopup=createToolMenu();
	toolMenu=new MutexMenu(toolMenuPopup);
	
	/* Assign the tool selection menu to the tool creation tool: */
	toolCreationTool->setMenu(toolMenu);
	
	/* Register callbacks with the input device manager: */
	inputDeviceManager->getInputDeviceDestructionCallbacks().add(this,&ToolManager::inputDeviceDestructionCallback);
	
	/* Initialize the tool kill zone: */
	std::string killZoneType=configFileSection->retrieveString("./killZoneType");
	if(killZoneType=="Box")
		toolKillZone=new ToolKillZoneBox(*configFileSection);
	else if(killZoneType=="Frustum")
		toolKillZone=new ToolKillZoneFrustum(*configFileSection);
	else
		Misc::throwStdErr("ToolManager: Unknown kill zone type \"%s\"",killZoneType.c_str());
	}

ToolManager::~ToolManager(void)
	{
	/* Destroy the tool kill zone: */
	delete toolKillZone;
	
	/* Destroy a potentially dangling tool creation state: */
	if(toolCreationState!=0)
		{
		if(toolCreationState->toolSelectionDevice!=0)
			toolCreationState->toolSelectionDevice->getTrackingCallbacks().remove(this,&ToolManager::toolCreationDeviceMotionCallback);
		delete toolCreationState;
		}
	
	/* Delete the tool creation device and its tool selection tool: */
	delete toolCreationTool;
	delete toolCreationDevice;
	
	/* Delete tool menu: */
	delete toolMenu;
	delete toolMenuPopup;
	
	/* Unregister callbacks from the input device manager: */
	inputDeviceManager->getInputDeviceDestructionCallbacks().remove(this,&ToolManager::inputDeviceDestructionCallback);
	
	/* Delete all tools: */
	for(ToolList::iterator tIt=tools.begin();tIt!=tools.end();++tIt)
		{
		#if DEBUGGING
		std::cout<<"Deleting tool "<<*tIt<<" of class "<<(*tIt)->getFactory()->getName()<<std::endl;
		#endif
		
		/* De-initialize the tool: */
		(*tIt)->deinitialize();
		
		/* Call tool destruction callbacks: */
		ToolDestructionCallbackData cbData(*tIt);
		toolDestructionCallbacks.call(&cbData);
		
		/* Remove the tool from the input graph: */
		inputGraphManager->removeTool(*tIt);
		
		/* Delete the tool: */
		(*tIt)->getFactory()->destroyTool(*tIt);
		}
	
	/* Destroy the configuration file section: */
	delete configFileSection;
	}

void ToolManager::addClass(ToolFactory* newFactory,ToolManager::BaseClass::DestroyFactoryFunction newDestroyFactoryFunction)
	{
	/* Call the base class method to register the tool factory: */
	BaseClass::addClass(newFactory,newDestroyFactoryFunction);
	
	/* Add the new tool factory to the tool selection menu if the latter already exists: */
	if(toolMenuPopup!=0)
		{
		/* Extract the new tool factory's ancestor classes: */
		const ToolFactory* ancestor=newFactory;
		std::vector<const ToolFactory*> ancestors;
		while(true)
			{
			ancestor=!ancestor->getParents().empty()?dynamic_cast<const ToolFactory*>(ancestor->getParents().front()):0;
			if(ancestor==0)
				break;
			ancestors.push_back(ancestor);
			}
		
		/* Traverse the tool menu, adding cascade buttons for parent classes: */
		GLMotif::Container* menu=static_cast<GLMotif::Container*>(toolMenuPopup->getFirstChild());
		for(std::vector<const ToolFactory*>::reverse_iterator aIt=ancestors.rbegin();aIt!=ancestors.rend();++aIt)
			{
			GLMotif::Widget* ancestorWidget=menu->findChild((*aIt)->getClassName());
			GLMotif::CascadeButton* ancestorCascade=dynamic_cast<GLMotif::CascadeButton*>(ancestorWidget);
			if(ancestorCascade!=0)
				{
				/* Go to the ancestor's tool submenu: */
				menu=static_cast<GLMotif::Container*>(ancestorCascade->getPopup()->getFirstChild());
				}
			else if(ancestorWidget==0)
				{
				/* Create a new cascade button and tool submenu for the ancestor: */
				GLMotif::CascadeButton* ancestorCascade=new GLMotif::CascadeButton((*aIt)->getClassName(),menu,(*aIt)->getName());
				
				char popupName[256];
				snprintf(popupName,sizeof(popupName),"%sSubmenuPopup",(*aIt)->getClassName());
				GLMotif::Popup* ancestorPopup=new GLMotif::Popup(popupName,getWidgetManager());
				GLMotif::SubMenu* ancestorSubmenu=new GLMotif::SubMenu("ToolSubmenu",ancestorPopup,true);
				
				ancestorCascade->setPopup(ancestorPopup);
				
				menu=ancestorSubmenu;
				}
			else
				Misc::throwStdErr("Vrui::ToolManager::addClass: Base class name \"%s\" already exists as concrete class",(*aIt)->getClassName());
			}
		
		/* Create a button for the new tool factory: */
		GLMotif::Button* toolButton=new GLMotif::Button(newFactory->getClassName(),menu,newFactory->getName());
		toolButton->getSelectCallbacks().add(this,&ToolManager::toolMenuSelectionCallback);
		}
	}

void ToolManager::releaseClass(const char* className)
	{
	/* Get a pointer to the given class' factory object: */
	ToolFactory* factory=getFactory(className);
	
	/* Bail out if the class does not exist: */
	if(factory==0)
		return;
	
	/* Create a list of all tools of the given class: */
	std::vector<Tool*> destroyTools;
	for(ToolList::iterator tIt=tools.begin();tIt!=tools.end();++tIt)
		if((*tIt)->getFactory()==factory)
			destroyTools.push_back(*tIt);
	
	/* Destroy all tools in the list: */
	for(std::vector<Tool*>::iterator tIt=destroyTools.begin();tIt!=destroyTools.end();++tIt)
		{
		/* Destroy the tool: */
		destroyTool(*tIt);
		}
	
	/* Call the base class method to release the tool class: */
	BaseClass::releaseClass(className);
	}

void ToolManager::addAbstractClass(ToolFactory* newFactory,ToolManager::BaseClass::DestroyFactoryFunction newDestroyFactoryFunction)
	{
	/* Call the base class method to register the tool factory: */
	BaseClass::addClass(newFactory,newDestroyFactoryFunction);
	}

void ToolManager::defaultToolFactoryDestructor(ToolFactory* factory)
	{
	delete factory;
	}

Misc::ConfigurationFileSection ToolManager::getToolClassSection(const char* toolClassName) const
	{
	/* Return the section of the same name under the tool manager's section: */
	return configFileSection->getSection(toolClassName);
	}

void ToolManager::loadToolBinding(const char* toolSectionName)
	{
	/* Go to tool's section: */
	Misc::ConfigurationFileSection toolSection=configFileSection->getSection(toolSectionName);
	
	/* Get pointer to factory for tool's class: */
	ToolFactory* factory=loadClass(toolSection.retrieveString("./toolClass").c_str());
	const ToolInputLayout& layout=factory->getLayout();
	
	/* Read all feature assignments: */
	typedef std::vector<ToolAssignment> ToolAssignmentList;
	ToolAssignmentList bindings=toolSection.retrieveValue<ToolAssignmentList>("./bindings",ToolAssignmentList());
	
	/* Gather tool's input device assignments: */
	ToolInputAssignment tia(layout);
	
	/* Process all feature assignments: */
	int buttonSlotIndex=0;
	int valuatorSlotIndex=0;
	for(ToolAssignmentList::iterator bIt=bindings.begin();bIt!=bindings.end();++bIt)
		for(std::vector<int>::iterator fiIt=bIt->featureIndices.begin();fiIt!=bIt->featureIndices.end();++fiIt)
			{
			/* Find an unassigned forwarded feature for the current feature: */
			InputDeviceFeature openFeature=inputGraphManager->findFirstUnassignedFeature(InputDeviceFeature(bIt->device,*fiIt));
			
			/* Check if the forwarded feature is valid: */
			if(openFeature.isValid())
				{
				/* Check if the forwarded feature is a button or a valuator: */
				if(openFeature.isButton())
					{
					/* Assign the button: */
					if(buttonSlotIndex<layout.getNumButtons())
						tia.setButtonSlot(buttonSlotIndex,openFeature.getDevice(),openFeature.getIndex());
					else if(layout.hasOptionalButtons())
						tia.addButtonSlot(openFeature.getDevice(),openFeature.getIndex());
					++buttonSlotIndex;
					}
				if(openFeature.isValuator())
					{
					/* Assign the valuator: */
					if(valuatorSlotIndex<layout.getNumValuators())
						tia.setValuatorSlot(valuatorSlotIndex,openFeature.getDevice(),openFeature.getIndex());
					else if(layout.hasOptionalValuators())
						tia.addValuatorSlot(openFeature.getDevice(),openFeature.getIndex());
					++valuatorSlotIndex;
					}
				}
			else
				{
				std::string featureName=getInputDeviceManager()->getFeatureName(InputDeviceFeature(bIt->device,*fiIt));
				Misc::throwStdErr("ToolManager::loadToolBinding: Feature %s on input device %s is already assigned",featureName.c_str(),bIt->device->getDeviceName());
				}
			}
	
	/* Check if there are enough assignments: */
	if(buttonSlotIndex<layout.getNumButtons())
		Misc::throwStdErr("ToolManager::loadToolBinding: Not enough button bindings; got %d, need %d",buttonSlotIndex,layout.getNumButtons());
	if(valuatorSlotIndex<layout.getNumValuators())
		Misc::throwStdErr("ToolManager::loadToolBinding: Not enough valuator bindings; got %d, need %d",valuatorSlotIndex,layout.getNumValuators());
	
	/* Create tool of given class: */
	createTool(factory,tia,&toolSection);
	}

void ToolManager::loadDefaultTools(void)
	{
	/* Read the name of the section containing default tool bindings: */
	std::string defaultToolSectionName=configFileSection->retrieveValue<std::string>("./defaultTools");
	
	/* Read all tool bindings from the given section: */
	Misc::ConfigurationFileSection defaultToolSection=configFileSection->getSection(defaultToolSectionName.c_str());
	inputGraphManager->loadInputGraph(defaultToolSection);
	
	#if 0
	
	/* Configure initial tool assignment: */
	typedef std::vector<std::string> StringList;
	StringList toolNames=configFileSection->retrieveValue<StringList>("./toolNames");
	for(StringList::const_iterator tnIt=toolNames.begin();tnIt!=toolNames.end();++tnIt)
		{
		try
			{
			/* Load the tool binding: */
			loadToolBinding(tnIt->c_str());
			}
		catch(std::runtime_error err)
			{
			/* Print a warning message and carry on: */
			std::cout<<"ToolManager::loadDefaultTools: Ignoring tool binding "<<*tnIt<<" due to exception "<<err.what()<<std::endl;
			}
		}
	
	#endif
	}

void ToolManager::startToolCreation(const InputDeviceFeature& feature)
	{
	/* Create the tool creation state: */
	toolCreationState=new ToolManagerToolCreationState(*inputDeviceManager,feature);
	
	/* Find the root device of the given device: */
	InputDevice* rootDevice=inputGraphManager->getRootDevice(feature.getDevice());
	
	/* Remember the root device: */
	toolCreationState->toolSelectionDevice=rootDevice;
	
	/* Let the tool creation device shadow the root device: */
	rootDevice->getTrackingCallbacks().add(this,&ToolManager::toolCreationDeviceMotionCallback);
	toolCreationDevice->setTrackType(rootDevice->getTrackType());
	toolCreationDevice->setDeviceRay(rootDevice->getDeviceRayDirection(),rootDevice->getDeviceRayStart());
	toolCreationDevice->setTransformation(rootDevice->getTransformation());
	
	/* Press the tool creation tool's button, which will pop up the tool selection menu: */
	toolCreationDevice->setButtonState(0,true);
	}

void ToolManager::prepareFeatureAssignment(const InputDeviceFeature& feature)
	{
	/* Only prepare the given feature if no other feature is already prepared: */
	if(!toolCreationState->preparedFeature.isValid())
		{
		/* Prepare the given feature: */
		toolCreationState->preparedFeature=feature;
		}
	}

void ToolManager::assignFeature(const InputDeviceFeature& feature)
	{
	/* Only assign the given feature if it is the prepared one: */
	if(feature==toolCreationState->preparedFeature)
		{
		/* Check if this is the first feature assignment: */
		if(toolCreationState->buttonSlotIndex==0&&toolCreationState->valuatorSlotIndex==0)
			{
			/* Release the tool creation tool's button: */
			toolCreationDevice->setButtonState(0,false);
			
			/*****************************************************************
			Releasing the button will cause a menu selection callback if a
			tool class was selected, which will then assign the tool factory
			pointer. So if there is no tool factory pointer at this point, no
			tool class was selected and the whole thing was a bust.
			*****************************************************************/
			
			/* Remove the motion callback from the tool selection device: */
			toolCreationState->toolSelectionDevice->getTrackingCallbacks().remove(this,&ToolManager::toolCreationDeviceMotionCallback);
			toolCreationState->toolSelectionDevice=0;
			
			/* Check if a factory was selected: */
			if(toolCreationState->factory!=0)
				{
				/* Create the tool input assignment: */
				toolCreationState->tia=new ToolInputAssignment(toolCreationState->factory->getLayout());
				}
			else
				{
				/* Cancel tool creation: */
				delete toolCreationState;
				toolCreationState=0;
				return;
				}
			}
		
		/* Let the tool creation state do the actual work: */
		if(toolCreationState->assignFeature())
			{
			/* Check if tool creation was successful: */
			if(toolCreationState->isComplete())
				{
				/* Append a tool creation job to the tool management queue: */
				ToolManagementQueue::iterator tmqIt=toolManagementQueue.insert(toolManagementQueue.end(),ToolManagementQueueItem());
				tmqIt->itemFunction=ToolManagementQueueItem::CREATE_TOOL;
				tmqIt->createToolFactory=toolCreationState->factory;
				tmqIt->tia=toolCreationState->tia;
				toolCreationState->tia=0;
				tmqIt->tool=0;
				}
			
			/* Leave tool creation mode: */
			delete toolCreationState;
			toolCreationState=0;
			}
		else
			{
			/* Reset the prepared feature: */
			toolCreationState->preparedFeature=InputDeviceFeature();
			}
		}
	}

Tool* ToolManager::createTool(ToolFactory* factory,const ToolInputAssignment& tia,const Misc::ConfigurationFileSection* cfg)
	{
	/* Create tool of given class: */
	Tool* newTool=factory->createTool(tia);
	#if DEBUGGING
	std::cout<<"Created tool "<<newTool<<" of class "<<newTool->getFactory()->getName()<<std::endl;
	#endif
	int state=0;
	try
		{
		if(cfg!=0)
			{
			/* Let the tool configure itself: */
			newTool->configure(*cfg);
			}
		++state;
		
		/* Add tool to the input graph: */
		inputGraphManager->addTool(newTool);
		++state;
		
		/* Add tool to list: */
		tools.push_back(newTool);
		++state;
		
		/* Call tool creation callbacks: */
		ToolCreationCallbackData cbData(newTool,cfg);
		toolCreationCallbacks.call(&cbData);
		++state;
		
		/* Initialize the tool: */
		newTool->initialize();
		++state;
		
		/* Assign main menu to tool if it is a menu tool and has no menu yet: */
		MenuTool* menuTool=dynamic_cast<MenuTool*>(newTool);
		if(menuTool!=0&&menuTool->getMenu()==0)
			menuTool->setMenu(getMainMenu());
		}
	catch(...)
		{
		/* Unravel tool creation if shit got fucked up: */
		if(state>=5)
			{
			/* De-initialize the tool: */
			newTool->deinitialize();
			}
		if(state>=4)
			{
			/* Call tool destruction callbacks: */
			ToolDestructionCallbackData cbData(newTool);
			toolDestructionCallbacks.call(&cbData);
			}
		if(state>=3)
			{
			/* Remove tool from list: */
			for(ToolList::iterator tIt=tools.begin();tIt!=tools.end();++tIt)
				if(*tIt==newTool)
					{
					tools.erase(tIt);
					break;
					}
			}
		if(state>=2)
			{
			/* Remove tool from input graph: */
			inputGraphManager->removeTool(newTool);
			}
		
		/* Delete the tool: */
		delete newTool;
		
		/* Re-throw the exception: */
		throw;
		}
	
	return newTool;
	}

void ToolManager::destroyTool(Tool* tool,bool destroyImmediately)
	{
	if(destroyImmediately)
		{
		#if DEBUGGING
		std::cout<<"Deleting tool "<<tool<<" of class "<<tool->getFactory()->getName()<<std::endl;
		#endif
		
		/* De-initialize the tool: */
		tool->deinitialize();
		
		/* Call tool destruction callbacks: */
		ToolDestructionCallbackData cbData(tool);
		toolDestructionCallbacks.call(&cbData);
		
		/* Remove tool from list: */
		for(ToolList::iterator tIt=tools.begin();tIt!=tools.end();++tIt)
			if(*tIt==tool)
				{
				tools.erase(tIt);
				break;
				}
		
		/* Remove tool from input graph: */
		inputGraphManager->removeTool(tool);
		
		/* Destroy the tool: */
		tool->getFactory()->destroyTool(tool);
		}
	else
		{
		/* Append a tool destruction job to the tool management queue: */
		ToolManagementQueue::iterator tmqIt=toolManagementQueue.insert(toolManagementQueue.end(),ToolManagementQueueItem());
		tmqIt->itemFunction=ToolManagementQueueItem::DESTROY_TOOL;
		tmqIt->createToolFactory=0;
		tmqIt->tia=0;
		tmqIt->tool=tool;
		}
	}

void ToolManager::update(void)
	{
	/* Process the tool management queue: */
	for(ToolManagementQueue::iterator tmqIt=toolManagementQueue.begin();tmqIt!=toolManagementQueue.end();++tmqIt)
		{
		switch(tmqIt->itemFunction)
			{
			case ToolManagementQueueItem::CREATE_TOOL:
				{
				#if DEBUGGING
				std::cout<<"Creating new tool of class "<<tmqIt->createToolFactory->getName()<<std::endl;
				#endif
				
				/* Create the new tool: */
				createTool(tmqIt->createToolFactory,*tmqIt->tia);
				
				/* Clean up: */
				delete tmqIt->tia;
				break;
				}
			
			case ToolManagementQueueItem::DESTROY_TOOL:
				{
				if(tmqIt->tool!=0)
					{
					#if DEBUGGING
					std::cout<<"Destroying tool "<<tmqIt->tool<<" of class "<<tmqIt->tool->getFactory()->getName()<<std::endl;
					#endif
					
					/* Destroy the tool: */
					destroyTool(tmqIt->tool);
					}
				break;
				}
			}
		}
	
	/* Clear the tool management queue: */
	toolManagementQueue.clear();
	}

void ToolManager::glRenderAction(GLContextData& contextData) const
	{
	/* Check if the tool creation tool needs to be rendered: */
	if(toolCreationState!=0&&toolCreationState->factory==0)
		toolCreationTool->display(contextData);
	
	/* Render the tool kill zone: */
	toolKillZone->glRenderAction(contextData);
	}

bool ToolManager::isDeviceInToolKillZone(const InputDevice* device) const
	{
	return toolKillZone->isDeviceIn(device);
	}

}
