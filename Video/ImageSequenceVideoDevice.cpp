/***********************************************************************
ImageSequenceVideoDevice - Class for "fake" video capture devices
showing a set of image files.
Copyright (c) 2014 Oliver Kreylos

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

#include <Video/ImageSequenceVideoDevice.h>

#include <stdio.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/FunctionCalls.h>
#include <IO/OpenFile.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <Images/ReadImageFile.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/Label.h>
#include <Video/ImageExtractorRGB8.h>

namespace Video {

/***************************************************
Methods of class ImageSequenceVideoDevice::DeviceId:
***************************************************/

VideoDevice* ImageSequenceVideoDevice::DeviceId::createDevice(void) const
	{
	return new ImageSequenceVideoDevice(frameNameTemplate.c_str());
	}

/*****************************************
Methods of class ImageSequenceVideoDevice:
*****************************************/

void ImageSequenceVideoDevice::loadFrame(int frameIndex)
	{
	/* Load the requested image: */
	char frameName[2048];
	snprintf(frameName,sizeof(frameName),frameNameTemplate.c_str(),frameIndex);
	currentFrame=Images::readImageFile(frameName,frameDir->openFile(frameName));
	
	/* Update the frame buffer for streaming: */
	currentBuffer.start=currentFrame.modifyPixels()->getRgba();
	currentBuffer.size=frameSize[1]*frameSize[0]*3*sizeof(unsigned char);
	currentBuffer.used=currentBuffer.size;
	}

void ImageSequenceVideoDevice::frameIndexSliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData)
	{
	/* Load the requested frame: */
	currentIndex=int(Math::floor(cbData->value+0.5));
	loadFrame(currentIndex);
	
	/* Check if we're currently streaming: */
	if(streamingCallback!=0)
		{
		/* Call the callback function: */
		(*streamingCallback)(const_cast<const FrameBuffer*>(&currentBuffer));
		}
	}

ImageSequenceVideoDevice::ImageSequenceVideoDevice(const char* sFrameNameTemplate)
	:frameNameTemplate(sFrameNameTemplate)
	{
	/* Check if the frame name template is valid: */
	int indexBegin=-1;
	int indexEnd=-1;
	int lastSlash=-1;
	int length=int(frameNameTemplate.length());
	int i=0;
	while(i<length)
		{
		/* Look for the next conversion: */
		while(i<length&&frameNameTemplate[i]!='%')
			{
			if(frameNameTemplate[i]=='/')
				lastSlash=i;
			++i;
			}
		if(i>=length)
			break;
		
		/* Check if it's a decimal conversion: */
		int begin=i;
		++i;
		while(i<length&&frameNameTemplate[i]>='0'&&frameNameTemplate[i]<='9')
			++i;
		if(i>=length)
			break;
		if(frameNameTemplate[i]=='d')
			{
			if(indexBegin==-1)
				{
				indexBegin=begin;
				++i;
				indexEnd=i;
				}
			else
				Misc::throwStdErr("Video::ImageSequenceVideoDevice: More than one %d conversion in frame name template %s",frameNameTemplate.c_str());
			}
		else
			Misc::throwStdErr("Video::ImageSequenceVideoDevice: Invalid % conversion in frame name template %s",frameNameTemplate.c_str());
		}
	if(indexBegin==-1)
		Misc::throwStdErr("Video::ImageSequenceVideoDevice: No %d conversion in frame name template %s",frameNameTemplate.c_str());
	
	/* Split the frame name template into directory and file name: */
	if(lastSlash>=indexBegin)
		Misc::throwStdErr("Video::ImageSequenceVideoDevice: Frame name template %s has %d conversion in path name",frameNameTemplate.c_str());
	std::string frameDirName(frameNameTemplate.begin(),frameNameTemplate.begin()+lastSlash+1);
	frameDir=IO::openDirectory(frameDirName.c_str());
	frameNameTemplate=std::string(frameNameTemplate.begin()+lastSlash+1,frameNameTemplate.end());
	indexBegin-=lastSlash+1;
	indexEnd-=lastSlash+1;
	
	/* Determine the index range of the frame sequence: */
	firstIndex=Math::Constants<int>::max;
	lastIndex=Math::Constants<int>::min;
	frameDir->rewind();
	while(frameDir->readNextEntry())
		{
		/* Check if the current file is a frame file: */
		const char* cPtr=frameDir->getEntryName();
		if(strncmp(cPtr,frameNameTemplate.c_str(),indexBegin)==0)
			{
			cPtr+=indexBegin;
			int index=0;
			while(*cPtr>='0'&&*cPtr<='9')
				{
				index=index*10+int(*cPtr-'0');
				++cPtr;
				}
			if(strcmp(cPtr,frameNameTemplate.c_str()+indexEnd)==0)
				{
				/* Update the index range: */
				if(firstIndex>index)
					firstIndex=index;
				if(lastIndex<index+1)
					lastIndex=index+1;
				}
			}
		}
	if(firstIndex>=lastIndex)
		Misc::throwStdErr("Video::ImageSequenceVideoDevice: No frame images found");
	
	/* Load the first image and get its image size: */
	currentIndex=firstIndex;
	loadFrame(currentIndex);
	for(int i=0;i<2;++i)
		frameSize[i]=currentFrame.getSize(i);
	}

ImageSequenceVideoDevice::~ImageSequenceVideoDevice(void)
	{
	}

std::vector<VideoDataFormat> ImageSequenceVideoDevice::getVideoFormatList(void) const
	{
	/* Return the only video data format this source supports as a singleton list: */
	std::vector<VideoDataFormat> result;
	VideoDataFormat dataFormat=getVideoFormat();
	result.push_back(dataFormat);
	return result;
	}

VideoDataFormat ImageSequenceVideoDevice::getVideoFormat(void) const
	{
	/* Create the only video data format this source supports: */
	VideoDataFormat dataFormat;
	dataFormat.setPixelFormat("RGB8");
	for(int i=0;i<2;++i)
		dataFormat.size[i]=frameSize[i];
	dataFormat.lineSize=dataFormat.size[0]*3*sizeof(unsigned char);
	dataFormat.frameSize=dataFormat.size[1]*dataFormat.lineSize;
	dataFormat.frameIntervalCounter=1;
	dataFormat.frameIntervalDenominator=1;
	
	return dataFormat;
	}

VideoDataFormat& ImageSequenceVideoDevice::setVideoFormat(VideoDataFormat& newFormat)
	{
	/* Just ignore the given format: */
	newFormat=getVideoFormat();
	return newFormat;
	}

void ImageSequenceVideoDevice::configure(const Misc::ConfigurationFileSection& cfg)
	{
	}

ImageExtractor* ImageSequenceVideoDevice::createImageExtractor(void) const
	{
	return new ImageExtractorRGB8(frameSize);
	}

GLMotif::Widget* ImageSequenceVideoDevice::createControlPanel(GLMotif::WidgetManager* widgetManager)
	{
	/* Get the style sheet: */
	const GLMotif::StyleSheet* ss=widgetManager->getStyleSheet();
	
	/* Create the control panel dialog window: */
	GLMotif::PopupWindow* controlPanelPopup=new GLMotif::PopupWindow("ImageSequenceVideoDeviceControlPanelPopup",widgetManager,"Video Source Control");
	controlPanelPopup->setResizableFlags(true,false);
	
	GLMotif::RowColumn* controlPanel=new GLMotif::RowColumn("ControlPanel",controlPanelPopup,false);
	controlPanel->setNumMinorWidgets(2);
	
	new GLMotif::Label("FrameIndexLabel",controlPanel,"Frame Index");
	
	/* Create the frame index slider: */
	GLMotif::TextFieldSlider* frameIndexSlider=new GLMotif::TextFieldSlider("FrameIndexSlider",controlPanel,6,ss->fontHeight*20.0f);
	frameIndexSlider->setSliderMapping(GLMotif::TextFieldSlider::LINEAR);
	frameIndexSlider->setValueType(GLMotif::TextFieldSlider::UINT);
	frameIndexSlider->setValueRange(double(firstIndex),double(lastIndex-1),1.0);
	frameIndexSlider->setValue(double(currentIndex));
	frameIndexSlider->getValueChangedCallbacks().add(this,&ImageSequenceVideoDevice::frameIndexSliderCallback);
	
	controlPanel->manageChild();
	
	return controlPanelPopup;
	}

unsigned int ImageSequenceVideoDevice::allocateFrameBuffers(unsigned int requestedNumFrameBuffers)
	{
	/* Just return 1; there isn't really any streaming: */
	return 1;
	}

void ImageSequenceVideoDevice::startStreaming(void)
	{
	/* Call the base class method: */
	VideoDevice::startStreaming();
	}

void ImageSequenceVideoDevice::startStreaming(VideoDevice::StreamingCallback* newStreamingCallback)
	{
	/* Call the base class method: */
	VideoDevice::startStreaming(newStreamingCallback);
	}

FrameBuffer* ImageSequenceVideoDevice::dequeueFrame(void)
	{
	/* Return the current framebuffer: */
	return &currentBuffer;
	}

void ImageSequenceVideoDevice::enqueueFrame(FrameBuffer* frame)
	{
	/* Nothing to be done */
	}

void ImageSequenceVideoDevice::stopStreaming(void)
	{
	/* Call the base class method: */
	VideoDevice::stopStreaming();
	}

void ImageSequenceVideoDevice::releaseFrameBuffers(void)
	{
	/* Nothing to be done */
	}

void ImageSequenceVideoDevice::enumerateDevices(std::vector<VideoDevice::DeviceIdPtr>& devices)
	{
	/* Can't really do anything here... */
	
	/* BUT I HAFTA! */
	
	/* Create a device ID: */
	DeviceId* newDeviceId=new DeviceId("ImageSequence");
	
	/* Store the frame name template (THIS IS WHERE IT GETS UGLY, BUT I NEED THIS NOW): */
	newDeviceId->frameNameTemplate="/home/okreylos/VideoFrames/VideoFrame%04d.png";
	
	/* Store the device ID: */
	devices.push_back(newDeviceId);
	}

}
