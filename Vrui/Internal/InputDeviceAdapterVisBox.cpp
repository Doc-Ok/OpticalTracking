/***********************************************************************
InputDeviceAdapterVisBox - Class to connect the VisBox head tracking
daemon to a Vrui application.
Copyright (c) 2007-2010 Oliver Kreylos

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

#include <Vrui/Internal/InputDeviceAdapterVisBox.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string>
#include <Misc/ThrowStdErr.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Geometry/Vector.h>
#include <Geometry/Rotation.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Geometry/GeometryValueCoders.h>
#include <Vrui/Geometry.h>
#include <Vrui/Vrui.h>
#include <Vrui/GlyphRenderer.h>
#include <Vrui/InputDevice.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/InputGraphManager.h>

namespace Vrui {

/*****************************************
Methods of class InputDeviceAdapterVisBox:
*****************************************/

InputDeviceAdapterVisBox::InputDeviceAdapterVisBox(InputDeviceManager* sInputDeviceManager,const Misc::ConfigurationFileSection& configFileSection)
	:InputDeviceAdapter(sInputDeviceManager),
	 xyzhpr((const float*)-1)
	{
	/* Retrieve the shared memory key from the configuration file: */
	key_t sharedMemoryKey=key_t(configFileSection.retrieveValue<int>("./sharedMemoryKey",0xDEAD));
	
	/* Try attaching to the shared memory segment: */
	int sharedMemoryID=shmget(sharedMemoryKey,6*sizeof(float),0777);
	if(sharedMemoryID<0)
		Misc::throwStdErr("InputDeviceAdapterVisBox::InputDeviceAdapterVisBox: Could not attach to shared memory segment using key %x",int(sharedMemoryKey));
	
	/* Get the pointer to the tracker state variables: */
	xyzhpr=reinterpret_cast<const float*>(shmat(sharedMemoryID,0,SHM_RDONLY));
	if(xyzhpr==(const float*)-1)
		Misc::throwStdErr("InputDeviceAdapterVisBox::InputDeviceAdapterVisBox: Could not map shared memory segment using key %x",int(sharedMemoryKey));
	
	/* Allocate new adapter state arrays: */
	numInputDevices=1;
	inputDevices=new InputDevice*[numInputDevices];
	
	/* Create new input device: */
	std::string deviceName=configFileSection.retrieveString("./name");
	inputDevices[0]=inputDeviceManager->createInputDevice(deviceName.c_str(),InputDevice::TRACK_POS|InputDevice::TRACK_DIR|InputDevice::TRACK_ORIENT,0,0,true);
	inputDevices[0]->setDeviceRay(configFileSection.retrieveValue<Vector>("./deviceRayDirection",Vector(0,1,0)),configFileSection.retrieveValue<Scalar>("./deviceRayStart",-getInchFactor()));
	
	/* Initialize the new device's glyph from the current configuration file section: */
	Glyph& deviceGlyph=inputDeviceManager->getInputGraphManager()->getInputDeviceGlyph(inputDevices[0]);
	deviceGlyph.configure(configFileSection,"./deviceGlyphType","./deviceGlyphMaterial");
	
	/* Set device's linear and angular velocities to zero, because we don't know any better: */
	inputDevices[0]->setLinearVelocity(Vector::zero);
	inputDevices[0]->setAngularVelocity(Vector::zero);
	}

InputDeviceAdapterVisBox::~InputDeviceAdapterVisBox(void)
	{
	/* Detach from the shared memory segment: */
	if(xyzhpr!=(const float*)-1)
		shmdt(xyzhpr);
	}

void InputDeviceAdapterVisBox::updateInputDevices(void)
	{
	/*********************************************************************
	Convert the most recent xyzhpr values from the shared memory segment
	into a tracker state:
	*********************************************************************/
	
	/* Translation vector is straightforward: */
	Vector translation=Vector(Scalar(xyzhpr[0]),Scalar(xyzhpr[1]),Scalar(xyzhpr[2]));
	
	/* To assemble the orientation, we assume all angles are in degrees, and the order of rotations is as follows: */
	Rotation rotation=Rotation::rotateZ(Math::rad(xyzhpr[3]));
	rotation*=Rotation::rotateX(Math::rad(xyzhpr[4]));
	rotation*=Rotation::rotateY(Math::rad(xyzhpr[5]));
	
	inputDevices[0]->setTransformation(TrackerState(translation,rotation));
	}

}
