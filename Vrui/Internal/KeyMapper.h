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

#ifndef VRUI_INTERNAL_KEYMAPPER_INCLUDED
#define VRUI_INTERNAL_KEYMAPPER_INCLUDED

#include <string>

namespace Vrui {

class KeyMapper
	{
	/* Embedded classes: */
	public:
	struct KeyName // Structure associating human-readable key names with X key symbols
		{
		/* Elements: */
		public:
		const char* name; // Human-readable key name
		int keysym; // X key symbol
		};
	
	struct QualifiedKey // Structure identifying a key with a specific modifier combination
		{
		/* Elements: */
		public:
		int keysym; // X key symbol
		int modifierMask; // X modifier mask
		
		/* Constructors and destructors: */
		QualifiedKey(int sKeysym,int sModifierMask); // Constructs qualified key, ignoring unsupported modifier bits
		
		/* Methods: */
		bool matches(int otherKeysym,int otherModifierMask) const; // Returns true if qualified key matches given key symbol and modifier mask, ignoring unsupported modifiers in the latter
		friend bool operator==(const QualifiedKey& qk1,const QualifiedKey& qk2)
			{
			return qk1.keysym==qk2.keysym&&qk1.modifierMask==qk2.modifierMask;
			}
		friend bool operator!=(const QualifiedKey& qk1,const QualifiedKey& qk2)
			{
			return qk1.keysym!=qk2.keysym||qk1.modifierMask!=qk2.modifierMask;
			}
		static size_t hash(const QualifiedKey& source,size_t tableSize)
			{
			return (source.keysym+(source.modifierMask<<24))%tableSize;
			}
		};
	
	/* Elements: */
	private:
	static const KeyName keyNames[]; // Array of key names for keys where the X name is not exactly human-readable
	
	/* Methods: */
	public:
	static int getKeysym(const std::string& name); // Returns the X key symbol for a key name
	static std::string getName(int keysym); // Returns the human-readable name for an X key symbol
	static QualifiedKey getQualifiedKey(const std::string& name); // Parses a qualified key name (Modifier1+...+ModifierN+Key name)
	static std::string getName(const QualifiedKey& qKey); // Returns a human-readable name for the given qualified key
	};

}

#endif
