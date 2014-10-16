/***********************************************************************
CSVSource - Class to read tabular data from input streams in generalized
comma-separated value (CSV) format.
Copyright (c) 2010-2011 Oliver Kreylos

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

#include <IO/CSVSource.h>

#include <ctype.h>
#include <math.h>
#include <string>
#include <Misc/ThrowStdErr.h>

namespace IO {

namespace {

/*********************************************
Helper class to give readable names for types:
*********************************************/

template <class TypeParam>
class TypeName
	{
	/* Methods: */
	public:
	static const char* getName(void)
		{
		return "unkown";
		}
	};

template <>
class TypeName<unsigned int>
	{
	/* Methods: */
	public:
	static const char* getName(void)
		{
		return "unsigned int";
		}
	};

template <>
class TypeName<int>
	{
	/* Methods: */
	public:
	static const char* getName(void)
		{
		return "int";
		}
	};

template <>
class TypeName<float>
	{
	/* Methods: */
	public:
	static const char* getName(void)
		{
		return "float";
		}
	};

template <>
class TypeName<double>
	{
	/* Methods: */
	public:
	static const char* getName(void)
		{
		return "double";
		}
	};

template <>
class TypeName<std::string>
	{
	/* Methods: */
	public:
	static const char* getName(void)
		{
		return "std::string";
		}
	};

}

/***************************************
Methods of class CSVSource::FormatError:
***************************************/

CSVSource::FormatError::FormatError(unsigned int fieldIndex,size_t recordIndex)
	:std::runtime_error(Misc::printStdErrMsg("IO::CSVSource::read: Format error in field %u of record %u",fieldIndex,(unsigned int)recordIndex))
	{
	}

/*******************************************
Methods of class CSVSource::ConversionError:
*******************************************/

CSVSource::ConversionError::ConversionError(unsigned int fieldIndex,size_t recordIndex,const char* dataTypeName)
	:std::runtime_error(Misc::printStdErrMsg("IO::CSVSource::read: Could not convert field %u of record %u to type %s",fieldIndex,(unsigned int)recordIndex,dataTypeName))
	{
	}

/**************************
Methods of class CSVSource:
**************************/

bool CSVSource::skipRestOfField(bool quoted,int nextChar)
	{
	/* Keep track if any characters were actually skipped: */
	bool skippedAny=false;
	
	if(quoted)
		{
		/*******************
		Skip a quoted field:
		*******************/
		
		/* Skip characters until unquote: */
		while(true)
			{
			/* Skip characters until the next quote character or eof: */
			while(nextChar!=quote&&nextChar>=0)
				{
				skippedAny=true;
				nextChar=source->getChar();
				}
			
			/* Eof inside quote is a format error: */
			if(nextChar<0)
				throw FormatError(fieldIndex,recordIndex);
			
			/* Check for quoted quotes: */
			nextChar=source->getChar();
			if(nextChar==quote)
				{
				skippedAny=true;
				nextChar=source->getChar();
				}
			else
				break;
			}
		}
	else
		{
		/**********************
		Skip an unquoted field:
		**********************/
		
		/* Skip characters until the next field separator, record separator, eof, or quote: */
		while(nextChar!=fieldSeparator&&nextChar!=recordSeparator&&nextChar>=0&&nextChar!=quote)
			{
			skippedAny=true;
			nextChar=source->getChar();
			}
		}
	
	/* Check the next character: */
	if(nextChar==fieldSeparator)
		{
		/* Start a new field: */
		++fieldIndex;
		}
	else if(nextChar==recordSeparator||nextChar<0)
		{
		/* Record separator or eof start a new record: */
		fieldIndex=0;
		++recordIndex;
		}
	else
		{
		/* Signal a format error in the CSV source: */
		throw FormatError(fieldIndex,recordIndex);
		}
	
	return skippedAny;
	}

template <>
bool CSVSource::convertNumber(int& nextChar,unsigned int& value)
	{
	/* Signal a conversion error if the next character is not a digit: */
	if(nextChar<'0'||nextChar>'9')
		return false;
	
	/* Read the first digit: */
	value=(unsigned int)(nextChar-'0');
	nextChar=source->getChar();
	
	/* Read all following digits: */
	while(nextChar>='0'&&nextChar<='9')
		{
		value=value*10+(unsigned int)(nextChar-'0');
		nextChar=source->getChar();
		}
	
	return true;
	}

template <>
bool CSVSource::convertNumber(int& nextChar,int& value)
	{
	/* Check for optional sign: */
	bool negated=false;
	if(nextChar=='-')
		{
		negated=true;
		nextChar=source->getChar();
		}
	else if(nextChar=='+')
		nextChar=source->getChar();
	
	/* Signal a conversion error if the next character is not a digit: */
	if(nextChar<'0'||nextChar>'9')
		return false;
	
	/* Read the first digit: */
	unsigned int tempValue=(unsigned int)(nextChar-'0');
	nextChar=source->getChar();
	
	/* Read all following digits: */
	while(nextChar>='0'&&nextChar<='9')
		{
		tempValue=tempValue*10+(unsigned int)(nextChar-'0');
		nextChar=source->getChar();
		}
	
	/* Calculate the final value: */
	if(negated)
		value=-int(tempValue);
	else
		value=int(tempValue);
	
	return true;
	}

template <>
bool CSVSource::convertNumber(int& nextChar,double& value)
	{
	/* Check for optional sign: */
	bool negated=false;
	if(nextChar=='-')
		{
		negated=true;
		nextChar=source->getChar();
		}
	else if(nextChar=='+')
		nextChar=source->getChar();
	
	/* Keep track if any digits have been read: */
	bool haveDigit=false;
	
	/* Read an integral number part: */
	value=0.0;
	while(nextChar>='0'&&nextChar<='9')
		{
		haveDigit=true;
		value=value*10.0+double(nextChar-'0');
		nextChar=source->getChar();
		}
	
	/* Check for a period: */
	if(nextChar=='.')
		{
		nextChar=source->getChar();
		
		/* Read a fractional number part: */
		double fraction=0.0;
		double fractionBase=1.0;
		while(nextChar>='0'&&nextChar<='9')
			{
			haveDigit=true;
			fraction=fraction*10.0+double(nextChar-'0');
			fractionBase*=10.0;
			nextChar=source->getChar();
			}
		
		value+=fraction/fractionBase;
		}
	
	/* Signal a conversion error if no digits were read in the integral or fractional part: */
	if(!haveDigit)
		return false;
	
	/* Negate the result if a minus sign was read: */
	if(negated)
		value=-value;
	
	/* Check for an exponent indicator: */
	if(nextChar=='e'||nextChar=='E')
		{
		nextChar=source->getChar();
		
		/* Read a plus or minus sign: */
		bool exponentNegated=false;
		if(nextChar=='-')
			{
			exponentNegated=true;
			nextChar=source->getChar();
			}
		else if(nextChar=='+')
			nextChar=source->getChar();
		
		/* Signal a conversion error if the next character is not a digit: */
		if(nextChar<'0'||nextChar>'9')
			return false;
		
		/* Read the first exponent digit: */
		double exponent=double(nextChar-'0');
		nextChar=source->getChar();
		
		/* Read the rest of the exponent digits: */
		while(nextChar>='0'&&nextChar<='9')
			{
			exponent=exponent*10.0+double(nextChar-'0');
			nextChar=source->getChar();
			}
		
		/* Multiply the mantissa with the exponent: */
		if(exponentNegated)
			exponent=-exponent;
		value*=pow(10.0,exponent);
		}
	
	return true;
	}

template <>
bool CSVSource::convertNumber(int& nextChar,float& value)
	{
	/* Use the double conversion method internally: */
	double tempValue;
	if(!convertNumber<double>(nextChar,tempValue))
		return false;
	value=float(tempValue);
	return true;
	}

CSVSource::CSVSource(FilePtr sSource)
	:source(sSource),
	 fieldSeparator(','),recordSeparator('\n'),quote('\"'),
	 recordIndex(0),fieldIndex(0)
	{
	}

CSVSource::~CSVSource(void)
	{
	}

void CSVSource::setFieldSeparator(int newFieldSeparator)
	{
	fieldSeparator=newFieldSeparator;
	}

void CSVSource::setRecordSeparator(int newRecordSeparator)
	{
	recordSeparator=newRecordSeparator;
	}

void CSVSource::setQuote(int newQuote)
	{
	quote=newQuote;
	}

template <class ValueParam>
ValueParam CSVSource::readField(void)
	{
	/* Read the field's first character: */
	int nextChar=source->getChar();
	
	/* Check for quote: */
	bool quoted=false;
	if(nextChar==quote)
		{
		quoted=true;
		nextChar=source->getChar();
		}
	
	/* Skip whitespace: */
	while(isspace(nextChar)&&nextChar!=fieldSeparator&&nextChar!=recordSeparator&&nextChar>=0)
		nextChar=source->getChar();
	
	/* Read the numeric value: */
	ValueParam result(0);
	bool success=convertNumber(nextChar,result);
	
	/* Skip whitespace: */
	while(isspace(nextChar)&&nextChar!=fieldSeparator&&nextChar!=recordSeparator&&nextChar>=0)
		nextChar=source->getChar();
	
	/* Read until the end of the field, and invalidate the result if any further characters are encountered: */
	if(skipRestOfField(quoted,nextChar))
			success=false;
	
	/* Check for conversion errors: */
	if(!success)
		throw ConversionError(fieldIndex,recordIndex,TypeName<ValueParam>::getName());
	
	/* Return the result: */
	return result;
	}

template <>
std::string CSVSource::readField(void)
	{
	/* Read the field's first character: */
	int nextChar=source->getChar();
	
	std::string result;
	if(nextChar==quote)
		{
		/* Skip the opening quote: */
		nextChar=source->getChar();
		
		/********************
		Read a quoted string:
		********************/
		
		/* Read characters until unquote: */
		while(true)
			{
			/* Skip characters until the next quote character or eof: */
			while(nextChar!=quote&&nextChar>=0)
				{
				result.push_back(nextChar);
				nextChar=source->getChar();
				}
			
			/* Eof inside quote is a format error: */
			if(nextChar<0)
				throw FormatError(fieldIndex,recordIndex);
			
			/* Check for quoted quotes: */
			nextChar=source->getChar();
			if(nextChar==quote)
				{
				result.push_back(nextChar);
				nextChar=source->getChar();
				}
			else
				break;
			}
		}
	else
		{
		/***********************
		Read an unquoted string:
		***********************/
		
		/* Skip characters until the next field separator, record separator, eof, or quote: */
		while(nextChar!=fieldSeparator&&nextChar!=recordSeparator&&nextChar>=0&&nextChar!=quote)
			{
			result.push_back(nextChar);
			nextChar=source->getChar();
			}
		}
	
	/* Check the next character: */
	if(nextChar==fieldSeparator)
		{
		/* Start a new field: */
		++fieldIndex;
		}
	else if(nextChar==recordSeparator||nextChar<0)
		{
		/* Record separator or eof start a new record: */
		fieldIndex=0;
		++recordIndex;
		}
	else
		{
		/* Signal a format error in the CSV source: */
		throw FormatError(fieldIndex,recordIndex);
		}
	
	return result;
	}

/************************************************************************
Force instantiation of standard versions of CSVSource::readField methods:
************************************************************************/

template unsigned int CSVSource::readField<unsigned int>(void);
template int CSVSource::readField<int>(void);
template float CSVSource::readField<float>(void);
template double CSVSource::readField<double>(void);
template std::string CSVSource::readField<std::string>(void);

}
