/***********************************************************************
XBaseTable - Class to read records and extract fields from database
tables in XBase (dBASE III, FoxPro, ...) format based on a File
abstraction.
Copyright (c) 2009-2011 Oliver Kreylos

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

#include <IO/XBaseTable.h>

#include <ctype.h>
#include <Misc/ThrowStdErr.h>

namespace IO {

/***************************
Methods of class XBaseTable:
***************************/

XBaseTable::XBaseTable(const char* fileName,SeekableFilePtr sFile)
	:file(sFile)
	{
	/* Read the table header: */
	file->setEndianness(Misc::LittleEndian);
	version=file->read<unsigned char>();
	file->skip<char>(3); // Date of last update
	numRecords=file->read<unsigned int>();
	headerSize=file->read<unsigned short>();
	recordSize=file->read<unsigned short>();
	file->skip<char>(2+1); // Reserved; incomplete transaction flag
	if(file->read<char>()!=0x0)
		Misc::throwStdErr("IO::XBaseTable::XBaseTable: Table file %s is encrypted",fileName);
	file->skip<char>(4+8+1+1+2); // Free record thread; reserved; MDX flag; language driver; reserved
	
	/* Read all field descriptors: */
	ptrdiff_t fieldRecordOffset=1; // Deleted flag comes first
	while(true)
		{
		Field field;
		
		/* Read the field name's first character to check for the end-of-fields marker: */
		file->read(field.name,1);
		if(field.name[0]==0x0d)
			break;
		
		/* Read the rest of the field's name: */
		file->read(field.name+1,10);
		
		/* Read the rest of the field descriptor: */
		field.dataType=file->read<char>();
		file->skip<char>(4); // Field data address
		field.recordOffset=fieldRecordOffset;
		field.size=file->read<unsigned char>();
		field.numDecimals=file->read<unsigned char>();
		file->skip<char>(2+1+2+1+7+1); // Reserved; work area ID; reserved; SET FIELDS flag; reserved; index field flag
		
		/* Check the field for consistency: */
		if(field.dataType=='N'&&field.numDecimals>field.size)
			Misc::throwStdErr("IO::XBaseTable::XBaseTable: Field \"%s\" in table file %s has size %u, but %u decimals",field.name,fileName,(unsigned int)field.size,(unsigned int)field.numDecimals);
		if(field.dataType!='N'&&field.numDecimals>0)
			Misc::throwStdErr("IO::XBaseTable::XBaseTable: Field \"%s\" in table file %s is not of type NUMBER, but has %u decimals",field.name,fileName,(unsigned int)field.numDecimals);
		if(field.dataType=='L'&&field.size!=1)
			Misc::throwStdErr("IO::XBaseTable::XBaseTable: Field \"%s\" in table file %s is of type BOOLEAN, but has wrong size %u",field.name,fileName,(unsigned int)field.size);
		if(field.dataType=='D'&&field.size!=8)
			Misc::throwStdErr("IO::XBaseTable::XBaseTable: Field \"%s\" in table file %s is of type DATE, but has wrong size %u",field.name,fileName,(unsigned int)field.size);
		
		/* Store the field: */
		fields.push_back(field);
		fieldRecordOffset+=field.size;
		}
	
	/* Check table header for sanity: */
	if(fields.empty())
		Misc::throwStdErr("IO::XBaseTable::XBaseTable: Table file %s does not define fields",fileName);
	if(recordSize!=size_t(fieldRecordOffset))
		Misc::throwStdErr("IO::XBaseTable::XBaseTable: Table file %s reports record size %u, but has record size %u",fileName,(unsigned int)recordSize,(unsigned int)fieldRecordOffset);
	Offset currentPos=file->getReadPos();
	if(currentPos>headerSize)
		Misc::throwStdErr("IO::XBaseTable::XBaseTable: Table file %s has %u bytes of header data after field definitions",fileName,(unsigned int)(currentPos-headerSize));
	if(currentPos<headerSize)
		Misc::throwStdErr("IO::XBaseTable::XBaseTable: Table file %s has %u bytes of missing header data",fileName,(unsigned int)(headerSize-currentPos));
	}

XBaseTable::~XBaseTable(void)
	{
	}

XBaseTable::FieldDataType XBaseTable::getFieldDataType(size_t index) const
	{
	switch(fields[index].dataType)
		{
		case 'L':
			return BOOLEAN;
		
		case 'C':
			return STRING;
		
		case 'N':
			return NUMBER;
		
		case 'D':
			return DATE;
		
		default:
			return UNKNOWN;
		}
	}

XBaseTable::Record XBaseTable::readRecord(size_t recordIndex)
	{
	/* Create storage for the record: */
	unsigned int* storage=new unsigned int[(recordSize+sizeof(unsigned int)-1)/sizeof(unsigned int)+1];
	
	/* Read the record from the file: */
	file->setReadPosAbs(headerSize+Offset(recordIndex)*Offset(recordSize));
	file->read(reinterpret_cast<char*>(storage+1),recordSize);
	
	/* Return the record: */
	return Record(storage);
	}

XBaseTable::Record XBaseTable::makeRecord(void) const
	{
	/* Create storage for the record: */
	unsigned int* storage=new unsigned int[(recordSize+sizeof(unsigned int)-1)/sizeof(unsigned int)+1];
	
	/* Return the record: */
	return Record(storage);
	}

void XBaseTable::readRecord(size_t recordIndex,XBaseTable::Record& record)
	{
	/* Read the record from the file: */
	file->setReadPosAbs(headerSize+Offset(recordIndex)*Offset(recordSize));
	file->read(reinterpret_cast<char*>(record.storage+1),recordSize);
	}

XBaseTable::Maybe<bool> XBaseTable::getFieldBoolean(const XBaseTable::Record& record,size_t fieldIndex) const
	{
	Maybe<bool> result;
	
	const Field& f=fields[fieldIndex];
	const char* s=record.getStorage()+f.recordOffset;
	switch(f.dataType)
		{
		case 'L':
			if(s[0]=='Y'||s[0]=='y'||s[0]=='T'||s[0]=='t')
				result=true;
			else if(s[0]=='N'||s[0]=='n'||s[0]=='F'||s[0]=='f')
				result=false;
			else if(s[0]!='?'&&s[0]!=' ')
				Misc::throwStdErr("IO::XBaseTable::getFieldBoolean: Unrecognized value \"%c\" in boolean field",s[0]);
			break;
		
		default:
			Misc::throwStdErr("IO::XBaseTable::getFieldBoolean: Mismatching field data type \"%c\"",f.dataType);
		}
	
	return result;
	}

XBaseTable::Maybe<std::string> XBaseTable::getFieldString(const XBaseTable::Record& record,size_t fieldIndex) const
	{
	Maybe<std::string> result;
	
	const Field& f=fields[fieldIndex];
	const char* s=record.getStorage()+f.recordOffset;
	switch(f.dataType)
		{
		case 'C':
			result=std::string(s,f.size);
			break;
		
		case 'N':
			result=std::string(s,f.size-f.numDecimals);
			if(f.numDecimals>0)
				{
				result.value.push_back('.');
				result.value.append(s+(f.size-f.numDecimals),f.numDecimals);
				}
			break;
		
		case 'L':
			if(s[0]=='Y'||s[0]=='y'||s[0]=='T'||s[0]=='t')
				result="T";
			else if(s[0]=='N'||s[0]=='n'||s[0]=='F'||s[0]=='f')
				result="F";
			else
				result="?";
			break;
		
		case 'D':
			result=std::string(s+4,2);
			result.value.push_back('/');
			result.value.append(s+6,2);
			result.value.push_back('/');
			result.value.append(s,4);
			break;
		
		default:
			Misc::throwStdErr("IO::XBaseTable::getFieldString: Mismatching field data type \"%c\"",f.dataType);
		}
	
	return result;
	}

XBaseTable::Maybe<int> XBaseTable::getFieldInteger(const XBaseTable::Record& record,size_t fieldIndex) const
	{
	Maybe<int> result;
	
	const Field& f=fields[fieldIndex];
	const char* s=record.getStorage()+f.recordOffset;
	switch(f.dataType)
		{
		case 'N':
			{
			/* Skip whitespace at the beginning of the field: */
			size_t i=0;
			while(i<f.size&&isspace(s[i]))
				++i;
			
			/* Check for undefined values: */
			if(i<f.size)
				{
				/* Check for an optional sign: */
				bool negate=s[i]=='-';
				if(s[i]=='-'||s[i]=='+')
					++i;
				
				/* Read the integral part: */
				int integral=0;
				while(i<f.size-f.numDecimals&&isdigit(s[i]))
					{
					integral=integral*10+int(s[i]-'0');
					++i;
					}
				
				/* Negate the integral part if a minus sign was read: */
				if(negate)
					integral=-integral;
				
				result=integral;
				}
			
			break;
			}
		
		default:
			Misc::throwStdErr("IO::XBaseTable::getFieldInteger: Mismatching field data type \"%c\"",f.dataType);
		}
	
	return result;
	}

XBaseTable::Maybe<double> XBaseTable::getFieldNumber(const XBaseTable::Record& record,size_t fieldIndex) const
	{
	Maybe<double> result;
	
	const Field& f=fields[fieldIndex];
	const char* s=record.getStorage()+f.recordOffset;
	switch(f.dataType)
		{
		case 'N':
			{
			/* Skip whitespace at the beginning of the field: */
			size_t i=0;
			while(i<f.size&&isspace(s[i]))
				++i;
			
			/* Check for undefined values: */
			if(i<f.size)
				{
				/* Check for an optional sign: */
				bool negate=s[i]=='-';
				if(s[i]=='-'||s[i]=='+')
					++i;
				
				/* Read the integral part: */
				double integral=0.0;
				while(i<f.size-f.numDecimals&&isdigit(s[i]))
					{
					integral=integral*10.0+double(s[i]-'0');
					++i;
					}
				
				/* Check if the field is supposed to have a fractional part: */
				if(f.numDecimals>0)
					{
					/* Read the fractional part: */
					double fractional=0.0;
					double fractionalBase=1.0;
					
					/* Adjust the fractional base if the some fractional digits have already been skipped: */
					for(size_t j=f.size-f.numDecimals;j<i;++j)
						fractionalBase*=10.0;
					
					/* Read the fractional digits: */
					while(i<f.size&&isdigit(s[i]))
						{
						fractional=fractional*10.0+double(s[i]-'0');
						fractionalBase*=10.0;
						++i;
						}
					
					/* Assemble the result from integral and fractional parts: */
					result=integral+fractional/fractionalBase;
					}
				else
					result=integral;
				
				/* Negate the result if a minus sign was read: */
				if(negate)
					result.value=-result.value;
				}
			
			break;
			}
		
		default:
			Misc::throwStdErr("IO::XBaseTable::getFieldNumber: Mismatching field data type \"%c\"",f.dataType);
		}
	
	return result;
	}

XBaseTable::Maybe<XBaseTable::Date> XBaseTable::getFieldDate(const XBaseTable::Record& record,size_t fieldIndex) const
	{
	Maybe<Date> result;
	
	const Field& f=fields[fieldIndex];
	const char* s=record.getStorage()+f.recordOffset;
	switch(f.dataType)
		{
		case 'D':
			{
			/* Read the year: */
			int year=0;
			for(size_t i=0;i<4;++i)
				year=year*10+int(s[i]-'0');
			
			/* Read the month: */
			int month=0;
			for(size_t i=0;i<2;++i)
				month=month*10+int(s[i]-'0');
			
			/* Read the day: */
			int day=0;
			for(size_t i=0;i<2;++i)
				day=day*10+int(s[i]-'0');
			
			/* Check the date for validity: */
			if(year!=0&&month>=1&&month<=12&&day>=1&&day<=31)
				result=Date(year,month,day);
			
			break;
			}
		
		default:
			Misc::throwStdErr("IO::XBaseTable::getFieldDate: Mismatching field data type \"%c\"",f.dataType);
		}
	
	return result;
	}

}
