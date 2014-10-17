/***********************************************************************
RiftLEDControl - Vislet class to control the tracking LEDs on an Oculus
Rift DK2 interactively from inside a Vrui application.
Copyright (c) 2014 Oliver Kreylos

This file is part of the optical/inertial sensor fusion tracking
package.

The optical/inertial sensor fusion tracking package is free software;
you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

The optical/inertial sensor fusion tracking package is distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the optical/inertial sensor fusion tracking package; if not, write
to the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
Boston, MA 02111-1307 USA
***********************************************************************/

#include "RiftLEDControl.h"

#include <unistd.h>
#include <iostream>
#include <Misc/Time.h>
#include <RawHID/BusType.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/Blind.h>
#include <GLMotif/Label.h>
#include <GLMotif/Button.h>
#include <Vrui/Vrui.h>
#include <Vrui/VisletManager.h>

/**************************************
Methods of class RiftLEDControlFactory:
**************************************/

RiftLEDControlFactory::RiftLEDControlFactory(Vrui::VisletManager& visletManager)
	:Vrui::VisletFactory("RiftLEDControl",visletManager)
	{
	#if 0
	/* Insert class into class hierarchy: */
	Vrui::VisletFactory* visletFactory=visletManager.loadClass("Vislet");
	visletFactory->addChildClass(this);
	addParentClass(visletFactory);
	#endif
	
	/* Set vislet class' factory pointer: */
	RiftLEDControl::factory=this;
	}

RiftLEDControlFactory::~RiftLEDControlFactory(void)
	{
	/* Reset vislet class' factory pointer: */
	RiftLEDControl::factory=0;
	}

Vrui::Vislet* RiftLEDControlFactory::createVislet(int numArguments,const char* const arguments[]) const
	{
	return new RiftLEDControl(numArguments,arguments);
	}

void RiftLEDControlFactory::destroyVislet(Vrui::Vislet* vislet) const
	{
	delete vislet;
	}
/***************************************
Static elements of class RiftLEDControl:
***************************************/

RiftLEDControlFactory* RiftLEDControl::factory=0;

/*******************************
Methods of class RiftLEDControl:
*******************************/

void* RiftLEDControl::keepAliveThreadMethod(void)
	{
	Misc::Time wakeupInterval(9,0); // Update at least every 9 seconds
	
	#if 0
	CameraIntrinsicParameters cip;
	cip.get(rift);
	for(int i=0;i<3;++i)
		{
		for(int j=0;j<4;++j)
			std::cout<<' '<<cip.matrix[i][j];
		std::cout<<std::endl;
		}
	#endif
	
	#if 1
	/* Run some reports: */
	Unknown0x02 currentUnknown0x02(0x01U);
	currentUnknown0x02.get(rift);
	Unknown0x02 unknown0x02(0x01U);
	unknown0x02.set(rift,0x0000U);
	#endif
	
	/* Send the initial LED control report: */
	ledControl.set(rift,0x0000U);
	unsigned int lastSentLedControlVersion=ledControlVersion;
	
	while(true)
		{
		/* Wait on the LED control condition variable: */
		bool rlc;
		{
		Threads::MutexCond::Lock ledControlLock(ledControlCond);
		Misc::Time wakeupTime=Misc::Time::now();
		wakeupTime+=wakeupInterval;
		ledControlCond.timedWait(ledControlLock,wakeupTime);
		rlc=runLedCycle;
		runLedCycle=false;
		}
		
		if(rlc)
			{
			std::cout<<"Running LED cycle..."<<std::flush;
			
			RawHID::Device::Byte reportBuffer[7];
			reportBuffer[0]=0x02U;
			reportBuffer[1]=0x00U;
			reportBuffer[2]=0x00U;
			reportBuffer[3]=0x20U;
			reportBuffer[4]=0x13U; // or 0x01U
			reportBuffer[5]=0xe8U;
			reportBuffer[6]=0x03U;
			rift.writeFeatureReport(reportBuffer,sizeof(reportBuffer));
			
			/* Set up the control report to go into the LED cycle: */
			LEDControl ledCycleControl=ledControl;
			ledCycleControl.pattern=0;
			ledCycleControl.enable=true;
			ledCycleControl.autoIncrement=false;
			ledCycleControl.useCarrier=true;
			ledCycleControl.syncInput=true;
			ledCycleControl.vsyncLock=false;
			ledCycleControl.customPattern=false;
			ledCycleControl.set(rift,0x0000U);
			
			/* Query the full set of LED position reports: */
			LEDPosition ledPosition;
			ledPosition.get(rift);
			unsigned int numReports=ledPosition.numReports;
			for(unsigned int i=1;i<numReports;++i)
				{
				usleep(33333);
				ledPosition.get(rift);
				}
			
			/* Go back to normal mode: */
			ledControl.set(rift,0x0000U);
			
			std::cout<<" done"<<std::endl;
			}
		else
			{
			if(lastSentLedControlVersion!=ledControlVersion)
				{
				/* Send the current LED control report to the device: */
				LEDControl currentLedControl;
				currentLedControl.get(rift);
				ledControl.set(rift,0x0000U);
				currentLedControl.get(rift);
				lastSentLedControlVersion=ledControlVersion;
				}
			
			/* Send a keep-alive report to the device: */
			KeepAliveDK2 ka(true,10000);
			ka.set(rift,0x0000U);
			}
		}
	
	return 0;
	}

void RiftLEDControl::toggleButtonCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData,const int& toggleIndex)
	{
	Threads::MutexCond::Lock ledControlLock(ledControlCond);
	
	switch(toggleIndex)
		{
		case 0:
			ledControl.enable=cbData->set;
			break;
		
		case 1:
			ledControl.autoIncrement=cbData->set;
			break;
		
		case 2:
			ledControl.useCarrier=cbData->set;
			break;
		
		case 3:
			ledControl.syncInput=cbData->set;
			break;
		
		case 4:
			ledControl.vsyncLock=cbData->set;
			break;
		
		case 5:
			ledControl.customPattern=cbData->set;
			break;
		}
	
	/* Send the changed LED control settings to the device: */
	++ledControlVersion;
	ledControlCond.signal();
	}

void RiftLEDControl::ledCycleButtonCallback(Misc::CallbackData* cbData)
	{
	/* Tell the LED control thread to run a LED cycle: */
	Threads::MutexCond::Lock ledControlLock(ledControlCond);
	runLedCycle=true;
	ledControlCond.signal();
	}

void RiftLEDControl::textFieldSliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData,const int& sliderIndex)
	{
	Threads::MutexCond::Lock ledControlLock(ledControlCond);
	
	switch(sliderIndex)
		{
		case 0:
			ledControl.pattern=(unsigned int)(cbData->value+0.5);
			break;
		
		case 1:
			ledControl.exposureLength=(unsigned int)(cbData->value+0.5);
			break;
		
		case 2:
			ledControl.frameInterval=(unsigned int)(cbData->value+0.5);
			break;
		
		case 3:
			ledControl.vsyncOffset=(unsigned int)(cbData->value+0.5);
			break;
		
		case 4:
			ledControl.dutyCycle=(unsigned int)(cbData->value+0.5);
			break;
		}
	
	/* Send the changed LED control settings to the device: */
	++ledControlVersion;
	ledControlCond.signal();
	}

RiftLEDControl::RiftLEDControl(int numArguments,const char* const arguments[])
	:rift(RawHID::BUSTYPE_USB,0x2833U,0x0021U,0),
	 runLedCycle(false),ledControlVersion(0),
	 dialogWindow(0)
	{
	/* Build the graphical user interface: */
	const GLMotif::StyleSheet& ss=*Vrui::getUiStyleSheet();
	
	dialogWindow=new GLMotif::PopupWindow("LedControlsDialog",Vrui::getWidgetManager(),"Rift LED Control");
	dialogWindow->setHideButton(true);
	dialogWindow->setResizableFlags(true,false);
	
	GLMotif::RowColumn* ledControls=new GLMotif::RowColumn("LedControls",dialogWindow,false);
	ledControls->setOrientation(GLMotif::RowColumn::VERTICAL);
	ledControls->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	ledControls->setNumMinorWidgets(2);
	
	new GLMotif::Blind("Blind1",ledControls);
	
	GLMotif::RowColumn* toggleBox=new GLMotif::RowColumn("ToggleBox",ledControls,false);
	toggleBox->setOrientation(GLMotif::RowColumn::VERTICAL);
	toggleBox->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	toggleBox->setNumMinorWidgets(3);
	
	GLMotif::ToggleButton* enableToggle=new GLMotif::ToggleButton("EnableToggle",toggleBox,"Enable");
	enableToggle->setToggleType(GLMotif::ToggleButton::TOGGLE_BUTTON);
	enableToggle->setToggle(ledControl.enable);
	enableToggle->getValueChangedCallbacks().add(this,&RiftLEDControl::toggleButtonCallback,0);
	
	GLMotif::ToggleButton* autoIncrementToggle=new GLMotif::ToggleButton("AutoIncrementToggle",toggleBox,"Flash LED IDs");
	autoIncrementToggle->setToggleType(GLMotif::ToggleButton::TOGGLE_BUTTON);
	autoIncrementToggle->setToggle(ledControl.autoIncrement);
	autoIncrementToggle->getValueChangedCallbacks().add(this,&RiftLEDControl::toggleButtonCallback,1);
	
	GLMotif::ToggleButton* useCarrierToggle=new GLMotif::ToggleButton("UseCarrierToggle",toggleBox,"Modulate");
	useCarrierToggle->setToggleType(GLMotif::ToggleButton::TOGGLE_BUTTON);
	useCarrierToggle->setToggle(ledControl.useCarrier);
	useCarrierToggle->getValueChangedCallbacks().add(this,&RiftLEDControl::toggleButtonCallback,2);
	
	GLMotif::ToggleButton* syncInputToggle=new GLMotif::ToggleButton("SyncInputToggle",toggleBox,"Unknown");
	syncInputToggle->setToggleType(GLMotif::ToggleButton::TOGGLE_BUTTON);
	syncInputToggle->setToggle(ledControl.syncInput);
	syncInputToggle->getValueChangedCallbacks().add(this,&RiftLEDControl::toggleButtonCallback,3);
	
	GLMotif::ToggleButton* vsyncLockToggle=new GLMotif::ToggleButton("VsyncLockToggle",toggleBox,"Unknown");
	vsyncLockToggle->setToggleType(GLMotif::ToggleButton::TOGGLE_BUTTON);
	vsyncLockToggle->setToggle(ledControl.vsyncLock);
	vsyncLockToggle->getValueChangedCallbacks().add(this,&RiftLEDControl::toggleButtonCallback,4);
	
	GLMotif::ToggleButton* customPatternToggle=new GLMotif::ToggleButton("CustomPatternToggle",toggleBox,"Unknown");
	customPatternToggle->setToggleType(GLMotif::ToggleButton::TOGGLE_BUTTON);
	customPatternToggle->setToggle(ledControl.customPattern);
	customPatternToggle->getValueChangedCallbacks().add(this,&RiftLEDControl::toggleButtonCallback,5);
	
	toggleBox->manageChild();
	
	new GLMotif::Label("PatternLabel",ledControls,"Pattern");
	
	GLMotif::TextFieldSlider* patternSlider=new GLMotif::TextFieldSlider("PatternSlider",ledControls,8,ss.fontHeight*10.0f);
	patternSlider->getTextField()->setFieldWidth(8);
	patternSlider->setSliderMapping(GLMotif::TextFieldSlider::LINEAR);
	patternSlider->setValueType(GLMotif::TextFieldSlider::UINT);
	patternSlider->setValueRange(0,255,1);
	patternSlider->setValue(ledControl.pattern);
	patternSlider->getValueChangedCallbacks().add(this,&RiftLEDControl::textFieldSliderCallback,0);
	
	new GLMotif::Label("ExposureLengthLabel",ledControls,"LED Interval");
	
	GLMotif::TextFieldSlider* exposureLengthSlider=new GLMotif::TextFieldSlider("ExposureLengthSlider",ledControls,8,ss.fontHeight*10.0f);
	exposureLengthSlider->getTextField()->setFieldWidth(8);
	exposureLengthSlider->setSliderMapping(GLMotif::TextFieldSlider::LINEAR);
	exposureLengthSlider->setValueType(GLMotif::TextFieldSlider::UINT);
	exposureLengthSlider->setValueRange(0,65535,1);
	exposureLengthSlider->setValue(ledControl.exposureLength);
	exposureLengthSlider->getValueChangedCallbacks().add(this,&RiftLEDControl::textFieldSliderCallback,1);
	
	new GLMotif::Label("FrameIntervalLabel",ledControls,"Frame Interval");
	
	GLMotif::TextFieldSlider* frameIntervalSlider=new GLMotif::TextFieldSlider("FrameIntervalSlider",ledControls,8,ss.fontHeight*10.0f);
	frameIntervalSlider->getTextField()->setFieldWidth(8);
	frameIntervalSlider->setSliderMapping(GLMotif::TextFieldSlider::LINEAR);
	frameIntervalSlider->setValueType(GLMotif::TextFieldSlider::UINT);
	frameIntervalSlider->setValueRange(0,65535,1);
	frameIntervalSlider->setValue(ledControl.frameInterval);
	frameIntervalSlider->getValueChangedCallbacks().add(this,&RiftLEDControl::textFieldSliderCallback,2);
	
	new GLMotif::Label("VsyncOffsetLabel",ledControls,"Unknown");
	
	GLMotif::TextFieldSlider* vsyncOffsetSlider=new GLMotif::TextFieldSlider("VsyncOffsetSlider",ledControls,8,ss.fontHeight*10.0f);
	vsyncOffsetSlider->getTextField()->setFieldWidth(8);
	vsyncOffsetSlider->setSliderMapping(GLMotif::TextFieldSlider::LINEAR);
	vsyncOffsetSlider->setValueType(GLMotif::TextFieldSlider::UINT);
	vsyncOffsetSlider->setValueRange(0,65535,1);
	vsyncOffsetSlider->setValue(ledControl.vsyncOffset);
	vsyncOffsetSlider->getValueChangedCallbacks().add(this,&RiftLEDControl::textFieldSliderCallback,3);
	
	new GLMotif::Label("DutyCycleLabel",ledControls,"Modulation");
	
	GLMotif::TextFieldSlider* dutyCycleSlider=new GLMotif::TextFieldSlider("DutyCycleSlider",ledControls,8,ss.fontHeight*10.0f);
	dutyCycleSlider->getTextField()->setFieldWidth(8);
	dutyCycleSlider->setSliderMapping(GLMotif::TextFieldSlider::LINEAR);
	dutyCycleSlider->setValueType(GLMotif::TextFieldSlider::UINT);
	dutyCycleSlider->setValueRange(0,255,1);
	dutyCycleSlider->setValue(ledControl.dutyCycle);
	dutyCycleSlider->getValueChangedCallbacks().add(this,&RiftLEDControl::textFieldSliderCallback,4);
	
	GLMotif::Button* ledCycleButton=new GLMotif::Button("LedCycleButton",ledControls,"Run LED Cycle");
	ledCycleButton->getSelectCallbacks().add(this,&RiftLEDControl::ledCycleButtonCallback);
	
	ledControls->manageChild();
	
	/* Start the keep-alive thread: */
	keepAliveThread.start(this,&RiftLEDControl::keepAliveThreadMethod);
	}

RiftLEDControl::~RiftLEDControl(void)
	{
	/* Stop the keep-alive thread: */
	keepAliveThread.cancel();
	keepAliveThread.join();
	}

Vrui::VisletFactory* RiftLEDControl::getFactory(void) const
	{
	return factory;
	}

void RiftLEDControl::disable(void)
	{
	if(dialogWindow!=0)
		{
		/* Hide the LED control dialog: */
		Vrui::popdownPrimaryWidget(dialogWindow);
		}
	
	active=false;
	}

void RiftLEDControl::enable(void)
	{
	/* Show the viewer configuration dialog: */
	Vrui::popupPrimaryWidget(dialogWindow);
	
	active=true;
	}
