/***********************************************************************
OggPage - Wrapper class for ogg_page structure from Ogg API.
Copyright (c) 2010-2014 Oliver Kreylos

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

#ifndef VIDEO_OGGPAGE_INCLUDED
#define VIDEO_OGGPAGE_INCLUDED

#include <string.h>
#include <ogg/ogg.h>

namespace Video {

class OggPage:public ogg_page
	{
	/* Constructors and destructors: */
	public:
	OggPage(void)
		{
		/* Zero out the page: */
		memset(this,0,sizeof(ogg_page));
		}
	~OggPage(void)
		{
		}
	
	/* Methods: */
	int getPageVersion(void) const // Returns the API version number of the page
		{
		return ogg_page_version(const_cast<OggPage*>(this));
		}
	bool isContinued(void) const // Returns true if the page continues an Ogg packet from a previous page
		{
		return ogg_page_continued(const_cast<OggPage*>(this));
		}
	int getNumPackets(void) const // Returns the number of Ogg packets begun in the page
		{
		return ogg_page_packets(const_cast<OggPage*>(this));
		}
	bool isBos(void) const // Returns true if the page starts a new Ogg stream
		{
		return ogg_page_bos(const_cast<OggPage*>(this));
		}
	bool isEos(void) const // Returns true if the page completes an Ogg stream
		{
		return ogg_page_eos(const_cast<OggPage*>(this));
		}
	ogg_int64_t getGranulePos(void) const // Returns the exact granular position of the packet data at the end of the page
		{
		return ogg_page_granulepos(const_cast<OggPage*>(this));
		}
	int getSerialNumber(void) const // Returns the serial number of the logical stream to which this page belongs
		{
		return ogg_page_serialno(const_cast<OggPage*>(this));
		}
	long getPageNumber(void) const // Returns the sequential page number of the page
		{
		return ogg_page_pageno(const_cast<OggPage*>(this));
		}
	template <class SinkParam>
	void write(SinkParam& sink) const // Writes the page to the given data sink
		{
		/* Write the page header: */
		sink.template write<unsigned char>(header,header_len);
		
		/* Write the page body: */
		sink.template write<unsigned char>(body,body_len);
		}
	};

}

#endif
