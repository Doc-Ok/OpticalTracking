/***********************************************************************
IMU - Abstract base class for inertial measurement units.
Copyright (c) 2013-2020 Oliver Kreylos

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

#ifndef IMU_INCLUDED
#define IMU_INCLUDED

#include <string>
#include <Misc/SizedTypes.h>
#include <Geometry/Vector.h>
#include <Geometry/Matrix.h>

/* Forward declarations: */
namespace Misc {
template <class ParameterParam>
class FunctionCall;
}
namespace IO {
class File;
}
typedef Misc::SInt32 TimeStamp; // Type for cyclic time stamps at microsecond resolution

class IMU
	{
	/* Embedded classes: */
	public:
	struct RawSample // Structure containing a raw sampling update from an inertial measurement unit
		{
		/* Elements: */
		public:
		int accelerometer[3]; // Raw accelerometer measurement
		int gyroscope[3]; // Raw gyroscopy measurement
		int magnetometer[3]; // Raw magnetometer measurement
		TimeStamp timeStamp; // Absolute time at which the sample was taken in microseconds
		bool warmup; // Flag if this sample was taken during the "warm-up" period, when time stamps are not yet reliable
		};
	
	typedef Misc::FunctionCall<const RawSample&> RawSampleCallback; // Type of callback called when a new raw IMU sample arrives
	
	typedef double Scalar; // Scalar type for calibrated and rectified sensor measurements
	typedef Geometry::Vector<Scalar,3> Vector; // Type for calibrated measurements
	typedef Geometry::Matrix<Scalar,3,4> Matrix; // Type for calibration and rectification matrices for accelerometer, gyroscope, and magnetometer measurements
	
	struct CalibratedSample // Structure containing a calibrated sampling update from an inertial measurement unit
		{
		/* Elements: */
		public:
		Vector accelerometer; // Calibrated and rectified accelerometer measurement in m/s^2
		Vector gyroscope; // Calibrated and rectified gyroscope measurement in radians/s
		Vector magnetometer; // Calibrated and rectified magnetometer measurement in uT
		TimeStamp timeStamp; // Absolute time at which the sample was taken in microseconds
		bool warmup; // Flag if this sample was taken during the "warm-up" period, when time stamps are not yet reliable
		};
	
	typedef Misc::FunctionCall<const CalibratedSample&> CalibratedSampleCallback; // Type of callback called when a new calibrated IMU sample arrives
	
	struct CalibrationData // Structure containing calibration data to convert from raw samples to calibrated samples in m/s^2, radians/s, and uT
		{
		/* Elements: */
		public:
		Matrix accelerometerMatrix; // Calibration matrix from raw accelerometer measurements to rectified measurements in m/s^2
		Matrix gyroscopeMatrix; // Calibration matrix from raw gyroscope measurements to rectified measurements in radians/s
		bool magnetometer; // Flag whether the IMU device has a magnetometer
		Matrix magnetometerMatrix; // Calibration matrix from raw magnetometer measurements to rectified measurements in uT
		
		/* Methods: */
		void calibrate(const RawSample& rawSample,CalibratedSample& calibratedSample) const // Calibrates a raw sample
			{
			for(int i=0;i<3;++i)
				calibratedSample.accelerometer[i]=Scalar(rawSample.accelerometer[0])*accelerometerMatrix(i,0)
		                                		 +Scalar(rawSample.accelerometer[1])*accelerometerMatrix(i,1)
		                                		 +Scalar(rawSample.accelerometer[2])*accelerometerMatrix(i,2)
		                                		 +accelerometerMatrix(i,3);
			
			for(int i=0;i<3;++i)
				calibratedSample.gyroscope[i]=Scalar(rawSample.gyroscope[0])*gyroscopeMatrix(i,0)
		                            		 +Scalar(rawSample.gyroscope[1])*gyroscopeMatrix(i,1)
		                            		 +Scalar(rawSample.gyroscope[2])*gyroscopeMatrix(i,2)
		                            		 +gyroscopeMatrix(i,3);
			
			if(magnetometer)
				{
				for(int i=0;i<3;++i)
					calibratedSample.magnetometer[i]=Scalar(rawSample.magnetometer[0])*magnetometerMatrix(i,0)
			                                		+Scalar(rawSample.magnetometer[1])*magnetometerMatrix(i,1)
			                                		+Scalar(rawSample.magnetometer[2])*magnetometerMatrix(i,2)
			                                		+magnetometerMatrix(i,3);
				}
			
			calibratedSample.timeStamp=rawSample.timeStamp;
			calibratedSample.warmup=rawSample.warmup;
			}
		};
	
	struct BatteryState // Structure to report a change to an inertial measurement unit's battery state
		{
		/* Elements: */
		public:
		int level; // Current battery charge level in percent
		bool charging; // Flag if the battery is currently charging
		bool chargingComplete; // Flag if the battery is fully charged
		};
	
	typedef Misc::FunctionCall<const BatteryState&> BatteryStateCallback; // Type of callback called when an inertial measurement unit's battery state changes
	
	/* Elements: */
	protected:
	CalibrationData calibrationData; // Calibration data for the IMU device
	RawSampleCallback* rawSampleCallback; // Callback called when a new raw sample arrives
	CalibratedSampleCallback* calibratedSampleCallback; // Callback called when a new calibrated sample arrives
	BatteryStateCallback* batteryStateCallback; // Callback called when an inertial measurement unit's battery state changes
	
	/* Protected methods: */
	static TimeStamp getTime(void); // Returns the current host time as an absolute time stamp at microsecond resolution
	void initCalibrationData(Scalar accelerometerScale,Scalar gyroscopeScale,Scalar magnetometerScale); // Initializes calibration data from nominal sensor scale factors
	void loadCalibrationData(IO::File& calibrationFile); // Loads device's calibration data from an already-open binary file
	void sendSample(const RawSample& sample); // Sends a new raw sample to all registered callbacks
	void sendBatteryState(int level,bool charging,bool chargingComplete); // Sends a battery state update to all registered callbacks
	
	/* Constructors and destructors: */
	IMU(void); // Default constructor
	private:
	IMU(const IMU& source); // Prohibit copy constructor
	IMU& operator=(const IMU& source); // Prohibit assignment operator
	public:
	virtual ~IMU(void); // Destroys an IMU object
	
	/* Methods: */
	const CalibrationData& getCalibrationData(void) const // Returns the device's calibration data structure
		{
		return calibrationData;
		}
	virtual std::string getSerialNumber(void) const =0; // Returns a unique serial number among all IMU devices
	virtual Scalar getAccelerometerScale(void) const =0; // Returns nominal scale factor to convert from raw accelerometer sample units to m/s^2
	virtual Scalar getGyroscopeScale(void) const =0; // Returns nominal scale factor to convert from raw gyroscope sample units to radians/s
	virtual Scalar getMagnetometerScale(void) const =0; // Returns nominal scale factor to convert from raw magnetometer units to uT
	virtual bool hasBattery(void) const; // Returns true if the IMU device has a battery
	virtual void setBatteryStateCallback(BatteryStateCallback* newBatteryStateCallback); // Sets a callback to be called when an inertial measurement unit's battery state changes
	virtual void startStreamingRaw(RawSampleCallback* newRawSampleCallback); // Starts streaming raw sample data to the given callback function; will be called from background thread
	virtual void startStreamingCalibrated(CalibratedSampleCallback* newCalibratedSampleCallback); // Starts streaming calibrated sample data to the given callback function; will be called from background thread
	virtual void stopStreaming(void); // Stops streaming sample data
	};

#endif
