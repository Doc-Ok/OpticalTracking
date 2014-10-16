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

#ifndef IO_CSVSOURCE_INCLUDED
#define IO_CSVSOURCE_INCLUDED

#include <stdexcept>
#include <IO/File.h>

namespace IO {

class CSVSource
	{
	/* Embedded classes: */
	public:
	class FormatError:public std::runtime_error // Class to signal format errors in the CSV source's structure; continuing to read after this error occurs has undefined results
		{
		/* Constructors and destructors: */
		public:
		FormatError(unsigned int fieldIndex,size_t recordIndex);
		};
	
	class ConversionError:public std::runtime_error // Class to signal conversion errors while reading fields
		{
		/* Constructors and destructors: */
		public:
		ConversionError(unsigned int fieldIndex,size_t recordIndex,const char* dataTypeName);
		};
	
	/* Elements: */
	private:
	FilePtr source; // Data source for CSV source
	int fieldSeparator; // Character used to separate fields in a record; comma by default
	int recordSeparator; // Character used to separate records; newline by default
	int quote; // Character used to quote field contents; double quote by default
	size_t recordIndex; // Zero-based index of the currently read record; increments before field read on the last field in a record returns
	unsigned int fieldIndex; // Zero-based index of the currently read field; increments before field read returns; resets to zero before field read on the last field in a record returns
	
	/* Private methods: */
	bool skipRestOfField(bool quoted,int nextChar); // Skips the rest of the current field starting with the given character; returns true if any characters were skipped; throws format error if the end of the field cannot be determined reliably
	template <class ValueParam>
	bool convertNumber(int& nextChar,ValueParam& value); // Converts characters in a field into a numeric value of the given type; returns false on conversion error
	
	/* Constructors and destructors: */
	public:
	CSVSource(FilePtr sSource); // Creates a default CSV source for the given character source
	~CSVSource(void); // Destroys the CSV source
	
	/* Methods: */
	int getFieldSeparator(void) const // Returns the field separator character
		{
		return fieldSeparator;
		}
	void setFieldSeparator(int newFieldSeparator); // Sets the field separator used in all future field reads
	int getRecordSeparator(void) const // Returns the record separator character
		{
		return recordSeparator;
		}
	void setRecordSeparator(int newRecordSeparator); // Sets the record separator used in all future field reads
	int getQuote(void) const // Returns the quote character
		{
		return quote;
		}
	void setQuote(int newQuote); // Sets the quote character to be used in all future field reads
	
	size_t getRecordIndex(void) const // Returns the current record index
		{
		return recordIndex;
		}
	unsigned int getFieldIndex(void) const // Returns the current field index
		{
		return fieldIndex;
		}
	bool eof(void) const // Returns true when the entire character source was read
		{
		return source->eof();
		}
	bool eor(void) const // Returns true when the last read field terminated a record; returns true before the first field is read
		{
		return fieldIndex==0;
		}
	
	/* Field reading methods: */
	bool skipField(void) // Skips the current field; returns true if the field was non-empty after unquoting; throws exception if the end of the field cannot be determined reliably
		{
		/* Read the first character: */
		int nextChar=source->getChar();
		if(nextChar==quote)
			{
			/* Skip the opening quote: */
			nextChar=source->getChar();
			
			/* Skip a quoted field: */
			return skipRestOfField(true,nextChar);
			}
		else
			{
			/* Skip an unquoted field: */
			return skipRestOfField(false,nextChar);
			}
		}
	void skipRecord(void) // Skips the rest of the current record
		{
		/* Simply skip fields until the field index resets to zero: */
		do
			{
			skipField();
			}
		while(fieldIndex!=0);
		}
	template <class ValueParam>
	ValueParam readField(void); // Reads the next field as the given data type; throws exception if the field contents cannot be fully converted, or the end of the field cannot be determined reliably
	};

/**********************************************
Specializations of CSVSource::readField method:
**********************************************/

}

#endif
