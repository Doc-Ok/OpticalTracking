/***********************************************************************
NodeName - Simple string abstraction class to find VRML nodes by name.
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

#ifndef NODENAME_INCLUDED
#define NODENAME_INCLUDED

#include <string.h>

class NodeName
	{
	/* Elements: */
	private:
	char* string; // Pointer to null-terminated character array
	
	/* Constructors and destructors: */
	public:
	NodeName(char* sString,int) // Low-level constructor; string will adopt character array
		:string(sString)
		{
		};
	NodeName(void) // Creates empty string
		:string(new char[1])
		{
		string[0]='\0';
		};
	NodeName(const char* sString) // Creates string from standard C string
		:string(new char[strlen(sString)+1])
		{
		strcpy(string,sString);
		};
	NodeName(const char* sString,int numCharacters) // Creates string from (up to) first numCharacters characters of standard C string
		:string(new char[numCharacters+1])
		{
		strncpy(string,sString,numCharacters);
		string[numCharacters]='\0';
		};
	NodeName(const NodeName& source) // Copy constructor
		:string(new char[strlen(source.string)+1])
		{
		strcpy(string,source.string);
		};
	NodeName& operator=(const char* sourceString) // Assignment operator with standard C string
		{
		if(sourceString!=string)
			{
			delete[] string;
			string=new char[strlen(sourceString)+1];
			strcpy(string,sourceString);
			}
		return *this;
		};
	NodeName& operator=(const NodeName& source) // Assignment operator
		{
		if(this!=&source) // Check for self-assignment
			{
			delete[] string;
			string=new char[strlen(source.string)+1];
			strcpy(string,source.string);
			}
		return *this;
		};
	~NodeName(void) throw()
		{
		delete[] string;
		};
	
	/* Methods: */
	friend bool operator==(const NodeName& s1,const NodeName& s2) // Equality operator
		{
		return strcmp(s1.string,s2.string)==0;
		};
	friend bool operator!=(const NodeName& s1,const NodeName& s2) // Inequality operator
		{
		return strcmp(s1.string,s2.string)!=0;
		};
	friend bool operator<(const NodeName& s1,const NodeName& s2) // Lexicographic < operator
		{
		return strcmp(s1.string,s2.string)<0;
		};
	friend bool operator<=(const NodeName& s1,const NodeName& s2) // Lexicographic <= operator
		{
		return strcmp(s1.string,s2.string)<=0;
		};
	friend bool operator>(const NodeName& s1,const NodeName& s2) // Lexicographic > operator
		{
		return strcmp(s1.string,s2.string)>0;
		};
	friend bool operator>=(const NodeName& s1,const NodeName& s2) // Lexicographic >= operator
		{
		return strcmp(s1.string,s2.string)>=0;
		};
	size_t length(void) const // Returns string's length (not counting the terminating NUL character)
		{
		return strlen(string);
		};
	const char* toStr(void) const // Converts string to constant standard C string
		{
		return string;
		};
	NodeName& operator+=(const NodeName& other) // Concatenation operator
		{
		char* newString=new char[strlen(string)+strlen(other.string)+1];
		strcpy(newString,string);
		strcat(newString,other.string);
		delete[] string;
		string=newString;
		return *this;
		};
	friend NodeName operator+(const NodeName& s1,const NodeName& s2) // Concatenation operator
		{
		char* newString=new char[strlen(s1.string)+strlen(s2.string)+1];
		strcpy(newString,s1.string);
		strcat(newString,s2.string);
		return NodeName(newString,0); // Calls private constructor; newString now owned by returned object
		};
	static size_t rawHash(const char* string) // Returns non-wrapped raw hash number for a standard C string
		{
		int result=0;
		int byteCounter=0;
		size_t currentMask=0x0;
		for(const char* cPtr=string;*cPtr!='\0';++cPtr)
			{
			/* Shift current character into current mask: */
			currentMask<<=8;
			currentMask|=*cPtr;
			if(++byteCounter==sizeof(size_t)) // Mask has been filled by characters
				{
				result+=currentMask;
				currentMask=0x0;
				byteCounter=0;
				}
			}
		currentMask<<=(sizeof(size_t)-byteCounter)*8;
		result+=currentMask;
		return result;
		}
	size_t rawHash(void) const // Returns non-wrapped raw hash number for a string
		{
		return rawHash(string);
		};
	static size_t hash(const char* string,size_t tableSize) // Returns a hash value for a standard C string
		{
		return rawHash(string)%tableSize;
		};
	static size_t hash(const NodeName& string,size_t tableSize) // Ditto for a string object
		{
		return rawHash(string.string)%tableSize;
		};
	};

#endif
