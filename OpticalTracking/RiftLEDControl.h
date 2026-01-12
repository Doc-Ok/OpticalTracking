/***********************************************************************
RiftLEDControl - Vislet class to control the tracking LEDs on an Oculus
Rift DK2 interactively from inside a Vrui application.
Copyright (c) 2014-2019 Oliver Kreylos

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

#ifndef RIFTLEDCONTROL_INCLUDED
#define RIFTLEDCONTROL_INCLUDED

#include <Threads/MutexCond.h>
#include <Threads/Thread.h>
#include <RawHID/Device.h>
#include <GLMotif/ToggleButton.h>
#include <GLMotif/TextFieldSlider.h>
#include <Vrui/Vislet.h>

#include "OculusRiftHIDReports.h"

/* Forward declarations: */
namespace GLMotif {
class PopupWindow;
class RowColumn;
class Button;
}

class RiftLEDControl;

class RiftLEDControlFactory:public Vrui::VisletFactory
	{
	friend class RiftLEDControl;
	
	/* Constructors and destructors: */
	public:
	RiftLEDControlFactory(Vrui::VisletManager& visletManager);
	virtual ~RiftLEDControlFactory(void);
	
	/* Methods: */
	virtual Vrui::Vislet* createVislet(int numVisletArguments,const char* const visletArguments[]) const;
	virtual void destroyVislet(Vrui::Vislet* vislet) const;
	};

class RiftLEDControl:public Vrui::Vislet
	{
	friend class RiftLEDControlFactory;
	
	/* Elements: */
	private:
	static RiftLEDControlFactory* factory; // Pointer to the factory object for this class
	RawHID::Device rift; // The Rift's raw HID device
	Threads::MutexCond ledControlCond; // Condition variable protecting the LED control feature report and waking up the keep-alive thread
	bool runLedCycle; // Flag to tell the keep-alive thread to run a LED cycle
	unsigned int ledControlVersion; // Version number of data in LED control report
	LEDControl ledControl; // HID feature report to control LEDs
	Threads::Thread keepAliveThread; // Thread to send LED control messages to the Rift at regular intervals
	
	GLMotif::PopupWindow* dialogWindow; // LED controls dialog window
	
	/* Private methods: */
	void* keepAliveThreadMethod(void);
	void toggleButtonCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData,const int& toggleIndex);
	void textFieldSliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData,const int& sliderIndex);
	void ledCycleButtonCallback(Misc::CallbackData* cbData);
	
	/* Constructors and destructors: */
	public:
	RiftLEDControl(int numArguments,const char* const arguments[]);
	virtual ~RiftLEDControl(void);
	
	/* Methods from Vislet: */
	public:
	virtual Vrui::VisletFactory* getFactory(void) const;
	virtual void enable(bool startup);
	virtual void disable(bool shutdown);
	};

#endif
