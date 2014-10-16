/***********************************************************************
KeyMapper - Helper class to translate between human-readable key names
and modifier key names and X key symbols and modifier masks.
Copyright (c) 2014 Oliver Kreylos

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

#include <Vrui/Internal/KeyMapper.h>

#include <string.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <Misc/ThrowStdErr.h>

namespace Vrui {

namespace {

/***********************************************************************
We make our own modifier mask bits, because the ones we want don't exist
in X11 lingo, and Mac OS X blatantly lies about them anyway.
***********************************************************************/

#ifdef __linux__

/* For X11 on Linux, where Mod2Mask is NumLock: */

enum KeyModMasks
	{
	KMM_NONE=0x0,
	KMM_SHIFT=ShiftMask,
	KMM_CTRL=ControlMask,
	KMM_ALT=Mod1Mask,
	KMM_META=Mod3Mask,
	KMM_SUPER=Mod4Mask,
	KMM_HYPER=Mod5Mask,
	KMM_ALL=ShiftMask|ControlMask|Mod1Mask|Mod3Mask|Mod4Mask|Mod5Mask
	};

#endif

#ifdef __APPLE__

/* For X11 on Mac OS X: */

enum KeyModMasks
	{
	KMM_NONE=0x0,
	KMM_SHIFT=ShiftMask,
	KMM_CTRL=ControlMask,
	KMM_ALT=0x2000,
	KMM_META=Mod2Mask,
	KMM_SUPER=Mod3Mask,
	KMM_HYPER=Mod4Mask,
	KMM_ALL=ShiftMask|ControlMask|0x2000|Mod2Mask|Mod3Mask|Mod4Mask
	};

#endif

}

/****************************************
Methods of class KeyMapper::QualifiedKey:
****************************************/

KeyMapper::QualifiedKey::QualifiedKey(int sKeysym,int sModifierMask)
	:keysym(sKeysym),modifierMask(sModifierMask&KMM_ALL)
	{
	}

bool KeyMapper::QualifiedKey::matches(int otherKeysym,int otherModifierMask) const
	{
	return keysym==otherKeysym&&modifierMask==(otherModifierMask&KMM_ALL);
	}

/**********************************
Static elements of class KeyMapper:
**********************************/

const KeyMapper::KeyName KeyMapper::keyNames[]=
	{
	{"Space",XK_space},{"Tab",XK_Tab},{"Return",XK_Return},{"Backspace",XK_BackSpace},
	{"Left",XK_Left},{"Up",XK_Up},{"Right",XK_Right},{"Down",XK_Down},
	{"PageUp",XK_Page_Up},{"PageDown",XK_Page_Down},{"Home",XK_Home},{"End",XK_End},{"Insert",XK_Insert},{"Delete",XK_Delete},
	{"Num0",XK_KP_Insert},{"Num1",XK_KP_End},{"Num2",XK_KP_Down},{"Num3",XK_KP_Page_Down},{"Num4",XK_KP_Left},
	{"Num5",XK_KP_Begin},{"Num6",XK_KP_Right},{"Num7",XK_KP_Home},{"Num8",XK_KP_Up},{"Num9",XK_KP_Page_Up},
	{"Num/",XK_KP_Divide},{"Num*",XK_KP_Multiply},{"Num-",XK_KP_Subtract},{"Num+",XK_KP_Add},{"NumEnter",XK_KP_Enter},{"NumSep",XK_KP_Separator},
	{"LeftShift",XK_Shift_L},{"RightShift",XK_Shift_R},{"CapsLock",XK_Caps_Lock},{"LeftCtrl",XK_Control_L},{"RightCtrl",XK_Control_R},
	#ifdef __linux__
	{"LeftAlt",XK_Alt_L},{"RightAlt",XK_Alt_R},
	#endif
	#ifdef __APPLE__
	{"LeftAlt",XK_Mode_switch},{"RightAlt",XK_Mode_switch}, // X11 on Mac OS X treats both Alt keys the same
	#endif
	{"LeftMeta",XK_Meta_L},{"RightMeta",XK_Meta_R},
	{"LeftSuper",XK_Super_L},{"RightSuper",XK_Super_R},{"LeftHyper",XK_Hyper_L},{"RightHyper",XK_Hyper_R},
	{"F1",XK_F1},{"F2",XK_F2},{"F3",XK_F3},{"F4",XK_F4},{"F5",XK_F5},{"F6",XK_F6},
	{"F7",XK_F7},{"F8",XK_F8},{"F9",XK_F9},{"F10",XK_F10},{"F11",XK_F11},{"F12",XK_F12},
	{"Esc",XK_Escape},{"Print",XK_Print},{"ScrollLock",XK_Scroll_Lock},{"Pause",XK_Pause},{"Menu",XK_Menu}
	};

/**************************
Methods of class KeyMapper:
**************************/

int KeyMapper::getKeysym(const std::string& name)
	{
	int result=NoSymbol;
	
	/* Check for built-in legacy key names first: */
	const int keyNamesSize=int(sizeof(keyNames)/sizeof(KeyName));
	for(int i=0;i<keyNamesSize&&result==NoSymbol;++i)
		if(strcasecmp(name.c_str(),keyNames[i].name)==0)
			result=keyNames[i].keysym;
	
	/* Check for X key names: */
	if(result==NoSymbol)
		result=XStringToKeysym(name.c_str());
	
	if(result==NoSymbol)
		Misc::throwStdErr("Vrui::KeyMapper: Unknown key name \"%s\"",name.c_str());
	
	return result;
	}

std::string KeyMapper::getName(int keysym)
	{
	/* Check for built-in legacy key names first: */
	const int keyNamesSize=int(sizeof(keyNames)/sizeof(KeyName));
	for(int i=0;i<keyNamesSize;++i)
		if(keysym==keyNames[i].keysym)
			return keyNames[i].name;
	
	/* Check for X key names: */
	char* name=XKeysymToString(keysym);
	if(name!=0)
		return name;
	
	Misc::throwStdErr("Vrui::KeyMapper: Unknown key symbol %d",keysym);
	
	/* Never reached; just to make compiler happy: */
	return std::string();
	}

KeyMapper::QualifiedKey KeyMapper::getQualifiedKey(const std::string& name)
	{
	QualifiedKey result(NoSymbol,0x0);
	
	/* Parse modifier key prefixes: */
	const char* nPtr=name.c_str();
	while(true)
		{
		/* Find the next prefix separator: */
		const char* pref;
		for(pref=nPtr;*pref!='\0'&&*pref!='+';++pref)
			;
		if(*pref=='\0')
			break;
		
		/* Parse the prefix: */
		if(pref-nPtr==5&&strncasecmp(nPtr,"Shift",5)==0)
			result.modifierMask|=KMM_SHIFT;
		else if(pref-nPtr==4&&strncasecmp(nPtr,"Ctrl",4)==0)
			result.modifierMask|=KMM_CTRL;
		else if(pref-nPtr==3&&strncasecmp(nPtr,"Alt",3)==0)
			result.modifierMask|=KMM_ALT;
		else if(pref-nPtr==4&&strncasecmp(nPtr,"Meta",4)==0)
			result.modifierMask|=KMM_META;
		else if(pref-nPtr==5&&strncasecmp(nPtr,"Super",5)==0)
			result.modifierMask|=KMM_SUPER;
		else if(pref-nPtr==5&&strncasecmp(nPtr,"Hyper",5)==0)
			result.modifierMask|=KMM_HYPER;
		else
			Misc::throwStdErr("Vrui::KeyMapper: Unknown modifier key name %s",std::string(nPtr,pref).c_str());
		
		/* Skip the prefix separator: */
		nPtr=pref+1;
		}
	
	/* Parse the key name: */
	result.keysym=getKeysym(nPtr);
	
	return result;
	}

std::string KeyMapper::getName(const KeyMapper::QualifiedKey& qKey)
	{
	std::string result;
	
	/* Create the modifier key prefix: */
	if(qKey.modifierMask&KMM_SHIFT)
		result.append("Shift+");
	if(qKey.modifierMask&KMM_CTRL)
		result.append("Ctrl+");
	if(qKey.modifierMask&KMM_ALT)
		result.append("Alt+");
	if(qKey.modifierMask&KMM_META)
		result.append("Meta+");
	if(qKey.modifierMask&KMM_SUPER)
		result.append("Super+");
	if(qKey.modifierMask&KMM_HYPER)
		result.append("Hyper+");
	
	/* Append the key name: */
	result.append(getName(qKey.keysym));
	
	return result;
	}

}
