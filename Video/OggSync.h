/***********************************************************************
OggSync - Wrapper class for ogg_sync_state structure from Ogg API.
Copyright (c) 2010-2011 Oliver Kreylos

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

#ifndef VIDEO_OGGSYNC_INCLUDED
#define VIDEO_OGGSYNC_INCLUDED

#include <ogg/ogg.h>

/* Forward declarations: */
namespace IO {
class File;
}
namespace Video {
class OggPage;
}

namespace Video {

class OggSync:public ogg_sync_state
	{
	/* Constructors and destructors: */
	public:
	OggSync(void)
		{
		/* Initialize the ogg_sync_state: */
		ogg_sync_init(this);
		}
	~OggSync(void)
		{
		/* Clear the ogg_sync_state: */
		ogg_sync_clear(this);
		}
	
	/* Methods: */
	OggPage readPage(IO::File& file,size_t bufferSize =4096); // Reads an entire page of data from the given file
	};

}

#endif
