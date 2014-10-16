/***********************************************************************
VRDeviceDescriptor - Class describing the structure of an input device
represented by a VR device daemon.
Copyright (c) 2010-2013 Oliver Kreylos

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

#ifndef VRUI_INTERNAL_VRDEVICEDESCRIPTOR_INCLUDED
#define VRUI_INTERNAL_VRDEVICEDESCRIPTOR_INCLUDED

#include <string>
#include <Geometry/Vector.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}
namespace IO {
class File;
}

namespace Vrui {

class VRDeviceDescriptor
	{
	/* Embedded classes: */
	public:
	enum TrackType // Data type for input device tracking capabilities
		{
		TRACK_NONE=0x0, // No tracking at all
		TRACK_POS=0x1, // 3D position
		TRACK_DIR=0x2, // One 3D direction, defined in local coordinates by rayDirection
		TRACK_ORIENT=0x4 // Full 3D orientation
		};
	
	typedef Geometry::Vector<float,3> Vector; // Type for vectors
	
	/* Elements: */
	std::string name; // Device name
	int trackType; // Device's tracking type
	Vector rayDirection; // Device's preferred pointing direction in local device coordinates; ignored if trackType is TRACK_NONE
	float rayStart; // Starting parameter of device's ray in physical coordinate units; ignored if trackType is TRACK_NONE
	int trackerIndex; // Index of device's tracker in VR device daemon's flat namespace, or -1 if trackType is TRACK_NONE
	int numButtons; // Number of buttons on the device
	std::string* buttonNames; // Array of button names
	int* buttonIndices; // Array of indices of device's buttons in VR device daemon's flat namespace
	int numValuators; // Number of valuators on the device
	std::string* valuatorNames; // Array of valuator names
	int* valuatorIndices; // Array of indices of device's valuators in VR device daemon's flat namespace
	
	/* Constructors and destructors: */
	public:
	VRDeviceDescriptor(void); // Creates an empty descriptor
	VRDeviceDescriptor(int sNumButtons,int sNumValuators); // Creates a descriptor with the given number of buttons and valuators
	private:
	VRDeviceDescriptor(const VRDeviceDescriptor& source); // Prohibit copy constructor
	VRDeviceDescriptor& operator=(const VRDeviceDescriptor& source); // Prohibit assignment operator
	public:
	~VRDeviceDescriptor(void); // Destroys the descriptor
	
	/* Methods: */
	void write(IO::File& sink) const; // Writes the device descriptor to a data sink
	void read(IO::File& source); // Reads a device descriptor from a data source
	void save(Misc::ConfigurationFileSection& configFileSection) const; // Saves the device descriptor to the given configuration file section
	void load(const Misc::ConfigurationFileSection& configFileSection); // Loads the device descriptor from the given configuration file section
	};

}

#endif
