/***********************************************************************
ArtDTrack - Class for ART DTrack tracking devices.
Copyright (c) 2004-2013 Oliver Kreylos

This file is part of the Vrui VR Device Driver Daemon (VRDeviceDaemon).

The Vrui VR Device Driver Daemon is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Vrui VR Device Driver Daemon is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Vrui VR Device Driver Daemon; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef ARTDTRACK_INCLUDED
#define ARTDTRACK_INCLUDED

#include <Comm/UDPSocket.h>

#include <VRDeviceDaemon/VRDevice.h>

class ArtDTrack:public VRDevice
	{
	/* Embedded classes: */
	private:
	typedef Vrui::VRDeviceState::TrackerState::PositionOrientation PositionOrientation;
	typedef PositionOrientation::Vector Vector;
	typedef Vector::Scalar VScalar;
	typedef PositionOrientation::Rotation Rotation;
	typedef Rotation::Scalar RScalar;
	
	public:
	enum DataFormat // Enumerated type for data formats
		{
		ASCII,BINARY
		};
	
	enum DeviceReportFormat // Enumerated type for device tracking data reporting formats
		{
		DRF_6D=0,DRF_6DF,DRF_6DF2,DRF_6DMT,DRF_GL,DRF_3D,DRF_NUMFORMATS
		};
	
	private:
	struct Device // Structure describing DTrack devices
		{
		/* Elements: */
		public:
		DeviceReportFormat reportFormat; // Device's report format
		int id; // Device's DTrack ID
		int numButtons; // Number of buttons associated with the device
		int firstButtonIndex; // Index of first button on device
		int numValuators; // Number of valuators associated with the device
		int firstValuatorIndex; // Index of first valuator on device
		};
	
	/* Elements: */
	bool useRemoteControl; // Flag whether to remote control the A.R.T. server to start/stop when Vrui applications start/stop
	Comm::UDPSocket* controlSocket; // DTrack control socket
	Comm::UDPSocket dataSocket; // DTrack data socket
	DataFormat dataFormat; // Format of tracking data stream
	Device* devices; // Array of tracked devices
	int maxDeviceId[DRF_NUMFORMATS]; // Largest ID of any configured tracked device for each report format
	int* deviceIdToIndex[DRF_NUMFORMATS]; // Arrays mapping from device IDs for each report format to device indices
	
	/* Private methods: */
	void processAsciiData(void); // Processes tracking data in ASCII format
	void processBinaryData(void); // Processes tracking data in binary format
	
	/* Protected methods: */
	protected:
	virtual void deviceThreadMethod(void);
	
	/* Constructors and destructors: */
	public:
	ArtDTrack(VRDevice::Factory* sFactory,VRDeviceManager* sDeviceManager,Misc::ConfigurationFile& configFile);
	virtual ~ArtDTrack(void);
	
	/* Methods: */
	virtual void start(void);
	virtual void stop(void);
	};

#endif
