/***********************************************************************
KinectRecorder - Class to save 3D video of a Vrui session recorded with
an InputDeviceDataSaver.
Copyright (c) 2011 Oliver Kreylos

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

#include <Vrui/Internal/KinectRecorder.h>

#include <string.h>
#include <Misc/FunctionCalls.h>
#include <Misc/File.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/CompoundValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Geometry/GeometryValueCoders.h>
#include <Kinect/USBDeviceList.h>
#include <Kinect/FrameBuffer.h>
#include <Kinect/KinectCamera.h>
#include <Kinect/KinectFrameSaver.h>

namespace Vrui {

/***********************************************
Methods of class KinectRecorder::KinectStreamer:
***********************************************/

void KinectRecorder::KinectStreamer::depthStreamingCallback(const FrameBuffer& frameBuffer)
	{
	/* Save the frame: */
	frameSaver->saveDepthFrame(frameBuffer);
	}

void KinectRecorder::KinectStreamer::colorStreamingCallback(const FrameBuffer& frameBuffer)
	{
	/* Save the frame: */
	frameSaver->saveColorFrame(frameBuffer);
	}

KinectRecorder::KinectStreamer::KinectStreamer(libusb_device* sDevice,std::string calibrationFilesPath,std::string saveFileNamePrefix,Misc::ConfigurationFileSection& cameraSection)
	:camera(sDevice),frameSaver(0)
	{
	/* Open the Kinect camera: */
	camera.open();
	
	/* Get the camera's serial number to load the proper calibration matrices: */
	std::string serialNumber=camera.getSerialNumber();
	
	/* Check if there is an existing background frame for the camera: */
	bool removeBackground=false;
	std::string backgroundFileNamePrefix=cameraSection.retrieveString("./backgroundFileNamePrefix","");
	if(!backgroundFileNamePrefix.empty())
		{
		/* Load a background frame file: */
		std::string backgroundFileName=calibrationFilesPath;
		backgroundFileName.push_back('/');
		backgroundFileName.append(backgroundFileNamePrefix);
		camera.loadBackground(backgroundFileName.c_str());
		removeBackground=true;
		}
	
	/* Check for background capture: */
	unsigned int numBackgroundFrames=cameraSection.retrieveValue<unsigned int>("./numBackgroundFrames",0);
	if(numBackgroundFrames>0)
		{
		/* Capture background: */
		camera.captureBackground(numBackgroundFrames,false);
		removeBackground=true;
		}
	
	/* Check if there is a maximum depth value: */
	unsigned int maxDepth=cameraSection.retrieveValue<unsigned int>("./maxDepth",0);
	if(maxDepth>0)
		{
		/* Set the maximum depth value: */
		camera.setMaxDepth(maxDepth);
		removeBackground=true;
		}
	
	/* Enable background removal if requested: */
	camera.setRemoveBackground(removeBackground);
	
	/* Set the background removal fuzz value: */
	camera.setBackgroundRemovalFuzz(cameraSection.retrieveValue<int>("./backgroundRemovalFuzz",camera.getBackgroundRemovalFuzz()));
	
	/* Check for high resolution color images: */
	bool highres=cameraSection.retrieveValue<bool>("./highResolution",false);
	
	/* Create the name of the calibration matrices file: */
	std::string calibrationFileName=calibrationFilesPath;
	calibrationFileName.push_back('/');
	calibrationFileName.append("CameraCalibrationMatrices-");
	calibrationFileName.append(serialNumber);
	if(highres)
		calibrationFileName.append("-high");
	calibrationFileName.append(".dat");
	
	/* Override the calibration file name from the configuration file: */
	calibrationFileName=cameraSection.retrieveString("./calibrationFileName",calibrationFileName);
	
	/* Create the name of the camera's model space transformation file: */
	std::string transformFileName=calibrationFilesPath;
	transformFileName.push_back('/');
	transformFileName.append("ProjectorTransform-");
	transformFileName.append(serialNumber);
	transformFileName.append(".txt");
	
	/* Override the transformation file name from the configuration file: */
	transformFileName=cameraSection.retrieveString("./transformationFileName",transformFileName);
	
	/* Load the camera's physical space transformation: */
	Misc::File transformFile(transformFileName.c_str(),"rt");
	char transform[1024];
	transformFile.gets(transform,sizeof(transform));
	KinectFrameSaver::Transform projectorTransform=Misc::ValueCoder<KinectFrameSaver::Transform>::decode(transform,transform+strlen(transform),0);
	
	/* Set the camera's frame size: */
	camera.setFrameSize(KinectCamera::COLOR,highres?KinectCamera::FS_1280_1024:KinectCamera::FS_640_480);
	
	/* Create the frame saver: */
	std::string depthFrameFileName=saveFileNamePrefix;
	depthFrameFileName.push_back('-');
	depthFrameFileName.append(serialNumber);
	depthFrameFileName.append(".depth");
	std::string colorFrameFileName=saveFileNamePrefix;
	colorFrameFileName.push_back('-');
	colorFrameFileName.append(serialNumber);
	colorFrameFileName.append(".color");
	frameSaver=new KinectFrameSaver(camera,calibrationFileName.c_str(),projectorTransform,depthFrameFileName.c_str(),colorFrameFileName.c_str());
	}

KinectRecorder::KinectStreamer::~KinectStreamer(void)
	{
	/* Stop streaming: */
	camera.stopStreaming();
	
	/* Delete the frame saver: */
	delete frameSaver;
	}

void KinectRecorder::KinectStreamer::startStreaming(void)
	{
	/* Start streaming: */
	camera.startStreaming(Misc::createFunctionCall(this,&KinectRecorder::KinectStreamer::colorStreamingCallback),Misc::createFunctionCall(this,&KinectRecorder::KinectStreamer::depthStreamingCallback));
	}

/*******************************
Methods of class KinectRecorder:
*******************************/

KinectRecorder::KinectRecorder(Misc::ConfigurationFileSection& configFileSection)
	{
	/* Enable background USB event handling: */
	usbContext.startEventHandling();
	
	/* Get the path to the camera calibration files: */
	std::string calibrationFilesPath=configFileSection.retrieveString("./calibrationFilesPath","./");
	
	/* Read the saved files' name prefix: */
	std::string saveFileNamePrefix=configFileSection.retrieveString("./saveFileNamePrefix");
	
	/* Read the list of Kinect cameras: */
	typedef std::vector<std::string> StringList;
	StringList cameraSections=configFileSection.retrieveValue<StringList>("./cameras");
	
	/* Enumerate all USB devices: */
	USBDeviceList usbDevices(usbContext);
	size_t numKinectCameras=usbDevices.getNumDevices(0x045eU,0x02aeU);
	
	/* Create streamers for all cameras: */
	for(StringList::const_iterator csIt=cameraSections.begin();csIt!=cameraSections.end();++csIt)
		{
		/* Go to the camera's section: */
		Misc::ConfigurationFileSection cameraSection=configFileSection.getSection(csIt->c_str());
		
		/* Find the Kinect camera with the given serial number: */
		std::string serialNumber=cameraSection.retrieveValue<std::string>("./serialNumber");
		unsigned int cameraIndex;
		for(cameraIndex=0;cameraIndex<numKinectCameras;++cameraIndex)
			{
			/* Tentatively open the Kinect camera device: */
			USBDevice cam(usbDevices.getDevice(0x045eU,0x02aeU,cameraIndex));
			if(cam.getSerialNumber()==serialNumber) // Bail out if the desired device was found
				break;
			}
		if(cameraIndex<numKinectCameras)
			{
			/* Create a streamer for the found camera: */
			streamers.push_back(new KinectStreamer(usbDevices.getDevice(0x045eU,0x02aeU,cameraIndex),calibrationFilesPath,saveFileNamePrefix,cameraSection));
			}
		}
	}

KinectRecorder::~KinectRecorder(void)
	{
	/* Delete all streamers: */
	for(std::vector<KinectStreamer*>::iterator sIt=streamers.begin();sIt!=streamers.end();++sIt)
		delete *sIt;
	}

void KinectRecorder::start(double currentTimeStamp)
	{
	/* Synchronize all cameras' time bases: */
	for(std::vector<KinectStreamer*>::iterator sIt=streamers.begin();sIt!=streamers.end();++sIt)
		(*sIt)->getCamera().resetFrameTimer(currentTimeStamp);
	
	/* Start recording: */
	for(std::vector<KinectStreamer*>::iterator sIt=streamers.begin();sIt!=streamers.end();++sIt)
		(*sIt)->startStreaming();
	}

}
