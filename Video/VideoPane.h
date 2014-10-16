/***********************************************************************
VideoPane - A GLMotif widget to display video streams in Y'CbCr 4:2:0
pixel format.
Copyright (c) 2010 Oliver Kreylos

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

#ifndef VIDEO_VIDEOPANE_INCLUDED
#define VIDEO_VIDEOPANE_INCLUDED

#include <GLMotif/Widget.h>
#include <Video/YpCbCr420Texture.h>

namespace GLMotif {

class VideoPane:public Widget
	{
	/* Elements: */
	private:
	Vector preferredSize; // The widget's preferred (and minimal) size
	Box frame; // The position and size of the video frame inside the widget's interior
	Video::YpCbCr420Texture texture; // The video texture
	
	/* Constructors and destructors: */
	public:
	VideoPane(const char* sName,Container* sParent,bool sManageChild =true);
	
	/* Methods inherited from Widget: */
	virtual Vector calcNaturalSize(void) const;
	virtual void resize(const Box& newExterior);
	virtual void draw(GLContextData& contextData) const;
	
	/* New methods: */
	void setPreferredSize(const Vector& newPreferredSize); // Sets a new preferred size; does not force video frame's aspect ratio
	const Video::YpCbCr420Texture& getTexture(void) const // Returns the video texture
		{
		return texture;
		}
	Video::YpCbCr420Texture& getTexture(void) // Ditto
		{
		return texture;
		}
	};

}

#endif
