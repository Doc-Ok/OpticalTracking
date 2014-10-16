/***********************************************************************
CreateNumberedFileName - Helper function to create file names with
monotonically increasing numbers based on a given base file name.
Zero-padded numbers with the given number of digits are inserted
immediately before the base file name's extension, and the number is
guaranteed to be higher than any other matchingly numbered file in the
same directory.
Copyright (c) 2008-2011 Oliver Kreylos

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

#include <Misc/CreateNumberedFileName.h>

#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <Misc/ThrowStdErr.h>

namespace Misc {

char* createNumberedFileName(const char* baseFileName,int numDigits,char* fileNameBuffer)
	{
	/* Split the file name template into directory name and base file name and find the file extension: */
	const char* slashPtr=0;
	const char* extPtr=0;
	const char* bfnPtr;
	for(bfnPtr=baseFileName;*bfnPtr!='\0';++bfnPtr)
		{
		if(*bfnPtr=='/')
			slashPtr=bfnPtr;
		else if(*bfnPtr=='.')
			extPtr=bfnPtr;
		}
	if(extPtr==0)
		extPtr=bfnPtr;
	
	/* Open the directory containing the base file name: */
	DIR* directory;
	const char* fileName;
	char* bufferPtr;
	if(slashPtr==0)
		{
		/* Initialize file name and buffer: */
		fileName=baseFileName;
		bufferPtr=fileNameBuffer;
		
		/* Search in the current directory: */
		directory=opendir(".");
		}
	else
		{
		/* Copy the directory prefix into the result buffer already: */
		fileName=slashPtr+1;
		size_t dirLen=fileName-baseFileName;
		memcpy(fileNameBuffer,baseFileName,dirLen);
		fileNameBuffer[dirLen]='\0';
		bufferPtr=fileNameBuffer+dirLen;
		
		/* Search in the given directory: */
		directory=opendir(fileNameBuffer);
		
		/* Prepare to append the file name: */
		*bufferPtr='/';
		++bufferPtr;
		}
	if(directory==0)
		Misc::throwStdErr("Misc::createNumberedFileName: error searching directory for %s",baseFileName);
	
	/* Search for all matching files in the directory: */
	size_t prefixLen=extPtr-fileName;
	int maxNumber=0;
	struct dirent* entry;
	while((entry=readdir(directory))!=0)
		{
		/* Check if the current file name matches the file name prefix: */
		if(memcmp(entry->d_name,fileName,prefixLen)==0)
			{
			/* Check if the current file name has a number: */
			int fileNumDigits=0;
			int fileNumber=0;
			for(const char* digitPtr=entry->d_name+prefixLen;isdigit(*digitPtr);++digitPtr,++fileNumDigits)
				fileNumber=fileNumber*10+int(*digitPtr-'0');
			if(fileNumDigits==numDigits)
				{
				/* Check if the current file name matches the extension: */
				if(strcmp(entry->d_name+prefixLen+numDigits,extPtr)==0)
					{
					/* File matches, compare its number to the maximum: */
					if(maxNumber<fileNumber)
						maxNumber=fileNumber;
					}
				}
			}
		}
	++maxNumber;
	
	/* Write the resulting file name into the buffer: */
	memcpy(bufferPtr,fileName,prefixLen);
	bufferPtr+=prefixLen+numDigits;
	for(int i=0;i<numDigits;++i)
		{
		--bufferPtr;
		*bufferPtr=char('0'+maxNumber%10);
		maxNumber/=10;
		}
	bufferPtr+=numDigits;
	strcpy(bufferPtr,extPtr);
	
	/* Clean up and return: */
	closedir(directory);
	return fileNameBuffer;
	}

std::string createNumberedFileName(const std::string& baseFileName,int numDigits)
	{
	/* Split the file name template into directory name and base file name and find the file extension: */
	std::string::const_iterator slashPtr=baseFileName.end();
	std::string::const_iterator extPtr=baseFileName.end();
	for(std::string::const_iterator bfnPtr=baseFileName.begin();bfnPtr!=baseFileName.end();++bfnPtr)
		{
		if(*bfnPtr=='/')
			slashPtr=bfnPtr;
		else if(*bfnPtr=='.')
			extPtr=bfnPtr;
		}
	
	/* Open the directory containing the base file name: */
	DIR* directory;
	std::string fileName;
	std::string result;
	if(slashPtr==baseFileName.end())
		{
		/* Initialize file name prefix and result string: */
		fileName=std::string(baseFileName.begin(),extPtr);
		result="";
		
		/* Search in the current directory: */
		directory=opendir(".");
		}
	else
		{
		/* Copy the directory prefix into the result string already: */
		fileName=std::string(slashPtr+1,extPtr);
		result=std::string(baseFileName.begin(),slashPtr);
		
		/* Search in the given directory: */
		directory=opendir(result.c_str());
		
		/* Prepare to append the file name: */
		result.push_back('/');
		}
	if(directory==0)
		Misc::throwStdErr("Misc::createNumberedFileName: error searching directory for %s",baseFileName.c_str());
	std::string fileNameExt=std::string(extPtr,baseFileName.end());
	
	/* Search for all matching files in the directory: */
	size_t prefixLen=fileName.size();
	int maxNumber=0;
	struct dirent* entry;
	while((entry=readdir(directory))!=0)
		{
		/* Check if the current file name matches the file name prefix: */
		if(memcmp(entry->d_name,fileName.c_str(),prefixLen)==0)
			{
			/* Check if the current file name has a number: */
			int fileNumDigits=0;
			int fileNumber=0;
			for(const char* digitPtr=entry->d_name+prefixLen;isdigit(*digitPtr);++digitPtr,++fileNumDigits)
				fileNumber=fileNumber*10+int(*digitPtr-'0');
			if(fileNumDigits==numDigits)
				{
				/* Check if the current file name matches the extension: */
				if(strcmp(entry->d_name+prefixLen+numDigits,fileNameExt.c_str())==0)
					{
					/* File matches, compare its number to the maximum: */
					if(maxNumber<fileNumber)
						maxNumber=fileNumber;
					}
				}
			}
		}
	++maxNumber;
	
	/* Assemble the result string: */
	result.append(fileName);
	char numberBuffer[11];
	char* nbPtr=numberBuffer+numDigits;
	*nbPtr='\0';
	for(int i=0;i<numDigits;++i)
		{
		--nbPtr;
		*nbPtr=char('0'+maxNumber%10);
		maxNumber/=10;
		}
	result.append(numberBuffer);
	result.append(fileNameExt);
	
	/* Clean up and return: */
	closedir(directory);
	return result;
	}

}
