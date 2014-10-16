/***********************************************************************
InputDeviceAdapterIndexMap - Base class for input device adapters that
use index maps to translate from "flat" device states to Vrui input
devices.
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

#ifndef VRUI_INTERNAL_INPUTDEVICEADAPTERINDEXMAP_INCLUDED
#define VRUI_INTERNAL_INPUTDEVICEADAPTERINDEXMAP_INCLUDED

#include <Vrui/Internal/InputDeviceAdapter.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}

namespace Vrui {

class InputDeviceAdapterIndexMap:public InputDeviceAdapter
	{
	/* Elements: */
	protected:
	int numRawTrackers; // Number of input trackers
	int numRawButtons; // Number of input buttons
	int numRawValuators; // Number of input valuators
	int* trackerIndexMapping; // Mapping from input device tracker indices to raw device state tracker indices
	int** buttonIndexMapping; // Mapping from input device button indices to raw device state button indices
	int** valuatorIndexMapping; // Mapping from input device valuator indices to raw device state valuator indices
	
	/* Protected methods from InputDeviceAdapter: */
	void createIndexMappings(void); // Creates the three index maps after the number of input devices has been determined
	virtual void createInputDevice(int deviceIndex,const Misc::ConfigurationFileSection& configFileSection);
	void initializeAdapter(int newNumRawTrackers,int newNumRawButtons,int newNumRawValuators,const Misc::ConfigurationFileSection& configFileSection); // Initializes adapter by reading configuration file section
	
	/* Constructors and destructors: */
	public:
	InputDeviceAdapterIndexMap(InputDeviceManager* sInputDeviceManager) // Dummy constructor; creates uninitialized input device adapter
		:InputDeviceAdapter(sInputDeviceManager),
		 trackerIndexMapping(0),
		 buttonIndexMapping(0),
		 valuatorIndexMapping(0)
		{
		}
	virtual ~InputDeviceAdapterIndexMap(void);
	};

}

#endif
