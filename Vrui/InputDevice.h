/***********************************************************************
InputDevice - Class to represent input devices (6-DOF tracker with
associated buttons and valuators) in virtual reality environments.
Copyright (c) 2000-2014 Oliver Kreylos

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

#ifndef VRUI_INPUTDEVICE_INCLUDED
#define VRUI_INPUTDEVICE_INCLUDED

#include <Misc/CallbackList.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Ray.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Vrui/Geometry.h>

namespace Vrui {

class InputDevice // Class for input devices
	{
	/* Embedded classes: */
	public:
	enum TrackType // Data type for input device tracking capabilities
		{
		TRACK_NONE=0x0, // No tracking at all
		TRACK_POS=0x1, // 3D position
		TRACK_DIR=0x2, // One 3D direction
		TRACK_ORIENT=0x4 // Full 3D orientation
		};
	
	enum FeatureType // Enumerated type for feature types, i.e., buttons and valuators
		{
		BUTTON,VALUATOR
		};
	
	struct CallbackData:public Misc::CallbackData // Generic callback data for input device events
		{
		/* Elements: */
		public:
		InputDevice* inputDevice; // The device that caused the callback
		
		/* Constructors and destructors: */
		CallbackData(InputDevice* sInputDevice)
			:inputDevice(sInputDevice)
			{
			}
		};
	
	struct ButtonCallbackData:public CallbackData // Callback data for button events
		{
		/* Elements: */
		public:
		int buttonIndex; // Index of button that changed state
		bool newButtonState; // New state of that button
		
		/* Constructors and destructors: */
		ButtonCallbackData(InputDevice* sInputDevice,int sButtonIndex,bool sNewButtonState)
			:CallbackData(sInputDevice),
			 buttonIndex(sButtonIndex),newButtonState(sNewButtonState)
			{
			}
		};
	
	struct ValuatorCallbackData:public CallbackData // Callback data for valuator events
		{
		/* Elements: */
		public:
		int valuatorIndex; // Index of valuator that changed value
		double oldValuatorValue,newValuatorValue; // Old and new valuator values
		
		/* Constructors and destructors: */
		ValuatorCallbackData(InputDevice* sInputDevice,int sValuatorIndex,double sOldValuatorValue,double sNewValuatorValue)
			:CallbackData(sInputDevice),
			 valuatorIndex(sValuatorIndex),oldValuatorValue(sOldValuatorValue),newValuatorValue(sNewValuatorValue)
			{
			}
		};
	
	/* Elements: */
	private:
	char* deviceName; // Arbitrary label to identify input devices
	int trackType; // Bitfield of tracking capabilities
	int numButtons; // Number of buttons on that device
	int numValuators; // Number of valuators on that device
	
	/* Callback management: */
	Misc::CallbackList trackingCallbacks; // List of tracking callbacks
	Misc::CallbackList* buttonCallbacks; // List of button callbacks for each button
	Misc::CallbackList* valuatorCallbacks; // List of valuator callbacks for each valuator
	
	/* Current device state: */
	Vector deviceRayDirection; // Preferred direction of ray devices in device coordinates
	Scalar deviceRayStart; // Ray parameter value from which the device ray is considered valid
	TrackerState transformation; // Full (orthonormal) transformation of locator device
	Vector linearVelocity,angularVelocity; // Velocities of locator device in physical units/second and radians/second, respectively
	bool* buttonStates; // Array of button press state(s)
	double* valuatorValues; // Array of valuator values, normalized from -1 to 1
	
	/* State for disabling callbacks: */
	bool callbacksEnabled; // Flag if callbacks are enabled
	bool* savedButtonStates; // Button states are saved at the time callbacks are disabled
	double* savedValuatorValues; // Valuator values are saved at the time callbacks are disabled
	
	/* Constructors and destructors: */
	public:
	InputDevice(void);
	InputDevice(const char* sDeviceName,int sTrackType,int sNumButtons =0,int sNumValuators =0);
	InputDevice(const InputDevice& source); // Prohibit copy constructor
	private:
	InputDevice& operator=(const InputDevice& source); // Prohibit assignment operator
	public:
	~InputDevice(void);
	
	/* Methods: */
	InputDevice& set(const char* sDeviceName,int sTrackType,int sNumButtons =0,int sNumValuators =0); // Changes input device's layout after creation
	void setTrackType(int newTrackType); // Overrides the tracking type of an existing input device
	
	/* Device layout access methods: */
	const char* getDeviceName(void) const
		{
		return deviceName;
		}
	int getTrackType(void) const
		{
		return trackType;
		}
	bool hasPosition(void) const
		{
		return trackType&TRACK_POS;
		}
	bool hasDirection(void) const
		{
		return trackType&TRACK_DIR;
		}
	bool hasOrientation(void) const
		{
		return trackType&TRACK_ORIENT;
		}
	bool isPositionDevice(void) const
		{
		return trackType==TRACK_POS;
		}
	bool isRayDevice(void) const
		{
		return trackType==(TRACK_POS|TRACK_DIR);
		}
	bool is6DOFDevice(void) const
		{
		return trackType==(TRACK_POS|TRACK_DIR|TRACK_ORIENT);
		}
	int getNumButtons(void) const
		{
		return numButtons;
		}
	int getNumValuators(void) const
		{
		return numValuators;
		}
	
	/* Feature-based accessor methods: */
	int getNumFeatures(void) const // Returns the number of buttons and valuators
		{
		return numButtons+numValuators;
		}
	FeatureType getFeatureType(int featureIndex) const // Returns the type of the given feature
		{
		return featureIndex<numButtons?BUTTON:VALUATOR;
		}
	bool isFeatureButton(int featureIndex) const // Returns true if the given feature is a button
		{
		return featureIndex<numButtons;
		}
	bool isFeatureValuator(int featureIndex) const // Returns true if the given feature is a valuator
		{
		return featureIndex>=numButtons;
		}
	int getFeatureIndex(FeatureType featureType,int featureTypeIndex) const // Returns the feature index of the given button or valuator
		{
		if(featureType==BUTTON)
			return featureTypeIndex;
		else
			return numButtons+featureTypeIndex;
		}
	int getButtonFeatureIndex(int buttonIndex) const // Returns the feature index of the given button
		{
		return buttonIndex;
		}
	int getValuatorFeatureIndex(int valuatorIndex) const // Returns the feature index for the given valuator
		{
		return numButtons+valuatorIndex;
		}
	int getFeatureTypeIndex(int featureIndex) const // Returns the index of a feature among features of its type, i.e., returns raw button or valuator index
		{
		if(featureIndex<numButtons)
			return featureIndex;
		else
			return featureIndex-numButtons;
		}
	
	/* Callback registration methods: */
	Misc::CallbackList& getTrackingCallbacks(void)
		{
		return trackingCallbacks;
		}
	Misc::CallbackList& getButtonCallbacks(int buttonIndex)
		{
		return buttonCallbacks[buttonIndex];
		}
	Misc::CallbackList& getValuatorCallbacks(int valuatorIndex)
		{
		return valuatorCallbacks[valuatorIndex];
		}
	Misc::CallbackList& getFeatureCallbacks(int featureIndex) // Returns callback list for button or valuator feature
		{
		if(featureIndex<numButtons)
			return buttonCallbacks[featureIndex];
		else
			return valuatorCallbacks[featureIndex-numButtons];
		}
	
	/* Device state manipulation methods: */
	void setDeviceRay(const Vector& newDeviceRayDirection,Scalar newDeviceRayStart); // Sets input device's ray direction and starting parameter in device coordinates
	void setTransformation(const TrackerState& newTransformation);
	void setLinearVelocity(const Vector& newLinearVelocity)
		{
		linearVelocity=newLinearVelocity;
		}
	void setAngularVelocity(const Vector& newAngularVelocity)
		{
		angularVelocity=newAngularVelocity;
		}
	void copyTrackingState(const InputDevice* source); // Copies the entire tracking (but not button nor valuator) state from the given source device
	void clearButtonStates(void);
	void setButtonState(int index,bool newButtonState);
	void setSingleButtonPressed(int index);
	void setValuator(int index,double value);
	
	/* Current state access methods: */
	const Vector& getDeviceRayDirection(void) const // Returns the device ray direction in device coordinates
		{
		return deviceRayDirection;
		}
	Scalar getDeviceRayStart(void) const
		{
		return deviceRayStart;
		}
	Point getPosition(void) const
		{
		return transformation.getOrigin();
		}
	Vector getRayDirection(void) const // Returns the device ray direction in physical coordinates
		{
		return transformation.transform(deviceRayDirection);
		}
	Ray getRay(void) const // Returns the device's ray in physical coordinates
		{
		/* Create the ray in physical coordinates: */
		Ray result(transformation.getOrigin(),transformation.transform(deviceRayDirection));
		
		/* Offset the ray's origin by the ray start parameter: */
		result.setOrigin(result.getOrigin()+result.getDirection()*deviceRayStart);
		
		return result;
		}
	const TrackerState::Rotation& getOrientation(void) const
		{
		return transformation.getRotation();
		}
	const TrackerState& getTransformation(void) const
		{
		return transformation;
		}
	const Vector& getLinearVelocity(void) const
		{
		return linearVelocity;
		}
	const Vector& getAngularVelocity(void) const
		{
		return angularVelocity;
		}
	bool getButtonState(int index) const
		{
		return buttonStates[index];
		}
	double getValuator(int index) const
		{
		return valuatorValues[index];
		}
	
	/* Callback enable/disable methods: */
	void disableCallbacks(void);
	void enableCallbacks(void);
	};

}

#endif
