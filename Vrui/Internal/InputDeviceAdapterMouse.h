/***********************************************************************
InputDeviceAdapterMouse - Class to convert mouse and keyboard into a
Vrui input device.
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

#ifndef VRUI_INTERNAL_INPUTDEVICEADAPTERMOUSE_INCLUDED
#define VRUI_INTERNAL_INPUTDEVICEADAPTERMOUSE_INCLUDED

#include <X11/Xlib.h>
#include <string>
#include <utility>
#include <vector>
#include <Misc/HashTable.h>
#include <Geometry/Point.h>
#include <Geometry/OrthonormalTransformation.h>
#include <GLMotif/TextEvent.h>
#include <GLMotif/TextControlEvent.h>
#include <Vrui/Geometry.h>
#include <Vrui/Internal/InputDeviceAdapter.h>
#include <Vrui/Internal/KeyMapper.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}
namespace Vrui {
class VRWindow;
}

namespace Vrui {

class InputDeviceAdapterMouse:public InputDeviceAdapter
	{
	/* Embedded classes: */
	private:
	typedef Misc::HashTable<KeyMapper::QualifiedKey,GLMotif::TextControlEvent,KeyMapper::QualifiedKey> ControlKeyMap; // Type for hash tables mapping qualified keys to GLMotif text control events
	
	/* Elements: */
	private:
	int numButtons; // Number of mapped mouse buttons
	int numButtonKeys; // Number of keys treated as mouse buttons
	int* buttonKeysyms; // Map from key symbols to button key indices
	int numModifierKeys; // Number of used modifier keys
	int* modifierKeysyms; // Map from key codes to modifier key indices
	bool modifiersAsButtons; // Flag to add the defined modifier keys as additional buttons
	int numButtonStates; // Number of button states (number of buttons times number of modifier key states)
	KeyMapper::QualifiedKey keyboardModeToggleKey; // Qualified key which switches keyboard between button and key mode
	ControlKeyMap controlKeyMap; // Map from qualified keys to GLMotif text control events
	int modifierKeyMask; // Current modifier key mask
	bool* buttonStates; // Array of current button states
	int numPressedButtons; // Number of currently pressed buttons, to keep track of mouse pointer grabs
	bool keyboardMode; // Flag whether the keyboard is in key mode
	int* numMouseWheelTicks; // Number of mouse wheel ticks for each modifier key mask accumulated during frame processing
	VRWindow* window; // VR window containing the last reported mouse position
	int mousePos[2]; // Current mouse position in window (pixel) coordinates of window containing the last known mouse position
	VRWindow* grabWindow; // Window that currently has a pointer grab
	bool mouseLocked; // Flag whether the mouse pointer is currently locked
	int lockedMousePos[2]; // Mouse pointer position at time of locking in window coordinates
	Vector lockedRayDirection; // Mouse device ray direction while the mouse is locked
	Scalar lockedRayStart; // Mouse device ray start parameter while the mouse is locked
	TrackerState lockedTransformation; // Mouse device transformation while the mouse is locked
	bool fakeMouseCursor; // Flag whether the adapter draws its own mouse cursor
	
	/* Private methods: */
	int getButtonIndex(int keysym) const; // Returns the button key index of the given key, or -1
	int getModifierIndex(int keysym) const; // Returns the modifier key index of the given key, or -1
	bool changeButtonState(int stateIndex,bool newState); // Changes the state of a button and does related processing; returns true if button state actually changed
	void changeModifierKeyMask(int newModifierKeyMask); // Called whenever the current modifier key mask changes
	
	/* Constructors and destructors: */
	public:
	InputDeviceAdapterMouse(InputDeviceManager* sInputDeviceManager,const Misc::ConfigurationFileSection& configFileSection);
	virtual ~InputDeviceAdapterMouse(void);
	
	/* Methods from InputDeviceAdapter: */
	virtual std::string getFeatureName(const InputDeviceFeature& feature) const;
	virtual int getFeatureIndex(InputDevice* device,const char* featureName) const;
	virtual void updateInputDevices(void);
	
	/* New methods: */
	bool needMouseCursor(void) const // Returns true if the input device adapter requires a hardware mouse cursor
		{
		return !fakeMouseCursor;
		}
	VRWindow* getWindow(void) const // Returns the window containing the last reported mouse position
		{
		return window;
		}
	const int* getMousePosition(void) const // Returns the current mouse position in window (pixel) coordinates
		{
		return mousePos;
		}
	void setMousePosition(VRWindow* newWindow,const int newMousePos[2]); // Sets current mouse position in window (pixel) coordinates of given window
	bool keyPressed(int keysym,int modifierMask,const char* string); // Notifies adapter that a key has been pressed; returns true if adapter's state changed
	bool keyReleased(int keysym); // Notifies adapter that a key has been released; returns true if adapter's state changed
	void resetKeys(const XKeymapEvent& event); // Resets pressed keys and the modifier key mask when the mouse cursor re-enters a window
	bool setButtonState(int buttonIndex,bool newButtonState); // Sets current button state; returns true if adapter's state changed
	void incMouseWheelTicks(void); // Increases the number of mouse wheel ticks
	void decMouseWheelTicks(void); // Decreases the number of mouse wheel ticks
	void lockMouse(void); // Locks the mouse pointer to the center of the current window; future device updates only report velocities
	void unlockMouse(void); // Unlocks the mouse pointer
	};

/****************
Helper functions:
****************/

ONTransform getMouseScreenTransform(InputDeviceAdapterMouse* mouseAdapter,Scalar viewport[4]); // Returns screen transformation of appropriate screen for given mouse adapter, and copies screen's viewport dimensions

}

#endif
