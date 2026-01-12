/***********************************************************************
OculusRiftHIDReports - Classes defining the feature reports and raw
reports used by the Oculus Rift DK1 and DK2's raw HID protocol.
Copyright (c) 2014-2018 Oliver Kreylos

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

#include <iostream>
#include <Misc/SizedTypes.h>

#include "IMU.h"

/* Forward declarations: */
namespace RawHID {
class Device;
}
class TimeStampSource;

class SensorConfig // Feature report 0x02: Sensor configuration (time outs and such)
	{
	/* Embedded classes: */
	public:
	enum Flags // Enumerated type for configuration flags
		{
		RawFlags=0x01,
		TestCalibFlags=0x02,
		UseCalibFlags=0x04,
		AutoCalibFlags=0x08,
		MotionKeepAliveFlags=0x10,
		CommandKeepAliveFlags=0x20,
		SensorCoordinatesFlags=0x40
		};
	
	/* Elements: */
	unsigned int flags; // Sensor configuration flags
	unsigned int packetInterval; // Interval at which sensor packets are sent, to reduce USB load when headset is idle
	unsigned int sampleRate; // Sample rate of the headset's IMU in Hz
	
	/* Constructors and destructors: */
	SensorConfig(void); // Creates report with default values
	
	/* Methods: */
	unsigned int get(RawHID::Device& device); // Reads report from given raw HID device; returns command ID
	void set(RawHID::Device& device,unsigned int commandId) const; // Writes report to given raw HID device
	void print(std::ostream& os) const;
	};

class IMUCalibration // Feature report 0x03: Retrieve IMU calibration data
	{
	/* Elements: */
	public:
	float accelMatrix[3][4]; // Calibration matrix for raw accelerometer samples
	float gyroMatrix[3][4]; // Calibration matrix for raw gyroscope samples
	float temperature; // Current temperature
	
	/* Constructors and destructors: */
	IMUCalibration(void); // Creates uninitialized calibration data
	
	/* Methods: */
	unsigned int get(RawHID::Device& device); // Reads IMU calibration data from given raw HID device; returns command ID
	void print(std::ostream& os) const;
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
	void print(std::ostream& os) const;
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
	void print(std::ostream& os) const;
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
	bool keepLeds; // Flag to keep the LEDs going as well; if enabled, no IMU data will be sent if LEDs are off
	unsigned int interval; // Keep-alive interval in ms
	
	/* Constructors and destructors: */
	KeepAliveDK2(bool sKeepLeds =true,unsigned int sInterval =10000U); // Creates a keep-alive interval with the given LED flag and timeout
	
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
	void print(std::ostream& os) const;
	};

class RadioControl // Feature report 0x1a: Control the HMD radio
	{
	/* Elements: */
	public:
	Misc::UInt8 command[3]; // Three-byte command sequence
	
	/* Constructors and destructors: */
	RadioControl(unsigned int c0,unsigned int c1,unsigned int c2); // Creates report for the given command sequence
	
	/* Methods: */
	unsigned int get(RawHID::Device& device); // Reads radio control reply from given raw HID device; returns command ID
	void set(RawHID::Device& device,unsigned int commandId) const; // Writes radio control command to given raw HID device
	};

class RadioData // Feature report 0x1b: Receive data from the HMD radio
	{
	/* Embedded classes: */
	public:
	enum ReportType // Type of this report
		{
		MemoryReport=0x0a,
		FirmwareVersionReport=0x82,
		SerialNumberReport=0x88
		};
	
	struct Memory
		{
		/* Elements: */
		public:
		unsigned int start; // Start of memory block
		unsigned int length; // Length of memory block
		Misc::UInt8 data[20]; // Memory block contents
		};
	
	struct FirmwareVersion
		{
		/* Elements: */
		public:
		char date[12]; // Firmware data
		char version[11]; // Firmware version string
		};
	
	struct SerialNumber
		{
		/* Elements: */
		public:
		Misc::UInt32 address; // Radio address to link with cameras
		unsigned int deviceType;
		char serialNumber[15];
		};
	
	/* Elements: */
	ReportType reportType; // Type of data contained in this report
	union
		{
		SerialNumber serialNumber;
		FirmwareVersion firmwareVersion;
		Memory memory;
		};
	
	/* Constructors and destructors: */
	RadioData(ReportType sReportType); // Creates default radio data report of the given type
	
	/* Methods: */
	unsigned int get(RawHID::Device& device); // Reads radio data from given raw HID device; returns command ID
	};

class ComponentStatus // Feature report 0x1d: Enable/disable HMD components
	{
	/* Embedded classes: */
	public:
	enum Flags // Enumerated type for component flags
		{
		DisplayFlag=0x1,
		AudioFlag=0x2,
		LedsFlag=0x4
		};
	
	/* Elements: */
	bool displayEnabled; // Flag whether display is enabled
	bool audioEnabled; // Flag whether headset audio is enabled
	bool ledsEnabled; // Flag whether headset tracking LEDs are enabled
	
	/* Constructors and destructors: */
	ComponentStatus(bool sDisplayEnabled,bool sAudioEnabled,bool sLedsEnabled);
	
	/* Methods: */
	unsigned int get(RawHID::Device& device); // Reads component status from given raw HID device; returns command ID
	void set(RawHID::Device& device,unsigned int commandId) const; // Writes component status to given raw HID device
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
	Misc::UInt8 pktBuffer[62]; // Buffer to unpack sensor data messages
	public:
	static const TimeStamp sampleInterval=999779; // True update rate between IMU samples in nanoseconds
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
	unsigned int get(RawHID::Device& device,IMU::RawSample rawSamples[3],TimeStampSource& timeStampSource); // Reads next sensor data packet from given raw HID device directly into the given raw sample structures, updates given time stamp source; returns number of contained samples
	};

#endif
