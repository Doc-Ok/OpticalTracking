/***********************************************************************
TheoraMovieSaver - Helper class to save movies as Theora video streams
packed into an Ogg container.
Copyright (c) 2010-2011 Oliver Kreylos

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

#ifndef VRUI_INTERNAL_THEORAMOVIESAVER_INCLUDED
#define VRUI_INTERNAL_THEORAMOVIESAVER_INCLUDED

#include <IO/File.h>
#include <Video/OggStream.h>
#include <Video/TheoraFrame.h>
#include <Video/TheoraEncoder.h>
#include <Vrui/Internal/MovieSaver.h>

/* Forward declarations: */
namespace Video {
class ImageExtractor;
}

namespace Vrui {

class TheoraMovieSaver:public MovieSaver
	{
	/* Elements: */
	private:
	IO::FilePtr movieFile; // The created movie file
	Video::OggStream oggStream; // The Ogg stream for the created movie file
	int theoraBitrate; // Target bitrate for Theora encoder in CBR mode
	int theoraQuality; // Target quality for Theora encoder in VBR mode
	int theoraGopSize; // Distance between keyframes in the Theora video stream
	int theoraFrameRate; // Integer frame rate
	Video::ImageExtractor* imageExtractor; // Extractor to convert RGB images to Y'CbCr 4:2:0 images
	Video::TheoraEncoder theoraEncoder; // Theora encoder object
	Video::TheoraFrame theoraFrame; // Frame buffer for frames in Y'CbCr 4:2:0 pixel format
	
	/* Protected methods from MovieSaver: */
	protected:
	virtual void frameWritingThreadMethod(void);
	
	/* Constructors and destructors: */
	public:
	TheoraMovieSaver(const Misc::ConfigurationFileSection& configFileSection);
	virtual ~TheoraMovieSaver(void);
	};

}

#endif
