/***********************************************************************
KinectPlayback - Class to play 3D video of a Vrui session previously
recorded with an InputDeviceDataSaver.
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

#include <Vrui/Internal/KinectPlayback.h>

#include <string>
#include <iostream>
#include <Misc/StandardValueCoders.h>
#include <Misc/CompoundValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <IO/File.h>
#include <IO/OpenFile.h>
#include <Math/Constants.h>
#include <Geometry/GeometryMarshallers.h>
#include <GL/gl.h>
#include <GL/GLTransformationWrappers.h>
#include <Kinect/DepthFrameReader.h>
#include <Kinect/ColorFrameReader.h>
#include <Kinect/KinectProjector.h>

namespace Vrui {

/***********************************************
Methods of class KinectPlayback::KinectStreamer:
***********************************************/

void* KinectPlayback::KinectStreamer::depthDecompressorThreadMethod(void)
	{
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	// Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	/* Create a depth frame reader for the depth file: */
	DepthFrameReader depthFrameReader(*depthFile);
	
	/* Read the first two depth frames: */
	FrameBuffer frames[2];
	for(int i=0;i<2;++i)
		frames[i]=depthFrameReader.readNextFrame();
	int mostRecentFrame=1;
	
	while(true)
		{
		/* Wait until the requested time stamp is no longer bracketed by the two current frames: */
		{
		Threads::MutexCond::Lock timeStampLock(timeStampCond);
		
		/* Check if the current frame pair brackets the requested time stamp: */
		if(readAheadTimeStamp<=frames[mostRecentFrame].timeStamp)
			{
			/* Signal arrival of the next current frame: */
			{
			Threads::MutexCond::Lock frameUpdateLock(frameUpdateCond);
			depthFrame=frames[1-mostRecentFrame];
			depthFrameValid=frames[mostRecentFrame].timeStamp;
			frameUpdateCond.signal();
			}
			
			/* Wait until the requested time stamp changes: */
			while(readAheadTimeStamp<=frames[mostRecentFrame].timeStamp)
				timeStampCond.wait(timeStampLock);
			}
		}
		
		/* Read the next frame: */
		frames[1-mostRecentFrame]=depthFrameReader.readNextFrame();
		mostRecentFrame=1-mostRecentFrame;
		if(frames[mostRecentFrame].timeStamp==Math::Constants<double>::max)
			break;
		}
	
	/* Signal arrival of the final frame: */
	{
	Threads::MutexCond::Lock frameUpdateLock(frameUpdateCond);
	depthFrame=frames[1-mostRecentFrame];
	depthFrameValid=frames[mostRecentFrame].timeStamp;
	frameUpdateCond.signal();
	}
	
	return 0;
	}

void* KinectPlayback::KinectStreamer::colorDecompressorThreadMethod(void)
	{
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	// Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	/* Create a color frame reader for the color file: */
	ColorFrameReader colorFrameReader(*colorFile);
	
	/* Read the first two color frames: */
	FrameBuffer frames[2];
	for(int i=0;i<2;++i)
		frames[i]=colorFrameReader.readNextFrame();
	int mostRecentFrame=1;
	
	while(true)
		{
		/* Wait until the requested time stamp is no longer bracketed by the two current frames: */
		{
		Threads::MutexCond::Lock timeStampLock(timeStampCond);
		
		/* Check if the current frame pair brackets the requested time stamp: */
		if(readAheadTimeStamp<=frames[mostRecentFrame].timeStamp)
			{
			/* Signal arrival of the next current frame: */
			{
			Threads::MutexCond::Lock frameUpdateLock(frameUpdateCond);
			colorFrame=frames[1-mostRecentFrame];
			colorFrameValid=frames[mostRecentFrame].timeStamp;
			frameUpdateCond.signal();
			}
			
			/* Wait until the requested time stamp changes: */
			while(readAheadTimeStamp<=frames[mostRecentFrame].timeStamp)
				timeStampCond.wait(timeStampLock);
			}
		}
		
		/* Read the next frame: */
		frames[1-mostRecentFrame]=colorFrameReader.readNextFrame();
		mostRecentFrame=1-mostRecentFrame;
		if(frames[mostRecentFrame].timeStamp==Math::Constants<double>::max)
			break;
		}
	
	/* Signal arrival of the final frame: */
	{
	Threads::MutexCond::Lock frameUpdateLock(frameUpdateCond);
	colorFrame=frames[1-mostRecentFrame];
	colorFrameValid=frames[mostRecentFrame].timeStamp;
	frameUpdateCond.signal();
	}
	
	return 0;
	}

KinectPlayback::KinectStreamer::KinectStreamer(double firstTimeStamp,std::string saveFileNamePrefix,Misc::ConfigurationFileSection& streamerSection)
	:readAheadTimeStamp(firstTimeStamp),
	 depthFile(0),colorFile(0),
	 projector(0),
	 depthFrameValid(0.0),currentDepthFrame(0.0),
	 colorFrameValid(0.0),currentColorFrame(0.0)
	{
	/* Read the recorded camera's serial number: */
	std::string serialNumber=streamerSection.retrieveString("./serialNumber");
	
	/* Open the depth file: */
	std::string depthFileName=saveFileNamePrefix;
	depthFileName.push_back('-');
	depthFileName.append(serialNumber);
	depthFileName.append(".depth");
	depthFile=IO::openFile(depthFileName.c_str());
	depthFile->setEndianness(IO::File::LittleEndian);
	
	/* Read the depth matrix and projector transformation: */
	double depthMatrix[4*4];
	depthFile->read<double>(depthMatrix,4*4);
	projectorTransform=Misc::Marshaller<OGTransform>::read(*depthFile);
	
	/* Start the depth decompression thread: */
	depthDecompressorThread.start(this,&KinectPlayback::KinectStreamer::depthDecompressorThreadMethod);
	
	/* Open the color file: */
	std::string colorFileName=saveFileNamePrefix;
	colorFileName.push_back('-');
	colorFileName.append(serialNumber);
	colorFileName.append(".color");
	colorFile=IO::openFile(colorFileName.c_str());
	colorFile->setEndianness(IO::File::LittleEndian);
	
	/* Read the color matrix: */
	double colorMatrix[4*4];
	colorFile->read<double>(colorMatrix,4*4);
	
	/* Start the color decompression thread: */
	colorDecompressorThread.start(this,&KinectPlayback::KinectStreamer::colorDecompressorThreadMethod);
	
	/* Create the facade projector: */
	projector=new KinectProjector(depthMatrix,colorMatrix);
	}

KinectPlayback::KinectStreamer::~KinectStreamer(void)
	{
	/* Delete the facade projector: */
	delete projector;
	
	/* Shut down the depth and color decompression threads: */
	depthDecompressorThread.cancel();
	colorDecompressorThread.cancel();
	depthDecompressorThread.join();
	colorDecompressorThread.join();
	}

void KinectPlayback::KinectStreamer::updateFrames(double currentTimeStamp,double nextTimeStamp)
	{
	bool canReadAhead;
	{
	/* Wait until the streamer's frames are ready: */
	Threads::MutexCond::Lock frameUpdateLock(frameUpdateCond);
	while(depthFrameValid<currentTimeStamp||colorFrameValid<currentTimeStamp)
		frameUpdateCond.wait(frameUpdateLock);
	canReadAhead=depthFrameValid<nextTimeStamp||colorFrameValid<nextTimeStamp;
	}
	
	/* Update the projector's current frames: */
	if(currentDepthFrame!=depthFrame.timeStamp)
		{
		projector->setDepthFrame(depthFrame);
		currentDepthFrame=depthFrame.timeStamp;
		}
	if(currentColorFrame!=colorFrame.timeStamp)
		{
		projector->setColorFrame(colorFrame);
		currentColorFrame=colorFrame.timeStamp;
		}
	
	if(canReadAhead)
		{
		/* Update the readahead time stamp and wake up any sleeping threads: */
		Threads::MutexCond::Lock timeStampLock(timeStampCond);
		readAheadTimeStamp=nextTimeStamp;
		timeStampCond.broadcast();
		}
	}

void KinectPlayback::KinectStreamer::glRenderAction(GLContextData& contextData) const
	{
	/* Go to the camera's facade's coordinate system: */
	glPushMatrix();
	glMultMatrix(projectorTransform);
	
	/* Draw the camera's facade: */
	projector->draw(contextData);
	
	/* Go back to the previous coordinate system: */
	glPopMatrix();
	}

/*******************************
Methods of class KinectPlayback:
*******************************/

KinectPlayback::KinectPlayback(double firstTimeStamp,Misc::ConfigurationFileSection& configFileSection)
	{
	/* Read the saved files' name prefix: */
	std::string saveFileNamePrefix=configFileSection.retrieveString("./saveFileNamePrefix");
	
	/* Read the list of Kinect cameras: */
	typedef std::vector<std::string> StringList;
	StringList cameraSections=configFileSection.retrieveValue<StringList>("./cameras");
	
	/* Create streamers for all cameras: */
	for(StringList::const_iterator csIt=cameraSections.begin();csIt!=cameraSections.end();++csIt)
		{
		/* Go to the camera's section: */
		Misc::ConfigurationFileSection cameraSection=configFileSection.getSection(csIt->c_str());
		
		streamers.push_back(new KinectStreamer(firstTimeStamp,saveFileNamePrefix,cameraSection));
		}
	}

KinectPlayback::~KinectPlayback(void)
	{
	/* Delete all streamers: */
	for(std::vector<KinectStreamer*>::iterator sIt=streamers.begin();sIt!=streamers.end();++sIt)
		delete *sIt;
	}

void KinectPlayback::frame(double currentTimeStamp,double nextTimeStamp)
	{
	/* Block until all streamers have frames valid for the current time stamp: */
	for(std::vector<KinectStreamer*>::iterator sIt=streamers.begin();sIt!=streamers.end();++sIt)
		(*sIt)->updateFrames(currentTimeStamp,nextTimeStamp);
	}

void KinectPlayback::glRenderAction(GLContextData& contextData) const
	{
	/* Render all streamers: */
	for(std::vector<KinectStreamer*>::const_iterator sIt=streamers.begin();sIt!=streamers.end();++sIt)
		(*sIt)->glRenderAction(contextData);
	}

}
