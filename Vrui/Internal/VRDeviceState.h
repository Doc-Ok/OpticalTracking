/***********************************************************************
VRDeviceState - Class to represent the current state of a single or
multiple VR devices.
Copyright (c) 2002-2014 Oliver Kreylos

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

#ifndef VRUI_INTERNAL_VRDEVICESTATE_INCLUDED
#define VRUI_INTERNAL_VRDEVICESTATE_INCLUDED

#include <Misc/SizedTypes.h>
#include <Misc/ArrayMarshallers.h>
#include <IO/File.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Geometry/GeometryMarshallers.h>

namespace Vrui {

class VRDeviceState
	{
	/* Embedded classes: */
	public:
	struct TrackerState // Type for tracker states
		{
		/* Embedded classes: */
		public:
		typedef Geometry::OrthonormalTransformation<float,3> PositionOrientation; // Type for tracker position/orientation
		typedef Geometry::Vector<float,3> LinearVelocity; // Type for linear velocity vectors
		typedef Geometry::Vector<float,3> AngularVelocity; // Type for angular velocity vectors
		
		/* Elements: */
		PositionOrientation positionOrientation; // Current tracker position/orientation
		LinearVelocity linearVelocity; // Current linear velocity in units/s in physical space
		AngularVelocity angularVelocity; // Current angular velocity in radians/s in physical space
		};
	
	typedef bool ButtonState; // Type for button states
	typedef float ValuatorState; // Type for valuator states
	typedef Misc::UInt32 TimeStamp; // Type for device state time stamps in microseconds
	
	/* Elements: */
	private:
	int numTrackers; // Number of represented trackers
	TrackerState* trackerStates; // Array of current tracker states
	TimeStamp* trackerTimeStamps; // Time stamps of current tracker states
	int numButtons; // Number of represented buttons
	ButtonState* buttonStates; // Array of current button states
	int numValuators; // Number of represented valuators
	ValuatorState* valuatorStates; // Array of current valuator states
	
	/* Private methods: */
	void initState(void)
		{
		for(int i=0;i<numTrackers;++i)
			{
			trackerStates[i].positionOrientation=TrackerState::PositionOrientation::identity;
			trackerStates[i].linearVelocity=TrackerState::LinearVelocity::zero;
			trackerStates[i].angularVelocity=TrackerState::AngularVelocity::zero;
			trackerTimeStamps[i]=0U;
			}
		for(int i=0;i<numButtons;++i)
			buttonStates[i]=false;
		for(int i=0;i<numValuators;++i)
			valuatorStates[i]=ValuatorState(0);
		}
	
	/* Constructors and destructors: */
	public:
	VRDeviceState(void) // Creates empty device state
		:numTrackers(0),trackerStates(0),trackerTimeStamps(0),
		 numButtons(0),buttonStates(0),
		 numValuators(0),valuatorStates(0)
		{
		}
	VRDeviceState(int sNumTrackers,int sNumButtons,int sNumValuators) // Creates device state of given layout
		:numTrackers(sNumTrackers),trackerStates(new TrackerState[numTrackers]),trackerTimeStamps(new TimeStamp[numTrackers]),
		 numButtons(sNumButtons),buttonStates(new ButtonState[numButtons]),
		 numValuators(sNumValuators),valuatorStates(new ValuatorState[numValuators])
		{
		/* Initialize state arrays: */
		initState();
		}
	~VRDeviceState(void)
		{
		delete[] trackerStates;
		delete[] trackerTimeStamps;
		delete[] buttonStates;
		delete[] valuatorStates;
		}
	
	/* Methods: */
	void setLayout(int newNumTrackers,int newNumButtons,int newNumValuators) // Sets the number of represented trackers, buttons and valuators
		{
		/* Re-allocate state arrays: */
		if(numTrackers!=newNumTrackers)
			{
			delete[] trackerStates;
			delete[] trackerTimeStamps;
			numTrackers=newNumTrackers;
			trackerStates=new TrackerState[numTrackers];
			trackerTimeStamps=new TimeStamp[numTrackers];
			}
		if(numButtons!=newNumButtons)
			{
			delete[] buttonStates;
			numButtons=newNumButtons;
			buttonStates=new ButtonState[numButtons];
			}
		if(numValuators!=newNumValuators)
			{
			delete[] valuatorStates;
			numValuators=newNumValuators;
			valuatorStates=new ValuatorState[numValuators];
			}
		
		/* Initialize new state arrays: */
		initState();
		}
	int getNumTrackers(void) const // Returns number of represented trackers
		{
		return numTrackers;
		}
	int getNumButtons(void) const // Returns number of represented buttons
		{
		return numButtons;
		}
	int getNumValuators(void) const // Returns number of represented valuators
		{
		return numValuators;
		}
	const TrackerState& getTrackerState(int trackerIndex) const // Returns state of single tracker
		{
		return trackerStates[trackerIndex];
		}
	void setTrackerState(int trackerIndex,const TrackerState& newTrackerState) // Updates state of single tracker
		{
		trackerStates[trackerIndex]=newTrackerState;
		}
	TimeStamp getTrackerTimeStamp(int trackerIndex) const // Returns time stamp of current state of the given tracker
		{
		return trackerTimeStamps[trackerIndex];
		}
	void setTrackerTimeStamp(int trackerIndex,TimeStamp newTrackerTimeStamp) const // Updates time stamp of current state of the given tracker
		{
		trackerTimeStamps[trackerIndex]=newTrackerTimeStamp;
		}
	ButtonState getButtonState(int buttonIndex) const // Returns state of single button
		{
		return buttonStates[buttonIndex];
		}
	void setButtonState(int buttonIndex,ButtonState newButtonState) // Updates state of single button
		{
		buttonStates[buttonIndex]=newButtonState;
		}
	ValuatorState getValuatorState(int valuatorIndex) const // Returns state of single valuator
		{
		return valuatorStates[valuatorIndex];
		}
	void setValuatorState(int valuatorIndex,ValuatorState newValuatorState) // Updates state of single valuator
		{
		valuatorStates[valuatorIndex]=newValuatorState;
		}
	const TrackerState* getTrackerStates(void) const // Returns array of tracker states
		{
		return trackerStates;
		}
	TrackerState* getTrackerStates(void) // Ditto
		{
		return trackerStates;
		}
	const TimeStamp* getTrackerTimeStamps(void) const // Returns array of tracker state time stamps
		{
		return trackerTimeStamps;
		}
	TimeStamp* getTrackerTimeStamps(void) // Ditto
		{
		return trackerTimeStamps;
		}
	const ButtonState* getButtonStates(void) const // Returns array of button states
		{
		return buttonStates;
		}
	ButtonState* getButtonStates(void) // Ditto
		{
		return buttonStates;
		}
	const ValuatorState* getValuatorStates(void) const // Returns array of valuator states
		{
		return valuatorStates;
		}
	ValuatorState* getValuatorStates(void) // Ditto
		{
		return valuatorStates;
		}
	void writeLayout(IO::File& sink) const // Writes device state's layout to given data sink
		{
		sink.write<int>(numTrackers);
		sink.write<int>(numButtons);
		sink.write<int>(numValuators);
		}
	void readLayout(IO::File& source) // Reads device state's layout from given data source
		{
		int newNumTrackers=source.read<int>();
		int newNumButtons=source.read<int>();
		int newNumValuators=source.read<int>();
		setLayout(newNumTrackers,newNumButtons,newNumValuators);
		}
	void write(IO::File& sink,bool writeTimeStamps) const // Writes device state to given data sink
		{
		Misc::FixedArrayMarshaller<TrackerState>::write(trackerStates,numTrackers,sink);
		if(writeTimeStamps)
			Misc::FixedArrayMarshaller<TimeStamp>::write(trackerTimeStamps,numTrackers,sink);
		Misc::FixedArrayMarshaller<ButtonState>::write(buttonStates,numButtons,sink);
		Misc::FixedArrayMarshaller<ValuatorState>::write(valuatorStates,numValuators,sink);
		}
	void read(IO::File& source,bool readTimeStamps) const // Reads device state from given data source
		{
		Misc::FixedArrayMarshaller<TrackerState>::read(trackerStates,numTrackers,source);
		if(readTimeStamps)
			Misc::FixedArrayMarshaller<TimeStamp>::read(trackerTimeStamps,numTrackers,source);
		Misc::FixedArrayMarshaller<ButtonState>::read(buttonStates,numButtons,source);
		Misc::FixedArrayMarshaller<ValuatorState>::read(valuatorStates,numValuators,source);
		}
	};

}

namespace Misc {

template <>
class Marshaller<Vrui::VRDeviceState::TrackerState>
	{
	/* Embedded classes: */
	public:
	typedef Vrui::VRDeviceState::TrackerState Value;
	
	/* Methods: */
	static size_t getSize(const Value& value)
		{
		size_t result=Marshaller<Value::PositionOrientation>::getSize(value.positionOrientation);
		result+=Marshaller<Value::LinearVelocity>::getSize(value.linearVelocity);
		result+=Marshaller<Value::AngularVelocity>::getSize(value.angularVelocity);
		return result;
		}
	template <class DataSinkParam>
	static void write(const Value& value,DataSinkParam& sink)
		{
		Marshaller<Value::PositionOrientation>::write(value.positionOrientation,sink);
		Marshaller<Value::LinearVelocity>::write(value.linearVelocity,sink);
		Marshaller<Value::AngularVelocity>::write(value.angularVelocity,sink);
		}
	template <class DataSourceParam>
	static Value read(DataSourceParam& source)
		{
		Value result;
		result.positionOrientation=Marshaller<Value::PositionOrientation>::read(source);
		result.linearVelocity=Marshaller<Value::LinearVelocity>::read(source);
		result.angularVelocity=Marshaller<Value::AngularVelocity>::read(source);
		return result;
		}
	};

}

#endif
