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

#ifndef VIDEO_THEORACOMMENT_INCLUDED
#define VIDEO_THEORACOMMENT_INCLUDED

#include <theora/codec.h>

namespace Video {

class TheoraComment:public th_comment
	{
	/* Constructors and destructors: */
	public:
	TheoraComment(void); // Creates a default Theora comment structure
	private:
	TheoraComment(const TheoraComment& source); // Prohibit copy constructor
	TheoraComment& operator=(const TheoraComment& source); // Prohibit assignment operator
	public:
	~TheoraComment(void); // Destroys the Theora comment structure
	
	/* Methods: */
	void setVendorString(const char* vendorString); // Sets the comment structure's vendor string
	void addComment(const char* comment); // Adds a comment string
	void addTagValue(const char* tag,const char* value); // Adds a tag/value pair
	int getNumTagValues(const char* tag) const; // Returns the number of values for the given tag
	const char* getTagValue(const char* tag,int index) const; // Returns the value of the given index for the given tag; result points to internal storage of comment structure
	};

}

#endif
