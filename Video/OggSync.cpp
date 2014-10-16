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

#include <Video/OggSync.h>

#include <Misc/ThrowStdErr.h>
#include <IO/File.h>
#include <Video/OggPage.h>

namespace Video {

/************************
Methods of class OggSync:
************************/

OggPage OggSync::readPage(IO::File& file,size_t bufferSize)
	{
	OggPage result;
	
	/* Read data into the ogg_sync_state until a page is complete: */
	while(ogg_sync_pageout(this,&result)!=1)
		{
		/* Get a data buffer from the Ogg synchronization state: */
		char* buffer=ogg_sync_buffer(this,bufferSize);
		if(buffer==0)
			Misc::throwStdErr("Video::OggSync::readPage: Error in ogg_sync_buffer");
		
		/* Read into the buffer: */
		size_t numBytes=file.readUpTo(buffer,bufferSize);
		if(numBytes==0)
			Misc::throwStdErr("Video::OggSync::readPage: End of file during page read");
		
		/* Pass the filled buffer to the synchronization state: */
		if(ogg_sync_wrote(this,numBytes)!=0)
			Misc::throwStdErr("Video::OggSync::readPage: Error in ogg_sync_wrote");
		}
	
	/* Return the Ogg page: */
	return result;
	}
}
