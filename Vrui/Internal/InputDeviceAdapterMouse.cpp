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

#include <Vrui/Internal/InputDeviceAdapterMouse.h>

#include <stdlib.h>
#include <stdio.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/CompoundValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Geometry/Vector.h>
#include <Geometry/Ray.h>
#include <Geometry/GeometryValueCoders.h>
#include <Vrui/Vrui.h>
#include <Vrui/GlyphRenderer.h>
#include <Vrui/InputDevice.h>
#include <Vrui/InputDeviceFeature.h>
#include <Vrui/TextEventDispatcher.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/VRScreen.h>
#include <Vrui/Viewer.h>
#include <Vrui/VRWindow.h>

namespace Vrui {

namespace {

/*******************************************************************
Helper structures to map from qualified keys to GLMotif text control
functions:
*******************************************************************/

typedef KeyMapper::QualifiedKey QK;
typedef GLMotif::TextControlEvent TCE;

struct ControlKeyMapItem
	{
	/* Elements: */
	public:
	QK qk;
	TCE tce;
	};

/* Fixed mapping from control keys in keyboard mode to GLMotif text control functions: */
static const ControlKeyMapItem rawControlKeyMap[]=
	{
	{QK(XK_Left,0x0),TCE(TCE::CURSOR_LEFT,false)},
	{QK(XK_Right,0x0),TCE(TCE::CURSOR_RIGHT,false)},
	{QK(XK_Left,ControlMask),TCE(TCE::CURSOR_WORD_LEFT,false)},
	{QK(XK_Right,ControlMask),TCE(TCE::CURSOR_WORD_RIGHT,false)},
	{QK(XK_Home,0x0),TCE(TCE::CURSOR_START,false)},
	{QK(XK_End,0x0),TCE(TCE::CURSOR_END,false)},
	{QK(XK_Up,0x0),TCE(TCE::CURSOR_UP,false)},
	{QK(XK_Down,0x0),TCE(TCE::CURSOR_DOWN,false)},
	{QK(XK_Page_Up,0x0),TCE(TCE::CURSOR_PAGE_UP,false)},
	{QK(XK_Page_Down,0x0),TCE(TCE::CURSOR_PAGE_DOWN,false)},
	{QK(XK_Home,ControlMask),TCE(TCE::CURSOR_TEXT_START,false)},
	{QK(XK_End,ControlMask),TCE(TCE::CURSOR_TEXT_END,false)},
	
	{QK(XK_Left,ShiftMask),TCE(TCE::CURSOR_LEFT,true)},
	{QK(XK_Right,ShiftMask),TCE(TCE::CURSOR_RIGHT,true)},
	{QK(XK_Left,ControlMask|ShiftMask),TCE(TCE::CURSOR_WORD_LEFT,true)},
	{QK(XK_Right,ControlMask|ShiftMask),TCE(TCE::CURSOR_WORD_RIGHT,true)},
	{QK(XK_Home,ShiftMask),TCE(TCE::CURSOR_START,true)},
	{QK(XK_End,ShiftMask),TCE(TCE::CURSOR_END,true)},
	{QK(XK_Up,ShiftMask),TCE(TCE::CURSOR_UP,true)},
	{QK(XK_Down,ShiftMask),TCE(TCE::CURSOR_DOWN,true)},
	{QK(XK_Page_Up,ShiftMask),TCE(TCE::CURSOR_PAGE_UP,true)},
	{QK(XK_Page_Down,ShiftMask),TCE(TCE::CURSOR_PAGE_DOWN,true)},
	{QK(XK_Home,ControlMask|ShiftMask),TCE(TCE::CURSOR_TEXT_START,true)},
	{QK(XK_End,ControlMask|ShiftMask),TCE(TCE::CURSOR_TEXT_END,true)},
	
	{QK(XK_Delete,0x0),TCE(TCE::DELETE)},
	{QK(XK_BackSpace,0x0),TCE(TCE::BACKSPACE)},
	
	{QK(XK_Delete,ShiftMask),TCE(TCE::CUT)},
	{QK(XK_x,ControlMask),TCE(TCE::CUT)},
	{QK(XK_X,ControlMask),TCE(TCE::CUT)},
	{QK(XK_Insert,ControlMask),TCE(TCE::COPY)},
	{QK(XK_c,ControlMask),TCE(TCE::COPY)},
	{QK(XK_C,ControlMask),TCE(TCE::COPY)},
	{QK(XK_Insert,ShiftMask),TCE(TCE::PASTE)},
	{QK(XK_v,ControlMask),TCE(TCE::PASTE)},
	{QK(XK_V,ControlMask),TCE(TCE::PASTE)},
	
	{QK(XK_Return,0x0),TCE(TCE::CONFIRM)}
	};

}

/****************************************
Methods of class InputDeviceAdapterMouse:
****************************************/

int InputDeviceAdapterMouse::getButtonIndex(int keysym) const
	{
	for(int i=0;i<numButtonKeys;++i)
		if(buttonKeysyms[i]==keysym)
			return i;
	
	return -1;
	}

int InputDeviceAdapterMouse::getModifierIndex(int keysym) const
	{
	for(int i=0;i<numModifierKeys;++i)
		if(modifierKeysyms[i]==keysym)
			return i;
	
	return -1;
	}

bool InputDeviceAdapterMouse::changeButtonState(int stateIndex,bool newState)
	{
	/* Check if the button state actually changed: */
	if(buttonStates[stateIndex]==newState)
		return false;
	
	/* Keep track of the total number of pressed buttons: */
	if(buttonStates[stateIndex])
		--numPressedButtons;
	else
		++numPressedButtons;
	
	/* Set the new button state: */
	buttonStates[stateIndex]=newState;
	
	/* Grab or release the mouse pointer if necessary: */
	if(numPressedButtons>0&&grabWindow==0)
		{
		/* Try grabbing the mouse pointer: */
		if(window->grabPointer())
			grabWindow=window;
		}
	if(numPressedButtons==0&&grabWindow!=0)
		{
		/* Release the mouse pointer: */
		grabWindow->releasePointer();
		grabWindow=0;
		}
	
	return true;
	}

void InputDeviceAdapterMouse::changeModifierKeyMask(int newModifierKeyMask)
	{
	/* Copy all button states from the old layer to the new layer: */
	bool* oldLayer=buttonStates+(numButtons+numButtonKeys)*modifierKeyMask;
	bool* newLayer=buttonStates+(numButtons+numButtonKeys)*newModifierKeyMask;
	for(int i=0;i<numButtons+numButtonKeys;++i)
		{
		if(newLayer[i]&&!oldLayer[i])
			--numPressedButtons;
		if(!newLayer[i]&&oldLayer[i])
			++numPressedButtons;
		newLayer[i]=oldLayer[i];
		}
	
	if(modifiersAsButtons)
		{
		/* Update the states of the forwarded modifier key buttons: */
		int firstModifierKeyButton=(numButtons+numButtonKeys)*(1<<numModifierKeys);
		for(int i=0;i<numModifierKeys;++i)
			{
			if((modifierKeyMask&(0x1<<i))==0x0&&(newModifierKeyMask&(0x1<<i))!=0x0)
				{
				buttonStates[firstModifierKeyButton+i]=true;
				++numPressedButtons;
				}
			else if((modifierKeyMask&(0x1<<i))!=0x0&&(newModifierKeyMask&(0x1<<i))==0x0)
				{
				buttonStates[firstModifierKeyButton+i]=false;
				--numPressedButtons;
				}
			}
		}
	
	/* Change the modifier key mask: */
	modifierKeyMask=newModifierKeyMask;
	
	/* Grab or release the mouse pointer if necessary: */
	if(numPressedButtons>0&&grabWindow==0)
		{
		/* Try grabbing the mouse pointer: */
		if(window->grabPointer())
			grabWindow=window;
		}
	if(numPressedButtons==0&&grabWindow!=0)
		{
		/* Release the mouse pointer: */
		grabWindow->releasePointer();
		grabWindow=0;
		}
	}

InputDeviceAdapterMouse::InputDeviceAdapterMouse(InputDeviceManager* sInputDeviceManager,const Misc::ConfigurationFileSection& configFileSection)
	:InputDeviceAdapter(sInputDeviceManager),
	 numButtons(0),
	 numButtonKeys(0),buttonKeysyms(0),
	 numModifierKeys(0),modifierKeysyms(0),modifiersAsButtons(false),
	 numButtonStates(0),
	 keyboardModeToggleKey(0,0),controlKeyMap(101),
	 modifierKeyMask(0x0),buttonStates(0),numPressedButtons(0),
	 keyboardMode(false),
	 numMouseWheelTicks(0),
	 window(0),
	 grabWindow(0),
	 mouseLocked(false),
	 fakeMouseCursor(false)
	{
	typedef std::vector<std::string> StringList;
	
	/* Allocate new adapter state arrays: */
	numInputDevices=1;
	inputDevices=new InputDevice*[numInputDevices];
	inputDevices[0]=0;
	
	/* Retrieve the number of mouse buttons: */
	numButtons=configFileSection.retrieveValue<int>("./numButtons",0);
	
	/* Retrieve button key list: */
	StringList buttonKeyNames=configFileSection.retrieveValue<StringList>("./buttonKeys",StringList());
	numButtonKeys=buttonKeyNames.size();
	if(numButtonKeys>0)
		{
		/* Get key codes for all button keys: */
		buttonKeysyms=new int[numButtonKeys];
		for(int i=0;i<numButtonKeys;++i)
			buttonKeysyms[i]=KeyMapper::getKeysym(buttonKeyNames[i]);
		}
	
	/* Retrieve modifier key list: */
	StringList modifierKeyNames=configFileSection.retrieveValue<StringList>("./modifierKeys",StringList());
	numModifierKeys=modifierKeyNames.size();
	if(numModifierKeys>0)
		{
		/* Get key codes for all modifier keys: */
		modifierKeysyms=new int[numModifierKeys];
		for(int i=0;i<numModifierKeys;++i)
			modifierKeysyms[i]=KeyMapper::getKeysym(modifierKeyNames[i]);
		}
	
	/* Read the modifiers-as-buttons flag: */
	modifiersAsButtons=configFileSection.retrieveValue<bool>("./modifiersAsButtons",modifiersAsButtons);
	
	/* Calculate number of buttons and valuators: */
	numButtonStates=(numButtons+numButtonKeys)*(1<<numModifierKeys);
	if(modifiersAsButtons)
		numButtonStates+=numModifierKeys;
	int numValuators=1<<numModifierKeys;
	
	/* Create new input device: */
	InputDevice* newDevice=inputDeviceManager->createInputDevice("Mouse",InputDevice::TRACK_POS|InputDevice::TRACK_DIR,numButtonStates,numValuators,true);
	
	/* Store the input device: */
	inputDevices[0]=newDevice;
	
	/* Retrieve the keyboard toggle key symbol: */
	keyboardModeToggleKey=KeyMapper::getQualifiedKey(configFileSection.retrieveValue<std::string>("./keyboardModeToggleKey","F1"));
	
	/* Create the control key map: */
	const int controlKeyMapSize=sizeof(rawControlKeyMap)/sizeof(ControlKeyMapItem);
	for(int i=0;i<controlKeyMapSize;++i)
		controlKeyMap.setEntry(ControlKeyMap::Entry(rawControlKeyMap[i].qk,rawControlKeyMap[i].tce));
	
	/* Initialize button and valuator states: */
	buttonStates=new bool[numButtonStates];
	for(int i=0;i<numButtonStates;++i)
		buttonStates[i]=false;
	numMouseWheelTicks=new int[numValuators];
	for(int i=0;i<numValuators;++i)
		numMouseWheelTicks[i]=0;
	
	/* Initialize the mouse position: */
	mousePos[0]=mousePos[1]=0;
	
	/* Check if this adapter is supposed to draw a fake mouse cursor: */
	fakeMouseCursor=configFileSection.retrieveValue<bool>("./fakeMouseCursor",fakeMouseCursor);
	if(fakeMouseCursor)
		{
		/* Enable the device's glyph as a cursor: */
		Glyph& deviceGlyph=inputDeviceManager->getInputGraphManager()->getInputDeviceGlyph(newDevice);
		deviceGlyph.enable();
		deviceGlyph.setGlyphType(Glyph::CURSOR);
		}
	}

InputDeviceAdapterMouse::~InputDeviceAdapterMouse(void)
	{
	delete[] buttonKeysyms;
	delete[] modifierKeysyms;
	delete[] buttonStates;
	delete[] numMouseWheelTicks;
	}

std::string InputDeviceAdapterMouse::getFeatureName(const InputDeviceFeature& feature) const
	{
	/* Check for forwarded modifier key buttons: */
	if(modifiersAsButtons&&feature.isButton()&&feature.getIndex()>=(numButtons+numButtonKeys)*(1<<numModifierKeys))
		{
		/* Return the name of the forwarded modifier key itself: */
		return KeyMapper::getName(modifierKeysyms[feature.getIndex()-(numButtons+numButtonKeys)*(1<<numModifierKeys)]);
		}
	
	std::string result;
	
	/* Calculate the feature's modifier mask: */
	int featureModifierMask=0x0;
	if(feature.isButton())
		featureModifierMask=feature.getIndex()/(numButtons+numButtonKeys);
	if(feature.isValuator())
		featureModifierMask=feature.getIndex();
	
	/* Create the feature's modifier prefix: */
	for(int i=0;i<numModifierKeys;++i)
		if(featureModifierMask&(0x1<<i))
			{
			/* Append the modifier key's name to the prefix: */
			result.append(KeyMapper::getName(modifierKeysyms[i]));
			result.push_back('+');
			}
	
	/* Append the feature's name: */
	if(feature.isButton())
		{
		int buttonIndex=feature.getIndex()%(numButtons+numButtonKeys);
		
		/* Check if the button is a mouse button or a button key: */
		if(buttonIndex<numButtons)
			{
			/* Append a mouse button name: */
			char buttonName[40];
			snprintf(buttonName,sizeof(buttonName),"Mouse%d",buttonIndex+1);
			result.append(buttonName);
			}
		else
			{
			/* Append a button key name: */
			result.append(KeyMapper::getName(buttonKeysyms[buttonIndex-numButtons]));
			}
		}
	if(feature.isValuator())
		result.append("MouseWheel");
	
	return result;
	}

int InputDeviceAdapterMouse::getFeatureIndex(InputDevice* device,const char* featureName) const
	{
	/* Extract a modifier key mask from the feature name: */
	int featureModifierKeyMask=0x0;
	const char* fPtr=featureName;
	while(true)
		{
		/* Find the next prefix separator: */
		const char* pref;
		for(pref=fPtr;*pref!='\0'&&*pref!='+';++pref)
			;
		if(*pref=='\0')
			break;
		
		/* Parse the prefix key name: */
		int prefixKeysym=-1;
		try
			{
			prefixKeysym=KeyMapper::getKeysym(std::string(fPtr,pref));
			}
		catch(std::runtime_error)
			{
			return -1;
			}
		
		/* Match the prefix's key symbol against a modifier key symbol: */
		int modifierIndex;
		for(modifierIndex=0;modifierIndex<numModifierKeys&&modifierKeysyms[modifierIndex]!=prefixKeysym;++modifierIndex)
			;
		if(modifierIndex<numModifierKeys)
			featureModifierKeyMask|=0x1<<modifierIndex;
		else
			return -1;
		
		/* Skip the prefix and continue: */
		fPtr=pref+1;
		}
	
	/* Check if the feature suffix matches a mouse feature or a button key: */
	if(strncasecmp(fPtr,"Mouse",5)==0)
		{
		fPtr+=5;
		
		/* Check if the feature is the mouse wheel or a mouse button: */
		if(strcasecmp(fPtr,"Wheel")==0)
			{
			/* Return the mouse wheel feature: */
			return device->getValuatorFeatureIndex(featureModifierKeyMask);
			}
		else
			{
			/* Return a mouse button feature: */
			int buttonIndex=atoi(fPtr)-1;
			if(buttonIndex<numButtons)
				return device->getButtonFeatureIndex((numButtons+numButtonKeys)*featureModifierKeyMask+buttonIndex);
			}
		}
	else
		{
		/* Parse the suffix key name: */
		int suffixKeysym=-1;
		try
			{
			suffixKeysym=KeyMapper::getKeysym(fPtr);
			}
		catch(std::runtime_error)
			{
			return -1;
			}
		
		/* Match the suffix key symbol against a button key symbol: */
		for(int i=0;i<numButtonKeys;++i)
			if(buttonKeysyms[i]==suffixKeysym)
				return device->getButtonFeatureIndex((numButtons+numButtonKeys)*featureModifierKeyMask+numButtons+i);
		
		if(modifiersAsButtons&&featureModifierKeyMask==0x0)
			{
			/* Match the suffix key symbol against a modifier key symbol: */
			for(int i=0;i<numModifierKeys;++i)
				if(modifierKeysyms[i]==suffixKeysym)
					return device->getButtonFeatureIndex((numButtons+numButtonKeys)*(1<<numModifierKeys)+i);
			}
		}
	
	return -1;
	}

void InputDeviceAdapterMouse::updateInputDevices(void)
	{
	if(window!=0)
		{
		/* Set mouse device's transformation and device ray: */
		Point lastMousePos=inputDevices[0]->getPosition();
		window->updateMouseDevice(mousePos,inputDevices[0]);
		
		/* Calculate the mouse device's linear velocity: */
		inputDevices[0]->setLinearVelocity((inputDevices[0]->getPosition()-lastMousePos)/Vrui::getFrameTime());
		
		if(mouseLocked)
			{
			/* Move the mouse cursor back to the window center: */
			int windowCenter[2];
			window->getWindowCenterPos(windowCenter);
			if(mousePos[0]!=windowCenter[0]||mousePos[1]!=windowCenter[1])
				{
				for(int i=0;i<2;++i)
					mousePos[i]=windowCenter[i];
				window->setCursorPos(mousePos[0],mousePos[1]);
				
				/* Reset the mouse device's ray and transformation to the locked values: */
				inputDevices[0]->setDeviceRay(lockedRayDirection,lockedRayStart);
				inputDevices[0]->setTransformation(lockedTransformation);
				}
			}
		
		/* Set mouse device button states: */
		for(int i=0;i<numButtonStates;++i)
			inputDevices[0]->setButtonState(i,buttonStates[i]);
		
		/* Set mouse device valuator states: */
		int numValuators=1<<numModifierKeys;
		for(int i=0;i<numValuators;++i)
			{
			/* Convert the mouse wheel tick count into a valuator value (ugh): */
			double mouseWheelValue=double(numMouseWheelTicks[i])/3.0;
			if(mouseWheelValue<-1.0)
				mouseWheelValue=-1.0;
			else if(mouseWheelValue>1.0)
				mouseWheelValue=1.0;
			inputDevices[0]->setValuator(i,mouseWheelValue);
			
			/* If there were mouse ticks, request another Vrui frame in a short while because there will be no "no mouse ticks" message: */
			if(numMouseWheelTicks[i]!=0)
				scheduleUpdate(getApplicationTime()+0.1);
			numMouseWheelTicks[i]=0;
			}
		
		#if 0
		inputDevices[0]->setValuator(numValuators+0,Scalar(2)*mousePos[0]/window->getVRScreen()->getWidth()-Scalar(1));
		inputDevices[0]->setValuator(numValuators+1,Scalar(2)*mousePos[1]/window->getVRScreen()->getHeight()-Scalar(1));
		inputDevices[0]->setValuator(numValuators+2,0.0);
		inputDevices[0]->setValuator(numValuators+3,0.0);
		#endif
		}
	}

void InputDeviceAdapterMouse::setMousePosition(VRWindow* newWindow,const int newMousePos[2])
	{
	/* Set current mouse position: */
	window=newWindow;
	mousePos[0]=newMousePos[0];
	mousePos[1]=newMousePos[1];
	
	// requestUpdate();
	}

bool InputDeviceAdapterMouse::keyPressed(int keysym,int modifierMask,const char* string)
	{
	bool stateChanged=false;
	
	if(keyboardModeToggleKey.matches(keysym,modifierMask))
		{
		keyboardMode=!keyboardMode;
		if(fakeMouseCursor)
			{
			/* Change the glyph renderer's cursor type to a text cursor: */
			}
		else if(keyboardMode)
			{
			/* Change the cursor in all windows to a text cursor: */
			for(int i=0;i<getNumWindows();++i)
				{
				VRWindow* win=Vrui::getWindow(i);
				if(win!=0)
					{
					Cursor cursor=XCreateFontCursor(win->getContext().getDisplay(),XC_xterm);
					XDefineCursor(win->getContext().getDisplay(),win->getWindow(),cursor);
					XFreeCursor(win->getContext().getDisplay(),cursor);
					}
				}
			}
		else
			{
			/* Change the cursor in all windows back to the regular: */
			for(int i=0;i<getNumWindows();++i)
				{
				VRWindow* win=Vrui::getWindow(i);
				if(win!=0)
					XUndefineCursor(win->getContext().getDisplay(),win->getWindow());
				}
			}
		}
	else if(keyboardMode)
		{
		/* Process the key event: */
		ControlKeyMap::Iterator ckmIt=controlKeyMap.findEntry(KeyMapper::QualifiedKey(keysym,modifierMask));
		if(!ckmIt.isFinished())
			{
			/* Enqueue a text control event: */
			inputDeviceManager->getTextEventDispatcher()->textControl(ckmIt->getDest());
			}
		else if(string!=0&&string[0]!='\0')
			{
			/* Enqueue a text event: */
			inputDeviceManager->getTextEventDispatcher()->text(string);
			}
		
		stateChanged=true;
		}
	else
		{
		/* Check if the key is a button key: */
		int buttonIndex=getButtonIndex(keysym);
		if(buttonIndex>=0)
			{
			/* Set button state: */
			int stateIndex=(numButtons+numButtonKeys)*modifierKeyMask+numButtons+buttonIndex;
			stateChanged=changeButtonState(stateIndex,true);
			}
		
		/* Check if the key is a modifier key: */
		int modifierIndex=getModifierIndex(keysym);
		if(modifierIndex>=0)
			{
			/* Change current modifier mask: */
			changeModifierKeyMask(modifierKeyMask|(0x1<<modifierIndex));
			stateChanged=true;
			}
		}
	
	// requestUpdate();
	
	return stateChanged;
	}

bool InputDeviceAdapterMouse::keyReleased(int keysym)
	{
	bool stateChanged=false;
	
	if(!keyboardMode)
		{
		/* Check if the key is a button key: */
		int buttonIndex=getButtonIndex(keysym);
		if(buttonIndex>=0)
			{
			/* Set button state: */
			int stateIndex=(numButtons+numButtonKeys)*modifierKeyMask+numButtons+buttonIndex;
			stateChanged=changeButtonState(stateIndex,false);
			}
		
		/* Check if the key is a modifier key: */
		int modifierIndex=getModifierIndex(keysym);
		if(modifierIndex>=0)
			{
			/* Change current modifier mask: */
			changeModifierKeyMask(modifierKeyMask&~(0x1<<modifierIndex));
			stateChanged=true;
			}
		
		// requestUpdate();
		}
	
	return stateChanged;
	}

void InputDeviceAdapterMouse::resetKeys(const XKeymapEvent& event)
	{
	/* Calculate the new modifier key mask: */
	int newModifierKeyMask=0x0;
	for(int i=0;i<256;++i)
		if(event.key_vector[i>>3]&(0x1<<(i&0x7)))
			{
			/* Convert the keycode to a keysym: */
			XKeyEvent keyEvent;
			keyEvent.type=KeyPress;
			keyEvent.serial=event.serial;
			keyEvent.send_event=event.send_event;
			keyEvent.display=event.display;
			keyEvent.window=event.window;
			keyEvent.state=0x0;
			keyEvent.keycode=i;
			KeySym keysym=XLookupKeysym(&keyEvent,0);
			
			int modifierIndex=getModifierIndex(keysym);
			if(modifierIndex>=0)
				newModifierKeyMask|=0x1<<modifierIndex;
			}
	
	/* Set the new modifier key mask: */
	changeModifierKeyMask(newModifierKeyMask);
	
	/* Set the states of all button keys: */
	for(int i=0;i<numButtonKeys;++i)
		{
		int stateIndex=(numButtons+numButtonKeys)*modifierKeyMask+numButtons+i;
		changeButtonState(stateIndex,false);
		}
	for(int i=0;i<256;++i)
		if(event.key_vector[i>>3]&(0x1<<(i&0x7)))
			{
			/* Convert the keycode to a keysym: */
			XKeyEvent keyEvent;
			keyEvent.type=KeyPress;
			keyEvent.serial=event.serial;
			keyEvent.send_event=event.send_event;
			keyEvent.display=event.display;
			keyEvent.window=event.window;
			keyEvent.state=0x0;
			keyEvent.keycode=i;
			KeySym keysym=XLookupKeysym(&keyEvent,0);
			
			int buttonIndex=getButtonIndex(keysym);
			if(buttonIndex>=0)
				{
				int stateIndex=(numButtons+numButtonKeys)*modifierKeyMask+numButtons+buttonIndex;
				changeButtonState(stateIndex,true);
				}
			}
	
	// requestUpdate();
	}

bool InputDeviceAdapterMouse::setButtonState(int buttonIndex,bool newButtonState)
	{
	bool stateChanged=false;
	
	/* Check if given button is represented: */
	if(buttonIndex>=0&&buttonIndex<numButtons)
		{
		/* Set current button state: */
		int stateIndex=(numButtons+numButtonKeys)*modifierKeyMask+buttonIndex;
		stateChanged=changeButtonState(stateIndex,newButtonState);
		
		// requestUpdate();
		}
	
	return stateChanged;
	}

void InputDeviceAdapterMouse::incMouseWheelTicks(void)
	{
	++numMouseWheelTicks[modifierKeyMask];
	
	// requestUpdate();
	}

void InputDeviceAdapterMouse::decMouseWheelTicks(void)
	{
	--numMouseWheelTicks[modifierKeyMask];
	
	// requestUpdate();
	}

void InputDeviceAdapterMouse::lockMouse(void)
	{
	/* Do nothing if the mouse is already locked, or if the current window is unknown: */
	if(mouseLocked||window==0)
		return;
	
	mouseLocked=true;
	
	/* Remember the current mouse pointer position to restore it upon unlock: */
	for(int i=0;i<2;++i)
		lockedMousePos[i]=mousePos[i];
	
	/* Move the mouse pointer to the center of the current window: */
	window->getWindowCenterPos(mousePos);
	window->updateMouseDevice(mousePos,inputDevices[0]);
	inputDevices[0]->setLinearVelocity(Vector::zero);
	window->setCursorPos(mousePos[0],mousePos[1]);
	
	/* Hide the mouse cursor: */
	if(fakeMouseCursor)
		getInputGraphManager()->getInputDeviceGlyph(inputDevices[0]).disable();
	else
		window->hideCursor();
	
	/* Remember the mouse transformation and ray at the window center: */
	lockedRayDirection=inputDevices[0]->getDeviceRayDirection();
	lockedRayStart=inputDevices[0]->getDeviceRayStart();
	lockedTransformation=inputDevices[0]->getTransformation();
	}

void InputDeviceAdapterMouse::unlockMouse(void)
	{
	/* Do nothing if the mouse is not locked: */
	if(!mouseLocked)
		return;
	
	mouseLocked=false;
	
	/* Move the mouse pointer back to its pre-lock position: */
	for(int i=0;i<2;++i)
		mousePos[i]=lockedMousePos[i];
	window->setCursorPos(mousePos[0],mousePos[1]);
	window->updateMouseDevice(mousePos,inputDevices[0]);
	inputDevices[0]->setLinearVelocity(Vector::zero);
	
	/* Show the mouse cursor: */
	if(fakeMouseCursor)
		getInputGraphManager()->getInputDeviceGlyph(inputDevices[0]).enable();
	else
		window->showCursor();
	}

ONTransform getMouseScreenTransform(InputDeviceAdapterMouse* mouseAdapter,Scalar viewport[4])
	{
	/* Check if the mouse adapter is valid: */
	VRScreen* screen=0;
	if(mouseAdapter!=0&&mouseAdapter->getWindow()!=0)
		{
		/* Use the window associated with the mouse adapter: */
		VRWindow* window=mouseAdapter->getWindow();
		screen=window->getVRScreen();
		window->getScreenViewport(viewport);
		}
	else
		{
		/* Use the main screen: */
		screen=getMainScreen();
		screen->getViewport(viewport);
		}
	
	/* Return the screen's transformation: */
	return screen->getScreenTransformation();
	}

}
