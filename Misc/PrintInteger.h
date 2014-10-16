/***********************************************************************
PrintIntegers - Simple helper functions to print integer values of
several types. The functions expect a pointer after where the rightmost
digit is expected, and write right-aligned from there. The functions
NUL-terminate the written number, and return a pointer to the first
written character.
Copyright (c) 2012 Oliver Kreylos

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

#ifndef MISC_PRINTINTEGER_INCLUDED
#define MISC_PRINTINTEGER_INCLUDED

namespace Misc {

inline char* print(unsigned int value,char* bufferEnd)
	{
	/* NUL-terminate the number: */
	*bufferEnd='\0';
	
	/* Write digits right-to-left: */
	do
		{
		/* Write the rightmost digit and shift the number to the right: */
		*(--bufferEnd)=char(value%10U)+char('0');
		value/=10U;
		}
	while(value!=0U);
	
	/* Return pointer to the last character written: */
	return bufferEnd;
	}

inline char* print(int value,char* bufferEnd)
	{
	/* Convert the number to unsigned int: */
	unsigned int uValue=value;
	if(value<0)
		{
		/* Calculate twos' complement: */
		uValue=~uValue+1U;
		}
	
	/* NUL-terminate the number: */
	*bufferEnd='\0';
	
	/* Write digits right-to-left: */
	do
		{
		/* Write the rightmost digit and shift the number to the right: */
		*(--bufferEnd)=char(uValue%10U)+char('0');
		uValue/=10U;
		}
	while(uValue!=0U);
	
	/* Add a minus sign if the number was negative: */
	if(value<0)
		*(--bufferEnd)='-';
	
	/* Return pointer to the last character written: */
	return bufferEnd;
	}

inline char* print(unsigned long int value,char* bufferEnd)
	{
	/* NUL-terminate the number: */
	*bufferEnd='\0';
	
	/* Write digits right-to-left: */
	do
		{
		/* Write the rightmost digit and shift the number to the right: */
		*(--bufferEnd)=char(value%10UL)+char('0');
		value/=10UL;
		}
	while(value!=0UL);
	
	/* Return pointer to the last character written: */
	return bufferEnd;
	}

inline char* print(long int value,char* bufferEnd)
	{
	/* Convert the number to unsigned int: */
	unsigned long int uValue=value;
	if(value<0L)
		{
		/* Calculate twos' complement: */
		uValue=~uValue+1UL;
		}
	
	/* NUL-terminate the number: */
	*bufferEnd='\0';
	
	/* Write digits right-to-left: */
	do
		{
		/* Write the rightmost digit and shift the number to the right: */
		*(--bufferEnd)=char(uValue%10UL)+char('0');
		uValue/=10UL;
		}
	while(uValue!=0UL);
	
	/* Add a minus sign if the number was negative: */
	if(value<0)
		*(--bufferEnd)='-';
	
	/* Return pointer to the last character written: */
	return bufferEnd;
	}

}

#endif
