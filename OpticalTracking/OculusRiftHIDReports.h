/***********************************************************************
OculusRiftHIDReports - Classes defining the feature reports and raw
reports used by the Oculus Rift DK1 and DK2's raw HID protocol.
Copyright (c) 2014 Oliver Kreylos

This file is part of the optical/inertial sensor fusion tracking
package.

The optical/inertial sensor fusion tracking package is free software;
you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

The optical/inertial sensor fusion tracking package is distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the optical/inertial sensor fusion tracking package; if not, write
to the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef OCULUSRIFTHIDREPORTS_INCLUDED
#define OCULUSRIFTHIDREPORTS_INCLUDED

#include <Misc/SizedTypes.h>
#include <IO/FixedMemoryFile.h>

/* Forward declarations: */
namespace RawHID {
class Device;
}

class Unknown0x02 // Feature report 0x02: Unknown function; probably LED-related
	{
	/* Elements: */
	public:
	unsigned int value; // Unknown value; either 0x01 or 0x13
	
	/* Constructors and destructors: */
	Unknown0x02(unsigned int sValue); // Creates report with given value
	
	/* Methods: */
	unsigned int get(RawHID::Device& device); // Reads report from given raw HID device; returns command ID
	void set(RawHID::Device& device,unsigned int commandId) const; // Writes report to given raw HID device
	};

class SensorRange // Feature report 0x04: Get/set maximum value ranges of accelerometer, gyroscope, and magnetometer
	{
	/* Elements: */
	public:
	static const unsigned int accelRanges[4]; // Range scale factors supported by linear accelerometers
	static const unsigned int gyroRanges[4]; // Range scale factors supported by rate gyroscopes
	static const unsigned int magRanges[4]; // Range scale factors supported by magnetometers
	float accelFactor; // Scale factor to convert raw linear accelerometer measurements to m/(s^2)
	float gyroFactor; // Scale factor to convert raw rate gyroscope measurements to radians/s
	float magFactor; // Scale factor to convert raw magnetometer measurements to gauss
	
	/* Private methods: */
	private:
	static unsigned int findRangeMatch(float requested,float scale,const unsigned int* range,unsigned int rangeSize);
	
	/* Constructors and destructors: */
	public:
	SensorRange(void); // Creates maximum range values
	
	/* Methods: */
	unsigned int get(RawHID::Device& device); // Reads sensor ranges from given raw HID device; returns command ID
	void set(RawHID::Device& device,unsigned int commandId) const; // Writes sensor ranges to given raw HID device
	};

class KeepAliveDK1 // Feature report 0x08: Get/set keep-alive interval for Rift DK1
	{
	/* Elements: */
	public:
	unsigned int interval; // Keep-alive interval in ms
	
	/* Constructors and destructors: */
	KeepAliveDK1(unsigned int sInterval =10000U); // Creates a keep-alive interval with the given timeout
	
	/* Methods: */
	unsigned int get(RawHID::Device& device); // Reads keep-alive interval from given raw HID device; returns command ID
	void set(RawHID::Device& device,unsigned int commandId) const; // Writes keep-alive interval to given raw HID device
	};

class DisplayInfo // Feature report 0x09: Get display information
	{
	/* Elements: */
	public:
	unsigned int distortionType;
	unsigned int screenResolution[2];
	float screenSize[2];
	float screenCenterY;
	float lensDistanceX;
	float eyePos[2];
	float distortionCoeffs[6];
	
	/* Constructors and destructors: */
	DisplayInfo(void); // Creates default display information for Rift DK1
	
	/* Methods: */
	unsigned int get(RawHID::Device& device); // Reads display information from given raw HID device; returns command ID
	};

class Unknown0x0a // Feature report 0x0a: Unknown function; get-only
	{
	/* Constructors and destructors: */
	Unknown0x0a(void); // Creates report
	
	/* Methods: */
	unsigned int get(RawHID::Device& device); // Reads report from given raw HID device; returns command ID
	};

class LEDControl // Feature report 0x0c: Get/set LED control settings
	{
	/* Elements: */
	public:
	unsigned int pattern;
	bool enable;
	bool autoIncrement;
	bool useCarrier;
	bool syncInput;
	bool vsyncLock;
	bool customPattern;
	unsigned int exposureLength;
	unsigned int frameInterval;
	unsigned int vsyncOffset;
	unsigned int dutyCycle;
	
	/* Constructors and destructors: */
	public:
	LEDControl(void); // Creates default LED control settings with LEDs turned off
	
	/* Methods: */
	unsigned int get(RawHID::Device& device); // Reads LED control settings from given raw HID device; returns command ID
	void set(RawHID::Device& device,unsigned int commandId) const; // Writes LED control settings to given raw HID device
	};

class Unknown0x0d // Feature report 0x0d: Unknown function; get-only
	{
	/* Constructors and destructors: */
	Unknown0x0d(void); // Creates report
	
	/* Methods: */
	unsigned int get(RawHID::Device& device); // Reads report from given raw HID device; returns command ID
	};

class CameraIntrinsicParameters // Feature report 0x0e: Get tracking camera's intrinsic parameters?
	{
	/* Elements: */
	public:
	float matrix[3][4]; // Some 3x4 matrix maybe?
	
	/* Constructors and destructors: */
	public:
	CameraIntrinsicParameters(void); // Creates a default camera intrinsic parameter report
	
	/* Methods: */
	unsigned int get(RawHID::Device& device); // Reads camera intrinsic parameters from given raw HID device; returns command ID
	};

class LEDPosition // Feature report 0x0f: Get 3D LED position in HMD space
	{
	/* Elements: */
	public:
	unsigned int numReports;
	unsigned int reportIndex;
	bool isLed;
	float pos[3];
	float dir[3];
	
	/* Constructors and destructors: */
	public:
	LEDPosition(void); // Creates a default LED position report
	
	/* Methods: */
	unsigned int get(RawHID::Device& device); // Reads LED position from given raw HID device; returns command ID
	};

class KeepAliveDK2 // Feature report 0x11: Get/set keep-alive interval for Rift DK2
	{
	/* Elements: */
	public:
	unsigned int interval; // Keep-alive interval in ms
	
	/* Constructors and destructors: */
	KeepAliveDK2(unsigned int sInterval =10000U); // Creates a keep-alive interval with the given timeout
	
	/* Methods: */
	unsigned int get(RawHID::Device& device); // Reads keep-alive interval from given raw HID device; returns command ID
	void set(RawHID::Device& device,unsigned int commandId) const; // Writes keep-alive interval to given raw HID device
	};

class SerialNumber // Feature report 0x13: Get Oculus Rift DK2's serial number
	{
	/* Elements: */
	char serialNumber[21]; // NUL-terminated serial number
	
	/* Constructors and destructors: */
	SerialNumber(void); // Creates report
	
	/* Methods: */
	unsigned int get(RawHID::Device& device); // Reads serial number from given raw HID device; returns command ID
	};

class LensConfiguration // Feature report 0x16: Get lens configuration (group of two reports)
	{
	/* Elements: */
	public:
	unsigned int numReports;
	unsigned int reportIndex;
	unsigned int version;
	float r2Max;
	float catmullRom[11];
	float pixelSize;
	float eyeRelief;
	float redPolynomial[2];
	float bluePolynomial[2];
	
	/* Constructors and destructors: */
	LensConfiguration(void); // Creates default lens configuration
	
	/* Methods: */
	unsigned int get(RawHID::Device& device); // Reads lens configuration data from given raw HID device; returns command ID
	};

class SensorData // Input report: Receive sensor data from Rift's IMU
	{
	/* Embedded classes: */
	public:
	struct SensorSample
		{
		/* Elements: */
		public:
		int accel[3];
		int gyro[3];
		};
	
	/* Elements: */
	private:
	IO::FixedMemoryFile pktBuffer; // Buffer to unpack sensor data messages
	public:
	unsigned int numSamples;
	Misc::UInt16 timeStamp;
	unsigned int temperature;
	SensorSample samples[3];
	int mag[3];
	
	/* Constructors and destructors: */
	public:
	SensorData(void); // Initializes sensor data structure to receive data
	
	/* Methods: */
	void get(RawHID::Device& device); // Reads next sensor data packet from given raw HID device
	};

#endif
