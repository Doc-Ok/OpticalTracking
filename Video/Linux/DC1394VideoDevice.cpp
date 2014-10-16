/***********************************************************************
DC1394VideoDevice - Wrapper class around video devices as represented by
the dc1394 IEEE 1394 (Firewire) DCAM video library.
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

#include <Video/Linux/DC1394VideoDevice.h>

#include <string.h>
#include <iostream>
#include <Misc/StandardHashFunction.h>
#include <Misc/HashTable.h>
#include <Misc/FunctionCalls.h>
#include <Misc/StringPrintf.h>
#include <Misc/ThrowStdErr.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/Label.h>
#include <GLMotif/TextFieldSlider.h>
#include <GLMotif/Margin.h>
#include <GLMotif/DropdownBox.h>
#include <Video/ImageExtractorBA81.h>

namespace Video {

namespace {

/****************
Helper functions:
****************/

bool calcFrameInterval(dc1394framerate_t framerate,unsigned int& intervalCounter,unsigned int& intervalDenominator)
	{
	switch(framerate)
		{
		case DC1394_FRAMERATE_1_875:
			intervalCounter=8;
			intervalDenominator=15;
			break;
		
		case DC1394_FRAMERATE_3_75:
			intervalCounter=4;
			intervalDenominator=15;
			break;
		
		case DC1394_FRAMERATE_7_5:
			intervalCounter=2;
			intervalDenominator=15;
			break;
		
		case DC1394_FRAMERATE_15:
			intervalCounter=1;
			intervalDenominator=15;
			break;
		
		case DC1394_FRAMERATE_30:
			intervalCounter=1;
			intervalDenominator=30;
			break;
		
		case DC1394_FRAMERATE_60:
			intervalCounter=1;
			intervalDenominator=60;
			break;
		
		case DC1394_FRAMERATE_120:
			intervalCounter=1;
			intervalDenominator=120;
			break;
		
		case DC1394_FRAMERATE_240:
			intervalCounter=1;
			intervalDenominator=240;
			break;
		
		default:
			intervalCounter=0;
			intervalDenominator=0;
			return false;
		}
	
	return true;
	}

}

/********************************************
Methods of class DC1394VideoDevice::DeviceId:
********************************************/

VideoDevice* DC1394VideoDevice::DeviceId::createDevice(void) const
	{
	return new DC1394VideoDevice(guid,unitIndex);
	}

/**********************************
Methods of class DC1394VideoDevice:
**********************************/

bool DC1394VideoDevice::parseVideoMode(dc1394video_mode_t videoMode,VideoDataFormat& format) const
	{
	/* Query the video mode's pixel format: */
	dc1394color_coding_t colorCoding;
	if(dc1394_get_color_coding_from_video_mode(camera,videoMode,&colorCoding)!=DC1394_SUCCESS)
		return false;
	switch(colorCoding)
		{
		case DC1394_COLOR_CODING_RAW8:
			format.setPixelFormat("BA81");
			break;
		
		case DC1394_COLOR_CODING_RAW16:
			format.setPixelFormat("BA82");
			break;
		
		case DC1394_COLOR_CODING_MONO8:
		case DC1394_COLOR_CODING_MONO16:
		case DC1394_COLOR_CODING_MONO16S:
			if(bayerPattern!=BAYER_INVALID)
				{
				if(colorCoding==DC1394_COLOR_CODING_MONO8)
					format.setPixelFormat("BA81");
				else
					format.setPixelFormat("BA82");
				}
			else
				format.setPixelFormat("GREY");
			break;
		
		case DC1394_COLOR_CODING_RGB8:
		case DC1394_COLOR_CODING_RGB16:
		case DC1394_COLOR_CODING_RGB16S:
			format.setPixelFormat("RGB3");
			break;
		
		case DC1394_COLOR_CODING_YUV411:
			format.setPixelFormat("Y41P");
			break;
		
		case DC1394_COLOR_CODING_YUV422:
			format.setPixelFormat("UYVY");
			break;
		
		case DC1394_COLOR_CODING_YUV444:
			format.setPixelFormat("Y444");
			break;
		
		default:
			return false;
		}
	
	/* Query the video format's resolution: */
	if(dc1394_get_image_size_from_video_mode(camera,videoMode,&format.size[0],&format.size[1])!=DC1394_SUCCESS)
		return false;
	
	/* Calculate the video format's line size and frame size: */
	switch(colorCoding)
		{
		case DC1394_COLOR_CODING_RAW8:
		case DC1394_COLOR_CODING_MONO8:
			format.lineSize=format.size[0];
			format.frameSize=format.size[1]*format.lineSize;
			break;
		
		case DC1394_COLOR_CODING_RAW16:
		case DC1394_COLOR_CODING_MONO16:
		case DC1394_COLOR_CODING_MONO16S:
		case DC1394_COLOR_CODING_YUV444:
			format.lineSize=format.size[0]*2;
			format.frameSize=format.size[1]*format.lineSize;
			break;
		
		case DC1394_COLOR_CODING_RGB8:
			format.lineSize=format.size[0]*3;
			format.frameSize=format.size[1]*format.lineSize;
			break;
		
		case DC1394_COLOR_CODING_RGB16:
		case DC1394_COLOR_CODING_RGB16S:
			format.lineSize=format.size[0]*6;
			format.frameSize=format.size[1]*format.lineSize;
			break;
		
		case DC1394_COLOR_CODING_YUV411:
			format.lineSize=((format.size[0]+7)/8)*12;
			format.frameSize=format.size[1]*format.lineSize;
			break;
		
		case DC1394_COLOR_CODING_YUV422:
			format.lineSize=((format.size[0]+1)/2)*4;
			format.frameSize=format.size[1]*format.lineSize;
			break;
		
		default:
			return false;
		}
	
	return true;
	}

void DC1394VideoDevice::triggerModeCallback(Misc::CallbackData* cbData)
	{
	/* Get the proper callback data type: */
	GLMotif::DropdownBox::ValueChangedCallbackData* myCbData=dynamic_cast<GLMotif::DropdownBox::ValueChangedCallbackData*>(cbData);
	if(myCbData==0)
		return;
	
	/* Set the trigger's mode: */
	unsigned int mode=myCbData->dropdownBox->getManager()->getWidgetAttribute<unsigned int>(myCbData->dropdownBox->getItemWidget(myCbData->newSelectedItem));
	if(mode==0)
		{
		/* Disable the trigger: */
		dc1394_external_trigger_set_power(camera,DC1394_OFF);
		}
	else
		{
		/* Enable the trigger: */
		dc1394_external_trigger_set_power(camera,DC1394_OFF);
		
		/* Set the trigger's mode: */
		dc1394_external_trigger_set_mode(camera,dc1394trigger_mode_t(mode));
		}
	}

void DC1394VideoDevice::triggerSourceCallback(Misc::CallbackData* cbData)
	{
	/* Get the proper callback data type: */
	GLMotif::DropdownBox::ValueChangedCallbackData* myCbData=dynamic_cast<GLMotif::DropdownBox::ValueChangedCallbackData*>(cbData);
	if(myCbData==0)
		return;
	
	/* Set the trigger's source: */
	unsigned int source=myCbData->dropdownBox->getManager()->getWidgetAttribute<unsigned int>(myCbData->dropdownBox->getItemWidget(myCbData->newSelectedItem));
	dc1394_external_trigger_set_source(camera,dc1394trigger_source_t(source));
	}

void DC1394VideoDevice::triggerPolarityCallback(Misc::CallbackData* cbData)
	{
	/* Get the proper callback data type: */
	GLMotif::DropdownBox::ValueChangedCallbackData* myCbData=dynamic_cast<GLMotif::DropdownBox::ValueChangedCallbackData*>(cbData);
	if(myCbData==0)
		return;
	
	/* Set the trigger's polarity: */
	dc1394_external_trigger_set_polarity(camera,dc1394trigger_polarity_t(DC1394_TRIGGER_ACTIVE_MIN+myCbData->newSelectedItem));
	}

void DC1394VideoDevice::featureModeCallback(Misc::CallbackData* cbData,const unsigned int& featureId)
	{
	/* Get the proper callback data type: */
	GLMotif::DropdownBox::ValueChangedCallbackData* myCbData=dynamic_cast<GLMotif::DropdownBox::ValueChangedCallbackData*>(cbData);
	if(myCbData==0)
		return;
	
	/* Set the feature's mode: */
	unsigned int mode=myCbData->dropdownBox->getManager()->getWidgetAttribute<unsigned int>(myCbData->dropdownBox->getItemWidget(myCbData->newSelectedItem));
	if(mode==0)
		{
		/* Disable the feature: */
		dc1394_feature_set_power(camera,dc1394feature_t(featureId),DC1394_OFF);
		}
	else
		{
		/* Enable the feature: */
		dc1394_feature_set_power(camera,dc1394feature_t(featureId),DC1394_ON);
		
		/* Set the feature's mode: */
		dc1394_feature_set_mode(camera,dc1394feature_t(featureId),dc1394feature_mode_t(mode));
		}
	}

void DC1394VideoDevice::whiteBalanceBUValueCallback(Misc::CallbackData* cbData)
	{
	/* Get the proper callback data type: */
	GLMotif::TextFieldSlider::ValueChangedCallbackData* myCbData=dynamic_cast<GLMotif::TextFieldSlider::ValueChangedCallbackData*>(cbData);
	if(myCbData==0)
		return;
	
	/* Get the current white balance values: */
	uint32_t bu,rv;
	dc1394_feature_whitebalance_get_value(camera,&bu,&rv);
	
	/* Update the bu value from the slider: */
	bu=uint32_t(myCbData->value+0.5);
	
	/* Set the adjusted white balance values: */
	dc1394_feature_whitebalance_set_value(camera,bu,rv);
	}

void DC1394VideoDevice::whiteBalanceRVValueCallback(Misc::CallbackData* cbData)
	{
	/* Get the proper callback data type: */
	GLMotif::TextFieldSlider::ValueChangedCallbackData* myCbData=dynamic_cast<GLMotif::TextFieldSlider::ValueChangedCallbackData*>(cbData);
	if(myCbData==0)
		return;
	
	/* Get the current white balance values: */
	uint32_t bu,rv;
	dc1394_feature_whitebalance_get_value(camera,&bu,&rv);
	
	/* Update the rv value from the slider: */
	rv=uint32_t(myCbData->value+0.5);
	
	/* Set the adjusted white balance values: */
	dc1394_feature_whitebalance_set_value(camera,bu,rv);
	}

void DC1394VideoDevice::featureValueCallback(Misc::CallbackData* cbData,const unsigned int& featureId)
	{
	/* Get the proper callback data type: */
	GLMotif::TextFieldSlider::ValueChangedCallbackData* myCbData=dynamic_cast<GLMotif::TextFieldSlider::ValueChangedCallbackData*>(cbData);
	if(myCbData==0)
		return;
	
	/* Set the feature's value: */
	dc1394_feature_set_value(camera,dc1394feature_t(featureId),(unsigned int)(myCbData->value+0.5));
	}

void* DC1394VideoDevice::streamingThreadMethod(void)
	{
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	// Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	while(true)
		{
		/* Dequeue the next available frame buffer: */
		dc1394video_frame_t* frame=0;
		if(dc1394_capture_dequeue(camera,DC1394_CAPTURE_POLICY_WAIT,&frame)==DC1394_SUCCESS&&frame!=0)
			{
			/* Fill in the frame buffer structure: */
			DC1394FrameBuffer frameBuffer;
			frameBuffer.start=frame->image;
			frameBuffer.size=frame->total_bytes;
			frameBuffer.used=frame->image_bytes;
			frameBuffer.frame=frame;
			
			/* Call the streaming callback: */
			(*streamingCallback)(&frameBuffer);
			
			/* Put the frame buffer back into the capture queue: */
			dc1394_capture_enqueue(camera,frameBuffer.frame);
			}
		}
	
	return 0;
	}

DC1394VideoDevice::DC1394VideoDevice(uint64_t guid,unsigned int unitIndex)
	:context(0),camera(0),bayerPattern(BAYER_INVALID)
	{
	dc1394camera_list_t* cameraList=0;
	try
		{
		/* Open the DC1394 device context: */
		if((context=dc1394_new())==0)
			Misc::throwStdErr("Video::DC1394VideoDevice::DC1394VideoDevice: Unable to open DC1394 device context");
		
		/* Get the list of all cameras in the device context: */
		if(dc1394_camera_enumerate(context,&cameraList)!=DC1394_SUCCESS||cameraList==0)
			Misc::throwStdErr("Video::DC1394VideoDevice::DC1394VideoDevice: Unable to query list of cameras in DC1394 device context");
		if(cameraList->num==0)
			Misc::throwStdErr("Video::DC1394VideoDevice::DC1394VideoDevice: DC1394 device context contains no cameras");
		
		/* Get the index of the requested camera in the device context: */
		int cameraIndex=-1;
		if(guid!=0)
			{
			/* Find the camera with the given GUID: */
			for(unsigned int i=0;i<cameraList->num&&cameraIndex==-1;++i)
				if(guid==cameraList->ids[i].guid&&unitIndex==cameraList->ids[i].unit)
					cameraIndex=i;
			
			if(cameraIndex==-1)
				Misc::throwStdErr("Video::DC1394VideoDevice::DC1394VideoDevice: Device context contains no camera with GUID %lx and unit index %u",guid,unitIndex);
			}
		else
			cameraIndex=0;
		
		/* Open the camera: */
		if((camera=dc1394_camera_new_unit(context,cameraList->ids[cameraIndex].guid,unitIndex))==0)
			Misc::throwStdErr("Video::DC1394VideoDevice::DC1394VideoDevice: Unable to open camera with GUID %lx and unit index %u",cameraList->ids[cameraIndex].guid,unitIndex);
		
		/* Delete the camera list: */
		dc1394_camera_free_list(cameraList);
		cameraList=0;
		}
	catch(...)
		{
		/* Clean up and re-throw the exception: */
		if(camera!=0)
			dc1394_camera_free(camera);
		if(cameraList!=0)
			dc1394_camera_free_list(cameraList);
		if(context!=0)
			dc1394_free(context);
		throw;
		}
	
	/* Check if the camera is a Point Grey Research camera: */
	if(strcmp(camera->vendor,"Point Grey Research")==0)
		{
		/* Query the Bayer pattern register: */
		uint32_t pattern;
		if(dc1394_get_control_register(camera,0x1040,&pattern)==DC1394_SUCCESS)
			{
			switch(pattern)
				{
				case 0x52474742: // 'RGGB'
					bayerPattern=BAYER_RGGB;
					break;
				
				case 0x47425247: // 'GBRG'
					bayerPattern=BAYER_GBRG;
					break;
				
				case 0x47524247: // 'GRBG'
					bayerPattern=BAYER_GRBG;
					break;
				
				case 0x42474752: // 'BGGR'
					bayerPattern=BAYER_BGGR;
					break;
				
				default:
					bayerPattern=BAYER_INVALID;
				}
			}
		}
	}

DC1394VideoDevice::~DC1394VideoDevice(void)
	{
	/* Stop capture on the camera: */
	dc1394_video_set_transmission(camera,DC1394_OFF);
	
	if(streamingCallback!=0)
		{
		/* Stop the background streaming thread: */
		streamingThread.cancel();
		streamingThread.join();
		}
	
	/* Close the camera and device context: */
	dc1394_capture_stop(camera);
	dc1394_camera_free(camera);
	dc1394_free(context);
	}

std::vector<VideoDataFormat> DC1394VideoDevice::getVideoFormatList(void) const
	{
	std::vector<VideoDataFormat> result;
	
	/* Query the device's available video modes: */
	dc1394video_modes_t videoModes;
	if(dc1394_video_get_supported_modes(camera,&videoModes)!=DC1394_SUCCESS)
		Misc::throwStdErr("Video::DC1394VideoDevice::getVideoFormatList: Unable to query camera's video format list");
	
	/* Process all video modes: */
	VideoDataFormat format;
	for(unsigned int modeIndex=0;modeIndex<videoModes.num;++modeIndex)
		{
		/* Parse the video mode: */
		if(parseVideoMode(videoModes.modes[modeIndex],format))
			{
			/* Query the device's available frame rates for this video mode: */
			dc1394framerates_t frameRates;
			if(dc1394_video_get_supported_framerates(camera,videoModes.modes[modeIndex],&frameRates)!=DC1394_SUCCESS)
				Misc::throwStdErr("Video::DC1394VideoDevice::getVideoFormatList: Unable to query list of supported frame rates");
			
			/* Process all frame rates: */
			for(unsigned int frameRateIndex=0;frameRateIndex<frameRates.num;++frameRateIndex)
				{
				if(calcFrameInterval(frameRates.framerates[frameRateIndex],format.frameIntervalCounter,format.frameIntervalDenominator))
					{
					/* Store the video format: */
					result.push_back(format);
					}
				}
			}
		}
	
	return result;
	}

VideoDataFormat DC1394VideoDevice::getVideoFormat(void) const
	{
	/* Query the current video mode: */
	dc1394video_mode_t videoMode;
	if(dc1394_video_get_mode(camera,&videoMode)!=DC1394_SUCCESS)
		Misc::throwStdErr("Video::DC1394VideoDevice::getVideoFormat: Unable to query camera's current video mode");
	
	/* Parse the video mode: */
	VideoDataFormat result;
	if(!parseVideoMode(videoMode,result))
		Misc::throwStdErr("Video::DC1394VideoDevice::getVideoFormat: Unable to parse camera's current video mode");
	
	/* Query the camera's current frame rate: */
	dc1394framerate_t framerate;
	if(dc1394_video_get_framerate(camera,&framerate)!=DC1394_SUCCESS)
		Misc::throwStdErr("Video::DC1394VideoDevice::getVideoFormat: Unable to query camera's current frame rate");
	if(!calcFrameInterval(framerate,result.frameIntervalCounter,result.frameIntervalDenominator))
		Misc::throwStdErr("Video::DC1394VideoDevice::getVideoFormat: Unable to parse camera's current frame rate");
	
	return result;
	}

VideoDataFormat& DC1394VideoDevice::setVideoFormat(VideoDataFormat& newFormat)
	{
	/* Extract a DC1394 color coding from the video format's pixel format: */
	dc1394color_coding_t colorCoding=DC1394_COLOR_CODING_MONO8;
	switch(newFormat.pixelFormat)
		{
		case 0x59455247: // 'GREY'
		case 0x20363159: // 'Y16 '
			if(bayerPattern!=BAYER_INVALID)
				Misc::throwStdErr("Video::DC1394VideoDevice::setVideoFormat: Camera does not support greyscale pixel formats");
			colorCoding=DC1394_COLOR_CODING_MONO8;
			newFormat.setPixelFormat("GREY");
			break;
		
		case 0x31384142: // 'BA81'
		case 0x32384142: // 'BA82'
			if(bayerPattern==BAYER_INVALID)
				Misc::throwStdErr("Video::DC1394VideoDevice::setVideoFormat: Camera does not support Bayer pixel formats");
			if(newFormat.pixelFormat==0x31384142)
				colorCoding=DC1394_COLOR_CODING_MONO8;
			else
				colorCoding=DC1394_COLOR_CODING_MONO16;
			break;
		
		case 0x31424752: // 'RGB1'
		case 0x34343452: // 'R444'
		case 0x4f424752: // 'RGBO'
		case 0x50424752: // 'RGBP'
		case 0x51424752: // 'RGBQ'
		case 0x52424752: // 'RGBR'
		case 0x33524742: // 'BGR3'
		case 0x33424752: // 'RGB3'
		case 0x34524742: // 'BGR4'
		case 0x34424752: // 'RGB4'
			colorCoding=DC1394_COLOR_CODING_RGB8;
			newFormat.setPixelFormat("RGB3");
			break;
		
		case 0x50313459: // 'Y41P'
		case 0x39565559: // 'YUV9'
		case 0x39555659: // 'YVU9'
		case 0x50313134: // '411P'
			colorCoding=DC1394_COLOR_CODING_YUV411;
			newFormat.setPixelFormat("Y41P");
			break;
		
		case 0x56595559: // 'YUYV'
		case 0x59565955: // 'UYVY'
		case 0x32315659: // 'YV12'
		case 0x32315559: // 'YU12'
		case 0x50323234: // '422P'
		case 0x3231564e: // 'NV12'
		case 0x3132564e: // 'NV21'
			colorCoding=DC1394_COLOR_CODING_YUV422;
			newFormat.setPixelFormat("UYUV");
			break;
		
		case 0x34343459: // 'Y444'
		case 0x4f565559: // 'YUVO'
		case 0x50565559: // 'YUVP'
		case 0x34565559: // 'YUV4'
			colorCoding=DC1394_COLOR_CODING_YUV444;
			newFormat.setPixelFormat("Y444");
			break;
		
		default:
			Misc::throwStdErr("Video::DC1394VideoDevice::setVideoFormat: Unknown pixel format");
		}
	
	/* Get the list of video modes supported by the camera device: */
	dc1394video_modes_t videoModes;
	if(dc1394_video_get_supported_modes(camera,&videoModes)!=DC1394_SUCCESS)
		Misc::throwStdErr("Video::DC1394VideoDevice::setVideoFormat: Unable to query camera's supported video formats");
	
	/* Find the best-matching video mode in the list: */
	int bestModeIndex=-1;
	double bestSizeMatch=1.0e10;
	for(unsigned int modeIndex=0;modeIndex<videoModes.num;++modeIndex)
		{
		/* Check if the video mode matches the requested color coding: */
		dc1394color_coding_t modeColorCoding;
		if(dc1394_get_color_coding_from_video_mode(camera,videoModes.modes[modeIndex],&modeColorCoding)==DC1394_SUCCESS&&colorCoding==modeColorCoding)
			{
			/* Get the video mode's image size: */
			unsigned int modeSize[2];
			if(dc1394_get_image_size_from_video_mode(camera,videoModes.modes[modeIndex],&modeSize[0],&modeSize[1])==DC1394_SUCCESS)
				{
				/* Calculate the image size discrepancy: */
				double sizeMatch=1.0;
				for(int i=0;i<2;++i)
					{
					if(newFormat.size[i]>=modeSize[i])
						sizeMatch*=double(newFormat.size[i])/double(modeSize[i]);
					else
						sizeMatch*=double(modeSize[i])/double(newFormat.size[i]);
					}
				if(bestSizeMatch>sizeMatch)
					{
					bestModeIndex=modeIndex;
					bestSizeMatch=sizeMatch;
					}
				}
			}
		}
	if(bestModeIndex==-1)
		Misc::throwStdErr("Video::DC1394VideoDevice::setVideoFormat: Camera does not support the requested video format");
	dc1394_get_image_size_from_video_mode(camera,videoModes.modes[bestModeIndex],&newFormat.size[0],&newFormat.size[1]);
	
	/* Get the list of frame rates supported for the selected video mode: */
	dc1394framerates_t framerates;
	if(dc1394_video_get_supported_framerates(camera,videoModes.modes[bestModeIndex],&framerates)!=DC1394_SUCCESS)
		Misc::throwStdErr("Video::DC1394VideoDevice::setVideoFormat: Unable to query camera's supported frame rates");
	
	/* Find the best-matching frame rate in the list: */
	double requestedInterval=double(newFormat.frameIntervalCounter)/double(newFormat.frameIntervalDenominator);
	int bestFramerateIndex=-1;
	double bestFramerateMatch=1.0e10;
	for(unsigned int framerateIndex=0;framerateIndex<framerates.num;++framerateIndex)
		{
		unsigned int counter,denominator;
		calcFrameInterval(framerates.framerates[framerateIndex],counter,denominator);
		double interval=double(counter)/double(denominator);
		double framerateMatch=1.0;
		if(requestedInterval>interval)
			framerateMatch*=requestedInterval/interval;
		else
			framerateMatch*=interval/requestedInterval;
		if(bestFramerateMatch>framerateMatch)
			{
			bestFramerateIndex=framerateIndex;
			bestFramerateMatch=framerateMatch;
			}
		}
	if(bestFramerateIndex==-1)
		Misc::throwStdErr("Video::DC1394VideoDevice::setVideoFormat: Camera does not support the requested video frame rate");
	calcFrameInterval(framerates.framerates[bestFramerateIndex],newFormat.frameIntervalCounter,newFormat.frameIntervalDenominator);
	
	/* Set the camera's video mode: */
	if(dc1394_video_set_mode(camera,videoModes.modes[bestModeIndex])!=DC1394_SUCCESS)
		Misc::throwStdErr("Video::DC1394VideoDevice::setVideoFormat: Unable to set camera's video mode");
	if(dc1394_video_set_framerate(camera,framerates.framerates[bestFramerateIndex])!=DC1394_SUCCESS)
		Misc::throwStdErr("Video::DC1394VideoDevice::setVideoFormat: Unable to set camera's frame rate");
	
	return newFormat;
	}

void DC1394VideoDevice::configure(const Misc::ConfigurationFileSection& cfg)
	{
	/* Call the base class method to select frame size, frame rate, and pixel format: */
	VideoDevice::configure(cfg);
	}

ImageExtractor* DC1394VideoDevice::createImageExtractor(void) const
	{
	/* Get the current video format: */
	VideoDataFormat format=getVideoFormat();
	
	/* Create an extractor based on the video format's pixel format: */
	if(format.isPixelFormat("BA81"))
		return new ImageExtractorBA81(format.size,getBayerPattern());
	else
		{
		char fcc[5];
		Misc::throwStdErr("Video::DC1394VideoDevice::createImageExtractor: Unknown pixel format %s",format.getFourCC(fcc));
		}
	
	/* Execution never gets here; just to make compiler happy: */
	return 0;
	}

namespace {

static const char* featureNames[]=
	{
	"Brightness","Exposure","Sharpness","White Balance","Hue","Saturation","Gamma",
	"Shutter","Gain","Iris","Focus","Temperature","Trigger","Trigger Delay","White Shading",
	"Frame Rate","Zoom","Pan","Tilt","Optical Filter","Capture Size","Capture Quality"
	};

static const char* featureModeNames[]=
	{
	"Manual","Auto","One Push Auto"
	};

static const char* triggerModeNames[]=
	{
	"Mode 0","Mode 1","Mode 2","Mode 3","Mode 4","Mode 5","Mode 14","Mode 15"
	};
	
static const char* triggerSourceNames[]=
	{
	"Source 0","Source 1","Source 2","Source 3","Software"
	};

static const char* triggerPolarityNames[]=
	{
	"Active Low","Active High"
	};

}

GLMotif::Widget* DC1394VideoDevice::createControlPanel(GLMotif::WidgetManager* widgetManager)
	{
	/* Get the style sheet: */
	const GLMotif::StyleSheet* ss=widgetManager->getStyleSheet();
	
	/* Create the control panel dialog window: */
	GLMotif::PopupWindow* controlPanelPopup=new GLMotif::PopupWindow("DC1394VideoDeviceControlPanelPopup",widgetManager,"Video Source Control");
	controlPanelPopup->setResizableFlags(true,false);
	
	GLMotif::RowColumn* controlPanel=new GLMotif::RowColumn("ControlPanel",controlPanelPopup,false);
	controlPanel->setNumMinorWidgets(2);
	
	/* Enumerate the camera's features: */
	dc1394featureset_t* features=new dc1394featureset_t;
	if(dc1394_feature_get_all(camera,features)!=DC1394_SUCCESS)
		Misc::throwStdErr("Video::DC1394VideoDevice::configure: Unable to query camera's features");
	
	/* Process all features: */
	for(int featureIndex=0;featureIndex<DC1394_FEATURE_NUM;++featureIndex)
		{
		dc1394feature_info_t* feature=&features->feature[featureIndex];
		if(feature->available)
			{
			/* Get the feature's bounds and options: */
			if(dc1394_feature_get(camera,feature)!=DC1394_SUCCESS)
				Misc::throwStdErr("Video::DC1394VideoDevice::configure: Unable to query camera's feature \"%s\"",featureNames[feature->id-DC1394_FEATURE_MIN]);
			
			/* Create a label for the feature: */
			new GLMotif::Label(Misc::stringPrintf("Label%d",featureIndex).c_str(),controlPanel,featureNames[feature->id-DC1394_FEATURE_MIN]);
			
			/* Create widgets to control the feature: */
			if(feature->id==DC1394_FEATURE_TRIGGER)
				{
				/* Create a rowcolumn inside a margin to hold the trigger's widgets: */
				GLMotif::Margin* triggerMargin=new GLMotif::Margin(Misc::stringPrintf("Margin%d",featureIndex).c_str(),controlPanel,false);
				triggerMargin->setAlignment(GLMotif::Alignment::LEFT);
				GLMotif::RowColumn* triggerBox=new GLMotif::RowColumn("Box",triggerMargin,false);
				triggerBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
				triggerBox->setPacking(GLMotif::RowColumn::PACK_TIGHT);
				triggerBox->setNumMinorWidgets(1);
				
				/* Create a drop-down box to select trigger modes: */
				std::vector<std::string> modes;
				if(feature->on_off_capable)
					modes.push_back("Off");
				int currentIndex=0;
				for(unsigned int i=0;i<feature->trigger_modes.num;++i)
					{
					if(feature->trigger_mode==feature->trigger_modes.modes[i])
						currentIndex=i;
					modes.push_back(triggerModeNames[feature->trigger_modes.modes[i]-DC1394_TRIGGER_MODE_MIN]);
					}
				if(feature->on_off_capable)
					{
					if(!feature->is_on)
						currentIndex=0;
					else
						++currentIndex;
					}
				GLMotif::DropdownBox* triggerModeBox=new GLMotif::DropdownBox("TriggerModeBox",triggerBox,modes);
				triggerModeBox->setSelectedItem(currentIndex);
				triggerModeBox->getValueChangedCallbacks().add(this,&DC1394VideoDevice::triggerModeCallback);
				if(feature->on_off_capable)
					widgetManager->setWidgetAttribute<unsigned int>(triggerModeBox->getItemWidget(0),0);
				for(unsigned int i=0;i<feature->trigger_modes.num;++i)
					widgetManager->setWidgetAttribute<unsigned int>(triggerModeBox->getItemWidget(feature->on_off_capable?i+1:i),feature->trigger_modes.modes[i]);
				
				if(feature->trigger_sources.num>0)
					{
					/* Create a drop-down box to select trigger sources: */
					std::vector<std::string> sources;
					currentIndex=0;
					for(unsigned int i=0;i<feature->trigger_sources.num;++i)
						{
						if(feature->trigger_source==feature->trigger_sources.sources[i])
							currentIndex=i;
						sources.push_back(triggerSourceNames[feature->trigger_sources.sources[i]-DC1394_TRIGGER_SOURCE_MIN]);
						}
					GLMotif::DropdownBox* triggerSourceBox=new GLMotif::DropdownBox("TriggerSourceBox",triggerBox,sources);
					triggerSourceBox->setSelectedItem(currentIndex);
					triggerSourceBox->getValueChangedCallbacks().add(this,&DC1394VideoDevice::triggerSourceCallback);
					for(unsigned int i=0;i<feature->trigger_sources.num;++i)
						widgetManager->setWidgetAttribute<unsigned int>(triggerSourceBox->getItemWidget(i),feature->trigger_sources.sources[i]);
					}
				
				if(feature->polarity_capable)
					{
					/* Create a drop-down box to select trigger polarities: */
					std::vector<std::string> polarities;
					currentIndex=0;
					for(unsigned int i=0;i<2;++i)
						{
						if(feature->trigger_polarity==DC1394_TRIGGER_ACTIVE_MIN+int(i))
							currentIndex=i;
						polarities.push_back(triggerPolarityNames[i]);
						}
					GLMotif::DropdownBox* triggerPolarityBox=new GLMotif::DropdownBox("TriggerPolarityBox",triggerBox,polarities);
					triggerPolarityBox->setSelectedItem(currentIndex);
					triggerPolarityBox->getValueChangedCallbacks().add(this,&DC1394VideoDevice::triggerPolarityCallback);
					}
				
				triggerBox->manageChild();
				triggerMargin->manageChild();
				}
			else
				{
				/* Create a rowcolumn to hold the feature's widgets: */
				GLMotif::RowColumn* featureBox=new GLMotif::RowColumn(Misc::stringPrintf("Box%d",featureIndex).c_str(),controlPanel,false);
				featureBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
				featureBox->setPacking(GLMotif::RowColumn::PACK_TIGHT);
				featureBox->setNumMinorWidgets(1);
				
				/* Create a drop-down box to select feature modes: */
				std::vector<std::string> modes;
				if(feature->on_off_capable)
					modes.push_back("Off");
				int currentIndex=0;
				for(unsigned int i=0;i<feature->modes.num;++i)
					{
					if(feature->current_mode==feature->modes.modes[i])
						currentIndex=i;
					modes.push_back(featureModeNames[feature->modes.modes[i]-DC1394_FEATURE_MODE_MIN]);
					}
				if(feature->on_off_capable)
					{
					if(!feature->is_on)
						currentIndex=0;
					else
						++currentIndex;
					}
				GLMotif::DropdownBox* featureModeBox=new GLMotif::DropdownBox("FeatureModeBox",featureBox,modes);
				featureModeBox->setSelectedItem(currentIndex);
				featureModeBox->getValueChangedCallbacks().add(this,&DC1394VideoDevice::featureModeCallback,(unsigned int)feature->id);
				if(feature->on_off_capable)
					widgetManager->setWidgetAttribute<unsigned int>(featureModeBox->getItemWidget(0),0);
				for(unsigned int i=0;i<feature->modes.num;++i)
					widgetManager->setWidgetAttribute<unsigned int>(featureModeBox->getItemWidget(feature->on_off_capable?i+1:i),feature->modes.modes[i]);
				
				if(feature->id==DC1394_FEATURE_WHITE_BALANCE)
					{
					/* Create a pair of sliders to set white balance values: */
					GLMotif::RowColumn* wbBox=new GLMotif::RowColumn("WbBox",featureBox,false);
					wbBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
					wbBox->setPacking(GLMotif::RowColumn::PACK_GRID);
					wbBox->setNumMinorWidgets(1);
					
					GLMotif::TextFieldSlider* buValueSlider=new GLMotif::TextFieldSlider("BuValueSlider",wbBox,3,ss->fontHeight*5.0f);
					buValueSlider->setSliderMapping(GLMotif::TextFieldSlider::LINEAR);
					buValueSlider->setValueRange(feature->min,feature->max,1);
					buValueSlider->setValue(feature->BU_value);
					buValueSlider->getValueChangedCallbacks().add(this,&DC1394VideoDevice::whiteBalanceBUValueCallback);
					
					GLMotif::TextFieldSlider* rvValueSlider=new GLMotif::TextFieldSlider("RvValueSlider",wbBox,3,ss->fontHeight*5.0f);
					rvValueSlider->setSliderMapping(GLMotif::TextFieldSlider::LINEAR);
					rvValueSlider->setValueRange(feature->min,feature->max,1);
					rvValueSlider->setValue(feature->RV_value);
					rvValueSlider->getValueChangedCallbacks().add(this,&DC1394VideoDevice::whiteBalanceRVValueCallback);
					
					wbBox->manageChild();
					}
				else
					{
					/* Create a slider to set feature values: */
					GLMotif::TextFieldSlider* featureValueSlider=new GLMotif::TextFieldSlider("FeatureValueSlider",featureBox,4,ss->fontHeight*10.0f);
					featureValueSlider->setSliderMapping(GLMotif::TextFieldSlider::LINEAR);
					featureValueSlider->setValueRange(feature->min,feature->max,1);
					featureValueSlider->setValue(feature->value);
					featureValueSlider->getValueChangedCallbacks().add(this,&DC1394VideoDevice::featureValueCallback,(unsigned int)feature->id);
					}
				
				featureBox->manageChild();
				}
			}
		}
	
	controlPanel->manageChild();
	
	return controlPanelPopup;
	}

unsigned int DC1394VideoDevice::allocateFrameBuffers(unsigned int requestedNumFrameBuffers)
	{
	if(dc1394_capture_setup(camera,requestedNumFrameBuffers,DC1394_CAPTURE_FLAGS_DEFAULT)!=DC1394_SUCCESS)
		Misc::throwStdErr("Video::DC1394VideoDevice::allocateFrameBuffers: Unable to allocate frame buffers");
	return requestedNumFrameBuffers;
	}

void DC1394VideoDevice::startStreaming(void)
	{
	/* Call the base class method: */
	VideoDevice::startStreaming();
	
	/* Start streaming: */
	if(dc1394_video_set_transmission(camera,DC1394_ON)!=DC1394_SUCCESS)
		Misc::throwStdErr("Video::DC1394VideoDevice::startStreaming: Unable to start image transfers");
	}

void DC1394VideoDevice::startStreaming(VideoDevice::StreamingCallback* newStreamingCallback)
	{
	/* Call the base class method: */
	VideoDevice::startStreaming(newStreamingCallback);
	
	/* Start streaming: */
	if(dc1394_video_set_transmission(camera,DC1394_ON)!=DC1394_SUCCESS)
		Misc::throwStdErr("Video::DC1394VideoDevice::startStreaming: Unable to start image transfers");
	
	/* Start the background capture thread: */
	streamingThread.start(this,&DC1394VideoDevice::streamingThreadMethod);
	}

FrameBuffer* DC1394VideoDevice::dequeueFrame(void)
	{
	/* Get the next frame buffer from the camera's DMA ring buffer: */
	dc1394video_frame_t* frame=0;
	if(dc1394_capture_dequeue(camera,DC1394_CAPTURE_POLICY_WAIT,&frame)!=DC1394_SUCCESS||frame==0)
		Misc::throwStdErr("Video::DC1394VideoDevice::getNextFrame: Unable to dequeue the next frame buffer");
	
	/* Fill in the result structure: */
	DC1394FrameBuffer* result=new DC1394FrameBuffer;
	result->start=frame->image;
	result->size=frame->total_bytes;
	result->used=frame->image_bytes;
	result->frame=frame;
	return result;
	}

void DC1394VideoDevice::enqueueFrame(FrameBuffer* frame)
	{
	DC1394FrameBuffer* myFrame=dynamic_cast<DC1394FrameBuffer*>(frame);
	if(myFrame==0)
		Misc::throwStdErr("Video::DC1394VideoDevice::enqueueFrame: Mismatching frame buffer type");
	
	/* Return the original DC1394 frame to the camera's DMA ring buffer: */
	if(dc1394_capture_enqueue(camera,myFrame->frame)!=DC1394_SUCCESS)
		Misc::throwStdErr("Video::DC1394VideoDevice::enqueueFrame: Unable to enqueue the frame buffer");
	
	/* Delete the frame: */
	delete frame;
	}

void DC1394VideoDevice::stopStreaming(void)
	{
	/* Stop streaming: */
	if(dc1394_video_set_transmission(camera,DC1394_OFF)!=DC1394_SUCCESS)
		Misc::throwStdErr("Video::DC1394VideoDevice::stopStreaming: Unable to stop image transfers");
	
	if(streamingCallback!=0)
		{
		/* Stop the background streaming thread: */
		streamingThread.cancel();
		streamingThread.join();
		}
	
	/* Call the base class method: */
	VideoDevice::stopStreaming();
	}

void DC1394VideoDevice::releaseFrameBuffers(void)
	{
	if(dc1394_capture_stop(camera)!=DC1394_SUCCESS)
		Misc::throwStdErr("Video::DC1394VideoDevice::releastFrameBuffers: Unable to release frame buffers");
	}

namespace {

/************
Helper class:
************/

struct CameraVendorModelId
	{
	/* Elements: */
	public:
	unsigned int vendorId;
	unsigned int modelId;
	
	/* Constructors and destructors: */
	public:
	CameraVendorModelId(unsigned int sVendorId,unsigned int sModelId)
		:vendorId(sVendorId),modelId(sModelId)
		{
		}
	
	/* Methods: */
	bool operator!=(const CameraVendorModelId& other) const
		{
		return vendorId!=other.vendorId||modelId!=other.modelId;
		}
	static size_t hash(const CameraVendorModelId& value,size_t tableSize)
		{
		return (size_t(value.vendorId)*1001+size_t(value.modelId))%tableSize;
		}
	};

}

void DC1394VideoDevice::enumerateDevices(std::vector<VideoDevice::DeviceIdPtr>& devices)
	{
	/* Create a temporary DC1394 context: */
	dc1394_t* context=dc1394_new();
	if(context==0)
		return;
	
	/* Get the list of all cameras in the device context: */
	dc1394camera_list_t* cameraList=0;
	if(dc1394_camera_enumerate(context,&cameraList)!=DC1394_SUCCESS||cameraList==0)
		{
		dc1394_free(context);
		return;
		}
	
	/* Process all cameras: */
	Misc::HashTable<CameraVendorModelId,unsigned int> cameraHash(17);
	for(unsigned int i=0;i<cameraList->num;++i)
		{
		/* Open the camera: */
		dc1394camera_t* camera=dc1394_camera_new_unit(context,cameraList->ids[i].guid,cameraList->ids[i].unit);
		if(camera!=0)
			{
			/* Store the camera's name: */
			std::string name=camera->vendor;
			name.push_back(' ');
			name.append(camera->model);
			
			/* Check if a camera of the same vendor / model ID was already enumerated: */
			CameraVendorModelId cvm(camera->vendor_id,camera->model_id);
			Misc::HashTable<CameraVendorModelId,unsigned int>::Iterator cIt=cameraHash.findEntry(cvm);
			if(cIt.isFinished())
				{
				/* Store the camera's vendor / model ID: */
				cameraHash.setEntry(Misc::HashTable<CameraVendorModelId,unsigned int>::Entry(cvm,0));
				}
			else
				{
				/* Append an index to the name: */
				name.push_back(':');
				char index[10];
				++cIt->getDest();
				snprintf(index,sizeof(index),"%u",cIt->getDest());
				name.append(index);
				}
			
			/* Create a device ID for the camera: */
			DeviceId* newDeviceId=new DeviceId(name);
			
			/* Store the camera's GUID and unit index: */
			newDeviceId->guid=cameraList->ids[i].guid;
			newDeviceId->unitIndex=cameraList->ids[i].unit;
			
			/* Store the device ID: */
			devices.push_back(newDeviceId);
			
			/* Clean up: */
			dc1394_camera_free(camera);
			}
		}
	
	/* Clean up: */
	dc1394_camera_free_list(cameraList);
	dc1394_free(context);
	}

}
