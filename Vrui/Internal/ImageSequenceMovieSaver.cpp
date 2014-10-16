/***********************************************************************
ImageSequenceMovieSaver - Helper class to save movies as sequences of
image files in formats supported by the Images library.
Copyright (c) 2010 Oliver Kreylos

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

#include <Vrui/Internal/ImageSequenceMovieSaver.h>

#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <Misc/ThrowStdErr.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Images/WriteImageFile.h>

namespace Vrui {

/*********************************
Methods of class TheoraMovieSaver:
*********************************/

void ImageSequenceMovieSaver::frameWritingThreadMethod(void)
	{
	/* Save frames until shut down: */
	unsigned int frameIndex=0;
	while(!done)
		{
		/* Add the most recent frame to the captured frame queue: */
		{
		Threads::MutexCond::Lock captureLock(captureCond);
		frames.lockNewValue();
		capturedFrames.push_back(frames.getLockedValue());
		captureCond.signal();
		}
		
		/* Wait for the next frame: */
		int numSkippedFrames=waitForNextFrame();
		if(numSkippedFrames>0)
			{
			std::cerr<<"MovieSaver: Skipped frames "<<frameIndex<<" to "<<frameIndex+numSkippedFrames-1<<std::endl;
			frameIndex+=numSkippedFrames;
			}
		}
	}

void* ImageSequenceMovieSaver::frameSavingThreadMethod(void)
	{
	unsigned int frameIndex=0;
	while(true)
		{
		/* Wait for the next frame: */
		FrameBuffer frame;
		{
		Threads::MutexCond::Lock captureLock(captureCond);
		while(!done&&capturedFrames.empty())
			captureCond.wait(captureLock);
		if(capturedFrames.empty()) // Bail out if there will be no more frames
			break;
		frame=capturedFrames.front();
		capturedFrames.pop_front();
		}
		
		/* Write the next frame image file: */
		char frameName[1024];
		snprintf(frameName,sizeof(frameName),frameNameTemplate.c_str(),frameIndex);
		++frameIndex;
		
		Images::writeImageFile(frame.getFrameSize()[0],frame.getFrameSize()[1],frame.getBuffer(),frameName);
		}
	
	return 0;
	}

ImageSequenceMovieSaver::ImageSequenceMovieSaver(const Misc::ConfigurationFileSection& configFileSection)
	:MovieSaver(configFileSection),
	 frameNameTemplate(configFileSection.retrieveString("./movieFrameNameTemplate")),
	 done(false)
	{
	/* Check if the frame name template has the correct format: */
	int numConversions=0;
	bool hasIntConversion=false;
	for(std::string::const_iterator fntIt=frameNameTemplate.begin();fntIt!=frameNameTemplate.end();++fntIt)
		{
		if(*fntIt=='%')
			{
			++fntIt;
			if(*fntIt!='%')
				{
				++numConversions;
				
				/* Skip width modifiers: */
				while(isdigit(*fntIt))
					++fntIt;
				
				/* Check for unsigned integer conversion: */
				if(*fntIt=='u')
					hasIntConversion=true;
				}
			}
		else if(*fntIt=='/') // Only accept conversions in the file name part
			hasIntConversion=false;
		}
	if(numConversions!=1||!hasIntConversion)
		Misc::throwStdErr("MovieSaver::MovieSaver: movie frame name template \"%s\" does not have exactly one %%u conversion",frameNameTemplate.c_str());
	
	/* Start the image writing thread: */
	frameSavingThread.start(this,&ImageSequenceMovieSaver::frameSavingThreadMethod);
	}

ImageSequenceMovieSaver::~ImageSequenceMovieSaver(void)
	{
	/* Signal the frame capturing and saving threads to shut down: */
	done=true;
	captureCond.signal();
	
	/* Wait until the frame saving thread has saved all frames and terminates: */
	frameSavingThread.join();
	}

}
