/***********************************************************************
InputGraphManager - Class to maintain the bipartite input device / tool
graph formed by tools being assigned to input devices, and input devices
in turn being grabbed by tools.
Copyright (c) 2004-2013 Oliver Kreylos

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

#include <Vrui/InputGraphManager.h>

#include <iostream>
#include <Misc/SelfDestructArray.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/PrintInteger.h>
#include <Misc/FileTests.h>
#include <Misc/StringHashFunctions.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/CompoundValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Misc/StringMarshaller.h>
#include <IO/Directory.h>
#include <Cluster/MulticastPipe.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Geometry/Ray.h>
#include <Geometry/GeometryValueCoders.h>
#include <GL/GLTransformationWrappers.h>
#include <SceneGraph/GroupNode.h>
#include <SceneGraph/TransformNode.h>
#include <SceneGraph/AppearanceNode.h>
#include <SceneGraph/MaterialNode.h>
#include <SceneGraph/FontStyleNode.h>
#include <SceneGraph/ShapeNode.h>
#include <SceneGraph/BoxNode.h>
#include <SceneGraph/CoordinateNode.h>
#include <SceneGraph/IndexedLineSetNode.h>
#include <SceneGraph/TextNode.h>
#include <Vrui/Vrui.h>
#include <Vrui/VirtualInputDevice.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/Viewer.h>
#include <Vrui/DeviceForwarder.h>
#include <Vrui/ToolInputAssignment.h>
#include <Vrui/Tool.h>
#include <Vrui/ToolManager.h>
#include <Vrui/SceneGraphSupport.h>
#include <Vrui/Internal/ToolKillZone.h>

namespace Vrui {

/*********************************************
Methods of class InputGraphManager::GraphTool:
*********************************************/

InputGraphManager::GraphTool::GraphTool(Tool* sTool,int sLevel)
	:tool(sTool),level(sLevel),
	 levelPred(0),levelSucc(0)
	{
	}

/********************************************
Methods of class InputGraphManager::ToolSlot:
********************************************/

InputGraphManager::ToolSlot::ToolSlot(void)
	:tool(0),
	 preempted(false),inKillZone(false)
	{
	}

InputGraphManager::ToolSlot::~ToolSlot(void)
	{
	if(feature.isValid())
		{
		/* Remove the appropriate callback from the device feature: */
		if(feature.isButton())
			feature.getCallbacks().remove(this,&ToolSlot::inputDeviceButtonCallback);
		if(feature.isValuator())
			feature.getCallbacks().remove(this,&ToolSlot::inputDeviceValuatorCallback);
		}
	}

void InputGraphManager::ToolSlot::initialize(InputDevice* sDevice,int sFeatureIndex)
	{
	/* Initialize the feature: */
	feature=InputDeviceFeature(sDevice,sFeatureIndex);
	
	/* Add the appropriate callback to the device feature: */
	if(feature.isButton())
		feature.getCallbacks().add(this,&ToolSlot::inputDeviceButtonCallback);
	if(feature.isValuator())
		feature.getCallbacks().add(this,&ToolSlot::inputDeviceValuatorCallback);
	}

void InputGraphManager::ToolSlot::inputDeviceButtonCallback(InputDevice::ButtonCallbackData* cbData)
	{
	bool interruptCallback=false;
	if(cbData->newButtonState) // Button has just been pressed
		interruptCallback=pressed();
	else // Button has just been released
		interruptCallback=released();
	if(interruptCallback)
		{
		/* Interrupt processing of this callback: */
		cbData->callbackList->requestInterrupt();
		}
	}

void InputGraphManager::ToolSlot::inputDeviceValuatorCallback(InputDevice::ValuatorCallbackData* cbData)
	{
	bool interruptCallback=false;
	if(Math::abs(cbData->oldValuatorValue)<=0.25&&Math::abs(cbData->newValuatorValue)>0.25) // Valuator has just been moved from the idle position
		interruptCallback=pressed();
	else if(Math::abs(cbData->oldValuatorValue)>0.1&&Math::abs(cbData->newValuatorValue)<=0.1) // Valuator has just returned to the idle position
		interruptCallback=released();
	else
		interruptCallback=preempted;
	if(interruptCallback)
		{
		/* Interrupt processing of this callback: */
		cbData->callbackList->requestInterrupt();
		}
	}

bool InputGraphManager::ToolSlot::pressed(void)
	{
	/* Get pointer to the tool manager: */
	ToolManager* tm=getToolManager();
	
	/* React depending on the tool manager's and this slot's state: */
	if(tm->isCreatingTool()) // Tool manager is in the middle of creating a new tool
		{
		/* Assign the slot if it doesn't have an assigned tool yet: */
		if(tool==0)
			{
			tm->prepareFeatureAssignment(feature);
			
			/* Preempt this event: */
			preempted=true;
			}
		}
	else if(tm->getToolKillZone()->isDeviceIn(feature.getDevice())) // Push event happened while inside tool kill zone
		{
		/* Remember that the device was inside the kill zone: */
		inKillZone=true;
		
		if(tool!=0)
			{
			/* Show the selected feature's tool stack: */
			getInputGraphManager()->showToolStack(feature);
			}
		
		/* Preempt this event: */
		preempted=true;
		}
	else if(tool==0) // Slot does not have an assigned tool yet
		{
		/* Start the tool manager's tool creation process: */
		tm->startToolCreation(feature);
		
		/* Preempt this event: */
		preempted=true;
		}
	
	return preempted;
	}

bool InputGraphManager::ToolSlot::released(void)
	{
	bool interruptCallback=false;
	
	/* Check if the corresponding push event was preempted: */
	if(preempted)
		{
		/* Get pointer to the tool manager: */
		ToolManager* tm=getToolManager();
		
		/* React depending on the tool manager's and this slot's state: */
		if(tm->isCreatingTool()) // Tool manager is in the middle of creating a new tool
			{
			/* Assign the slot if it doesn't have an assigned tool yet: */
			if(tool==0)
				tm->assignFeature(feature);
			}
		else if(inKillZone)
			{
			if(feature==getInputGraphManager()->toolStackBaseFeature)
				{
				/* Stop showing the feature's tool stack: */
				getInputGraphManager()->toolStackNode=0;
				}
			
			if(tool!=0&&tm->getToolKillZone()->isDeviceIn(feature.getDevice())) // Device is still in kill zone
				{
				/* Find the last tool in a chain of device forwarders: */
				ToolSlot* destroyTs=this;
				while(true)
					{
					/* Check if the tool is a device forwarder: */
					DeviceForwarder* df=dynamic_cast<DeviceForwarder*>(destroyTs->tool->tool);
					if(df==0)
						break;
					
					/* Get the list of forwarded features: */
					InputDeviceFeatureSet forwardedFeatures=df->getForwardedFeatures(destroyTs->feature);
					
					/* Follow the first forwarded feature that has a tool attached: */
					ToolSlot* nextDestroyTs=0;
					for(InputDeviceFeatureSet::iterator ffIt=forwardedFeatures.begin();nextDestroyTs==0&&ffIt!=forwardedFeatures.end();++ffIt)
						{
						ToolSlot& forwardedTs=getInputGraphManager()->deviceMap.getEntry(ffIt->getDevice()).getDest()->toolSlots[ffIt->getFeatureIndex()];
						if(forwardedTs.tool!=0)
							nextDestroyTs=&forwardedTs;
						}
					if(nextDestroyTs==0)
						break;
					destroyTs=nextDestroyTs;
					}
				
				/* Destroy the tool: */
				tm->destroyTool(destroyTs->tool->tool,false);
				}
			}
		
		/* Reset the kill zone flag: */
		inKillZone=false;
		
		/* Reset the preempted flag and interrupt processing of this callback: */
		preempted=false;
		interruptCallback=true;
		}
	
	return interruptCallback;
	}

/****************************************************
Methods of class InputGraphManager::GraphInputDevice:
****************************************************/

InputGraphManager::GraphInputDevice::GraphInputDevice(InputDevice* sDevice)
	:device(sDevice),
	 toolSlots(new ToolSlot[device->getNumFeatures()]),
	 level(0),
	 navigational(false),
	 levelPred(0),levelSucc(0),
	 grabber(0)
	{
	/* Initialize the new device's tool slots: */
	for(int featureIndex=0;featureIndex<device->getNumFeatures();++featureIndex)
		toolSlots[featureIndex].initialize(device,featureIndex);
	}

InputGraphManager::GraphInputDevice::~GraphInputDevice(void)
	{
	/* Delete the device's tool slots: */
	delete[] toolSlots;
	}

/**********************************
Methods of class InputGraphManager:
**********************************/

void InputGraphManager::linkInputDevice(InputGraphManager::GraphInputDevice* gid)
	{
	/* Link the graph input device to its graph level: */
	gid->levelPred=0;
	gid->levelSucc=deviceLevels[gid->level];
	if(deviceLevels[gid->level]!=0)
		deviceLevels[gid->level]->levelPred=gid;
	deviceLevels[gid->level]=gid;	
	}

void InputGraphManager::unlinkInputDevice(InputGraphManager::GraphInputDevice* gid)
	{
	/* Unlink the graph input device from its graph level: */
	if(gid->levelPred!=0)
		gid->levelPred->levelSucc=gid->levelSucc;
	else
		deviceLevels[gid->level]=gid->levelSucc;
	if(gid->levelSucc!=0)
		gid->levelSucc->levelPred=gid->levelPred;
	}

void InputGraphManager::linkTool(InputGraphManager::GraphTool* gt)
	{
	/* Link the graph tool to its graph level: */
	gt->levelPred=0;
	gt->levelSucc=toolLevels[gt->level];
	if(toolLevels[gt->level]!=0)
		toolLevels[gt->level]->levelPred=gt;
	toolLevels[gt->level]=gt;	
	}

void InputGraphManager::unlinkTool(InputGraphManager::GraphTool* gt)
	{
	/* Unlink the graph tool from its graph level: */
	if(gt->levelPred!=0)
		gt->levelPred->levelSucc=gt->levelSucc;
	else
		toolLevels[gt->level]=gt->levelSucc;
	if(gt->levelSucc!=0)
		gt->levelSucc->levelPred=gt->levelPred;
	}

int InputGraphManager::calcToolLevel(const Tool* tool) const
	{
	/* Get tool's input device assignment: */
	const ToolInputAssignment& tia=tool->getInputAssignment();
	
	/* Determine the maximal graph level of all input devices to which the tool is assigned: */
	int maxDeviceLevel=0;
	for(int slotIndex=0;slotIndex<tia.getNumSlots();++slotIndex)
		{
		/* Get pointer to graph input device: */
		GraphInputDevice* gid=deviceMap.getEntry(tia.getSlotDevice(slotIndex)).getDest();
		
		/* Check the device's level: */
		if(maxDeviceLevel<gid->level)
			maxDeviceLevel=gid->level;
		}
	
	/* Return the calculated graph level: */
	return maxDeviceLevel;
	}

void InputGraphManager::growInputGraph(int level)
	{
	/* Check whether the max graph level needs to be adjusted: */
	if(maxGraphLevel<level)
		{
		/* Set the new max graph level: */
		maxGraphLevel=level;
		
		/* Initialize the new levels in the input graph: */
		while(int(deviceLevels.size())<=maxGraphLevel)
			deviceLevels.push_back(0);
		while(int(toolLevels.size())<=maxGraphLevel)
			toolLevels.push_back(0);
		}
	}

void InputGraphManager::shrinkInputGraph(void)
	{
	/* Check whether there are empty levels at the end of the graph: */
	while(maxGraphLevel>=0&&deviceLevels[maxGraphLevel]==0&&toolLevels[maxGraphLevel]==0)
		--maxGraphLevel;
	}

void InputGraphManager::updateInputGraph(void)
	{
	/* Iterate through all graph levels and move all input devices and tools to their correct positions: */
	for(int level=0;level<=maxGraphLevel;++level)
		{
		/* Check all input devices: */
		GraphInputDevice* gid=deviceLevels[level];
		while(gid!=0)
			{
			GraphInputDevice* succ=gid->levelSucc;
			
			/* Ensure that ungrabbed devices are in level 0, and grabbed devices are exactly one level above their grabbers: */
			if(gid->grabber==0&&gid->level!=0)
				{
				/* Move the input device to level 0: */
				unlinkInputDevice(gid);
				gid->level=0;
				linkInputDevice(gid);
				}
			else if(gid->grabber!=0&&gid->level!=gid->grabber->level+1)
				{
				/* Move the input device to the level after its grabbing tool: */
				unlinkInputDevice(gid);
				gid->level=gid->grabber->level+1;
				growInputGraph(gid->level);
				linkInputDevice(gid);
				}
			
			gid=succ;
			}
		
		/* Check all tools: */
		GraphTool* gt=toolLevels[level];
		while(gt!=0)
			{
			GraphTool* succ=gt->levelSucc;
			
			/* Calculate the tool's correct graph level: */
			int toolLevel=calcToolLevel(gt->tool);
			
			/* Ensure that the tool is on the correct level: */
			if(gt->level!=toolLevel)
				{
				/* Move the tool to the correct level: */
				unlinkTool(gt);
				gt->level=toolLevel;
				linkTool(gt);
				}
			
			gt=succ;
			}
		}
	
	/* Shrink the input graph: */
	shrinkInputGraph();
	}

/* Helper class to represent tool stack creation state: */
struct InputGraphManagerToolStackState
	{
	/* Elements: */
	public:
	SceneGraph::Scalar size;
	SceneGraph::Size toolSize;
	SceneGraph::AppearanceNodePointer toolAppearance;
	SceneGraph::FontStyleNodePointer slotFont;
	SceneGraph::FontStyleNodePointer toolFont;
	SceneGraph::Size deviceSize;
	SceneGraph::AppearanceNodePointer deviceAppearance;
	};

SceneGraph::GraphNodePointer InputGraphManager::showToolStack(const ToolSlot& ts,InputGraphManagerToolStackState& tss) const
	{
	/* Create the result node: */
	SceneGraph::TransformNodePointer root(new SceneGraph::TransformNode);
	
	/**********************************************
	Draw a line from the origin to the slot's tool:
	**********************************************/
	
	SceneGraph::ShapeNodePointer slotShape(new SceneGraph::ShapeNode);
	slotShape->appearance.setValue(tss.toolAppearance);
	
	SceneGraph::IndexedLineSetNodePointer slotLine(new SceneGraph::IndexedLineSetNode);
	
	SceneGraph::CoordinateNodePointer slotCoord(new SceneGraph::CoordinateNode);
	slotCoord->point.appendValue(SceneGraph::Point(0,0,0));
	slotCoord->point.appendValue(SceneGraph::Point(0,tss.size,0));
	slotCoord->update();
	
	slotLine->coord.setValue(slotCoord);
	slotLine->coordIndex.appendValue(0);
	slotLine->coordIndex.appendValue(1);
	slotLine->colorPerVertex.setValue(false);
	slotLine->update();
	
	slotShape->geometry.setValue(slotLine);
	slotShape->update();
	
	root->children.appendValue(slotShape);
	
	/****************************************
	Label the line with the tool slot's name:
	****************************************/
	
	SceneGraph::TransformNodePointer slotLabelPos(new SceneGraph::TransformNode);
	slotLabelPos->translation.setValue(SceneGraph::Vector(-tss.size*SceneGraph::Scalar(0.5),tss.size*SceneGraph::Scalar(0.5),0));
	
	SceneGraph::ShapeNodePointer slotLabelShape(new SceneGraph::ShapeNode);
	slotLabelShape->appearance.setValue(tss.toolAppearance);
	
	SceneGraph::TextNodePointer slotLabel(new SceneGraph::TextNode);
	
	/* Find the tool slot feature's slot index on the tool: */
	const ToolInputAssignment& tia=ts.tool->tool->getInputAssignment();
	int slotIndex=tia.findFeature(ts.feature);
	
	/* Label the slot with the function description of the found button or valuator slot: */
	if(tia.isSlotButton(slotIndex))
		slotLabel->string.setValue(ts.tool->tool->getFactory()->getButtonFunction(tia.getButtonSlotIndex(slotIndex)));
	if(tia.isSlotValuator(slotIndex))
		slotLabel->string.setValue(ts.tool->tool->getFactory()->getValuatorFunction(tia.getValuatorSlotIndex(slotIndex)));
	
	slotLabel->fontStyle.setValue(tss.slotFont);
	slotLabel->update();
	
	slotLabelShape->geometry.setValue(slotLabel);
	slotLabelShape->update();
	
	slotLabelPos->children.appendValue(slotLabelShape);
	slotLabelPos->update();
	
	root->children.appendValue(slotLabelPos);
	
	/****************
	Draw a tool icon:
	****************/
	
	SceneGraph::ShapeNodePointer toolShape(new SceneGraph::ShapeNode);
	toolShape->appearance.setValue(tss.toolAppearance);
	
	SceneGraph::BoxNodePointer toolBox(new SceneGraph::BoxNode);
	toolBox->center.setValue(SceneGraph::Point(0,tss.size+tss.toolSize[1]*SceneGraph::Scalar(0.5),0));
	toolBox->size.setValue(tss.toolSize);
	toolBox->update();
	
	toolShape->geometry.setValue(toolBox);
	toolShape->update();
	
	root->children.appendValue(toolShape);
	
	/****************************************
	Label the tool icon with the tool's name:
	****************************************/
	
	SceneGraph::TransformNodePointer toolLabelPos(new SceneGraph::TransformNode);
	toolLabelPos->translation.setValue(SceneGraph::Vector(tss.toolSize[0]*SceneGraph::Scalar(0.5)+tss.size*SceneGraph::Scalar(0.5),tss.size+tss.toolSize[1]*SceneGraph::Scalar(0.5),0));
	
	/* Create a shape for the tool label: */
	SceneGraph::ShapeNodePointer toolLabelShape(new SceneGraph::ShapeNode);
	toolLabelShape->appearance.setValue(tss.toolAppearance);
	
	SceneGraph::TextNodePointer toolLabel(new SceneGraph::TextNode);
	toolLabel->string.setValue(ts.tool->tool->getName());
	toolLabel->fontStyle.setValue(tss.toolFont);
	toolLabel->update();
	
	toolLabelShape->geometry.setValue(toolLabel);
	toolLabelShape->update();
	
	toolLabelPos->children.appendValue(toolLabelShape);
	toolLabelPos->update();
	
	root->children.appendValue(toolLabelPos);
	
	/* Check if the tool is a device forwarder: */
	Vrui::DeviceForwarder* df=dynamic_cast<Vrui::DeviceForwarder*>(ts.tool->tool);
	if(df!=0)
		{
		/* Get the forwarded features: */
		InputDeviceFeatureSet forwardedFeatures=df->getForwardedFeatures(ts.feature);
		if(!forwardedFeatures.empty())
			{
			/***********************************************************
			Create tool stack visualizations for each forwarded feature:
			***********************************************************/
			
			std::vector<SceneGraph::TransformNodePointer> forwardedToolStacks;
			SceneGraph::Scalar totalWidth=SceneGraph::Scalar(0);
			for(InputDeviceFeatureSet::iterator ffIt=forwardedFeatures.begin();ffIt!=forwardedFeatures.end();++ffIt)
				{
				const ToolSlot& forwardedTs=deviceMap.getEntry(ffIt->getDevice()).getDest()->toolSlots[ffIt->getFeatureIndex()];
				if(forwardedTs.tool!=0)
					{
					SceneGraph::TransformNodePointer forwardedToolStack=showToolStack(forwardedTs,tss);
					forwardedToolStacks.push_back(forwardedToolStack);
					SceneGraph::Scalar width=SceneGraph::Scalar(0);
					if(forwardedToolStack!=0)
						width=forwardedToolStack->calcBoundingBox().getSize(0);
					if(width<tss.deviceSize[0])
						width=tss.deviceSize[0];
					totalWidth+=width;
					}
				else
					{
					forwardedToolStacks.push_back(0);
					totalWidth+=tss.deviceSize[0];
					}
				}
			totalWidth+=tss.size*SceneGraph::Scalar(forwardedFeatures.size()-1);
			
			/* Lay out the forwarded tool stacks: */
			SceneGraph::Scalar left=-totalWidth*SceneGraph::Scalar(0.5);
			for(std::vector<SceneGraph::TransformNodePointer>::iterator ftsIt=forwardedToolStacks.begin();ftsIt!=forwardedToolStacks.end();++ftsIt)
				{
				/* Calculate the forwarded tool stack's total width and center position: */
				SceneGraph::Scalar width;
				SceneGraph::Scalar center;
				if(*ftsIt!=0)
					{
					SceneGraph::Box box=(*ftsIt)->calcBoundingBox();
					width=box.getSize(0);
					if(width<tss.deviceSize[0])
						{
						width=tss.deviceSize[0];
						center=left+width*SceneGraph::Scalar(0.5);
						}
					else
						center=left-box.min[0];
					}
				else
					{
					width=tss.deviceSize[0];
					center=left+width*SceneGraph::Scalar(0.5);
					}
				
				/******************
				Draw a device icon:
				******************/
				
				SceneGraph::ShapeNodePointer deviceShape(new SceneGraph::ShapeNode);
				deviceShape->appearance.setValue(tss.deviceAppearance);
				
				SceneGraph::BoxNodePointer deviceBox(new SceneGraph::BoxNode);
				deviceBox->center.setValue(SceneGraph::Point(center,tss.size*SceneGraph::Scalar(2)+tss.toolSize[1]+tss.deviceSize[1]*SceneGraph::Scalar(0.5),0));
				deviceBox->size.setValue(tss.deviceSize);
				deviceBox->update();
				
				deviceShape->geometry.setValue(deviceBox);
				deviceShape->update();
				
				root->children.appendValue(deviceShape);
				
				if(*ftsIt!=0)
					{
					/* Position the forwarded tool stack: */
					(*ftsIt)->translation.setValue(SceneGraph::Vector(center,tss.size*SceneGraph::Scalar(2)+tss.toolSize[1]+tss.deviceSize[1],0));
					(*ftsIt)->update();
					
					root->children.appendValue(*ftsIt);
					}
				
				left+=width+tss.size;
				}
			}
		}
	
	root->update();
	return root;
	}

InputGraphManager::InputGraphManager(GlyphRenderer* sGlyphRenderer,VirtualInputDevice* sVirtualInputDevice)
	:glyphRenderer(sGlyphRenderer),
	 virtualInputDevice(sVirtualInputDevice),
	 inputDeviceManager(0,-1),
	 deviceMap(17),
	 toolMap(17),
	 maxGraphLevel(-1)
	{
	}

InputGraphManager::~InputGraphManager(void)
	{
	/* Delete all graph input devices and tools: */
	for(int i=0;i<=maxGraphLevel;++i)
		{
		/* Delete all graph input devices: */
		GraphInputDevice* gid=deviceLevels[i];
		while(gid!=0)
			{
			GraphInputDevice* succ=gid->levelSucc;
			delete gid;
			gid=succ;
			}
		
		/* Delete all graph tools: */
		GraphTool* gt=toolLevels[i];
		while(gt!=0)
			{
			GraphTool* succ=gt->levelSucc;
			delete gt;
			gt=succ;
			}
		}
	}

void InputGraphManager::addInputDevice(InputDevice* newDevice)
	{
	/* Disable all callbacks for the device: */
	newDevice->disableCallbacks();
	
	/* Add the new device to level 0 of the input graph: */
	GraphInputDevice* newGid=new GraphInputDevice(newDevice);
	growInputGraph(0);
	linkInputDevice(newGid);
	
	/* Add the new graph device to the graph device map: */
	deviceMap.setEntry(DeviceMap::Entry(newDevice,newGid));
	}

void InputGraphManager::removeInputDevice(InputDevice* device)
	{
	/* Find the device's entry in the device map: */
	DeviceMap::Iterator dIt=deviceMap.findEntry(device);
	
	/* Get pointer to the graph input device: */
	GraphInputDevice* gid=dIt->getDest();
	
	/* Gather all graph tools assigned to the input device: */
	Misc::HashTable<Tool*,void> destroyTools(17);
	for(int featureIndex=0;featureIndex<gid->device->getNumFeatures();++featureIndex)
		if(gid->toolSlots[featureIndex].tool!=0)
			destroyTools.setEntry(Misc::HashTable<Tool*,void>::Entry(gid->toolSlots[featureIndex].tool->tool));
	
	/* Get a pointer to the tool manager: */
	ToolManager* tm=getToolManager();
	
	/* Ask the tool manager to destroy all gathered tools: */
	for(Misc::HashTable<Tool*,void>::Iterator dtIt=destroyTools.begin();!dtIt.isFinished();++dtIt)
		tm->destroyTool(dtIt->getSource());
	
	/* Remove the graph input device from its graph level and from the graph device map: */
	unlinkInputDevice(gid);
	deviceMap.removeEntry(dIt);
	
	/* Delete the graph input device: */
	delete gid;
	
	/* Shrink the input graph: */
	shrinkInputGraph();
	}

void InputGraphManager::addTool(Tool* newTool)
	{
	/* Get tool's input device assignment: */
	const ToolInputAssignment& tia=newTool->getInputAssignment();
	
	/* Check if the new tool can be added to the input graph, and at which level: */
	int maxDeviceLevel=0;
	for(int slotIndex=0;slotIndex<tia.getNumSlots();++slotIndex)
		{
		/* Get pointer to assigned graph input device: */
		GraphInputDevice* gid=deviceMap.getEntry(tia.getSlotDevice(slotIndex)).getDest();
		
		/* Check the device's level: */
		if(maxDeviceLevel<gid->level)
			maxDeviceLevel=gid->level;
		
		/* Check if the assigned feature is still available: */
		int featureIndex=tia.getSlotFeatureIndex(slotIndex);
		ToolSlot& ts=gid->toolSlots[featureIndex];
		if(ts.tool!=0)
			Misc::throwStdErr("InputGraphManager::addTool: Cannot add tool because %s %d on input device %s is already assigned",ts.feature.isButton()?"button":"valuator",ts.feature.getIndex(),gid->device->getDeviceName());
		}
	
	/* Add the new tool to the correct graph level and to the graph tool map: */
	GraphTool* newGt=new GraphTool(newTool,maxDeviceLevel);
	linkTool(newGt);
	toolMap.setEntry(ToolMap::Entry(newTool,newGt));
	
	/* Assign the new tool to all its assigned input device features: */
	for(int slotIndex=0;slotIndex<tia.getNumSlots();++slotIndex)
		{
		/* Assign the tool to its feature: */
		GraphInputDevice* gid=deviceMap.getEntry(tia.getSlotDevice(slotIndex)).getDest();
		gid->toolSlots[tia.getSlotFeatureIndex(slotIndex)].tool=newGt;
		}
	}

void InputGraphManager::removeTool(Tool* tool)
	{
	/* Find the tool's entry in the tool map: */
	ToolMap::Iterator tIt=toolMap.findEntry(tool);
	
	/* Get pointer to the graph tool: */
	GraphTool* gt=tIt->getDest();
	
	/* Get tool's input device assignment: */
	const ToolInputAssignment& tia=tool->getInputAssignment();
	
	/* Unassign the graph tool from its assigned input device features: */
	for(int slotIndex=0;slotIndex<tia.getNumSlots();++slotIndex)
		{
		/* Unassign the tool from its feature: */
		GraphInputDevice* gid=deviceMap.getEntry(tia.getSlotDevice(slotIndex)).getDest();
		gid->toolSlots[tia.getSlotFeatureIndex(slotIndex)].tool=0;
		}
	
	/* Remove the graph tool from its graph level and from the graph tool map: */
	unlinkTool(gt);
	toolMap.removeEntry(tIt);
	
	/* Delete the graph tool: */
	delete gt;
	}

void InputGraphManager::clear(void)
	{
	/* Repeat virtual input devices or tools until there are no more: */
	while(maxGraphLevel>=0)
		{
		/* Find the first virtual input device on graph level 0: */
		InputDevice* device=0;
		for(GraphInputDevice* gidPtr=deviceLevels[0];gidPtr!=0;gidPtr=gidPtr->levelSucc)
			if(gidPtr->grabber==0)
				{
				device=gidPtr->device;
				break;
				}
		
		if(device!=0)
			{
			/* Delete the device: */
			getInputDeviceManager()->destroyInputDevice(device);
			}
		else if(toolLevels[0]!=0)
			{
			/* Delete the first tool on graph level 0: */
			getToolManager()->destroyTool(toolLevels[0]->tool);
			}
		else
			{
			/* We're clear: */
			break;
			}
		}
	}

void InputGraphManager::loadInputGraph(const Misc::ConfigurationFileSection& baseSection)
	{
	/* Create a hash table to map section names to the forwarded or virtual input devices created by them: */
	typedef Misc::HashTable<std::string,InputDevice*> CreatedDeviceMap;
	CreatedDeviceMap createdDeviceMap(17);
	
	/* Process all subsections of the base section: */
	for(Misc::ConfigurationFileSection sIt=baseSection.beginSubsections();sIt!=baseSection.endSubsections();++sIt)
		{
		/* Determine the section's type: */
		if(sIt.hasTag("./toolClass"))
			{
			/*******************
			Read a tool section:
			*******************/
			
			try
				{
				/* Read the tool's class name: */
				std::string toolClass=sIt.retrieveValue<std::string>("./toolClass");
				
				/* Get the tool class' factory object: */
				ToolFactory* factory=getToolManager()->loadClass(toolClass.c_str());
				
				/* Get the tool's input layout and create an empty assignment: */
				ToolInputAssignment tia(factory->getLayout());
				int buttonSlotIndex=0;
				int valuatorSlotIndex=0;
				
				/* Read and process the tool's bindings: */
				std::vector<std::vector<std::string> > bindings=sIt.retrieveValue<std::vector<std::vector<std::string> > >("./bindings");
				for(std::vector<std::vector<std::string> >::iterator bsIt=bindings.begin();bsIt!=bindings.end();++bsIt)
					if(!bsIt->empty()) // Empty binding is nonsense, but not illegal
						{
						/* Get the current binding's input device name: */
						std::vector<std::string>::iterator bIt=bsIt->begin();
						
						/* Get the device pointer by first checking the map of created devices and then all existing devices: */
						InputDevice* device=0;
						CreatedDeviceMap::Iterator cdmIt=createdDeviceMap.findEntry(*bIt);
						if(!cdmIt.isFinished())
							{
							/* Get the previously created device: */
							device=cdmIt->getDest();
							}
						else
							{
							/* Find the device among the already-existing devices: */
							device=getInputDeviceManager()->findInputDevice(bIt->c_str());
							if(device==0)
								Misc::throwStdErr("Unknown device %s",bIt->c_str());
							}
						
						/* Process the current binding's features: */
						for(++bIt;bIt!=bsIt->end();++bIt)
							{
							/* Get the feature's index on the input device: */
							int featureIndex=getInputDeviceManager()->getFeatureIndex(device,bIt->c_str());
							if(featureIndex==-1)
								Misc::throwStdErr("Unknown feature %s on device %s",bIt->c_str(),bsIt->front().c_str());
							
							/* Find an unassigned forwarded feature for the current feature: */
							InputDeviceFeature openFeature=findFirstUnassignedFeature(InputDeviceFeature(device,featureIndex));
							
							/* Check if the forwarded feature is valid: */
							if(openFeature.isValid())
								{
								/* Check if the forwarded feature is a button or a valuator: */
								if(openFeature.isButton())
									{
									/* Assign the button: */
									if(buttonSlotIndex<factory->getLayout().getNumButtons())
										tia.setButtonSlot(buttonSlotIndex,openFeature.getDevice(),openFeature.getIndex());
									else if(factory->getLayout().hasOptionalButtons())
										tia.addButtonSlot(openFeature.getDevice(),openFeature.getIndex());
									++buttonSlotIndex;
									}
								if(openFeature.isValuator())
									{
									/* Assign the valuator: */
									if(valuatorSlotIndex<factory->getLayout().getNumValuators())
										tia.setValuatorSlot(valuatorSlotIndex,openFeature.getDevice(),openFeature.getIndex());
									else if(factory->getLayout().hasOptionalValuators())
										tia.addValuatorSlot(openFeature.getDevice(),openFeature.getIndex());
									++valuatorSlotIndex;
									}
								}
							else
								{
								std::string featureName=getInputDeviceManager()->getFeatureName(InputDeviceFeature(device,featureIndex));
								Misc::throwStdErr("Feature %s on device %s is already assigned",featureName.c_str(),bsIt->front().c_str());
								}
							}
						}
				
				/* Check if the binding is complete: */
				if(buttonSlotIndex<factory->getLayout().getNumButtons())
					Misc::throwStdErr("Not enough button bindings; got %d, need %d",buttonSlotIndex,factory->getLayout().getNumButtons());
				if(valuatorSlotIndex<factory->getLayout().getNumValuators())
					Misc::throwStdErr("Not enough valuator bindings; got %d, need %d",valuatorSlotIndex,factory->getLayout().getNumValuators());
				
				/* Create a tool of the given class and input assignment: */
				Tool* newTool=getToolManager()->createTool(factory,tia,&sIt);
				
				/* Check if the tool has forwarded devices: */
				DeviceForwarder* df=dynamic_cast<DeviceForwarder*>(newTool);
				if(df!=0)
					{
					/* Add each forwarded device to the created device map: */
					std::vector<InputDevice*> forwardedDevices=df->getForwardedDevices();
					if(forwardedDevices.size()==1)
						{
						/* Add the only forwarded device: */
						createdDeviceMap[sIt.getName()]=forwardedDevices[0];
						}
					else
						{
						/* Add each of the forwarded devices, appending its index: */
						for(unsigned int index=0;index<forwardedDevices.size();++index)
							{
							char indexString[11];
							std::string forwardedDeviceName=sIt.getName();
							forwardedDeviceName.append(Misc::print(index,indexString+10));
							createdDeviceMap[forwardedDeviceName]=forwardedDevices[index];
							}
						}
					}
				}
			catch(std::runtime_error err)
				{
				/* Print error message and carry on: */
				std::cout<<"InputGraphManager::loadInputGraph: Ignoring tool binding section "<<sIt.getName()<<" due to exception "<<err.what()<<std::endl;
				}
			}
		else
			{
			/*********************
			Read a device section:
			*********************/
			
			/* Read the device's layout and create a new virtual device: */
			int numButtons=sIt.retrieveValue<int>("./numButtons",0);
			int numValuators=sIt.retrieveValue<int>("./numValuators",0);
			InputDevice* newDevice=addVirtualInputDevice("VirtualInputDevice",numButtons,numValuators);
			
			/* Get the graph input device representing the new device: */
			GraphInputDevice* gidPtr=deviceMap.getEntry(newDevice).getDest();
			
			/* Read the device's navigational flag: */
			if(sIt.retrieveValue<bool>("./navigational",false))
				{
				/* Flag the device as navigational: */
				gidPtr->navigational=true;
				
				/* Read the device's position and orientation in navigational space: */
				TrackerState navPos=TrackerState::translateFromOriginTo(getInverseNavigationTransformation().transform(getDisplayCenter()));
				gidPtr->fromNavTransform=sIt.retrieveValue<TrackerState>("./transform",navPos);
				
				/* Set the device's initial phyical-space position: */
				NavTrackerState physPos=getNavigationTransformation();
				physPos*=gidPtr->fromNavTransform;
				physPos.renormalize();
				newDevice->setTransformation(TrackerState(physPos.getTranslation(),physPos.getRotation()));
				}
			else
				{
				/* Read the device's position and orientation in physical space: */
				TrackerState physPos=TrackerState::translateFromOriginTo(getDisplayCenter());
				newDevice->setTransformation(sIt.retrieveValue<TrackerState>("./transform",physPos));
				}
			
			/* Add the name of the created device to the device map: */
			createdDeviceMap[sIt.getName()]=newDevice;
			}
		}
	}

void InputGraphManager::loadInputGraph(IO::Directory& directory,const char* configurationFileName,const char* baseSectionName)
	{
	Misc::ConfigurationFile cfgFile;
	if(isMaster())
		{
		try
			{
			/* Load the configuration file: */
			cfgFile.load(directory.getPath(configurationFileName).c_str());
			
			if(getMainPipe()!=0)
				{
				/* Forward the configuration file to the slaves: */
				Misc::writeCString(0,*getMainPipe());
				cfgFile.writeToPipe(*getMainPipe());
				getMainPipe()->flush();
				}
			}
		catch(std::runtime_error err)
			{
			if(getMainPipe()!=0)
				{
				/* Forward the error message to the slaves: */
				Misc::writeCString(err.what(),*getMainPipe());
				getMainPipe()->flush();
				}
			
			/* Re-throw the exception: */
			throw;
			}
		}
	else
		{
		/* Check if there was an error on the master: */
		Misc::SelfDestructArray<char> error(Misc::readCString(*getMainPipe()));
		if(error.getArray()==0)
			{
			/* Receive a configuration file from the master: */
			cfgFile.readFromPipe(*getMainPipe());
			}
		else
			{
			/* Throw an exception: */
			throw std::runtime_error(error.getArray());
			}
		}
	
	/* Navigate to the base section: */
	Misc::ConfigurationFileSection baseSection=cfgFile.getSection(baseSectionName);
	
	/* Read the input graph stored in the base section: */
	loadInputGraph(baseSection);
	}

void InputGraphManager::saveInputGraph(IO::Directory& directory,const char* configurationFileName,const char* baseSectionName) const
	{
	if(isMaster())
		{
		try
			{
			/* Create a new configuration file: */
			Misc::ConfigurationFile cfgFile;
			
			/* Merge in contents of the given configuration file, if it exists: */
			std::string configurationFilePath=directory.getPath(configurationFileName);
			if(Misc::isFileReadable(configurationFilePath.c_str()))
				cfgFile.merge(configurationFilePath.c_str());
			
			/* Navigate to the base section and clear it: */
			Misc::ConfigurationFileSection baseSection=cfgFile.getSection(baseSectionName);
			baseSection.clear();
			
			/* Create a hash table to map device pointers to temporary device names for disambiguation: */
			typedef Misc::HashTable<InputDevice*,std::string> DeviceNameMap;
			DeviceNameMap deviceNameMap(17);
			int virtualDeviceIndex=0;
			int toolIndex=0;
			
			/* Save all virtual input devices and all tools from all input graph levels: */
			for(int level=0;level<=maxGraphLevel;++level)
				{
				/* Check if there are any unrepresented non-physical devices in this input graph level: */
				for(const GraphInputDevice* gidPtr=deviceLevels[level];gidPtr!=0;gidPtr=gidPtr->levelSucc)
					if(gidPtr->grabber!=&inputDeviceManager&&!deviceNameMap.isEntry(gidPtr->device))
						{
						/* Create a new section for the virtual input device: */
						std::string deviceSectionName="Device";
						char deviceIndexString[11];
						deviceSectionName.append(Misc::print(virtualDeviceIndex,deviceIndexString+10));
						++virtualDeviceIndex;
						Misc::ConfigurationFileSection deviceSection=baseSection.getSection(deviceSectionName.c_str());
						
						/* Write the virtual input device's layout: */
						deviceSection.storeValue<int>("./numButtons",gidPtr->device->getNumButtons());
						deviceSection.storeValue<int>("./numValuators",gidPtr->device->getNumValuators());
						
						/* Write the virtual input device's navigation flag: */
						deviceSection.storeValue<bool>("./navigational",gidPtr->navigational);
						
						/* Write the virtual input device's position and orientation: */
						if(gidPtr->navigational)
							{
							/* Write the navigational-space position and orientation: */
							TrackerState navPos(gidPtr->fromNavTransform.getTranslation(),gidPtr->fromNavTransform.getRotation());
							deviceSection.storeValue<TrackerState>("./transform",navPos);
							}
						else
							{
							/* Write the physical-space position and orientation: */
							deviceSection.storeValue<TrackerState>("./transform",gidPtr->device->getTransformation());
							}
						
						/* Add the virtual input device to the device name mapper: */
						deviceNameMap[gidPtr->device]=deviceSectionName;
						}
				
				/* Save all tools in this level: */
				for(GraphTool* gtPtr=toolLevels[level];gtPtr!=0;gtPtr=gtPtr->levelSucc)
					{
					/* Create a new section for the tool: */
					std::string toolSectionName="Tool";
					char toolIndexString[11];
					toolSectionName.append(Misc::print(toolIndex,toolIndexString+10));
					++toolIndex;
					Misc::ConfigurationFileSection toolSection=baseSection.getSection(toolSectionName.c_str());
					
					/* Write the tool's class name: */
					toolSection.storeValue<std::string>("./toolClass",gtPtr->tool->getFactory()->getClassName());
					
					/* Write the tool's feature bindings: */
					std::string bindings="((";
					const ToolInputAssignment& tia=gtPtr->tool->getInputAssignment();
					
					/* Initialize the current device to collate features by device: */
					InputDevice* currentDevice=tia.getSlotDevice(0);
					
					/* Get a (mapped) name for the current device: */
					DeviceNameMap::Iterator dnmIt=deviceNameMap.findEntry(currentDevice);
					if(dnmIt.isFinished())
						bindings.append(currentDevice->getDeviceName());
					else
						bindings.append(dnmIt->getDest());
					
					for(int i=0;i<tia.getNumSlots();++i)
						{
						/* Check for a device change: */
						if(currentDevice!=tia.getSlotDevice(i))
							{
							/* Close the current per-device feature list: */
							bindings.append("), (");
							
							/* Start a new per-device feature list: */
							currentDevice=tia.getSlotDevice(i);
							
							/* Get a (mapped) name for the current device: */
							DeviceNameMap::Iterator dnmIt=deviceNameMap.findEntry(currentDevice);
							if(dnmIt.isFinished())
								bindings.append(Misc::ValueCoder<std::string>::encode(currentDevice->getDeviceName()));
							else
								bindings.append(Misc::ValueCoder<std::string>::encode(dnmIt->getDest()));
							}
						
						/* Add the bound feature name: */
						bindings.append(", ");
						bindings.append(Misc::ValueCoder<std::string>::encode(getInputDeviceManager()->getFeatureName(tia.getSlotFeature(i))));
						}
					bindings.append("))");
					toolSection.storeString("bindings",bindings);
					
					/* Let the tool store its settings: */
					gtPtr->tool->storeState(toolSection);
					
					/* Check if the tool has forwarded devices: */
					DeviceForwarder* df=dynamic_cast<DeviceForwarder*>(gtPtr->tool);
					if(df!=0)
						{
						/* Create a mapped name for each forwarded device: */
						std::vector<InputDevice*> forwardedDevices=df->getForwardedDevices();
						if(forwardedDevices.size()==1)
							{
							/* Add an entry for the single forwarded device: */
							deviceNameMap[forwardedDevices[0]]=toolSectionName;
							}
						else
							{
							/* Add an entry for each forwarded device, appending their indices: */
							for(unsigned int index=0;index<forwardedDevices.size();++index)
								{
								std::string forwardedDeviceName=toolSectionName;
								char indexString[11];
								forwardedDeviceName.append(Misc::print(index,indexString+10));
								deviceNameMap[forwardedDevices[0]]=forwardedDeviceName;
								}
							}
						}
					}
				}
			
			/* Save the configuration file: */
			cfgFile.saveAs(configurationFilePath.c_str());
			
			if(getMainPipe()!=0)
				{
				/* Send a success flag to the slaves: */
				Misc::writeCString(0,*getMainPipe());
				getMainPipe()->flush();
				}
			}
		catch(std::runtime_error err)
			{
			if(getMainPipe()!=0)
				{
				/* Send an error message to the slaves: */
				Misc::writeCString(err.what(),*getMainPipe());
				getMainPipe()->flush();
				}
			
			/* Re-throw the exception: */
			throw;
			}
		}
	else
		{
		/* Check if there was an error on the master: */
		Misc::SelfDestructArray<char> error(Misc::readCString(*getMainPipe()));
		if(error.getArray()!=0)
			{
			/* Throw an exception: */
			throw std::runtime_error(error.getArray());
			}
		}
	}

bool InputGraphManager::isNavigational(InputDevice* device) const
	{
	/* Get pointer to the graph input device: */
	const GraphInputDevice* gid=deviceMap.getEntry(device).getDest();
	
	/* Return the device's navigation flag: */
	return gid->navigational;
	}

void InputGraphManager::setNavigational(InputDevice* device,bool newNavigational)
	{
	/* Get pointer to the graph input device: */
	GraphInputDevice* gid=deviceMap.getEntry(device).getDest();
	
	if(newNavigational)
		{
		/* Calculate the transformation from navigation coordinates to the device's current coordinates: */
		gid->fromNavTransform=device->getTransformation();
		gid->fromNavTransform.leftMultiply(getInverseNavigationTransformation());
		}
	
	/* Set the device's navigation flag: */
	gid->navigational=newNavigational;
	}

Glyph& InputGraphManager::getInputDeviceGlyph(InputDevice* device)
	{
	/* Get pointer to the graph input device: */
	GraphInputDevice* gid=deviceMap.getEntry(device).getDest();
	
	/* Return the device's glyph: */
	return gid->deviceGlyph;
	}

bool InputGraphManager::isReal(InputDevice* device) const
	{
	/* Get pointer to the graph input device: */
	const GraphInputDevice* gid=deviceMap.getEntry(device).getDest();
	
	/* Return true if the device is in graph level 0: */
	return gid->level==0;
	}

bool InputGraphManager::isGrabbed(InputDevice* device) const
	{
	/* Get pointer to the graph input device: */
	const GraphInputDevice* gid=deviceMap.getEntry(device).getDest();
	
	/* Return true if the device is grabbed: */
	return gid->grabber!=0;
	}

InputDevice* InputGraphManager::getFirstInputDevice(void)
	{
	/* Search for the first ungrabbed device in graph level 0: */
	InputDevice* result=0;
	for(GraphInputDevice* gid=deviceLevels[0];gid!=0;gid=gid->levelSucc)
		if(gid->grabber==0)
			{
			result=gid->device;
			break;
			}
	return result;
	}

InputDevice* InputGraphManager::getNextInputDevice(InputDevice* device)
	{
	/* Bail out if the device pointer is invalid: */
	if(device==0)
		return 0;
	
	/* Get the graph input device corresponding to the given device: */
	GraphInputDevice* gid=deviceMap.getEntry(device).getDest();
	
	/* Search for the next ungrabbed device: */
	InputDevice* result=0;
	for(gid=gid->levelSucc;gid!=0;gid=gid->levelSucc)
		if(gid->grabber==0)
			{
			result=gid->device;
			break;
			}
	return result;
	}

InputDevice* InputGraphManager::findInputDevice(const Point& position,bool ungrabbedOnly)
	{
	InputDevice* result=0;
	Scalar gs=Scalar(0.575)*glyphRenderer->getGlyphSize();
	int maxSearchLevel=ungrabbedOnly?0:maxGraphLevel;
	
	/* Check all input devices in all relevant graph levels: */
	for(int level=0;level<=maxSearchLevel;++level)
		for(GraphInputDevice* gid=deviceLevels[level];gid!=0;gid=gid->levelSucc)
			if(gid->grabber==0)
				{
				if(virtualInputDevice->pick(gid->device,position))
					{
					/* Remember the device and stop searching: */
					result=gid->device;
					goto foundIt;
					}
				}
			else if(!ungrabbedOnly)
				{
				/* Check if the given position is inside the input device's glyph: */
				Point dp=gid->device->getTransformation().inverseTransform(position);
				bool inside=true;
				for(int i=0;i<3&&inside;++i)
					inside=Math::abs(dp[i])<=gs;
				
				if(inside)
					{
					/* Remember the device and stop searching: */
					result=gid->device;
					goto foundIt;
					}
				}
	foundIt:
	
	return result;
	}

InputDevice* InputGraphManager::findInputDevice(const Ray& ray,bool ungrabbedOnly)
	{
	InputDevice* result=0;
	Scalar gs=Scalar(0.575)*glyphRenderer->getGlyphSize();
	int maxSearchLevel=ungrabbedOnly?0:maxGraphLevel;
	Scalar lambdaMin=Math::Constants<Scalar>::max;
	
	/* Check all input devices in all relevant graph levels: */
	for(int level=0;level<=maxSearchLevel;++level)
		for(GraphInputDevice* gid=deviceLevels[level];gid!=0;gid=gid->levelSucc)
			if(gid->grabber==0)
				{
				Scalar lambda=virtualInputDevice->pick(gid->device,ray);
				if(lambdaMin>lambda)
					{
					result=gid->device;
					lambdaMin=lambda;
					}
				}
			else if(!ungrabbedOnly)
				{
				Ray r=ray;
				r.inverseTransform(gid->device->getTransformation());
				
				Scalar lMin=Scalar(0);
				Scalar lMax=Math::Constants<Scalar>::max;
				for(int i=0;i<3;++i)
					{
					Scalar l1,l2;
					if(r.getDirection()[i]<Scalar(0))
						{
						l1=(gs-r.getOrigin()[i])/r.getDirection()[i];
						l2=(-gs-r.getOrigin()[i])/r.getDirection()[i];
						}
					else if(r.getDirection()[i]>Scalar(0))
						{
						l1=(-gs-r.getOrigin()[i])/r.getDirection()[i];
						l2=(gs-r.getOrigin()[i])/r.getDirection()[i];
						}
					else if(-gs<=r.getOrigin()[i]&&r.getOrigin()[i]<gs)
						{
						l1=Scalar(0);
						l2=Math::Constants<Scalar>::max;
						}
					else
						l1=l2=Scalar(-1);
					if(lMin<l1)
						lMin=l1;
					if(lMax>l2)
						lMax=l2;
					}
				
				if(lMin<lMax&&lMin<lambdaMin)
					{
					result=gid->device;
					lambdaMin=lMin;
					}
				}
	
	return result;
	}

bool InputGraphManager::grabInputDevice(InputDevice* device,Tool* grabber)
	{
	/* Get pointer to the graph input device: */
	GraphInputDevice* gid=deviceMap.getEntry(device).getDest();
	
	/* Bail out if the device is already grabbed: */
	if(gid->grabber!=0)
		return false;
	
	/* Get pointer to the graph tool: */
	GraphTool* gt=grabber!=0?toolMap.getEntry(grabber).getDest():&inputDeviceManager;
	
	/* Mark the input device as grabbed: */
	gid->grabber=gt;
	
	/* Update the input graph: */
	updateInputGraph();
	
	return true;
	}

void InputGraphManager::releaseInputDevice(InputDevice* device,Tool* grabber)
	{
	/* Get pointer to the graph input device: */
	GraphInputDevice* gid=deviceMap.getEntry(device).getDest();
	
	/* Get pointer to the graph tool: */
	GraphTool* gt=grabber!=0?toolMap.getEntry(grabber).getDest():&inputDeviceManager;
	
	/* Bail out if the device is not grabbed by the self-proclaimed grabber: */
	if(gid->grabber!=gt)
		return;
	
	/* Check if the device is in navigational mode: */
	if(gid->navigational)
		{
		/* Update the transformation from navigation coordinates to the device's current coordinates: */
		gid->fromNavTransform=device->getTransformation();
		gid->fromNavTransform.leftMultiply(getInverseNavigationTransformation());
		}
	
	/* Release the device grab: */
	gid->grabber=0;
	
	/* Update the input graph: */
	updateInputGraph();
	}

InputDevice* InputGraphManager::getRootDevice(InputDevice* device)
	{
	/* Trace the input device back through the input graph: */
	while(true)
		{
		/* Check if the device is grabbed by a device forwarding tool: */
		GraphInputDevice* gid=deviceMap.getEntry(device).getDest();
		if(gid->grabber==0)
			break;
		DeviceForwarder* df=dynamic_cast<DeviceForwarder*>(gid->grabber->tool);
		if(df==0)
			break;
		
		/* Check if the device forwarder has a source device: */
		InputDevice* sourceDevice=df->getSourceDevice(device);
		if(sourceDevice==0)
			break;
		
		/* Set the device to the source device and continue: */
		device=sourceDevice;
		}
	
	return device;
	}

InputDeviceFeature InputGraphManager::findFirstUnassignedFeature(const InputDeviceFeature& feature) const
	{
	/* Do a depth-first search for unassigned features: */
	std::vector<InputDeviceFeature> stack;
	stack.push_back(feature);
	while(!stack.empty())
		{
		/* Pop the current entry: */
		InputDeviceFeature f=stack.back();
		stack.pop_back();
		
		/* Get the feature's tool slot: */
		GraphInputDevice* gid=deviceMap.getEntry(f.getDevice()).getDest();
		ToolSlot& ts=gid->toolSlots[f.getFeatureIndex()];
		
		/* Check if the feature is unassigned: */
		if(ts.tool==0)
			return ts.feature; // Success!
		
		/* Check if the assigned tool is a device forwarder: */
		Vrui::DeviceForwarder* df=dynamic_cast<Vrui::DeviceForwarder*>(ts.tool->tool);
		if(df!=0)
			{
			/* Get the forwarded features: */
			InputDeviceFeatureSet forwardedFeatures=df->getForwardedFeatures(InputDeviceFeature(ts.feature));
			
			/* Add all forwarded features to the stack: */
			for(InputDeviceFeatureSet::reverse_iterator ffIt=forwardedFeatures.rbegin();ffIt!=forwardedFeatures.rend();++ffIt)
				stack.push_back(*ffIt);
			}
		}
	
	/* Nothing was found; return invalid feature: */
	return InputDeviceFeature();
	}

void InputGraphManager::showToolStack(const InputDeviceFeature& feature)
	{
	InputGraphManagerToolStackState tss;
	
	/* Get a size for the visualization elements: */
	tss.size=SceneGraph::Scalar(getUiSize())*SceneGraph::Scalar(2);
	
	/* Create a shared appearance node for tools: */
	tss.toolAppearance=new SceneGraph::AppearanceNode;
	
	SceneGraph::MaterialNodePointer toolMaterial(new SceneGraph::MaterialNode);
	toolMaterial->diffuseColor.setValue(SceneGraph::Color(0.5f,0.75f,0.5f));
	toolMaterial->specularColor.setValue(SceneGraph::Color(0.0f,0.0f,0.0f));
	toolMaterial->shininess.setValue(0.0f);
	toolMaterial->update();
	
	tss.toolAppearance->material.setValue(toolMaterial);
	tss.toolAppearance->update();
	
	tss.toolSize=SceneGraph::Size(tss.size*SceneGraph::Scalar(2),tss.size*SceneGraph::Scalar(2),tss.size*SceneGraph::Scalar(0.1));
	
	/* Create a fontstyle node for slot functions: */
	tss.slotFont=new SceneGraph::FontStyleNode;
	tss.slotFont->size=tss.size;
	tss.slotFont->justify.setValue("END");
	tss.slotFont->justify.appendValue("MIDDLE");
	tss.slotFont->update();
	
	/* Create a fontstyle node for tool names: */
	tss.toolFont=new SceneGraph::FontStyleNode;
	tss.toolFont->size=tss.size;
	tss.toolFont->justify.setValue("BEGIN");
	tss.toolFont->justify.appendValue("MIDDLE");
	tss.toolFont->update();
	
	/* Create a shared appearance node for devices: */
	tss.deviceAppearance=new SceneGraph::AppearanceNode;
	
	SceneGraph::MaterialNodePointer deviceMaterial(new SceneGraph::MaterialNode);
	deviceMaterial->diffuseColor.setValue(SceneGraph::Color(0.5f,0.5f,0.75f));
	deviceMaterial->specularColor.setValue(SceneGraph::Color(0.0f,0.0f,0.0f));
	deviceMaterial->shininess.setValue(0.0f);
	deviceMaterial->update();
	
	tss.deviceAppearance->material.setValue(deviceMaterial);
	tss.deviceAppearance->update();
	
	tss.deviceSize=SceneGraph::Size(tss.size*SceneGraph::Scalar(2),tss.size*SceneGraph::Scalar(2),tss.size*SceneGraph::Scalar(0.1));
	
	/* Remember the tool stack's base feature: */
	toolStackBaseFeature=feature;
	
	/* Create a tool stack for the feature's tool slot: */
	const ToolSlot& ts=deviceMap.getEntry(feature.getDevice()).getDest()->toolSlots[feature.getFeatureIndex()];
	toolStackNode=showToolStack(ts,tss);
	}

void InputGraphManager::update(void)
	{
	/* Set the transformations of ungrabbed navigational devices in the first graph level: */
	for(GraphInputDevice* gid=deviceLevels[0];gid!=0;gid=gid->levelSucc)
		if(gid->navigational&&gid->grabber==0)
			{
			/* Set the device's transformation: */
			NavTrackerState transform=getNavigationTransformation();
			transform*=gid->fromNavTransform;
			transform.renormalize();
			gid->device->setTransformation(TrackerState(transform.getTranslation(),transform.getRotation()));
			}
	
	/* Go through all graph levels: */
	for(int i=0;i<=maxGraphLevel;++i)
		{
		/* Trigger callbacks on all input devices in the level: */
		for(GraphInputDevice* gid=deviceLevels[i];gid!=0;gid=gid->levelSucc)
			{
			gid->device->enableCallbacks();
			gid->device->disableCallbacks();
			}
		
		/* Call frame method on all tools in the level: */
		for(GraphTool* gt=toolLevels[i];gt!=0;gt=gt->levelSucc)
			gt->tool->frame();
		}
	}

void InputGraphManager::glRenderDevices(GLContextData& contextData) const
	{
	/* Get the glyph renderer's context data item: */
	const GlyphRenderer::DataItem* glyphRendererContextDataItem=glyphRenderer->getContextDataItem(contextData);
	
	/* Render all input devices in the first input graph level: */
	for(const GraphInputDevice* gid=deviceLevels[0];gid!=0;gid=gid->levelSucc)
		{
		/* Check if the device is an ungrabbed virtual input device: */
		if(gid->grabber==0)
			virtualInputDevice->renderDevice(gid->device,gid->navigational,glyphRendererContextDataItem,contextData);
		else
			{
			OGTransform transform(gid->device->getTransformation());
			if(gid->deviceGlyph.getGlyphType()==Glyph::CONE)
				{
				/* Rotate the glyph so that its Y axis aligns to the device's ray direction: */
				transform*=OGTransform::rotate(Rotation::rotateFromTo(Vector(0,1,0),gid->device->getDeviceRayDirection()));
				}
			glyphRenderer->renderGlyph(gid->deviceGlyph,transform,glyphRendererContextDataItem);
			}
		}
	
	/* Iterate through all higher input graph levels: */
	for(int level=1;level<=maxGraphLevel;++level)
		{
		/* Render all input devices in this level: */
		for(const GraphInputDevice* gid=deviceLevels[level];gid!=0;gid=gid->levelSucc)
			{
			OGTransform transform(gid->device->getTransformation());
			if(gid->deviceGlyph.getGlyphType()==Glyph::CONE)
				{
				/* Rotate the glyph so that its Y axis aligns to the device's ray direction: */
				transform*=OGTransform::rotate(Rotation::rotateFromTo(Vector(0,1,0),gid->device->getDeviceRayDirection()));
				}
			glyphRenderer->renderGlyph(gid->deviceGlyph,transform,glyphRendererContextDataItem);
			}
		}
	
	/* Check if there is a tool stack visualization to display: */
	if(toolStackNode!=0)
		{
		/* Save OpenGL state: */
		glPushAttrib(GL_ENABLE_BIT|GL_LIGHTING_BIT|GL_TEXTURE_BIT);
		
		/* Visualize the tool stack: */
		renderSceneGraph(toolStackNode.getPointer(),calcHUDTransform(toolStackBaseFeature.getDevice()->getPosition()),false,contextData);
		
		/* Restore OpenGL state: */
		glPopAttrib();
		}
	}

void InputGraphManager::glRenderTools(GLContextData& contextData) const
	{
	/* Render all tools in the first input graph level: */
	for(const GraphTool* gt=toolLevels[0];gt!=0;gt=gt->levelSucc)
		gt->tool->display(contextData);
	
	/* Iterate through all higher input graph levels: */
	for(int level=1;level<=maxGraphLevel;++level)
		{
		/* Render all tools in this level: */
		for(const GraphTool* gt=toolLevels[level];gt!=0;gt=gt->levelSucc)
			gt->tool->display(contextData);
		}
	}

}
