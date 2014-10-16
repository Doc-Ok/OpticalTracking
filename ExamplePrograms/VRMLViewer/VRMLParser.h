/***********************************************************************
VRMLParser - Class to parse certain parts of the geometry definitions of
VRML 2.0 / VRML 97 files.
Copyright (c) 2006-2008 Oliver Kreylos

This file is part of the Virtual Reality VRML viewer (VRMLViewer).

The Virtual Reality VRML viewer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Virtual Reality VRML viewer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality VRML viewer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef VRMLPARSER_INCLUDED
#define VRMLPARSER_INCLUDED

#include <string.h>
#include <string>
#include <Misc/Autopointer.h>
#include <Misc/File.h>
#include <Misc/HashTable.h>

#include "NodeName.h"

/* Forward declarations: */
class VRMLNode;
typedef Misc::Autopointer<VRMLNode> VRMLNodePointer;

class VRMLParser
	{
	/* Embedded classes: */
	private:
	typedef Misc::HashTable<NodeName,VRMLNodePointer,NodeName> NodeDictionary; // Type to look up nodes by name
	
	/* Elements: */
	Misc::File vrmlFile; // The VRML input file
	std::string baseUrl; // Base URL of the original VRML file; used as starting point for local file names
	int nextChar; // The next character to be read from the input file
	size_t tokenBufferSize; // Allocated size of the token buffer
	char* token; // Pointer to the beginning of the token buffer
	size_t tokenLength; // Length of the token currently in the buffer
	NodeDictionary nodeDictionary; // Dictionary of named nodes
	
	/* Private methods: */
	void addTokenChar(char newChar); // Adds a single character to the current token
	
	/* Constructors and destructors: */
	public:
	VRMLParser(const char* vrmlFileName); // Creates a parser object for the given input file
	~VRMLParser(void);
	
	/* Methods: */
	bool eof(void) const // Returns true if the end of the input file has been reached
		{
		return tokenLength==0;
		};
	const char* getToken(void) const // Returns pointer to the current token
		{
		return token;
		};
	size_t getLength(void) const // Returns the length of the current token
		{
		return tokenLength;
		};
	bool isToken(const char* expectedToken) const // Checks if the current token is the expected one
		{
		return strcasecmp(token,expectedToken)==0;
		};
	void getNextToken(void); // Discards the current token and reads the next one
	VRMLNodePointer getNextNode(void); // Returns the next VRML node parsed from the input file
	std::string getFullUrl(const char* relativeUrl) const; // Returns a fully qualified URL based on an absolute or relative URL
	};

#endif
