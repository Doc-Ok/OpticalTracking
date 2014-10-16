/***********************************************************************
ToolInputLayout - Class to represent the input requirements of tools.
Copyright (c) 2004-2010 Oliver Kreylos

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

#ifndef VRUI_TOOLINPUTLAYOUT_INCLUDED
#define VRUI_TOOLINPUTLAYOUT_INCLUDED

namespace Vrui {

class ToolInputLayout // Class describing the input layout of a tool
	{
	/* Elements: */
	private:
	int numButtons; // Number of required buttons
	bool optionalButtons; // Flag if the tool can accept additional buttons
	int numValuators; // Number of required valuators
	bool optionalValuators; // Flag if the tool can accept additional valuators
	
	/* Constructors and destructors: */
	public:
	ToolInputLayout(void); // Creates empty layout
	
	/* Methods: */
	void setNumButtons(int newNumButtons,bool newOptionalButtons =false); // Sets the number of required and optional buttons
	void setNumValuators(int newNumValuators,bool newOptionalValuators =false); // Sets the number of required and optional valuators
	int getNumButtons(void) const // Returns the number of required buttons
		{
		return numButtons;
		}
	bool hasOptionalButtons(void) const // Returns true if the tool has optional button slots
		{
		return optionalButtons;
		}
	int getNumValuators(void) const // Returns the number of required valuators
		{
		return numValuators;
		}
	bool hasOptionalValuators(void) const // Returns true if the tool has optional valuator slots
		{
		return optionalValuators;
		}
	};

}

#endif
