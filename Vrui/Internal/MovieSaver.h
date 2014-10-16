/***********************************************************************
MovieSaver - Helper class to save movies, as sequences of frames or
already encoded into a video container format, from VR windows.
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

#ifndef VRUI_INTERNAL_MOVIESAVER_INCLUDED
#define VRUI_INTERNAL_MOVIESAVER_INCLUDED

#include <Misc/Time.h>
#include <Threads/Thread.h>
#include <Threads/TripleBuffer.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}
namespace Sound {
class SoundRecorder;
}

namespace Vrui {

class MovieSaver
	{
	/* Embedded classes: */
	public:
	class FrameBuffer // Class to hold a movie frame
		{
		/* Elements: */
		private:
		int frameSize[2]; // The frame's width and height
		unsigned char* buffer; // Pointer to the frame's image data
		
		/* Private methods: */
		void ref(void) // Adds a reference to a frame's image data
			{
			if(buffer!=0)
				++reinterpret_cast<unsigned int*>(buffer)[-1];
			}
		void unref(void) // Removes a reference from a frame's image data and deletes the image data if reference count reaches zero
			{
			if(buffer!=0&&--reinterpret_cast<unsigned int*>(buffer)[-1]==0)
				delete[] (reinterpret_cast<unsigned int*>(buffer)-1);
			}
		
		/* Constructors and destructors: */
		public:
		FrameBuffer(void); // Creates invalid frame buffer
		FrameBuffer(const FrameBuffer& source);
		FrameBuffer& operator=(const FrameBuffer& source);
		~FrameBuffer(void); // Destroys the frame buffer
		
		/* Methods: */
		void setFrameSize(int newWidth,int newHeight); // Changes the frame's size
		void prepareWrite(void); // Prepares for writing into the frame buffer by ensuring that the image data are not shared by another frame buffer
		const int* getFrameSize(void) const // Returns the frame's size
			{
			return frameSize;
			}
		const unsigned char* getBuffer(void) const // Returns the buffer for reading
			{
			return buffer;
			}
		unsigned char* getBuffer(void) // Returns the buffer for writing
			{
			return buffer;
			}
		};
	
	/* Elements: */
	protected:
	double frameRate; // Number of frames to write per second
	Misc::Time frameInterval; // Time between adjacent frames; == 1.0/frame rate
	Threads::TripleBuffer<FrameBuffer> frames; // Triple buffer of movie frames
	Threads::Thread frameWritingThread; // Thread to write movie frames at fixed intervals
	Sound::SoundRecorder* soundRecorder; // Pointer to a sound recorder if sound recording was started
	Misc::Time nextFrameTime; // Time point at which the next frame needs to be written
	bool firstFrame; // Flag to indicate the first saved frame
	
	/* Private methods: */
	void* frameWritingThreadWrapper(void);
	
	/* Protected methods: */
	protected:
	int waitForNextFrame(void); // Suspends the caller until the next frame is due to be written; skips frames if caller lags; returns number of skipped frames
	virtual void frameWritingThreadMethod(void) =0; // Runs in background and writes movie frames at fixed intervals
	
	/* Constructors and destructors: */
	public:
	MovieSaver(const Misc::ConfigurationFileSection& configFileSection); // Initializes movie saver by reading settings from configuration file section
	virtual ~MovieSaver(void); // Destroys movie saver
	
	/* Methods: */
	static MovieSaver* createMovieSaver(const Misc::ConfigurationFileSection& configFileSection); // Returns a new-allocated movie saver object configured by the given configuration file section
	FrameBuffer& startNewFrame(void) // Returns a frame buffer ready to receive a new movie frame
		{
		return frames.startNewValue();
		}
	void postNewFrame(void); // Signals that the new frame has been received
	};

}

#endif
