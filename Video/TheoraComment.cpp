/***********************************************************************
TheoraComment - Wrapper class for th_comment structure from Theora v1.1
API.
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

#include <Video/TheoraComment.h>

#include <stdlib.h>
#include <string.h>

namespace Video {

/******************************
Methods of class TheoraComment:
******************************/

TheoraComment::TheoraComment(void)
	{
	th_comment_init(this);
	}

TheoraComment::~TheoraComment(void)
	{
	th_comment_clear(this);
	}

void TheoraComment::setVendorString(const char* vendorString)
	{
	/* Free the existing vendor string: */
	if(vendor!=0)
		free(vendor);
	
	/* Copy the new vendor string: */
	size_t len=strlen(vendorString);
	vendor=static_cast<char*>(calloc(len+1,sizeof(char)));
	memcpy(vendor,vendorString,len+1);
	}

void TheoraComment::addComment(const char* comment)
	{
	/* API failure: need to convert to non-const string: */
	th_comment_add(this,const_cast<char*>(comment));
	}

void TheoraComment::addTagValue(const char* tag,const char* value)
	{
	/* API failure: need to convert to non-const string: */
	th_comment_add_tag(this,const_cast<char*>(tag),const_cast<char*>(value));
	}

int TheoraComment::getNumTagValues(const char* tag) const
	{
	/* API failure: need to convert to non-const this and string: */
	return th_comment_query_count(const_cast<TheoraComment*>(this),const_cast<char*>(tag));
	}

const char* TheoraComment::getTagValue(const char* tag,int index) const
	{
	/* API failure: need to convert to non-const this and string: */
	return th_comment_query(const_cast<TheoraComment*>(this),const_cast<char*>(tag),index);
	}

}
