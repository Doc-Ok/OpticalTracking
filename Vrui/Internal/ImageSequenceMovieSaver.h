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

#ifndef VRUI_INTERNAL_IMAGESEQUENCEMOVIESAVER_INCLUDED
#define VRUI_INTERNAL_IMAGESEQUENCEMOVIESAVER_INCLUDED

#include <string>
#include <deque>
#include <Threads/MutexCond.h>
#include <Threads/Thread.h>
#include <Vrui/Internal/MovieSaver.h>

namespace Vrui {

class ImageSequenceMovieSaver:public MovieSaver
	{
	/* Elements: */
	private:
	std::string frameNameTemplate; // Template for creating image file names; must contain exactly one %d placeholder
	Threads::MutexCond captureCond; // Condition variable to signal that a new frame has been captured and added to the queue
	std::deque<FrameBuffer> capturedFrames; // Queue of frame buffers selected for writing
	Threads::Thread frameSavingThread; // Thread to write captured frames to disk; in separate thread to avoid latency issues
	volatile bool done; // Flag whether all frames have been captured
	
	/* Protected methods from MovieSaver: */
	protected:
	virtual void frameWritingThreadMethod(void);
	
	/* Private methods: */
	private:
	void* frameSavingThreadMethod(void); // Thread method to write captured frames to disk
	
	/* Constructors and destructors: */
	public:
	ImageSequenceMovieSaver(const Misc::ConfigurationFileSection& configFileSection);
	virtual ~ImageSequenceMovieSaver(void);
	};

}

#endif
