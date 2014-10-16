/***********************************************************************
IMU - Abstract base class for inertial measurement units.
Copyright (c) 2013-2014 Oliver Kreylos

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
		int timeStep; // Raw time step since last reported sample
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
		Scalar timeStep; // Time step since last reported sample in s
		};
	
	typedef Misc::FunctionCall<const CalibratedSample&> CalibratedSampleCallback; // Type of callback called when a new calibrated IMU sample arrives
	
	struct CalibrationData // Structure containing calibration data to convert from raw samples to calibrated samples in m/s^2, radians/s, and uT
		{
		/* Elements: */
		public:
		Scalar accelerometerFactor; // Nominal conversion factor from raw accelerometer measurements to m/s^2
		Matrix accelerometerMatrix; // Calibration matrix from raw accelerometer measurements to rectified measurements in m/s^2
		Scalar gyroscopeFactor; // Nominal conversion factor from raw gyroscope measurements to radians/s
		Matrix gyroscopeMatrix; // Calibration matrix from raw gyroscope measurements to rectified measurements in radians/s
		bool magnetometer; // Flag whether the IMU device has a magnetometer
		Scalar magnetometerFactor; // Nominal conversion factor from raw magnetometer measurements to uT
		Matrix magnetometerMatrix; // Calibration matrix from raw magnetometer measurements to rectified measurements in uT
		Scalar timeStepFactor; // Conversion factor from raw time step units to s
		
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
			
			calibratedSample.timeStep=Scalar(rawSample.timeStep)*timeStepFactor;
			}
		};
	
	/* Elements: */
	protected:
	CalibrationData calibrationData; // Calibration data for the IMU device
	RawSampleCallback* rawSampleCallback; // Callback called when a new raw sample arrives
	CalibratedSampleCallback* calibratedSampleCallback; // Callback called when a new calibrated sample arrives
	
	/* Protected methods: */
	void loadCalibrationData(IO::File& calibrationFile); // Loads device's calibration data from an already-open binary file
	void sendSample(const RawSample& sample); // Sends a new raw sample to all registered callbacks
	
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
	virtual void startStreamingRaw(RawSampleCallback* newRawSampleCallback); // Starts streaming raw sample data to the given callback function; will be called from background thread
	virtual void startStreamingCalibrated(CalibratedSampleCallback* newCalibratedSampleCallback); // Starts streaming calibrated sample data to the given callback function; will be called from background thread
	virtual void stopStreaming(void); // Stops streaming sample data
	};

#endif
