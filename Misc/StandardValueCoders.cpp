/***********************************************************************
StandardValueCoders - Specialized ValueCoder classes for standard data
types.
Copyright (c) 2004-2012 Oliver Kreylos

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

#include <Misc/StandardValueCoders.h>

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <Misc/PrintInteger.h>

namespace Misc {

/****************
Helper functions:
****************/

namespace {

int parseSignedInteger(const char* start,const char* end,const char** decodeEnd)
	{
	int result=0;
	const char* cPtr=start;
	
	/* Check for optional sign: */
	bool negative=false;
	if(cPtr!=end&&*cPtr=='-')
		{
		negative=true;
		++cPtr;
		}
	else if(cPtr!=end&&*cPtr=='+')
		++cPtr;
	
	/* Scan until end of text or non-integer character is reached: */
	bool haveDigit=false;
	while(cPtr!=end&&*cPtr>='0'&&*cPtr<='9')
		{
		result*=10;
		result+=int(*cPtr-'0');
		haveDigit=true;
		++cPtr;
		}
	
	/* Check if any digits were parsed: */
	if(!haveDigit)
		throw 42;
	
	if(decodeEnd!=0)
		*decodeEnd=cPtr;
	if(negative)
		result=-result;
	return result;
	}

unsigned int parseUnsignedInteger(const char* start,const char* end,const char** decodeEnd)
	{
	unsigned int result=0;
	const char* cPtr=start;
	
	/* Check for optional plus sign: */
	if(cPtr!=end&&*cPtr=='+')
		++cPtr;
	
	/* Scan until end of text or non-integer character is reached: */
	bool haveDigit=false;
	while(cPtr!=end&&*cPtr>='0'&&*cPtr<='9')
		{
		result*=10;
		result+=(unsigned int)(*cPtr-'0');
		haveDigit=true;
		++cPtr;
		}
	
	/* Check if any digits were parsed: */
	if(!haveDigit)
		throw 42;
	
	if(decodeEnd!=0)
		*decodeEnd=cPtr;
	return result;
	}

double parseFloatingPoint(const char* start,const char* end,const char** decodeEnd)
	{
	double result=0.0;
	const char* cPtr=start;
	
	/* Check for optional sign: */
	bool negative=false;
	if(cPtr!=end&&*cPtr=='-')
		{
		negative=true;
		++cPtr;
		}
	else if(cPtr!=end&&*cPtr=='+')
		++cPtr;
	
	/* Scan integer portion of number: */
	bool haveDigit=false;
	while(cPtr!=end&&*cPtr>='0'&&*cPtr<='9')
		{
		result*=10.0;
		result+=double(*cPtr-'0');
		haveDigit=true;
		++cPtr;
		}
	
	/* Check for optional fractional part: */
	int exponent=0;
	if(cPtr!=end&&*cPtr=='.')
		{
		++cPtr;
		
		/* Scan fractional portion of number: */
		while(cPtr!=end&&*cPtr>='0'&&*cPtr<='9')
			{
			result*=10.0;
			result+=double(*cPtr-'0');
			--exponent;
			haveDigit=true;
			++cPtr;
			}
		}
	
	if(!haveDigit)
		throw 42;
		
	/* Check for optional exponent: */
	if(cPtr!=end&&(*cPtr=='e'||*cPtr=='E'))
		exponent+=parseSignedInteger(cPtr+1,end,&cPtr);
	
	if(decodeEnd!=0)
		*decodeEnd=cPtr;
	if(exponent>0)
		{
		double multiplier=1.0;
		for(int i=0;i<exponent;++i)
			multiplier*=10.0;
		result*=multiplier;
		}
	else if(exponent<0)
		{
		double divisor=1.0;
		for(int i=0;i>exponent;--i)
			divisor*=10.0;
		result/=divisor;
		}
	if(negative)
		result=-result;
	return result;
	}

}

/****************************************
Methods of class ValueCoder<std::string>:
****************************************/

std::string ValueCoder<std::string>::encode(const std::string& value)
	{
	/* Check whether string needs quotation: */
	bool needsQuotes=false;
	for(std::string::const_iterator vIt=value.begin();vIt!=value.end();++vIt)
		if(!(isalnum(*vIt)||*vIt=='_'))
			{
			needsQuotes=true;
			break;
			}
	
	if(needsQuotes)
		{
		/* Default quoting style is double-quote: */
		std::string result="\"";
		for(std::string::const_iterator vIt=value.begin();vIt!=value.end();++vIt)
			{
			/* Quote special characters the C way: */
			switch(*vIt)
				{
				case '\a':
					result.push_back('\\');
					result.push_back('a');
					break;
				
				case '\b':
					result.push_back('\\');
					result.push_back('b');
					break;
				
				case '\f':
					result.push_back('\\');
					result.push_back('f');
					break;
				
				case '\n':
					result.push_back('\\');
					result.push_back('n');
					break;
				
				case '\r':
					result.push_back('\\');
					result.push_back('r');
					break;
				
				case '\t':
					result.push_back('\\');
					result.push_back('t');
					break;
				
				case '\v':
					result.push_back('\\');
					result.push_back('v');
					break;
				
				case '\\':
					result.push_back('\\');
					result.push_back('\\');
					break;
				
				case '\"':
					result.push_back('\\');
					result.push_back('\"');
					break;
				
				default:
					result.push_back(*vIt);
				}
			}
		result.push_back('\"');
		
		return result;
		}
	else
		return value;
	}

std::string ValueCoder<std::string>::decode(const char* start,const char* end,const char** decodeEnd)
	{
	std::string result;
	
	/* Determine whether the string is quoted: */
	if(end!=start&&(*start=='\''||*start=='\"'))
		{
		/* Read characters until the next matching quote character, or end of the code string: */
		char quoteChar=*start;
		bool escape=false;
		const char* cPtr;
		for(cPtr=start+1;cPtr!=end;++cPtr)
			{
			if(escape)
				{
				/* Process escape sequences the C way: */
				switch(*cPtr)
					{
					case 'a':
						result.push_back('\a');
						break;
					
					case 'b':
						result.push_back('\b');
						break;
					
					case 'f':
						result.push_back('\f');
						break;
					
					case 'n':
						result.push_back('\n');
						break;
					
					case 'r':
						result.push_back('\r');
						break;
					
					case 't':
						result.push_back('\t');
						break;
					
					case 'v':
						result.push_back('\v');
						break;
					
					default:
						result.push_back(*cPtr);
					}
				escape=false;
				}
			else
				{
				if(*cPtr=='\\')
					escape=true;
				else if(*cPtr!=quoteChar)
					result.push_back(*cPtr);
				else
					break;
				}
			}
		
		/* Check for dangling escape character: */
		if(escape)
			throw DecodingError(std::string("Dangling escape character at end of string ")+std::string(start,end));
		
		/* Check for correct quote character at end of string: */
		if(*cPtr!=quoteChar)
			throw DecodingError(std::string("Unmatched quote at end of string ")+std::string(start,end));
		
		if(decodeEnd!=0)
			*decodeEnd=cPtr+1;
		}
	else
		{
		/* Read characters until the next whitespace, or end of the code string: */
		bool escape=false;
		const char* cPtr;
		for(cPtr=start;cPtr!=end;++cPtr)
			{
			if(escape)
				{
				/* Process escape sequences the C way: */
				switch(*cPtr)
					{
					case 'a':
						result.push_back('\a');
						break;
					
					case 'b':
						result.push_back('\b');
						break;
					
					case 'f':
						result.push_back('\f');
						break;
					
					case 'n':
						result.push_back('\n');
						break;
					
					case 'r':
						result.push_back('\r');
						break;
					
					case 't':
						result.push_back('\t');
						break;
					
					case 'v':
						result.push_back('\v');
						break;
					
					default:
						result.push_back(*cPtr);
					}
				escape=false;
				}
			else
				{
				if(*cPtr=='\\')
					escape=true;
				else if(isalnum(*cPtr)||*cPtr=='_')
					result.push_back(*cPtr);
				else
					break;
				}
			}
		
		/* Check for dangling escape character: */
		if(escape)
			throw DecodingError(std::string("Dangling escape character at end of string ")+std::string(start,end));
		
		if(decodeEnd!=0)
			*decodeEnd=cPtr;
		}
	
	return result;
	}

/*********************************
Methods of class ValueCoder<bool>:
*********************************/

std::string ValueCoder<bool>::encode(const bool& value)
	{
	if(value)
		return "true";
	else
		return "false";
	}

bool ValueCoder<bool>::decode(const char* start,const char* end,const char** decodeEnd)
	{
	if(end-start>=4&&strncasecmp(start,"true",4)==0)
		{
		if(decodeEnd!=0)
			*decodeEnd=start+4;
		return true;
		}
	else if(end-start>=5&&strncasecmp(start,"false",5)==0)
		{
		if(decodeEnd!=0)
			*decodeEnd=start+5;
		return false;
		}
	else
		throw DecodingError(std::string("Unable to convert \"")+std::string(start,end)+std::string("\" to bool"));
	}

/**************************************
Methods of class ValueCoder<short int>:
**************************************/

std::string ValueCoder<short int>::encode(const short int& value)
	{
	char buffer[7];
	return std::string(print(value,buffer+6));
	}

short int ValueCoder<short int>::decode(const char* start,const char* end,const char** decodeEnd)
	{
	try
		{
		return (short int)(parseSignedInteger(start,end,decodeEnd));
		}
	catch(...)
		{
		/* Re-throw the exception with a meaningful error string: */
		throw DecodingError(std::string("Unable to convert \"")+std::string(start,end)+std::string("\" to short int"));
		}
	}

/***********************************************
Methods of class ValueCoder<unsigned short int>:
***********************************************/

std::string ValueCoder<unsigned short int>::encode(const unsigned short int& value)
	{
	char buffer[6];
	return std::string(print(value,buffer+5));
	}

unsigned short int ValueCoder<unsigned short int>::decode(const char* start,const char* end,const char** decodeEnd)
	{
	try
		{
		return (unsigned short int)(parseUnsignedInteger(start,end,decodeEnd));
		}
	catch(...)
		{
		/* Re-throw the exception with a meaningful error string: */
		throw DecodingError(std::string("Unable to convert \"")+std::string(start,end)+std::string("\" to unsigned short int"));
		}
	}

/********************************
Methods of class ValueCoder<int>:
********************************/

std::string ValueCoder<int>::encode(const int& value)
	{
	char buffer[12];
	return std::string(print(value,buffer+11));
	}

int ValueCoder<int>::decode(const char* start,const char* end,const char** decodeEnd)
	{
	try
		{
		return parseSignedInteger(start,end,decodeEnd);
		}
	catch(...)
		{
		/* Re-throw the exception with a meaningful error string: */
		throw DecodingError(std::string("Unable to convert \"")+std::string(start,end)+std::string("\" to int"));
		}
	}

/*****************************************
Methods of class ValueCoder<unsigned int>:
*****************************************/

std::string ValueCoder<unsigned int>::encode(const unsigned int& value)
	{
	char buffer[11];
	return std::string(print(value,buffer+10));
	}

unsigned int ValueCoder<unsigned int>::decode(const char* start,const char* end,const char** decodeEnd)
	{
	try
		{
		return parseUnsignedInteger(start,end,decodeEnd);
		}
	catch(...)
		{
		/* Re-throw the exception with a meaningful error string: */
		throw DecodingError(std::string("Unable to convert \"")+std::string(start,end)+std::string("\" to unsigned int"));
		}
	}

/**********************************
Methods of class ValueCoder<float>:
**********************************/

std::string ValueCoder<float>::encode(const float& value)
	{
	char buffer[40];
	snprintf(buffer,sizeof(buffer),"%.9g",value);
	return std::string(buffer);
	}

float ValueCoder<float>::decode(const char* start,const char* end,const char** decodeEnd)
	{
	try
		{
		return float(parseFloatingPoint(start,end,decodeEnd));
		}
	catch(...)
		{
		/* Re-throw the exception with a meaningful error string: */
		throw DecodingError(std::string("Unable to convert \"")+std::string(start,end)+std::string("\" to float"));
		}
	}

/***********************************
Methods of class ValueCoder<double>:
***********************************/

std::string ValueCoder<double>::encode(const double& value)
	{
	char buffer[40];
	snprintf(buffer,sizeof(buffer),"%.18g",value);
	return std::string(buffer);
	}

double ValueCoder<double>::decode(const char* start,const char* end,const char** decodeEnd)
	{
	try
		{
		return parseFloatingPoint(start,end,decodeEnd);
		}
	catch(...)
		{
		/* Re-throw the exception with a meaningful error string: */
		throw DecodingError(std::string("Unable to convert \"")+std::string(start,end)+std::string("\" to double"));
		}
	}

}
