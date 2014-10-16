/***********************************************************************
V4L2VideoDevice - Wrapper class around video devices as represented by
the Video for Linux version 2 (V4L2) library.
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

#include <Video/Linux/V4L2VideoDevice.h>

#include <Images/Config.h>
#include <Video/Config.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <string>
#include <vector>
#include <Misc/FunctionCalls.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/Label.h>
#include <GLMotif/Slider.h>
#include <GLMotif/Margin.h>
#include <GLMotif/ToggleButton.h>
#include <GLMotif/DropdownBox.h>
#include <Video/ImageExtractorY10B.h>
#include <Video/ImageExtractorYUYV.h>
#include <Video/ImageExtractorUYVY.h>
#include <Video/ImageExtractorBA81.h>
#if IMAGES_CONFIG_HAVE_JPEG
#include <Video/ImageExtractorMJPG.h>
#endif

namespace Video {

/******************************************
Methods of class V4L2VideoDevice::DeviceId:
******************************************/

VideoDevice* V4L2VideoDevice::DeviceId::createDevice(void) const
	{
	return new V4L2VideoDevice(deviceFileName.c_str());
	}

/********************************
Methods of class V4L2VideoDevice:
********************************/

void V4L2VideoDevice::enumFrameIntervals(VideoDataFormat& format,std::vector<VideoDataFormat>& formatList) const
	{
	/* Enumerate the format's available frame intervals: */
	v4l2_frmivalenum frameIntervalEnum;
	memset(&frameIntervalEnum,0,sizeof(v4l2_frmivalenum));
	frameIntervalEnum.index=0;
	frameIntervalEnum.pixel_format=format.pixelFormat;
	frameIntervalEnum.width=format.size[0];
	frameIntervalEnum.height=format.size[1];
	while(ioctl(videoFd,VIDIOC_ENUM_FRAMEINTERVALS,&frameIntervalEnum)==0)
		{
		/* Check if the frame interval is discrete or stepwise (ignore continuous): */
		if(frameIntervalEnum.type==V4L2_FRMIVAL_TYPE_DISCRETE)
			{
			/* Add the video data format to the list: */
			format.frameIntervalCounter=frameIntervalEnum.discrete.numerator;
			format.frameIntervalDenominator=frameIntervalEnum.discrete.denominator;
			formatList.push_back(format);
			}
		else if(frameIntervalEnum.type==V4L2_FRMIVAL_TYPE_STEPWISE)
			{
			/* Check if the denominators match: */
			if(frameIntervalEnum.stepwise.min.denominator==frameIntervalEnum.stepwise.step.denominator&&frameIntervalEnum.stepwise.step.denominator==frameIntervalEnum.stepwise.max.denominator)
				{
				/* Add a video data format for each available frame interval to the list: */
				format.frameIntervalCounter=frameIntervalEnum.stepwise.min.numerator;
				format.frameIntervalDenominator=frameIntervalEnum.stepwise.min.denominator;
				while(format.frameIntervalCounter<=frameIntervalEnum.stepwise.max.numerator)
					{
					/* Add the video data format to the list: */
					formatList.push_back(format);
					
					/* Go to the next frame interval step: */
					format.frameIntervalCounter+=frameIntervalEnum.stepwise.step.numerator;
					}
				}
			}
		
		/* Go to the next frame interval: */
		++frameIntervalEnum.index;
		}
	}

void V4L2VideoDevice::setControl(unsigned int controlId,const char* controlTag,const Misc::ConfigurationFileSection& cfg)
	{
	/* Query the control's type and value range: */
	v4l2_queryctrl queryControl;
	memset(&queryControl,0,sizeof(v4l2_queryctrl));
	queryControl.id=controlId;
	if(ioctl(videoFd,VIDIOC_QUERYCTRL,&queryControl)!=0)
		return; // Fail silently; control is just not supported by camera
	
	/* Query the control's current value: */
	v4l2_control control;
	memset(&control,0,sizeof(v4l2_control));
	control.id=controlId;
	if(ioctl(videoFd,VIDIOC_G_CTRL,&control)!=0)
		return;
		// Misc::throwStdErr("Video::V4L2VideoDevice::setControl: Error while querying control %s",controlTag);
	
	/* Retrieve the desired control value from the configuration file section: */
	int oldControlValue=control.value;
	if(queryControl.type==V4L2_CTRL_TYPE_INTEGER)
		control.value=cfg.retrieveValue<int>(controlTag,control.value);
	else if(queryControl.type==V4L2_CTRL_TYPE_BOOLEAN)
		control.value=cfg.retrieveValue<bool>(controlTag,control.value!=0)?1:0;
	else if(queryControl.type==V4L2_CTRL_TYPE_MENU)
		{
		/* Query the name of the currently selected menu choice: */
		v4l2_querymenu queryMenu;
		memset(&queryMenu,0,sizeof(queryMenu));
		queryMenu.id=controlId;
		queryMenu.index=control.value;
		if(ioctl(videoFd,VIDIOC_QUERYMENU,&queryMenu)!=0)
			{
			/* Let's agree to fail silently for now: */
			// Misc::throwStdErr("Video::V4L2VideoDevice::setControl: Error while querying control %s",controlTag);
			return;
			}
		
		/* Retrieve the desired menu choice from the configuration file: */
		std::string menuChoice=cfg.retrieveValue<std::string>(controlTag,reinterpret_cast<char*>(queryMenu.name));
		
		/* Find the index of the selected menu choice: */
		int selectedMenuChoice=-1;
		for(int index=0;index<=queryControl.maximum;++index)
			{
			queryMenu.id=controlId;
			queryMenu.index=index;
			if(ioctl(videoFd,VIDIOC_QUERYMENU,&queryMenu)!=0)
				{
				/* Let's agree to fail silently for now: */
				// Misc::throwStdErr("Video::V4L2VideoDevice::setControl: Error while querying control %s",controlTag);
				return;
				}
			if(menuChoice==reinterpret_cast<char*>(queryMenu.name))
				{
				selectedMenuChoice=index;
				break;
				}
			}
		if(selectedMenuChoice<0)
			{
			/* Let's agree to fail silently for now: */
			// Misc::throwStdErr("Video::V4L2VideoDevice::setControl: %s is not a valid choice for menu control %s",menuChoice.c_str(),controlTag);
			return;
			}
		control.value=selectedMenuChoice;
		}
	else
		{
		/* Let's agree to fail silently for now: */
		// Misc::throwStdErr("Video::V4L2VideoDevice::setControl: %s is not a settable control",controlTag);
		return;
		}
	
	if(control.value!=oldControlValue)
		{
		/* Set the control's value: */
		control.id=controlId;
		if(ioctl(videoFd,VIDIOC_S_CTRL,&control)!=0)
			{
			/* Let's agree to fail silently for now: */
			// Misc::throwStdErr("Video::V4L2VideoDevice::setControl: Error while setting value for control %s",controlTag);
			return;
			}
		}
	}

void V4L2VideoDevice::integerControlChangedCallback(Misc::CallbackData* cbData,const unsigned int& controlId)
	{
	/* Get the proper callback data type: */
	GLMotif::Slider::ValueChangedCallbackData* myCbData=dynamic_cast<GLMotif::Slider::ValueChangedCallbackData*>(cbData);
	if(myCbData==0)
		return;
	
	/* Change the video device's control value: */
	v4l2_control control;
	control.id=controlId;
	control.value=int(Math::floor(myCbData->value+0.5f));
	ioctl(videoFd,VIDIOC_S_CTRL,&control);
	}

void V4L2VideoDevice::booleanControlChangedCallback(Misc::CallbackData* cbData,const unsigned int& controlId)
	{
	/* Get the proper callback data type: */
	GLMotif::ToggleButton::ValueChangedCallbackData* myCbData=dynamic_cast<GLMotif::ToggleButton::ValueChangedCallbackData*>(cbData);
	if(myCbData==0)
		return;
	
	/* Change the video device's control value: */
	v4l2_control control;
	control.id=controlId;
	control.value=myCbData->set?1:0;
	ioctl(videoFd,VIDIOC_S_CTRL,&control);
	}

void V4L2VideoDevice::menuControlChangedCallback(Misc::CallbackData* cbData,const unsigned int& controlId)
	{
	/* Get the proper callback data type: */
	GLMotif::DropdownBox::ValueChangedCallbackData* myCbData=dynamic_cast<GLMotif::DropdownBox::ValueChangedCallbackData*>(cbData);
	if(myCbData==0)
		return;
	
	/* Change the video device's control value: */
	v4l2_control control;
	control.id=controlId;
	control.value=myCbData->newSelectedItem;
	ioctl(videoFd,VIDIOC_S_CTRL,&control);
	}

void* V4L2VideoDevice::streamingThreadMethod(void)
	{
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	// Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	while(true)
		{
		/* Dequeue the next available frame buffer: */
		v4l2_buffer buffer;
		memset(&buffer,0,sizeof(v4l2_buffer));
		buffer.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if(frameBuffersMemoryMapped)
			buffer.memory=V4L2_MEMORY_MMAP;
		else
			buffer.memory=V4L2_MEMORY_USERPTR;
		if(ioctl(videoFd,VIDIOC_DQBUF,&buffer)<0)
			{
			if(errno==EINTR) // If the ioctl was interrupted, try again
				{
				/* Ignore and try again: */
				continue;
				}
			else
				break;
			}
		
		/* Find the frame buffer object, and fill in its capture state: */
		V4L2FrameBuffer* frame=&frameBuffers[buffer.index];
		frame->sequence=buffer.sequence;
		frame->used=buffer.bytesused;
		
		/* Call the streaming callback: */
		(*streamingCallback)(frame);
		
		/* Put the frame buffer back into the capture queue: */
		if(ioctl(videoFd,VIDIOC_QBUF,&buffer)<0)
			{
			/* This is a serious problem, so we have to bail out: */
			break;
			}
		}
	
	return 0;
	}

V4L2VideoDevice::V4L2VideoDevice(const char* videoDeviceName)
	:videoFd(-1),
	 canRead(false),canStream(false),
	 frameBuffersMemoryMapped(false),numFrameBuffers(0),frameBuffers(0)
	{
	/* Open the video device: */
	videoFd=open(videoDeviceName,O_RDWR); // Read/write access is required, even for capture only!
	if(videoFd<0)
		Misc::throwStdErr("Video::V4L2VideoDevice::V4L2VideoDevice: Unable to open V4L2 video device %s",videoDeviceName);
	
	/* Check that the device can capture video: */
	{
	v4l2_capability videoCap;
	if(ioctl(videoFd,VIDIOC_QUERYCAP,&videoCap)!=0)
		{
		close(videoFd);
		Misc::throwStdErr("Video::V4L2VideoDevice::V4L2VideoDevice: Error while querying capabilities of V4L2 video device %s",videoDeviceName);
		}
	
	/* Check for capture capability: */
	if((videoCap.capabilities&V4L2_CAP_VIDEO_CAPTURE)==0)
		{
		close(videoFd);
		Misc::throwStdErr("Video::V4L2VideoDevice::V4L2VideoDevice: V4L2 video device %s does not support video capture",videoDeviceName);
		}
	
	/* Check for supported I/O modes: */
	canRead=(videoCap.capabilities&V4L2_CAP_READWRITE);
	canStream=(videoCap.capabilities&V4L2_CAP_STREAMING);
	}
	}

V4L2VideoDevice::~V4L2VideoDevice(void)
	{
	if(streamingCallback!=0)
		{
		/* Stop the background streaming thread: */
		streamingThread.cancel();
		streamingThread.join();
		}
	
	/* Release all allocated frame buffers: */
	for(unsigned int i=0;i<numFrameBuffers;++i)
		{
		/* Unmap a memory-mapped buffer; delete[] a user-space buffer: */
		if(frameBuffersMemoryMapped)
			{
			if(frameBuffers[i].start!=0)
				munmap(frameBuffers[i].start,frameBuffers[i].size);
			}
		else
			delete[] frameBuffers[i].start;
		}
	delete[] frameBuffers;
	
	/* Close the video device, if it was ever opened: */
	if(videoFd>=0)
		close(videoFd);
	}

std::vector<VideoDataFormat> V4L2VideoDevice::getVideoFormatList(void) const
	{
	std::vector<VideoDataFormat> result;
	
	/* Enumerate all available video formats: */
	v4l2_fmtdesc formatDesc;
	memset(&formatDesc,0,sizeof(v4l2_fmtdesc));
	formatDesc.index=0;
	formatDesc.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	while(ioctl(videoFd,VIDIOC_ENUM_FMT,&formatDesc)==0)
		{
		VideoDataFormat format;
		
		/* Copy the format's pixel format: */
		format.pixelFormat=formatDesc.pixelformat;
		
		/* Enumerate the format's available image sizes: */
		v4l2_frmsizeenum frameSizeEnum;
		memset(&frameSizeEnum,0,sizeof(v4l2_frmsizeenum));
		frameSizeEnum.index=0;
		frameSizeEnum.pixel_format=formatDesc.pixelformat;
		while(ioctl(videoFd,VIDIOC_ENUM_FRAMESIZES,&frameSizeEnum)==0)
			{
			/* Check if the frame size is discrete or stepwise (ignore continuous): */
			if(frameSizeEnum.type==V4L2_FRMSIZE_TYPE_DISCRETE)
				{
				/* Update the video data format's size: */
				format.size[0]=frameSizeEnum.discrete.width;
				format.size[1]=frameSizeEnum.discrete.height;
				
				/* Enumerate the format's available frame intervals: */
				enumFrameIntervals(format,result);
				}
			else if(frameSizeEnum.type==V4L2_FRMSIZE_TYPE_STEPWISE)
				{
				/* Append video data format for all available frame sizes to the list: */
				format.size[0]=frameSizeEnum.stepwise.min_width;
				format.size[1]=frameSizeEnum.stepwise.min_height;
				while(format.size[0]<=frameSizeEnum.stepwise.max_width&&format.size[1]<=frameSizeEnum.stepwise.max_height)
					{
					/* Enumerate the format's available frame intervals: */
					enumFrameIntervals(format,result);
					
					/* Go to the next frame size step: */
					format.size[0]+=frameSizeEnum.stepwise.step_width;
					format.size[1]+=frameSizeEnum.stepwise.step_height;
					}
				}
			
			/* Go to the next frame size: */
			++frameSizeEnum.index;
			}
		
		/* Go to the next video format: */
		++formatDesc.index;
		}
	return result;
	}

VideoDataFormat V4L2VideoDevice::getVideoFormat(void) const
	{
	VideoDataFormat result;
	
	/* Query the current image format: */
	v4l2_format videoFormat;
	memset(&videoFormat,0,sizeof(v4l2_format));
	videoFormat.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl(videoFd,VIDIOC_G_FMT,&videoFormat)!=0)
		Misc::throwStdErr("Video::V4L2VideoDevice::getVideoFormat: Error while querying image format");
	
	result.pixelFormat=videoFormat.fmt.pix.pixelformat;
	result.size[0]=videoFormat.fmt.pix.width;
	result.size[1]=videoFormat.fmt.pix.height;
	result.lineSize=videoFormat.fmt.pix.bytesperline;
	result.frameSize=videoFormat.fmt.pix.sizeimage;
	
	/* Query the current frame interval: */
	v4l2_streamparm streamParameters;
	memset(&streamParameters,0,sizeof(v4l2_streamparm));
	streamParameters.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl(videoFd,VIDIOC_G_PARM,&streamParameters)!=0)
		Misc::throwStdErr("Video::V4L2VideoDevice::getVideoFormat: Error while querying capture frame rate");
	result.frameIntervalCounter=streamParameters.parm.capture.timeperframe.numerator;
	result.frameIntervalDenominator=streamParameters.parm.capture.timeperframe.denominator;
	
	return result;
	}

VideoDataFormat& V4L2VideoDevice::setVideoFormat(VideoDataFormat& newFormat)
	{
	v4l2_format format;
	memset(&format,0,sizeof(v4l2_format));
	format.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	format.fmt.pix.width=newFormat.size[0];
	format.fmt.pix.height=newFormat.size[1];
	format.fmt.pix.pixelformat=newFormat.pixelFormat;
	format.fmt.pix.field=V4L2_FIELD_ANY;
	if(ioctl(videoFd,VIDIOC_S_FMT,&format)!=0)
		Misc::throwStdErr("Video::V4L2VideoDevice::setVideoFormat: Error while setting image format");
	
	v4l2_streamparm streamParameters;
	memset(&streamParameters,0,sizeof(v4l2_streamparm));
	streamParameters.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	streamParameters.parm.capture.timeperframe.numerator=newFormat.frameIntervalCounter;
	streamParameters.parm.capture.timeperframe.denominator=newFormat.frameIntervalDenominator;
	if(ioctl(videoFd,VIDIOC_S_PARM,&streamParameters)!=0)
		Misc::throwStdErr("Video::V4L2VideoDevice::setVideoFormat: Error while setting capture frame rate");
	
	/* Update the format structure: */
	newFormat.pixelFormat=format.fmt.pix.pixelformat;
	newFormat.size[0]=format.fmt.pix.width;
	newFormat.size[1]=format.fmt.pix.height;
	newFormat.lineSize=format.fmt.pix.bytesperline;
	newFormat.frameSize=format.fmt.pix.sizeimage;
	newFormat.frameIntervalCounter=streamParameters.parm.capture.timeperframe.numerator;
	newFormat.frameIntervalDenominator=streamParameters.parm.capture.timeperframe.denominator;
	
	return newFormat;
	}

void V4L2VideoDevice::configure(const Misc::ConfigurationFileSection& cfg)
	{
	/* Call the base class method to select frame size, frame rate, and pixel format: */
	VideoDevice::configure(cfg);
	
	/****************************************
	Set the video device's standard controls:
	****************************************/
	
	setControl(V4L2_CID_BRIGHTNESS,"brightness",cfg);
	setControl(V4L2_CID_CONTRAST,"contrast",cfg);
	setControl(V4L2_CID_SATURATION,"saturation",cfg);
	setControl(V4L2_CID_HUE,"hue",cfg);
	setControl(V4L2_CID_AUTO_WHITE_BALANCE,"autoWhiteBalance",cfg);
	setControl(V4L2_CID_GAMMA,"gamma",cfg);
	setControl(V4L2_CID_EXPOSURE,"exposure",cfg);
	setControl(V4L2_CID_AUTOGAIN,"autoGain",cfg);
	setControl(V4L2_CID_GAIN,"gain",cfg);
	setControl(V4L2_CID_POWER_LINE_FREQUENCY,"powerLineFrequency",cfg);
	setControl(V4L2_CID_WHITE_BALANCE_TEMPERATURE,"whiteBalanceTemperature",cfg);
	setControl(V4L2_CID_SHARPNESS,"sharpness",cfg);
	setControl(V4L2_CID_BACKLIGHT_COMPENSATION,"gamma",cfg);
	}

ImageExtractor* V4L2VideoDevice::createImageExtractor(void) const
	{
	/* Get the current video format: */
	VideoDataFormat format=getVideoFormat();
	
	/* Create an extractor based on the video format's pixel format: */
	if(format.isPixelFormat("Y10B"))
		return new ImageExtractorY10B(format.size);
	else if(format.isPixelFormat("YUYV"))
		return new ImageExtractorYUYV(format.size);
	else if(format.isPixelFormat("UYVY"))
		return new ImageExtractorUYVY(format.size);
	else if(format.isPixelFormat("GRBG"))
		return new ImageExtractorBA81(format.size,BAYER_GRBG);
	#if IMAGES_CONFIG_HAVE_JPEG
	else if(format.isPixelFormat("MJPG"))
		return new ImageExtractorMJPG(format.size);
	#endif
	else
		{
		char fcc[5];
		Misc::throwStdErr("Video::V4L2VideoDevice::createImageExtractor: Unknown pixel format %s",format.getFourCC(fcc));
		}
	
	/* Execution never gets here; just to make compiler happy: */
	return 0;
	}

GLMotif::Widget* V4L2VideoDevice::createControlPanel(GLMotif::WidgetManager* widgetManager)
	{
	/* Get the style sheet: */
	const GLMotif::StyleSheet* ss=widgetManager->getStyleSheet();
	
	/* Create the control panel dialog window: */
	GLMotif::PopupWindow* controlPanelPopup=new GLMotif::PopupWindow("V4L2VideoDeviceControlPanelPopup",widgetManager,"Video Source Control");
	controlPanelPopup->setResizableFlags(true,false);
	
	GLMotif::RowColumn* controlPanel=new GLMotif::RowColumn("ControlPanel",controlPanelPopup,false);
	controlPanel->setNumMinorWidgets(2);
	
	/* Enumerate all controls exposed by the V4L2 video device: */
	v4l2_queryctrl queryControl;
	memset(&queryControl,0,sizeof(v4l2_queryctrl));
	queryControl.id=V4L2_CTRL_FLAG_NEXT_CTRL;
	while(ioctl(videoFd,VIDIOC_QUERYCTRL,&queryControl)==0)
		{
		/* Query the control's current value: */
		v4l2_control control;
		memset(&control,0,sizeof(v4l2_control));
		control.id=queryControl.id;
		if(ioctl(videoFd,VIDIOC_G_CTRL,&control)==0)
			{
			/* Create a control row for the current control: */
			char widgetName[40];
			
			/* Create a label naming the control: */
			snprintf(widgetName,sizeof(widgetName),"Label%u",queryControl.id);
			new GLMotif::Label(widgetName,controlPanel,reinterpret_cast<char*>(queryControl.name));
			
			/* Create a widget to change the control's value: */
			if(queryControl.type==V4L2_CTRL_TYPE_INTEGER)
				{
				/* Create a slider: */
				snprintf(widgetName,sizeof(widgetName),"Slider%u",queryControl.id);
				GLMotif::Slider* controlSlider=new GLMotif::Slider(widgetName,controlPanel,GLMotif::Slider::HORIZONTAL,ss->fontHeight*10.0f);
				controlSlider->setValueRange(queryControl.minimum,queryControl.maximum,queryControl.step);
				controlSlider->setValue(control.value);
				controlSlider->getValueChangedCallbacks().add(this,&V4L2VideoDevice::integerControlChangedCallback,(unsigned int)queryControl.id);
				}
			else if(queryControl.type==V4L2_CTRL_TYPE_BOOLEAN)
				{
				/* Create a toggle button inside a margin: */
				snprintf(widgetName,sizeof(widgetName),"Margin%u",queryControl.id);
				GLMotif::Margin* controlMargin=new GLMotif::Margin(widgetName,controlPanel,false);
				controlMargin->setAlignment(GLMotif::Alignment::LEFT);
				
				snprintf(widgetName,sizeof(widgetName),"ToggleButton%u",queryControl.id);
				GLMotif::ToggleButton* controlToggleButton=new GLMotif::ToggleButton(widgetName,controlMargin,"Enabled");
				controlToggleButton->setBorderWidth(0.0f);
				controlToggleButton->setHAlignment(GLFont::Left);
				controlToggleButton->setToggle(control.value!=0);
				controlToggleButton->getValueChangedCallbacks().add(this,&V4L2VideoDevice::booleanControlChangedCallback,(unsigned int)queryControl.id);
				
				controlMargin->manageChild();
				}
			else if(queryControl.type==V4L2_CTRL_TYPE_MENU)
				{
				/* Query the names of all menu choices: */
				std::vector<std::string> menuChoices;
				for(int menuItem=0;menuItem<=queryControl.maximum;++menuItem)
					{
					v4l2_querymenu queryMenu;
					memset(&queryMenu,0,sizeof(queryMenu));
					queryMenu.id=queryControl.id;
					queryMenu.index=menuItem;
					if(ioctl(videoFd,VIDIOC_QUERYMENU,&queryMenu)==0)
						menuChoices.push_back(reinterpret_cast<char*>(queryMenu.name));
					}
				
				/* Create a drop-down box inside a margin: */
				snprintf(widgetName,sizeof(widgetName),"Margin%u",queryControl.id);
				GLMotif::Margin* controlMargin=new GLMotif::Margin(widgetName,controlPanel,false);
				controlMargin->setAlignment(GLMotif::Alignment::LEFT);
				
				snprintf(widgetName,sizeof(widgetName),"DropdownBox%u",queryControl.id);
				GLMotif::DropdownBox* controlDropdownBox=new GLMotif::DropdownBox(widgetName,controlMargin,menuChoices);
				controlDropdownBox->setSelectedItem(control.value);
				controlDropdownBox->getValueChangedCallbacks().add(this,&V4L2VideoDevice::menuControlChangedCallback,(unsigned int)queryControl.id);
				
				controlMargin->manageChild();
				}
			}
		
		/* Query the next control: */
		queryControl.id|=V4L2_CTRL_FLAG_NEXT_CTRL;
		}
	
	controlPanel->manageChild();
	
	return controlPanelPopup;
	}

unsigned int V4L2VideoDevice::allocateFrameBuffers(unsigned int requestedNumFrameBuffers)
	{
	/* Check if the device supports streaming: */
	if(!canStream)
		Misc::throwStdErr("Video::V4L2VideoDevice::allocateFrameBuffers: Video device does not support streaming I/O");
	
	/* Request the given number of frame buffers: */
	frameBuffersMemoryMapped=true;
	v4l2_requestbuffers requestBuffers;
	memset(&requestBuffers,0,sizeof(v4l2_requestbuffers));
	requestBuffers.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	requestBuffers.memory=V4L2_MEMORY_MMAP;
	requestBuffers.count=requestedNumFrameBuffers;
	if(ioctl(videoFd,VIDIOC_REQBUFS,&requestBuffers)==0)
		{
		/* Allocate the frame buffer set: */
		numFrameBuffers=requestBuffers.count;
		frameBuffers=new V4L2FrameBuffer[numFrameBuffers];
		bool frameBuffersOk=true;
		for(unsigned int i=0;i<numFrameBuffers;++i)
			{
			/* Query the frame buffer's size and device space offset: */
			v4l2_buffer buffer;
			memset(&buffer,0,sizeof(v4l2_buffer));
			buffer.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buffer.memory=V4L2_MEMORY_MMAP;
			buffer.index=i;
			if(ioctl(videoFd,VIDIOC_QUERYBUF,&buffer)!=0)
				{
				/* That's a nasty error: */
				frameBuffersOk=false;
				break;
				}
			
			/* Map the device driver space frame buffer into application space: */
			void* bufferStart=mmap(0,buffer.length,PROT_READ|PROT_WRITE,MAP_SHARED,videoFd,buffer.m.offset);
			if(bufferStart==MAP_FAILED)
				{
				/* That's another nasty error: */
				frameBuffersOk=false;
				break;
				}
			frameBuffers[i].index=i;
			frameBuffers[i].start=reinterpret_cast<unsigned char*>(bufferStart);
			frameBuffers[i].size=buffer.length;
			}
		
		if(!frameBuffersOk)
			{
			/* Release all successfully allocated frame buffers: */
			releaseFrameBuffers();
			
			/* Signal an error: */
			Misc::throwStdErr("Video::V4L2VideoDevice::allocateFrameBuffers: Error while allocating memory-mapped streaming buffers");
			}
		}
	else
		{
		/* Video device can't do memory-mapped I/O; fall back to user pointer I/O: */
		frameBuffersMemoryMapped=false;
		requestBuffers.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
		requestBuffers.memory=V4L2_MEMORY_USERPTR;
		if(ioctl(videoFd,VIDIOC_REQBUFS,&requestBuffers)!=0)
			Misc::throwStdErr("Video::V4L2VideoDevice::allocateFrameBuffers: Error while allocating user-space streaming buffers");
		
		/* Determine the required frame buffer size: */
		VideoDataFormat currentFormat=getVideoFormat();
		
		/* Allocate all frame buffers using good old new[]: */
		numFrameBuffers=requestedNumFrameBuffers;
		frameBuffers=new V4L2FrameBuffer[numFrameBuffers];
		for(unsigned int i=0;i<numFrameBuffers;++i)
			{
			frameBuffers[i].index=i;
			frameBuffers[i].start=new unsigned char[currentFormat.frameSize];
			frameBuffers[i].size=currentFormat.frameSize;
			}
		}
	
	return numFrameBuffers;
	}

void V4L2VideoDevice::startStreaming(void)
	{
	/* Call the base class method: */
	VideoDevice::startStreaming();
	
	/* Enqueue all frame buffers: */
	for(unsigned int i=0;i<numFrameBuffers;++i)
		enqueueFrame(&frameBuffers[i]);
	
	/* Start streaming: */
	int streamType=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl(videoFd,VIDIOC_STREAMON,&streamType)!=0)
		Misc::throwStdErr("Video::V4L2VideoDevice::startStreaming: Error starting streaming video capture");
	}

void V4L2VideoDevice::startStreaming(VideoDevice::StreamingCallback* newStreamingCallback)
	{
	/* Call the base class method: */
	VideoDevice::startStreaming(newStreamingCallback);
	
	/* Enqueue all frame buffers: */
	for(unsigned int i=0;i<numFrameBuffers;++i)
		enqueueFrame(&frameBuffers[i]);
	
	/* Start streaming: */
	int streamType=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl(videoFd,VIDIOC_STREAMON,&streamType)!=0)
		Misc::throwStdErr("Video::V4L2VideoDevice::startStreaming: Error starting streaming video capture");
	
	/* Start the background capture thread: */
	streamingThread.start(this,&V4L2VideoDevice::streamingThreadMethod);
	}

FrameBuffer* V4L2VideoDevice::dequeueFrame(void)
	{
	/* Dequeue the next available frame buffer: */
	v4l2_buffer buffer;
	memset(&buffer,0,sizeof(v4l2_buffer));
	buffer.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(frameBuffersMemoryMapped)
		buffer.memory=V4L2_MEMORY_MMAP;
	else
		buffer.memory=V4L2_MEMORY_USERPTR;
	if(ioctl(videoFd,VIDIOC_DQBUF,&buffer)!=0)
		Misc::throwStdErr("Video::V4L2VideoDevice::getNextFrame: Error while dequeueing frame buffer");
	
	/* Find the frame buffer object, and fill in its capture state: */
	V4L2FrameBuffer* frame=&frameBuffers[buffer.index];
	frame->sequence=buffer.sequence;
	frame->used=buffer.bytesused;
	
	return frame;
	}

void V4L2VideoDevice::enqueueFrame(FrameBuffer* frame)
	{
	V4L2FrameBuffer* myFrame=dynamic_cast<V4L2FrameBuffer*>(frame);
	if(myFrame==0)
		Misc::throwStdErr("Video::V4L2VideoDevice::enqueueFrame: Mismatching frame buffer type");
	
	v4l2_buffer buffer;
	memset(&buffer,0,sizeof(v4l2_buffer));
	buffer.index=myFrame->index;
	buffer.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(frameBuffersMemoryMapped)
		buffer.memory=V4L2_MEMORY_MMAP;
	else
		{
		buffer.memory=V4L2_MEMORY_USERPTR;
		buffer.m.userptr=reinterpret_cast<unsigned long>(myFrame->start);
		buffer.length=myFrame->size;
		}
	if(ioctl(videoFd,VIDIOC_QBUF,&buffer)!=0)
		Misc::throwStdErr("Video::V4L2VideoDevice::enqueueFrame: Error while enqueueing frame buffer");
	}

void V4L2VideoDevice::stopStreaming(void)
	{
	/* Stop streaming: */
	int streamType=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl(videoFd,VIDIOC_STREAMOFF,&streamType)!=0)
		Misc::throwStdErr("Video::V4L2VideoDevice::stopStreaming: Error stopping streaming video capture");
	
	if(streamingCallback!=0)
		{
		/* Stop the background streaming thread: */
		streamingThread.cancel();
		streamingThread.join();
		}
	
	/* Call the base class method: */
	VideoDevice::stopStreaming();
	}

void V4L2VideoDevice::releaseFrameBuffers(void)
	{
	/* Release all successfully allocated buffers: */
	for(unsigned int i=0;i<numFrameBuffers;++i)
		{
		if(frameBuffersMemoryMapped)
			{
			if(frameBuffers[i].start!=0)
				munmap(frameBuffers[i].start,frameBuffers[i].size);
			}
		else
			delete[] frameBuffers[i].start;
		}
	numFrameBuffers=0;
	delete[] frameBuffers;
	frameBuffers=0;
	
	if(frameBuffersMemoryMapped)
		{
		/* Release all requested buffers on the driver side: */
		v4l2_requestbuffers requestBuffers;
		memset(&requestBuffers,0,sizeof(v4l2_requestbuffers));
		requestBuffers.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
		requestBuffers.memory=V4L2_MEMORY_MMAP;
		requestBuffers.count=0;
		ioctl(videoFd,VIDIOC_REQBUFS,&requestBuffers);
		frameBuffersMemoryMapped=false;
		}
	}

void V4L2VideoDevice::enumerateDevices(std::vector<VideoDevice::DeviceIdPtr>& devices)
	{
	/* Enumerate all /dev/videoXXX device file nodes: */
	for(int videoDeviceIndex=0;;++videoDeviceIndex)
		{
		/* Create the device file name: */
		char deviceFileName[40];
		snprintf(deviceFileName,sizeof(deviceFileName),"/dev/video%d",videoDeviceIndex);
		
		/* Try opening the device file: */
		int videoFd=open(deviceFileName,O_RDWR); // Read/write access is required, even for capture only!
		if(videoFd<0)
			{
			if(errno==ENOENT) // No more video device nodes
				break;
			else
				continue; // Some other error; ignore this device node
			}
		
		/* Check if the device can capture video in streaming mode: */
		v4l2_capability videoCap;
		if(ioctl(videoFd,VIDIOC_QUERYCAP,&videoCap)==0)
			if((videoCap.capabilities&V4L2_CAP_VIDEO_CAPTURE)!=0&&(videoCap.capabilities&V4L2_CAP_STREAMING)!=0)
				{
				/* Query the device's name: */
				std::string name=reinterpret_cast<const char*>(videoCap.card);
				
				/* Create a device ID: */
				DeviceId* newDeviceId=new DeviceId(name);
				
				/* Store the device file name: */
				newDeviceId->deviceFileName=deviceFileName;
				
				/* Store the device ID: */
				devices.push_back(newDeviceId);
				}
		
		/* Close the device file: */
		close(videoFd);
		}
	}

}
