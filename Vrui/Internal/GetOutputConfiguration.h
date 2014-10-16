/***********************************************************************
GetOutputConfiguration - Helper function to find the physical size and
panning domain of an output connector or connected output device using
the XRANDR extension.
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

#ifndef VRUI_INTERNAL_GETOUTPUTCONFIGURATION_INCLUDED
#define VRUI_INTERNAL_GETOUTPUTCONFIGURATION_INCLUDED

/* Forward declarations: */
typedef struct _XDisplay Display;

namespace Vrui {

struct OutputConfiguration // Structure describing the physical size and panning domain of an output
	{
	/* Elements: */
	public:
	int sizeMm[2]; // Width and height of the connected output device in mm, if known
	int domainOrigin[2]; // Origin of the output device's pixel rectangle in the virtual screen
	int domainSize[2]; // Width and height of the output device's pixel rectangle in the virtual screen
	};

OutputConfiguration getOutputConfiguration(Display* display,const char* outputName); // Returns the configuration of an output of the given name, or the default output if no match is found

}

#endif
