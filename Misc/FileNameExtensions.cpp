/***********************************************************************
FileNameExtensions - Helper functions to extract or test path names or
extensions from file names.
Copyright (c) 2009-2012 Oliver Kreylos

This file is part of the Miscellaneous Support Library (Misc).

The Miscellaneous Support Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Miscellaneous Support Library is distributed in the hope that it
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Miscellaneous Support Library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#include <string.h>

namespace Misc {

const char* getFileName(const char* pathName)
	{
	const char* result=pathName;
	for(const char* pnPtr=pathName;*pnPtr!='\0';++pnPtr)
		if(*pnPtr=='/')
			result=pnPtr+1;
	
	return result;
	}

const char* getExtension(const char* fileName)
	{
	/* Find the final period in the final component of the given path name: */
	const char* extPtr=0;
	const char* fnPtr;
	for(fnPtr=fileName;*fnPtr!='\0';++fnPtr)
		if(*fnPtr=='.') // Remember period position
			extPtr=fnPtr;
		else if(*fnPtr=='/') // Reset period position when a new component is started
			extPtr=0;
	
	/* If there was no extension, point to the end of the path name: */
	if(extPtr==0)
		extPtr=fnPtr;
	
	return extPtr;
	}

const char* getExtension(const char* fileNameBegin,const char* fileNameEnd)
	{
	/* Find the final period in the final component of the given path name: */
	const char* extPtr=fileNameEnd;
	for(const char* fnPtr=fileNameBegin;fnPtr!=fileNameEnd;++fnPtr)
		if(*fnPtr=='.') // Remember period position
			extPtr=fnPtr;
		else if(*fnPtr=='/') // Reset period position when a new component is started
			extPtr=fileNameEnd;
	
	return extPtr;
	}

bool hasExtension(const char* fileName,const char* extension)
	{
	/* Get the extension: */
	const char* extPtr=getExtension(fileName);
	
	/* Compare the extension against the given pattern: */
	return strcmp(extPtr,extension)==0;
	}

bool hasCaseExtension(const char* fileName,const char* extension)
	{
	/* Get the extension: */
	const char* extPtr=getExtension(fileName);
	
	/* Compare the extension against the given pattern: */
	return strcasecmp(extPtr,extension)==0;
	}

}
