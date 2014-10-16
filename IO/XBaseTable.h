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

#ifndef IO_XBASETABLE_INCLUDED
#define IO_XBASETABLE_INCLUDED

#include <string>
#include <vector>
#include <IO/SeekableFile.h>

namespace IO {

class XBaseTable
	{
	/* Embedded classes: */
	private:
	typedef SeekableFile::Offset Offset; // Type for file offsets
	
	struct Field // Structure representing a field in a record
		{
		/* Elements: */
		public:
		char name[11]; // Field name
		char dataType; // Field data type
		ptrdiff_t recordOffset; // Offset of field in data records
		size_t size; // Length of field in bytes
		size_t numDecimals; // Number of decimals in the field
		};
	
	public:
	enum FieldDataType // Enumerated type for field data types
		{
		BOOLEAN,STRING,NUMBER,DATE,UNKNOWN
		};
	
	struct Date // Structure to represent dates read from an XBase table
		{
		/* Elements: */
		public:
		int year;
		int month; // January is 1, December is 12
		int day;
		
		/* Constructors and destructors: */
		Date(void)
			:year(),month(),day()
			{
			}
		Date(int sYear,int sMonth,int sDay)
			:year(sYear),month(sMonth),day(sDay)
			{
			}
		};
	
	template <class ValueParam>
	struct Maybe // Structure to represent values that may be undefined
		{
		/* Elements: */
		public:
		bool defined; // Flag if the value is defined
		ValueParam value; // Value, if defined
		
		/* Constructors and destructors: */
		Maybe(void) // Creates an undefined value
			:defined(false),value()
			{
			}
		Maybe(const ValueParam& sValue) // Creates a defined value
			:defined(true),value(sValue)
			{
			}
		Maybe& operator=(const ValueParam& sValue) // Assigns a defined value
			{
			defined=true;
			value=sValue;
			return *this;
			}
		};
	
	class Record // Class to represent a record in memory
		{
		friend class XBaseTable;
		
		/* Elements: */
		private:
		unsigned int* storage; // Pointer to allocated record storage; first element is reference count
		
		/* Constructors and destructors: */
		public:
		Record(void) // Creates a record with no storage
			:storage(0)
			{
			}
		private:
		Record(unsigned int* sStorage) // Creates a new record and attaches to the given unattached storage
			:storage(sStorage)
			{
			/* Attach to the storage: */
			storage[0]=1;
			}
		public:
		Record(const Record& source) // Attaches to the given record's storage
			:storage(source.storage)
			{
			/* Attach to the storage: */
			if(storage!=0)
				++storage[0];
			}
		Record& operator=(const Record& source) // Assignment operator
			{
			/* Check for aliasing: */
			if(this!=&source)
				{
				/* Detach from the current storage: */
				if(storage!=0&&--storage[0]==0)
					delete[] storage;
				
				/* Attach to the new storage: */
				storage=source.storage;
				if(storage!=0)
					++storage[0];
				}
			return *this;
			}
		~Record(void) // Destroys the record
			{
			/* Detach from the storage: */
			if(storage!=0&&--storage[0]==0)
				delete[] storage;
			}
		
		/* Methods: */
		const char* getStorage(void) const // Returns the record's raw storage
			{
			return reinterpret_cast<const char*>(storage+1);
			}
		};
	
	/* Elements: */
	private:
	SeekableFilePtr file; // The underlying file object
	unsigned char version; // Format version number of the file
	std::vector<Field> fields; // List of fields in each record
	Offset headerSize; // Total size of file header in bytes
	size_t recordSize; // Size of each record in bytes
	size_t numRecords; // Number of records in the file
	
	/* Constructors and destructors: */
	public:
	XBaseTable(const char* fileName,SeekableFilePtr sFile); // Opens the given file of the given name as an XBase table
	~XBaseTable(void); // Destroys the table
	
	/* Methods: */
	size_t getNumFields(void) const // Returns the number of fields in each record
		{
		return fields.size();
		}
	const char* getFieldName(size_t index) const // Returns the name of the given field
		{
		return fields[index].name;
		}
	FieldDataType getFieldDataType(size_t index) const; // Returns the data type of the given field
	size_t getFieldSize(size_t index) const // Returns the size of the given field
		{
		return fields[index].size;
		}
	size_t getFieldNumDecimals(size_t index) const // Returns the number of decimals of the given field; only valid if field is of number type
		{
		return fields[index].numDecimals;
		}
	size_t getNumRecords(void) const // Returns the number of records in the file
		{
		return numRecords;
		}
	Record readRecord(size_t recordIndex); // Reads the record of the given index
	Record makeRecord(void) const; // Returns a new, uninitialized record
	void readRecord(size_t recordIndex,Record& record); // Ditto, but reads into given record
	bool isDeleted(const Record& record) const // Returns true if the given record has been flagged as deleted
		{
		return record.getStorage()[0]=='*';
		}
	Maybe<bool> getFieldBoolean(const Record& record,size_t fieldIndex) const; // Returns the given field from the given record as a boolean
	Maybe<std::string> getFieldString(const Record& record,size_t fieldIndex) const; // Returns the given field from the given record as a string
	Maybe<int> getFieldInteger(const Record& record,size_t fieldIndex) const; // Returns the given field from the given record as an integer number by ignoring decimals
	Maybe<double> getFieldNumber(const Record& record,size_t fieldIndex) const; // Returns the given field from the given record as a floating-point number
	Maybe<Date> getFieldDate(const Record& record,size_t fieldIndex) const; // Returns the given field from the given record as a date
	};

}

#endif
