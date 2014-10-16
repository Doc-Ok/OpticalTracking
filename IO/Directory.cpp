/***********************************************************************
Directory - Base class to access directory-like objects in a generic
fashion.
Copyright (c) 2010-2014 Oliver Kreylos

This file is part of the I/O Support Library (IO).

The I/O Support Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

The I/O Support Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the I/O Support Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <IO/Directory.h>

#include <Misc/ThrowStdErr.h>

namespace IO {

/*************************************
Methods of class Directory::OpenError:
*************************************/

Directory::OpenError::OpenError(const char* directoryName)
	:std::runtime_error(Misc::printStdErrMsg("Error opening directory %s",directoryName))
	{
	}

/**************************
Methods of class Directory:
**************************/

std::string& Directory::normalizePath(std::string& path,unsigned int prefixLength)
	{
	/* Process path components until the path is over: */
	std::string::size_type length=path.length();
	std::string::size_type compBegin=prefixLength;
	while(compBegin!=length)
		{
		/* Find the end and length of the current path component: */
		std::string::size_type compEnd;
		for(compEnd=compBegin;compEnd!=length&&path[compEnd]!='/';++compEnd)
			;
		std::string::size_type compLen=compEnd-compBegin;
		
		/* Process the current component: */
		if(compLen==0||(compLen==1&&path[compBegin]=='.'))
			{
			/* Erase the redundant component: */
			if(compEnd!=length)
				{
				path.erase(compBegin,compLen+1);
				length-=compLen+1;
				}
			else
				{
				path.erase(compBegin,compLen);
				length-=compLen;
				}
			}
		else if(compLen==2&&path[compBegin]=='.'&&path[compBegin+1]=='.')
			{
			/* Check if the current path component is at the root: */
			if(compBegin==prefixLength)
				throw OpenError(path.c_str());
			
			/* Find the beginning of the previous path component: */
			std::string::size_type previousBegin;
			for(previousBegin=compBegin-1;previousBegin>prefixLength&&path[previousBegin-1]!='/';--previousBegin)
				;
			
			/* Erase the previous component and the current .. component: */
			compBegin=compEnd;
			if(compBegin!=length)
				++compBegin;
			path.erase(previousBegin,compBegin-previousBegin);
			length-=compBegin-previousBegin;
			compBegin=previousBegin;
			}
		else
			{
			/* Go to the next path component: */
			compBegin=compEnd;
			if(compBegin!=length)
				++compBegin;
			}
		}
	
	/* Remove a potential trailing slash: */
	if(length>prefixLength&&path[length-1]=='/')
		path.erase(length-1,1);
	
	return path;
	}

std::string::const_iterator Directory::getLastComponent(const std::string& path,unsigned int prefixLength)
	{
	/* Check for the special case of the root directory: */
	std::string::const_iterator prefixEnd=path.begin()+prefixLength;
	if(prefixEnd==path.end())
		return path.begin();
	else
		{
		/* Find the last slash in the path: */
		std::string::const_iterator compBegin;
		for(compBegin=path.end();compBegin!=prefixEnd&&compBegin[-1]!='/';--compBegin)
			;
		
		/* Return the path suffix after the last slash: */
		return compBegin;
		}
	}

Directory::~Directory(void)
	{
	}

std::string Directory::createNumberedFileName(const char* fileNameTemplate,int numDigits)
	{
	/* Check if the template contains any slashes: */
	const char* slashPtr=0;
	for(const char* tPtr=fileNameTemplate;*tPtr!='\0';++tPtr)
		if(*tPtr=='/')
			slashPtr=tPtr;
	
	/* Find the directory containing the template: */
	DirectoryPtr dir=this;
	if(slashPtr!=0)
		{
		dir=openDirectory(std::string(fileNameTemplate,slashPtr).c_str());
		fileNameTemplate=slashPtr+1;
		}
	
	/* Find the template's first extension: */
	const char* extPtr;
	for(extPtr=fileNameTemplate;*extPtr!='\0'&&*extPtr!='.';++extPtr)
		;
	size_t prefixLen=extPtr-fileNameTemplate;
	
	/* Find any matching files in the search directory: */
	unsigned int nextFreeNumber=1;
	dir->rewind();
	while(dir->readNextEntry())
		{
		/* Check if the file name matches the template prefix: */
		const char* enPtr=dir->getEntryName();
		if(strncmp(fileNameTemplate,enPtr,prefixLen)==0)
			{
			/* Check if the next numDigits characters are digits: */
			enPtr+=prefixLen;
			unsigned int entryValue=0;
			int numEntryDigits;
			for(numEntryDigits=0;numEntryDigits<numDigits&&*enPtr>='0'&&*enPtr<='9';++numEntryDigits,++enPtr)
				entryValue=entryValue*10+(*enPtr-'0');
			if(numEntryDigits==numDigits)
				{
				/* Check if the entry suffix matches the template suffix: */
				if(strcmp(enPtr,fileNameTemplate+prefixLen)==0)
					{
					/* Have a match; remember the entry value: */
					if(nextFreeNumber<=entryValue)
						nextFreeNumber=entryValue+1;
					}
				}
			}
		}
	
	/* Construct the result file name: */
	std::string result(fileNameTemplate,fileNameTemplate+prefixLen);
	for(int i=0;i<numDigits;++i)
		result.push_back('0');
	for(int i=1;i<=numDigits&&nextFreeNumber>0;++i)
		{
		result[prefixLen+numDigits-i]=nextFreeNumber%10+'0';
		nextFreeNumber/=10;
		}
	if(nextFreeNumber>0)
		Misc::throwStdErr("IO::Directory::createUniqueFileName: No more available names");
	result.append(fileNameTemplate+prefixLen);
	
	return result;
	}

}
