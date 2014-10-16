/***********************************************************************
VideoDevice - Base class for video capture devices.
Copyright (c) 2009-2014 Oliver Kreylos

This file is part of the Basic Video Library (Video).

The Basic Video Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

The Basic Video Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Basic Video Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <Video/VideoDevice.h>

#include <Video/Config.h>

#include <Misc/ThrowStdErr.h>
#include <Misc/FunctionCalls.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#if 0
#include <Video/ImageSequenceVideoDevice.h>
#endif
#if VIDEO_CONFIG_HAVE_V4L2
#include <Video/Linux/V4L2VideoDevice.h>
#include <Video/Linux/OculusRiftDK2VideoDevice.h>
#endif
#if VIDEO_CONFIG_HAVE_DC1394
#include <Video/Linux/DC1394VideoDevice.h>
#endif

namespace Video {

/************************************
Static elements of class VideoDevice:
************************************/

VideoDevice::DeviceClass* VideoDevice::deviceClasses=0;

/****************************
Methods of class VideoDevice:
****************************/

VideoDevice::VideoDevice(void)
	:streamingCallback(0)
	{
	}

VideoDevice::~VideoDevice(void)
	{
	delete streamingCallback;
	}

void VideoDevice::registerDeviceClass(VideoDevice::EnumerateVideoDevicesFunc enumerateVideoDevices)
	{
	/* Create a new device class entry: */
	DeviceClass* newClass=new DeviceClass;
	newClass->enumerateVideoDevices=enumerateVideoDevices;
	newClass->succ=deviceClasses;
	
	/* Register the new class: */
	deviceClasses=newClass;
	}

void VideoDevice::unregisterDeviceClass(VideoDevice::EnumerateVideoDevicesFunc enumerateVideoDevices)
	{
	/* Find the enumeration function in the list of device classes: */
	DeviceClass* ptr0=0;
	for(DeviceClass* ptr1=deviceClasses;ptr1!=0;ptr0=ptr1,ptr1=ptr1->succ)
		if(ptr1->enumerateVideoDevices==enumerateVideoDevices)
			{
			/* Delete the device class entry: */
			if(ptr0!=0)
				ptr0->succ=ptr1->succ;
			else
				deviceClasses=ptr1->succ;
			delete ptr1;
			break;
			}
	}

std::vector<VideoDevice::DeviceIdPtr> VideoDevice::getVideoDevices(void)
	{
	std::vector<VideoDevice::DeviceIdPtr> result;
	
	/* Enumerate all video devices handled by all additional device classes: */
	for(DeviceClass* dcPtr=deviceClasses;dcPtr!=0;dcPtr=dcPtr->succ)
		dcPtr->enumerateVideoDevices(result);
	
	#if 0
	
	/* Enumerate all image sequence video devices in the system (THIS IS A HUGE PROBLEM): */
	ImageSequenceVideoDevice::enumerateDevices(result);
	
	#endif
	
	#if VIDEO_CONFIG_HAVE_V4L2
	
	/* Enumerate all quirky V4L2 video devices in the system: */
	OculusRiftDK2VideoDevice::enumerateDevices(result);
	
	/* Enumerate all V4L2 video devices in the system: */
	V4L2VideoDevice::enumerateDevices(result);
	
	#endif
	
	#if VIDEO_CONFIG_HAVE_DC1394
	
	/* Enumerate all DC1394 video devices in the system: */
	DC1394VideoDevice::enumerateDevices(result);
	
	#endif
	
	return result;
	}

VideoDevice* VideoDevice::createVideoDevice(VideoDevice::DeviceIdPtr deviceId)
	{
	/* Let the device ID object handle device creation: */
	return deviceId->createDevice();
	}

void VideoDevice::configure(const Misc::ConfigurationFileSection& cfg)
	{
	/* Get the device's current video format to use as default: */
	VideoDataFormat currentFormat=getVideoFormat();
	
	/* Get the list of the device's supported video formats: */
	std::vector<VideoDataFormat> deviceFormats=getVideoFormatList();
	
	/* Read the requested frame size: */
	currentFormat.size[0]=cfg.retrieveValue<unsigned int>("./width",currentFormat.size[0]);
	currentFormat.size[1]=cfg.retrieveValue<unsigned int>("./height",currentFormat.size[1]);
	
	/* Find the best-matching frame size among the supported video formats: */
	std::vector<VideoDataFormat>::iterator bestSizeMatch=deviceFormats.end();
	double bestSizeMatchRatio=1.0e10;
	for(std::vector<VideoDataFormat>::iterator dfIt=deviceFormats.begin();dfIt!=deviceFormats.end();++dfIt)
		{
		/* Calculate the format's size mismatch: */
		double sizeMatchRatio=0.0;
		for(int i=0;i<2;++i)
			{
			if(dfIt->size[i]<currentFormat.size[i])
				sizeMatchRatio+=double(currentFormat.size[i])/double(dfIt->size[i]);
			else
				sizeMatchRatio+=double(dfIt->size[i])/double(currentFormat.size[i]);
			}
		if(bestSizeMatchRatio>sizeMatchRatio)
			{
			bestSizeMatch=dfIt;
			bestSizeMatchRatio=sizeMatchRatio;
			}
		}
	currentFormat.size[0]=bestSizeMatch->size[0];
	currentFormat.size[1]=bestSizeMatch->size[1];
	
	/* Read the requested frame rate: */
	double frameRate=cfg.retrieveValue<double>("./frameRate",double(currentFormat.frameIntervalDenominator)/double(currentFormat.frameIntervalCounter));
	
	/* Find the best-matching frame rate among the supporting video formats: */
	std::vector<VideoDataFormat>::iterator bestRateMatch=deviceFormats.end();
	double bestRateMatchRatio=1.0e10;
	for(std::vector<VideoDataFormat>::iterator dfIt=deviceFormats.begin();dfIt!=deviceFormats.end();++dfIt)
		if(dfIt->size[0]==currentFormat.size[0]&&dfIt->size[1]==currentFormat.size[1])
			{
			/* Calculate the format's frame rate mismatch: */
			double rate=double(dfIt->frameIntervalDenominator)/double(dfIt->frameIntervalCounter);
			double rateMatchRatio;
			if(rate<frameRate)
				rateMatchRatio=frameRate/rate;
			else
				rateMatchRatio=rate/frameRate;
			if(bestRateMatchRatio>rateMatchRatio)
				{
				bestRateMatch=dfIt;
				bestRateMatchRatio=rateMatchRatio;
				}
			}
	currentFormat.frameIntervalCounter=bestRateMatch->frameIntervalCounter;
	currentFormat.frameIntervalDenominator=bestRateMatch->frameIntervalDenominator;
	currentFormat.pixelFormat=bestRateMatch->pixelFormat;
	
	/* If the configuration file contains a pixel format tag, see if there is a match among the supporting video formats: */
	if(cfg.hasTag("./pixelFormat"))
		{
		std::string pixelFormat=cfg.retrieveValue<std::string>("./pixelFormat");
		for(std::vector<VideoDataFormat>::iterator dfIt=deviceFormats.begin();dfIt!=deviceFormats.end();++dfIt)
			if(dfIt->size[0]==currentFormat.size[0]&&dfIt->size[1]==currentFormat.size[1]&&dfIt->frameIntervalCounter*currentFormat.frameIntervalDenominator==dfIt->frameIntervalDenominator*currentFormat.frameIntervalCounter)
				{
				/* Check if the pixel format matches the request: */
				if(dfIt->isPixelFormat(pixelFormat.c_str()))
					{
					currentFormat.pixelFormat=dfIt->pixelFormat;
					break;
					}
				}
		}
	
	/* Set the selected video format: */
	setVideoFormat(currentFormat);
	}

void VideoDevice::startStreaming(void)
	{
	/* Delete the streaming callback: */
	delete streamingCallback;
	streamingCallback=0;
	}

void VideoDevice::startStreaming(VideoDevice::StreamingCallback* newStreamingCallback)
	{
	/* Store the new streaming callback: */
	delete streamingCallback;
	streamingCallback=newStreamingCallback;
	}

void VideoDevice::stopStreaming(void)
	{
	/* Delete the streaming callback: */
	delete streamingCallback;
	streamingCallback=0;
	}

}
